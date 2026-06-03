param(
    [string]$Project = "C:\UE\T66\T66.uproject",
    [string]$EditorExe = "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe",
    [string]$SystemPath = "/Game/VFXLab/Hero1Axe/AOE/NS_Hero1AxeAOE_MeshSlash",
    [string]$OutputDir,
    [string]$TargetPath = "",
    [int]$ResX = 1600,
    [int]$ResY = 1200,
    [double]$OrthoWidth = 1400.0,
    [double]$CameraHeight = 1200.0,
    [double]$CameraX = 0.0,
    [double]$CameraY = 0.0,
    [double]$CameraPitch = -90.0,
    [double]$CameraYaw = 0.0,
    [double]$CameraRoll = 0.0,
    [double]$ActualTimeSeconds = 0.42,
    [string]$PreviewTimesSeconds = "0.06;0.18;0.30;0.42;0.54;0.66",
    [double]$SeekDeltaSeconds = 0.0166667,
    [int]$WarmupTicks = 4,
    [double]$WarmupTickSeconds = 0.0166667,
    [double]$NonBlackPixelRatioThreshold = 0.002,
    [int]$ArrayProofCount = 0,
    [string]$ArrayProofParameter = "User.TravelerPositions",
    [double]$ArrayProofSpacing = 18.0,
    [int]$TimeoutSeconds = 240,
    [switch]$DebugPrimitive,
    [switch]$PrintOnly
)

$ErrorActionPreference = "Stop"

function Get-RepoRoot {
    return [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot ".."))
}

$repoRoot = Get-RepoRoot
if (-not $OutputDir) {
    $timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
    $OutputDir = Join-Path $repoRoot "Saved\VFXResearch\Hero1Axe\AOE_AmericanFlagVisualTarget\EditorIsolation\$timestamp"
}

$OutputDir = [System.IO.Path]::GetFullPath($OutputDir)
New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null

$argsList = @(
    "`"$Project`"",
    "-run=T66NiagaraIsolationCapture",
    "-AllowCommandletRendering",
    "-unattended",
    "-nop4",
    "-nosplash",
    "-T66NiagaraIsolationSystem=`"$SystemPath`"",
    "-T66NiagaraIsolationOutput=`"$OutputDir`"",
    "-T66NiagaraIsolationResX=$ResX",
    "-T66NiagaraIsolationResY=$ResY",
    "-T66NiagaraIsolationOrthoWidth=$OrthoWidth",
    "-T66NiagaraIsolationCameraHeight=$CameraHeight",
    "-T66NiagaraIsolationCameraX=$CameraX",
    "-T66NiagaraIsolationCameraY=$CameraY",
    "-T66NiagaraIsolationCameraPitch=$CameraPitch",
    "-T66NiagaraIsolationCameraYaw=$CameraYaw",
    "-T66NiagaraIsolationCameraRoll=$CameraRoll",
    "-T66NiagaraIsolationActualTime=$ActualTimeSeconds",
    "-T66NiagaraIsolationPreviewTimes=`"$PreviewTimesSeconds`"",
    "-T66NiagaraIsolationSeekDelta=$SeekDeltaSeconds",
    "-T66NiagaraIsolationWarmupTicks=$WarmupTicks",
    "-T66NiagaraIsolationWarmupTickSeconds=$WarmupTickSeconds",
    "-T66NiagaraIsolationNonBlackThreshold=$NonBlackPixelRatioThreshold",
    "-T66NiagaraIsolationArrayProofCount=$ArrayProofCount",
    "-T66NiagaraIsolationArrayProofParameter=`"$ArrayProofParameter`"",
    "-T66NiagaraIsolationArrayProofSpacing=$ArrayProofSpacing"
)

if ($TargetPath) {
    $argsList += "-T66NiagaraIsolationTarget=`"$TargetPath`""
}
if ($DebugPrimitive) {
    $argsList += "-T66NiagaraIsolationDebugPrimitive"
}

$commandLine = "`"$EditorExe`" " + ($argsList -join " ")
Write-Host "[CaptureT66NiagaraEditorIsolation] $commandLine"
if ($PrintOnly) {
    return
}

$logPath = Join-Path $OutputDir "capture.log"
$process = Start-Process -FilePath $EditorExe -ArgumentList $argsList -NoNewWindow -PassThru -RedirectStandardOutput $logPath -RedirectStandardError (Join-Path $OutputDir "capture.err.log")
if (-not $process.WaitForExit($TimeoutSeconds * 1000)) {
    try {
        $process.Kill()
    } catch {
    }
    throw "Timed out after $TimeoutSeconds seconds. OutputDir=$OutputDir"
}
$process.Refresh()

$exitCode = $process.ExitCode
if ($null -ne $exitCode -and $exitCode -ne 0) {
    throw "Niagara isolation capture failed with exit code $($process.ExitCode). See $logPath and $OutputDir"
}
if ($null -eq $exitCode) {
    Write-Warning "Unreal commandlet process exited but PowerShell did not expose an ExitCode. Continuing to artifact and manifest validation."
}

$required = @("actual.png", "actual_crop.png", "contact_sheet.png", "manifest.json", "mismatch_notes.md")
foreach ($name in $required) {
    $path = Join-Path $OutputDir $name
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
        throw "Missing required artifact: $path"
    }
}

$manifestPath = Join-Path $OutputDir "manifest.json"
$manifest = Get-Content -LiteralPath $manifestPath -Raw | ConvertFrom-Json
if (-not $manifest.render_success) {
    throw "Capture manifest reported render_success=false failure_mode=$($manifest.failure_mode)"
}

Write-Host "[CaptureT66NiagaraEditorIsolation] OutputDir=$OutputDir"
Write-Host "[CaptureT66NiagaraEditorIsolation] actual.png=$(Join-Path $OutputDir 'actual.png')"
Write-Host "[CaptureT66NiagaraEditorIsolation] contact_sheet.png=$(Join-Path $OutputDir 'contact_sheet.png')"
