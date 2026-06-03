<#
.SYNOPSIS
Approval-free read-only Claude Operator run for T66.

.DESCRIPTION
Thin wrapper over Scripts\Invoke-ClaudeDirectRead.ps1 that forces
-Mode Operator -ToolProfile ReadOnly. This is the obvious entry point for an
Operator-shaped run that only needs read-only tools (Read, Grep, Glob) and
therefore needs no Codex approval artifact. It produces an Operator work
artifact, not a greenlight; Codex still validates the output.

For mutating Operator work that needs edits/shell/MCP tools, use
Invoke-ClaudeDirectRead.ps1 -ToolProfile FullOperator with a Codex approval
artifact instead.

Session persistence follows the same opt-in rule as Invoke-ClaudeDirectRead.ps1:
off by default when -MaxTurns <= 0 (the read-only default) and on when
-MaxTurns > 0. Use -SessionPersistence to force it on or -NoSessionPersistence
to force it off.

Pass -Preflight to print the effective configuration without invoking Claude.
All other parameters forward to Invoke-ClaudeDirectRead.ps1.
#>

[CmdletBinding()]
param(
    [Parameter(Mandatory = $false)]
    [string] $PromptPath,

    [string] $TaskName = "ReadOnlyOperator",

    [string] $OutputRoot = "C:\UE\T66\Reports\AgentReviews\ClaudeDirectRead",

    [int] $MaxTurns = 0,

    [int] $MaxTurnContinuations = 3,

    [switch] $NoSessionPersistence,

    [switch] $SessionPersistence,

    [int] $Attempts = 2,

    [int] $TimeoutSeconds = -1,

    [string] $Model = "claude-opus-4-8",

    [ValidateSet("", "low", "medium", "high", "xhigh", "max")]
    [string] $Effort = "",

    [ValidateSet("text", "json")]
    [string] $ClaudeOutputFormat = "json",

    [string[]] $AddDir = @("C:\UE\T66"),

    [string] $ClaudeExe = "",

    [switch] $AllowApiKeyBilling,

    [switch] $Preflight
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$DirectRead = Join-Path $PSScriptRoot "Invoke-ClaudeDirectRead.ps1"
if (-not (Test-Path -LiteralPath $DirectRead -PathType Leaf)) {
    throw "Invoke-ClaudeDirectRead.ps1 not found next to this wrapper: $DirectRead"
}

# Forward only the parameters the caller actually supplied, then force the
# read-only Operator shape so this wrapper cannot be turned into a mutating run.
$Forward = @{}
foreach ($Name in $PSBoundParameters.Keys) {
    $Forward[$Name] = $PSBoundParameters[$Name]
}
$Forward["Mode"] = "Operator"
$Forward["ToolProfile"] = "ReadOnly"

& $DirectRead @Forward
