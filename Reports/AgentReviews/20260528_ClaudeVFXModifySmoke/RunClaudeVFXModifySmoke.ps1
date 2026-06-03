param(
    [int]$TimeoutSeconds = 180
)

$ErrorActionPreference = "Stop"

$OutputDir = "C:\UE\T66\Reports\AgentReviews\20260528_ClaudeVFXModifySmoke"
$EditorExe = "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
$Project = "C:\UE\T66\T66.uproject"
$PythonScript = Join-Path $OutputDir "ClaudeVFXModifySmoke.py"
$ReportPath = Join-Path $OutputDir "vfx_modify_smoke_report.json"
$ModifyStdout = Join-Path $OutputDir "vfx_modify_unreal_modify_stdout.log"
$ModifyStderr = Join-Path $OutputDir "vfx_modify_unreal_modify_stderr.log"
$VerifyStdout = Join-Path $OutputDir "vfx_modify_unreal_verify_stdout.log"
$VerifyStderr = Join-Path $OutputDir "vfx_modify_unreal_verify_stderr.log"

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
    }
    $report | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $ReportPath -Encoding UTF8
}

function Invoke-UnrealSmokeMode {
    param(
        [Parameter(Mandatory = $true)][string]$Mode,
        [Parameter(Mandatory = $true)][string]$StdoutPath,
        [Parameter(Mandatory = $true)][string]$StderrPath
    )

    $arguments = "`"$Project`" -run=pythonscript -script=`"$PythonScript`" -T66ClaudeVFXModifyMode=$Mode -unattended -nop4 -nosplash"
    Write-Host "[ClaudeVFXModifySmoke] Launching ${Mode}: `"$EditorExe`" $arguments"

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
        Write-FailureReport -FailureKind "Unreal${Mode}TimedOut" -Message "$Mode timed out after $TimeoutSeconds seconds."
        throw "Unreal${Mode}TimedOut after $TimeoutSeconds seconds"
    }

    Set-Content -LiteralPath $StdoutPath -Value $stdoutTask.Result -Encoding UTF8
    Set-Content -LiteralPath $StderrPath -Value $stderrTask.Result -Encoding UTF8

    if ($process.ExitCode -ne 0) {
        Write-FailureReport -FailureKind "Unreal${Mode}ExitNonZero" -Message "$Mode exited with code $($process.ExitCode)."
        throw "Unreal${Mode}ExitNonZero: $($process.ExitCode). See $StdoutPath and $StderrPath"
    }
}

New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null
Remove-Item -LiteralPath $ReportPath,$ModifyStdout,$ModifyStderr,$VerifyStdout,$VerifyStderr -Force -ErrorAction SilentlyContinue

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

Invoke-UnrealSmokeMode -Mode "modify" -StdoutPath $ModifyStdout -StderrPath $ModifyStderr
Invoke-UnrealSmokeMode -Mode "verify" -StdoutPath $VerifyStdout -StderrPath $VerifyStderr

if (-not (Test-Path -LiteralPath $ReportPath -PathType Leaf)) {
    Write-FailureReport -FailureKind "MissingReport" -Message "Smoke did not write $ReportPath."
    throw "MissingReport: $ReportPath"
}

try {
    $report = Get-Content -LiteralPath $ReportPath -Raw | ConvertFrom-Json -ErrorAction Stop
} catch {
    Write-FailureReport -FailureKind "InvalidReport" -Message "Report exists but is invalid JSON: $($_.Exception.Message)"
    throw "InvalidReport: $($_.Exception.Message)"
}

if (-not $report.success -or -not $report.save_success -or -not $report.verify_process_success -or -not $report.reload_verified) {
    throw "SmokeAssertionsFailed: report did not satisfy success/save/verify/reload gates. See $ReportPath"
}
if ([string]::IsNullOrWhiteSpace([string]$report.modified_property.name)) {
    throw "SmokeAssertionsFailed: modified_property.name was empty. See $ReportPath"
}
if (-not $report.file_metadata.after_save.exists -or [string]::IsNullOrWhiteSpace([string]$report.file_metadata.after_save.sha256)) {
    throw "SmokeAssertionsFailed: after_save file metadata missing. See $ReportPath"
}
if (-not $report.file_metadata.after_verify.exists -or [string]::IsNullOrWhiteSpace([string]$report.file_metadata.after_verify.sha256)) {
    throw "SmokeAssertionsFailed: after_verify file metadata missing. See $ReportPath"
}

Write-Host "CLAUDE_VFX_MODIFY_SMOKE_SUCCESS"
Write-Host "Report=$ReportPath"
Write-Host "SourcePath=$($report.source_asset.path)"
Write-Host "TargetPath=$($report.target_asset.path)"
Write-Host "TargetClass=$($report.target_asset.class_name)"
Write-Host "ModifiedProperty=$($report.modified_property.name)"
Write-Host "Before=$($report.modified_property.before)"
Write-Host "AfterRequested=$($report.modified_property.after_requested)"
Write-Host "AfterVerify=$($report.modified_property.after_verify)"
Write-Host "AfterSaveSha256=$($report.file_metadata.after_save.sha256)"
Write-Host "AfterVerifySha256=$($report.file_metadata.after_verify.sha256)"
