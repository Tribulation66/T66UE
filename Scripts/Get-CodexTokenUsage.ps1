<#
.SYNOPSIS
Read-only helper that reports the latest completed Codex turn token usage from
Codex rollout JSONL files (a non-goal source).

.DESCRIPTION
T66 agents call this before a final user-facing answer to populate
`Codex Token Spent` without using any native goal tool. It does NOT use
`/goal`, `create_goal`, `get_goal`, or any equivalent native goal API, and it
does not write or mutate anything.

It resolves the Codex home from $env:CODEX_HOME when set (otherwise
C:\Users\DoPra\.codex), finds the most recently written rollout-*.jsonl under
<CodexHome>\sessions, and scans that file from the end for the latest
`event_msg` line whose payload type is `token_count` and whose
payload.info.last_token_usage.total_tokens exists.

The reported `CodexTokenSpent` is the latest COMPLETED Codex turn before the
final answer. The final answer's own tokens are not included until that turn
flushes its own token_count event, which happens after the answer is sent.

.PARAMETER CodexHome
Override the Codex home directory. Defaults to $env:CODEX_HOME or
C:\Users\DoPra\.codex.

.PARAMETER MaxFiles
Maximum number of recent rollout files to consider (newest first). Default 5.

.PARAMETER Json
Emit the result as a JSON string instead of a PowerShell object.

.OUTPUTS
A PSCustomObject (or JSON with -Json) carrying Available, CodexTokenSpent,
Label, TotalSessionTokens, InputTokens, CachedInputTokens, OutputTokens,
ReasoningOutputTokens, PrimaryUsedPercent, SecondaryUsedPercent, RolloutPath,
Timestamp, Caveat, and UnavailableReason.
#>

[CmdletBinding()]
param(
    [string] $CodexHome = "",
    [int] $MaxFiles = 5,
    [switch] $Json
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function New-Result {
    param(
        [bool] $Available,
        [Nullable[long]] $CodexTokenSpent = $null,
        [string] $Label = "latest completed Codex turn before final answer",
        [Nullable[long]] $TotalSessionTokens = $null,
        [Nullable[long]] $InputTokens = $null,
        [Nullable[long]] $CachedInputTokens = $null,
        [Nullable[long]] $OutputTokens = $null,
        [Nullable[long]] $ReasoningOutputTokens = $null,
        [Nullable[double]] $PrimaryUsedPercent = $null,
        [Nullable[double]] $SecondaryUsedPercent = $null,
        [string] $RolloutPath = $null,
        [string] $Timestamp = $null,
        [string] $Caveat = "Final answer tokens are not included; the latest completed Codex turn flushes its own token_count event only after the answer is sent.",
        [string] $UnavailableReason = $null
    )

    return [PSCustomObject][ordered]@{
        Available             = $Available
        CodexTokenSpent       = $CodexTokenSpent
        Label                 = $Label
        TotalSessionTokens    = $TotalSessionTokens
        InputTokens           = $InputTokens
        CachedInputTokens     = $CachedInputTokens
        OutputTokens          = $OutputTokens
        ReasoningOutputTokens = $ReasoningOutputTokens
        PrimaryUsedPercent    = $PrimaryUsedPercent
        SecondaryUsedPercent  = $SecondaryUsedPercent
        RolloutPath           = $RolloutPath
        Timestamp             = $Timestamp
        Caveat                = $Caveat
        UnavailableReason     = $UnavailableReason
    }
}

function Write-Output-Result {
    param([Parameter(Mandatory = $true)] $Result)

    if ($Json) {
        return ($Result | ConvertTo-Json -Depth 6)
    }
    return $Result
}

# Resolve Codex home (read-only; no creation).
if ([string]::IsNullOrWhiteSpace($CodexHome)) {
    if (-not [string]::IsNullOrWhiteSpace($env:CODEX_HOME)) {
        $CodexHome = $env:CODEX_HOME
    }
    else {
        $CodexHome = "C:\Users\DoPra\.codex"
    }
}

$sessionsDir = Join-Path $CodexHome "sessions"
if (-not (Test-Path -LiteralPath $sessionsDir)) {
    return (Write-Output-Result (New-Result -Available $false -UnavailableReason "Codex sessions directory not found: $sessionsDir"))
}

# Find recent rollout files, newest first.
$rolloutFiles = @(
    Get-ChildItem -LiteralPath $sessionsDir -Recurse -File -Filter "rollout-*.jsonl" -ErrorAction SilentlyContinue |
        Sort-Object LastWriteTime -Descending |
        Select-Object -First $MaxFiles
)

if ($rolloutFiles.Count -eq 0) {
    return (Write-Output-Result (New-Result -Available $false -UnavailableReason "No rollout-*.jsonl files found under $sessionsDir"))
}

foreach ($file in $rolloutFiles) {
    $lines = $null
    try {
        $lines = Get-Content -LiteralPath $file.FullName -ErrorAction Stop
    }
    catch {
        continue
    }
    if ($null -eq $lines) { continue }

    # Scan from the end for the latest matching token_count event.
    for ($i = $lines.Count - 1; $i -ge 0; $i--) {
        $line = $lines[$i]
        if ([string]::IsNullOrWhiteSpace($line)) { continue }
        if ($line -notmatch "token_count") { continue }

        $obj = $null
        try {
            $obj = $line | ConvertFrom-Json -ErrorAction Stop
        }
        catch {
            continue
        }

        if ($null -eq $obj) { continue }
        if (($obj.PSObject.Properties.Name -notcontains "type") -or ($obj.type -ne "event_msg")) { continue }
        if (($obj.PSObject.Properties.Name -notcontains "payload") -or ($null -eq $obj.payload)) { continue }

        $payload = $obj.payload
        if (($payload.PSObject.Properties.Name -notcontains "type") -or ($payload.type -ne "token_count")) { continue }
        if (($payload.PSObject.Properties.Name -notcontains "info") -or ($null -eq $payload.info)) { continue }

        $info = $payload.info
        if (($info.PSObject.Properties.Name -notcontains "last_token_usage") -or ($null -eq $info.last_token_usage)) { continue }

        $last = $info.last_token_usage
        if (($last.PSObject.Properties.Name -notcontains "total_tokens") -or ($null -eq $last.total_tokens)) { continue }

        $total = $null
        if (($info.PSObject.Properties.Name -contains "total_token_usage") -and ($null -ne $info.total_token_usage)) {
            $total = $info.total_token_usage
        }

        $primaryPct = $null
        $secondaryPct = $null
        if (($payload.PSObject.Properties.Name -contains "rate_limits") -and ($null -ne $payload.rate_limits)) {
            $rl = $payload.rate_limits
            if (($rl.PSObject.Properties.Name -contains "primary") -and ($null -ne $rl.primary) -and ($rl.primary.PSObject.Properties.Name -contains "used_percent")) {
                $primaryPct = [double] $rl.primary.used_percent
            }
            if (($rl.PSObject.Properties.Name -contains "secondary") -and ($null -ne $rl.secondary) -and ($rl.secondary.PSObject.Properties.Name -contains "used_percent")) {
                $secondaryPct = [double] $rl.secondary.used_percent
            }
        }

        $timestamp = $null
        if (($obj.PSObject.Properties.Name -contains "timestamp") -and ($null -ne $obj.timestamp)) {
            if ($obj.timestamp -is [datetime]) {
                $timestamp = $obj.timestamp.ToUniversalTime().ToString("o")
            }
            else {
                $timestamp = [string] $obj.timestamp
            }
        }

        $result = New-Result `
            -Available $true `
            -CodexTokenSpent ([long] $last.total_tokens) `
            -TotalSessionTokens $(if ($total -and ($total.PSObject.Properties.Name -contains "total_tokens")) { [long] $total.total_tokens } else { $null }) `
            -InputTokens $(if ($last.PSObject.Properties.Name -contains "input_tokens") { [long] $last.input_tokens } else { $null }) `
            -CachedInputTokens $(if ($last.PSObject.Properties.Name -contains "cached_input_tokens") { [long] $last.cached_input_tokens } else { $null }) `
            -OutputTokens $(if ($last.PSObject.Properties.Name -contains "output_tokens") { [long] $last.output_tokens } else { $null }) `
            -ReasoningOutputTokens $(if ($last.PSObject.Properties.Name -contains "reasoning_output_tokens") { [long] $last.reasoning_output_tokens } else { $null }) `
            -PrimaryUsedPercent $primaryPct `
            -SecondaryUsedPercent $secondaryPct `
            -RolloutPath $file.FullName `
            -Timestamp $timestamp

        return (Write-Output-Result $result)
    }
}

return (Write-Output-Result (New-Result -Available $false -UnavailableReason "No token_count event with last_token_usage.total_tokens found in the $($rolloutFiles.Count) most recent rollout file(s)."))
