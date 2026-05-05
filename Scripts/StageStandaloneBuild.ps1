param(
    [ValidateSet("Development", "Shipping")]
    [string]$ClientConfig = "Development",

    [string]$EngineRoot = "C:\Program Files\Epic Games\UE_5.7",

    [switch]$SkipBuild,
    [switch]$SkipCook,
    [switch]$SkipShortcutRefresh
)

$ErrorActionPreference = "Stop"

$ProjectRoot = Split-Path -Parent $PSScriptRoot
$UProjectPath = Join-Path $ProjectRoot "T66.uproject"
$StageRoot = Join-Path $ProjectRoot "Saved\StagedBuilds"
$RunUATPath = Join-Path $EngineRoot "Engine\Build\BatchFiles\RunUAT.bat"

function Update-StandaloneShortcut {
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

    if (-not $SkipShortcutRefresh) {
        $ExpectedExe = (Resolve-Path -LiteralPath $ExpectedExe).Path
        $StandaloneWorkingDirectory = Split-Path -Parent $ExpectedExe
        $StandaloneLogDirectory = Join-Path $ProjectRoot "Saved\StandaloneLogs"
        $StandaloneLogPath = Join-Path $StandaloneLogDirectory "T66_Standalone.log"
        $StandaloneArguments = "-abslog=`"$StandaloneLogPath`" -forcelogflush"
        $PinnedTaskbarShortcut = Join-Path $env:APPDATA "Microsoft\Internet Explorer\Quick Launch\User Pinned\TaskBar\T66 Standalone.lnk"
        $ShortcutPaths = @(
            (Join-Path $ProjectRoot "T66 Standalone.lnk"),
            $PinnedTaskbarShortcut
        )

        New-Item -ItemType Directory -Path $StandaloneLogDirectory -Force | Out-Null

        foreach ($ShortcutPath in $ShortcutPaths) {
            if ($ShortcutPath -eq $PinnedTaskbarShortcut -and -not (Test-Path -LiteralPath $ShortcutPath)) {
                Write-Host "Pinned taskbar shortcut not found at '$ShortcutPath'; skipping taskbar refresh."
                continue
            }

            Update-StandaloneShortcut `
                -ShortcutPath $ShortcutPath `
                -TargetPath $ExpectedExe `
                -Arguments $StandaloneArguments `
                -WorkingDirectory $StandaloneWorkingDirectory

            Write-Host "Updated standalone shortcut '$ShortcutPath' -> '$ExpectedExe'."
        }
    }
} else {
    throw "Build completed but expected executable was not found at '$ExpectedExe'."
}
