param(
    [int[]]$Counts = @(500, 2000, 10000, 20000),
    [string]$Project = "C:\UE\T66\T66.uproject",
    [string]$EditorExe = "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe",
    [string]$Map = "/Game/Maps/GameplayLevel",
    [string]$OutputRoot,
    [int]$ResX = 1280,
    [int]$ResY = 720,
    [int]$FrameCount = 1,
    [int]$FrameRate = 12,
    [double]$CaptureIntervalSeconds = 0.083333,
    [double]$DelaySeconds = 4.0,
    # The single proof frame is intentionally delayed until after the stress
    # manifest sample window; repeated screenshots distort game-thread timing.
    [double]$PostCaptureDelaySeconds = 4.0,
    [int]$TimeoutSeconds = 420,
    [switch]$KeepFrames,
    [switch]$PrintOnly
)

$ErrorActionPreference = "Stop"

$repoRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot ".."))
if (-not $OutputRoot) {
    $timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
    $OutputRoot = Join-Path $repoRoot "Saved\VideoCaptures\OutgoingTravelerStressAB_$timestamp"
}
$outputRootPath = [System.IO.Path]::GetFullPath($OutputRoot)
New-Item -ItemType Directory -Force -Path $outputRootPath | Out-Null

$captureScript = Join-Path $PSScriptRoot "CaptureT66GameplayVideo.ps1"
if (-not (Test-Path -LiteralPath $captureScript)) {
    throw "Missing capture script: $captureScript"
}

$execCmds = "T66.Camera.GameplayPreset 1,T66.Camera.LockedChasePitch -18,T66.Camera.LockedChaseArmLength 2400,T66.Camera.LockedChasePivotHeight 520,T66.Camera.ConstrainAgainstTowerWalls 0,T66.Camera.WallOcclusionEnabled 0,t.MaxFPS 0,r.VSync 0"
$summary = @()

foreach ($count in $Counts) {
    foreach ($config in @("individual_mesh", "pooled")) {
        $usePool = if ($config -eq "pooled") { 1 } else { 0 }
        $caseName = "{0}_{1}" -f $config, $count
        $caseDir = Join-Path $outputRootPath $caseName
        $frameDir = Join-Path $caseDir "frames"
        $videoPath = Join-Path $caseDir "$caseName.mp4"
        $manifestPath = Join-Path $caseDir "stress_manifest.json"

        New-Item -ItemType Directory -Force -Path $caseDir | Out-Null
        $extraArgs = @(
            "-T66OutgoingTravelerStressCount=$count",
            "-T66OutgoingTravelerStressUsePool=$usePool",
            "-T66OutgoingTravelerStressManifest=$manifestPath",
            "-T66OutgoingTravelerStressWarmupSeconds=0.75",
            "-T66OutgoingTravelerStressSampleSeconds=2.5",
            "-T66OutgoingTravelerStressSpacing=10",
            "-T66OutgoingTravelerStressSpawnDistance=2400",
            "-T66OutgoingTravelerStressDisableCollision=1"
        )

        Write-Host "=== Outgoing traveler stress: $config count=$count ==="
        try {
            $captureParams = @{
                Project = $Project
                EditorExe = $EditorExe
                Map = $Map
                CaptureMode = "outgoingtravelerstress"
                Output = $videoPath
                FrameDir = $frameDir
                FramePrefix = "frame"
                ResX = $ResX
                ResY = $ResY
                FrameCount = $FrameCount
                FrameRate = $FrameRate
                CaptureIntervalSeconds = $CaptureIntervalSeconds
                DelaySeconds = $DelaySeconds
                PostCaptureDelaySeconds = $PostCaptureDelaySeconds
                TimeoutSeconds = $TimeoutSeconds
                ExecCmds = $execCmds
                ExtraArgs = $extraArgs
            }
            if (-not $KeepFrames) {
                $captureParams.RemoveFrames = $true
            }
            if ($PrintOnly) {
                $captureParams.PrintOnly = $true
            }
            & $captureScript @captureParams
            $exitCode = if ($LASTEXITCODE -ne $null) { $LASTEXITCODE } else { 0 }
        }
        catch {
            Write-Warning $_.Exception.Message
            $exitCode = 1
        }
        $status = if ($exitCode -eq 0) { "completed" } else { "failed" }
        $summary += [pscustomobject]@{
            Config = $config
            Count = $count
            Status = $status
            ExitCode = $exitCode
            Manifest = $manifestPath
            Video = $videoPath
        }
        if ($exitCode -ne 0) {
            Write-Warning "Capture failed for $config count=$count with exit code $exitCode. Continuing remaining cases."
        }
    }
}

$summaryPath = Join-Path $outputRootPath "stress_ab_summary.json"
$summary | ConvertTo-Json -Depth 4 | Set-Content -LiteralPath $summaryPath -Encoding UTF8

Write-Host "Outgoing traveler stress A/B root: $outputRootPath"
Write-Host "Summary: $summaryPath"
