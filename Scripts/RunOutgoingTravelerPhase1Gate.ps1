param(
    [ValidateSet("Snapshot", "Collision", "Both")]
    [string]$Gate = "Both",
    [string]$StagedExe = "C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe",
    [string]$Map = "/Game/Maps/GameplayLevel",
    [string]$OutputRoot,
    [int]$Runs = 3,
    [int]$Count = 5000,
    [int]$TargetCount = 128,
    [int]$ResX = 1920,
    [int]$ResY = 1080,
    [int]$HeroHPOverride = 20000,
    [int]$TimeoutSeconds = 260,
    [switch]$PrintOnly
)

$ErrorActionPreference = "Stop"

function Get-MetricAverage {
    param($Json, [string]$Name)
    $metric = $Json.$Name
    if ($null -eq $metric -or $null -eq $metric.avg -or $metric.avg -is [string]) {
        return $null
    }
    return [double]$metric.avg
}

function Get-Median {
    param([double[]]$Values)
    $valid = @($Values | Where-Object { $null -ne $_ } | Sort-Object)
    if ($valid.Count -eq 0) {
        return $null
    }
    $middle = [int][Math]::Floor($valid.Count / 2)
    if (($valid.Count % 2) -eq 1) {
        return [double]$valid[$middle]
    }
    return ([double]$valid[$middle - 1] + [double]$valid[$middle]) / 2.0
}

function Get-GateConfig {
    param([string]$GateName)
    switch ($GateName) {
        "Snapshot" {
            return [pscustomobject]@{
                Name = "snapshot"
                TargetSnapshot = 1
                ArrivalCollision = 0
                TravelerSpeed = 1
                WarmupSeconds = 0.75
                SampleSeconds = 2.5
            }
        }
        "Collision" {
            return [pscustomobject]@{
                Name = "collision"
                TargetSnapshot = 1
                ArrivalCollision = 1
                TravelerSpeed = 2400
                WarmupSeconds = 0.75
                SampleSeconds = 2.5
            }
        }
        default {
            throw "Unknown gate '$GateName'."
        }
    }
}

$repoRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot ".."))
if (-not $OutputRoot) {
    $timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
    $OutputRoot = Join-Path $repoRoot "Saved\VideoCaptures\OutgoingTravelerPhase1_$timestamp"
}
$outputRootPath = [System.IO.Path]::GetFullPath($OutputRoot)
New-Item -ItemType Directory -Force -Path $outputRootPath | Out-Null

if (-not (Test-Path -LiteralPath $StagedExe)) {
    throw "Missing staged executable: $StagedExe"
}

$exeItem = Get-Item -LiteralPath $StagedExe
$exeHash = (Get-FileHash -LiteralPath $StagedExe -Algorithm SHA256).Hash
$workingDirectory = Split-Path -Parent $exeItem.FullName
$gateNames = if ($Gate -eq "Both") { @("Snapshot", "Collision") } else { @($Gate) }
$execCmds = "T66.Camera.GameplayPreset 1,T66.Camera.LockedChasePitch -18,T66.Camera.LockedChaseArmLength 2400,T66.Camera.LockedChasePivotHeight 520,T66.Camera.ConstrainAgainstTowerWalls 0,T66.Camera.WallOcclusionEnabled 0,t.MaxFPS 0,r.VSync 0"
$rows = New-Object System.Collections.Generic.List[object]

foreach ($gateName in $gateNames) {
    $gateConfig = Get-GateConfig -GateName $gateName
    for ($runIndex = 1; $runIndex -le $Runs; ++$runIndex) {
        $caseName = "{0}_run{1}" -f $gateConfig.Name, $runIndex
        $caseDir = Join-Path $outputRootPath $caseName
        $frameDir = Join-Path $caseDir "frames"
        $manifestPath = Join-Path $caseDir "stress_manifest.json"
        $poolManifestPath = Join-Path $caseDir "pool_manifest.json"
        $logPath = Join-Path $caseDir "T66.log"
        New-Item -ItemType Directory -Force -Path $frameDir | Out-Null

        $argsList = @(
            $Map,
            "-windowed",
            "-ResX=$ResX",
            "-ResY=$ResY",
            "-T66AutomationResX=$ResX",
            "-T66AutomationResY=$ResY",
            "-T66AutomationWindowed",
            "-T66GameplayAutoCapture=outgoingtravelerstress",
            "-T66GameplayAutoScreenshotSequenceDir=$frameDir",
            "-T66GameplayAutoScreenshotSequencePrefix=frame",
            "-T66GameplayAutoScreenshotSequenceCount=1",
            "-T66GameplayAutoScreenshotSequenceInterval=0.083333",
            "-T66GameplayAutoScreenshotDelay=4.0",
            "-T66GameplayAutoPostCaptureScreenshotDelay=4.0",
            "-T66OutgoingTravelerStressCount=$Count",
            "-T66OutgoingTravelerStressUsePool=1",
            "-T66OutgoingTravelerStressManifest=$manifestPath",
            "-T66OutgoingTravelerPoolProofManifest=$poolManifestPath",
            "-T66OutgoingTravelerStressWarmupSeconds=$($gateConfig.WarmupSeconds)",
            "-T66OutgoingTravelerStressSampleSeconds=$($gateConfig.SampleSeconds)",
            "-T66OutgoingTravelerStressSpacing=10",
            "-T66OutgoingTravelerStressSpawnDistance=2400",
            "-T66OutgoingTravelerStressDisableCollision=1",
            "-T66OutgoingTravelerStressTargetSnapshot=$($gateConfig.TargetSnapshot)",
            "-T66OutgoingTravelerStressArrivalCollision=$($gateConfig.ArrivalCollision)",
            "-T66OutgoingTravelerStressTargetCount=$TargetCount",
            "-T66OutgoingTravelerStressTravelerSpeed=$($gateConfig.TravelerSpeed)",
            "-T66AutoCaptureHeroHPOverride=$HeroHPOverride",
            "-ExecCmds=$execCmds",
            "-abslog=$logPath",
            "-forcelogflush",
            "-unattended",
            "-nosplash"
        )

        Write-Host "=== Phase 1 outgoing traveler gate: $($gateConfig.Name) run=$runIndex count=$Count ==="
        Write-Host "$($exeItem.FullName) $($argsList -join ' ')"
        if ($PrintOnly) {
            continue
        }

        $process = Start-Process -FilePath $exeItem.FullName -ArgumentList $argsList -WorkingDirectory $workingDirectory -PassThru
        $deadline = (Get-Date).AddSeconds($TimeoutSeconds)
        while (-not $process.HasExited -and (Get-Date) -lt $deadline) {
            Start-Sleep -Milliseconds 500
        }
        if (-not $process.HasExited) {
            Stop-Process -Id $process.Id -Force -ErrorAction SilentlyContinue
            throw "Timed out waiting for staged outgoing traveler gate '$($gateConfig.Name)' run $runIndex after $TimeoutSeconds seconds."
        }
        if ($process.ExitCode -ne 0) {
            throw "Staged outgoing traveler gate '$($gateConfig.Name)' run $runIndex exited with code $($process.ExitCode). Log: $logPath"
        }
        if (-not (Test-Path -LiteralPath $manifestPath)) {
            throw "Missing stress manifest for '$($gateConfig.Name)' run $runIndex`: $manifestPath"
        }
        $frames = @(Get-ChildItem -LiteralPath $frameDir -Filter "frame_*.png" -File -ErrorAction SilentlyContinue)
        if ($frames.Count -lt 1) {
            throw "Missing full-resolution screenshot frame for '$($gateConfig.Name)' run $runIndex`: $frameDir"
        }

        $json = Get-Content -LiteralPath $manifestPath -Raw | ConvertFrom-Json
        $pool = $json.pool_diagnostics
        $rows.Add([pscustomobject]@{
            Gate = $gateConfig.Name
            Run = $runIndex
            Count = $Count
            TargetCount = $TargetCount
            Manifest = $manifestPath
            PoolManifest = $poolManifestPath
            Log = $logPath
            Frame = $frames[0].FullName
            ExitCode = $process.ExitCode
            FpsAvg = Get-MetricAverage $json "fps"
            FrameMsAvg = Get-MetricAverage $json "frame_ms"
            GameThreadMsAvg = Get-MetricAverage $json "game_thread_ms"
            GpuFrameMsAvg = Get-MetricAverage $json "gpu_frame_ms"
            DrawCallsAvg = Get-MetricAverage $json "draw_calls"
            PoolSimulationMsAvg = Get-MetricAverage $json "pool_last_simulation_ms_sampled"
            PoolTargetSnapshotMsAvg = Get-MetricAverage $json "pool_last_target_snapshot_ms_sampled"
            PoolArrivalCollisionMsAvg = Get-MetricAverage $json "pool_last_arrival_collision_ms_sampled"
            PoolUploadMsAvg = Get-MetricAverage $json "pool_last_upload_ms_sampled"
            PoolLiveCount = [int]$pool.live_count
            PoolPeakLiveCount = [int]$pool.peak_live_count
            TargetSnapshotBuildCount = [int]$pool.target_snapshot_build_count
            LastTargetSnapshotCount = [int]$pool.last_target_snapshot_count
            ArrivalCheckTotal = [int]$pool.arrival_check_total
            ArrivalDamageAppliedTotal = [int]$pool.arrival_damage_applied_total
            ArrivalFizzleNoTargetTotal = [int]$pool.arrival_fizzle_no_target_total
            SimulatedArrivedTotal = [int]$pool.simulated_arrived_total
            SimulatedExpiredTotal = [int]$pool.simulated_expired_total
            DroppedTotal = [int]$pool.dropped_total
        }) | Out-Null
    }
}

$medianRows = foreach ($gateName in ($rows | Select-Object -ExpandProperty Gate -Unique)) {
    $gateRows = @($rows | Where-Object { $_.Gate -eq $gateName })
    [pscustomobject]@{
        Gate = $gateName
        Runs = $gateRows.Count
        Count = $Count
        FpsMedian = Get-Median @($gateRows | ForEach-Object { $_.FpsAvg })
        FrameMsMedian = Get-Median @($gateRows | ForEach-Object { $_.FrameMsAvg })
        GameThreadMsMedian = Get-Median @($gateRows | ForEach-Object { $_.GameThreadMsAvg })
        GpuFrameMsMedian = Get-Median @($gateRows | ForEach-Object { $_.GpuFrameMsAvg })
        DrawCallsMedian = Get-Median @($gateRows | ForEach-Object { $_.DrawCallsAvg })
        PoolSimulationMsMedian = Get-Median @($gateRows | ForEach-Object { $_.PoolSimulationMsAvg })
        PoolTargetSnapshotMsMedian = Get-Median @($gateRows | ForEach-Object { $_.PoolTargetSnapshotMsAvg })
        PoolArrivalCollisionMsMedian = Get-Median @($gateRows | ForEach-Object { $_.PoolArrivalCollisionMsAvg })
        PoolUploadMsMedian = Get-Median @($gateRows | ForEach-Object { $_.PoolUploadMsAvg })
        PoolPeakLiveCountMax = ($gateRows | Measure-Object -Property PoolPeakLiveCount -Maximum).Maximum
        TargetSnapshotBuildCountMedian = Get-Median @($gateRows | ForEach-Object { [double]$_.TargetSnapshotBuildCount })
        LastTargetSnapshotCountMedian = Get-Median @($gateRows | ForEach-Object { [double]$_.LastTargetSnapshotCount })
        ArrivalCheckTotalMedian = Get-Median @($gateRows | ForEach-Object { [double]$_.ArrivalCheckTotal })
        ArrivalDamageAppliedTotalMedian = Get-Median @($gateRows | ForEach-Object { [double]$_.ArrivalDamageAppliedTotal })
        ArrivalFizzleNoTargetTotalMedian = Get-Median @($gateRows | ForEach-Object { [double]$_.ArrivalFizzleNoTargetTotal })
        DroppedTotalMax = ($gateRows | Measure-Object -Property DroppedTotal -Maximum).Maximum
    }
}

$summary = [pscustomobject]@{
    Tool = "RunOutgoingTravelerPhase1Gate"
    StagedExe = $exeItem.FullName
    StagedExeLastWriteTime = $exeItem.LastWriteTime.ToString("o")
    StagedExeSha256 = $exeHash
    OutputRoot = $outputRootPath
    Resolution = "{0}x{1}" -f $ResX, $ResY
    HeroHPOverride = $HeroHPOverride
    Rows = $rows
    Medians = $medianRows
}

$summaryPath = Join-Path $outputRootPath "phase1_gate_summary.json"
$summary | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $summaryPath -Encoding UTF8

Write-Host "Phase 1 outgoing traveler gate root: $outputRootPath"
Write-Host "Summary: $summaryPath"
if ($medianRows) {
    $medianRows | Format-Table Gate, Runs, Count, FpsMedian, GameThreadMsMedian, GpuFrameMsMedian, DrawCallsMedian, PoolSimulationMsMedian, PoolTargetSnapshotMsMedian, PoolArrivalCollisionMsMedian, PoolPeakLiveCountMax, ArrivalDamageAppliedTotalMedian
}
