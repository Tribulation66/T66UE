param(
    [string]$Target,
    [string]$Output,
    [string]$Dump,
    [string]$Exe = "C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe",
    [string]$Map = "/Game/Maps/GameplayLevel",
    [string]$FrontendScreen,
    [string]$CaptureMode = "hudreview",
    [int]$ResX = 1920,
    [int]$ResY = 1080,
    [double]$DelaySeconds = 4.0,
    [int]$TimeoutSeconds = 90,
    [int]$DisplayNumber = 1,
    [int]$WindowOffsetX = 0,
    [int]$WindowOffsetY = 0,
    [switch]$NoPrepareWindowedSettings,
    [switch]$NoDump,
    [switch]$NoAutoClose,
    [switch]$PrintOnly,
    [string[]]$ExtraArgs = @()
)

$ErrorActionPreference = "Stop"
$script:WindowedSettingsBackup = $null

function Get-DisplayBounds {
    param([int]$Number)

    Add-Type -AssemblyName System.Windows.Forms
    $screens = [System.Windows.Forms.Screen]::AllScreens
    $deviceSuffix = "DISPLAY$Number"
    $screen = $screens | Where-Object { $_.DeviceName -match [regex]::Escape($deviceSuffix) } | Select-Object -First 1

    if (-not $screen -and $Number -eq 1) {
        $screen = $screens | Sort-Object { $_.Bounds.X } | Select-Object -First 1
    }

    if (-not $screen) {
        throw "Could not find Windows display $Number. Available displays: $($screens.DeviceName -join ', ')"
    }

    return $screen.Bounds
}

function Get-StagedGameUserSettingsPath {
    param([string]$ExecutablePath)

    $exeDir = Split-Path -Parent ([System.IO.Path]::GetFullPath($ExecutablePath))
    $gameRoot = Resolve-Path -LiteralPath (Join-Path $exeDir "..\..")
    return Join-Path $gameRoot "Saved\Config\Windows\GameUserSettings.ini"
}

function Get-StagedLogDirectory {
    param([string]$ExecutablePath)

    $exeDir = Split-Path -Parent ([System.IO.Path]::GetFullPath($ExecutablePath))
    $gameRoot = Resolve-Path -LiteralPath (Join-Path $exeDir "..\..")
    return Join-Path $gameRoot "Saved\Logs"
}

function Get-LatestStagedLog {
    param(
        [string]$LogDirectory,
        [datetime]$Since
    )

    if (-not $LogDirectory -or -not (Test-Path -LiteralPath $LogDirectory)) {
        return $null
    }

    return Get-ChildItem -LiteralPath $LogDirectory -Filter "*.log" -File |
        Where-Object { $_.LastWriteTime -ge $Since.AddSeconds(-5) } |
        Sort-Object LastWriteTime -Descending |
        Select-Object -First 1
}

function Backup-GameUserSettingsForRestore {
    param([string]$SettingsPath)

    if ($script:WindowedSettingsBackup) {
        return
    }

    $script:WindowedSettingsBackup = [pscustomobject]@{
        Path = $SettingsPath
        Existed = Test-Path -LiteralPath $SettingsPath
        Content = if (Test-Path -LiteralPath $SettingsPath) { Get-Content -LiteralPath $SettingsPath -Raw } else { $null }
    }
}

function Restore-GameUserSettings {
    if (-not $script:WindowedSettingsBackup) {
        return
    }

    $settingsPath = $script:WindowedSettingsBackup.Path
    if ($script:WindowedSettingsBackup.Existed) {
        $settingsDir = Split-Path -Parent $settingsPath
        New-Item -ItemType Directory -Force -Path $settingsDir | Out-Null
        Set-Content -LiteralPath $settingsPath -Value $script:WindowedSettingsBackup.Content -Encoding UTF8
        Write-Host "Restored GameUserSettings: $settingsPath"
    } elseif (Test-Path -LiteralPath $settingsPath) {
        Remove-Item -LiteralPath $settingsPath -Force
        Write-Host "Removed temporary GameUserSettings: $settingsPath"
    }

    $script:WindowedSettingsBackup = $null
}

function Set-WindowedGameUserSettings {
    param(
        [string]$ExecutablePath,
        [int]$Width,
        [int]$Height
    )

    $settingsPath = Get-StagedGameUserSettingsPath -ExecutablePath $ExecutablePath
    Backup-GameUserSettingsForRestore -SettingsPath $settingsPath
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
    Write-Host "Prepared windowed GameUserSettings: $settingsPath ($Width x $Height, FullscreenMode=2)"
}

function Ensure-ParentDirectory {
    param([string]$Path)

    if ($Path) {
        $dir = Split-Path -Parent $Path
        if ($dir) {
            New-Item -ItemType Directory -Force -Path $dir | Out-Null
        }
    }
}

try {
    if (-not (Test-Path -LiteralPath $Exe)) {
        throw "Missing executable: $Exe"
    }

    $outputPath = if ($Output) { [System.IO.Path]::GetFullPath($Output) } else { $null }
    $dumpPath = if ($NoDump) { $null } elseif ($Dump) { [System.IO.Path]::GetFullPath($Dump) } else { $null }
    if (-not $outputPath -and -not $dumpPath) {
        throw "Provide -Output, -Dump, or both."
    }
    if ($dumpPath -and [string]::IsNullOrWhiteSpace($Target)) {
        throw "Provide -Target when writing a widget dump, or pass -NoDump for screenshot-only captures."
    }
    if (-not $dumpPath) {
        if (-not $NoDump -and $outputPath) {
            $dumpPath = [System.IO.Path]::ChangeExtension($outputPath, ".json")
        }
    }
    if ($dumpPath -and [string]::IsNullOrWhiteSpace($Target)) {
        throw "Provide -Target when writing a widget dump, or pass -NoDump for screenshot-only captures."
    }

    Ensure-ParentDirectory -Path $outputPath
    Ensure-ParentDirectory -Path $dumpPath
    foreach ($path in @($outputPath, $dumpPath)) {
        if ($path -and (Test-Path -LiteralPath $path)) {
            Remove-Item -LiteralPath $path -Force
        }
    }

    if (-not $NoPrepareWindowedSettings) {
        Set-WindowedGameUserSettings -ExecutablePath $Exe -Width $ResX -Height $ResY
    }

    $bounds = Get-DisplayBounds -Number $DisplayNumber
    $winX = [int]$bounds.X + $WindowOffsetX
    $winY = [int]$bounds.Y + $WindowOffsetY

    $argsList = @()
    if (-not $FrontendScreen -and $Map) {
        $argsList += $Map
    }
    $argsList += @(
        "-windowed",
        "-ResX=$ResX",
        "-ResY=$ResY",
        "-WinX=$winX",
        "-WinY=$winY",
        "-T66AutomationResX=$ResX",
        "-T66AutomationResY=$ResY",
        "-T66AutomationWindowed"
    )

    if ($FrontendScreen) {
        $argsList += "-T66FrontendScreen=$FrontendScreen"
        if ($outputPath) {
            $argsList += "-T66AutoScreenshot=`"$($outputPath.Replace('"', '\"'))`""
            $argsList += "-T66AutoScreenshotDelay=$DelaySeconds"
        }
        if ($dumpPath) {
            $argsList += "-T66AutoDumpWidget=`"$Target`:$($dumpPath.Replace('"', '\"'))`""
            $argsList += "-T66AutoDumpWidgetDelay=$DelaySeconds"
        }
    } else {
        $argsList += "-T66GameplayAutoCapture=$CaptureMode"
        if ($outputPath) {
            $argsList += "-T66GameplayAutoScreenshot=`"$($outputPath.Replace('"', '\"'))`""
            $argsList += "-T66GameplayAutoScreenshotDelay=$DelaySeconds"
        }
        if ($dumpPath) {
            $argsList += "-T66AutoDumpWidget=`"$Target`:$($dumpPath.Replace('"', '\"'))`""
            $argsList += "-T66AutoDumpWidgetDelay=$DelaySeconds"
        }
    }

    $argsList += $ExtraArgs

    Write-Host "Launching on display $DisplayNumber at WinX=$winX WinY=$winY"
    Write-Host "$Exe $($argsList -join ' ')"

    if ($PrintOnly) {
        return
    }

    $launchTime = Get-Date
    $logDirectory = Get-StagedLogDirectory -ExecutablePath $Exe
    $process = Start-Process -FilePath $Exe -ArgumentList $argsList -PassThru

    if ($NoAutoClose) {
        Write-Host "Started PID $($process.Id). NoAutoClose was set."
        return
    }

    $deadline = (Get-Date).AddSeconds($TimeoutSeconds)
    while ((Get-Date) -lt $deadline) {
        $hasOutput = (-not $outputPath) -or (Test-Path -LiteralPath $outputPath)
        $hasDump = (-not $dumpPath) -or (Test-Path -LiteralPath $dumpPath)
        if ($hasOutput -and $hasDump) {
            break
        }
        if ($process.HasExited) {
            Start-Sleep -Milliseconds 500
            break
        }
        Start-Sleep -Milliseconds 500
    }

    $latestLog = Get-LatestStagedLog -LogDirectory $logDirectory -Since $launchTime
    if ($process.HasExited -and $process.ExitCode -ne 0) {
        throw "Game exited with ExitCode=$($process.ExitCode). Log: $($latestLog.FullName)"
    }

    Stop-Process -Id $process.Id -Force -ErrorAction SilentlyContinue

    if ($outputPath -and -not (Test-Path -LiteralPath $outputPath)) {
        throw "Screenshot was not created before timeout: $outputPath"
    }
    if ($dumpPath -and -not (Test-Path -LiteralPath $dumpPath)) {
        throw "Widget dump was not created before timeout: $dumpPath"
    }

    if ($outputPath) {
        Add-Type -AssemblyName System.Drawing
        $image = [System.Drawing.Image]::FromFile($outputPath)
        try {
            if ($image.Width -ne $ResX -or $image.Height -ne $ResY) {
                throw "Screenshot dimensions were $($image.Width)x$($image.Height), expected $ResX x $ResY`: $outputPath"
            }
        }
        finally {
            $image.Dispose()
        }
    }

    if ($outputPath) {
        Write-Host "Captured $outputPath"
    }
    if ($dumpPath) {
        Write-Host "Dumped $dumpPath"
    }
} finally {
    Restore-GameUserSettings
}
