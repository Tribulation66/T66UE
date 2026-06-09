param(
    [ValidateSet("Development", "Shipping")]
    [string]$ClientConfig = "Development",

    [string]$EngineRoot = "C:\Program Files\Epic Games\UE_5.7",

    [string]$StageRoot = "",

    [string]$OutputRoot,

    [int]$BuildProcessWaitSeconds = 120,

    [int]$FrontendTimeoutSeconds = 70,

    [int]$DurableTimeoutSeconds = 120,

    [int]$LifecycleTimeoutSeconds = 180,

    [int]$DurableSlotIndex = 8,

    [int]$LifecycleTravels = 6,

    [int]$LifecycleStressCount = 6,

    [switch]$SkipBuild,

    [switch]$SkipCook,

    [switch]$SkipShortcutRefresh,

    [switch]$ResetSavedGames,

    [switch]$SkipStage,

    [switch]$SkipSmoke,

    [switch]$SkipFrontend,

    [switch]$SkipDurable,

    [switch]$SkipLifecycle,

    [switch]$ContinueOnFailure,

    [switch]$PrintOnly
)

$ErrorActionPreference = "Stop"

$ProjectRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot "..")).TrimEnd("\")
$StageScript = Join-Path $PSScriptRoot "StageStandaloneBuild.ps1"
$SmokeSuiteScript = Join-Path $PSScriptRoot "RunPreReleaseSmokeSuite.ps1"

if ([string]::IsNullOrWhiteSpace($StageRoot)) {
    $StageRoot = Join-Path $ProjectRoot "Saved\StagedBuilds"
} else {
    $StageRoot = [System.IO.Path]::GetFullPath($StageRoot)
}

if (-not $OutputRoot) {
    $stamp = Get-Date -Format "yyyyMMdd_HHmmss"
    $OutputRoot = Join-Path $ProjectRoot "Saved\StagedBuildReadiness\$stamp"
}
$OutputRoot = [System.IO.Path]::GetFullPath($OutputRoot)

$StagedExe = [System.IO.Path]::GetFullPath((Join-Path $StageRoot "Windows\T66\Binaries\Win64\T66.exe"))
$SummaryJsonPath = Join-Path $OutputRoot "summary.json"
$SummaryMdPath = Join-Path $OutputRoot "summary.md"

function Get-TextOrEmpty {
    param([string]$Path)

    if (Test-Path -LiteralPath $Path) {
        return Get-Content -LiteralPath $Path -Raw
    }

    return ""
}

function Convert-ArgListToCommandLine {
    param([string[]]$ArgsList)

    $quoted = foreach ($arg in $ArgsList) {
        if ($arg -match '[\s"`]') {
            '"' + ($arg -replace '"', '\"') + '"'
        } else {
            $arg
        }
    }

    return $quoted -join " "
}

function Convert-CimCreationDateToIsoOrNull {
    param([object]$CreationDate)

    if (-not $CreationDate) {
        return $null
    }

    try {
        if ($CreationDate -is [datetime]) {
            return $CreationDate.ToString("o")
        }

        return [System.Management.ManagementDateTimeConverter]::ToDateTime([string]$CreationDate).ToString("o")
    } catch {
        return $null
    }
}

function Get-BlockingBuildProcessSnapshot {
    $processes = @(
        Get-CimInstance Win32_Process -ErrorAction SilentlyContinue |
            Where-Object {
                $name = [string]$_.Name
                $commandLine = [string]$_.CommandLine
                if ($name -in @("AutomationTool.exe", "UnrealBuildTool.exe", "UnrealPak.exe")) {
                    return $true
                }

                if ($name -eq "dotnet.exe") {
                    return ($commandLine -match "AutomationTool|UnrealBuildTool|UnrealPak|BuildCookRun")
                }

                if ($name -in @("powershell.exe", "pwsh.exe")) {
                    return ($commandLine -match "-File\s+.*(StageStandaloneBuild\.ps1|RunUAT\.ps1)|RunUAT\.bat")
                }

                return $false
            }
    )
    return @($processes | ForEach-Object {
        [pscustomobject]@{
            Id = $_.ProcessId
            ProcessName = $_.Name
            StartTime = Convert-CimCreationDateToIsoOrNull $_.CreationDate
            Path = $_.ExecutablePath
            CommandLine = $_.CommandLine
        }
    })
}

function Wait-ForBuildProcessesToDrain {
    param([int]$TimeoutSeconds)

    $stopwatch = [System.Diagnostics.Stopwatch]::StartNew()
    $initial = Get-BlockingBuildProcessSnapshot
    if ($initial.Count -eq 0 -or $TimeoutSeconds -le 0) {
        return [pscustomobject]@{
            Status = if ($initial.Count -eq 0) { "CLEAR" } else { "TIMEOUT" }
            WaitedSeconds = 0
            InitialProcesses = $initial
            RemainingProcesses = $initial
        }
    }

    Write-Host "Waiting for existing Unreal build/stage processes to drain for up to $TimeoutSeconds seconds..."
    do {
        Start-Sleep -Seconds 5
        $remaining = Get-BlockingBuildProcessSnapshot
        if ($remaining.Count -eq 0) {
            $stopwatch.Stop()
            return [pscustomobject]@{
                Status = "CLEAR"
                WaitedSeconds = [Math]::Round($stopwatch.Elapsed.TotalSeconds, 3)
                InitialProcesses = $initial
                RemainingProcesses = @()
            }
        }
    } while ($stopwatch.Elapsed.TotalSeconds -lt $TimeoutSeconds)

    $stopwatch.Stop()
    return [pscustomobject]@{
        Status = "TIMEOUT"
        WaitedSeconds = [Math]::Round($stopwatch.Elapsed.TotalSeconds, 3)
        InitialProcesses = $initial
        RemainingProcesses = Get-BlockingBuildProcessSnapshot
    }
}

function Invoke-LoggedProcess {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Name,

        [Parameter(Mandatory = $true)]
        [string]$FilePath,

        [Parameter(Mandatory = $true)]
        [string[]]$ArgumentList,

        [Parameter(Mandatory = $true)]
        [string]$OutputDirectory
    )

    New-Item -ItemType Directory -Force -Path $OutputDirectory | Out-Null
    $stdoutPath = Join-Path $OutputDirectory "stdout.log"
    $stderrPath = Join-Path $OutputDirectory "stderr.log"
    foreach ($path in @($stdoutPath, $stderrPath)) {
        if (Test-Path -LiteralPath $path) {
            Remove-Item -LiteralPath $path -Force
        }
    }

    $argumentString = Convert-ArgListToCommandLine -ArgsList $ArgumentList
    $commandLine = "$FilePath $argumentString"
    Write-Host "=== $Name ==="
    Write-Host $commandLine

    $exitCode = $null
    $launchError = $null
    $stopwatch = [System.Diagnostics.Stopwatch]::StartNew()
    try {
        $process = Start-Process `
            -FilePath $FilePath `
            -ArgumentList $argumentString `
            -WorkingDirectory $ProjectRoot `
            -RedirectStandardOutput $stdoutPath `
            -RedirectStandardError $stderrPath `
            -Wait `
            -PassThru
        $exitCode = $process.ExitCode
    } catch {
        $exitCode = 1
        $launchError = $_.Exception.Message
        $launchError | Set-Content -LiteralPath $stderrPath -Encoding UTF8
    }
    $stopwatch.Stop()

    return [pscustomobject]@{
        Name = $Name
        Status = if ($exitCode -eq 0) { "PASS" } else { "FAIL" }
        ExitCode = $exitCode
        DurationSeconds = [Math]::Round($stopwatch.Elapsed.TotalSeconds, 3)
        Command = $commandLine
        OutputDirectory = $OutputDirectory
        Stdout = $stdoutPath
        Stderr = $stderrPath
        Error = $launchError
    }
}

function Get-StagedExecutableInfo {
    param([string]$Path)

    if (-not (Test-Path -LiteralPath $Path)) {
        return [pscustomobject]@{
            Path = [System.IO.Path]::GetFullPath($Path)
            Exists = $false
            Length = $null
            LastWriteTimeUtc = $null
        }
    }

    $item = Get-Item -LiteralPath $Path
    return [pscustomobject]@{
        Path = $item.FullName
        Exists = $true
        Length = $item.Length
        LastWriteTimeUtc = $item.LastWriteTimeUtc.ToString("o")
    }
}

function Get-ShortcutVerification {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Name,

        [Parameter(Mandatory = $true)]
        [string]$ShortcutPath,

        [Parameter(Mandatory = $true)]
        [string]$ExpectedTarget,

        [bool]$Required
    )

    $shortcutFullPath = [System.IO.Path]::GetFullPath($ShortcutPath)
    $expectedFullPath = [System.IO.Path]::GetFullPath($ExpectedTarget)
    if (-not (Test-Path -LiteralPath $shortcutFullPath)) {
        return [pscustomobject]@{
            Name = $Name
            Status = if ($Required) { "FAIL" } else { "NOT_PINNED" }
            Required = $Required
            ShortcutPath = $shortcutFullPath
            Exists = $false
            ExpectedTarget = $expectedFullPath
            ActualTarget = $null
            Arguments = $null
            WorkingDirectory = $null
            Match = $false
            Error = if ($Required) { "Required shortcut is missing." } else { "Pinned taskbar shortcut is not present on this machine." }
        }
    }

    $actualTarget = $null
    $arguments = $null
    $workingDirectory = $null
    $errorText = $null
    try {
        $shell = New-Object -ComObject WScript.Shell
        $shortcut = $shell.CreateShortcut($shortcutFullPath)
        $actualTarget = $shortcut.TargetPath
        $arguments = $shortcut.Arguments
        $workingDirectory = $shortcut.WorkingDirectory
    } catch {
        $errorText = $_.Exception.Message
    }

    $actualFullPath = if ([string]::IsNullOrWhiteSpace($actualTarget)) { $actualTarget } else { [System.IO.Path]::GetFullPath($actualTarget) }
    $matches = $false
    if (-not [string]::IsNullOrWhiteSpace($actualFullPath)) {
        $matches = [string]::Equals($actualFullPath, $expectedFullPath, [System.StringComparison]::OrdinalIgnoreCase)
    }

    return [pscustomobject]@{
        Name = $Name
        Status = if ($matches) { "PASS" } else { "FAIL" }
        Required = $Required
        ShortcutPath = $shortcutFullPath
        Exists = $true
        ExpectedTarget = $expectedFullPath
        ActualTarget = $actualFullPath
        Arguments = $arguments
        WorkingDirectory = $workingDirectory
        Match = $matches
        Error = $errorText
    }
}

function Get-SmokeGateRollup {
    param([string]$SummaryPath)

    if (-not (Test-Path -LiteralPath $SummaryPath)) {
        return @()
    }

    try {
        $summary = Get-Content -LiteralPath $SummaryPath -Raw | ConvertFrom-Json
        return @($summary.Gates | ForEach-Object {
            [pscustomobject]@{
                Name = $_.Name
                Status = $_.Status
                ExitCode = $_.ExitCode
                Summary = $_.Summary
            }
        })
    } catch {
        return @([pscustomobject]@{
            Name = "SummaryParse"
            Status = "FAIL"
            ExitCode = $null
            Summary = $SummaryPath
            Error = $_.Exception.Message
        })
    }
}

function Write-ReadinessSummary {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Status,

        [string]$ErrorMessage,

        [object]$BuildProcessPreflight,

        [object]$StageResult,

        [object]$ExecutableInfo,

        [object[]]$ShortcutResults,

        [object]$SmokeResult,

        [object[]]$SmokeGateRollup
    )

    New-Item -ItemType Directory -Force -Path $OutputRoot | Out-Null

    $summary = [pscustomobject]@{
        Status = $Status
        CreatedAt = (Get-Date).ToString("o")
        ClientConfig = $ClientConfig
        ProjectRoot = $ProjectRoot
        StageRoot = $StageRoot
        StagedExe = $StagedExe
        OutputRoot = $OutputRoot
        PrintOnly = [bool]$PrintOnly
        Skipped = [pscustomobject]@{
            Stage = [bool]$SkipStage
            Smoke = [bool]$SkipSmoke
            Frontend = [bool]$SkipFrontend
            Durable = [bool]$SkipDurable
            Lifecycle = [bool]$SkipLifecycle
        }
        Error = $ErrorMessage
        BuildProcessPreflight = $BuildProcessPreflight
        Stage = $StageResult
        StagedExecutable = $ExecutableInfo
        ShortcutVerification = $ShortcutResults
        SmokeSuite = $SmokeResult
        SmokeGates = $SmokeGateRollup
    }

    $summary | ConvertTo-Json -Depth 10 | Set-Content -LiteralPath $SummaryJsonPath -Encoding UTF8

    $md = @()
    $md += "# Staged Build Readiness Gate"
    $md += ""
    $md += "Status: $Status"
    $md += "Client config: $ClientConfig"
    $md += "Staged exe: $StagedExe"
    $md += "Output root: $OutputRoot"
    if ($ErrorMessage) {
        $md += "Error: $ErrorMessage"
    }
    $md += ""
    $md += "## Stage"
    $md += ""
    $md += "- Status: $($StageResult.Status)"
    $md += "- Exit code: $($StageResult.ExitCode)"
    $md += "- Duration seconds: $($StageResult.DurationSeconds)"
    $md += "- Stdout: $($StageResult.Stdout)"
    $md += "- Stderr: $($StageResult.Stderr)"
    $md += ""
    $md += "## Staged Executable"
    $md += ""
    $md += "- Exists: $($ExecutableInfo.Exists)"
    $md += "- Length: $($ExecutableInfo.Length)"
    $md += "- Last write UTC: $($ExecutableInfo.LastWriteTimeUtc)"
    $md += ""
    $md += "## Shortcuts"
    $md += ""
    foreach ($shortcut in $ShortcutResults) {
        $md += "- $($shortcut.Name): $($shortcut.Status) expected=`"$($shortcut.ExpectedTarget)`" actual=`"$($shortcut.ActualTarget)`""
    }
    $md += ""
    $md += "## Smoke Suite"
    $md += ""
    $md += "- Status: $($SmokeResult.Status)"
    $md += "- Exit code: $($SmokeResult.ExitCode)"
    $md += "- Summary: $($SmokeResult.Summary)"
    foreach ($gate in $SmokeGateRollup) {
        $md += "- $($gate.Name): $($gate.Status)"
    }
    $md += ""
    $md += "This gate is the one-shot readiness wrapper over `StageStandaloneBuild.ps1`, shortcut target verification, and `RunPreReleaseSmokeSuite.ps1`. The child stage and smoke logs remain the detailed evidence sources."
    $md -join "`r`n" | Set-Content -LiteralPath $SummaryMdPath -Encoding UTF8

    Write-Host "Staged build readiness $Status`: $OutputRoot"
    Write-Host "Summary: $SummaryJsonPath"
}

if ($BuildProcessWaitSeconds -lt 0 -or $BuildProcessWaitSeconds -gt 1800) {
    throw "BuildProcessWaitSeconds must be between 0 and 1800."
}
if ($FrontendTimeoutSeconds -lt 10 -or $FrontendTimeoutSeconds -gt 600) {
    throw "FrontendTimeoutSeconds must be between 10 and 600."
}
if ($DurableTimeoutSeconds -lt 10 -or $DurableTimeoutSeconds -gt 600) {
    throw "DurableTimeoutSeconds must be between 10 and 600."
}
if ($LifecycleTimeoutSeconds -lt 10 -or $LifecycleTimeoutSeconds -gt 600) {
    throw "LifecycleTimeoutSeconds must be between 10 and 600."
}
if ($DurableSlotIndex -lt 0 -or $DurableSlotIndex -gt 8) {
    throw "DurableSlotIndex must be between 0 and 8."
}
if ($LifecycleTravels -lt 1 -or $LifecycleTravels -gt 24) {
    throw "LifecycleTravels must be between 1 and 24."
}
if ($LifecycleStressCount -lt 1 -or $LifecycleStressCount -gt 24) {
    throw "LifecycleStressCount must be between 1 and 24."
}
if (-not (Test-Path -LiteralPath $StageScript)) {
    throw "Missing stage script: $StageScript"
}
if (-not (Test-Path -LiteralPath $SmokeSuiteScript)) {
    throw "Missing smoke suite script: $SmokeSuiteScript"
}
if ($ClientConfig -eq "Shipping" -and -not $SkipSmoke -and -not $SkipDurable -and -not $SkipLifecycle) {
    throw "Shipping staged builds do not support the durable save and lifecycle smoke gates. Use Development, pass -SkipSmoke, or explicitly skip durable/lifecycle."
}
if ($SkipSmoke -and ($SkipFrontend -or $SkipDurable -or $SkipLifecycle)) {
    throw "Do not combine -SkipSmoke with individual smoke-gate skip switches."
}
if ($SkipFrontend -and $SkipDurable -and $SkipLifecycle -and -not $SkipSmoke) {
    throw "At least one smoke gate must run unless -SkipSmoke is passed."
}

New-Item -ItemType Directory -Force -Path $OutputRoot | Out-Null
foreach ($path in @($SummaryJsonPath, $SummaryMdPath)) {
    if (Test-Path -LiteralPath $path) {
        Remove-Item -LiteralPath $path -Force
    }
}

$stageArgs = @(
    "-NoProfile",
    "-ExecutionPolicy",
    "Bypass",
    "-File",
    $StageScript,
    "-ClientConfig",
    $ClientConfig,
    "-EngineRoot",
    $EngineRoot,
    "-StageRoot",
    $StageRoot
)
if ($SkipBuild) { $stageArgs += "-SkipBuild" }
if ($SkipCook) { $stageArgs += "-SkipCook" }
if ($SkipShortcutRefresh) { $stageArgs += "-SkipShortcutRefresh" }
if ($ResetSavedGames) { $stageArgs += "-ResetSavedGames" }

$smokeOutputRoot = Join-Path $OutputRoot "smoke_suite"
$smokeArgs = @(
    "-NoProfile",
    "-ExecutionPolicy",
    "Bypass",
    "-File",
    $SmokeSuiteScript,
    "-Exe",
    $StagedExe,
    "-OutputRoot",
    $smokeOutputRoot,
    "-FrontendTimeoutSeconds",
    "$FrontendTimeoutSeconds",
    "-DurableTimeoutSeconds",
    "$DurableTimeoutSeconds",
    "-LifecycleTimeoutSeconds",
    "$LifecycleTimeoutSeconds",
    "-DurableSlotIndex",
    "$DurableSlotIndex",
    "-LifecycleTravels",
    "$LifecycleTravels",
    "-LifecycleStressCount",
    "$LifecycleStressCount"
)
if ($SkipFrontend) { $smokeArgs += "-SkipFrontend" }
if ($SkipDurable) { $smokeArgs += "-SkipDurable" }
if ($SkipLifecycle) { $smokeArgs += "-SkipLifecycle" }
if ($ContinueOnFailure) { $smokeArgs += "-ContinueOnFailure" }
if ($PrintOnly) { $smokeArgs += "-PrintOnly" }

$buildProcessPreflight = $null
$stageResult = $null
$exeInfo = Get-StagedExecutableInfo -Path $StagedExe
$shortcutResults = @()
$smokeResult = $null
$smokeGateRollup = @()
$status = if ($PrintOnly) { "PRINT_ONLY" } else { "PASS" }
$errorMessage = $null

try {
    $buildProcessPreflight = if ($SkipStage -or $PrintOnly) {
        [pscustomobject]@{
            Status = "SKIPPED"
            WaitedSeconds = 0
            InitialProcesses = @()
            RemainingProcesses = @()
        }
    } else {
        Wait-ForBuildProcessesToDrain -TimeoutSeconds $BuildProcessWaitSeconds
    }

    if ($buildProcessPreflight.Status -eq "TIMEOUT") {
        throw "Existing Unreal build/stage processes did not drain within $BuildProcessWaitSeconds seconds."
    }

    if ($PrintOnly) {
        $stageResult = [pscustomobject]@{
            Name = "StageStandaloneBuild"
            Status = "PRINT_ONLY"
            ExitCode = $null
            DurationSeconds = 0
            Command = "powershell $(Convert-ArgListToCommandLine -ArgsList $stageArgs)"
            OutputDirectory = Join-Path $OutputRoot "stage"
            Stdout = $null
            Stderr = $null
            Error = "PrintOnly does not execute StageStandaloneBuild.ps1 because that script has no dry-run mode."
        }
        Write-Host "=== StageStandaloneBuild ==="
        Write-Host $stageResult.Command
    } elseif ($SkipStage) {
        $stageResult = [pscustomobject]@{
            Name = "StageStandaloneBuild"
            Status = "SKIPPED"
            ExitCode = $null
            DurationSeconds = 0
            Command = "powershell $(Convert-ArgListToCommandLine -ArgsList $stageArgs)"
            OutputDirectory = Join-Path $OutputRoot "stage"
            Stdout = $null
            Stderr = $null
            Error = "Skipped by -SkipStage."
        }
    } else {
        $stageResult = Invoke-LoggedProcess -Name "StageStandaloneBuild" -FilePath "powershell" -ArgumentList $stageArgs -OutputDirectory (Join-Path $OutputRoot "stage")
        if ($stageResult.Status -ne "PASS") {
            throw "StageStandaloneBuild.ps1 failed with exit code $($stageResult.ExitCode)."
        }
    }

    $exeInfo = Get-StagedExecutableInfo -Path $StagedExe
    if (-not $PrintOnly -and -not $exeInfo.Exists) {
        throw "Missing staged executable after stage step: $StagedExe"
    }

    $pinnedShortcut = Join-Path $env:APPDATA "Microsoft\Internet Explorer\Quick Launch\User Pinned\TaskBar\T66 Standalone.lnk"
    $shortcutResults = @(
        (Get-ShortcutVerification -Name "ProjectRoot" -ShortcutPath (Join-Path $ProjectRoot "T66 Standalone.lnk") -ExpectedTarget $StagedExe -Required $true),
        (Get-ShortcutVerification -Name "PinnedTaskbar" -ShortcutPath $pinnedShortcut -ExpectedTarget $StagedExe -Required $false)
    )

    if (-not $PrintOnly) {
        $failedShortcut = $shortcutResults | Where-Object { $_.Status -eq "FAIL" } | Select-Object -First 1
        if ($failedShortcut) {
            throw "Shortcut verification failed for $($failedShortcut.Name)."
        }
    }

    if ($SkipSmoke) {
        $smokeResult = [pscustomobject]@{
            Name = "PreReleaseSmokeSuite"
            Status = "SKIPPED"
            ExitCode = $null
            DurationSeconds = 0
            Command = "powershell $(Convert-ArgListToCommandLine -ArgsList $smokeArgs)"
            OutputDirectory = $smokeOutputRoot
            Summary = $null
            Stdout = $null
            Stderr = $null
            Error = "Skipped by -SkipSmoke."
        }
    } elseif ($PrintOnly -and -not $exeInfo.Exists) {
        $smokeResult = [pscustomobject]@{
            Name = "PreReleaseSmokeSuite"
            Status = "PRINT_ONLY"
            ExitCode = $null
            DurationSeconds = 0
            Command = "powershell $(Convert-ArgListToCommandLine -ArgsList $smokeArgs)"
            OutputDirectory = $smokeOutputRoot
            Summary = Join-Path $smokeOutputRoot "summary.json"
            Stdout = $null
            Stderr = $null
            Error = "PrintOnly smoke command was not launched because the staged executable does not exist yet."
        }
        Write-Host "=== PreReleaseSmokeSuite ==="
        Write-Host $smokeResult.Command
    } else {
        $smokeProcess = Invoke-LoggedProcess -Name "PreReleaseSmokeSuite" -FilePath "powershell" -ArgumentList $smokeArgs -OutputDirectory (Join-Path $OutputRoot "smoke_process")
        $smokeSummaryPath = Join-Path $smokeOutputRoot "summary.json"
        $smokeStatus = $smokeProcess.Status
        $parsedSmokeStatus = $null
        $smokeError = $smokeProcess.Error
        if (Test-Path -LiteralPath $smokeSummaryPath) {
            try {
                $parsedSmoke = Get-Content -LiteralPath $smokeSummaryPath -Raw | ConvertFrom-Json
                $parsedSmokeStatus = [string]$parsedSmoke.Status
                $smokeStatus = $parsedSmokeStatus
            } catch {
                $smokeStatus = "FAIL"
                $smokeError = $_.Exception.Message
            }
        } elseif ($smokeProcess.Status -eq "PASS") {
            $smokeStatus = "FAIL"
            $smokeError = "Smoke suite exited 0 but did not produce summary.json."
        }

        $smokeResult = [pscustomobject]@{
            Name = "PreReleaseSmokeSuite"
            Status = $smokeStatus
            ExitCode = $smokeProcess.ExitCode
            DurationSeconds = $smokeProcess.DurationSeconds
            Command = $smokeProcess.Command
            OutputDirectory = $smokeOutputRoot
            Summary = $smokeSummaryPath
            Stdout = $smokeProcess.Stdout
            Stderr = $smokeProcess.Stderr
            ParsedStatus = $parsedSmokeStatus
            Error = $smokeError
        }
        $smokeGateRollup = Get-SmokeGateRollup -SummaryPath $smokeSummaryPath

        if ($smokeResult.Status -notin @("PASS", "PRINT_ONLY")) {
            throw "RunPreReleaseSmokeSuite.ps1 returned $($smokeResult.Status)."
        }
    }
} catch {
    $status = "FAIL"
    $errorMessage = $_.Exception.Message
}

if (-not $smokeResult) {
    $smokeResult = [pscustomobject]@{
        Name = "PreReleaseSmokeSuite"
        Status = "NOT_RUN"
        ExitCode = $null
        DurationSeconds = 0
        Command = "powershell $(Convert-ArgListToCommandLine -ArgsList $smokeArgs)"
        OutputDirectory = $smokeOutputRoot
        Summary = Join-Path $smokeOutputRoot "summary.json"
        Stdout = $null
        Stderr = $null
        Error = "Not run because an earlier readiness step failed."
    }
}

if (-not $stageResult) {
    $stageResult = [pscustomobject]@{
        Name = "StageStandaloneBuild"
        Status = "NOT_RUN"
        ExitCode = $null
        DurationSeconds = 0
        Command = "powershell $(Convert-ArgListToCommandLine -ArgsList $stageArgs)"
        OutputDirectory = Join-Path $OutputRoot "stage"
        Stdout = $null
        Stderr = $null
        Error = "Not run."
    }
}

if (-not $buildProcessPreflight) {
    $buildProcessPreflight = [pscustomobject]@{
        Status = "NOT_RUN"
        WaitedSeconds = 0
        InitialProcesses = @()
        RemainingProcesses = @()
    }
}

if (-not $ShortcutResults) {
    $shortcutResults = @()
}

if ($status -ne "FAIL") {
    if (-not $PrintOnly -and $stageResult.Status -notin @("PASS", "SKIPPED")) {
        $status = "FAIL"
        $errorMessage = "Stage step returned $($stageResult.Status)."
    } elseif (-not $PrintOnly -and -not $exeInfo.Exists) {
        $status = "FAIL"
        $errorMessage = "Missing staged executable: $StagedExe"
    } elseif (-not $PrintOnly -and ($shortcutResults | Where-Object { $_.Status -eq "FAIL" })) {
        $failed = $shortcutResults | Where-Object { $_.Status -eq "FAIL" } | Select-Object -First 1
        $status = "FAIL"
        $errorMessage = "Shortcut verification failed for $($failed.Name)."
    } elseif (-not $SkipSmoke -and $smokeResult.Status -notin @("PASS", "PRINT_ONLY")) {
        $status = "FAIL"
        $errorMessage = "Smoke suite returned $($smokeResult.Status)."
    }
}

Write-ReadinessSummary `
    -Status $status `
    -ErrorMessage $errorMessage `
    -BuildProcessPreflight $buildProcessPreflight `
    -StageResult $stageResult `
    -ExecutableInfo $exeInfo `
    -ShortcutResults $shortcutResults `
    -SmokeResult $smokeResult `
    -SmokeGateRollup $smokeGateRollup

if ($status -eq "FAIL") {
    throw $errorMessage
}
