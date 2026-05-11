param(
    [ValidateSet("Development", "Shipping")]
    [string]$ClientConfig = "Development",

    [string]$EngineRoot = "C:\Program Files\Epic Games\UE_5.7",

    [string]$StageRoot = "",

    [switch]$SkipBuild,
    [switch]$SkipCook,
    [switch]$SkipShortcutRefresh
)

$ErrorActionPreference = "Stop"

$ProjectRoot = Split-Path -Parent $PSScriptRoot
$StageStandaloneScript = Join-Path $PSScriptRoot "StageStandaloneBuild.ps1"
if ([string]::IsNullOrWhiteSpace($StageRoot)) {
    $StageRoot = Join-Path $ProjectRoot "Saved\StagedBuildsDemo"
} else {
    $StageRoot = [System.IO.Path]::GetFullPath($StageRoot)
}

function Update-DemoShortcut {
    param(
        [Parameter(Mandatory = $true)]
        [string]$ShortcutPath,

        [Parameter(Mandatory = $true)]
        [string]$TargetPath,

        [Parameter(Mandatory = $true)]
        [string]$Arguments,

        [Parameter(Mandatory = $true)]
        [string]$WorkingDirectory
    )

    $ParentDirectory = Split-Path -Parent $ShortcutPath
    if (-not (Test-Path $ParentDirectory)) {
        New-Item -ItemType Directory -Path $ParentDirectory -Force | Out-Null
    }

    $Shell = New-Object -ComObject WScript.Shell
    $Shortcut = $Shell.CreateShortcut($ShortcutPath)
    $Shortcut.TargetPath = $TargetPath
    $Shortcut.Arguments = $Arguments
    $Shortcut.WorkingDirectory = $WorkingDirectory
    $Shortcut.IconLocation = "$TargetPath,0"
    $Shortcut.Save()
}

if (-not (Test-Path -LiteralPath $StageStandaloneScript)) {
    throw "StageStandaloneBuild.ps1 was not found at '$StageStandaloneScript'."
}

$StageArgs = @{
    ClientConfig = $ClientConfig
    EngineRoot = $EngineRoot
    StageRoot = $StageRoot
    SkipShortcutRefresh = $true
}
if ($SkipBuild) { $StageArgs.SkipBuild = $true }
if ($SkipCook) { $StageArgs.SkipCook = $true }

& $StageStandaloneScript @StageArgs
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

$ExpectedExe = Join-Path $StageRoot "Windows\T66\Binaries\Win64\T66.exe"
if (-not (Test-Path -LiteralPath $ExpectedExe)) {
    throw "Demo stage completed but expected executable was not found at '$ExpectedExe'."
}

$ExpectedExe = (Resolve-Path -LiteralPath $ExpectedExe).Path
$ExeDirectory = Split-Path -Parent $ExpectedExe
$AppIdPath = Join-Path $ExeDirectory "steam_appid.txt"
Set-Content -LiteralPath $AppIdPath -Value "4718770" -NoNewline
Write-Host "Wrote demo steam_appid.txt at '$AppIdPath'."

if (-not $SkipShortcutRefresh) {
    $DemoLogDirectory = Join-Path $ProjectRoot "Saved\StandaloneLogs"
    $DemoLogPath = Join-Path $DemoLogDirectory "T66_Demo_Standalone.log"
    New-Item -ItemType Directory -Path $DemoLogDirectory -Force | Out-Null

    $DemoShortcutPath = Join-Path $ProjectRoot "T66 Demo Standalone.lnk"
    $DemoArguments = "-T66Demo -abslog=`"$DemoLogPath`" -forcelogflush"
    Update-DemoShortcut `
        -ShortcutPath $DemoShortcutPath `
        -TargetPath $ExpectedExe `
        -Arguments $DemoArguments `
        -WorkingDirectory $ExeDirectory

    Write-Host "Updated demo shortcut '$DemoShortcutPath' -> '$ExpectedExe'."
}

Write-Host "Demo build ready at '$ExpectedExe'."
