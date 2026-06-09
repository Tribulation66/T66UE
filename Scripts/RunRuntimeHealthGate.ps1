param(
    [string]$Exe = "C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe",

    [string]$OutputRoot,

    [int]$ExpectedPerformanceSchemaVersion = 0,

    [int]$RuntimeHoldSeconds = 8,

    [int]$RuntimeTimeoutSeconds = 90,

    [int]$StagedReadinessTimeoutSeconds = 120,

    [switch]$SkipStaticChecks,

    [switch]$SkipStagedReadiness,

    [switch]$SkipRuntimeLaunch,

    [switch]$SkipPerformanceArtifactChecks,

    [switch]$PrintOnly
)

$ErrorActionPreference = "Stop"

$ProjectRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot "..")).TrimEnd("\")
$Exe = [System.IO.Path]::GetFullPath($Exe)

if (-not $OutputRoot) {
    $stamp = Get-Date -Format "yyyyMMdd_HHmmss"
    $OutputRoot = Join-Path $ProjectRoot "Saved\RuntimeHealthGate\$stamp"
}
$OutputRoot = [System.IO.Path]::GetFullPath($OutputRoot)
$SummaryJsonPath = Join-Path $OutputRoot "summary.json"
$SummaryMdPath = Join-Path $OutputRoot "summary.md"

$Checks = New-Object System.Collections.Generic.List[object]
$CreatedAtUtc = (Get-Date).ToUniversalTime().ToString("o")

function Convert-ArgListToCommandLine {
    param([string[]]$Arguments)

    $quoted = foreach ($arg in $Arguments) {
        if ($null -eq $arg) {
            continue
        }

        if ($arg -match '[\s"]') {
            '"' + ($arg -replace '"', '\"') + '"'
        } else {
            $arg
        }
    }

    return ($quoted -join " ")
}

function Add-Check {
    param(
        [string]$Name,
        [ValidateSet("PASS", "FAIL", "WARN", "SKIPPED", "PRINT_ONLY")]
        [string]$Status,
        [string]$Evidence = "",
        [string]$Details = ""
    )

    $script:Checks.Add([pscustomobject]@{
        Name = $Name
        Status = $Status
        Evidence = $Evidence
        Details = $Details
    }) | Out-Null
}

function Get-RepoRelativePath {
    param([string]$Path)

    $full = [System.IO.Path]::GetFullPath($Path)
    if ($full.StartsWith($ProjectRoot, [System.StringComparison]::OrdinalIgnoreCase)) {
        return $full.Substring($ProjectRoot.Length).TrimStart("\")
    }

    return $full
}

function Resolve-StagedGameRoot {
    param([string]$ExecutablePath)

    $win64Dir = Split-Path -Parent $ExecutablePath
    $binariesDir = Split-Path -Parent $win64Dir
    return Split-Path -Parent $binariesDir
}

function Get-JsonOrNull {
    param([string]$Path)

    if (-not (Test-Path -LiteralPath $Path)) {
        return $null
    }

    try {
        return Get-Content -LiteralPath $Path -Raw | ConvertFrom-Json
    } catch {
        return $null
    }
}

function Get-PerformanceSchemaVersion {
    if ($ExpectedPerformanceSchemaVersion -gt 0) {
        return $ExpectedPerformanceSchemaVersion
    }

    $sourcePath = Join-Path $ProjectRoot "Source\T66\PerformanceSystem\T66PerformanceSubsystem.cpp"
    if (Test-Path -LiteralPath $sourcePath) {
        $source = Get-Content -LiteralPath $sourcePath -Raw
        $match = [regex]::Match($source, "T66PerformanceSchemaVersion\s*=\s*(\d+)")
        if ($match.Success) {
            return [int]$match.Groups[1].Value
        }
    }

    $changelog = Join-Path $ProjectRoot "PerformanceSystem\schema\SCHEMA_CHANGELOG.md"
    if (Test-Path -LiteralPath $changelog) {
        $text = Get-Content -LiteralPath $changelog -Raw
        $match = [regex]::Match($text, "##\s+SchemaVersion\s+(\d+)")
        if ($match.Success) {
            return [int]$match.Groups[1].Value
        }
    }

    throw "Unable to derive PerformanceSystem schema version from runtime source or changelog."
}

function Test-RequiredPath {
    param(
        [string]$RelativePath,
        [string]$Kind = "Path"
    )

    $path = Join-Path $ProjectRoot $RelativePath
    if (Test-Path -LiteralPath $path) {
        Add-Check -Name "$Kind exists: $RelativePath" -Status "PASS" -Evidence $RelativePath
    } else {
        Add-Check -Name "$Kind exists: $RelativePath" -Status "FAIL" -Evidence $RelativePath -Details "Required project health path is missing."
    }
}

function Test-FileContains {
    param(
        [string]$RelativePath,
        [string]$RequiredText,
        [string]$CheckName
    )

    $path = Join-Path $ProjectRoot $RelativePath
    if (-not (Test-Path -LiteralPath $path)) {
        Add-Check -Name $CheckName -Status "FAIL" -Evidence $RelativePath -Details "File does not exist."
        return
    }

    $text = Get-Content -LiteralPath $path -Raw
    if ($text -match [regex]::Escape($RequiredText)) {
        Add-Check -Name $CheckName -Status "PASS" -Evidence $RelativePath -Details $RequiredText
    } else {
        Add-Check -Name $CheckName -Status "FAIL" -Evidence $RelativePath -Details "Missing required reference: $RequiredText"
    }
}

function Invoke-StaticChecks {
    param([int]$SchemaVersion)

    $requiredPaths = @(
        "T66.uproject",
        "AGENTS.md",
        "OPTIONAL_VALIDATOR_PROTOCOL.md",
        "Config\DefaultEngine.ini",
        "Config\DefaultGame.ini",
        "Config\DefaultInput.ini",
        "Scripts\StageStandaloneBuild.ps1",
        "Scripts\RunStagedBuildReadinessGate.ps1",
        "Scripts\RunPreReleaseSmokeSuite.ps1",
        "Scripts\RunFrontendTagClickSmokeMatrix.ps1",
        "Scripts\RunDurableSaveIntegritySmokeGate.ps1",
        "Scripts\RunLifecycleTransitionSmokeGate.ps1",
        "Scripts\RunRuntimeHealthGate.ps1",
        "PerformanceSystem\PERFORMANCE_SYSTEM_AGENTS.md",
        "PerformanceSystem\README.md",
        "PerformanceSystem\RUNTIME_HEALTH_GATE.md",
        "PerformanceSystem\RUNTIME_OWNERSHIP_INVENTORY.md",
        "PerformanceSystem\schema\SCHEMA_CHANGELOG.md",
        "PerformanceSystem\schema\performance_event.schema.v$SchemaVersion.json",
        "PerformanceSystem\schema\performance_session_report.schema.v$SchemaVersion.json",
        "PerformanceSystem\schema\board_saturation_frame_sample.schema.v$SchemaVersion.json"
    )

    foreach ($relativePath in $requiredPaths) {
        Test-RequiredPath -RelativePath $relativePath -Kind "Required file"
    }

    $dataPairs = @(
        "Heroes",
        "Enemies",
        "Weapons",
        "Idols",
        "Stages",
        "CharacterVisuals",
        "MobVertexAnimations",
        "NPCs"
    )

    foreach ($name in $dataPairs) {
        Test-RequiredPath -RelativePath "Content\Data\$name.csv" -Kind "Source data"
        Test-RequiredPath -RelativePath "Content\Data\DT_$name.uasset" -Kind "Cooked data asset"
    }

    $readmePath = Join-Path $ProjectRoot "PerformanceSystem\README.md"
    if (Test-Path -LiteralPath $readmePath) {
        $readme = Get-Content -LiteralPath $readmePath -Raw
        $expectedReadmeLine = "Current runtime schema version is ``$SchemaVersion``"
        if ($readme -match [regex]::Escape($expectedReadmeLine)) {
            Add-Check -Name "PerformanceSystem README schema matches runtime" -Status "PASS" -Evidence (Get-RepoRelativePath $readmePath) -Details "SchemaVersion $SchemaVersion"
        } else {
            Add-Check -Name "PerformanceSystem README schema matches runtime" -Status "FAIL" -Evidence (Get-RepoRelativePath $readmePath) -Details "Expected the README to state current schema version `$SchemaVersion`."
        }
    }

    $ownershipInventory = "PerformanceSystem/RUNTIME_OWNERSHIP_INVENTORY.md"
    Test-FileContains -RelativePath "AGENTS.md" -RequiredText "PerformanceSystem\RUNTIME_OWNERSHIP_INVENTORY.md" -CheckName "Root router links runtime ownership inventory"
    Test-FileContains -RelativePath "PerformanceSystem\README.md" -RequiredText $ownershipInventory -CheckName "PerformanceSystem README links runtime ownership inventory"
    Test-FileContains -RelativePath "PerformanceSystem\PERFORMANCE_SYSTEM_AGENTS.md" -RequiredText $ownershipInventory -CheckName "PerformanceSystem agents links runtime ownership inventory"
    Test-FileContains -RelativePath "PerformanceSystem\RUNTIME_HEALTH_GATE.md" -RequiredText $ownershipInventory -CheckName "Runtime health gate doc links runtime ownership inventory"
    Test-FileContains -RelativePath "LifecycleSystem\README.md" -RequiredText $ownershipInventory -CheckName "Lifecycle README links runtime ownership inventory"
    Test-FileContains -RelativePath "LifecycleSystem\LIFECYCLE_SYSTEM_AGENTS.md" -RequiredText $ownershipInventory -CheckName "Lifecycle agents links runtime ownership inventory"
    Test-FileContains -RelativePath "LifecycleSystem\LIFECYCLE_COORDINATOR_REGISTRY.md" -RequiredText $ownershipInventory -CheckName "Lifecycle registry links runtime ownership inventory"
    Test-FileContains -RelativePath "ShutdownSystem\README.md" -RequiredText $ownershipInventory -CheckName "Shutdown README links runtime ownership inventory"
    Test-FileContains -RelativePath "Scripts\README.md" -RequiredText $ownershipInventory -CheckName "Scripts README links runtime ownership inventory"
}

function Invoke-StagedReadiness {
    $readinessScript = Join-Path $PSScriptRoot "RunStagedBuildReadinessGate.ps1"
    $readinessRoot = Join-Path $OutputRoot "staged_readiness"
    $stdoutPath = Join-Path $OutputRoot "staged_readiness_stdout.log"
    $stderrPath = Join-Path $OutputRoot "staged_readiness_stderr.log"

    if (-not (Test-Path -LiteralPath $readinessScript)) {
        Add-Check -Name "Staged readiness gate available" -Status "FAIL" -Evidence (Get-RepoRelativePath $readinessScript)
        return $null
    }

    $args = @(
        "-NoProfile",
        "-ExecutionPolicy", "Bypass",
        "-File", $readinessScript,
        "-SkipStage",
        "-SkipSmoke",
        "-OutputRoot", $readinessRoot
    )
    if ($PrintOnly) {
        $args += "-PrintOnly"
    }

    $process = Start-Process -FilePath "powershell" -ArgumentList (Convert-ArgListToCommandLine $args) -RedirectStandardOutput $stdoutPath -RedirectStandardError $stderrPath -PassThru -WindowStyle Hidden
    $exited = $process.WaitForExit($StagedReadinessTimeoutSeconds * 1000)
    if (-not $exited) {
        try {
            Stop-Process -Id $process.Id -Force -ErrorAction SilentlyContinue
        } catch {
        }
        Add-Check -Name "Staged readiness gate completed" -Status "FAIL" -Evidence $readinessRoot -Details "Timed out after $StagedReadinessTimeoutSeconds seconds."
        return $null
    }
    $process.WaitForExit()
    $process.Refresh()

    $summaryPath = Join-Path $readinessRoot "summary.json"
    $summary = Get-JsonOrNull -Path $summaryPath
    if ($null -eq $summary) {
        Add-Check -Name "Staged readiness summary produced" -Status "FAIL" -Evidence $summaryPath -Details "ExitCode=$($process.ExitCode)"
        return $null
    }

    $expectedStatus = if ($PrintOnly) { "PRINT_ONLY" } else { "PASS" }
    $exitCodeText = if ($null -eq $process.ExitCode) { "Unavailable" } else { "$($process.ExitCode)" }
    $exitCodeOk = ($null -eq $process.ExitCode) -or ($process.ExitCode -eq 0)
    if ($exitCodeOk -and ($summary.Status -eq $expectedStatus)) {
        Add-Check -Name "Staged readiness cheap gate" -Status "PASS" -Evidence $summaryPath -Details "Status=$($summary.Status); ExitCode=$exitCodeText"
    } else {
        Add-Check -Name "Staged readiness cheap gate" -Status "FAIL" -Evidence $summaryPath -Details "Expected $expectedStatus, got Status=$($summary.Status); ExitCode=$exitCodeText"
    }

    return $summary
}

function Invoke-RuntimeLaunch {
    param(
        [int]$SchemaVersion
    )

    if (-not (Test-Path -LiteralPath $Exe)) {
        Add-Check -Name "Runtime executable exists" -Status "FAIL" -Evidence $Exe
        return $null
    }

    $stagedGameRoot = Resolve-StagedGameRoot -ExecutablePath $Exe
    $performanceRoot = Join-Path $stagedGameRoot "Saved\PerformanceSystem"
    $runtimeDir = Join-Path $OutputRoot "runtime_launch"
    New-Item -ItemType Directory -Force -Path $runtimeDir | Out-Null

    $absLogPath = Join-Path $runtimeDir "runtime.log"
    $stdoutPath = Join-Path $runtimeDir "stdout.log"
    $stderrPath = Join-Path $runtimeDir "stderr.log"
    $screenshotPath = Join-Path $runtimeDir "health_mainmenu.png"
    $launchStartUtc = (Get-Date).ToUniversalTime()
    $exeHashBefore = Get-FileHash -Algorithm SHA256 -LiteralPath $Exe

    if ($PrintOnly) {
        $args = @(
            "-windowed",
            "-ResX=1280",
            "-ResY=720",
            "-T66AutomationWindowed",
            "-T66AutomationResX=1280",
            "-T66AutomationResY=720",
            "-T66FrontendScreen=MainMenu",
            "-T66AutoScreenshot=$screenshotPath",
            "-T66AutoScreenshotDelay=$RuntimeHoldSeconds",
            "-abslog=$absLogPath",
            "-forcelogflush",
            "-nop4",
            "-nosplash",
            "-unattended"
        )
        Add-Check -Name "Runtime launch command printable" -Status "PRINT_ONLY" -Evidence "$Exe $(Convert-ArgListToCommandLine $args)"
        return [pscustomobject]@{
            StagedGameRoot = $stagedGameRoot
            PerformanceRoot = $performanceRoot
            LaunchStartUtc = $launchStartUtc.ToString("o")
            RuntimeLog = $absLogPath
            Screenshot = $screenshotPath
            ExitCode = $null
            ExeHashBefore = $exeHashBefore.Hash
            ExeHashAfter = $null
        }
    }

    $args = @(
        "-windowed",
        "-ResX=1280",
        "-ResY=720",
        "-T66AutomationWindowed",
        "-T66AutomationResX=1280",
        "-T66AutomationResY=720",
        "-T66FrontendScreen=MainMenu",
        "-T66AutoScreenshot=$screenshotPath",
        "-T66AutoScreenshotDelay=$RuntimeHoldSeconds",
        "-abslog=$absLogPath",
        "-forcelogflush",
        "-nop4",
        "-nosplash",
        "-unattended"
    )

    $process = Start-Process -FilePath $Exe -ArgumentList (Convert-ArgListToCommandLine $args) -RedirectStandardOutput $stdoutPath -RedirectStandardError $stderrPath -PassThru
    $exited = $process.WaitForExit($RuntimeTimeoutSeconds * 1000)
    if (-not $exited) {
        try {
            Stop-Process -Id $process.Id -Force -ErrorAction SilentlyContinue
        } catch {
        }
        Add-Check -Name "Runtime launch exits within deadline" -Status "FAIL" -Evidence $absLogPath -Details "Timed out after $RuntimeTimeoutSeconds seconds."
        return [pscustomobject]@{
            StagedGameRoot = $stagedGameRoot
            PerformanceRoot = $performanceRoot
            LaunchStartUtc = $launchStartUtc.ToString("o")
            RuntimeLog = $absLogPath
            Screenshot = $screenshotPath
            ExitCode = $null
            ExeHashBefore = $exeHashBefore.Hash
            ExeHashAfter = (Get-FileHash -Algorithm SHA256 -LiteralPath $Exe).Hash
        }
    }
    $process.WaitForExit()
    $process.Refresh()

    $exeHashAfter = Get-FileHash -Algorithm SHA256 -LiteralPath $Exe
    if ($exeHashBefore.Hash -eq $exeHashAfter.Hash) {
        Add-Check -Name "Executable provenance stable during runtime gate" -Status "PASS" -Evidence $Exe -Details "SHA256=$($exeHashAfter.Hash)"
    } else {
        Add-Check -Name "Executable provenance stable during runtime gate" -Status "FAIL" -Evidence $Exe -Details "Before=$($exeHashBefore.Hash); After=$($exeHashAfter.Hash)"
    }

    $runtimeExitCodeText = if ($null -eq $process.ExitCode) { "Unavailable" } else { "$($process.ExitCode)" }
    $logIndicatesStatusZero = $false
    if (Test-Path -LiteralPath $absLogPath) {
        $logForExitStatus = Get-Content -LiteralPath $absLogPath -Raw
        $logIndicatesStatusZero = ($logForExitStatus -match "RequestExitWithStatus\(0,\s*0")
    }

    if ($process.ExitCode -eq 0) {
        Add-Check -Name "Runtime launch exit status" -Status "PASS" -Evidence $absLogPath -Details "ExitCode=0"
    } elseif (($null -eq $process.ExitCode) -and $logIndicatesStatusZero) {
        Add-Check -Name "Runtime launch exit status" -Status "PASS" -Evidence $absLogPath -Details "ExitCode=Unavailable; runtime log contains RequestExitWithStatus(0, 0)."
    } else {
        Add-Check -Name "Runtime launch exit status" -Status "FAIL" -Evidence $absLogPath -Details "ExitCode=$runtimeExitCodeText"
    }

    if (Test-Path -LiteralPath $screenshotPath) {
        Add-Check -Name "Runtime launch produced Unreal screenshot" -Status "PASS" -Evidence $screenshotPath
    } else {
        Add-Check -Name "Runtime launch produced Unreal screenshot" -Status "FAIL" -Evidence $screenshotPath
    }

    if (Test-Path -LiteralPath $absLogPath) {
        $log = Get-Content -LiteralPath $absLogPath -Raw
        if ($log -match "LogExit: Exiting") {
            Add-Check -Name "Runtime log reached engine exit" -Status "PASS" -Evidence $absLogPath
        } else {
            Add-Check -Name "Runtime log reached engine exit" -Status "FAIL" -Evidence $absLogPath -Details "Missing LogExit: Exiting."
        }

        $forbiddenPatterns = @(
            "Fatal error",
            "Unhandled Exception",
            "LogOutputDevice: Error",
            "Assertion failed",
            "Ensure condition failed",
            "T66FrontendScreenUnknown",
            "T66AutoScreenshot.*failed"
        )

        $matches = @()
        foreach ($pattern in $forbiddenPatterns) {
            if ($log -match $pattern) {
                $matches += $pattern
            }
        }

        if ($matches.Count -eq 0) {
            Add-Check -Name "Runtime log has no fatal proof markers" -Status "PASS" -Evidence $absLogPath
        } else {
            Add-Check -Name "Runtime log has no fatal proof markers" -Status "FAIL" -Evidence $absLogPath -Details ($matches -join "; ")
        }
    } else {
        Add-Check -Name "Runtime log exists" -Status "FAIL" -Evidence $absLogPath
    }

    return [pscustomobject]@{
        StagedGameRoot = $stagedGameRoot
        PerformanceRoot = $performanceRoot
        LaunchStartUtc = $launchStartUtc.ToString("o")
        RuntimeLog = $absLogPath
        Screenshot = $screenshotPath
        ExitCode = $process.ExitCode
        ExeHashBefore = $exeHashBefore.Hash
        ExeHashAfter = $exeHashAfter.Hash
        SchemaVersion = $SchemaVersion
    }
}

function Test-ObjectFields {
    param(
        [object]$Object,
        [string[]]$Fields
    )

    $missing = @()
    foreach ($field in $Fields) {
        if ($null -eq $Object.PSObject.Properties[$field]) {
            $missing += $field
        }
    }

    return $missing
}

function Test-PerformanceArtifacts {
    param(
        [object]$RuntimeResult,
        [int]$SchemaVersion
    )

    if ($null -eq $RuntimeResult) {
        Add-Check -Name "Performance artifact check has runtime context" -Status "FAIL" -Details "Runtime launch result is unavailable."
        return
    }

    $performanceRoot = $RuntimeResult.PerformanceRoot
    $launchStartUtc = [datetime]::Parse($RuntimeResult.LaunchStartUtc).ToUniversalTime()
    $snapshotPath = Join-Path $performanceRoot "snapshot.current.json"
    $sessionsRoot = Join-Path $performanceRoot "Sessions"

    if (-not (Test-Path -LiteralPath $performanceRoot)) {
        Add-Check -Name "PerformanceSystem output root exists" -Status "FAIL" -Evidence $performanceRoot
        return
    }

    Add-Check -Name "PerformanceSystem output root exists" -Status "PASS" -Evidence $performanceRoot

    $snapshot = Get-JsonOrNull -Path $snapshotPath
    if ($null -eq $snapshot) {
        Add-Check -Name "PerformanceSystem snapshot is parseable" -Status "FAIL" -Evidence $snapshotPath
    } else {
        if ([int]$snapshot.SchemaVersion -eq $SchemaVersion) {
            Add-Check -Name "PerformanceSystem snapshot schema" -Status "PASS" -Evidence $snapshotPath -Details "SchemaVersion=$($snapshot.SchemaVersion)"
        } else {
            Add-Check -Name "PerformanceSystem snapshot schema" -Status "FAIL" -Evidence $snapshotPath -Details "Expected $SchemaVersion, got $($snapshot.SchemaVersion)"
        }

        $snapshotInfo = Get-Item -LiteralPath $snapshotPath
        if ($snapshotInfo.LastWriteTimeUtc -ge $launchStartUtc.AddSeconds(-2)) {
            Add-Check -Name "PerformanceSystem snapshot is fresh" -Status "PASS" -Evidence $snapshotPath -Details "LastWriteUtc=$($snapshotInfo.LastWriteTimeUtc.ToString("o"))"
        } else {
            Add-Check -Name "PerformanceSystem snapshot is fresh" -Status "FAIL" -Evidence $snapshotPath -Details "LastWriteUtc=$($snapshotInfo.LastWriteTimeUtc.ToString("o")); LaunchStartUtc=$($launchStartUtc.ToString("o"))"
        }
    }

    if (-not (Test-Path -LiteralPath $sessionsRoot)) {
        Add-Check -Name "PerformanceSystem sessions root exists" -Status "FAIL" -Evidence $sessionsRoot
        return
    }

    $freshSummaries = Get-ChildItem -LiteralPath $sessionsRoot -Directory |
        ForEach-Object {
            $summaryPath = Join-Path $_.FullName "session_summary.json"
            if (Test-Path -LiteralPath $summaryPath) {
                Get-Item -LiteralPath $summaryPath
            }
        } |
        Where-Object { $_.LastWriteTimeUtc -ge $launchStartUtc.AddSeconds(-2) } |
        Sort-Object LastWriteTimeUtc -Descending

    if (-not $freshSummaries -or $freshSummaries.Count -eq 0) {
        Add-Check -Name "PerformanceSystem fresh session summary exists" -Status "FAIL" -Evidence $sessionsRoot -Details "No session_summary.json written after runtime launch."
        return
    }

    $summaryItem = $freshSummaries[0]
    $summary = Get-JsonOrNull -Path $summaryItem.FullName
    if ($null -eq $summary) {
        Add-Check -Name "PerformanceSystem fresh session summary is parseable" -Status "FAIL" -Evidence $summaryItem.FullName
        return
    }

    Add-Check -Name "PerformanceSystem fresh session summary exists" -Status "PASS" -Evidence $summaryItem.FullName -Details "LastWriteUtc=$($summaryItem.LastWriteTimeUtc.ToString("o"))"

    if ([int]$summary.SchemaVersion -eq $SchemaVersion) {
        Add-Check -Name "PerformanceSystem session schema" -Status "PASS" -Evidence $summaryItem.FullName -Details "SchemaVersion=$($summary.SchemaVersion)"
    } else {
        Add-Check -Name "PerformanceSystem session schema" -Status "FAIL" -Evidence $summaryItem.FullName -Details "Expected $SchemaVersion, got $($summary.SchemaVersion)"
    }

    $requiredFields = @(
        "SchemaVersion",
        "SessionId",
        "StartedUtc",
        "EndedUtc",
        "ExitReason",
        "Build",
        "EventCounts",
        "FrameSummary",
        "MemorySummary",
        "DetectorRuntime",
        "PerformanceWriteQueue"
    )
    $missingFields = Test-ObjectFields -Object $summary -Fields $requiredFields
    if ($missingFields.Count -eq 0) {
        Add-Check -Name "PerformanceSystem session summary required fields" -Status "PASS" -Evidence $summaryItem.FullName
    } else {
        Add-Check -Name "PerformanceSystem session summary required fields" -Status "FAIL" -Evidence $summaryItem.FullName -Details ($missingFields -join ", ")
    }

    $queue = $summary.PerformanceWriteQueue
    $queueMissing = Test-ObjectFields -Object $queue -Fields @("AttemptedWrites", "CompletedWrites", "FailedWrites", "AbandonedWrites", "FallbackWrites", "CurrentQueueDepth")
    if ($queueMissing.Count -gt 0) {
        Add-Check -Name "PerformanceSystem write queue fields" -Status "FAIL" -Evidence $summaryItem.FullName -Details ($queueMissing -join ", ")
    } else {
        $failed = [int]$queue.FailedWrites
        $abandoned = [int]$queue.AbandonedWrites
        if (($failed -eq 0) -and ($abandoned -eq 0)) {
            Add-Check -Name "PerformanceSystem write queue clean" -Status "PASS" -Evidence $summaryItem.FullName -Details "FailedWrites=$failed; AbandonedWrites=$abandoned; CurrentQueueDepth=$($queue.CurrentQueueDepth)"
        } else {
            Add-Check -Name "PerformanceSystem write queue clean" -Status "FAIL" -Evidence $summaryItem.FullName -Details "FailedWrites=$failed; AbandonedWrites=$abandoned; CurrentQueueDepth=$($queue.CurrentQueueDepth)"
        }
    }

    $eventPath = Join-Path (Split-Path -Parent $summaryItem.FullName) "events.jsonl"
    if (Test-Path -LiteralPath $eventPath) {
        Add-Check -Name "PerformanceSystem events jsonl exists" -Status "PASS" -Evidence $eventPath
    } else {
        Add-Check -Name "PerformanceSystem events jsonl exists" -Status "FAIL" -Evidence $eventPath
    }

    $markdownPath = Join-Path (Split-Path -Parent $summaryItem.FullName) "session_summary.md"
    if (Test-Path -LiteralPath $markdownPath) {
        Add-Check -Name "PerformanceSystem markdown summary exists" -Status "PASS" -Evidence $markdownPath
    } else {
        Add-Check -Name "PerformanceSystem markdown summary exists" -Status "FAIL" -Evidence $markdownPath
    }
}

function Write-GateSummary {
    param(
        [int]$SchemaVersion,
        [object]$RuntimeResult
    )

    $status = "PASS"
    if ($Checks | Where-Object { $_.Status -eq "FAIL" }) {
        $status = "FAIL"
    } elseif ($PrintOnly) {
        $status = "PRINT_ONLY"
    } elseif ($Checks | Where-Object { $_.Status -eq "WARN" }) {
        $status = "WARN"
    }

    $summary = [pscustomobject]@{
        Status = $status
        CreatedAtUtc = $CreatedAtUtc
        ProjectRoot = $ProjectRoot
        Exe = $Exe
        OutputRoot = $OutputRoot
        ExpectedPerformanceSchemaVersion = $SchemaVersion
        RuntimeHoldSeconds = $RuntimeHoldSeconds
        RuntimeTimeoutSeconds = $RuntimeTimeoutSeconds
        RuntimeResult = $RuntimeResult
        Checks = @($Checks.ToArray())
    }

    $summary | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $SummaryJsonPath -Encoding UTF8

    $lines = New-Object System.Collections.Generic.List[string]
    $lines.Add("# Runtime Health Gate Summary") | Out-Null
    $lines.Add("") | Out-Null
    $lines.Add("- Status: $status") | Out-Null
    $lines.Add("- CreatedAtUtc: $CreatedAtUtc") | Out-Null
    $lines.Add("- Exe: $Exe") | Out-Null
    $lines.Add("- OutputRoot: $OutputRoot") | Out-Null
    $lines.Add("- ExpectedPerformanceSchemaVersion: $SchemaVersion") | Out-Null
    $lines.Add("") | Out-Null
    $lines.Add("## Checks") | Out-Null
    $lines.Add("") | Out-Null
    $lines.Add("| Status | Check | Evidence | Details |") | Out-Null
    $lines.Add("|---|---|---|---|") | Out-Null
    foreach ($check in $Checks) {
        $name = ($check.Name -replace "\|", "\|")
        $evidence = (($check.Evidence | Out-String).Trim() -replace "\|", "\|")
        $details = (($check.Details | Out-String).Trim() -replace "\|", "\|")
        $lines.Add("| $($check.Status) | $name | $evidence | $details |") | Out-Null
    }

    $lines | Set-Content -LiteralPath $SummaryMdPath -Encoding UTF8

    return $summary
}

New-Item -ItemType Directory -Force -Path $OutputRoot | Out-Null

$schemaVersion = Get-PerformanceSchemaVersion

if ($SkipStaticChecks) {
    Add-Check -Name "Static contract checks" -Status "SKIPPED" -Details "Skipped by caller."
} else {
    Invoke-StaticChecks -SchemaVersion $schemaVersion
}

if ($SkipStagedReadiness) {
    Add-Check -Name "Staged readiness cheap gate" -Status "SKIPPED" -Details "Skipped by caller."
} else {
    Invoke-StagedReadiness | Out-Null
}

$runtimeResult = $null
if ($SkipRuntimeLaunch) {
    Add-Check -Name "Runtime launch" -Status "SKIPPED" -Details "Skipped by caller."
} else {
    $runtimeResult = Invoke-RuntimeLaunch -SchemaVersion $schemaVersion
}

if ($SkipPerformanceArtifactChecks) {
    Add-Check -Name "PerformanceSystem artifact checks" -Status "SKIPPED" -Details "Skipped by caller."
} elseif ($PrintOnly) {
    Add-Check -Name "PerformanceSystem artifact checks" -Status "PRINT_ONLY" -Details "Runtime artifact validation is execution-only."
} elseif ($SkipRuntimeLaunch) {
    Add-Check -Name "PerformanceSystem artifact checks" -Status "SKIPPED" -Details "Runtime launch was skipped."
} else {
    Test-PerformanceArtifacts -RuntimeResult $runtimeResult -SchemaVersion $schemaVersion
}

$summary = Write-GateSummary -SchemaVersion $schemaVersion -RuntimeResult $runtimeResult
Write-Host "Runtime health gate status: $($summary.Status)"
Write-Host "Summary: $SummaryJsonPath"

if ($summary.Status -eq "FAIL") {
    exit 1
}

exit 0
