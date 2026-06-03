<#
.SYNOPSIS
Sets the project-global T66 Operator/Validator state.

.DESCRIPTION
Writes the canonical repo-local operator state at .t66\operator-state.json and,
by default, mirrors the same JSON to the AI usage tray runtime state file.
The tray file is display state; the repo-local file is what future T66 agents
are instructed to read.
#>

[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [ValidateSet("Claude", "Codex")]
    [string] $Operator,

    [ValidateSet("", "Claude", "Codex")]
    [string] $Validator = "",

    [ValidateSet("Global", "Thread", "Manual")]
    [string] $Scope = "Global",

    [string] $Source = "UserOperatorSwitch",

    [string] $RepoStatePath = "",

    [string] $TrayStatePath = "",

    [switch] $NoTrayMirror
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Get-OtherModel {
    param([Parameter(Mandatory = $true)][string] $Model)

    if ($Model -eq "Claude") {
        return "Codex"
    }

    return "Claude"
}

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

if ([string]::IsNullOrWhiteSpace($Validator)) {
    $Validator = Get-OtherModel -Model $Operator
}

if ($Validator -eq $Operator) {
    throw "Operator and Validator must be different models."
}

if ([string]::IsNullOrWhiteSpace($RepoStatePath)) {
    $RepoRoot = Split-Path -Parent $PSScriptRoot
    $RepoStatePath = Join-Path $RepoRoot ".t66\operator-state.json"
}

if ([string]::IsNullOrWhiteSpace($TrayStatePath)) {
    $LocalAppData = [Environment]::GetFolderPath([Environment+SpecialFolder]::LocalApplicationData)
    $TrayStatePath = Join-Path $LocalAppData "T66UsageTray\operator-state.json"
}

$State = [ordered]@{
    operator = $Operator
    validator = $Validator
    scope = $Scope
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
    Operator = $Operator
    Validator = $Validator
    Scope = $Scope
    Source = $Source
    RepoStatePath = $RepoStatePath
    TrayStatePath = if ($MirroredTray) { $TrayStatePath } else { $null }
    MirroredTray = $MirroredTray
    UpdatedAtLocal = $State.updatedAtLocal
}
