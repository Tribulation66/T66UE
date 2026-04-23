param(
    [Parameter(Mandatory = $true)]
    [string]$InputImage,
    [Parameter(Mandatory = $true)]
    [string]$OutputDir,
    [string]$InstallRoot = "",
    [ValidateSet("realesr-animevideov3", "realesrgan-x4plus", "realesrnet-x4plus")]
    [string]$Model = "realesrgan-x4plus",
    [ValidateSet(2, 3, 4)]
    [int]$Scale = 4,
    [switch]$Tta
)

$ErrorActionPreference = "Stop"

if ([string]::IsNullOrWhiteSpace($InstallRoot)) {
    $InstallRoot = Join-Path $PSScriptRoot "..\Tools\Temp\UIUpscaleStack"
}

$InstallRoot = [System.IO.Path]::GetFullPath($InstallRoot)
$ManifestPath = Join-Path $InstallRoot "stack_manifest.json"
if (-not (Test-Path -LiteralPath $ManifestPath)) {
    throw "Missing stack manifest at $ManifestPath. Run SetupFreeUIUpscaleStack.ps1 first."
}

$Manifest = Get-Content -Raw -LiteralPath $ManifestPath | ConvertFrom-Json
$Exe = $Manifest.tools.realesrgan_ncnn_vulkan.exe
if (-not (Test-Path -LiteralPath $Exe)) {
    throw "Missing Real-ESRGAN executable at $Exe."
}

$InputImage = [System.IO.Path]::GetFullPath($InputImage)
$OutputDir = [System.IO.Path]::GetFullPath($OutputDir)

if (-not (Test-Path -LiteralPath $InputImage)) {
    throw "Input image does not exist: $InputImage"
}

if (-not (Test-Path -LiteralPath $OutputDir)) {
    New-Item -ItemType Directory -Path $OutputDir | Out-Null
}

$inputItem = Get-Item -LiteralPath $InputImage
if ($inputItem.PSIsContainer) {
    $outputTarget = $OutputDir
}
else {
    $extension = [System.IO.Path]::GetExtension($InputImage)
    $fileName = [System.IO.Path]::GetFileNameWithoutExtension($InputImage)
    $outputTarget = Join-Path $OutputDir ("{0}_x{1}{2}" -f $fileName, $Scale, $extension)
}

$arguments = @(
    "-i", $InputImage,
    "-o", $outputTarget,
    "-n", $Model,
    "-s", $Scale
)

if ($Tta.IsPresent) {
    $arguments += "-x"
}

Write-Host "Running Real-ESRGAN"
Write-Host "$Exe $($arguments -join ' ')"

Push-Location (Split-Path -Parent $Exe)
try {
    & $Exe @arguments
}
finally {
    Pop-Location
}

if ($LASTEXITCODE -ne 0) {
    throw "Real-ESRGAN exited with code $LASTEXITCODE"
}
