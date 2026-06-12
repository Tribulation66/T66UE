param(
    [string]$Exe = "C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe",
    [string]$OutputRoot,
    [int]$Travels = 6,
    [double]$DelaySeconds = 0.75,
    [int]$StressCount = 6,
    [double]$StressSettleSeconds = 0.5,
    [int]$TimeoutSeconds = 180,
    [switch]$NoStress,
    [switch]$PrintOnly
)

$ErrorActionPreference = "Stop"

$ProjectRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot "..")).TrimEnd('\')

if (-not $OutputRoot) {
    $stamp = Get-Date -Format "yyyyMMdd_HHmmss"
    $OutputRoot = Join-Path $ProjectRoot "Saved\LifecycleTransitionSmokeGate\$stamp"
}
$OutputRoot = [System.IO.Path]::GetFullPath($OutputRoot)

$ExpectedSubsystems = @(
    "UT66MobManagerSubsystem",
    "UT66MobLootSubsystem",
    "UT66ProjectileManagerSubsystem",
    "UT66BossHazardSubsystem",
    "UT66OutgoingTravelerPoolSubsystem",
    "UT66PixelVFXSubsystem"
)

function ConvertTo-ForwardSlashPath {
    param([string]$Path)
    return $Path.Replace('\', '/')
}

function Assert-Condition {
    param(
        [bool]$Condition,
        [string]$Message
    )

    if (-not $Condition) {
        throw $Message
    }
}

function Write-GateSummary {
    param(
        [string]$Status,
        [string]$ErrorMessage,
        [object[]]$ValidationRows,
        [int]$ExitCode
    )

    $summary = [pscustomobject]@{
        Status = $Status
        CreatedAt = (Get-Date).ToString("o")
        Exe = [System.IO.Path]::GetFullPath($Exe)
        OutputRoot = $OutputRoot
        Manifest = $ManifestPath
        Log = $LogPath
        Command = $ProofCommand
        Travels = $Travels
        DelaySeconds = $DelaySeconds
        StressEnabled = -not $NoStress
        StressCount = $StressCount
        StressSettleSeconds = $StressSettleSeconds
        TimeoutSeconds = $TimeoutSeconds
        ExitCode = $ExitCode
        Error = $ErrorMessage
        Validation = $ValidationRows
    }

    $summaryJson = Join-Path $OutputRoot "summary.json"
    $summaryMd = Join-Path $OutputRoot "summary.md"
    $summary | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $summaryJson -Encoding UTF8

    $md = @()
    $md += "# Lifecycle Transition Smoke Gate"
    $md += ""
    $md += "Status: $Status"
    $md += "Executable: $($summary.Exe)"
    $md += "Manifest: $ManifestPath"
    $md += "Log: $LogPath"
    $md += "Command: $ProofCommand"
    if ($ErrorMessage) {
        $md += "Error: $ErrorMessage"
    }
    $md += ""
    foreach ($row in $ValidationRows) {
        $md += "- $($row.Check): $($row.Status) $($row.Detail)"
    }
    $md += ""
    $md += "This gate wraps the development-only `T66.WorldRuntime.ProofTravel` command. It is structural lifecycle leak evidence; it does not replace ShutdownSystem quit proof and it does not approve a new runtime coordinator."
    $md -join "`r`n" | Set-Content -LiteralPath $summaryMd -Encoding UTF8

    Write-Host "Lifecycle transition smoke gate $Status`: $OutputRoot"
    Write-Host "Summary: $summaryJson"
}

function Add-ValidationRow {
    param(
        [System.Collections.Generic.List[object]]$Rows,
        [string]$Check,
        [string]$Detail
    )

    $Rows.Add([pscustomobject]@{
        Check = $Check
        Status = "PASS"
        Detail = $Detail
    })
}

function Test-LifecycleManifest {
    param([pscustomobject]$Manifest)

    $rows = New-Object System.Collections.Generic.List[object]

    Assert-Condition ($Manifest.tool -eq "T66WorldRuntimeProof") "Manifest tool was '$($Manifest.tool)', expected 'T66WorldRuntimeProof'."
    Add-ValidationRow $rows "tool" "T66WorldRuntimeProof"

    Assert-Condition ($Manifest.mode -eq "proof_travel") "Manifest mode was '$($Manifest.mode)', expected 'proof_travel'."
    Add-ValidationRow $rows "mode" "proof_travel"

    Assert-Condition ($Manifest.status -eq "complete") "Manifest status was '$($Manifest.status)', expected 'complete'."
    Add-ValidationRow $rows "status" "complete"

    Assert-Condition ([int]$Manifest.requested_travel_count -eq $Travels) "requested_travel_count was '$($Manifest.requested_travel_count)', expected '$Travels'."
    Assert-Condition ([int]$Manifest.completed_travel_count -eq $Travels) "completed_travel_count was '$($Manifest.completed_travel_count)', expected '$Travels'."
    Add-ValidationRow $rows "travel_count" "$Travels/$Travels completed"

    Assert-Condition ([bool]$Manifest.observer_only) "observer_only was not true."
    Add-ValidationRow $rows "observer_only" "true"

    $snapshots = @($Manifest.snapshots)
    Assert-Condition ($snapshots.Count -gt 0) "Manifest did not contain snapshots."
    Assert-Condition ($snapshots.Count -ge ($Travels + 1)) "Manifest had $($snapshots.Count) snapshots, expected at least $($Travels + 1)."
    Add-ValidationRow $rows "snapshot_count" "$($snapshots.Count) snapshots"

    foreach ($snapshot in $snapshots) {
        $label = if ($snapshot.label) { [string]$snapshot.label } else { "sequence_$($snapshot.sequence)" }
        Assert-Condition ([int]$snapshot.non_current_world_proof_candidate_resource_count -eq 0) "Snapshot '$label' had non_current_world_proof_candidate_resource_count=$($snapshot.non_current_world_proof_candidate_resource_count)."
        Assert-Condition ($snapshot.status -eq "no_non_current_world_resources_observed") "Snapshot '$label' status was '$($snapshot.status)'."

        $candidateSubsystems = @($snapshot.candidate_subsystems)
        foreach ($expected in $ExpectedSubsystems) {
            $match = $candidateSubsystems | Where-Object { $_.system -eq $expected } | Select-Object -First 1
            Assert-Condition ($null -ne $match) "Snapshot '$label' missing candidate subsystem '$expected'."
            Assert-Condition ([bool]$match.present) "Snapshot '$label' candidate subsystem '$expected' was not present."
        }
    }
    Add-ValidationRow $rows "non_current_world_resources" "all snapshots reported 0"
    Add-ValidationRow $rows "candidate_subsystems" "all $($ExpectedSubsystems.Count) expected subsystems present in every snapshot"

    if (-not $NoStress) {
        Assert-Condition ([bool]$Manifest.stress_enabled) "stress_enabled was not true."
        Assert-Condition ([int]$Manifest.stress_count -eq $StressCount) "stress_count was '$($Manifest.stress_count)', expected '$StressCount'."
        Assert-Condition ($null -ne $Manifest.stress_population) "Missing stress_population object."
        Assert-Condition ($Manifest.stress_population.status -eq "complete") "stress_population status was '$($Manifest.stress_population.status)', expected 'complete'."

        # Mob loot is shelve-gated (FT66ShelvedFeatureGate / T66.MobLoot.Enabled); the
        # manifest reports the effective state so the expectation tracks it.
        $mobLootEnabled = $false
        if ($null -ne $Manifest.stress_population.PSObject.Properties['mob_loot_enabled']) {
            $mobLootEnabled = [bool]$Manifest.stress_population.mob_loot_enabled
        }
        $stressChecks = @{
            mobs_spawned = $StressCount
            mob_loot_spawned = $(if ($mobLootEnabled) { $StressCount } else { 0 })
            projectiles_fired = $StressCount
            hazards_spawned = [Math]::Min($StressCount, 8)
            travelers_fired = $StressCount
            pixel_vfx_spawned = $StressCount
        }

        foreach ($key in $stressChecks.Keys) {
            $actual = [int]$Manifest.stress_population.$key
            $expected = [int]$stressChecks[$key]
            Assert-Condition ($actual -eq $expected) "stress_population.$key was '$actual', expected '$expected'."
        }
        Add-ValidationRow $rows "stress_population" "all expected active resource counts populated"
    } else {
        Add-ValidationRow $rows "stress_population" "skipped by -NoStress"
    }

    return $rows.ToArray()
}

if ($Travels -lt 1 -or $Travels -gt 24) {
    throw "Travels must be between 1 and 24."
}
if ($StressCount -lt 1 -or $StressCount -gt 24) {
    throw "StressCount must be between 1 and 24."
}
if ($DelaySeconds -lt 0.05 -or $DelaySeconds -gt 10.0) {
    throw "DelaySeconds must be between 0.05 and 10.0."
}
if ($StressSettleSeconds -lt 0.05 -or $StressSettleSeconds -gt 5.0) {
    throw "StressSettleSeconds must be between 0.05 and 5.0."
}
if (-not (Test-Path -LiteralPath $Exe)) {
    throw "Missing executable: $Exe"
}

New-Item -ItemType Directory -Force -Path $OutputRoot | Out-Null

$ManifestPath = Join-Path $OutputRoot "world_runtime_travel_proof.json"
$LogPath = Join-Path $OutputRoot "run.log"
$ManifestArg = ConvertTo-ForwardSlashPath $ManifestPath
$StressArg = if ($NoStress) { 0 } else { 1 }
$ProofCommand = "T66.WorldRuntime.ProofTravel Path=$ManifestArg Travels=$Travels Delay=$DelaySeconds Stress=$StressArg StressCount=$StressCount StressSettle=$StressSettleSeconds ExitOnComplete=1 ExitCode=0"
$ArgsList = @(
    "-ExecCmds=`"$ProofCommand`"",
    "-abslog=`"$LogPath`"",
    "-forcelogflush",
    "-nop4",
    "-nosplash",
    "-unattended",
    "-windowed",
    "-ResX=1280",
    "-ResY=720"
)

foreach ($path in @($ManifestPath, $LogPath, (Join-Path $OutputRoot "summary.json"), (Join-Path $OutputRoot "summary.md"))) {
    if (Test-Path -LiteralPath $path) {
        Remove-Item -LiteralPath $path -Force
    }
}

Write-Host "Lifecycle transition smoke gate uses development-only T66.WorldRuntime.ProofTravel."
Write-Host "$Exe $($ArgsList -join ' ')"

if ($PrintOnly) {
    Write-GateSummary -Status "PRINT_ONLY" -ErrorMessage $null -ValidationRows @() -ExitCode $null
    return
}

$validationRows = @()
$exitCode = $null
try {
    $process = Start-Process -FilePath $Exe -ArgumentList $ArgsList -WorkingDirectory $ProjectRoot -PassThru
    $deadline = (Get-Date).AddSeconds($TimeoutSeconds)
    while (-not $process.HasExited -and (Get-Date) -lt $deadline) {
        Start-Sleep -Milliseconds 500
    }

    if (-not $process.HasExited) {
        Stop-Process -Id $process.Id -Force -ErrorAction SilentlyContinue
        $missingHint = if (-not (Test-Path -LiteralPath $ManifestPath)) {
            " No manifest was written; this usually means the executable is a Shipping build or T66.WorldRuntime.ProofTravel was not available."
        } else {
            ""
        }
        throw "Timed out waiting for lifecycle transition smoke gate after $TimeoutSeconds seconds.$missingHint Log: $LogPath"
    }

    $exitCode = $process.ExitCode
    if ($exitCode -ne 0) {
        throw "Lifecycle transition smoke gate exited with code $exitCode. Log: $LogPath"
    }

    if (-not (Test-Path -LiteralPath $ManifestPath)) {
        throw "Missing lifecycle transition manifest: $ManifestPath. This gate requires a Development/non-shipping build with T66.WorldRuntime.ProofTravel available."
    }

    $manifest = Get-Content -LiteralPath $ManifestPath -Raw | ConvertFrom-Json
    $validationRows = Test-LifecycleManifest -Manifest $manifest
    Write-GateSummary -Status "PASS" -ErrorMessage $null -ValidationRows $validationRows -ExitCode $exitCode
} catch {
    $message = $_.Exception.Message
    Write-GateSummary -Status "FAIL" -ErrorMessage $message -ValidationRows $validationRows -ExitCode $exitCode
    throw
}
