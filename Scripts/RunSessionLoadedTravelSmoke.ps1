param(
    [string]$Exe = "C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe",
    [string]$OutputRoot,
    [int]$SlotIndex = 8,
    [string]$Marker,
    [int]$ResX = 1280,
    [int]$ResY = 720,
    [int]$TimeoutSeconds = 90,
    [switch]$PrintOnly
)

$ErrorActionPreference = "Stop"

$ProjectRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot "..")).TrimEnd('\')

if (-not $OutputRoot) {
    $stamp = Get-Date -Format "yyyyMMdd_HHmmss"
    $OutputRoot = Join-Path $ProjectRoot "Saved\SessionLoadedTravelSmoke\$stamp"
}
$OutputRoot = [System.IO.Path]::GetFullPath($OutputRoot)

if (-not $Marker) {
    $Marker = "SessionLoadedTravel_$((Get-Date).ToString('yyyyMMdd_HHmmss'))"
}

$SlotFileName = "T66_Slot_{0:D2}.sav" -f $SlotIndex
$IndexFileName = "T66_SaveIndex.sav"
$SeedLogPath = Join-Path $OutputRoot "seed.log"
$PlanLogPath = Join-Path $OutputRoot "travel_plan.log"
$BackupManifestPath = Join-Path $OutputRoot "save_backup_manifest.json"
$SummaryJsonPath = Join-Path $OutputRoot "summary.json"
$SummaryMdPath = Join-Path $OutputRoot "summary.md"
$SeedCommand = "T66.Session.QueueLoadedTravelSeed $SlotIndex $Marker CONFIRM 0"
$PlanCommand = "T66.Session.VerifyLoadedTravelPlan $SlotIndex $Marker CONFIRM 0"

function Assert-Condition {
    param(
        [bool]$Condition,
        [string]$Message
    )

    if (-not $Condition) {
        throw $Message
    }
}

function Get-FileHashString {
    param([string]$Path)

    if (-not (Test-Path -LiteralPath $Path)) {
        return $null
    }

    return (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash
}

function Get-DurableSaveRoots {
    $roots = New-Object System.Collections.Generic.List[string]

    $exeFullPath = [System.IO.Path]::GetFullPath($Exe)
    $exeDir = [System.IO.Path]::GetDirectoryName($exeFullPath)
    if ($exeDir) {
        $exeDirLeaf = Split-Path -Leaf $exeDir
        if ($exeDirLeaf -ne "Win64") {
            $topLevelGameRoot = Join-Path $exeDir ([System.IO.Path]::GetFileNameWithoutExtension($exeFullPath))
            $roots.Add((Join-Path $topLevelGameRoot "Saved\SaveGames"))
        }

        $binariesDir = Split-Path -Parent $exeDir
        if ($exeDirLeaf -eq "Win64" -and $binariesDir -and (Split-Path -Leaf $binariesDir) -eq "Binaries") {
            $gameRoot = Split-Path -Parent $binariesDir
            if ($gameRoot) {
                $roots.Add((Join-Path $gameRoot "Saved\SaveGames"))
            }
        }
    }

    $roots.Add((Join-Path $ProjectRoot "Saved\StagedBuilds\Windows\T66\Saved\SaveGames"))
    $roots.Add((Join-Path $ProjectRoot "Saved\SaveGames"))

    if ($env:LOCALAPPDATA) {
        $roots.Add((Join-Path $env:LOCALAPPDATA "T66\Saved\SaveGames"))
    }

    $seen = @{}
    foreach ($root in $roots) {
        if (-not $root) {
            continue
        }

        $full = [System.IO.Path]::GetFullPath($root)
        $key = $full.ToLowerInvariant()
        if (-not $seen.ContainsKey($key)) {
            $seen[$key] = $true
            $full
        }
    }
}

function New-SaveSnapshot {
    param([string[]]$SaveRoots)

    $backupRoot = Join-Path $OutputRoot "save_backups"
    New-Item -ItemType Directory -Force -Path $backupRoot | Out-Null

    $rows = New-Object System.Collections.Generic.List[object]
    $rootIndex = 0
    foreach ($root in $SaveRoots) {
        $rootIndex += 1
        $rootBackup = Join-Path $backupRoot ("root_{0:D2}" -f $rootIndex)
        New-Item -ItemType Directory -Force -Path $rootBackup | Out-Null

        foreach ($fileName in @($SlotFileName, $IndexFileName)) {
            $target = Join-Path $root $fileName
            $backup = Join-Path $rootBackup $fileName
            $existed = Test-Path -LiteralPath $target
            $beforeHash = Get-FileHashString -Path $target

            if ($existed) {
                Copy-Item -LiteralPath $target -Destination $backup -Force
            }

            $rows.Add([pscustomobject]@{
                Root = $root
                FileName = $fileName
                TargetPath = $target
                BackupPath = $backup
                ExistedBefore = $existed
                BeforeHash = $beforeHash
                Restored = $false
                RemovedCreatedFile = $false
                AfterRestoreHash = $null
            })
        }
    }

    $rows | ConvertTo-Json -Depth 4 | Set-Content -LiteralPath $BackupManifestPath -Encoding UTF8
    return $rows
}

function Restore-SaveSnapshot {
    param([System.Collections.Generic.List[object]]$SnapshotRows)

    $restoreRows = New-Object System.Collections.Generic.List[object]
    if (-not $SnapshotRows) {
        return $restoreRows
    }

    foreach ($row in $SnapshotRows) {
        if ($row.ExistedBefore) {
            $targetDir = Split-Path -Parent $row.TargetPath
            New-Item -ItemType Directory -Force -Path $targetDir | Out-Null
            Copy-Item -LiteralPath $row.BackupPath -Destination $row.TargetPath -Force
            $row.Restored = $true
            $row.AfterRestoreHash = Get-FileHashString -Path $row.TargetPath
        } elseif (Test-Path -LiteralPath $row.TargetPath) {
            Remove-Item -LiteralPath $row.TargetPath -Force
            $row.RemovedCreatedFile = $true
        }

        $restoreRows.Add([pscustomobject]@{
            TargetPath = $row.TargetPath
            ExistedBefore = $row.ExistedBefore
            Restored = [bool]$row.Restored
            RemovedCreatedFile = [bool]$row.RemovedCreatedFile
            BeforeHash = $row.BeforeHash
            AfterRestoreHash = $row.AfterRestoreHash
        })
    }

    $SnapshotRows | ConvertTo-Json -Depth 4 | Set-Content -LiteralPath $BackupManifestPath -Encoding UTF8
    return $restoreRows
}

function Sync-SeededSaveFilesToAllRoots {
    param(
        [string[]]$SaveRoots,
        [System.Collections.Generic.List[object]]$SnapshotRows
    )

    $sourceRoot = $null
    $sourceSlot = $null
    $sourceIndex = $null

    foreach ($root in $SaveRoots) {
        $slotPath = Join-Path $root $SlotFileName
        $indexPath = Join-Path $root $IndexFileName
        if (-not (Test-Path -LiteralPath $slotPath) -or -not (Test-Path -LiteralPath $indexPath)) {
            continue
        }

        $slotRow = $SnapshotRows | Where-Object { $_.TargetPath -eq $slotPath } | Select-Object -First 1
        $indexRow = $SnapshotRows | Where-Object { $_.TargetPath -eq $indexPath } | Select-Object -First 1
        $slotHash = Get-FileHashString -Path $slotPath
        $indexHash = Get-FileHashString -Path $indexPath
        $slotChanged = (-not $slotRow) -or ($slotHash -ne $slotRow.BeforeHash)
        $indexChanged = (-not $indexRow) -or ($indexHash -ne $indexRow.BeforeHash)

        if ($slotChanged -or $indexChanged) {
            $sourceRoot = $root
            $sourceSlot = $slotPath
            $sourceIndex = $indexPath
            break
        }
    }

    if (-not $sourceRoot) {
        throw "Seed phase passed but no changed seeded save root was found for $SlotFileName / $IndexFileName."
    }

    $syncRows = New-Object System.Collections.Generic.List[object]
    foreach ($root in $SaveRoots) {
        New-Item -ItemType Directory -Force -Path $root | Out-Null

        $targetSlot = Join-Path $root $SlotFileName
        $targetIndex = Join-Path $root $IndexFileName
        $copiedSlot = $false
        $copiedIndex = $false

        if ($targetSlot -ne $sourceSlot) {
            Copy-Item -LiteralPath $sourceSlot -Destination $targetSlot -Force
            $copiedSlot = $true
        }
        if ($targetIndex -ne $sourceIndex) {
            Copy-Item -LiteralPath $sourceIndex -Destination $targetIndex -Force
            $copiedIndex = $true
        }

        $syncRows.Add([pscustomobject]@{
            SourceRoot = $sourceRoot
            TargetRoot = $root
            SlotPath = $targetSlot
            IndexPath = $targetIndex
            CopiedSlot = $copiedSlot
            CopiedIndex = $copiedIndex
            SlotHash = Get-FileHashString -Path $targetSlot
            IndexHash = Get-FileHashString -Path $targetIndex
        })
    }

    return $syncRows
}

function ConvertTo-SmokeText {
    param([AllowNull()][object]$Value)

    if ($null -eq $Value) {
        return ""
    }

    if ($Value -is [System.Array]) {
        return (($Value | ForEach-Object { [string]$_ }) -join "`n")
    }

    return [string]$Value
}

function Read-LogText {
    param([string]$Path)

    if (-not (Test-Path -LiteralPath $Path)) {
        return ""
    }

    try {
        return ConvertTo-SmokeText -Value (Get-Content -LiteralPath $Path -Raw)
    } catch {
        return ""
    }
}

function Test-SmokeTextContains {
    param(
        [AllowNull()][object]$Text,
        [string]$Marker
    )

    $normalizedText = ConvertTo-SmokeText -Value $Text
    return ($normalizedText.IndexOf($Marker, [System.StringComparison]::OrdinalIgnoreCase) -ge 0)
}

function Assert-LogContains {
    param(
        [AllowNull()][object]$LogText,
        [string]$Marker,
        [string]$Path,
        [string]$Phase
    )

    if (-not (Test-SmokeTextContains -Text $LogText -Marker $Marker)) {
        throw "$Phase log missing expected marker '$Marker'. Log: $Path"
    }
}

function Assert-LogDoesNotContain {
    param(
        [AllowNull()][object]$LogText,
        [string]$Marker,
        [string]$Path,
        [string]$Phase
    )

    if (Test-SmokeTextContains -Text $LogText -Marker $Marker) {
        throw "$Phase log contained forbidden marker '$Marker'. Log: $Path"
    }
}

function Invoke-SessionCommandPhase {
    param(
        [string]$Name,
        [string]$Command,
        [string]$LogPath,
        [string[]]$RequiredMarkers,
        [string[]]$ForbiddenMarkers
    )

    if (Test-Path -LiteralPath $LogPath) {
        Remove-Item -LiteralPath $LogPath -Force
    }

    $argsList = @(
        "-ExecCmds=`"$Command`"",
        "-abslog=`"$LogPath`"",
        "-forcelogflush",
        "-nop4",
        "-nosplash",
        "-unattended",
        "-windowed",
        "-ResX=$ResX",
        "-ResY=$ResY"
    )

    Write-Host "$Name phase: $Exe $($argsList -join ' ')"

    if ($PrintOnly) {
        return [pscustomobject]@{
            Name = $Name
            Status = "PRINT_ONLY"
            Command = $Command
            Log = $LogPath
            RequiredMarkers = $RequiredMarkers
            ForbiddenMarkers = $ForbiddenMarkers
        }
    }

    $process = Start-Process -FilePath $Exe -ArgumentList $argsList -WorkingDirectory $ProjectRoot -PassThru
    $deadline = (Get-Date).AddSeconds($TimeoutSeconds)
    while (-not $process.HasExited -and (Get-Date) -lt $deadline) {
        Start-Sleep -Milliseconds 500
    }

    if (-not $process.HasExited) {
        Stop-Process -Id $process.Id -Force -ErrorAction SilentlyContinue
        throw "$Name phase timed out after $TimeoutSeconds seconds. Log: $LogPath"
    }

    if ($process.ExitCode -ne 0) {
        throw "$Name phase exited with code $($process.ExitCode). Expected 0. Log: $LogPath"
    }

    $logText = Read-LogText -Path $LogPath
    foreach ($forbidden in $ForbiddenMarkers) {
        Assert-LogDoesNotContain -LogText $logText -Marker $forbidden -Path $LogPath -Phase $Name
    }
    foreach ($required in $RequiredMarkers) {
        Assert-LogContains -LogText $logText -Marker $required -Path $LogPath -Phase $Name
    }

    return [pscustomobject]@{
        Name = $Name
        Status = "PASS"
        Command = $Command
        Log = $LogPath
        ExitCode = $process.ExitCode
        RequiredMarkers = $RequiredMarkers
        ForbiddenMarkers = $ForbiddenMarkers
    }
}

function Write-SmokeSummary {
    param(
        [string]$Status,
        [string]$ErrorMessage,
        [object[]]$PhaseResults,
        [object[]]$SaveRoots,
        [object[]]$SeedSyncRows,
        [object[]]$RestoreRows
    )

    $summary = [pscustomobject]@{
        Status = $Status
        CreatedAt = (Get-Date).ToString("o")
        Exe = [System.IO.Path]::GetFullPath($Exe)
        OutputRoot = $OutputRoot
        SlotIndex = $SlotIndex
        SlotFileName = $SlotFileName
        Marker = $Marker
        ResX = $ResX
        ResY = $ResY
        TimeoutSeconds = $TimeoutSeconds
        SeedCommand = $SeedCommand
        PlanCommand = $PlanCommand
        SeedLog = $SeedLogPath
        PlanLog = $PlanLogPath
        BackupManifest = $BackupManifestPath
        SaveRoots = $SaveRoots
        Phases = $PhaseResults
        SeedSync = $SeedSyncRows
        Restore = $RestoreRows
        Error = $ErrorMessage
        Limitation = "Plan-level standalone proof only. It validates session-owned loaded-save apply and listen travel URL calculation, but intentionally skips live ServerTravel and does not prove a joined remote client."
    }

    $summary | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $SummaryJsonPath -Encoding UTF8

    $md = @()
    $md += "# Session Loaded-Travel Smoke"
    $md += ""
    $md += "Status: $Status"
    $md += "Executable: $($summary.Exe)"
    $md += "Slot: $SlotIndex ($SlotFileName)"
    $md += "Marker: $Marker"
    $md += "Seed log: $SeedLogPath"
    $md += "Travel-plan log: $PlanLogPath"
    $md += "Backup manifest: $BackupManifestPath"
    if ($ErrorMessage) {
        $md += "Error: $ErrorMessage"
    }
    $md += ""
    foreach ($phase in $PhaseResults) {
        $md += "## $($phase.Name)"
        $md += ""
        $md += "- Status: $($phase.Status)"
        $md += "- Log: $($phase.Log)"
        foreach ($markerText in $phase.RequiredMarkers) {
            $md += "- Required marker: ``$markerText``"
        }
        $md += ""
    }
    foreach ($row in $SeedSyncRows) {
        $md += "- Seed synced: $($row.TargetRoot) CopiedSlot=$($row.CopiedSlot) CopiedIndex=$($row.CopiedIndex)"
    }
    foreach ($row in $RestoreRows) {
        if ($row.ExistedBefore) {
            $md += "- Restored: $($row.TargetPath)"
        } elseif ($row.RemovedCreatedFile) {
            $md += "- Removed proof-created file: $($row.TargetPath)"
        }
    }
    $md += ""
    $md += "This gate seeds a protected Duo save through the session-owned non-shipping harness, reloads it in a fresh process, applies the loaded-run state through ``UT66SessionSubsystem``, verifies the computed ``?listen`` gameplay travel plan, and restores protected save/index files afterward."
    $md += ""
    $md += "Limitation: this is a plan-level standalone proof. It intentionally logs ``LiveTravelSkipped=1`` and does not prove a remote client joins after ``ServerTravel``."
    $md -join "`r`n" | Set-Content -LiteralPath $SummaryMdPath -Encoding UTF8

    Write-Host "Session loaded-travel smoke $Status`: $OutputRoot"
    Write-Host "Summary: $SummaryJsonPath"
}

if ($SlotIndex -lt 0 -or $SlotIndex -gt 8) {
    throw "SlotIndex must be between 0 and 8."
}
if ($Marker -notmatch '^[A-Za-z0-9_.-]+$') {
    throw "Marker may only contain letters, numbers, underscore, hyphen, or dot. Current marker: '$Marker'"
}
if ($TimeoutSeconds -lt 10 -or $TimeoutSeconds -gt 600) {
    throw "TimeoutSeconds must be between 10 and 600."
}
if (-not (Test-Path -LiteralPath $Exe)) {
    throw "Missing executable: $Exe"
}

New-Item -ItemType Directory -Force -Path $OutputRoot | Out-Null
foreach ($path in @($SeedLogPath, $PlanLogPath, $BackupManifestPath, $SummaryJsonPath, $SummaryMdPath)) {
    if (Test-Path -LiteralPath $path) {
        Remove-Item -LiteralPath $path -Force
    }
}

$seedRequiredMarkers = @(
    "[SessionLoadedTravelSeed] PASS Slot=$SlotIndex Marker=$Marker",
    "LoadedOk=1",
    "MetaOk=1",
    "PartyShapeOk=1",
    "SnapshotOk=1"
)
$planRequiredMarkers = @(
    "[SessionLoadedTravel] PASS Slot=$SlotIndex Marker=$Marker",
    "SlotExists=1",
    "LoadedOk=1",
    "MetaOk=1",
    "OwnerOk=1",
    "PartyShapeOk=1",
    "SnapshotOk=1",
    "ApplyPlanOk=1",
    "TravelPlanOk=1",
    "?listen",
    "LiveTravelSkipped=1",
    "RequiresActiveSession=1",
    "RequiresHost=1"
)
$commonForbiddenMarkers = @(
    "Usage: T66.Session.",
    "Command failed: no world",
    "Command failed: no session subsystem",
    "Reason=Missing"
)
$seedForbiddenMarkers = @("[SessionLoadedTravelSeed] FAIL") + $commonForbiddenMarkers
$planForbiddenMarkers = @("[SessionLoadedTravel] FAIL") + $commonForbiddenMarkers

$saveRoots = @(Get-DurableSaveRoots)
$phaseResults = New-Object System.Collections.Generic.List[object]
$snapshotRows = $null
$restoreRows = New-Object System.Collections.Generic.List[object]
$seedSyncRows = New-Object System.Collections.Generic.List[object]
$status = "UNKNOWN"
$errorMessage = $null

try {
    Write-Host "Session loaded-travel smoke uses a protected Duo save seed and session-owned travel-plan verification."
    Write-Host "Save roots protected:"
    foreach ($root in $saveRoots) {
        Write-Host "  $root"
    }
    Write-Host "Selected slot $SlotIndex ($SlotFileName) will be written by the proof, then restored from the save snapshot."
    Write-Host "Seed command: $SeedCommand"
    Write-Host "Plan command: $PlanCommand"

    if ($PrintOnly) {
        $status = "PRINT_ONLY"
        $phaseResults.Add((Invoke-SessionCommandPhase -Name "seed" -Command $SeedCommand -LogPath $SeedLogPath -RequiredMarkers $seedRequiredMarkers -ForbiddenMarkers $seedForbiddenMarkers))
        $phaseResults.Add((Invoke-SessionCommandPhase -Name "travel_plan" -Command $PlanCommand -LogPath $PlanLogPath -RequiredMarkers $planRequiredMarkers -ForbiddenMarkers $planForbiddenMarkers))
    } else {
        $snapshotRows = New-SaveSnapshot -SaveRoots $saveRoots
        $phaseResults.Add((Invoke-SessionCommandPhase -Name "seed" -Command $SeedCommand -LogPath $SeedLogPath -RequiredMarkers $seedRequiredMarkers -ForbiddenMarkers $seedForbiddenMarkers))
        Assert-Condition -Condition (Test-Path -LiteralPath $Exe) -Message "Staged executable disappeared after seed phase: $Exe"
        $seedSyncRows = Sync-SeededSaveFilesToAllRoots -SaveRoots $saveRoots -SnapshotRows $snapshotRows
        $phaseResults.Add((Invoke-SessionCommandPhase -Name "travel_plan" -Command $PlanCommand -LogPath $PlanLogPath -RequiredMarkers $planRequiredMarkers -ForbiddenMarkers $planForbiddenMarkers))
        $status = "PASS"
    }
} catch {
    $status = "FAIL"
    $errorMessage = $_.Exception.Message
} finally {
    if (-not $PrintOnly -and $snapshotRows) {
        try {
            $restoreRows = Restore-SaveSnapshot -SnapshotRows $snapshotRows
        } catch {
            $status = "FAIL"
            $restoreError = $_.Exception.Message
            if ($errorMessage) {
                $errorMessage = "$errorMessage RestoreError=$restoreError"
            } else {
                $errorMessage = "RestoreError=$restoreError"
            }
        }
    }

    Write-SmokeSummary -Status $status -ErrorMessage $errorMessage -PhaseResults $phaseResults.ToArray() -SaveRoots $saveRoots -SeedSyncRows $seedSyncRows -RestoreRows $restoreRows
}

if ($status -eq "FAIL") {
    throw $errorMessage
}
