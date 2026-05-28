param(
    [string]$Project = "C:\UE\T66\T66.uproject",
    [string]$EditorExe = "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe",
    [string]$Map = "/Game/Maps/GameplayLevel",
    [string]$CaptureMode = "hero1axeaoe",
    [string]$Output,
    [string]$FrameDir,
    [string]$FramePrefix = "frame",
    [switch]$LootUIAnimationBatch,
    [string]$BatchOutputRoot,
    [int]$ResX = 1280,
    [int]$ResY = 720,
    [int]$FrameCount = 36,
    [int]$FrameRate = 12,
    [double]$CaptureIntervalSeconds = 0.08,
    [double]$DelaySeconds = 5.0,
    [double]$PostCaptureDelaySeconds = 0.2,
    [int]$TimeoutSeconds = 150,
    [string]$ExecCmds = "",
    [string[]]$ExtraArgs = @(),
    [switch]$UseReviewCamera,
    [switch]$UseHero1AxePreviewStaging,
    [int]$PreviewCameraArmLength = 540,
    [double]$PreviewCameraPitch = -30.0,
    [double]$PreviewCameraPivotHeight = 145.0,
    [double]$Hero1AxeLabForwardOffset = 360.0,
    [double]$Hero1AxeLabRightOffset = 0.0,
    [double]$Hero1AxeLabVerticalOffset = -82.0,
    [int]$Hero1AxeTargetCount = 3,
    [double]$Hero1AxeTargetSpacing = 145.0,
    [double]$Hero1AxeTargetForwardOffset = 210.0,
    [string]$Hero1AxeTargetMob = "Slime",
    [double]$Hero1AxeHitboxFireDelay = 7.6,
    [double]$Hero1AxeHitboxVFXLeadSeconds = 0.12,
    [string]$Hero1AxeProofItems = "",
    [int]$Hero1AxeProofLine1 = 8,
    [int]$Hero1AxeProofSecondary = 1,
    [switch]$Hero1AxeKeepProofInventory,
    [switch]$NoHero1AxeTargets,
    [switch]$RemoveFrames,
    [switch]$EvidenceBundle,
    [string]$EvidenceRoot,
    [string]$EvidenceLabel = "",
    [string]$EvidenceSelectedFrames = "",
    # Opt-in evidence packaging helper only; gameplay capture defaults stay unchanged.
    [switch]$EvidenceAutoSelectFrames,
    [switch]$PrintOnly
)

$ErrorActionPreference = "Stop"

function Update-ProcessPathFromRegistry {
    $machinePath = [Environment]::GetEnvironmentVariable("Path", "Machine")
    $userPath = [Environment]::GetEnvironmentVariable("Path", "User")
    $processPath = [Environment]::GetEnvironmentVariable("Path", "Process")
    $parts = @($machinePath, $userPath, $processPath) -join ";"
    $env:Path = (($parts -split ";" | Where-Object { $_ } | Select-Object -Unique) -join ";")
}

function Resolve-FFmpeg {
    Update-ProcessPathFromRegistry

    $command = Get-Command ffmpeg -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($command) {
        return $command.Source
    }

    $wrapper = Join-Path $env:USERPROFILE "bin\ffmpeg.cmd"
    if (Test-Path -LiteralPath $wrapper) {
        return $wrapper
    }

    $python = Get-Command python -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($python) {
        $imageioFFmpeg = & $python.Source -c "import imageio_ffmpeg; print(imageio_ffmpeg.get_ffmpeg_exe())" 2>$null
        if ($LASTEXITCODE -eq 0 -and $imageioFFmpeg -and (Test-Path -LiteralPath $imageioFFmpeg.Trim())) {
            return $imageioFFmpeg.Trim()
        }
    }

    $wingetRoot = Join-Path $env:LOCALAPPDATA "Microsoft\WinGet\Packages"
    if (Test-Path -LiteralPath $wingetRoot) {
        $wingetFFmpeg = Get-ChildItem -LiteralPath $wingetRoot -Filter ffmpeg.exe -Recurse -File -ErrorAction SilentlyContinue |
            Where-Object { $_.FullName -match "Gyan\.FFmpeg" } |
            Sort-Object LastWriteTime -Descending |
            Select-Object -First 1
        if ($wingetFFmpeg) {
            return $wingetFFmpeg.FullName
        }
    }

    throw "Could not resolve ffmpeg. Install FFmpeg or run the user PATH setup, then open a fresh terminal."
}

function Resolve-FFprobe {
    param([string]$FFmpegPath)

    Update-ProcessPathFromRegistry

    $command = Get-Command ffprobe -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($command) {
        return $command.Source
    }

    $wrapper = Join-Path $env:USERPROFILE "bin\ffprobe.cmd"
    if (Test-Path -LiteralPath $wrapper) {
        return $wrapper
    }

    if ($FFmpegPath) {
        $ffmpegItem = Get-Item -LiteralPath $FFmpegPath -ErrorAction SilentlyContinue
        if ($ffmpegItem -and $ffmpegItem.DirectoryName) {
            foreach ($candidateName in @("ffprobe.exe", "ffprobe.cmd")) {
                $candidate = Join-Path $ffmpegItem.DirectoryName $candidateName
                if (Test-Path -LiteralPath $candidate) {
                    return $candidate
                }
            }
        }
    }

    $wingetRoot = Join-Path $env:LOCALAPPDATA "Microsoft\WinGet\Packages"
    if (Test-Path -LiteralPath $wingetRoot) {
        $wingetFFprobe = Get-ChildItem -LiteralPath $wingetRoot -Filter ffprobe.exe -Recurse -File -ErrorAction SilentlyContinue |
            Where-Object { $_.FullName -match "Gyan\.FFmpeg" } |
            Sort-Object LastWriteTime -Descending |
            Select-Object -First 1
        if ($wingetFFprobe) {
            return $wingetFFprobe.FullName
        }
    }

    throw "Could not resolve ffprobe. Evidence mode requires ffprobe metadata; install FFmpeg with ffprobe or add ffprobe to PATH."
}

function Resolve-Python {
    $python = Get-Command python -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($python) {
        return $python.Source
    }
    throw "Could not resolve python. Evidence mode requires Python with Pillow installed."
}

function Get-RepoRoot {
    return [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot ".."))
}

function Join-CommandLineValue {
    param([string]$Name, [string]$Value)
    if ($Value -match "[\s,]") {
        return "$Name=`"$($Value.Replace('"', '\"'))`""
    }
    return "$Name=$Value"
}

$repoRoot = Get-RepoRoot
$timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
$reviewCameraExecCmds = "T66.Camera.GameplayPreset 1,T66.Camera.LockedChasePitch -72,T66.Camera.LockedChaseArmLength 1550,T66.Camera.LockedChasePivotHeight 260,T66.Camera.ConstrainAgainstTowerWalls 0"

if ($UseReviewCamera -and -not $ExecCmds) {
    $ExecCmds = $reviewCameraExecCmds
}
if ($UseHero1AxePreviewStaging) {
    if (-not $ExecCmds) {
        $ExecCmds = "T66.Camera.GameplayPreset 1,T66.Camera.LockedChasePitch $PreviewCameraPitch,T66.Camera.LockedChaseArmLength $PreviewCameraArmLength,T66.Camera.LockedChasePivotHeight $PreviewCameraPivotHeight,T66.Camera.ConstrainAgainstTowerWalls 0"
    }
    $ExtraArgs += @(
        "-T66Hero1AxeAOECenterPlayer",
        "-T66GameplayAutoLockCameraZoom",
        "-T66GameplayAutoCameraArmLength=$PreviewCameraArmLength",
        "-T66Hero1AxeAOELabForwardOffset=$Hero1AxeLabForwardOffset",
        "-T66Hero1AxeAOELabRightOffset=$Hero1AxeLabRightOffset",
        "-T66Hero1AxeAOELabVerticalOffset=$Hero1AxeLabVerticalOffset"
    )
    if (-not $NoHero1AxeTargets) {
        $ExtraArgs += @(
            "-T66Hero1AxeAOESpawnTargets",
            "-T66Hero1AxeAOETargetCount=$Hero1AxeTargetCount",
            "-T66Hero1AxeAOETargetSpacing=$Hero1AxeTargetSpacing",
            "-T66Hero1AxeAOETargetForwardOffset=$Hero1AxeTargetForwardOffset",
            "-T66Hero1AxeAOETargetMob=$Hero1AxeTargetMob"
        )
    }
}

$normalizedCaptureMode = $CaptureMode.Trim().ToLowerInvariant()
if ($normalizedCaptureMode -eq "hero1axeaoehitbox" -or $normalizedCaptureMode -eq "hero1axeaoevfxbinding") {
    $ExtraArgs += @(
        "-T66Hero1AxeAOEHitboxFireDelay=$Hero1AxeHitboxFireDelay",
        "-T66Hero1AxeAOEHitboxVFXLeadSeconds=$Hero1AxeHitboxVFXLeadSeconds"
    )
    if ($normalizedCaptureMode -eq "hero1axeaoevfxbinding") {
        if ($Hero1AxeProofItems) {
            $ExtraArgs += "-T66Hero1AxeAOEProofItems=$Hero1AxeProofItems"
        }
        $ExtraArgs += @(
            "-T66Hero1AxeAOEProofLine1=$Hero1AxeProofLine1",
            "-T66Hero1AxeAOEProofSecondary=$Hero1AxeProofSecondary"
        )
        if ($Hero1AxeKeepProofInventory) {
            $ExtraArgs += "-T66Hero1AxeAOEKeepProofInventory"
        }
    }
    if (-not $ExecCmds) {
        $ExecCmds = "T66.Camera.GameplayPreset 1,T66.Camera.LockedChasePitch $PreviewCameraPitch,T66.Camera.LockedChaseArmLength $PreviewCameraArmLength,T66.Camera.LockedChasePivotHeight $PreviewCameraPivotHeight,T66.Camera.ConstrainAgainstTowerWalls 0,T66.Combat.DebugView 2,T66.Combat.DebugLabels 1"
    }
    elseif ($ExecCmds -notmatch "T66\.Combat\.DebugView") {
        $ExecCmds = "$ExecCmds,T66.Combat.DebugView 2,T66.Combat.DebugLabels 1"
    }
}

if ($LootUIAnimationBatch) {
    if ($EvidenceBundle) {
        throw "EvidenceBundle is not supported with LootUIAnimationBatch. Run per-target gameplay captures when VFX evidence is needed."
    }
    if (-not $BatchOutputRoot) {
        $BatchOutputRoot = Join-Path $repoRoot "Saved\VideoCaptures\LootUIAnimations_$timestamp"
    }
    $batchRootPath = [System.IO.Path]::GetFullPath($BatchOutputRoot)
    New-Item -ItemType Directory -Force -Path $batchRootPath | Out-Null

    $targets = @(
        [pscustomobject]@{
            Name = "LootCrate"
            Mode = "lootcrate"
            FrameCount = 108
            FrameRate = 12
            Interval = 0.083333
            PostDelay = 0.1
            Taxonomy = "LiveLootUIAnimation"
            Notes = "Uses StartCrateOpenHUD with deterministic Yellow source rarity."
        },
        [pscustomobject]@{
            Name = "LootChest"
            Mode = "lootchest"
            FrameCount = 48
            FrameRate = 12
            Interval = 0.083333
            PostDelay = 0.1
            Taxonomy = "LiveLootUIAnimation"
            Notes = "Uses StartChestRewardHUD with deterministic Yellow rarity and 188 gold."
        },
        [pscustomobject]@{
            Name = "LootWheel"
            Mode = "lootwheel"
            FrameCount = 84
            FrameRate = 12
            Interval = 0.083333
            PostDelay = 0.1
            Taxonomy = "LiveLootUIAnimation"
            Notes = "Spawns a Yellow showcase LootWheel interactable and triggers the real lock/deferred-commit radial wheel path."
        },
        [pscustomobject]@{
            Name = "LootBag"
            Mode = "lootbag"
            FrameCount = 36
            FrameRate = 12
            Interval = 0.083333
            PostDelay = 0.1
            Taxonomy = "RewardCardUI"
            Notes = "Adds Item_AoeDamage at Yellow rarity to RunState, then shows the pickup item card."
        }
    )

    foreach ($target in $targets) {
        $targetRoot = Join-Path $batchRootPath $target.Name
        $targetOutput = Join-Path $targetRoot "$($target.Name)_isolated.mp4"
        $targetFrames = Join-Path $targetRoot "frames"
        $targetArgs = @(
            "-NoProfile",
            "-ExecutionPolicy",
            "Bypass",
            "-File",
            $PSCommandPath,
            "-Project",
            $Project,
            "-EditorExe",
            $EditorExe,
            "-Map",
            $Map,
            "-CaptureMode",
            $target.Mode,
            "-Output",
            $targetOutput,
            "-FrameDir",
            $targetFrames,
            "-FramePrefix",
            "frame",
            "-ResX",
            $ResX,
            "-ResY",
            $ResY,
            "-FrameCount",
            $target.FrameCount,
            "-FrameRate",
            $target.FrameRate,
            "-CaptureIntervalSeconds",
            $target.Interval,
            "-DelaySeconds",
            $DelaySeconds,
            "-PostCaptureDelaySeconds",
            $target.PostDelay,
            "-TimeoutSeconds",
            $TimeoutSeconds
        )
        if ($ExecCmds) {
            $targetArgs += @("-ExecCmds", $ExecCmds)
        }
        if ($RemoveFrames) {
            $targetArgs += "-RemoveFrames"
        }
        if ($PrintOnly) {
            $targetArgs += "-PrintOnly"
        }

        Write-Host "=== Loot UI batch target: $($target.Name) ($($target.Mode)) ==="
        & powershell @targetArgs
        if ($LASTEXITCODE -ne 0) {
            throw "Loot UI batch target failed: $($target.Name)"
        }
    }

    $summaryPath = Join-Path $batchRootPath "LootUIAnimationCaptureSummary.md"
    $summaryLines = @(
        "# Loot UI Animation Capture Summary",
        "",
        "Default targets exclude Mini/minigame content unless explicitly named.",
        "",
        "| Target | Taxonomy | Artifact | Notes |",
        "|---|---|---|---|"
    )
    foreach ($target in $targets) {
        $artifact = Join-Path (Join-Path $batchRootPath $target.Name) "$($target.Name)_isolated.mp4"
        $summaryLines += "| $($target.Name) | $($target.Taxonomy) | `$artifact` | $($target.Notes) |"
    }
    $summaryLines += ""
    $summaryLines += "Taxonomy:"
    $summaryLines += "- `LiveLootUIAnimation`: target-owned animated UI after interaction."
    $summaryLines += "- `RewardCardUI`: post-reward/result card UI after interaction, not an opening/spin animation."
    $summaryLines += "- `Gap_NoCurrentUIAnimation`: requested target lacks target-owned live UI animation."
    $summaryLines += "- `SelectionCardUI`: pre-commit choice/selection card UI such as idol/weapon altar; preserved but not part of this default batch."
    $summaryLines | Set-Content -LiteralPath $summaryPath -Encoding UTF8

    Write-Host "Loot UI batch root: $batchRootPath"
    Write-Host "Summary: $summaryPath"
    return
}

if (-not $Output) {
    $Output = Join-Path $repoRoot "Saved\VideoCaptures\${CaptureMode}_$timestamp\$CaptureMode.mp4"
}
if (-not $FrameDir) {
    $FrameDir = Join-Path (Split-Path -Parent $Output) "frames"
}

$projectPath = [System.IO.Path]::GetFullPath($Project)
$outputPath = [System.IO.Path]::GetFullPath($Output)
$frameDirPath = [System.IO.Path]::GetFullPath($FrameDir)
$ffmpeg = Resolve-FFmpeg
$evidenceRootPath = $null
if ($EvidenceBundle) {
    if ($RemoveFrames) {
        throw "EvidenceBundle requires retained PNG frames. Do not pass -RemoveFrames with -EvidenceBundle."
    }
    if ($FrameCount -lt 4) {
        throw "EvidenceBundle requires at least 4 frames for start/mid/impact/dissipate selection."
    }
    if (-not $EvidenceRoot) {
        $EvidenceRoot = Join-Path (Split-Path -Parent $outputPath) "evidence"
    }
    $evidenceRootPath = [System.IO.Path]::GetFullPath($EvidenceRoot)
}

if (-not (Test-Path -LiteralPath $projectPath)) {
    throw "Missing project: $projectPath"
}
if (-not (Test-Path -LiteralPath $EditorExe)) {
    throw "Missing editor executable: $EditorExe"
}
if ($FrameCount -lt 1 -or $FrameCount -gt 240) {
    throw "FrameCount must be between 1 and 240."
}
if ($FrameRate -lt 1 -or $FrameRate -gt 120) {
    throw "FrameRate must be between 1 and 120."
}

New-Item -ItemType Directory -Force -Path $frameDirPath | Out-Null
New-Item -ItemType Directory -Force -Path (Split-Path -Parent $outputPath) | Out-Null
Get-ChildItem -LiteralPath $frameDirPath -Filter "$FramePrefix*.png" -File -ErrorAction SilentlyContinue | Remove-Item -Force
if (Test-Path -LiteralPath $outputPath) {
    Remove-Item -LiteralPath $outputPath -Force
}

$argsList = @(
    $projectPath,
    $Map,
    "-game",
    "-windowed",
    "-ResX=$ResX",
    "-ResY=$ResY",
    "-T66AutomationResX=$ResX",
    "-T66AutomationResY=$ResY",
    "-T66AutomationWindowed",
    "-T66GameplayAutoCapture=$CaptureMode",
    (Join-CommandLineValue "-T66GameplayAutoScreenshotSequenceDir" $frameDirPath),
    "-T66GameplayAutoScreenshotSequencePrefix=$FramePrefix",
    "-T66GameplayAutoScreenshotSequenceCount=$FrameCount",
    "-T66GameplayAutoScreenshotSequenceInterval=$CaptureIntervalSeconds",
    "-T66GameplayAutoScreenshotDelay=$DelaySeconds",
    "-T66GameplayAutoPostCaptureScreenshotDelay=$PostCaptureDelaySeconds",
    "-unattended",
    "-nop4",
    "-nosplash"
)

if ($ExecCmds) {
    $argsList += Join-CommandLineValue "-ExecCmds" $ExecCmds
}
$argsList += $ExtraArgs

Write-Host "FFmpeg: $ffmpeg"
Write-Host "Frames: $frameDirPath"
Write-Host "Output: $outputPath"
if ($EvidenceBundle) {
    Write-Host "Evidence bundle: $evidenceRootPath"
}
Write-Host "$EditorExe $($argsList -join ' ')"

if ($PrintOnly) {
    return
}

$process = Start-Process -FilePath $EditorExe -ArgumentList $argsList -PassThru
$deadline = (Get-Date).AddSeconds($TimeoutSeconds)
while (-not $process.HasExited -and (Get-Date) -lt $deadline) {
    Start-Sleep -Milliseconds 500
}

if (-not $process.HasExited) {
    Stop-Process -Id $process.Id -Force -ErrorAction SilentlyContinue
    throw "Timed out waiting for Unreal capture after $TimeoutSeconds seconds."
}
if ($process.ExitCode -ne 0) {
    throw "Unreal exited with ExitCode=$($process.ExitCode). Check C:\UE\T66\Saved\Logs for details."
}

Start-Sleep -Seconds 2
$frames = Get-ChildItem -LiteralPath $frameDirPath -Filter "$FramePrefix*.png" -File | Sort-Object Name
if ($frames.Count -lt $FrameCount) {
    throw "Expected $FrameCount frames, found $($frames.Count) in $frameDirPath."
}

$inputPattern = Join-Path $frameDirPath "$FramePrefix`_%04d.png"
& $ffmpeg -hide_banner -y -framerate $FrameRate -i $inputPattern -vf "scale=${ResX}:-2" -c:v libx264 -pix_fmt yuv420p -movflags +faststart $outputPath
if ($LASTEXITCODE -ne 0) {
    throw "ffmpeg failed with exit code $LASTEXITCODE."
}
if (-not (Test-Path -LiteralPath $outputPath)) {
    throw "Video was not created: $outputPath"
}

$videoFile = Get-Item -LiteralPath $outputPath
if ($videoFile.Length -le 0) {
    throw "Video is empty: $outputPath"
}

if ($EvidenceBundle) {
    $ffprobe = Resolve-FFprobe -FFmpegPath $ffmpeg
    $python = Resolve-Python
    $evidenceScript = Join-Path $PSScriptRoot "BuildT66VideoEvidenceBundle.py"
    if (-not (Test-Path -LiteralPath $evidenceScript)) {
        throw "Missing evidence helper: $evidenceScript"
    }

    New-Item -ItemType Directory -Force -Path $evidenceRootPath | Out-Null
    $ffprobeJsonPath = Join-Path $evidenceRootPath "ffprobe.json"
    $ffprobeArgs = @(
        "-v", "error",
        "-select_streams", "v:0",
        "-show_entries", "format=duration,size,bit_rate:stream=width,height,nb_frames,r_frame_rate,avg_frame_rate,duration,codec_name,pix_fmt",
        "-of", "json",
        $outputPath
    )
    $ffprobeOutput = & $ffprobe @ffprobeArgs
    if ($LASTEXITCODE -ne 0) {
        throw "ffprobe failed with exit code $LASTEXITCODE."
    }
    $ffprobeOutput | Set-Content -LiteralPath $ffprobeJsonPath -Encoding UTF8

    $frameRateText = $FrameRate.ToString([System.Globalization.CultureInfo]::InvariantCulture)
    $evidenceArgs = @(
        $evidenceScript,
        "--video", $outputPath,
        "--frame-dir", $frameDirPath,
        "--frame-prefix", $FramePrefix,
        "--frame-rate", $frameRateText,
        "--capture-mode", $CaptureMode,
        "--output-root", $evidenceRootPath,
        "--ffprobe-json", $ffprobeJsonPath,
        "--res-x", $ResX,
        "--res-y", $ResY
    )
    if ($EvidenceLabel) {
        $evidenceArgs += @("--label", $EvidenceLabel)
    }
    if ($EvidenceSelectedFrames) {
        $evidenceArgs += @("--selected-frames", $EvidenceSelectedFrames)
    }
    if ($EvidenceAutoSelectFrames) {
        $evidenceArgs += @("--auto-select-frames")
    }
    & $python @evidenceArgs
    if ($LASTEXITCODE -ne 0) {
        throw "Evidence bundle helper failed with exit code $LASTEXITCODE."
    }
    Write-Host "Evidence bundle: $evidenceRootPath"
}

if ($RemoveFrames) {
    Remove-Item -LiteralPath $frameDirPath -Recurse -Force
} else {
    Write-Host "Frame sequence: $frameDirPath"
}
Write-Host "Captured video: $outputPath"
