# Copyright Tribulation 66. All Rights Reserved.
#
# MotionRig capture harness (MOTION_RIG.md section 4).
# Launches the game straight into the Test Room with Hero_1 (which spawns the
# MotionRig pawn while t66.MotionRig.TestRoom=1), self-runs a deterministic
# scenario, captures a frame sequence via the existing T66 screenshot
# sequencer, then assembles a review MP4 + contact-sheet PNGs and collects the
# telemetry CSV the scenario wrote.
#
# Example:
#   pwsh Scripts/MotionRig/CaptureMotionRig.ps1 -Scenario dive -Camera side -Label dive_v1
#
param(
    [string]$Project = "C:\UE\T66\T66.uproject",
    [string]$EditorExe = "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe",
    [string]$Map = "/Game/Maps/GameplayLevel",
    [ValidateSet("walkcircle", "jumptriple", "dive", "impact", "full")]
    [string]$Scenario = "full",
    [ValidateSet("side", "front", "threequarter", "chase")]
    [string]$Camera = "side",
    [string]$Label = "",
    [string]$OutRoot = "C:\UE\T66\Reports\AgentReviews\MotionRig_20260609\captures",
    [int]$ResX = 1280,
    [int]$ResY = 720,
    [double]$FrameInterval = 0.125,
    [int]$FrameCount = 0,            # 0 = derive from scenario length
    [double]$StartDelaySeconds = 6.0, # world load + pawn settle before scenario+frames
    [double]$SloMo = 1.0,             # 0.25 for slow-motion review passes
    [int]$TimeoutSeconds = 240,
    [string]$ExtraExecCmds = "",
    [switch]$KeepFrames
)

$ErrorActionPreference = "Stop"

# Scenario wall-clock lengths (seconds) — keep in sync with
# UT66MotionRigScenario::BuildScenario.
$scenarioSeconds = @{
    walkcircle = 10.0
    jumptriple = 8.0
    dive       = 7.0
    impact     = 8.0
    full       = 31.0
}

if (-not $Label) { $Label = "{0}_{1}" -f $Scenario, $Camera }
$outDir = Join-Path $OutRoot $Label
$frameDir = Join-Path $outDir "frames"
New-Item -ItemType Directory -Force -Path $frameDir | Out-Null
Get-ChildItem -LiteralPath $frameDir -Filter "frame_*.png" -ErrorAction SilentlyContinue | Remove-Item -Force

$lengthSeconds = $scenarioSeconds[$Scenario] / [Math]::Max(0.05, $SloMo)
if ($FrameCount -le 0) {
    $FrameCount = [Math]::Min(240, [int][Math]::Ceiling(($lengthSeconds + 1.5) / $FrameInterval))
}

$execCmds = "t66.MotionRig.TestRoom 1"
if ($SloMo -ne 1.0) { $execCmds += ",slomo $SloMo" }
if ($ExtraExecCmds) { $execCmds += ",$ExtraExecCmds" }

$args = @(
    $Project,
    $Map,
    "-game",
    "-windowed",
    "-ResX=$ResX", "-ResY=$ResY",
    "-NoSound",
    "-NoSplash",
    "-T66Entry=Run:TestRoom",
    "-T66Hero=Hero_1",
    "-T66MotionRigScenario=$Scenario",
    "-T66MotionRigCamera=$Camera",
    "-T66MotionRigScenarioDelay=$StartDelaySeconds",
    "-T66GameplayAutoScreenshotSequenceDir=$frameDir",
    "-T66GameplayAutoScreenshotSequenceCount=$FrameCount",
    "-T66GameplayAutoScreenshotSequenceInterval=$FrameInterval",
    "-T66GameplayAutoScreenshotDelay=$StartDelaySeconds",
    "-ExecCmds=$execCmds"
)

Write-Host "[MotionRig] Capturing scenario '$Scenario' camera '$Camera' -> $outDir"
Write-Host "[MotionRig] Frames: $FrameCount @ ${FrameInterval}s (scenario ${lengthSeconds}s)"

$process = Start-Process -FilePath $EditorExe -ArgumentList $args -PassThru
if (-not $process.WaitForExit($TimeoutSeconds * 1000)) {
    Write-Warning "[MotionRig] Timeout after ${TimeoutSeconds}s — terminating capture process."
    Stop-Process -Id $process.Id -Force -ErrorAction SilentlyContinue
    Start-Sleep -Seconds 2
}

$frames = @(Get-ChildItem -LiteralPath $frameDir -Filter "frame_*.png" -ErrorAction SilentlyContinue | Sort-Object Name)
Write-Host "[MotionRig] Captured $($frames.Count) frames."
if ($frames.Count -lt 8) {
    Write-Error "[MotionRig] Capture produced too few frames ($($frames.Count)) — inspect the game log."
}

# --- locate ffmpeg (same resolution order as CaptureT66GameplayVideo.ps1) ---
function Resolve-FFmpeg {
    $command = Get-Command ffmpeg -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($command) { return $command.Source }
    $wrapper = Join-Path $env:USERPROFILE "bin\ffmpeg.cmd"
    if (Test-Path -LiteralPath $wrapper) { return $wrapper }
    $python = Get-Command python -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($python) {
        $imageioFFmpeg = & $python.Source -c "import imageio_ffmpeg; print(imageio_ffmpeg.get_ffmpeg_exe())" 2>$null
        if ($LASTEXITCODE -eq 0 -and $imageioFFmpeg -and (Test-Path -LiteralPath $imageioFFmpeg.Trim())) {
            return $imageioFFmpeg.Trim()
        }
    }
    return $null
}

$ffmpeg = Resolve-FFmpeg
if ($ffmpeg -and $frames.Count -ge 8) {
    $fps = [Math]::Round(1.0 / $FrameInterval, 2)
    $mp4 = Join-Path $outDir "$Label.mp4"
    & $ffmpeg -y -framerate $fps -i (Join-Path $frameDir "frame_%04d.png") -c:v libx264 -pix_fmt yuv420p -crf 20 $mp4 2>$null | Out-Null
    Write-Host "[MotionRig] Video: $mp4"

    # Contact sheets: 6x5 grids of evenly sampled frames, max ~2 sheets.
    $tileCount = 30
    $sheets = [Math]::Min(2, [Math]::Ceiling($frames.Count / [double]$tileCount))
    for ($s = 0; $s -lt $sheets; $s++) {
        $sheetPath = Join-Path $outDir ("{0}_sheet{1}.png" -f $Label, ($s + 1))
        $startIdx = [int]($s * $frames.Count / $sheets)
        $endIdx = [int](($s + 1) * $frames.Count / $sheets) - 1
        $span = [Math]::Max(1, $endIdx - $startIdx)
        $select = "select='between(n\,$startIdx\,$endIdx)*not(mod(n-$startIdx\,$([Math]::Max(1, [int]($span / $tileCount)))))'"
        & $ffmpeg -y -i (Join-Path $frameDir "frame_%04d.png") -vf "$select,scale=320:-1,tile=6x5" -frames:v 1 $sheetPath 2>$null | Out-Null
        if (Test-Path $sheetPath) { Write-Host "[MotionRig] Sheet: $sheetPath" }
    }
}
elseif (-not $ffmpeg) {
    Write-Warning "[MotionRig] ffmpeg not found — frames retained, no video/sheets."
    $KeepFrames = $true
}

# --- collect the newest telemetry CSV for this scenario ---
$telemetryDir = "C:\UE\T66\Saved\MotionRig"
$telemetry = Get-ChildItem -LiteralPath $telemetryDir -Filter "telemetry_${Scenario}_*.csv" -ErrorAction SilentlyContinue |
    Sort-Object LastWriteTime -Descending | Select-Object -First 1
if ($telemetry) {
    Copy-Item $telemetry.FullName (Join-Path $outDir "telemetry.csv") -Force
    Write-Host "[MotionRig] Telemetry: $(Join-Path $outDir 'telemetry.csv')"
}
else {
    Write-Warning "[MotionRig] No telemetry CSV found for scenario '$Scenario' under $telemetryDir."
}

if (-not $KeepFrames) {
    # Keep every 4th frame for archaeology; drop the rest to save disk.
    $kept = 0
    for ($i = 0; $i -lt $frames.Count; $i++) {
        if ($i % 4 -ne 0) { Remove-Item $frames[$i].FullName -Force -ErrorAction SilentlyContinue }
        else { $kept++ }
    }
    Write-Host "[MotionRig] Kept $kept of $($frames.Count) raw frames."
}

Write-Host "[MotionRig] DONE -> $outDir"
