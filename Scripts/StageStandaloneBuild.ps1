param(
    [ValidateSet("Development", "Shipping")]
    [string]$ClientConfig = "Development",

    [string]$EngineRoot = "C:\Program Files\Epic Games\UE_5.7",

    [string]$StageRoot = "",

    [switch]$SkipBuild,
    [switch]$SkipCook,
    [switch]$SkipShortcutRefresh,
    [switch]$ResetSavedGames
)

$ErrorActionPreference = "Stop"

$ProjectRoot = Split-Path -Parent $PSScriptRoot
$UProjectPath = Join-Path $ProjectRoot "T66.uproject"
if ([string]::IsNullOrWhiteSpace($StageRoot)) {
    $StageRoot = Join-Path $ProjectRoot "Saved\StagedBuilds"
} else {
    $StageRoot = [System.IO.Path]::GetFullPath($StageRoot)
}
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

function Set-StandaloneGameUserSettings {
    param(
        [Parameter(Mandatory = $true)]
        [string]$ExecutablePath,

        [int]$Width = 1920,

        [int]$Height = 1080
    )

    $exeDir = Split-Path -Parent ([System.IO.Path]::GetFullPath($ExecutablePath))
    $gameRoot = Resolve-Path -LiteralPath (Join-Path $exeDir "..\..")
    $settingsPath = Join-Path $gameRoot "Saved\Config\Windows\GameUserSettings.ini"
    $settingsDir = Split-Path -Parent $settingsPath
    New-Item -ItemType Directory -Force -Path $settingsDir | Out-Null

    $content = if (Test-Path -LiteralPath $settingsPath) {
        Get-Content -LiteralPath $settingsPath -Raw
    } else {
        ";METADATA=(Diff=true, UseCommands=true)`r`n[ScalabilityGroups]`r`nsg.ResolutionQuality=100`r`n`r`n[/Script/Engine.GameUserSettings]`r`n"
    }

    if ($content -notmatch "(?m)^\[/Script/Engine\.GameUserSettings\]") {
        if (-not $content.EndsWith("`r`n")) {
            $content += "`r`n"
        }
        $content += "`r`n[/Script/Engine.GameUserSettings]`r`n"
    }

    $pairs = [ordered]@{
        "ResolutionSizeX" = "$Width"
        "ResolutionSizeY" = "$Height"
        "LastUserConfirmedResolutionSizeX" = "$Width"
        "LastUserConfirmedResolutionSizeY" = "$Height"
        "DesiredScreenWidth" = "$Width"
        "DesiredScreenHeight" = "$Height"
        "LastUserConfirmedDesiredScreenWidth" = "$Width"
        "LastUserConfirmedDesiredScreenHeight" = "$Height"
        "bUseDesiredScreenHeight" = "False"
        "FullscreenMode" = "2"
        "LastConfirmedFullscreenMode" = "2"
        "PreferredFullscreenMode" = "2"
    }

    foreach ($key in $pairs.Keys) {
        $pattern = "(?m)^$([regex]::Escape($key))=.*$"
        $line = "$key=$($pairs[$key])"
        if ($content -match $pattern) {
            $content = [regex]::Replace($content, $pattern, $line)
        } else {
            $content = [regex]::Replace(
                $content,
                "(?m)^(\[/Script/Engine\.GameUserSettings\]\r?\n)",
                "`$1$line`r`n",
                1)
        }
    }

    Set-Content -LiteralPath $settingsPath -Value $content -Encoding UTF8
    Write-Host "Reset standalone GameUserSettings: $settingsPath ($Width x $Height, windowed)."
}

function Get-LooseRuntimeContentRoots {
    param(
        [Parameter(Mandatory = $true)]
        [string]$ConfigPath
    )

    if (-not (Test-Path -LiteralPath $ConfigPath)) {
        return @()
    }

    $Roots = New-Object System.Collections.Generic.List[string]
    foreach ($Line in Get-Content -LiteralPath $ConfigPath) {
        $Match = [regex]::Match($Line, 'LooseRuntimeContentRoots=\(RelativePath="([^"]+)"')
        if ($Match.Success) {
            $Roots.Add($Match.Groups[1].Value)
        }
    }

    return $Roots.ToArray()
}

function Copy-LooseRuntimeContentRoot {
    param(
        [Parameter(Mandatory = $true)]
        [string]$RelativePath,

        [Parameter(Mandatory = $true)]
        [string]$DestinationProjectRoot
    )

    $NormalizedRelativePath = $RelativePath.Replace("/", "\").Trim()
    if ($NormalizedRelativePath.EndsWith("\...")) {
        $NormalizedRelativePath = $NormalizedRelativePath.Substring(0, $NormalizedRelativePath.Length - 4)
    } elseif ($NormalizedRelativePath.EndsWith("...")) {
        $NormalizedRelativePath = $NormalizedRelativePath.Substring(0, $NormalizedRelativePath.Length - 3).TrimEnd("\")
    }

    $NormalizedRelativePath = $NormalizedRelativePath.TrimEnd("\")
    if ([string]::IsNullOrWhiteSpace($NormalizedRelativePath)) {
        return
    }

    $ProjectRootFull = [System.IO.Path]::GetFullPath($ProjectRoot).TrimEnd("\") + "\"
    $DestinationRootFull = [System.IO.Path]::GetFullPath($DestinationProjectRoot).TrimEnd("\") + "\"
    $SourcePath = [System.IO.Path]::GetFullPath((Join-Path $ProjectRoot $NormalizedRelativePath))
    $DestinationPath = [System.IO.Path]::GetFullPath((Join-Path $DestinationProjectRoot $NormalizedRelativePath))

    if (-not $SourcePath.StartsWith($ProjectRootFull, [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "Loose runtime source '$SourcePath' is outside project root '$ProjectRootFull'."
    }

    if (-not $DestinationPath.StartsWith($DestinationRootFull, [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "Loose runtime destination '$DestinationPath' is outside staged project root '$DestinationRootFull'."
    }

    if (-not (Test-Path -LiteralPath $SourcePath)) {
        Write-Host "Loose runtime root '$RelativePath' not found in project; skipping."
        return
    }

    $SourceItem = Get-Item -LiteralPath $SourcePath
    if ($SourceItem.PSIsContainer) {
        New-Item -ItemType Directory -Force -Path $DestinationPath | Out-Null
        foreach ($ChildItem in Get-ChildItem -LiteralPath $SourcePath -Force) {
            $ChildDestinationPath = Join-Path $DestinationPath $ChildItem.Name
            if ($ChildItem.PSIsContainer) {
                New-Item -ItemType Directory -Force -Path $ChildDestinationPath | Out-Null
                foreach ($DescendantItem in Get-ChildItem -LiteralPath $ChildItem.FullName -Recurse -Force) {
                    $RelativeDescendantPath = $DescendantItem.FullName.Substring($ChildItem.FullName.Length).TrimStart("\")
                    if ([string]::IsNullOrWhiteSpace($RelativeDescendantPath)) {
                        continue
                    }

                    $DescendantDestinationPath = Join-Path $ChildDestinationPath $RelativeDescendantPath
                    if ($DescendantItem.PSIsContainer) {
                        New-Item -ItemType Directory -Force -Path $DescendantDestinationPath | Out-Null
                    } else {
                        $DescendantDestinationParent = Split-Path -Parent $DescendantDestinationPath
                        New-Item -ItemType Directory -Force -Path $DescendantDestinationParent | Out-Null
                        Copy-Item -LiteralPath $DescendantItem.FullName -Destination $DescendantDestinationPath -Force
                    }
                }
            } else {
                $ChildDestinationParent = Split-Path -Parent $ChildDestinationPath
                New-Item -ItemType Directory -Force -Path $ChildDestinationParent | Out-Null
                Copy-Item -LiteralPath $ChildItem.FullName -Destination $ChildDestinationPath -Force
            }
        }

        $FileCount = @(Get-ChildItem -LiteralPath $SourcePath -File -Recurse -Force).Count
        Write-Host "Refreshed loose runtime root '$RelativePath' -> '$DestinationPath' ($FileCount files)."
    } else {
        $DestinationParent = Split-Path -Parent $DestinationPath
        New-Item -ItemType Directory -Force -Path $DestinationParent | Out-Null
        Copy-Item -LiteralPath $SourcePath -Destination $DestinationPath -Force
        Write-Host "Refreshed loose runtime file '$RelativePath' -> '$DestinationPath'."
    }
}

function Backup-StagedSaveGames {
    param(
        [Parameter(Mandatory = $true)]
        [string]$SaveGamesPath,

        [Parameter(Mandatory = $true)]
        [string]$BackupRoot,

        [Parameter(Mandatory = $true)]
        [string]$StageRootPath
    )

    if ($ResetSavedGames) {
        Write-Host "ResetSavedGames requested; staged SaveGames will not be preserved."
        return $null
    }

    if (-not (Test-Path -LiteralPath $SaveGamesPath)) {
        Write-Host "No staged SaveGames found to preserve at '$SaveGamesPath'."
        return $null
    }

    $SaveGamesFull = (Resolve-Path -LiteralPath $SaveGamesPath).Path
    $StageRootFull = [System.IO.Path]::GetFullPath($StageRootPath).TrimEnd("\") + "\"
    if (-not $SaveGamesFull.StartsWith($StageRootFull, [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "Refusing to back up SaveGames outside stage root. SaveGames='$SaveGamesFull' StageRoot='$StageRootFull'."
    }

    $Timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
    $BackupPath = Join-Path $BackupRoot "StandaloneSaveGames_$Timestamp"
    New-Item -ItemType Directory -Force -Path $BackupPath | Out-Null

    foreach ($ChildItem in Get-ChildItem -LiteralPath $SaveGamesFull -Force) {
        Copy-Item -LiteralPath $ChildItem.FullName -Destination $BackupPath -Recurse -Force
    }

    $FileCount = @(Get-ChildItem -LiteralPath $BackupPath -File -Recurse -Force).Count
    Write-Host "Preserved staged SaveGames from '$SaveGamesFull' -> '$BackupPath' ($FileCount files)."
    return $BackupPath
}

function Restore-StagedSaveGames {
    param(
        [string]$BackupPath,

        [Parameter(Mandatory = $true)]
        [string]$SaveGamesPath,

        [Parameter(Mandatory = $true)]
        [string]$StageRootPath
    )

    if ([string]::IsNullOrWhiteSpace($BackupPath)) {
        return
    }

    if (-not (Test-Path -LiteralPath $BackupPath)) {
        throw "Expected staged SaveGames backup was not found at '$BackupPath'."
    }

    $StageRootFull = [System.IO.Path]::GetFullPath($StageRootPath).TrimEnd("\") + "\"
    $SaveGamesFull = [System.IO.Path]::GetFullPath($SaveGamesPath)
    if (-not $SaveGamesFull.StartsWith($StageRootFull, [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "Refusing to restore SaveGames outside stage root. SaveGames='$SaveGamesFull' StageRoot='$StageRootFull'."
    }

    New-Item -ItemType Directory -Force -Path $SaveGamesFull | Out-Null
    foreach ($ChildItem in Get-ChildItem -LiteralPath $BackupPath -Force) {
        Copy-Item -LiteralPath $ChildItem.FullName -Destination $SaveGamesFull -Recurse -Force
    }

    $FileCount = @(Get-ChildItem -LiteralPath $SaveGamesFull -File -Recurse -Force).Count
    Write-Host "Restored staged SaveGames to '$SaveGamesFull' ($FileCount files)."
}

function Remove-StagedSaveGamesBackup {
    param(
        [string]$BackupPath,

        [Parameter(Mandatory = $true)]
        [string]$BackupRoot
    )

    if ([string]::IsNullOrWhiteSpace($BackupPath) -or -not (Test-Path -LiteralPath $BackupPath)) {
        return
    }

    $BackupFull = (Resolve-Path -LiteralPath $BackupPath).Path
    $BackupRootFull = [System.IO.Path]::GetFullPath($BackupRoot).TrimEnd("\") + "\"
    if (-not $BackupFull.StartsWith($BackupRootFull, [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "Refusing to remove SaveGames backup outside backup root. Backup='$BackupFull' BackupRoot='$BackupRootFull'."
    }

    Remove-Item -LiteralPath $BackupFull -Recurse -Force
    Write-Host "Removed temporary staged SaveGames backup '$BackupFull'."
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
    "-utf8output",
    # Multiple agents stage in parallel; without this a concurrent UAT
    # instance makes BuildCookRun fail instantly with "conflicting instance".
    "-WaitForUATMutex"
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

$StagedProjectRoot = Join-Path $StageRoot "Windows\T66"
$StagedSaveGamesPath = Join-Path $StagedProjectRoot "Saved\SaveGames"
$SaveGamesBackupRoot = Join-Path $ProjectRoot "Saved\StageBackups"
$SaveGamesBackupPath = Backup-StagedSaveGames -SaveGamesPath $StagedSaveGamesPath -BackupRoot $SaveGamesBackupRoot -StageRootPath $StageRoot

Write-Host "Staging standalone build to '$StageRoot\Windows\T66'..."
Write-Host "& `"$RunUATPath`" $($UatArgs -join ' ')"

& $RunUATPath @UatArgs
if ($LASTEXITCODE -ne 0) {
    Restore-StagedSaveGames -BackupPath $SaveGamesBackupPath -SaveGamesPath $StagedSaveGamesPath -StageRootPath $StageRoot
    exit $LASTEXITCODE
}

Restore-StagedSaveGames -BackupPath $SaveGamesBackupPath -SaveGamesPath $StagedSaveGamesPath -StageRootPath $StageRoot
Remove-StagedSaveGamesBackup -BackupPath $SaveGamesBackupPath -BackupRoot $SaveGamesBackupRoot

$LooseRuntimeConfig = Join-Path $ProjectRoot "Config\DefaultGame.ini"
foreach ($LooseRuntimeRoot in Get-LooseRuntimeContentRoots -ConfigPath $LooseRuntimeConfig) {
    Copy-LooseRuntimeContentRoot -RelativePath $LooseRuntimeRoot -DestinationProjectRoot $StagedProjectRoot
}

$ExpectedExe = Join-Path $StageRoot "Windows\T66\Binaries\Win64\T66.exe"
$ExpectedExeAvailable = $false
for ($Attempt = 0; $Attempt -lt 10; ++$Attempt) {
    if (Test-Path -LiteralPath $ExpectedExe) {
        $ExpectedExeAvailable = $true
        break
    }

    Start-Sleep -Milliseconds 500
}

if ($ExpectedExeAvailable) {
    Write-Host "Standalone build ready at '$ExpectedExe'."
    Set-StandaloneGameUserSettings -ExecutablePath $ExpectedExe

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
