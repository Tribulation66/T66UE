param(
    [Parameter(Mandatory = $true)]
    [string]$RawGlb,

    [Parameter(Mandatory = $true)]
    [string]$AssetName,

    [string]$TargetDir = "/Game/ToonStyle/TestAssets/Lineup",
    [string]$WorkingDir = "",
    [double]$TargetHeight = 180.0,
    [double]$HeightTolerance = 0.10,
    [int]$FlattenK = 6,
    [double]$HighlightCap = 0.85,
    [switch]$IsHumanoid,
    [switch]$RetainedFromPhase1A,
    [string]$ProjectRoot = "C:\UE\T66",
    [string]$BlenderExe = "C:\Program Files\Blender Foundation\Blender 5.1\blender.exe",
    [string]$UnrealEditorExe = "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe"
)

$ErrorActionPreference = "Stop"
if (Get-Variable -Name PSNativeCommandUseErrorActionPreference -Scope Global -ErrorAction SilentlyContinue) {
    $global:PSNativeCommandUseErrorActionPreference = $false
}

function Resolve-FullPath([string]$PathValue) {
    if ([System.IO.Path]::IsPathRooted($PathValue)) {
        return [System.IO.Path]::GetFullPath($PathValue)
    }
    return [System.IO.Path]::GetFullPath((Join-Path $ProjectRoot $PathValue))
}

$ProjectRoot = [System.IO.Path]::GetFullPath($ProjectRoot)
$RawGlbPath = Resolve-FullPath $RawGlb
if (-not (Test-Path -LiteralPath $RawGlbPath)) {
    throw "Raw GLB not found: $RawGlbPath"
}
if (-not (Test-Path -LiteralPath $BlenderExe)) {
    throw "Blender executable not found: $BlenderExe"
}
if (-not (Test-Path -LiteralPath $UnrealEditorExe)) {
    throw "UnrealEditor executable not found: $UnrealEditorExe"
}

if ([string]::IsNullOrWhiteSpace($WorkingDir)) {
    $rawParent = Split-Path -Parent $RawGlbPath
    $assetRoot = Split-Path -Parent $rawParent
    $WorkingDirPath = Join-Path (Join-Path $assetRoot "Working") $AssetName
} else {
    $WorkingDirPath = Resolve-FullPath $WorkingDir
}
New-Item -ItemType Directory -Force -Path $WorkingDirPath | Out-Null

$BlenderScript = Join-Path $ProjectRoot "ToonStyle\BlenderScripts\run_toon_pipeline.py"
$ImportScript = Join-Path $ProjectRoot "ToonStyle\Source\ImportPixal3DAsset.py"
$ProjectFile = Join-Path $ProjectRoot "T66.uproject"

Write-Host "[Pixal3DToUE] Blender extraction/normalization: $AssetName"
$BlenderArgs = @(
    "--background",
    "--python", $BlenderScript,
    "--",
    "--input", $RawGlbPath,
    "--working-dir", $WorkingDirPath,
    "--asset-name", $AssetName,
    "--target-height", $TargetHeight,
    "--flatten-k", $FlattenK,
    "--highlight-cap", $HighlightCap
)
if ($IsHumanoid) {
    $BlenderArgs += "--is-humanoid"
}
if ($RetainedFromPhase1A) {
    $BlenderArgs += "--retained-from-phase1a"
}
& $BlenderExe @BlenderArgs
if ($LASTEXITCODE -ne 0) {
    throw "Blender extraction/normalization failed for $AssetName with exit code $LASTEXITCODE"
}

Write-Host "[Pixal3DToUE] UE import: $AssetName -> $TargetDir"
$oldWorking = $env:T66_PIXAL3D_WORKING_DIR
$oldAsset = $env:T66_PIXAL3D_ASSET_NAME
$oldTarget = $env:T66_PIXAL3D_TARGET_DIR
$oldExpected = $env:T66_PIXAL3D_EXPECTED_HEIGHT
$oldTolerance = $env:T66_PIXAL3D_HEIGHT_TOLERANCE
$oldQuit = $env:T66_PIXAL3D_QUIT_EDITOR
try {
    $env:T66_PIXAL3D_WORKING_DIR = $WorkingDirPath
    $env:T66_PIXAL3D_ASSET_NAME = $AssetName
    $env:T66_PIXAL3D_TARGET_DIR = $TargetDir
    $env:T66_PIXAL3D_EXPECTED_HEIGHT = [string]$TargetHeight
    $env:T66_PIXAL3D_HEIGHT_TOLERANCE = [string]$HeightTolerance
    $env:T66_PIXAL3D_QUIT_EDITOR = "1"

    & $UnrealEditorExe $ProjectFile "-ExecutePythonScript=$ImportScript" -unattended -nop4 -nosplash -NullRHI
    $UEExitCode = $LASTEXITCODE
} finally {
    $env:T66_PIXAL3D_WORKING_DIR = $oldWorking
    $env:T66_PIXAL3D_ASSET_NAME = $oldAsset
    $env:T66_PIXAL3D_TARGET_DIR = $oldTarget
    $env:T66_PIXAL3D_EXPECTED_HEIGHT = $oldExpected
    $env:T66_PIXAL3D_HEIGHT_TOLERANCE = $oldTolerance
    $env:T66_PIXAL3D_QUIT_EDITOR = $oldQuit
}

$VerifyPath = Join-Path $WorkingDirPath "$AssetName`_ue_verify.json"
if (-not (Test-Path -LiteralPath $VerifyPath)) {
    throw "UE import failed for $AssetName with exit code $UEExitCode and no verification JSON: $VerifyPath"
}

$Verify = Get-Content -LiteralPath $VerifyPath -Raw | ConvertFrom-Json
$BoundsHeight = [double]$Verify.static_mesh_bounds.height
$MinHeight = $TargetHeight * (1.0 - $HeightTolerance)
$MaxHeight = $TargetHeight * (1.0 + $HeightTolerance)
if ($BoundsHeight -lt $MinHeight -or $BoundsHeight -gt $MaxHeight) {
    throw "UE import produced invalid bounds for $AssetName. Height=$BoundsHeight expected $TargetHeight +/- $($HeightTolerance * 100)%"
}
if ([string]::IsNullOrWhiteSpace([string]$Verify.static_mesh)) {
    throw "UE verification for $AssetName did not include a static mesh path."
}
if ([string]::IsNullOrWhiteSpace([string]$Verify.outline_static_mesh)) {
    throw "UE verification for $AssetName did not include an outline static mesh path."
}
if ([string]::IsNullOrWhiteSpace([string]$Verify.material_instance)) {
    throw "UE verification for $AssetName did not include a material instance path."
}
if ($null -eq $Verify.textures -or $Verify.textures.Count -lt 1) {
    throw "UE verification for $AssetName did not include any imported textures."
}
if ([string]::IsNullOrWhiteSpace([string]$Verify.slot0_material)) {
    throw "UE verification for $AssetName did not include a slot 0 material assignment."
}
if ($UEExitCode -ne 0) {
    Write-Warning "UnrealEditor exited with code $UEExitCode after writing a valid verification JSON. Treating as verified success; see UE log for the post-save shutdown crash."
}
Write-Host "[Pixal3DToUE] DONE"
Write-Host "  StaticMesh: $($Verify.static_mesh)"
Write-Host "  Material:   $($Verify.material_instance)"
Write-Host "  Textures:   $($Verify.textures -join ', ')"
Write-Host "  Bounds:     $($Verify.static_mesh_bounds.size -join ' x ')"
exit 0
