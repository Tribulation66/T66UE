param(
    [string]$SourcePath = (Join-Path $PSScriptRoot '..\UE5RFX_UE5.2_Experimental_V8\UE5RFX_UE5.2_Experimental_V8\Content\UE5RFX'),
    [string]$ProjectRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path,
    [switch]$BackupExisting,
    [switch]$ReplaceExisting
)

$ErrorActionPreference = 'Stop'

function Resolve-FullPath {
    param([string]$PathValue)

    $resolved = Resolve-Path -Path $PathValue -ErrorAction SilentlyContinue
    if ($resolved) {
        return $resolved.Path
    }

    return [System.IO.Path]::GetFullPath($PathValue)
}

$source = Resolve-FullPath -PathValue $SourcePath
$destination = Join-Path $ProjectRoot 'Content\UE5RFX'

if (-not (Test-Path $source)) {
    throw "UE5RFX source folder not found: $source"
}

$editorProcesses = Get-Process -Name 'UnrealEditor','UE4Editor' -ErrorAction SilentlyContinue
if ($editorProcesses) {
    throw 'Close Unreal Editor before installing UE5RFX content.'
}

if (Test-Path $destination) {
    if ($BackupExisting) {
        $timestamp = Get-Date -Format 'yyyyMMdd_HHmmss'
        $backupPath = Join-Path (Split-Path $destination -Parent) ("UE5RFX_Backup_$timestamp")
        Move-Item -Path $destination -Destination $backupPath
        Write-Host "Backed up existing UE5RFX content to: $backupPath"
    }
    elseif ($ReplaceExisting) {
        Remove-Item -Path $destination -Recurse -Force
        Write-Host "Removed existing UE5RFX content at: $destination"
    }
    else {
        throw "Destination already exists: $destination. Re-run with -BackupExisting or -ReplaceExisting."
    }
}

Copy-Item -Path $source -Destination $destination -Recurse -Force

Write-Host ''
Write-Host 'UE5RFX install complete.'
Write-Host "Source      : $source"
Write-Host "Destination : $destination"
Write-Host 'Package path: /Game/UE5RFX'
Write-Host ''
Write-Host 'Next steps:'
Write-Host '1. Open the T66 project in Unreal Editor.'
Write-Host '2. Let the asset registry discover /Game/UE5RFX.'
Write-Host '3. Open the Settings screen and use the new Retro FX tab.'
