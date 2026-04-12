param(
    [ValidateSet("Development", "Shipping")]
    [string]$ClientConfig = "Development",

    [string]$EngineRoot = "C:\Program Files\Epic Games\UE_5.7",

    [switch]$SkipBuild,
    [switch]$SkipCook
)

$ErrorActionPreference = "Stop"

$ProjectRoot = Split-Path -Parent $PSScriptRoot
$UProjectPath = Join-Path $ProjectRoot "T66.uproject"
$StageRoot = Join-Path $ProjectRoot "Saved\StagedBuilds"
$RunUATPath = Join-Path $EngineRoot "Engine\Build\BatchFiles\RunUAT.bat"

if (-not (Test-Path $RunUATPath)) {
    throw "RunUAT.bat not found at '$RunUATPath'. Pass -EngineRoot with the correct Unreal installation root."
}

$UatArgs = @(
    "BuildCookRun",
    "-project=$UProjectPath",
    "-noP4",
    "-platform=Win64",
    "-clientconfig=$ClientConfig",
    "-stage",
    "-pak",
    "-package",
    "-stagingdirectory=$StageRoot",
    "-utf8output"
)

if ($SkipBuild) {
    $UatArgs += "-skipbuild"
} else {
    $UatArgs += "-build"
}

if ($SkipCook) {
    $UatArgs += "-skipcook"
} else {
    $UatArgs += "-cook"
}

Write-Host "Staging standalone build to '$StageRoot\Windows\T66'..."
Write-Host "& `"$RunUATPath`" $($UatArgs -join ' ')"

& $RunUATPath @UatArgs
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

$ExpectedExe = Join-Path $StageRoot "Windows\T66\Binaries\Win64\T66.exe"
if (Test-Path $ExpectedExe) {
    Write-Host "Standalone build ready at '$ExpectedExe'."
} else {
    throw "Build completed but expected executable was not found at '$ExpectedExe'."
}
