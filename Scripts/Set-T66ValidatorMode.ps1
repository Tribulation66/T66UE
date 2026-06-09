<#
.SYNOPSIS
Sets the optional T66 validator mode.

.DESCRIPTION
Writes the canonical repo-local validator state at .t66\validator-state.json and,
by default, mirrors the same JSON to the AI usage tray runtime state file.
Single-agent work is the default; validatorMode controls whether the optional
Validator loop is persistently engaged.
#>

[CmdletBinding()]
param(
    [ValidateSet("", "off", "on")]
    [string] $ValidatorMode = "",

    [ValidateSet("", "Claude", "Codex")]
    [string] $Validator = "",

    [string] $Source = "UserValidatorSetting",

    [string] $RepoStatePath = "",

    [string] $TrayStatePath = "",

    [switch] $NoTrayMirror
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Write-JsonAtomic {
    param(
        [Parameter(Mandatory = $true)][string] $Path,
        [Parameter(Mandatory = $true)] $Value
    )

    $Directory = Split-Path -Parent $Path
    if (-not [string]::IsNullOrWhiteSpace($Directory)) {
        $null = New-Item -ItemType Directory -Force -Path $Directory
    }

    $TempPath = "$Path.tmp-$PID-$([guid]::NewGuid().ToString('N'))"
    $Value | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath $TempPath -Encoding UTF8
    Move-Item -LiteralPath $TempPath -Destination $Path -Force
}

function Test-ValidatorName {
    param($Value)
    return ($Value -eq "Claude" -or $Value -eq "Codex")
}

$RepoRoot = Split-Path -Parent $PSScriptRoot
if ([string]::IsNullOrWhiteSpace($RepoStatePath)) {
    $RepoStatePath = Join-Path $RepoRoot ".t66\validator-state.json"
}

if ([string]::IsNullOrWhiteSpace($TrayStatePath)) {
    $LocalAppData = [Environment]::GetFolderPath([Environment+SpecialFolder]::LocalApplicationData)
    $TrayStatePath = Join-Path $LocalAppData "T66UsageTray\validator-state.json"
}

$ExistingState = $null
if (Test-Path -LiteralPath $RepoStatePath) {
    try {
        $ExistingState = Get-Content -LiteralPath $RepoStatePath -Raw | ConvertFrom-Json
    } catch {
        $ExistingState = $null
    }
}

if ([string]::IsNullOrWhiteSpace($ValidatorMode)) {
    if ($ExistingState -and ($ExistingState.validatorMode -eq "on" -or $ExistingState.validatorMode -eq "off")) {
        $ValidatorMode = [string]$ExistingState.validatorMode
    } else {
        $ValidatorMode = "off"
    }
}

if ([string]::IsNullOrWhiteSpace($Validator)) {
    if ($ExistingState -and (Test-ValidatorName -Value $ExistingState.validator)) {
        $Validator = [string]$ExistingState.validator
    } else {
        $Validator = "Claude"
    }
}

$State = [ordered]@{
    validatorMode = $ValidatorMode
    validator = $Validator
    source = $Source
    updatedAtLocal = (Get-Date).ToString("o")
}

Write-JsonAtomic -Path $RepoStatePath -Value $State

$MirroredTray = $false
if (-not $NoTrayMirror) {
    Write-JsonAtomic -Path $TrayStatePath -Value $State
    $MirroredTray = $true
}

[pscustomobject]@{
    ValidatorMode = $ValidatorMode
    Validator = $Validator
    Source = $Source
    RepoStatePath = $RepoStatePath
    TrayStatePath = if ($MirroredTray) { $TrayStatePath } else { $null }
    MirroredTray = $MirroredTray
    UpdatedAtLocal = $State.updatedAtLocal
}
