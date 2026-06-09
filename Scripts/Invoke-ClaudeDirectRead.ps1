<#
.SYNOPSIS
Runs a Claude Code operator or reviewer task for T66.

.DESCRIPTION
This helper supports read-only review/operator runs and Codex-approved full
Operator runs. Full Operator runs can use the normal Claude Code tool surface,
including edits, shell commands, and configured MCP tools, but only after Codex
has approved the requested change scope. Operator outputs are work artifacts,
not greenlights. Review outputs must use the result or verdict contract requested
by the calling protocol.

By default no `--max-turns` cap is passed (set -MaxTurns to cap explicitly).
Claude session persistence is opt-in based on the max-turn cap: it defaults off
when -MaxTurns <= 0 and on when -MaxTurns > 0. Persistence mainly exists to
support max-turn --resume auto-continuation, so an unbounded run (no max-turn
resume to support) does not need it, and keeping a session there only risks
resume failures on modified thinking/redacted_thinking blocks. Use
-NoSessionPersistence to always force it off, or -SessionPersistence to force it
on (the explicit opt-in for persistence when -MaxTurns <= 0). Effective
session persistence is reported truthfully in preflight and manifests, and
max-turn auto-continuation is disabled whenever effective persistence is off.
When -TimeoutSeconds is not supplied, FullOperator runs default to 0
(unbounded, no per-attempt wall-clock guard) so implementation phases are not
killed mid-run, while read-only profiles keep a 180s guard; pass an explicit
-TimeoutSeconds N to intentionally timebox a probe. When a run reports max-turn
exhaustion (error_max_turns / terminal_reason
max_turns) and persistence is on, the helper auto-continues the same session_id
via --resume, bounded by -MaxTurnContinuations. A manifest is written even when
the run ultimately fails, including FailureKind, attempt stdout/stderr paths,
and whether continuation was attempted.
#>

[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [ValidateSet("Review", "Operator")]
    [string] $Mode,

    [Parameter(Mandatory = $false)]
    [string] $PromptPath,

    [string] $TaskName = "DirectRead",

    [string] $OutputRoot = "C:\UE\T66\Reports\AgentReviews\ClaudeDirectRead",

    [ValidateRange(0, 100)]
    [int] $MaxTurns = 0,

    [ValidateRange(0, 10)]
    [int] $MaxTurnContinuations = 3,

    [switch] $NoSessionPersistence,

    # Force session persistence on. Primarily the explicit opt-in for persistence when
    # -MaxTurns <= 0 (where it defaults off). Cannot be combined with -NoSessionPersistence.
    [switch] $SessionPersistence,

    [ValidateRange(1, 3)]
    [int] $Attempts = 2,

    # 0 means unbounded (no wall-clock timeout). Otherwise a per-attempt wall-clock guard in seconds.
    # When -TimeoutSeconds is not supplied, the effective default depends on the tool profile:
    # FullOperator defaults to 0 (unbounded) so implementation phases are not killed mid-run;
    # read-only profiles default to 180s. Pass an explicit -TimeoutSeconds N to timebox a probe.
    [ValidateRange(0, 86400)]
    [int] $TimeoutSeconds = -1,

    [string] $Model = "claude-opus-4-8",

    [ValidateSet("", "low", "medium", "high", "xhigh", "max")]
    [string] $Effort = "",

    [ValidateSet("text", "json")]
    [string] $ClaudeOutputFormat = "json",

    [string[]] $AllowedTools = @("Read", "Grep", "Glob"),

    [string[]] $AddDir = @("C:\UE\T66"),

    [ValidateSet("acceptEdits", "auto", "bypassPermissions", "default", "dontAsk", "plan")]
    [string] $PermissionMode = "default",

    [ValidateSet("", "ReadOnly", "FullOperator")]
    [string] $ToolProfile = "",

    [string] $CodexApprovalPath = "",

    [string] $ReviewedOperatorRun = "",

    [string] $ClaudeExe = "",

    [switch] $AllowApiKeyBilling,

    [switch] $AllowBroaderTools,

    # Print the effective run configuration and exit without invoking Claude.
    [switch] $Preflight
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Resolve-ClaudeCliPath {
    param([string] $RequestedPath)

    $Candidates = @()
    if (-not [string]::IsNullOrWhiteSpace($RequestedPath)) {
        $Candidates += $RequestedPath
    }

    $LocalUserCli = Join-Path $env:USERPROFILE ".local\bin\claude.exe"
    $Candidates += $LocalUserCli

    foreach ($Name in @("claude.exe", "claude")) {
        $Command = Get-Command $Name -ErrorAction SilentlyContinue
        if ($Command) {
            $Candidates += $Command.Source
        }
    }

    foreach ($Candidate in ($Candidates | Select-Object -Unique)) {
        if ([string]::IsNullOrWhiteSpace($Candidate)) {
            continue
        }
        if ((Test-Path -LiteralPath $Candidate -PathType Leaf) -and $Candidate -notmatch "\\WindowsApps\\Claude_") {
            return (Resolve-Path -LiteralPath $Candidate).Path
        }
    }

    throw "FailureKind=ClaudeUnavailable; Claude Code CLI was not found. Expected a CLI such as $LocalUserCli."
}

function Assert-NoAnthropicApiKey {
    param([switch] $AllowApiKeyBilling)

    if ($AllowApiKeyBilling) {
        return
    }

    $ScopesWithKey = @()
    foreach ($Scope in @("Process", "User", "Machine")) {
        $Value = [Environment]::GetEnvironmentVariable("ANTHROPIC_API_KEY", $Scope)
        if (-not [string]::IsNullOrWhiteSpace($Value)) {
            $ScopesWithKey += $Scope
        }
    }

    if ($ScopesWithKey.Count -gt 0) {
        $Scopes = $ScopesWithKey -join ", "
        throw "FailureKind=ClaudeProcessFailed; ANTHROPIC_API_KEY is set in scope(s): $Scopes. Unset it before running Claude helper tasks so Claude Code uses the subscription login instead of API billing."
    }
}

function Assert-CodexApproval {
    param([string] $ApprovalPath)

    if ([string]::IsNullOrWhiteSpace($ApprovalPath) -or -not (Test-Path -LiteralPath $ApprovalPath -PathType Leaf)) {
        throw "FailureKind=MissingCodexApproval; FullOperator runs require a Codex approval artifact path."
    }

    $FirstNonEmptyLine = $null
    foreach ($Line in (Get-Content -LiteralPath $ApprovalPath)) {
        if ([string]::IsNullOrWhiteSpace($Line)) {
            continue
        }

        $FirstNonEmptyLine = $Line.TrimEnd()
        break
    }

    if ($FirstNonEmptyLine -cne "Codex Approval: APPROVE") {
        throw "FailureKind=MissingCodexApproval; FullOperator approval first non-empty line must be exactly 'Codex Approval: APPROVE'."
    }

    return (Resolve-Path -LiteralPath $ApprovalPath).Path
}

function Get-CodexApprovalStatus {
    param([AllowEmptyString()][string] $ApprovalPath)

    if ([string]::IsNullOrWhiteSpace($ApprovalPath)) {
        return "NotProvided"
    }
    if (-not (Test-Path -LiteralPath $ApprovalPath -PathType Leaf)) {
        return "Missing ($ApprovalPath)"
    }

    $FirstNonEmptyLine = $null
    foreach ($Line in (Get-Content -LiteralPath $ApprovalPath)) {
        if ([string]::IsNullOrWhiteSpace($Line)) {
            continue
        }
        $FirstNonEmptyLine = $Line.TrimEnd()
        break
    }

    if ($FirstNonEmptyLine -cne "Codex Approval: APPROVE") {
        return "InvalidFirstLine ($ApprovalPath)"
    }

    return "Valid ($((Resolve-Path -LiteralPath $ApprovalPath).Path))"
}

function Get-ClaudeCliVersion {
    param([Parameter(Mandatory = $true)][string] $Executable)

    try {
        $Output = & $Executable --version 2>$null
        return (($Output | Select-Object -First 1) -as [string]).Trim()
    } catch {
        return "unknown"
    }
}

function Get-ClaudeSubscriptionAuthStatus {
    param([AllowEmptyString()][string] $RawJson)

    $Status = $null
    $Reason = $null
    try {
        $Status = $RawJson | ConvertFrom-Json -ErrorAction Stop
    } catch {
        $Reason = "MalformedAuthStatusJson"
    }

    $LoggedIn = $false
    $AuthMethod = $null
    $ApiProvider = $null
    $SubscriptionType = $null

    if ($null -ne $Status) {
        $Names = $Status.PSObject.Properties.Name
        if ($Names -contains "loggedIn") {
            $LoggedIn = [bool]$Status.loggedIn
        }
        if ($Names -contains "authMethod") {
            $AuthMethod = [string]$Status.authMethod
        }
        if ($Names -contains "apiProvider") {
            $ApiProvider = [string]$Status.apiProvider
        }
        if ($Names -contains "subscriptionType") {
            $SubscriptionType = [string]$Status.subscriptionType
        }
    }

    if (-not $Reason) {
        if (-not $LoggedIn) {
            $Reason = "NotLoggedIn"
        } elseif ($AuthMethod -cne "claude.ai") {
            $Reason = "UnexpectedAuthMethod"
        } elseif ($ApiProvider -cne "firstParty") {
            $Reason = "UnexpectedApiProvider"
        }
    }

    $Authenticated = -not $Reason
    return [pscustomobject]@{
        Authenticated = $Authenticated
        LoggedIn = $LoggedIn
        AuthMethod = $AuthMethod
        ApiProvider = $ApiProvider
        SubscriptionType = $SubscriptionType
        OutcomeKind = if ($Authenticated) { "ClaudeSubscriptionAuthValid" } else { "ClaudeSubscriptionAuthInvalid" }
        FailureKind = if ($Authenticated) { $null } else { "ClaudeUnavailable" }
        StatusReason = $Reason
    }
}

function Assert-ClaudeSubscriptionAuth {
    param([Parameter(Mandatory = $true)][string] $Executable)

    $Output = ""
    try {
        $Output = & $Executable auth status --json 2>&1 | Out-String
        if ($LASTEXITCODE -ne 0) {
            throw "auth status exited with code $LASTEXITCODE"
        }
    } catch {
        throw "FailureKind=ClaudeUnavailable; Claude auth status failed. Confirm Claude Code is logged in with the user's subscription before running helper tasks."
    }

    $Parsed = Get-ClaudeSubscriptionAuthStatus -RawJson $Output
    if (-not $Parsed.Authenticated) {
        throw "FailureKind=ClaudeUnavailable; Claude auth status did not report loggedIn=true, authMethod=claude.ai, and apiProvider=firstParty. StatusReason=$($Parsed.StatusReason)."
    }

    return $Parsed
}

function Test-ClaudeUnavailableSignal {
    param([AllowEmptyString()][string] $Text)

    if ([string]::IsNullOrWhiteSpace($Text)) {
        return $false
    }

    foreach ($Pattern in @("subscription session limit", "session limit", "usage limit", "limit reached", "not logged in", "login required", "authentication required", "please log in")) {
        if ($Text.IndexOf($Pattern, [StringComparison]::OrdinalIgnoreCase) -ge 0) {
            return $true
        }
    }

    return $false
}

function Get-ClaudeReviewVerdict {
    param([Parameter(Mandatory = $true)][string] $ReviewPath)

    if (-not (Test-Path -LiteralPath $ReviewPath -PathType Leaf)) {
        throw "Review artifact not found: $ReviewPath"
    }

    $Raw = Get-Content -LiteralPath $ReviewPath -Raw
    $VerdictLine = $null
    $Verdict = $null
    $Greenlit = $false

    if (-not [string]::IsNullOrEmpty($Raw)) {
        foreach ($Line in ($Raw -split "`r?`n")) {
            $Candidate = $Line
            if ($Candidate.Length -gt 0 -and [int][char]$Candidate[0] -eq 0xFEFF) {
                $Candidate = $Candidate.Substring(1)
            }
            if ([string]::IsNullOrWhiteSpace($Candidate)) {
                continue
            }
            if ($Candidate -match '^\s' -or $Candidate -match '^#' -or $Candidate -match '^>') {
                break
            }
            $TrimmedEnd = $Candidate.TrimEnd()
            if ($TrimmedEnd -cmatch '^Verdict:\s*(APPROVE|REVISE|NEEDS_HUMAN_DECISION|BLOCK)\s*$') {
                $VerdictLine = $TrimmedEnd
                $Verdict = $Matches[1]
                $Greenlit = $Verdict -ceq "APPROVE"
            }
            break
        }
    }

    $OutcomeKind = "ClaudeMalformedVerdict"
    $FailureKind = "ClaudeMalformedVerdict"
    if ($Verdict) {
        $OutcomeKind = "ClaudeValidVerdict"
        $FailureKind = $null
    }

    return [pscustomobject]@{
        Greenlit = $Greenlit
        Verdict = $Verdict
        VerdictLine = $VerdictLine
        OutcomeKind = $OutcomeKind
        FailureKind = $FailureKind
    }
}

function Get-FileSnippet {
    param(
        [AllowEmptyString()][string] $Path,
        [int] $MaxChars = 400
    )

    if ([string]::IsNullOrWhiteSpace($Path) -or -not (Test-Path -LiteralPath $Path -PathType Leaf)) {
        return ""
    }

    $Text = Get-Content -LiteralPath $Path -Raw
    if ([string]::IsNullOrWhiteSpace($Text)) {
        return ""
    }

    $Compact = ($Text -replace '\s+', ' ').Trim()
    if ($Compact.Length -gt $MaxChars) {
        return $Compact.Substring(0, $MaxChars)
    }
    return $Compact
}

function Export-ClaudeStdoutArtifact {
    param(
        [Parameter(Mandatory = $true)][string] $StdoutPath,
        [Parameter(Mandatory = $true)][string] $OutputPath,
        [Parameter(Mandatory = $true)][string] $ClaudeOutputFormat
    )

    if ($ClaudeOutputFormat -ne "json") {
        Copy-Item -LiteralPath $StdoutPath -Destination $OutputPath -Force
        return $null
    }

    $Raw = Get-Content -LiteralPath $StdoutPath -Raw
    if ([string]::IsNullOrWhiteSpace($Raw)) {
        throw "Claude JSON output was empty: $StdoutPath"
    }

    $Payload = $Raw | ConvertFrom-Json -ErrorAction Stop
    $PayloadNames = $Payload.PSObject.Properties.Name
    $ResultText = if ($PayloadNames -contains "result") {
        [string]$Payload.result
    } elseif ($PayloadNames -contains "text") {
        [string]$Payload.text
    } else {
        $Raw
    }

    Set-Content -LiteralPath $OutputPath -Value $ResultText -Encoding UTF8
    return $Payload
}

function Join-CommandArguments {
    param([Parameter(Mandatory = $true)][string[]] $Arguments)

    $Quoted = foreach ($Argument in $Arguments) {
        if ($Argument -match '^[A-Za-z0-9_./:=,\\-]+$') {
            $Argument
        } else {
            '"' + ($Argument -replace '"', '\"') + '"'
        }
    }
    return ($Quoted -join " ")
}

function Get-ClaudePayloadFromStdout {
    param(
        [AllowEmptyString()][string] $StdoutPath,
        [Parameter(Mandatory = $true)][string] $ClaudeOutputFormat
    )

    if ($ClaudeOutputFormat -ne "json") {
        return $null
    }
    if ([string]::IsNullOrWhiteSpace($StdoutPath) -or -not (Test-Path -LiteralPath $StdoutPath -PathType Leaf)) {
        return $null
    }

    $Raw = Get-Content -LiteralPath $StdoutPath -Raw
    if ([string]::IsNullOrWhiteSpace($Raw)) {
        return $null
    }

    try {
        return ($Raw | ConvertFrom-Json -ErrorAction Stop)
    } catch {
        return $null
    }
}

function Test-ClaudeMaxTurnResult {
    param($Payload)

    if ($null -eq $Payload) {
        return $false
    }

    $Names = $Payload.PSObject.Properties.Name
    if ($Names -contains "subtype" -and ([string]$Payload.subtype) -eq "error_max_turns") {
        return $true
    }
    if ($Names -contains "terminal_reason" -and ([string]$Payload.terminal_reason) -eq "max_turns") {
        return $true
    }
    return $false
}

function Get-ClaudeSessionId {
    param($Payload)

    if ($null -eq $Payload) {
        return $null
    }

    $Names = $Payload.PSObject.Properties.Name
    if ($Names -contains "session_id" -and -not [string]::IsNullOrWhiteSpace([string]$Payload.session_id)) {
        return [string]$Payload.session_id
    }
    return $null
}

function Invoke-ClaudeRun {
    param(
        [Parameter(Mandatory = $true)][string] $Executable,
        [Parameter(Mandatory = $true)][string] $TaskPrompt,
        [Parameter(Mandatory = $true)][int] $TimeoutSeconds,
        [Parameter(Mandatory = $true)][int] $MaxTurns,
        [Parameter(Mandatory = $true)][string] $Model,
        [Parameter(Mandatory = $true)][string] $Effort,
        [Parameter(Mandatory = $true)][string] $ClaudeOutputFormat,
        [Parameter(Mandatory = $true)][string] $PermissionMode,
        [Parameter(Mandatory = $true)][string] $ToolProfile,
        [Parameter(Mandatory = $true)][string[]] $AllowedTools,
        [Parameter(Mandatory = $true)][string[]] $AddDir,
        [Parameter(Mandatory = $true)][string] $StdoutPath,
        [Parameter(Mandatory = $true)][string] $StderrPath,
        [switch] $NoSessionPersistence,
        [AllowEmptyString()][string] $ResumeSessionId = ""
    )

    $StartInfo = [System.Diagnostics.ProcessStartInfo]::new()
    $StartInfo.FileName = $Executable
    $StartInfo.UseShellExecute = $false
    $StartInfo.RedirectStandardInput = $true
    $StartInfo.RedirectStandardOutput = $true
    $StartInfo.RedirectStandardError = $true
    $StartInfo.CreateNoWindow = $true

    $Arguments = @("-p")
    if ($NoSessionPersistence) {
        $Arguments += "--no-session-persistence"
    }
    if (-not [string]::IsNullOrWhiteSpace($ResumeSessionId)) {
        $Arguments += "--resume"
        $Arguments += $ResumeSessionId
    }
    $Arguments += @("--permission-mode", $PermissionMode)
    if ($MaxTurns -gt 0) {
        $Arguments += "--max-turns"
        $Arguments += "$MaxTurns"
    }
    $Arguments += @("--model", $Model, "--effort", $Effort, "--output-format", $ClaudeOutputFormat)
    if ($PermissionMode -eq "bypassPermissions") {
        $Arguments += "--allow-dangerously-skip-permissions"
    }
    if ($ToolProfile -eq "FullOperator") {
        $Arguments += "--tools"
        $Arguments += "default"
    } else {
        $Arguments += "--allowedTools"
        $Arguments += ($AllowedTools -join ",")
    }
    foreach ($Dir in $AddDir) {
        $Arguments += "--add-dir"
        $Arguments += $Dir
    }
    $StartInfo.Arguments = Join-CommandArguments -Arguments $Arguments

    $Process = [System.Diagnostics.Process]::new()
    $Process.StartInfo = $StartInfo

    $null = $Process.Start()
    $Process.StandardInput.Write($TaskPrompt)
    $Process.StandardInput.Close()

    $StdoutTask = $Process.StandardOutput.ReadToEndAsync()
    $StderrTask = $Process.StandardError.ReadToEndAsync()

    # TimeoutSeconds <= 0 means an explicit unbounded run: block until the process exits with no wall-clock guard.
    if ($TimeoutSeconds -le 0) {
        $Process.WaitForExit()
        Set-Content -LiteralPath $StdoutPath -Value $StdoutTask.Result -Encoding UTF8
        Set-Content -LiteralPath $StderrPath -Value $StderrTask.Result -Encoding UTF8
        return [pscustomobject]@{
            Success = $Process.ExitCode -eq 0
            TimedOut = $false
            ExitCode = $Process.ExitCode
            StdoutPath = $StdoutPath
            StderrPath = $StderrPath
        }
    }

    if (-not $Process.WaitForExit($TimeoutSeconds * 1000)) {
        try {
            $Process.Kill($true)
        } catch {
            try {
                $Process.Kill()
            } catch {
            }
        }
        $Process.WaitForExit()
        Set-Content -LiteralPath $StdoutPath -Value $StdoutTask.Result -Encoding UTF8
        Set-Content -LiteralPath $StderrPath -Value (($StderrTask.Result), "Timed out after $TimeoutSeconds seconds.") -Encoding UTF8
        return [pscustomobject]@{
            Success = $false
            TimedOut = $true
            ExitCode = $null
            StdoutPath = $StdoutPath
            StderrPath = $StderrPath
        }
    }

    Set-Content -LiteralPath $StdoutPath -Value $StdoutTask.Result -Encoding UTF8
    Set-Content -LiteralPath $StderrPath -Value $StderrTask.Result -Encoding UTF8
    return [pscustomobject]@{
        Success = $Process.ExitCode -eq 0
        TimedOut = $false
        ExitCode = $Process.ExitCode
        StdoutPath = $StdoutPath
        StderrPath = $StderrPath
    }
}

function Invoke-ClaudeAttempt {
    param(
        [Parameter(Mandatory = $true)][string] $Executable,
        [Parameter(Mandatory = $true)][string] $TaskPrompt,
        [Parameter(Mandatory = $true)][int] $Attempt,
        [Parameter(Mandatory = $true)][int] $TimeoutSeconds,
        [Parameter(Mandatory = $true)][int] $MaxTurns,
        [Parameter(Mandatory = $true)][int] $MaxTurnContinuations,
        [Parameter(Mandatory = $true)][string] $Model,
        [Parameter(Mandatory = $true)][string] $Effort,
        [Parameter(Mandatory = $true)][string] $ClaudeOutputFormat,
        [Parameter(Mandatory = $true)][string] $PermissionMode,
        [Parameter(Mandatory = $true)][string] $ToolProfile,
        [Parameter(Mandatory = $true)][string[]] $AllowedTools,
        [Parameter(Mandatory = $true)][string[]] $AddDir,
        [Parameter(Mandatory = $true)][string] $AttemptRoot,
        [switch] $NoSessionPersistence
    )

    $StdoutExtension = if ($ClaudeOutputFormat -eq "json") { "json" } else { "md" }
    $StdoutPath = Join-Path $AttemptRoot "stdout_attempt$Attempt.$StdoutExtension"
    $StderrPath = Join-Path $AttemptRoot "stderr_attempt$Attempt.txt"

    $Result = Invoke-ClaudeRun `
        -Executable $Executable `
        -TaskPrompt $TaskPrompt `
        -TimeoutSeconds $TimeoutSeconds `
        -MaxTurns $MaxTurns `
        -Model $Model `
        -Effort $Effort `
        -ClaudeOutputFormat $ClaudeOutputFormat `
        -PermissionMode $PermissionMode `
        -ToolProfile $ToolProfile `
        -AllowedTools $AllowedTools `
        -AddDir $AddDir `
        -StdoutPath $StdoutPath `
        -StderrPath $StderrPath `
        -NoSessionPersistence:$NoSessionPersistence `
        -ResumeSessionId ""

    $Payload = Get-ClaudePayloadFromStdout -StdoutPath $Result.StdoutPath -ClaudeOutputFormat $ClaudeOutputFormat
    $ContinuationAttempted = $false
    $ContinuationCount = 0

    # Session resume is impossible without persistence, so only auto-continue when sessions are kept.
    if (-not $NoSessionPersistence) {
        while ((Test-ClaudeMaxTurnResult -Payload $Payload) -and ($ContinuationCount -lt $MaxTurnContinuations)) {
            $SessionId = Get-ClaudeSessionId -Payload $Payload
            if ([string]::IsNullOrWhiteSpace($SessionId)) {
                break
            }

            $ContinuationAttempted = $true
            $ContinuationCount++
            $ContStdoutPath = Join-Path $AttemptRoot "stdout_attempt${Attempt}_cont$ContinuationCount.$StdoutExtension"
            $ContStderrPath = Join-Path $AttemptRoot "stderr_attempt${Attempt}_cont$ContinuationCount.txt"

            $Result = Invoke-ClaudeRun `
                -Executable $Executable `
                -TaskPrompt "Please continue the task from where you stopped. Complete the remaining work and produce the final required output." `
                -TimeoutSeconds $TimeoutSeconds `
                -MaxTurns $MaxTurns `
                -Model $Model `
                -Effort $Effort `
                -ClaudeOutputFormat $ClaudeOutputFormat `
                -PermissionMode $PermissionMode `
                -ToolProfile $ToolProfile `
                -AllowedTools $AllowedTools `
                -AddDir $AddDir `
                -StdoutPath $ContStdoutPath `
                -StderrPath $ContStderrPath `
                -NoSessionPersistence:$NoSessionPersistence `
                -ResumeSessionId $SessionId

            $Payload = Get-ClaudePayloadFromStdout -StdoutPath $Result.StdoutPath -ClaudeOutputFormat $ClaudeOutputFormat
        }
    }

    # A run that still reports max-turns after exhausting continuations is not a usable success.
    if (Test-ClaudeMaxTurnResult -Payload $Payload) {
        $Result.Success = $false
    }

    $Result | Add-Member -NotePropertyName MaxTurnContinuationAttempted -NotePropertyValue $ContinuationAttempted
    $Result | Add-Member -NotePropertyName MaxTurnContinuationCount -NotePropertyValue $ContinuationCount
    return $Result
}

if ([string]::IsNullOrWhiteSpace($Effort)) {
    $Effort = if ($Mode -eq "Review") { "low" } else { "high" }
}

if ([string]::IsNullOrWhiteSpace($ToolProfile)) {
    $ToolProfile = if ($Mode -eq "Operator") { "FullOperator" } else { "ReadOnly" }
}

if ($ToolProfile -eq "FullOperator" -and $Mode -ne "Operator") {
    throw "FailureKind=ClaudeProcessFailed; FullOperator tool profile is only valid with -Mode Operator."
}

if ($ToolProfile -eq "FullOperator" -and $PSBoundParameters.ContainsKey("PermissionMode") -eq $false) {
    $PermissionMode = "bypassPermissions"
}

if ($ToolProfile -eq "FullOperator" -and $PermissionMode -eq "plan") {
    throw "FailureKind=ClaudeProcessFailed; FullOperator runs must not use Claude plan mode."
}

# Resolve the effective per-attempt timeout. -1 is the unset sentinel; 0 means unbounded.
# FullOperator implementation phases default to unbounded so long authoring/build/capture
# work is not killed mid-run; pass an explicit -TimeoutSeconds N to intentionally timebox a
# probe. Read-only profiles keep the short 180s guard.
if ($TimeoutSeconds -lt 0) {
    $TimeoutSeconds = if ($ToolProfile -eq "FullOperator") { 0 } else { 180 }
}

$BaselineTools = @("Read", "Grep", "Glob")
$UnexpectedTools = @($AllowedTools | Where-Object { $BaselineTools -notcontains $_ })
if ($ToolProfile -eq "ReadOnly" -and $UnexpectedTools.Count -gt 0 -and -not $AllowBroaderTools) {
    throw "FailureKind=ClaudeProcessFailed; Broader Claude tools requested without -AllowBroaderTools: $($UnexpectedTools -join ', '). Baseline is Read,Grep,Glob."
}

# Resolve effective session persistence. Persistence mainly supports max-turn --resume
# auto-continuation, so it defaults off when there is no max-turn cap to resume (-MaxTurns <= 0)
# and on when -MaxTurns > 0. Defaulting off for unbounded runs also avoids resume failures on
# modified thinking/redacted_thinking blocks. -NoSessionPersistence always forces it off;
# -SessionPersistence forces it on (the explicit opt-in for -MaxTurns <= 0). The two switches conflict.
if ($NoSessionPersistence -and $SessionPersistence) {
    throw "FailureKind=ClaudeProcessFailed; -NoSessionPersistence and -SessionPersistence are mutually exclusive."
}
$EffectiveSessionPersistence = if ($NoSessionPersistence) {
    $false
} elseif ($SessionPersistence) {
    $true
} else {
    $MaxTurns -gt 0
}
$SessionPersistenceSource = if ($NoSessionPersistence) {
    "forced off by -NoSessionPersistence"
} elseif ($SessionPersistence) {
    "forced on by -SessionPersistence"
} elseif ($MaxTurns -gt 0) {
    "default on (MaxTurns > 0 can --resume on max-turn)"
} else {
    "default off (MaxTurns <= 0 has no max-turn resume to support)"
}

# Effective capability fields. FullOperator gets the full Claude Code tool surface, not just Read,Grep,Glob.
$EffectiveToolSurface = if ($ToolProfile -eq "FullOperator") {
    "default (full Claude Code tool surface: edits, shell, configured MCP/editor tools)"
} else {
    ($AllowedTools -join ",")
}
$MutatingCapability = ($ToolProfile -eq "FullOperator")
$ApprovalRequired = ($ToolProfile -eq "FullOperator")
$ApiKeyBillingBlocked = (-not $AllowApiKeyBilling)
$TimeoutPolicy = if ($TimeoutSeconds -le 0) { "Unbounded (no wall-clock guard)" } else { "$TimeoutSeconds s per attempt" }
$MaxTurnPolicy = if ($MaxTurns -le 0) { "Uncapped (no --max-turns cap)" } else { "$MaxTurns max turns per attempt" }
$CodexApprovalStatus = Get-CodexApprovalStatus -ApprovalPath $CodexApprovalPath

if ($Preflight) {
    $PreflightReport = @"
CLAUDE HELPER PREFLIGHT (no Claude run performed)
Mode: $Mode
ToolProfile: $ToolProfile
ApprovalRequired: $ApprovalRequired
CodexApprovalPath: $(if ([string]::IsNullOrWhiteSpace($CodexApprovalPath)) { "(none)" } else { $CodexApprovalPath })
CodexApprovalStatus: $CodexApprovalStatus
TimeoutPolicy: $TimeoutPolicy
MaxTurnPolicy: $MaxTurnPolicy
PermissionMode: $PermissionMode
Model: $Model
Effort: $Effort
EffectiveToolSurface: $EffectiveToolSurface
MutatingCapability: $MutatingCapability
AllowedTools: $($AllowedTools -join ",")
AddDir: $($AddDir -join "; ")
ApiKeyBillingBlocked: $ApiKeyBillingBlocked
SessionPersistence: $EffectiveSessionPersistence
SessionPersistenceSource: $SessionPersistenceSource
Attempts: $Attempts
ClaudeOutputFormat: $ClaudeOutputFormat
"@
    Write-Output $PreflightReport
    return
}

if ([string]::IsNullOrWhiteSpace($PromptPath) -or -not (Test-Path -LiteralPath $PromptPath -PathType Leaf)) {
    throw "Prompt file not found: $PromptPath"
}

$ResolvedCodexApprovalPath = $null
if ($ToolProfile -eq "FullOperator") {
    $ResolvedCodexApprovalPath = Assert-CodexApproval -ApprovalPath $CodexApprovalPath
}

Assert-NoAnthropicApiKey -AllowApiKeyBilling:$AllowApiKeyBilling
$ClaudePath = Resolve-ClaudeCliPath -RequestedPath $ClaudeExe
$ClaudeVersion = Get-ClaudeCliVersion -Executable $ClaudePath
if (-not $AllowApiKeyBilling) {
    $null = Assert-ClaudeSubscriptionAuth -Executable $ClaudePath
}

$ResolvedPromptPath = (Resolve-Path -LiteralPath $PromptPath).Path
$ResolvedOutputRoot = $OutputRoot
if (-not [System.IO.Path]::IsPathRooted($ResolvedOutputRoot)) {
    $ResolvedOutputRoot = Join-Path (Get-Location).Path $ResolvedOutputRoot
}

$SafeTaskName = ($TaskName -replace '[^A-Za-z0-9_.-]+', '_').Trim('_')
if ([string]::IsNullOrWhiteSpace($SafeTaskName)) {
    $SafeTaskName = "DirectRead"
}
$Stamp = Get-Date -Format "yyyyMMddTHHmmss"
$RunDir = Join-Path $ResolvedOutputRoot "$Stamp-$SafeTaskName-$Mode"
$AttemptRoot = Join-Path $RunDir "attempts"
$null = New-Item -ItemType Directory -Force -Path $AttemptRoot

$RawPrompt = Get-Content -LiteralPath $ResolvedPromptPath -Raw
$Header = if ($Mode -eq "Review") {
@"
You are Claude directly reviewing T66 repo context. You may inspect files with the allowed read-only tools, but you must not edit files, run shell commands, invoke Unreal Python, automate editors, or perform production asset writes.

Start your response immediately with one strict verdict line:
Verdict: APPROVE
Verdict: REVISE
Verdict: NEEDS_HUMAN_DECISION
Verdict: BLOCK

Then explain blockers, major issues, minor issues, questions, required verification, and rationale. If this reviews an operator artifact, identify whether the artifact is safe to rely on. Operator artifacts are not greenlights by themselves.

"@
} elseif ($ToolProfile -eq "FullOperator") {
@"
You are Claude acting as the full-access T66 Operator.

Codex approval artifact: $ResolvedCodexApprovalPath

Codex has approved you to make changes inside the approved task contract and scope. You may use the normal Claude Code tool surface available in this environment, including file edits, shell commands, and configured MCP/editor tools such as Blender or other available MCP servers, when they are needed for the approved task.

You must stop and report `Codex Approval Required:` before any material scope expansion, destructive operation, credential or billing change, git commit, git push, git tag, git reset, git clean, broad Git/LFS scan over Unreal binary asset folders, or any action that contradicts `AGENTS.md` or folder-owned instructions. If you are unsure whether an action is inside the approved scope, stop and request Codex approval instead of doing it.

Verification freshness: if the user explicitly asks for current compile, run, capture, test, or editor verification, you must attempt that exact current verification now unless it is physically impossible. Recent or prior evidence does not satisfy an explicit current-verification request; if you cannot run it, say so explicitly instead of substituting older evidence.

Your output is an Operator work artifact and is not a greenlight. Codex will validate your actual changes, run or review verification, and write the final user-facing report.

"@
} else {
@"
You are Claude acting as a direct-read T66 operator. This is an operator artifact, not a review greenlight. Inspect files with the allowed read-only tools and produce a concrete proposal, findings, or handoff for Codex to validate. Do not edit files, run shell commands, invoke Unreal Python, automate editors, or perform production asset writes.

Verification freshness: if the user explicitly asks for current compile, run, capture, test, or editor verification, that requires the full Operator tool surface; do not claim a current-verification request is satisfied by recent or prior evidence. State plainly that this read-only profile cannot run it and that a full Operator run is required.

"@
}

if (-not [string]::IsNullOrWhiteSpace($ReviewedOperatorRun)) {
    $Header += "Reviewed operator run: $ReviewedOperatorRun`n`n"
}

$TaskPrompt = $Header + $RawPrompt
$EffectivePromptPath = Join-Path $RunDir "effective_prompt.md"
Set-Content -LiteralPath $EffectivePromptPath -Value $TaskPrompt -Encoding UTF8

$SuccessfulAttempt = $null
$AttemptResults = @()
for ($Attempt = 1; $Attempt -le $Attempts; ++$Attempt) {
    $Result = Invoke-ClaudeAttempt `
        -Executable $ClaudePath `
        -TaskPrompt $TaskPrompt `
        -Attempt $Attempt `
        -TimeoutSeconds $TimeoutSeconds `
        -MaxTurns $MaxTurns `
        -MaxTurnContinuations $MaxTurnContinuations `
        -Model $Model `
        -Effort $Effort `
        -ClaudeOutputFormat $ClaudeOutputFormat `
        -PermissionMode $PermissionMode `
        -ToolProfile $ToolProfile `
        -AllowedTools $AllowedTools `
        -AddDir $AddDir `
        -AttemptRoot $AttemptRoot `
        -NoSessionPersistence:(-not $EffectiveSessionPersistence)

    $AttemptResults += $Result
    if (-not $Result.Success) {
        continue
    }

    $AttemptOutputPath = $Result.StdoutPath
    if ($ClaudeOutputFormat -eq "json") {
        $AttemptOutputPath = Join-Path $AttemptRoot "stdout_attempt$Attempt.result.md"
        try {
            $null = Export-ClaudeStdoutArtifact -StdoutPath $Result.StdoutPath -OutputPath $AttemptOutputPath -ClaudeOutputFormat $ClaudeOutputFormat
            $Result | Add-Member -NotePropertyName ResultOutputPath -NotePropertyValue $AttemptOutputPath
        } catch {
            $Result.Success = $false
            $Result | Add-Member -NotePropertyName FailureKind -NotePropertyValue "ClaudeMalformedJson"
            $Result | Add-Member -NotePropertyName OutputParseError -NotePropertyValue $_.Exception.Message
            continue
        }
    }

    if ($Mode -eq "Review") {
        $ParsedAttemptVerdict = Get-ClaudeReviewVerdict -ReviewPath $AttemptOutputPath
        $Result | Add-Member -NotePropertyName Greenlit -NotePropertyValue $ParsedAttemptVerdict.Greenlit
        $Result | Add-Member -NotePropertyName Verdict -NotePropertyValue $ParsedAttemptVerdict.Verdict
        $Result | Add-Member -NotePropertyName VerdictLine -NotePropertyValue $ParsedAttemptVerdict.VerdictLine
        $Result | Add-Member -NotePropertyName OutcomeKind -NotePropertyValue $ParsedAttemptVerdict.OutcomeKind
        $Result | Add-Member -NotePropertyName FailureKind -NotePropertyValue $ParsedAttemptVerdict.FailureKind

        if ($ParsedAttemptVerdict.OutcomeKind -ne "ClaudeMalformedVerdict") {
            $SuccessfulAttempt = $Result
            break
        }
    } else {
        $SuccessfulAttempt = $Result
        break
    }
}

if (-not $SuccessfulAttempt) {
    $Summary = $AttemptResults | ForEach-Object {
        "attempt stdout=$($_.StdoutPath) stderr=$($_.StderrPath) timeout=$($_.TimedOut) exit=$($_.ExitCode) stdoutSnippet='$(Get-FileSnippet -Path $_.StdoutPath)' stderrSnippet='$(Get-FileSnippet -Path $_.StderrPath)'"
    }
    $FailureKind = "ClaudeProcessFailed"
    $AnyMaxTurns = $false
    foreach ($AttemptResult in $AttemptResults) {
        $Text = "$(Get-FileSnippet -Path $AttemptResult.StdoutPath) $(Get-FileSnippet -Path $AttemptResult.StderrPath)"
        if (Test-ClaudeUnavailableSignal -Text $Text) {
            $FailureKind = "ClaudeUnavailable"
            break
        }
        if ($AttemptResult.PSObject.Properties.Name -contains "FailureKind" -and $AttemptResult.FailureKind -eq "ClaudeMalformedJson") {
            $FailureKind = "ClaudeMalformedJson"
        }
        if ($Mode -eq "Review" -and $AttemptResult.PSObject.Properties.Name -contains "OutcomeKind" -and $AttemptResult.OutcomeKind -eq "ClaudeMalformedVerdict") {
            $FailureKind = "ClaudeMalformedVerdict"
        }
        if ($AttemptResult.PSObject.Properties.Name -contains "MaxTurnContinuationAttempted" -and $AttemptResult.MaxTurnContinuationAttempted) {
            $AnyMaxTurns = $true
        }
    }

    $FailureManifestPath = Join-Path $RunDir "manifest.json"
    $FailureManifest = [ordered]@{
        ArtifactKind = "ClaudeHelperFailed"
        Mode = $Mode
        Model = $Model
        Effort = $Effort
        ClaudeOutputFormat = $ClaudeOutputFormat
        PermissionMode = $PermissionMode
        ToolProfile = $ToolProfile
        EffectiveToolSurface = $EffectiveToolSurface
        MutatingCapability = $MutatingCapability
        ApprovalRequired = $ApprovalRequired
        CodexApprovalPath = $ResolvedCodexApprovalPath
        AllowedTools = $AllowedTools
        AddDir = $AddDir
        PromptPath = $ResolvedPromptPath
        EffectivePromptPath = $EffectivePromptPath
        AttemptsRequested = $Attempts
        TimeoutSeconds = $TimeoutSeconds
        MaxTurns = $MaxTurns
        MaxTurnContinuations = $MaxTurnContinuations
        SessionPersistence = $EffectiveSessionPersistence
        SessionPersistenceSource = $SessionPersistenceSource
        NoSessionPersistence = (-not $EffectiveSessionPersistence)
        ClaudePath = $ClaudePath
        ClaudeVersion = $ClaudeVersion
        FailureKind = $FailureKind
        MaxTurnContinuationAttempted = $AnyMaxTurns
        Attempts = @($AttemptResults | ForEach-Object {
            [ordered]@{
                StdoutPath = $_.StdoutPath
                StderrPath = $_.StderrPath
                TimedOut = $_.TimedOut
                ExitCode = $_.ExitCode
                Success = $_.Success
                MaxTurnContinuationAttempted = if ($_.PSObject.Properties.Name -contains "MaxTurnContinuationAttempted") { $_.MaxTurnContinuationAttempted } else { $false }
                MaxTurnContinuationCount = if ($_.PSObject.Properties.Name -contains "MaxTurnContinuationCount") { $_.MaxTurnContinuationCount } else { 0 }
            }
        })
        OutcomeKind = "ClaudeHelperFailed"
    }
    $FailureManifest | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $FailureManifestPath -Encoding UTF8

    throw "FailureKind=$FailureKind; Claude helper $Mode failed after $Attempts attempt(s). Manifest: $FailureManifestPath. Effective prompt artifact: $EffectivePromptPath. $($Summary -join '; ')"
}

$OutputPath = Join-Path $RunDir "claude_direct_read_$($Mode.ToLowerInvariant()).md"
$null = Export-ClaudeStdoutArtifact -StdoutPath $SuccessfulAttempt.StdoutPath -OutputPath $OutputPath -ClaudeOutputFormat $ClaudeOutputFormat

$ParsedVerdict = $null
if ($Mode -eq "Review") {
    $ParsedVerdict = Get-ClaudeReviewVerdict -ReviewPath $OutputPath
}

$ArtifactKind = if ($Mode -eq "Operator" -and $ToolProfile -eq "FullOperator") { "OperatorWorkArtifactRequiresValidation" } elseif ($Mode -eq "Operator") { "OperatorArtifactNotGreenlight" } else { "ReviewArtifact" }
$ManifestPath = Join-Path $RunDir "manifest.json"
$Manifest = [ordered]@{
    ArtifactKind = $ArtifactKind
    Mode = $Mode
    Model = $Model
    Effort = $Effort
    ClaudeOutputFormat = $ClaudeOutputFormat
    PermissionMode = $PermissionMode
    ToolProfile = $ToolProfile
    EffectiveToolSurface = $EffectiveToolSurface
    MutatingCapability = $MutatingCapability
    ApprovalRequired = $ApprovalRequired
    CodexApprovalPath = $ResolvedCodexApprovalPath
    AllowedTools = $AllowedTools
    AddDir = $AddDir
    PromptPath = $ResolvedPromptPath
    EffectivePromptPath = $EffectivePromptPath
    OutputPath = $OutputPath
    ReviewedOperatorRun = if ([string]::IsNullOrWhiteSpace($ReviewedOperatorRun)) { $null } else { $ReviewedOperatorRun }
    AttemptsRequested = $Attempts
    TimeoutSeconds = $TimeoutSeconds
    MaxTurns = $MaxTurns
    MaxTurnContinuations = $MaxTurnContinuations
    SessionPersistence = $EffectiveSessionPersistence
    SessionPersistenceSource = $SessionPersistenceSource
    NoSessionPersistence = (-not $EffectiveSessionPersistence)
    MaxTurnContinuationAttempted = if ($SuccessfulAttempt.PSObject.Properties.Name -contains "MaxTurnContinuationAttempted") { $SuccessfulAttempt.MaxTurnContinuationAttempted } else { $false }
    ClaudePath = $ClaudePath
    ClaudeVersion = $ClaudeVersion
    SuccessfulStdoutPath = $SuccessfulAttempt.StdoutPath
    SuccessfulStderrPath = $SuccessfulAttempt.StderrPath
    ClaudeJsonPath = if ($ClaudeOutputFormat -eq "json") { $SuccessfulAttempt.StdoutPath } else { $null }
    Greenlit = if ($ParsedVerdict) { $ParsedVerdict.Greenlit } else { $false }
    Verdict = if ($ParsedVerdict) { $ParsedVerdict.Verdict } else { $null }
    VerdictLine = if ($ParsedVerdict) { $ParsedVerdict.VerdictLine } else { $null }
    OutcomeKind = if ($ParsedVerdict) { $ParsedVerdict.OutcomeKind } else { "ClaudeOperatorArtifact" }
    FailureKind = if ($ParsedVerdict) { $ParsedVerdict.FailureKind } else { $null }
}
$Manifest | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $ManifestPath -Encoding UTF8
$RetentionPath = Join-Path $RunDir ".report-run.json"
([ordered]@{
    kind = "ClaudeDirectReadRun"
    expiresAfterDays = 15
    createdAt = $Stamp
    durableSummary = $ManifestPath
} | ConvertTo-Json -Depth 4) | Set-Content -LiteralPath $RetentionPath -Encoding UTF8

[pscustomobject]@{
    RunDir = $RunDir
    ManifestPath = $ManifestPath
    RetentionPath = $RetentionPath
    OutputPath = $OutputPath
    ArtifactKind = $ArtifactKind
    Mode = $Mode
    Model = $Model
    Effort = $Effort
    ClaudeOutputFormat = $ClaudeOutputFormat
    PermissionMode = $PermissionMode
    ToolProfile = $ToolProfile
    EffectiveToolSurface = $EffectiveToolSurface
    MutatingCapability = $MutatingCapability
    ApprovalRequired = $ApprovalRequired
    CodexApprovalPath = $ResolvedCodexApprovalPath
    AllowedTools = ($AllowedTools -join ",")
    ReviewedOperatorRun = if ([string]::IsNullOrWhiteSpace($ReviewedOperatorRun)) { $null } else { $ReviewedOperatorRun }
    Greenlit = if ($ParsedVerdict) { $ParsedVerdict.Greenlit } else { $false }
    Verdict = if ($ParsedVerdict) { $ParsedVerdict.Verdict } else { $null }
    VerdictLine = if ($ParsedVerdict) { $ParsedVerdict.VerdictLine } else { $null }
    OutcomeKind = if ($ParsedVerdict) { $ParsedVerdict.OutcomeKind } else { "ClaudeOperatorArtifact" }
    FailureKind = if ($ParsedVerdict) { $ParsedVerdict.FailureKind } else { $null }
}
