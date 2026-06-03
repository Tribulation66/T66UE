param(
    [string]$Project = "C:\UE\T66\T66.uproject",
    [string]$EditorExe = "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe",
    [string]$Map = "/Game/Maps/GameplayLevel",
    [string]$EnemyID = "BoneWalker",
    [string]$Output,
    [string]$FrameDir,
    [int]$ResX = 1280,
    [int]$ResY = 720,
    [int]$FrameCount = 96,
    [int]$FrameRate = 15,
    [double]$CaptureIntervalSeconds = 0.0666667,
    [double]$DelaySeconds = 0.5,
    [double]$PostCaptureDelaySeconds = 4.0,
    [int]$TimeoutSeconds = 180,
    [double]$StartDistance = 1650.0,
    [double]$StopDistance = 140.0,
    [double]$CameraDistance = 1120.0,
    [double]$CameraSideOffset = 420.0,
    [double]$CameraHeight = 255.0,
    [double]$TargetForwardOffset = 0.0,
    [switch]$RemoveFrames,
    [switch]$PrintOnly
)

$ErrorActionPreference = "Stop"

function Get-RepoRoot {
    return [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot ".."))
}

$repoRoot = Get-RepoRoot
$timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
$safeEnemyID = ($EnemyID -replace "[^A-Za-z0-9_.-]", "_")
if (-not $Output) {
    $Output = Join-Path $repoRoot "Saved\VideoCaptures\EnemyAnimationPreview_$timestamp\${safeEnemyID}_enemyanimpreview.mp4"
}
if (-not $FrameDir) {
    $FrameDir = Join-Path (Split-Path -Parent $Output) "frames"
}

$captureScript = Join-Path $PSScriptRoot "CaptureT66GameplayVideo.ps1"
if (-not (Test-Path -LiteralPath $captureScript)) {
    throw "Missing gameplay capture script: $captureScript"
}

$extraArgs = @(
    "-T66EnemyAnimPreviewEnemyID=$EnemyID",
    "-T66EnemyAnimPreviewStartDistance=$StartDistance",
    "-T66EnemyAnimPreviewStopDistance=$StopDistance",
    "-T66EnemyAnimPreviewCameraDistance=$CameraDistance",
    "-T66EnemyAnimPreviewCameraSideOffset=$CameraSideOffset",
    "-T66EnemyAnimPreviewCameraHeight=$CameraHeight",
    "-T66EnemyAnimPreviewTargetForwardOffset=$TargetForwardOffset"
)

$captureParams = @{
    Project = $Project
    EditorExe = $EditorExe
    Map = $Map
    CaptureMode = "enemyanimpreview"
    Output = $Output
    FrameDir = $FrameDir
    ResX = $ResX
    ResY = $ResY
    FrameCount = $FrameCount
    FrameRate = $FrameRate
    CaptureIntervalSeconds = $CaptureIntervalSeconds
    DelaySeconds = $DelaySeconds
    PostCaptureDelaySeconds = $PostCaptureDelaySeconds
    TimeoutSeconds = $TimeoutSeconds
    ExtraArgs = $extraArgs
}

if ($RemoveFrames) {
    $captureParams["RemoveFrames"] = $true
}
if ($PrintOnly) {
    $captureParams["PrintOnly"] = $true
}

& $captureScript @captureParams
