param(
    [string]$Project = "C:\UE\T66\T66.uproject",
    [string]$EditorExe = "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe",
    [int]$TravelerCount = 5000,
    [int]$ResX = 1920,
    [int]$ResY = 1080,
    [double]$OrthoWidth = 2300.0,
    [double]$CameraHeight = 1600.0,
    [double]$ActualTimeSeconds = 0.25,
    [int]$TimeoutSeconds = 360,
    [switch]$SkipBuildAsset,
    [switch]$PrintOnly
)

$ErrorActionPreference = "Stop"

function Get-RepoRoot {
    return [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot ".."))
}

$repoRoot = Get-RepoRoot
$timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
$outputDir = Join-Path $repoRoot "Saved\VFXResearch\FoundationPhase0\OutgoingTravelerSwarm\$timestamp"
New-Item -ItemType Directory -Force -Path $outputDir | Out-Null

if (-not $SkipBuildAsset) {
    $buildArgs = @(
        "`"$Project`"",
        "-run=T66OutgoingTravelerSwarmVFX",
        "-unattended",
        "-nop4",
        "-nosplash"
    )
    $buildLog = Join-Path $outputDir "build_asset.log"
    $buildErr = Join-Path $outputDir "build_asset.err.log"
    $buildCommandLine = "`"$EditorExe`" " + ($buildArgs -join " ")
    Write-Host "[CaptureT66OutgoingTravelerSwarmProof] $buildCommandLine"
    if (-not $PrintOnly) {
        $buildProcess = Start-Process -FilePath $EditorExe -ArgumentList $buildArgs -NoNewWindow -PassThru -RedirectStandardOutput $buildLog -RedirectStandardError $buildErr
        if (-not $buildProcess.WaitForExit($TimeoutSeconds * 1000)) {
            try {
                $buildProcess.Kill()
            } catch {
            }
            throw "Timed out while building proof Niagara asset after $TimeoutSeconds seconds. OutputDir=$outputDir"
        }
        $buildProcess.Refresh()
        $exitCode = $buildProcess.ExitCode
        if ($null -ne $exitCode -and $exitCode -ne 0) {
            throw "Outgoing traveler proof Niagara asset build failed with exit code $($buildProcess.ExitCode). See $buildLog"
        }
        if ($null -eq $exitCode) {
            $buildText = Get-Content -LiteralPath $buildLog -Raw
            if ($buildText -notmatch '\[OutgoingTravelerSwarmVFX\] Saved ') {
                throw "Outgoing traveler proof Niagara asset build did not expose an ExitCode and no save marker was found. See $buildLog"
            }
            Write-Warning "Unreal commandlet process exited but PowerShell did not expose an ExitCode. Continuing because the proof asset save marker was found."
        }
    }
}

$captureScript = Join-Path $PSScriptRoot "CaptureT66GameplayVideo.ps1"
$manifestPath = Join-Path $outputDir "runtime_manifest.json"
$videoPath = Join-Path $outputDir "outgoing_travelers.mp4"
$frameDir = Join-Path $outputDir "frames"
$proofNiagaraPath = "/Game/VFXLab/Foundation/OutgoingTravelers/NS_OutgoingTravelerSwarmProof.NS_OutgoingTravelerSwarmProof"
$gameplayExtraArgs = @(
    "-T66Hero1AxeAOEOverrideNiagara=$proofNiagaraPath",
    "-T66OutgoingTravelerArrayProofCount=$TravelerCount",
    "-T66OutgoingTravelerArrayProofSpacing=12",
    "-T66OutgoingTravelerArrayProofManifest=$manifestPath",
    "-T66Hero1AxeAOECycleDuration=8",
    "-T66Hero1AxeAOEManualWarmupTicks=4",
    "-T66Hero1AxeAOEManualWarmupDelta=0.0166667"
)
$captureArgs = @{
    Project = $Project
    EditorExe = $EditorExe
    CaptureMode = "hero1axeaoe"
    Output = $videoPath
    FrameDir = $frameDir
    ResX = $ResX
    ResY = $ResY
    FrameCount = 48
    FrameRate = 12
    CaptureIntervalSeconds = 0.05
    DelaySeconds = 8.0
    TimeoutSeconds = $TimeoutSeconds
    UseHero1AxePreviewStaging = $true
    NoHero1AxeTargets = $true
    PreviewCameraArmLength = 2200
    PreviewCameraPitch = -72.0
    PreviewCameraPivotHeight = 260.0
    Hero1AxeLabForwardOffset = 0.0
    Hero1AxeLabRightOffset = -450.0
    Hero1AxeLabVerticalOffset = 40.0
    ExecCmds = "T66.Camera.GameplayPreset 1,T66.Camera.LockedChasePitch -72,T66.Camera.LockedChaseArmLength 2200,T66.Camera.LockedChasePivotHeight 260,T66.Camera.ConstrainAgainstTowerWalls 0,T66.Camera.WallOcclusionEnabled 0"
    ExtraArgs = $gameplayExtraArgs
}

if ($PrintOnly) {
    & $captureScript @captureArgs -PrintOnly
    return
}

& $captureScript @captureArgs

$ffmpegCommand = Get-Command ffmpeg -ErrorAction SilentlyContinue | Select-Object -First 1
if (-not $ffmpegCommand) {
    $ffmpegWrapper = Join-Path $env:USERPROFILE ".local\bin\ffmpeg.cmd"
    if (Test-Path -LiteralPath $ffmpegWrapper) {
        $ffmpegCommand = [pscustomobject]@{ Source = $ffmpegWrapper }
    }
}
if (-not $ffmpegCommand) {
    throw "Could not resolve ffmpeg for exact $ResX x $ResY Phase 0 proof encode."
}
$exactVideoPath = Join-Path $outputDir "outgoing_travelers_exact.mp4"
$scaleFilter = "scale=${ResX}:${ResY}:force_original_aspect_ratio=decrease,pad=${ResX}:${ResY}:(ow-iw)/2:(oh-ih)/2"
& $ffmpegCommand.Source -hide_banner -y -i $videoPath -vf $scaleFilter -c:v libx264 -pix_fmt yuv420p -movflags +faststart $exactVideoPath
if ($LASTEXITCODE -ne 0) {
    throw "Exact Phase 0 proof encode failed with exit code $LASTEXITCODE"
}
Move-Item -LiteralPath $exactVideoPath -Destination $videoPath -Force

if (-not (Test-Path -LiteralPath $manifestPath)) {
    throw "Runtime proof manifest was not created: $manifestPath"
}

$manifest = Get-Content -LiteralPath $manifestPath -Raw | ConvertFrom-Json
if ($manifest.array_proof_requested_count -ne $TravelerCount) {
    throw "Manifest requested count mismatch. Expected $TravelerCount got $($manifest.array_proof_requested_count)"
}
if ($manifest.array_proof_uploaded_count -ne $TravelerCount -or $manifest.array_proof_readback_count -ne $TravelerCount) {
    throw "Manifest array upload/readback mismatch. Uploaded=$($manifest.array_proof_uploaded_count) Readback=$($manifest.array_proof_readback_count)"
}
if (-not $manifest.single_persistent_niagara_component) {
    throw "Manifest did not confirm single_persistent_niagara_component=true"
}
if (-not $manifest.component_active -or $manifest.actual_execution_state -ne "Active") {
    throw "Manifest did not confirm an active Niagara component. ComponentActive=$($manifest.component_active) ActualState=$($manifest.actual_execution_state)"
}
if ($manifest.runtime_emitter_count -ne 1) {
    throw "Manifest did not confirm a single runtime emitter. RuntimeEmitterCount=$($manifest.runtime_emitter_count)"
}
$reportedParticleCount = 0
if ($null -ne $manifest.runtime_total_particles_reported) {
    $reportedParticleCount = [int]$manifest.runtime_total_particles_reported
} elseif ($null -ne $manifest.runtime_total_particles_cpu_counted) {
    $reportedParticleCount = [int]$manifest.runtime_total_particles_cpu_counted
}
if ($reportedParticleCount -lt $TravelerCount) {
    throw "Manifest did not confirm $TravelerCount live Niagara-reported particles. RuntimeTotal=$reportedParticleCount"
}

Write-Host "[CaptureT66OutgoingTravelerSwarmProof] OutputDir=$outputDir"
Write-Host "[CaptureT66OutgoingTravelerSwarmProof] video=$videoPath"
Write-Host "[CaptureT66OutgoingTravelerSwarmProof] frames=$frameDir"
Write-Host "[CaptureT66OutgoingTravelerSwarmProof] runtime_manifest.json=$manifestPath"
