param(
    [int]$TimeoutSeconds = 180
)

$ErrorActionPreference = "Stop"

$OutputDir = "C:\UE\T66\Reports\AgentReviews\20260528_ClaudeUnrealNiagaraSmoke"
$EditorExe = "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
$Project = "C:\UE\T66\T66.uproject"
$PythonScript = Join-Path $OutputDir "ClaudeUnrealNiagaraSmoke.py"
$ReportPath = Join-Path $OutputDir "unreal_niagara_smoke_report.json"
$StdoutPath = Join-Path $OutputDir "unreal_smoke_stdout.log"
$StderrPath = Join-Path $OutputDir "unreal_smoke_stderr.log"

function Write-FailureReport {
    param(
        [string]$FailureKind,
        [string]$Message
    )

    if (Test-Path -LiteralPath $ReportPath -PathType Leaf) {
        return
    }

    $report = [ordered]@{
        success = $false
        failure_kind = $FailureKind
        message = $Message
        created_utc = (Get-Date).ToUniversalTime().ToString("o")
        unreal_python_live = $false
        engine_version = ""
    }
    $report | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $ReportPath -Encoding UTF8
}

New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null
Remove-Item -LiteralPath $ReportPath,$StdoutPath,$StderrPath -Force -ErrorAction SilentlyContinue

if (-not (Test-Path -LiteralPath $EditorExe -PathType Leaf)) {
    Write-FailureReport -FailureKind "PreflightMissingEditor" -Message "Missing Unreal editor commandlet executable: $EditorExe"
    throw "PreflightMissingEditor: $EditorExe"
}
if (-not (Test-Path -LiteralPath $Project -PathType Leaf)) {
    Write-FailureReport -FailureKind "PreflightMissingProject" -Message "Missing project: $Project"
    throw "PreflightMissingProject: $Project"
}
if (-not (Test-Path -LiteralPath $PythonScript -PathType Leaf)) {
    Write-FailureReport -FailureKind "PreflightMissingScript" -Message "Missing Python smoke script: $PythonScript"
    throw "PreflightMissingScript: $PythonScript"
}

$arguments = "`"$Project`" -run=pythonscript -script=`"$PythonScript`" -unattended -nop4 -nosplash"
Write-Host "[ClaudeUnrealNiagaraSmoke] Launching: `"$EditorExe`" $arguments"

$startInfo = [System.Diagnostics.ProcessStartInfo]::new()
$startInfo.FileName = $EditorExe
$startInfo.Arguments = $arguments
$startInfo.UseShellExecute = $false
$startInfo.RedirectStandardOutput = $true
$startInfo.RedirectStandardError = $true
$startInfo.CreateNoWindow = $true

$process = [System.Diagnostics.Process]::new()
$process.StartInfo = $startInfo

$null = $process.Start()
$stdoutTask = $process.StandardOutput.ReadToEndAsync()
$stderrTask = $process.StandardError.ReadToEndAsync()

if (-not $process.WaitForExit($TimeoutSeconds * 1000)) {
    try {
        $process.Kill($true)
    } catch {
        try {
            $process.Kill()
        } catch {
        }
    }
    $process.WaitForExit()
    Set-Content -LiteralPath $StdoutPath -Value $stdoutTask.Result -Encoding UTF8
    Set-Content -LiteralPath $StderrPath -Value (($stderrTask.Result), "Timed out after $TimeoutSeconds seconds.") -Encoding UTF8
    Write-FailureReport -FailureKind "UnrealTimedOut" -Message "UnrealEditor-Cmd timed out after $TimeoutSeconds seconds."
    throw "UnrealTimedOut after $TimeoutSeconds seconds"
}

Set-Content -LiteralPath $StdoutPath -Value $stdoutTask.Result -Encoding UTF8
Set-Content -LiteralPath $StderrPath -Value $stderrTask.Result -Encoding UTF8

if ($process.ExitCode -ne 0) {
    Write-FailureReport -FailureKind "UnrealExitNonZero" -Message "UnrealEditor-Cmd exited with code $($process.ExitCode)."
    throw "UnrealExitNonZero: $($process.ExitCode). See $StdoutPath and $StderrPath"
}

if (-not (Test-Path -LiteralPath $ReportPath -PathType Leaf)) {
    Write-FailureReport -FailureKind "MissingReport" -Message "Unreal command completed but did not write $ReportPath."
    throw "MissingReport: $ReportPath"
}

try {
    $report = Get-Content -LiteralPath $ReportPath -Raw | ConvertFrom-Json -ErrorAction Stop
} catch {
    Write-FailureReport -FailureKind "InvalidReport" -Message "Report exists but is not valid JSON: $($_.Exception.Message)"
    throw "InvalidReport: $($_.Exception.Message)"
}

$target = $report.target_asset
$hasTargetNiagaraClass = $false
if ($null -ne $target -and $target.loaded -and "$($target.class_name)" -match "Niagara") {
    $hasTargetNiagaraClass = $true
}
$hasApiSignal = [int]$report.niagara_api_symbol_count -gt 0
$hasRegistrySignal = $report.vfxlab_niagara_like_assets_sample.Count -gt 0

if (-not $report.success -or -not $report.unreal_python_live -or [string]::IsNullOrWhiteSpace([string]$report.engine_version) -or -not ($hasTargetNiagaraClass -or $hasApiSignal -or $hasRegistrySignal)) {
    throw "SmokeAssertionsFailed: report did not contain required Unreal/Niagara proof. See $ReportPath"
}

Write-Host "CLAUDE_UNREAL_NIAGARA_SMOKE_SUCCESS"
Write-Host "Report=$ReportPath"
Write-Host "EngineVersion=$($report.engine_version)"
Write-Host "TargetExists=$($target.exists)"
Write-Host "TargetLoaded=$($target.loaded)"
Write-Host "TargetClass=$($target.class_name)"
Write-Host "NiagaraApiSymbolCount=$($report.niagara_api_symbol_count)"
Write-Host "VfxLabNiagaraLikeCount=$($report.vfxlab_niagara_like_assets_sample.Count)"
