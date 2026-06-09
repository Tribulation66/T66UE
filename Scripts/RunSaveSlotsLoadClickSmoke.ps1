param(
    [string]$Exe = "C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe",
    [string]$OutputRoot,
    [int]$SlotIndex = 0,
    [string]$Marker,
    [int]$ResX = 1280,
    [int]$ResY = 720,
    [double]$PreClickDumpDelaySeconds = 1.5,
    [double]$ClickDelaySeconds = 2.5,
    [int]$TimeoutSeconds = 90,
    [switch]$PrintOnly
)

$ErrorActionPreference = "Stop"

$ProjectRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot "..")).TrimEnd('\')

if (-not $OutputRoot) {
    $stamp = Get-Date -Format "yyyyMMdd_HHmmss"
    $OutputRoot = Join-Path $ProjectRoot "Saved\SaveSlotsLoadClickSmoke\$stamp"
}
$OutputRoot = [System.IO.Path]::GetFullPath($OutputRoot)

if (-not $Marker) {
    $Marker = "SaveSlotsLoadClick_$((Get-Date).ToString('yyyyMMdd_HHmmss'))"
}

$SlotFileName = "T66_Slot_{0:D2}.sav" -f $SlotIndex
$IndexFileName = "T66_SaveIndex.sav"
$QueueLogPath = Join-Path $OutputRoot "queue_seed.log"
$VerifyReloadLogPath = Join-Path $OutputRoot "verify_reload.log"
$LoadClickLogPath = Join-Path $OutputRoot "load_click.log"
$PreClickDumpPath = Join-Path $OutputRoot "pre_click_saveslots_dump.json"
$BackupManifestPath = Join-Path $OutputRoot "save_backup_manifest.json"
$SummaryJsonPath = Join-Path $OutputRoot "summary.json"
$SummaryMdPath = Join-Path $OutputRoot "summary.md"
$QueueCommand = "T66.Save.QueueIntegrityShutdown $SlotIndex $Marker CONFIRM 0"
$VerifyCommand = "T66.Save.VerifyIntegritySlot $SlotIndex $Marker CONFIRM 0"
$ClickTag = "SaveSlots.Slot$($SlotIndex + 1).LoadButton"

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

        $slotRow = $SnapshotRows | Where-Object {
            $_.TargetPath -eq $slotPath
        } | Select-Object -First 1
        $indexRow = $SnapshotRows | Where-Object {
            $_.TargetPath -eq $indexPath
        } | Select-Object -First 1

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
        $targetDir = $root
        New-Item -ItemType Directory -Force -Path $targetDir | Out-Null

        $targetSlot = Join-Path $targetDir $SlotFileName
        $targetIndex = Join-Path $targetDir $IndexFileName
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
            TargetRoot = $targetDir
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

function Invoke-QueueSeedPhase {
    if (Test-Path -LiteralPath $QueueLogPath) {
        Remove-Item -LiteralPath $QueueLogPath -Force
    }

    $argsList = @(
        "-ExecCmds=`"$QueueCommand`"",
        "-abslog=`"$QueueLogPath`"",
        "-forcelogflush",
        "-nop4",
        "-nosplash",
        "-unattended",
        "-windowed",
        "-ResX=$ResX",
        "-ResY=$ResY"
    )

    Write-Host "Seed phase: $Exe $($argsList -join ' ')"

    if ($PrintOnly) {
        return [pscustomobject]@{
            Name = "queue_seed"
            Status = "PRINT_ONLY"
            Command = $QueueCommand
            Log = $QueueLogPath
            RequiredMarkers = @("[SaveIntegrity] PASS Slot=$SlotIndex Marker=$Marker")
        }
    }

    $process = Start-Process -FilePath $Exe -ArgumentList $argsList -WorkingDirectory $ProjectRoot -PassThru
    $deadline = (Get-Date).AddSeconds($TimeoutSeconds)
    while (-not $process.HasExited -and (Get-Date) -lt $deadline) {
        Start-Sleep -Milliseconds 500
    }

    if (-not $process.HasExited) {
        Stop-Process -Id $process.Id -Force -ErrorAction SilentlyContinue
        throw "Seed phase timed out after $TimeoutSeconds seconds. Log: $QueueLogPath"
    }

    if ($process.ExitCode -ne 0) {
        throw "Seed phase exited with code $($process.ExitCode). Expected 0. Log: $QueueLogPath"
    }

    $logText = Read-LogText -Path $QueueLogPath
    foreach ($forbidden in @("[SaveIntegrity] FAIL", "Usage: T66.Save.", "Command failed: no world", "Command failed: no save subsystem")) {
        Assert-LogDoesNotContain -LogText $logText -Marker $forbidden -Path $QueueLogPath -Phase "queue_seed"
    }
    foreach ($required in @("[SaveIntegrity] PASS Slot=$SlotIndex Marker=$Marker", "LoadedOk=1", "MetaOk=1")) {
        Assert-LogContains -LogText $logText -Marker $required -Path $QueueLogPath -Phase "queue_seed"
    }

    [pscustomobject]@{
        Name = "queue_seed"
        Status = "PASS"
        Command = $QueueCommand
        Log = $QueueLogPath
        ExitCode = $process.ExitCode
        RequiredMarkers = @("[SaveIntegrity] PASS Slot=$SlotIndex Marker=$Marker", "LoadedOk=1", "MetaOk=1")
    }
}

function Invoke-VerifyReloadPhase {
    if (Test-Path -LiteralPath $VerifyReloadLogPath) {
        Remove-Item -LiteralPath $VerifyReloadLogPath -Force
    }

    $argsList = @(
        "-ExecCmds=`"$VerifyCommand`"",
        "-abslog=`"$VerifyReloadLogPath`"",
        "-forcelogflush",
        "-nop4",
        "-nosplash",
        "-unattended",
        "-windowed",
        "-ResX=$ResX",
        "-ResY=$ResY"
    )

    Write-Host "Verify reload phase: $Exe $($argsList -join ' ')"

    if ($PrintOnly) {
        return [pscustomobject]@{
            Name = "verify_reload"
            Status = "PRINT_ONLY"
            Command = $VerifyCommand
            Log = $VerifyReloadLogPath
            RequiredMarkers = @("[SaveIntegrityReload] PASS Slot=$SlotIndex Marker=$Marker")
        }
    }

    $process = Start-Process -FilePath $Exe -ArgumentList $argsList -WorkingDirectory $ProjectRoot -PassThru
    $deadline = (Get-Date).AddSeconds($TimeoutSeconds)
    while (-not $process.HasExited -and (Get-Date) -lt $deadline) {
        Start-Sleep -Milliseconds 500
    }

    if (-not $process.HasExited) {
        Stop-Process -Id $process.Id -Force -ErrorAction SilentlyContinue
        throw "Verify reload phase timed out after $TimeoutSeconds seconds. Log: $VerifyReloadLogPath"
    }

    if ($process.ExitCode -ne 0) {
        throw "Verify reload phase exited with code $($process.ExitCode). Expected 0. Log: $VerifyReloadLogPath"
    }

    $logText = Read-LogText -Path $VerifyReloadLogPath
    foreach ($forbidden in @("[SaveIntegrityReload] FAIL", "Usage: T66.Save.", "Command failed: no world", "Command failed: no save subsystem")) {
        Assert-LogDoesNotContain -LogText $logText -Marker $forbidden -Path $VerifyReloadLogPath -Phase "verify_reload"
    }
    foreach ($required in @("[SaveIntegrityReload] PASS Slot=$SlotIndex Marker=$Marker", "SlotExists=1", "LoadedOk=1", "MetaOk=1")) {
        Assert-LogContains -LogText $logText -Marker $required -Path $VerifyReloadLogPath -Phase "verify_reload"
    }

    [pscustomobject]@{
        Name = "verify_reload"
        Status = "PASS"
        Command = $VerifyCommand
        Log = $VerifyReloadLogPath
        ExitCode = $process.ExitCode
        RequiredMarkers = @("[SaveIntegrityReload] PASS Slot=$SlotIndex Marker=$Marker", "SlotExists=1", "LoadedOk=1", "MetaOk=1")
    }
}

function Invoke-LoadClickPhase {
    foreach ($path in @($LoadClickLogPath, $PreClickDumpPath)) {
        if (Test-Path -LiteralPath $path) {
            Remove-Item -LiteralPath $path -Force
        }
    }

    $argsList = @(
        "-windowed",
        "-ResX=$ResX",
        "-ResY=$ResY",
        "-T66AutomationResX=$ResX",
        "-T66AutomationResY=$ResY",
        "-T66AutomationWindowed",
        "-T66FrontendScreen=SaveSlots",
        "-T66AutoDumpScreen=`"$PreClickDumpPath`"",
        "-T66AutoDumpScreenDelay=$PreClickDumpDelaySeconds",
        "-T66AutoClickTag=`"$ClickTag`"",
        "-T66AutoClickDelay=$ClickDelaySeconds",
        "-abslog=`"$LoadClickLogPath`"",
        "-forcelogflush",
        "-nop4",
        "-nosplash",
        "-unattended"
    )

    $requiredMarkers = @(
        "Frontend automation: simulating Slate click Tag=$ClickTag",
        "Frontend automation: completed Slate click Tag=$ClickTag",
        "[LOAD] TransitionToGameplayLevel started pre-open asset preload.",
        "[LOAD] TransitionToGameplayLevel opening"
    )
    $forbiddenMarkers = @(
        "T66AutoClickTagResolveFailed",
        "T66AutoClickTagNoButton",
        "T66AutoClickTagNotClickable",
        "T66AutoClickTagInvalidGeometry",
        "T66AutoClickTagShipping",
        "T66AutoClickTagInvalid",
        "Frontend automation: failed to resolve click tag",
        "Frontend automation: resolved click tag '$ClickTag' to a disabled or hidden SButton"
    )

    Write-Host "Load-click phase: $Exe $($argsList -join ' ')"

    if ($PrintOnly) {
        return [pscustomobject]@{
            Name = "load_click"
            Status = "PRINT_ONLY"
            Screen = "SaveSlots"
            ClickTag = $ClickTag
            Log = $LoadClickLogPath
            PreClickDump = $PreClickDumpPath
            RequiredMarkers = $requiredMarkers
            ForbiddenMarkers = $forbiddenMarkers
        }
    }

    $process = Start-Process -FilePath $Exe -ArgumentList $argsList -WorkingDirectory $ProjectRoot -PassThru
    $deadline = (Get-Date).AddSeconds($TimeoutSeconds)
    $logText = ""
    $markersFound = $false

    try {
        while ((Get-Date) -lt $deadline) {
            $logText = Read-LogText -Path $LoadClickLogPath
            foreach ($forbidden in $forbiddenMarkers) {
                Assert-LogDoesNotContain -LogText $logText -Marker $forbidden -Path $LoadClickLogPath -Phase "load_click"
            }

            $markersFound = $true
            foreach ($required in $requiredMarkers) {
                if (-not (Test-SmokeTextContains -Text $logText -Marker $required)) {
                    $markersFound = $false
                    break
                }
            }

            if ($markersFound) {
                break
            }

            if ($process.HasExited) {
                throw "Load-click phase exited before all required markers were observed. ExitCode=$($process.ExitCode). Log: $LoadClickLogPath"
            }

            Start-Sleep -Milliseconds 500
        }

        if (-not $markersFound) {
            if (-not $process.HasExited) {
                Stop-Process -Id $process.Id -Force -ErrorAction SilentlyContinue
            }
            throw "Load-click phase timed out before all required markers were observed. Log: $LoadClickLogPath"
        }

        if (-not $process.HasExited) {
            Stop-Process -Id $process.Id -Force -ErrorAction SilentlyContinue
        }
    } finally {
        if ($process -and -not $process.HasExited) {
            Stop-Process -Id $process.Id -Force -ErrorAction SilentlyContinue
        }
    }

    if (-not (Test-Path -LiteralPath $PreClickDumpPath)) {
        throw "Load-click phase did not write the pre-click SaveSlots dump: $PreClickDumpPath"
    }

    $dumpText = Get-Content -LiteralPath $PreClickDumpPath -Raw
    try {
        $dumpJson = $dumpText | ConvertFrom-Json
    } catch {
        throw "Pre-click SaveSlots dump is not valid JSON. Dump: $PreClickDumpPath Error: $($_.Exception.Message)"
    }

    if ($dumpJson.screen -ne "SaveSlots") {
        throw "Pre-click dump was not captured on the SaveSlots screen. Screen='$($dumpJson.screen)' Dump: $PreClickDumpPath"
    }

    $loadButtonWidgets = @($dumpJson.widgets | Where-Object { $_.tag -eq $ClickTag })
    if ($loadButtonWidgets.Count -lt 1) {
        throw "Pre-click SaveSlots dump missing load button tag '$ClickTag'. Dump: $PreClickDumpPath"
    }

    $loadButtonWidget = $loadButtonWidgets[0]
    if (-not $loadButtonWidget.geometry.enabled) {
        throw "Pre-click SaveSlots load button '$ClickTag' was disabled. Dump: $PreClickDumpPath"
    }

    if (-not $loadButtonWidget.interactivity.has_click_handler) {
        throw "Pre-click SaveSlots load button '$ClickTag' had no click handler. Dump: $PreClickDumpPath"
    }

    $requiredDumpMarkers = @($ClickTag, "Stage 66", "Easy / Solo")
    foreach ($requiredDump in $requiredDumpMarkers) {
        if (-not (Test-SmokeTextContains -Text $dumpText -Marker $requiredDump)) {
            throw "Pre-click SaveSlots dump missing expected marker '$requiredDump'. Dump: $PreClickDumpPath"
        }
    }

    [pscustomobject]@{
        Name = "load_click"
        Status = "PASS"
        Screen = "SaveSlots"
        ClickTag = $ClickTag
        Log = $LoadClickLogPath
        PreClickDump = $PreClickDumpPath
        RequiredMarkers = $requiredMarkers
        RequiredDumpMarkers = $requiredDumpMarkers
        ForbiddenMarkers = $forbiddenMarkers
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
        PreClickDumpDelaySeconds = $PreClickDumpDelaySeconds
        ClickDelaySeconds = $ClickDelaySeconds
        TimeoutSeconds = $TimeoutSeconds
        QueueCommand = $QueueCommand
        VerifyCommand = $VerifyCommand
        ClickTag = $ClickTag
        QueueLog = $QueueLogPath
        VerifyReloadLog = $VerifyReloadLogPath
        LoadClickLog = $LoadClickLogPath
        PreClickDump = $PreClickDumpPath
        BackupManifest = $BackupManifestPath
        SaveRoots = $SaveRoots
        Phases = $PhaseResults
        SeedSync = $SeedSyncRows
        Restore = $RestoreRows
        Error = $ErrorMessage
    }

    $summary | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $SummaryJsonPath -Encoding UTF8

    $md = @()
    $md += "# SaveSlots Load-Click Smoke"
    $md += ""
    $md += "Status: $Status"
    $md += "Executable: $($summary.Exe)"
    $md += "Slot: $SlotIndex ($SlotFileName)"
    $md += "Marker: $Marker"
    $md += "Click tag: $ClickTag"
    $md += "Queue log: $QueueLogPath"
    $md += "Verify reload log: $VerifyReloadLogPath"
    $md += "Load-click log: $LoadClickLogPath"
    $md += "Pre-click dump: $PreClickDumpPath"
    $md += "Backup manifest: $BackupManifestPath"
    if ($ErrorMessage) {
        $md += "Error: $ErrorMessage"
    }
    $md += ""
    foreach ($phase in $PhaseResults) {
        $md += "## $($phase.Name)"
        $md += ""
        $md += "- Status: $($phase.Status)"
        if ($phase.Log) {
            $md += "- Log: $($phase.Log)"
        }
        if ($phase.PreClickDump) {
            $md += "- Pre-click dump: $($phase.PreClickDump)"
        }
        if ($phase.RequiredMarkers) {
            foreach ($markerText in $phase.RequiredMarkers) {
                $md += "- Required marker: ``$markerText``"
            }
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
    $md += "This gate seeds a protected solo save slot with the development-only save-integrity harness, clicks the matching SaveSlots load button through the non-shipping Slate tag resolver, asserts the local gameplay transition loading markers, and restores the protected save/index files afterward."
    $md -join "`r`n" | Set-Content -LiteralPath $SummaryMdPath -Encoding UTF8

    Write-Host "SaveSlots load-click smoke $Status`: $OutputRoot"
    Write-Host "Summary: $SummaryJsonPath"
}

if ($SlotIndex -lt 0 -or $SlotIndex -gt 2) {
    throw "SlotIndex must be between 0 and 2. This focused smoke only targets first-page SaveSlots entries."
}
if ($Marker -notmatch '^[A-Za-z0-9_.-]+$') {
    throw "Marker may only contain letters, numbers, underscore, hyphen, or dot. Current marker: '$Marker'"
}
if ($PreClickDumpDelaySeconds -le 0) {
    throw "PreClickDumpDelaySeconds must be greater than 0."
}
if ($ClickDelaySeconds -le $PreClickDumpDelaySeconds) {
    throw "ClickDelaySeconds ($ClickDelaySeconds) must be greater than PreClickDumpDelaySeconds ($PreClickDumpDelaySeconds)."
}
if ($TimeoutSeconds -lt 10 -or $TimeoutSeconds -gt 600) {
    throw "TimeoutSeconds must be between 10 and 600."
}
if (-not (Test-Path -LiteralPath $Exe)) {
    throw "Missing executable: $Exe"
}

New-Item -ItemType Directory -Force -Path $OutputRoot | Out-Null
foreach ($path in @($QueueLogPath, $LoadClickLogPath, $PreClickDumpPath, $BackupManifestPath, $SummaryJsonPath, $SummaryMdPath)) {
    if (Test-Path -LiteralPath $path) {
        Remove-Item -LiteralPath $path -Force
    }
}

$saveRoots = @(Get-DurableSaveRoots)
$phaseResults = New-Object System.Collections.Generic.List[object]
$snapshotRows = $null
$restoreRows = New-Object System.Collections.Generic.List[object]
$seedSyncRows = New-Object System.Collections.Generic.List[object]
$status = "UNKNOWN"
$errorMessage = $null

try {
    Write-Host "SaveSlots load-click smoke uses a protected solo save seed and non-shipping Slate tag automation."
    Write-Host "Save roots protected:"
    foreach ($root in $saveRoots) {
        Write-Host "  $root"
    }
    Write-Host "Selected slot $SlotIndex ($SlotFileName) will be written by the proof, then restored from the save snapshot."
    Write-Host "Queue command: $QueueCommand"
    Write-Host "Verify command: $VerifyCommand"
    Write-Host "Click tag: $ClickTag"

    if ($PrintOnly) {
        $status = "PRINT_ONLY"
        $phaseResults.Add((Invoke-QueueSeedPhase))
        $phaseResults.Add((Invoke-VerifyReloadPhase))
        $phaseResults.Add((Invoke-LoadClickPhase))
    } else {
        $snapshotRows = New-SaveSnapshot -SaveRoots $saveRoots
        $phaseResults.Add((Invoke-QueueSeedPhase))
        Assert-Condition -Condition (Test-Path -LiteralPath $Exe) -Message "Staged executable disappeared after seed phase: $Exe"
        $seedSyncRows = Sync-SeededSaveFilesToAllRoots -SaveRoots $saveRoots -SnapshotRows $snapshotRows
        $phaseResults.Add((Invoke-VerifyReloadPhase))
        $phaseResults.Add((Invoke-LoadClickPhase))
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
