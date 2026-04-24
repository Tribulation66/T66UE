param(
    [Parameter(Mandatory = $true)]
    [string]$BuildSource,

    [string]$SteamworksSdkRoot = "C:\SteamworksSDK\sdk",

    [string]$AppScript = "app_build_4464300_root.vdf",

    [string]$ContentFolderName = "CHADPOCALYPSE",

    [string]$Description = "",

    [string]$SetLiveBeta = "",

    [string]$SteamLogin = "tribulation66",

    [switch]$Preview
)

$ErrorActionPreference = "Stop"

function Resolve-FullPath([string]$PathValue) {
    return [System.IO.Path]::GetFullPath($PathValue)
}

$ResolvedBuildSource = Resolve-FullPath $BuildSource
if (-not (Test-Path -LiteralPath $ResolvedBuildSource)) {
    throw "Build source does not exist: $ResolvedBuildSource"
}

$ResolvedSdkRoot = Resolve-FullPath $SteamworksSdkRoot
$ContentBuilderRoot = Join-Path $ResolvedSdkRoot "tools\ContentBuilder"
$BuilderExe = Join-Path $ContentBuilderRoot "builder\steamcmd.exe"
$ScriptsRoot = Join-Path $ContentBuilderRoot "scripts"
$AppBuildScript = Join-Path $ScriptsRoot $AppScript
$ContentRoot = Join-Path $ContentBuilderRoot "content\$ContentFolderName"

if (-not (Test-Path -LiteralPath $BuilderExe)) {
    throw "steamcmd.exe was not found at $BuilderExe"
}

if (-not (Test-Path -LiteralPath $AppBuildScript)) {
    throw "App build script was not found at $AppBuildScript"
}

Write-Host "Preparing Steam content root: $ContentRoot"
New-Item -ItemType Directory -Force -Path $ContentRoot | Out-Null

Write-Host "Clearing previous staged Steam content..."
Get-ChildItem -LiteralPath $ContentRoot -Force -ErrorAction SilentlyContinue | Remove-Item -Recurse -Force

Write-Host "Copying packaged build from $ResolvedBuildSource"
Get-ChildItem -LiteralPath $ResolvedBuildSource -Force | ForEach-Object {
    Copy-Item -LiteralPath $_.FullName -Destination $ContentRoot -Recurse -Force
}

$SteamAppIdFiles = Get-ChildItem -LiteralPath $ContentRoot -Filter "steam_appid.txt" -Recurse -Force -ErrorAction SilentlyContinue
if ($SteamAppIdFiles) {
    Write-Host "Removing local-only steam_appid.txt from Steam upload content..."
    $SteamAppIdFiles | Remove-Item -Force
}

if (-not [string]::IsNullOrWhiteSpace($Description) -or $Preview.IsPresent -or -not [string]::IsNullOrWhiteSpace($SetLiveBeta)) {
    $AppBuildText = Get-Content -LiteralPath $AppBuildScript -Raw
    if (-not [string]::IsNullOrWhiteSpace($Description)) {
        $EscapedDescription = $Description.Replace('"', '\"')
        $AppBuildText = [regex]::Replace($AppBuildText, '"Desc"\s*"[^"]*"', ('"Desc" "{0}"' -f $EscapedDescription), 1)
    }

    if ($Preview.IsPresent) {
        $AppBuildText = [regex]::Replace($AppBuildText, '"Preview"\s*"[^"]*"', '"Preview" "1"', 1)
    } else {
        $AppBuildText = [regex]::Replace($AppBuildText, '"Preview"\s*"[^"]*"', '"Preview" "0"', 1)
    }

    if ([string]::IsNullOrWhiteSpace($SetLiveBeta)) {
        $AppBuildText = [regex]::Replace($AppBuildText, '^\s*"SetLive"\s*"[^"]*"\s*\r?\n', '', 'Multiline')
    } elseif ($AppBuildText -match '"SetLive"\s*"[^"]*"') {
        $AppBuildText = [regex]::Replace($AppBuildText, '"SetLive"\s*"[^"]*"', ('"SetLive" "{0}"' -f $SetLiveBeta), 1)
    } else {
        $PreviewValue = if ($Preview.IsPresent) { "1" } else { "0" }
        $Replacement = ('"Preview" "{0}"' -f $PreviewValue) + [Environment]::NewLine + "`t`"SetLive`" `"$SetLiveBeta`""
        $AppBuildText = [regex]::Replace($AppBuildText, '"Preview"\s*"[^"]*"', $Replacement, 1)
    }

    Set-Content -LiteralPath $AppBuildScript -Value $AppBuildText -NoNewline
}

Push-Location (Join-Path $ContentBuilderRoot "builder")
try {
    Write-Host "Uploading build using $AppBuildScript"
    $SteamCmdArgs = @()
    if (-not [string]::IsNullOrWhiteSpace($SteamLogin)) {
        $SteamCmdArgs += "+login"
        $SteamCmdArgs += $SteamLogin
    }
    $SteamCmdArgs += "+run_app_build"
    $SteamCmdArgs += $AppBuildScript
    $SteamCmdArgs += "+quit"
    & $BuilderExe @SteamCmdArgs
} finally {
    Pop-Location
}
