param(
    [string]$Screen = "MainMenu",
    [string]$Output,
    [string]$Exe = "C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe",
    [int]$ResX = 1920,
    [int]$ResY = 1080,
    [double]$DelaySeconds = 3.5,
    [int]$TimeoutSeconds = 45,
    [int]$DisplayNumber = 1,
    [int]$WindowOffsetX = 0,
    [int]$WindowOffsetY = 0,
    [switch]$NoPrepareWindowedSettings,
    [switch]$NoAutoClose,
    [switch]$PrintOnly,
    [string[]]$ExtraArgs = @()
)

$ErrorActionPreference = "Stop"

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

function Set-WindowedGameUserSettings {
    param(
        [string]$ExecutablePath,
        [int]$Width,
        [int]$Height
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
    Write-Host "Prepared windowed GameUserSettings: $settingsPath ($Width x $Height, FullscreenMode=2)"
}

if (-not (Test-Path -LiteralPath $Exe)) {
    throw "Missing executable: $Exe"
}

if (-not $NoPrepareWindowedSettings) {
    Set-WindowedGameUserSettings -ExecutablePath $Exe -Width $ResX -Height $ResY
}

$bounds = Get-DisplayBounds -Number $DisplayNumber
$winX = [int]$bounds.X + $WindowOffsetX
$winY = [int]$bounds.Y + $WindowOffsetY

$argsList = @(
    "-windowed",
    "-ResX=$ResX",
    "-ResY=$ResY",
    "-WinX=$winX",
    "-WinY=$winY",
    "-T66AutomationResX=$ResX",
    "-T66AutomationResY=$ResY",
    "-T66AutomationWindowed"
)

if ($Screen) {
    $argsList += "-T66FrontendScreen=$Screen"
}

if ($Output) {
    $outputPath = [System.IO.Path]::GetFullPath($Output)
    $outputDir = Split-Path -Parent $outputPath
    if ($outputDir) {
        New-Item -ItemType Directory -Force -Path $outputDir | Out-Null
    }
    if (Test-Path -LiteralPath $outputPath) {
        Remove-Item -LiteralPath $outputPath -Force
    }
    $argsList += "-T66AutoScreenshot=$outputPath"
    $argsList += "-T66AutoScreenshotDelay=$DelaySeconds"
} else {
    $outputPath = $null
}

$argsList += $ExtraArgs

Write-Host "Launching on display $DisplayNumber at WinX=$winX WinY=$winY"
Write-Host "$Exe $($argsList -join ' ')"

if ($PrintOnly) {
    return
}

$process = Start-Process -FilePath $Exe -ArgumentList $argsList -PassThru

if ($NoAutoClose) {
    Write-Host "Started PID $($process.Id). NoAutoClose was set."
    return
}

if ($outputPath) {
    $deadline = (Get-Date).AddSeconds($TimeoutSeconds)
    while ((Get-Date) -lt $deadline -and -not (Test-Path -LiteralPath $outputPath)) {
        Start-Sleep -Milliseconds 500
    }

    Stop-Process -Id $process.Id -Force -ErrorAction SilentlyContinue

    if (-not (Test-Path -LiteralPath $outputPath)) {
        throw "Screenshot was not created before timeout: $outputPath"
    }

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

    Write-Host "Captured $outputPath"
    return
}

Write-Host "Started PID $($process.Id)."
