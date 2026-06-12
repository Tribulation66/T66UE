param(
    [string]$Exe = "C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe",
    [string]$OutputRoot,
    [int]$FrontendTimeoutSeconds = 70,
    [int]$DurableTimeoutSeconds = 120,
    [int]$LifecycleTimeoutSeconds = 180,
    # Slot 7: keep the durable gate off slot 8 (session loaded-travel fixture) and 0-2 (SaveSlots fixture).
    [int]$DurableSlotIndex = 7,
    [int]$LifecycleTravels = 6,
    [int]$LifecycleStressCount = 6,
    [switch]$SkipFrontend,
    [switch]$SkipDurable,
    [switch]$SkipLifecycle,
    [switch]$ContinueOnFailure,
    [switch]$PrintOnly
)

$ErrorActionPreference = "Stop"

$ProjectRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot "..")).TrimEnd('\')

if (-not $OutputRoot) {
    $stamp = Get-Date -Format "yyyyMMdd_HHmmss"
    $OutputRoot = Join-Path $ProjectRoot "Saved\PreReleaseSmokeSuite\$stamp"
}
$OutputRoot = [System.IO.Path]::GetFullPath($OutputRoot)

$SummaryJsonPath = Join-Path $OutputRoot "summary.json"
$SummaryMdPath = Join-Path $OutputRoot "summary.md"

function Assert-Condition {
    param(
        [bool]$Condition,
        [string]$Message
    )

    if (-not $Condition) {
        throw $Message
    }
}

function Get-TextOrEmpty {
    param([string]$Path)

    if (Test-Path -LiteralPath $Path) {
        return Get-Content -LiteralPath $Path -Raw
    }

    return ""
}

function Get-BuildConfigHint {
    param(
        [bool]$RequiresNonShipping,
        [string]$Text
    )

    if (-not $RequiresNonShipping) {
        return $null
    }

    foreach ($needle in @("Shipping", "non-shipping", "compiled out", "command did not run", "missing required marker", "No manifest was written", "T66.WorldRuntime.ProofTravel", "T66.Save")) {
        if ($Text.IndexOf($needle, [System.StringComparison]::OrdinalIgnoreCase) -ge 0) {
            return "Requires Development/non-shipping build; child output suggests the command or marker was unavailable."
        }
    }

    return $null
}

function Invoke-SuiteGate {
    param(
        [string]$Name,
        [string]$ScriptPath,
        [string[]]$Arguments,
        [bool]$RequiresNonShipping
    )

    $gateRoot = Join-Path $OutputRoot $Name
    New-Item -ItemType Directory -Force -Path $gateRoot | Out-Null

    $stdoutPath = Join-Path $gateRoot "suite_stdout.log"
    $stderrPath = Join-Path $gateRoot "suite_stderr.log"
    $summaryPath = Join-Path $gateRoot "summary.json"
    foreach ($path in @($stdoutPath, $stderrPath)) {
        if (Test-Path -LiteralPath $path) {
            Remove-Item -LiteralPath $path -Force
        }
    }

    $argList = @(
        "-NoProfile",
        "-ExecutionPolicy",
        "Bypass",
        "-File",
        $ScriptPath,
        "-Exe",
        $Exe,
        "-OutputRoot",
        $gateRoot
    ) + $Arguments

    if ($PrintOnly) {
        $argList += "-PrintOnly"
    }

    $stopwatch = [System.Diagnostics.Stopwatch]::StartNew()
    Write-Host "=== $Name ==="
    Write-Host "powershell $($argList -join ' ')"

    $exitCode = $null
    $launchError = $null
    try {
        $process = Start-Process -FilePath "powershell" -ArgumentList $argList -WorkingDirectory $ProjectRoot -RedirectStandardOutput $stdoutPath -RedirectStandardError $stderrPath -Wait -PassThru
        $exitCode = $process.ExitCode
    } catch {
        $exitCode = 1
        $launchError = $_.Exception.Message
        $launchError | Set-Content -LiteralPath $stderrPath -Encoding UTF8
    }
    $stopwatch.Stop()

    $stdoutText = Get-TextOrEmpty -Path $stdoutPath
    $stderrText = Get-TextOrEmpty -Path $stderrPath
    $combinedText = "$stdoutText`n$stderrText`n$launchError"

    $parsedStatus = $null
    $summaryError = $null
    if (Test-Path -LiteralPath $summaryPath) {
        try {
            $parsed = Get-Content -LiteralPath $summaryPath -Raw | ConvertFrom-Json
            $parsedStatus = [string]$parsed.Status
        } catch {
            $summaryError = $_.Exception.Message
        }
    }

    $buildConfigHint = $null
    if ($exitCode -ne 0) {
        $buildConfigHint = Get-BuildConfigHint -RequiresNonShipping $RequiresNonShipping -Text $combinedText
    }
    $status = "FAIL"
    if ($exitCode -eq 0 -and $parsedStatus -eq "PASS") {
        $status = "PASS"
    } elseif ($exitCode -eq 0 -and $parsedStatus -eq "PRINT_ONLY") {
        $status = "PRINT_ONLY"
    } elseif ($exitCode -ne 0 -and $buildConfigHint) {
        $status = "BUILD_CONFIG_UNSUPPORTED"
    }

    if ($exitCode -eq 0 -and -not $parsedStatus) {
        $summaryError = if ($summaryError) { $summaryError } else { "Child exited 0 but did not produce a readable summary.json." }
        $status = "FAIL"
    }

    $errorCandidates = @($launchError, $summaryError, $stderrText) | Where-Object { -not [string]::IsNullOrWhiteSpace([string]$_) }
    $errorText = if ($errorCandidates.Count -gt 0) { [string]$errorCandidates[0] } else { $null }

    [pscustomobject]@{
        Name = $Name
        Status = $status
        ExitCode = $exitCode
        DurationSeconds = [Math]::Round($stopwatch.Elapsed.TotalSeconds, 3)
        Script = $ScriptPath
        OutputRoot = $gateRoot
        Summary = $summaryPath
        Stdout = $stdoutPath
        Stderr = $stderrPath
        ParsedChildStatus = $parsedStatus
        RequiresNonShipping = $RequiresNonShipping
        BuildConfigHint = $buildConfigHint
        Error = $errorText
    }
}

function Write-SuiteSummary {
    param(
        [string]$Status,
        [string]$ErrorMessage,
        [object[]]$GateResults
    )

    $summary = [pscustomobject]@{
        Status = $Status
        CreatedAt = (Get-Date).ToString("o")
        Exe = [System.IO.Path]::GetFullPath($Exe)
        OutputRoot = $OutputRoot
        PrintOnly = [bool]$PrintOnly
        ContinueOnFailure = [bool]$ContinueOnFailure
        Skipped = [pscustomobject]@{
            Frontend = [bool]$SkipFrontend
            Durable = [bool]$SkipDurable
            Lifecycle = [bool]$SkipLifecycle
        }
        Error = $ErrorMessage
        Gates = $GateResults
    }

    $summary | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $SummaryJsonPath -Encoding UTF8

    $md = @()
    $md += "# Pre-Release Smoke Suite"
    $md += ""
    $md += "Status: $Status"
    $md += "Executable: $($summary.Exe)"
    $md += "Output root: $OutputRoot"
    if ($ErrorMessage) {
        $md += "Error: $ErrorMessage"
    }
    $md += ""
    foreach ($gate in $GateResults) {
        $md += "## $($gate.Name)"
        $md += ""
        $md += "- Status: $($gate.Status)"
        $md += "- Exit code: $($gate.ExitCode)"
        $md += "- Duration seconds: $($gate.DurationSeconds)"
        $md += "- Summary: $($gate.Summary)"
        $md += "- Stdout: $($gate.Stdout)"
        $md += "- Stderr: $($gate.Stderr)"
        if ($gate.BuildConfigHint) {
            $md += "- Build config hint: $($gate.BuildConfigHint)"
        }
        if ($gate.Error) {
            $md += "- Error: $($gate.Error)"
        }
        $md += ""
    }
    $md += "This suite is an orchestration layer over the existing frontend, durable save, and lifecycle gates. Individual child summaries remain the source of detailed gate evidence."
    $md -join "`r`n" | Set-Content -LiteralPath $SummaryMdPath -Encoding UTF8

    Write-Host "Pre-release smoke suite $Status`: $OutputRoot"
    Write-Host "Summary: $SummaryJsonPath"
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
if ($SkipFrontend -and $SkipDurable -and $SkipLifecycle) {
    throw "At least one gate must run. Do not combine all three skip switches."
}
if (-not (Test-Path -LiteralPath $Exe)) {
    throw "Missing executable: $Exe"
}

$frontendScript = Join-Path $PSScriptRoot "RunFrontendTagClickSmokeMatrix.ps1"
$durableScript = Join-Path $PSScriptRoot "RunDurableSaveIntegritySmokeGate.ps1"
$lifecycleScript = Join-Path $PSScriptRoot "RunLifecycleTransitionSmokeGate.ps1"
foreach ($script in @($frontendScript, $durableScript, $lifecycleScript)) {
    if (-not (Test-Path -LiteralPath $script)) {
        throw "Missing child gate script: $script"
    }
}

New-Item -ItemType Directory -Force -Path $OutputRoot | Out-Null
foreach ($path in @($SummaryJsonPath, $SummaryMdPath)) {
    if (Test-Path -LiteralPath $path) {
        Remove-Item -LiteralPath $path -Force
    }
}

$results = New-Object System.Collections.Generic.List[object]
$suiteStatus = if ($PrintOnly) { "PRINT_ONLY" } else { "PASS" }
$suiteError = $null

$gatePlan = @(
    [pscustomobject]@{
        Name = "01_FrontendTagClick"
        Skip = [bool]$SkipFrontend
        Script = $frontendScript
        RequiresNonShipping = $false
        Arguments = @("-TimeoutSeconds", "$FrontendTimeoutSeconds")
    },
    [pscustomobject]@{
        Name = "02_DurableSaveIntegrity"
        Skip = [bool]$SkipDurable
        Script = $durableScript
        RequiresNonShipping = $true
        Arguments = @("-TimeoutSeconds", "$DurableTimeoutSeconds", "-SlotIndex", "$DurableSlotIndex")
    },
    [pscustomobject]@{
        Name = "03_LifecycleTransition"
        Skip = [bool]$SkipLifecycle
        Script = $lifecycleScript
        RequiresNonShipping = $true
        Arguments = @("-TimeoutSeconds", "$LifecycleTimeoutSeconds", "-Travels", "$LifecycleTravels", "-StressCount", "$LifecycleStressCount")
    }
)

$gateIndex = -1
foreach ($gate in $gatePlan) {
    $gateIndex += 1
    if ($gate.Skip) {
        $skipRoot = Join-Path $OutputRoot $gate.Name
        $results.Add([pscustomobject]@{
            Name = $gate.Name
            Status = "SKIPPED"
            ExitCode = $null
            DurationSeconds = 0
            Script = $gate.Script
            OutputRoot = $skipRoot
            Summary = $null
            Stdout = $null
            Stderr = $null
            ParsedChildStatus = $null
            RequiresNonShipping = [bool]$gate.RequiresNonShipping
            BuildConfigHint = $null
            Error = "Skipped by switch."
        })
        continue
    }

    $result = Invoke-SuiteGate -Name $gate.Name -ScriptPath $gate.Script -Arguments $gate.Arguments -RequiresNonShipping ([bool]$gate.RequiresNonShipping)
    $results.Add($result)

    if ($result.Status -notin @("PASS", "PRINT_ONLY")) {
        $suiteStatus = "FAIL"
        $suiteError = "$($result.Name) returned $($result.Status)."
        if (-not $ContinueOnFailure) {
            foreach ($remaining in $gatePlan | Select-Object -Skip ($gateIndex + 1)) {
                if (-not $remaining.Skip) {
                    $remainingRoot = Join-Path $OutputRoot $remaining.Name
                    $results.Add([pscustomobject]@{
                        Name = $remaining.Name
                        Status = "NOT_RUN"
                        ExitCode = $null
                        DurationSeconds = 0
                        Script = $remaining.Script
                        OutputRoot = $remainingRoot
                        Summary = $null
                        Stdout = $null
                        Stderr = $null
                        ParsedChildStatus = $null
                        RequiresNonShipping = [bool]$remaining.RequiresNonShipping
                        BuildConfigHint = $null
                        Error = "Not run after earlier suite failure. Use -ContinueOnFailure to collect all gates."
                    })
                }
            }
            break
        }
    }
}

if ($results | Where-Object { $_.Status -eq "FAIL" -or $_.Status -eq "BUILD_CONFIG_UNSUPPORTED" }) {
    $suiteStatus = "FAIL"
    if (-not $suiteError) {
        $firstFailure = $results | Where-Object { $_.Status -eq "FAIL" -or $_.Status -eq "BUILD_CONFIG_UNSUPPORTED" } | Select-Object -First 1
        $suiteError = "$($firstFailure.Name) returned $($firstFailure.Status)."
    }
}

Write-SuiteSummary -Status $suiteStatus -ErrorMessage $suiteError -GateResults $results.ToArray()

if ($suiteStatus -eq "FAIL") {
    throw $suiteError
}
