param(
    [string]$BuildSource = "",

    [string]$SteamworksSdkRoot = "C:\SteamworksSDK\sdk",

    [string]$Description = "CHADPOCALYPSE Steam demo build",

    [string]$SetLiveBeta = "",

    [string]$SteamLogin = "tribulation66",

    [switch]$Preview
)

$ErrorActionPreference = "Stop"

function Resolve-FullPath([string]$PathValue) {
    return [System.IO.Path]::GetFullPath($PathValue)
}

$ProjectRoot = Resolve-FullPath (Join-Path $PSScriptRoot "..\..\..")
if ([string]::IsNullOrWhiteSpace($BuildSource)) {
    $BuildSource = Join-Path $ProjectRoot "Saved\StagedBuildsDemo\Windows\T66"
}

$ResolvedSdkRoot = Resolve-FullPath $SteamworksSdkRoot
$ContentBuilderRoot = Join-Path $ResolvedSdkRoot "tools\ContentBuilder"
$ScriptsRoot = Join-Path $ContentBuilderRoot "scripts"
$AppBuildScript = Join-Path $ScriptsRoot "app_build_4718770_root.vdf"
$DepotBuildScript = Join-Path $ScriptsRoot "depot_build_4718771.vdf"

if (-not (Test-Path -LiteralPath $ScriptsRoot)) {
    throw "SteamPipe scripts folder was not found at '$ScriptsRoot'. Check -SteamworksSdkRoot."
}

$AppBuildText = @'
"AppBuild"
{
    "AppID" "4718770"
    "Desc" "CHADPOCALYPSE Steam demo build"
    "BuildOutput" "../output"
    "ContentRoot" "../content/CHADPOCALYPSE_DEMO"
    "Preview" "0"
    "Depots"
    {
        "4718771" "depot_build_4718771.vdf"
    }
}
'@

$DepotBuildText = @'
"DepotBuildConfig"
{
    "DepotID" "4718771"
    "ContentRoot" "../content/CHADPOCALYPSE_DEMO"
    "FileMapping"
    {
        "LocalPath" "*"
        "DepotPath" "."
        "recursive" "1"
    }
}
'@

if (-not (Test-Path -LiteralPath $AppBuildScript)) {
    Set-Content -LiteralPath $AppBuildScript -Value $AppBuildText -NoNewline
    Write-Host "Created SteamPipe app script '$AppBuildScript'."
}

if (-not (Test-Path -LiteralPath $DepotBuildScript)) {
    Set-Content -LiteralPath $DepotBuildScript -Value $DepotBuildText -NoNewline
    Write-Host "Created SteamPipe depot script '$DepotBuildScript'."
}

$UploadScript = Join-Path $PSScriptRoot "UploadToSteam.ps1"
if (-not (Test-Path -LiteralPath $UploadScript)) {
    throw "UploadToSteam.ps1 was not found at '$UploadScript'."
}

$UploadArgs = @{
    BuildSource = $BuildSource
    SteamworksSdkRoot = $SteamworksSdkRoot
    AppScript = "app_build_4718770_root.vdf"
    ContentFolderName = "CHADPOCALYPSE_DEMO"
    Description = $Description
    SetLiveBeta = $SetLiveBeta
    SteamLogin = $SteamLogin
}
if ($Preview) { $UploadArgs.Preview = $true }

& $UploadScript @UploadArgs
