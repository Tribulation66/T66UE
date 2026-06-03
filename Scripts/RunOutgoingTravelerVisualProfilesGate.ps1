param(
    [string]$StagedExe = "C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe",
    [string]$Map = "/Game/Maps/GameplayLevel",
    [string]$OutputRoot,
    [int]$Runs = 3,
    [int]$Count = 5000,
    [int]$ResX = 1920,
    [int]$ResY = 1080,
    [int]$HeroHPOverride = 20000,
    [int]$SpawnDistance = 900,
    [int]$Spacing = 10,
    [int]$GridColumns = 0,
    [double]$VisualScaleMultiplier = 1.0,
    [switch]$ProofCamera,
    [switch]$HideHeroForProof,
    [double]$ProofCameraDistance = 1800.0,
    [double]$ProofCameraFOV = 45.0,
    [switch]$DisableFixedProofLane,
    [double]$ProofLaneCenterX = 20000.0,
    [double]$ProofLaneCenterY = 0.0,
    [double]$ProofLaneCenterZ = 9000.0,
    [double]$ProofLaneYaw = 0.0,
    [double]$ProofLanePitch = -90.0,
    [double]$ScreenshotDelaySeconds = 38.0,
    [double]$PostCaptureScreenshotDelaySeconds = 2.0,
    [double]$SampleSeconds = 8.0,
    [int]$ScreenshotSequenceCount = 5,
    [double]$ScreenshotSequenceIntervalSeconds = 2.0,
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

$repoRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot ".."))
if (-not $OutputRoot) {
    $timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
    $OutputRoot = Join-Path $repoRoot "Saved\VideoCaptures\OutgoingTravelerVisualProfiles_$timestamp"
}
$outputRootPath = [System.IO.Path]::GetFullPath($OutputRoot)
New-Item -ItemType Directory -Force -Path $outputRootPath | Out-Null

if (-not (Test-Path -LiteralPath $StagedExe)) {
    throw "Missing staged executable: $StagedExe"
}

$exeItem = Get-Item -LiteralPath $StagedExe
$exeHash = (Get-FileHash -LiteralPath $StagedExe -Algorithm SHA256).Hash
$workingDirectory = Split-Path -Parent $exeItem.FullName
$execCmds = "T66.Camera.GameplayPreset 1,T66.Camera.LockedChasePitch -18,T66.Camera.LockedChaseArmLength 2400,T66.Camera.LockedChasePivotHeight 520,T66.Camera.ConstrainAgainstTowerWalls 0,T66.Camera.WallOcclusionEnabled 0,t.MaxFPS 0,r.VSync 0"
$rows = New-Object System.Collections.Generic.List[object]

$cases = @(
    [pscustomobject]@{ Name = "single_shape"; MixedProfiles = 0; ExpectedVisualProfiles = 0 },
    [pscustomobject]@{ Name = "mixed_16_profiles"; MixedProfiles = 1; ExpectedVisualProfiles = 16 }
)

foreach ($case in $cases) {
    for ($runIndex = 1; $runIndex -le $Runs; ++$runIndex) {
        $caseName = "{0}_run{1}" -f $case.Name, $runIndex
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
            "-T66GameplayAutoScreenshotSequenceCount=$ScreenshotSequenceCount",
            "-T66GameplayAutoScreenshotSequenceInterval=$ScreenshotSequenceIntervalSeconds",
            "-T66GameplayAutoScreenshotDelay=$ScreenshotDelaySeconds",
            "-T66GameplayAutoPostCaptureScreenshotDelay=$PostCaptureScreenshotDelaySeconds",
            "-T66OutgoingTravelerStressCount=$Count",
            "-T66OutgoingTravelerStressUsePool=1",
            "-T66OutgoingTravelerStressManifest=$manifestPath",
            "-T66OutgoingTravelerPoolProofManifest=$poolManifestPath",
            "-T66OutgoingTravelerStressWarmupSeconds=0.75",
            "-T66OutgoingTravelerStressSampleSeconds=$SampleSeconds",
            "-T66OutgoingTravelerStressSpacing=$Spacing",
            "-T66OutgoingTravelerStressSpawnDistance=$SpawnDistance",
            "-T66OutgoingTravelerStressTravelerSpeed=1",
            "-T66OutgoingTravelerStressVisualScaleMultiplier=$VisualScaleMultiplier",
            "-T66OutgoingTravelerStressProofCamera=$([int]$ProofCamera.IsPresent)",
            "-T66OutgoingTravelerStressHideHeroForProof=$([int]$HideHeroForProof.IsPresent)",
            "-T66OutgoingTravelerStressFixedProofLane=$([int](-not $DisableFixedProofLane.IsPresent))",
            "-T66OutgoingTravelerStressProofCameraDistance=$ProofCameraDistance",
            "-T66OutgoingTravelerStressProofCameraFOV=$ProofCameraFOV",
            "-T66OutgoingTravelerStressProofLaneCenterX=$ProofLaneCenterX",
            "-T66OutgoingTravelerStressProofLaneCenterY=$ProofLaneCenterY",
            "-T66OutgoingTravelerStressProofLaneCenterZ=$ProofLaneCenterZ",
            "-T66OutgoingTravelerStressProofLaneYaw=$ProofLaneYaw",
            "-T66OutgoingTravelerStressProofLanePitch=$ProofLanePitch",
            "-T66OutgoingTravelerStressDisableCollision=1",
            "-T66OutgoingTravelerStressTargetSnapshot=0",
            "-T66OutgoingTravelerStressArrivalCollision=0",
            "-T66OutgoingTravelerStressTargetCount=0",
            "-T66OutgoingTravelerStressMixedVisualProfiles=$($case.MixedProfiles)",
            "-T66OutgoingTravelerStressVisualProfileFamily=all",
            "-T66AutoCaptureHeroHPOverride=$HeroHPOverride",
            "-ExecCmds=$execCmds",
            "-abslog=$logPath",
            "-forcelogflush",
            "-unattended",
            "-nosplash"
        )
        if ($GridColumns -gt 0) {
            $argsList += "-T66OutgoingTravelerStressGridColumns=$GridColumns"
        }

        Write-Host "=== Outgoing traveler visual profile gate: $($case.Name) run=$runIndex count=$Count ==="
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
            throw "Timed out waiting for visual profile gate '$($case.Name)' run $runIndex after $TimeoutSeconds seconds."
        }
        if ($process.ExitCode -ne 0) {
            throw "Visual profile gate '$($case.Name)' run $runIndex exited with code $($process.ExitCode). Log: $logPath"
        }
        if (Test-Path -LiteralPath $logPath) {
            $blockingLogPatterns = @(
                "NS_OutgoingTravelerPool.*exceeded the maximum particle count",
                "M_OutgoingTravelerPool.*invalid ShaderMap",
                "M_OutgoingTravelerPool.*Failed to compile",
                "NS_OutgoingTravelerPool.*IsReadyToRunInternal"
            )
            $blockingLogHits = Select-String -Path $logPath -Pattern $blockingLogPatterns -ErrorAction SilentlyContinue
            if ($blockingLogHits) {
                $firstHit = $blockingLogHits | Select-Object -First 1
                throw "Visual profile gate '$($case.Name)' run $runIndex found outgoing traveler runtime warning/error: $($firstHit.Line.Trim()). Log: $logPath"
            }
        }
        if (-not (Test-Path -LiteralPath $manifestPath)) {
            throw "Missing stress manifest for '$($case.Name)' run $runIndex`: $manifestPath"
        }
        $frames = @(Get-ChildItem -LiteralPath $frameDir -Filter "frame_*.png" -File -ErrorAction SilentlyContinue)
        if ($frames.Count -lt 1) {
            throw "Missing full-resolution screenshot frame for '$($case.Name)' run $runIndex`: $frameDir"
        }

        $json = Get-Content -LiteralPath $manifestPath -Raw | ConvertFrom-Json
        $pool = $json.pool_diagnostics
        $visualProfilesUsedCount = [int]$json.visual_profiles_used_count
        $failedSpawnCount = [int]$json.failed_spawn_count
        $droppedTotal = [int]$pool.dropped_total
        $peakLive = [int]$pool.peak_live_count
        if ($failedSpawnCount -ne 0) {
            throw "Visual profile gate '$($case.Name)' run $runIndex had failed spawns: $failedSpawnCount"
        }
        if ($droppedTotal -ne 0) {
            throw "Visual profile gate '$($case.Name)' run $runIndex dropped travelers: $droppedTotal"
        }
        if ($peakLive -lt $Count) {
            throw "Visual profile gate '$($case.Name)' run $runIndex peak live $peakLive below requested $Count"
        }
        if ($case.ExpectedVisualProfiles -gt 0 -and $visualProfilesUsedCount -ne $case.ExpectedVisualProfiles) {
            throw "Visual profile gate '$($case.Name)' run $runIndex used $visualProfilesUsedCount profiles, expected $($case.ExpectedVisualProfiles)"
        }

        $rows.Add([pscustomobject]@{
            Case = $case.Name
            Run = $runIndex
            Count = $Count
            Manifest = $manifestPath
            PoolManifest = $poolManifestPath
            Log = $logPath
            Frame = $frames[0].FullName
            FpsAvg = Get-MetricAverage $json "fps"
            FrameMsAvg = Get-MetricAverage $json "frame_ms"
            GameThreadMsAvg = Get-MetricAverage $json "game_thread_ms"
            GpuFrameMsAvg = Get-MetricAverage $json "gpu_frame_ms"
            DrawCallsAvg = Get-MetricAverage $json "draw_calls"
            PoolUploadMsAvg = Get-MetricAverage $json "pool_last_upload_ms_sampled"
            PoolPackMsAvg = Get-MetricAverage $json "pool_last_pack_ms_sampled"
            PoolNiagaraArrayUploadMsAvg = Get-MetricAverage $json "pool_last_niagara_array_upload_ms_sampled"
            PoolSimulationMsAvg = Get-MetricAverage $json "pool_last_simulation_ms_sampled"
            VisualProfilesUsedCount = $visualProfilesUsedCount
            UsesSingleNiagaraSystemVisualSelector = [bool]$json.uses_single_niagara_system_visual_selector
            PoolPeakLiveCount = $peakLive
            DroppedTotal = $droppedTotal
        }) | Out-Null
    }
}

$medianRows = foreach ($caseName in ($rows | Select-Object -ExpandProperty Case -Unique)) {
    $caseRows = @($rows | Where-Object { $_.Case -eq $caseName })
    [pscustomobject]@{
        Case = $caseName
        Runs = $caseRows.Count
        Count = $Count
        FpsMedian = Get-Median @($caseRows | ForEach-Object { $_.FpsAvg })
        FrameMsMedian = Get-Median @($caseRows | ForEach-Object { $_.FrameMsAvg })
        GameThreadMsMedian = Get-Median @($caseRows | ForEach-Object { $_.GameThreadMsAvg })
        GpuFrameMsMedian = Get-Median @($caseRows | ForEach-Object { $_.GpuFrameMsAvg })
        DrawCallsMedian = Get-Median @($caseRows | ForEach-Object { $_.DrawCallsAvg })
        PoolUploadMsMedian = Get-Median @($caseRows | ForEach-Object { $_.PoolUploadMsAvg })
        PoolPackMsMedian = Get-Median @($caseRows | ForEach-Object { $_.PoolPackMsAvg })
        PoolNiagaraArrayUploadMsMedian = Get-Median @($caseRows | ForEach-Object { $_.PoolNiagaraArrayUploadMsAvg })
        PoolSimulationMsMedian = Get-Median @($caseRows | ForEach-Object { $_.PoolSimulationMsAvg })
        PoolPeakLiveCountMax = ($caseRows | Measure-Object -Property PoolPeakLiveCount -Maximum).Maximum
        VisualProfilesUsedCountMax = ($caseRows | Measure-Object -Property VisualProfilesUsedCount -Maximum).Maximum
        DroppedTotalMax = ($caseRows | Measure-Object -Property DroppedTotal -Maximum).Maximum
    }
}

$summary = [pscustomobject]@{
    Tool = "RunOutgoingTravelerVisualProfilesGate"
    StagedExe = $exeItem.FullName
    StagedExeLastWriteTime = $exeItem.LastWriteTime.ToString("o")
    StagedExeSha256 = $exeHash
    OutputRoot = $outputRootPath
    Resolution = "{0}x{1}" -f $ResX, $ResY
    HeroHPOverride = $HeroHPOverride
    ProofCamera = [bool]$ProofCamera.IsPresent
    HideHeroForProof = [bool]$HideHeroForProof.IsPresent
    FixedProofLane = -not $DisableFixedProofLane.IsPresent
    GridColumns = $GridColumns
    SpawnDistance = $SpawnDistance
    Spacing = $Spacing
    VisualScaleMultiplier = $VisualScaleMultiplier
    ProofCameraDistance = $ProofCameraDistance
    ProofCameraFOV = $ProofCameraFOV
    ProofLaneCenter = @{
        X = $ProofLaneCenterX
        Y = $ProofLaneCenterY
        Z = $ProofLaneCenterZ
    }
    ProofLaneYaw = $ProofLaneYaw
    ProofLanePitch = $ProofLanePitch
    Rows = $rows
    Medians = $medianRows
}

$summaryPath = Join-Path $outputRootPath "visual_profiles_gate_summary.json"
$summary | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $summaryPath -Encoding UTF8

Write-Host "Outgoing traveler visual profile gate root: $outputRootPath"
Write-Host "Summary: $summaryPath"
if ($medianRows) {
    $medianRows | Format-Table Case, Runs, Count, FpsMedian, GameThreadMsMedian, GpuFrameMsMedian, DrawCallsMedian, PoolUploadMsMedian, PoolNiagaraArrayUploadMsMedian, PoolSimulationMsMedian, PoolPeakLiveCountMax, VisualProfilesUsedCountMax
}
