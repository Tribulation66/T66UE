param(
    [string]$Exe = "C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe",
    [string]$OutputRoot,
    [int]$SlotIndex = 8,
    [string]$Marker,
    [int]$TimeoutSeconds = 90,
    [switch]$PrintOnly
)

$ErrorActionPreference = "Stop"

$ProjectRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot "..")).TrimEnd('\')

if (-not $OutputRoot) {
    $stamp = Get-Date -Format "yyyyMMdd_HHmmss"
    $OutputRoot = Join-Path $ProjectRoot "Saved\DurableSaveIntegritySmokeGate\$stamp"
}
$OutputRoot = [System.IO.Path]::GetFullPath($OutputRoot)

if (-not $Marker) {
    $Marker = "DurableGate_$((Get-Date).ToString('yyyyMMdd_HHmmss'))"
}

$SlotFileName = "T66_Slot_{0:D2}.sav" -f $SlotIndex
$IndexFileName = "T66_SaveIndex.sav"
$QueueLogPath = Join-Path $OutputRoot "queue_shutdown.log"
$VerifyLogPath = Join-Path $OutputRoot "reload_verify.log"
$BackupManifestPath = Join-Path $OutputRoot "save_backup_manifest.json"
$SummaryJsonPath = Join-Path $OutputRoot "summary.json"
$SummaryMdPath = Join-Path $OutputRoot "summary.md"
$QueueCommand = "T66.Save.QueueIntegrityShutdown $SlotIndex $Marker CONFIRM 0"
$VerifyCommand = "T66.Save.VerifyIntegritySlot $SlotIndex $Marker CONFIRM 0"

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

    $hashCommand = Get-Command Get-FileHash -ErrorAction SilentlyContinue
    if ($hashCommand) {
        return (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash
    }

    $stream = [System.IO.File]::OpenRead($Path)
    try {
        $sha = [System.Security.Cryptography.SHA256]::Create()
        try {
            $bytes = $sha.ComputeHash($stream)
            return -join ($bytes | ForEach-Object { $_.ToString("x2") })
        } finally {
            $sha.Dispose()
        }
    } finally {
        $stream.Dispose()
    }
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

function Invoke-DurableGatePhase {
    param(
        [string]$Name,
        [string]$Command,
        [string]$LogPath,
        [string]$RequiredMarker
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
        "-ResX=1280",
        "-ResY=720"
    )

    Write-Host "$Name`: $Exe $($argsList -join ' ')"

    $process = Start-Process -FilePath $Exe -ArgumentList $argsList -WorkingDirectory $ProjectRoot -PassThru
    $deadline = (Get-Date).AddSeconds($TimeoutSeconds)
    while (-not $process.HasExited -and (Get-Date) -lt $deadline) {
        Start-Sleep -Milliseconds 500
    }

    if (-not $process.HasExited) {
        Stop-Process -Id $process.Id -Force -ErrorAction SilentlyContinue
        $missingHint = if (-not (Test-Path -LiteralPath $LogPath)) {
            " No log was written; verify the executable path and build configuration."
        } else {
            " The non-shipping save integrity command may be unavailable in this build."
        }
        throw "$Name timed out after $TimeoutSeconds seconds.$missingHint Log: $LogPath"
    }

    $exitCode = $process.ExitCode
    if ($exitCode -ne 0) {
        throw "$Name exited with code $exitCode. Expected 0. Log: $LogPath"
    }

    if (-not (Test-Path -LiteralPath $LogPath)) {
        throw "$Name did not write a log. This gate requires a Development/non-shipping executable with the T66.Save integrity commands available."
    }

    $logText = Get-Content -LiteralPath $LogPath -Raw
    foreach ($forbidden in @("[SaveIntegrity] FAIL", "[SaveIntegrityReload] FAIL", "Usage: T66.Save.", "Command failed: no world", "Command failed: no save subsystem")) {
        if ($logText.IndexOf($forbidden, [System.StringComparison]::OrdinalIgnoreCase) -ge 0) {
            throw "$Name log contained forbidden marker '$forbidden'. Log: $LogPath"
        }
    }

    if ($logText.IndexOf($RequiredMarker, [System.StringComparison]::OrdinalIgnoreCase) -lt 0) {
        throw "$Name log missing required marker '$RequiredMarker'. This usually means the executable is Shipping or the console command did not run. Log: $LogPath"
    }

    [pscustomobject]@{
        Name = $Name
        Status = "PASS"
        Command = $Command
        Log = $LogPath
        ExitCode = $exitCode
        RequiredMarker = $RequiredMarker
    }
}

function Write-GateSummary {
    param(
        [string]$Status,
        [string]$ErrorMessage,
        [object[]]$PhaseResults,
        [object[]]$SaveRoots,
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
        TimeoutSeconds = $TimeoutSeconds
        QueueCommand = $QueueCommand
        VerifyCommand = $VerifyCommand
        QueueLog = $QueueLogPath
        VerifyLog = $VerifyLogPath
        BackupManifest = $BackupManifestPath
        SaveRoots = $SaveRoots
        Phases = $PhaseResults
        Restore = $RestoreRows
        Error = $ErrorMessage
    }

    $summary | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $SummaryJsonPath -Encoding UTF8

    $md = @()
    $md += "# Durable Save Integrity Smoke Gate"
    $md += ""
    $md += "Status: $Status"
    $md += "Executable: $($summary.Exe)"
    $md += "Slot: $SlotIndex ($SlotFileName)"
    $md += "Marker: $Marker"
    $md += "Queue log: $QueueLogPath"
    $md += "Verify log: $VerifyLogPath"
    $md += "Backup manifest: $BackupManifestPath"
    if ($ErrorMessage) {
        $md += "Error: $ErrorMessage"
    }
    $md += ""
    foreach ($phase in $PhaseResults) {
        $md += "- $($phase.Name): $($phase.Status) marker ``$($phase.RequiredMarker)``"
    }
    foreach ($row in $RestoreRows) {
        if ($row.ExistedBefore) {
            $md += "- Restored: $($row.TargetPath)"
        } elseif ($row.RemovedCreatedFile) {
            $md += "- Removed proof-created file: $($row.TargetPath)"
        }
    }
    $md += ""
    $md += 'This gate wraps the development-only `T66.Save.QueueIntegrityShutdown` and `T66.Save.VerifyIntegritySlot` commands. It protects selected staged save files by restoring `T66_Slot_XX.sav` and `T66_SaveIndex.sav` after the two-phase proof.'
    $md -join "`r`n" | Set-Content -LiteralPath $SummaryMdPath -Encoding UTF8

    Write-Host "Durable save integrity smoke gate $Status`: $OutputRoot"
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
foreach ($path in @($QueueLogPath, $VerifyLogPath, $BackupManifestPath, $SummaryJsonPath, $SummaryMdPath)) {
    if (Test-Path -LiteralPath $path) {
        Remove-Item -LiteralPath $path -Force
    }
}

$saveRoots = @(Get-DurableSaveRoots)
$phaseResults = New-Object System.Collections.Generic.List[object]
$snapshotRows = $null
$restoreRows = New-Object System.Collections.Generic.List[object]
$status = "UNKNOWN"
$errorMessage = $null

try {
    Write-Host "Durable save integrity smoke gate uses development-only T66.Save integrity commands."
    Write-Host "Save roots protected:"
    foreach ($root in $saveRoots) {
        Write-Host "  $root"
    }
    Write-Host "Selected slot $SlotIndex ($SlotFileName) will be written by the proof, then restored from the save snapshot."
    Write-Host "Queue command: $QueueCommand"
    Write-Host "Verify command: $VerifyCommand"

    if ($PrintOnly) {
        $status = "PRINT_ONLY"
    } else {
        $snapshotRows = New-SaveSnapshot -SaveRoots $saveRoots
        $phaseResults.Add((Invoke-DurableGatePhase -Name "queue_shutdown" -Command $QueueCommand -LogPath $QueueLogPath -RequiredMarker "[SaveIntegrity] PASS Slot=$SlotIndex Marker=$Marker"))
        $phaseResults.Add((Invoke-DurableGatePhase -Name "reload_verify" -Command $VerifyCommand -LogPath $VerifyLogPath -RequiredMarker "[SaveIntegrityReload] PASS Slot=$SlotIndex Marker=$Marker"))
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

    Write-GateSummary -Status $status -ErrorMessage $errorMessage -PhaseResults $phaseResults.ToArray() -SaveRoots $saveRoots -RestoreRows $restoreRows
}

if ($status -eq "FAIL") {
    throw $errorMessage
}
