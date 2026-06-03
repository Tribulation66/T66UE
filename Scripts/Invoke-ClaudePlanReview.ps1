<#
.SYNOPSIS
Runs a subscription-backed Claude Code independent-answer or cross-review pass.

.DESCRIPTION
This helper is intentionally read-only from Claude's point of view. In
IndependentAnswer mode, it sends the original user prompt/task contract to the
local Claude Code CLI so Claude can inspect the repo read-only and draft its own
answer before Codex finalizes. In CrossReview mode, it sends a Codex draft, the
original prompt, and optionally Claude's independent answer so Claude can compare
the two and return concrete corrections. Artifacts are written under
Saved\AgentReviews by default.

The script refuses to run when ANTHROPIC_API_KEY is present unless
-AllowApiKeyBilling is explicitly passed. The default T66 workflow is to use
the user's Claude subscription login, not Anthropic API billing.

Malformed Claude result output is not a greenlight and is not a Codex
fallback trigger. This helper reports typed outcomes so the active agent can
apply the repository policy without treating malformed text as availability
failure.

The helper uses Claude JSON output by default so token usage can be captured in
the returned object and a `claude_tokens.json` sidecar. The default max-turn cap
is intentionally not tiny; use at least 5 turns for plan-review runs.

.EXAMPLE
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\Invoke-ClaudePlanReview.ps1 `
  -Mode IndependentAnswer `
  -OriginalPromptPath C:\UE\T66\Saved\AgentReviews\original_prompt.md

.EXAMPLE
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\Invoke-ClaudePlanReview.ps1 `
  -Mode CrossReview `
  -OriginalPromptPath C:\UE\T66\Saved\AgentReviews\original_prompt.md `
  -PacketPath C:\UE\T66\Saved\AgentReviews\codex_draft.md `
  -IndependentAnswerPath C:\UE\T66\Saved\AgentReviews\claude_independent_answer.md
#>

[CmdletBinding()]
param(
    [ValidateSet("IndependentAnswer", "CrossReview")]
    [string] $Mode = "CrossReview",

    [string] $OriginalPromptPath = "",

    [string] $PacketPath = "",

    [string] $IndependentAnswerPath = "",

    [string] $OutputRoot = "C:\UE\T66\Saved\AgentReviews",

    [int] $Pass = 1,

    [ValidateRange(5, 100)]
    [int] $MaxTurns = 20,

    [ValidateRange(1, 3)]
    [int] $Attempts = 3,

    [ValidateRange(15, 600)]
    [int] $TimeoutSeconds = 120,

    [string] $Model = "claude-opus-4-8",

    [ValidateSet("low", "medium", "high", "xhigh", "max")]
    [string] $Effort = "low",

    [ValidateSet("text", "json")]
    [string] $ClaudeOutputFormat = "json",

    [string] $ClaudeExe = "",

    [switch] $AllowApiKeyBilling,

    [string] $ParseReviewPathOnly = "",

    [string] $ParseAuthStatusJsonOnly = ""
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

    $ResolvedCandidates = @()
    foreach ($Candidate in $Candidates) {
        if ([string]::IsNullOrWhiteSpace($Candidate)) {
            continue
        }

        if (Test-Path -LiteralPath $Candidate -PathType Leaf) {
            $ResolvedCandidates += (Resolve-Path -LiteralPath $Candidate).Path
        }
    }

    foreach ($Candidate in ($ResolvedCandidates | Select-Object -Unique)) {
        if ($Candidate -notmatch "\\WindowsApps\\Claude_") {
            return $Candidate
        }
    }

    throw "FailureKind=ClaudeUnavailable; Claude Code CLI was not found. The WindowsApps Claude desktop app shim is not valid for T66 review automation. Expected a CLI such as $LocalUserCli."
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
        throw "FailureKind=ClaudeProcessFailed; ANTHROPIC_API_KEY is set in scope(s): $Scopes. Unset it before running Claude review so Claude Code uses the subscription login instead of API billing."
    }
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
        throw "FailureKind=ClaudeUnavailable; Claude auth status failed. Confirm Claude Code is logged in with the user's subscription before running review."
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

    $Patterns = @(
        "subscription session limit",
        "session limit",
        "usage limit",
        "limit reached",
        "not logged in",
        "login required",
        "authentication required",
        "please log in"
    )

    foreach ($Pattern in $Patterns) {
        if ($Text.IndexOf($Pattern, [StringComparison]::OrdinalIgnoreCase) -ge 0) {
            return $true
        }
    }

    return $false
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

function Get-ClaudeAttemptFailureKind {
    param([Parameter(Mandatory = $true)] $AttemptResult)

    if ($AttemptResult.TimedOut) {
        return "ClaudeProcessFailed"
    }

    $Text = ""
    foreach ($Path in @($AttemptResult.StdoutPath, $AttemptResult.StderrPath)) {
        if (-not [string]::IsNullOrWhiteSpace($Path) -and (Test-Path -LiteralPath $Path -PathType Leaf)) {
            $Text += "`n"
            $Text += Get-Content -LiteralPath $Path -Raw
        }
    }

    if (Test-ClaudeUnavailableSignal -Text $Text) {
        return "ClaudeUnavailable"
    }

    return "ClaudeProcessFailed"
}

function Get-ClaudeTokenTotal {
    param($Payload)

    if ($null -eq $Payload) {
        return $null
    }

    $Total = [int64]0
    $HasValue = $false

    $PayloadNames = $Payload.PSObject.Properties.Name
    if ($PayloadNames -contains "modelUsage" -and $null -ne $Payload.modelUsage) {
        foreach ($ModelEntry in $Payload.modelUsage.PSObject.Properties) {
            $Usage = $ModelEntry.Value
            if ($null -eq $Usage) {
                continue
            }

            $UsageNames = $Usage.PSObject.Properties.Name
            foreach ($PropertyName in @("inputTokens", "outputTokens", "cacheCreationInputTokens", "cacheReadInputTokens")) {
                if ($UsageNames -contains $PropertyName) {
                    $PropertyValue = $Usage.PSObject.Properties[$PropertyName].Value
                    if ($null -eq $PropertyValue) {
                        continue
                    }
                    $Total += [int64]$PropertyValue
                    $HasValue = $true
                }
            }
        }

        if ($HasValue) {
            return $Total
        }
    }

    if ($PayloadNames -contains "usage" -and $null -ne $Payload.usage) {
        $Usage = $Payload.usage
        $UsageNames = $Usage.PSObject.Properties.Name
        foreach ($PropertyName in @("input_tokens", "output_tokens", "cache_creation_input_tokens", "cache_read_input_tokens")) {
            if ($UsageNames -contains $PropertyName) {
                $PropertyValue = $Usage.PSObject.Properties[$PropertyName].Value
                if ($null -eq $PropertyValue) {
                    continue
                }
                $Total += [int64]$PropertyValue
                $HasValue = $true
            }
        }
    }

    if ($HasValue) {
        return $Total
    }

    return $null
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

function Get-ClaudeReviewResult {
    param([Parameter(Mandatory = $true)][string] $ReviewPath)

    if (-not (Test-Path -LiteralPath $ReviewPath -PathType Leaf)) {
        throw "Review artifact not found: $ReviewPath"
    }

    $Raw = Get-Content -LiteralPath $ReviewPath -Raw
    $ResultLine = $null
    $ReviewResult = $null
    $Greenlit = $false
    $ResultSource = $null

    if (-not [string]::IsNullOrEmpty($Raw)) {
        $Lines = $Raw -split "`r?`n"
        foreach ($Line in $Lines) {
            if ($null -eq $Line) {
                continue
            }

            $Candidate = $Line
            if ($Candidate.Length -gt 0 -and [int][char]$Candidate[0] -eq 0xFEFF) {
                $Candidate = $Candidate.Substring(1)
            }

            if ([string]::IsNullOrWhiteSpace($Candidate)) {
                continue
            }

            $TrimmedEnd = $Candidate.TrimEnd()
            if ($TrimmedEnd -cmatch '(?i)\bResult:\s*(OK|NEEDS_USER)\b') {
                $ResultLine = "Result: $($Matches[1].ToUpperInvariant())"
                $ReviewResult = $Matches[1]
                $ReviewResult = $ReviewResult.ToUpperInvariant()
                $Greenlit = $ReviewResult -ceq "OK"
                $ResultSource = "Explicit"
                break
            }
        }

        if (-not $ReviewResult) {
            $Compact = ($Raw -replace '\s+', ' ').Trim()
            $Lower = $Compact.ToLowerInvariant()
            $NeedsUserPatterns = @(
                "needs_user",
                "needs user",
                "needs your attention",
                "requires your attention",
                "only the user can",
                "user-only",
                "is blocked",
                "we are blocked",
                "blocked by",
                "hard blocker",
                "cannot proceed",
                "can't proceed",
                "requires approval",
                "needs approval",
                "unavailable required tool",
                "missing required tool"
            )
            foreach ($Pattern in $NeedsUserPatterns) {
                if ($Lower.IndexOf($Pattern, [StringComparison]::OrdinalIgnoreCase) -ge 0) {
                    $ResultLine = "Result: NEEDS_USER"
                    $ReviewResult = "NEEDS_USER"
                    $Greenlit = $false
                    $ResultSource = "Inferred"
                    break
                }
            }
        }

        if (-not $ReviewResult) {
            $Compact = ($Raw -replace '\s+', ' ').Trim()
            $Lower = $Compact.ToLowerInvariant()
            $OkPatterns = @(
                "task appears complete",
                "appears complete",
                "looks good",
                "no blocker",
                "no blockers",
                "no user action",
                "no user-only decision",
                "no user decision",
                "none required",
                "safe to proceed",
                "usable",
                "can handle",
                "can resolve",
                "handled internally",
                "already implemented"
            )
            foreach ($Pattern in $OkPatterns) {
                if ($Lower.IndexOf($Pattern, [StringComparison]::OrdinalIgnoreCase) -ge 0) {
                    $ResultLine = "Result: OK"
                    $ReviewResult = "OK"
                    $Greenlit = $true
                    $ResultSource = "Inferred"
                    break
                }
            }
        }
    }

    $OutcomeKind = "ClaudeMalformedResult"
    $FailureKind = "ClaudeMalformedResult"
    if ($ReviewResult) {
        $OutcomeKind = "ClaudeValidResult"
        $FailureKind = $null
    }

    return [pscustomobject]@{
        Greenlit = $Greenlit
        Result = $ReviewResult
        ResultLine = $ResultLine
        ResultSource = $ResultSource
        OutcomeKind = $OutcomeKind
        FailureKind = $FailureKind
    }
}

if (-not [string]::IsNullOrWhiteSpace($ParseReviewPathOnly)) {
    $Parsed = Get-ClaudeReviewResult -ReviewPath $ParseReviewPathOnly
    [pscustomobject]@{
        ReviewPath = (Resolve-Path -LiteralPath $ParseReviewPathOnly).Path
        Greenlit = $Parsed.Greenlit
        Result = $Parsed.Result
        ResultLine = $Parsed.ResultLine
        ResultSource = $Parsed.ResultSource
        OutcomeKind = $Parsed.OutcomeKind
        FailureKind = $Parsed.FailureKind
    }
    return
}

if (-not [string]::IsNullOrWhiteSpace($ParseAuthStatusJsonOnly)) {
    $Parsed = Get-ClaudeSubscriptionAuthStatus -RawJson $ParseAuthStatusJsonOnly
    $Parsed
    return
}

if ($Mode -eq "IndependentAnswer") {
    if ([string]::IsNullOrWhiteSpace($OriginalPromptPath) -or -not (Test-Path -LiteralPath $OriginalPromptPath -PathType Leaf)) {
        throw "Independent answer input not found: $OriginalPromptPath"
    }
} elseif ($Mode -eq "CrossReview") {
    if ([string]::IsNullOrWhiteSpace($PacketPath) -or -not (Test-Path -LiteralPath $PacketPath -PathType Leaf)) {
        throw "Cross-review draft input not found: $PacketPath"
    }

    if (-not [string]::IsNullOrWhiteSpace($OriginalPromptPath) -and -not (Test-Path -LiteralPath $OriginalPromptPath -PathType Leaf)) {
        throw "Original prompt input not found: $OriginalPromptPath"
    }

    if (-not [string]::IsNullOrWhiteSpace($IndependentAnswerPath) -and -not (Test-Path -LiteralPath $IndependentAnswerPath -PathType Leaf)) {
        throw "Independent answer input not found: $IndependentAnswerPath"
    }
}

Assert-NoAnthropicApiKey -AllowApiKeyBilling:$AllowApiKeyBilling
$ClaudePath = Resolve-ClaudeCliPath -RequestedPath $ClaudeExe
$ClaudeVersion = Get-ClaudeCliVersion -Executable $ClaudePath
if (-not $AllowApiKeyBilling) {
    $null = Assert-ClaudeSubscriptionAuth -Executable $ClaudePath
}

$ResolvedPacketPath = ""
if (-not [string]::IsNullOrWhiteSpace($PacketPath)) {
    $ResolvedPacketPath = (Resolve-Path -LiteralPath $PacketPath).Path
}

$ResolvedOriginalPromptPath = ""
if (-not [string]::IsNullOrWhiteSpace($OriginalPromptPath)) {
    $ResolvedOriginalPromptPath = (Resolve-Path -LiteralPath $OriginalPromptPath).Path
}

$ResolvedIndependentAnswerPath = ""
if (-not [string]::IsNullOrWhiteSpace($IndependentAnswerPath)) {
    $ResolvedIndependentAnswerPath = (Resolve-Path -LiteralPath $IndependentAnswerPath).Path
}

$ResolvedOutputRoot = $OutputRoot
if (-not [System.IO.Path]::IsPathRooted($ResolvedOutputRoot)) {
    $ResolvedOutputRoot = Join-Path (Get-Location).Path $ResolvedOutputRoot
}

$Stamp = Get-Date -Format "yyyyMMddTHHmmss"
$RunDir = Join-Path $ResolvedOutputRoot "$Stamp-$Mode-pass$Pass"
$null = New-Item -ItemType Directory -Force -Path $RunDir

$OriginalPrompt = ""
if (-not [string]::IsNullOrWhiteSpace($ResolvedOriginalPromptPath)) {
    $OriginalPrompt = Get-Content -LiteralPath $ResolvedOriginalPromptPath -Raw
}

$Packet = ""
if (-not [string]::IsNullOrWhiteSpace($ResolvedPacketPath)) {
    $Packet = Get-Content -LiteralPath $ResolvedPacketPath -Raw
}

$IndependentAnswer = ""
if (-not [string]::IsNullOrWhiteSpace($ResolvedIndependentAnswerPath)) {
    $IndependentAnswer = Get-Content -LiteralPath $ResolvedIndependentAnswerPath -Raw
}

if ($Mode -eq "IndependentAnswer") {
    $Prompt = @"
You are Claude providing the independent Validator answer for the T66 Unreal project.

Rules:
- Include a clear `Result: OK` or `Result: NEEDS_USER` line near the top.
- Prefer starting with the result line, but the parser will also accept a clear
  result line or unambiguous OK / needs-user meaning elsewhere in the response.
- Do not edit files.
- Do not run mutating commands.
- Inspect the live repo read-only when repo context is needed.
- Treat Codex as the Operator/final router and you as the independent Validator.
- Produce the answer you would give to the user from the current evidence.
- Look for scope constraints, repo instructions, user-only decisions, missing evidence, and caveats.
- Ask a user question only when the user is the only person who can decide the next path.
- Keep the answer practical. Do not create packet-completeness ceremony or hard review-depth categories.

Your result should be one of these two lines:
Result: OK
Result: NEEDS_USER

After that result line, return a concise Markdown answer with exactly these headings:
Independent Answer
Evidence Checked
Questions Or Blockers
Caveats

Result meanings:
- OK: the models can handle the prompt internally. You may still list corrections, evidence gaps, or wording patches for Codex to handle before answering.
- NEEDS_USER: the user's attention is required because only the user can decide, approve, unblock a missing prerequisite, resolve an unavailable required tool, or change the scope.

Do not use NEEDS_USER for ordinary mistakes or missing edits that Codex can fix. List those inside the answer body and keep the result OK.

Independent answer scope:
- Original prompt path: $ResolvedOriginalPromptPath
- Output scope: read-only independent Validator answer for comparison with Codex's draft.

<original_prompt>
$OriginalPrompt
</original_prompt>
"@
} else {
    $OriginalPromptBlock = if ([string]::IsNullOrWhiteSpace($OriginalPrompt)) { "(not provided)" } else { $OriginalPrompt }
    $IndependentAnswerBlock = if ([string]::IsNullOrWhiteSpace($IndependentAnswer)) { "(not provided)" } else { $IndependentAnswer }

    $Prompt = @"
You are Claude cross-reviewing a Codex draft for the T66 Unreal project.

Rules:
- Include a clear `Result: OK` or `Result: NEEDS_USER` line near the top.
- Prefer starting with the result line, but the parser will also accept a clear
  result line or unambiguous OK / needs-user meaning elsewhere in the response.
- Do not edit files.
- Do not run mutating commands.
- Treat Codex as the Operator/final router and you as the Validator.
- Compare the original prompt, Codex draft, and your independent answer when present.
- Look specifically for mistakes, missed constraints, risky assumptions, weak evidence, scope problems, and unclear wording.
- Patch the answer text when the fix is straightforward.
- Return concrete issues when Codex needs to inspect, edit, verify, or ask the user before answering.
- Ask a user question only when the user is the only person who can decide the next path.
- Keep the review concise and practical. Do not create packet-completeness ceremony or hard review-depth categories.

Your result should be one of these two lines:
Result: OK
Result: NEEDS_USER

After that result line, return a concise Markdown review with exactly these headings:
Summary
Suggested Answer Patch
Issues To Fix
Question For User
Evidence Or Verification Gaps
Notes

Result meanings:
- OK: the models can handle the prompt internally. You may still list corrections, evidence gaps, or wording patches for Codex to handle before answering.
- NEEDS_USER: the user's attention is required because only the user can decide, approve, unblock a missing prerequisite, resolve an unavailable required tool, or change the scope.

Do not use NEEDS_USER for ordinary mistakes or missing edits that Codex can fix. List those inside the review body and keep the result OK.

Review scope:
- Original prompt path: $ResolvedOriginalPromptPath
- Codex draft path: $ResolvedPacketPath
- Independent answer path: $ResolvedIndependentAnswerPath
- Output scope: targeted cross-review and answer patch only.

<original_prompt>
$OriginalPromptBlock
</original_prompt>

<validator_independent_answer>
$IndependentAnswerBlock
</validator_independent_answer>

<codex_draft>
$Packet
</codex_draft>
"@
}

$PromptPath = Join-Path $RunDir "claude_review_prompt_pass$Pass.md"
$ReviewPath = Join-Path $RunDir "claude_review_pass$Pass.md"
$AttemptRoot = Join-Path $RunDir "attempts"
$null = New-Item -ItemType Directory -Force -Path $AttemptRoot
Set-Content -LiteralPath $PromptPath -Value $Prompt -Encoding UTF8

function Invoke-ClaudeAttempt {
    param(
        [Parameter(Mandatory = $true)][string] $Executable,
        [Parameter(Mandatory = $true)][string] $ReviewPrompt,
        [Parameter(Mandatory = $true)][int] $Attempt,
        [Parameter(Mandatory = $true)][int] $TimeoutSeconds,
        [Parameter(Mandatory = $true)][int] $MaxTurns,
        [Parameter(Mandatory = $true)][string] $Model,
        [Parameter(Mandatory = $true)][string] $Effort,
        [Parameter(Mandatory = $true)][string] $ClaudeOutputFormat,
        [Parameter(Mandatory = $true)][string] $AttemptRoot,
        [AllowEmptyString()][string] $AllowedTools = ""
    )

    $StdoutPath = Join-Path $AttemptRoot "stdout_attempt$Attempt.md"
    $StderrPath = Join-Path $AttemptRoot "stderr_attempt$Attempt.txt"

    $StartInfo = [System.Diagnostics.ProcessStartInfo]::new()
    $StartInfo.FileName = $Executable
    $StartInfo.UseShellExecute = $false
    $StartInfo.RedirectStandardInput = $true
    $StartInfo.RedirectStandardOutput = $true
    $StartInfo.RedirectStandardError = $true
    $StartInfo.CreateNoWindow = $true
    $AllowedToolsArgs = ""
    if (-not [string]::IsNullOrWhiteSpace($AllowedTools)) {
        $AllowedToolsArgs = " --allowedTools $AllowedTools"
    }
    $StartInfo.Arguments = "-p --no-session-persistence --permission-mode default --max-turns $MaxTurns --model $Model --effort $Effort --output-format $ClaudeOutputFormat$AllowedToolsArgs"

    $Process = [System.Diagnostics.Process]::new()
    $Process.StartInfo = $StartInfo

    $null = $Process.Start()
    $Process.StandardInput.Write($ReviewPrompt)
    $Process.StandardInput.Close()

    $StdoutTask = $Process.StandardOutput.ReadToEndAsync()
    $StderrTask = $Process.StandardError.ReadToEndAsync()

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

    $Stdout = $StdoutTask.Result
    $Stderr = $StderrTask.Result
    Set-Content -LiteralPath $StdoutPath -Value $Stdout -Encoding UTF8
    Set-Content -LiteralPath $StderrPath -Value $Stderr -Encoding UTF8

    return [pscustomobject]@{
        Success = $Process.ExitCode -eq 0
        TimedOut = $false
        ExitCode = $Process.ExitCode
        StdoutPath = $StdoutPath
        StderrPath = $StderrPath
    }
}

$SuccessfulAttempt = $null
$AttemptResults = @()
$AllowedToolsForMode = "Read,Grep,Glob,LS"
for ($Attempt = 1; $Attempt -le $Attempts; ++$Attempt) {
    $Result = Invoke-ClaudeAttempt `
        -Executable $ClaudePath `
        -ReviewPrompt $Prompt `
        -Attempt $Attempt `
        -TimeoutSeconds $TimeoutSeconds `
        -MaxTurns $MaxTurns `
        -Model $Model `
        -Effort $Effort `
        -ClaudeOutputFormat $ClaudeOutputFormat `
        -AttemptRoot $AttemptRoot `
        -AllowedTools $AllowedToolsForMode
    $AttemptResults += $Result

    if ($Result.Success) {
        $AttemptReviewPath = $Result.StdoutPath
        $AttemptPayload = $null
        if ($ClaudeOutputFormat -eq "json") {
            $AttemptReviewPath = Join-Path $AttemptRoot "stdout_attempt$Attempt.result.md"
            try {
                $AttemptPayload = Export-ClaudeStdoutArtifact -StdoutPath $Result.StdoutPath -OutputPath $AttemptReviewPath -ClaudeOutputFormat $ClaudeOutputFormat
                $Result | Add-Member -NotePropertyName ResultOutputPath -NotePropertyValue $AttemptReviewPath
                $Result | Add-Member -NotePropertyName ClaudeTokensSpent -NotePropertyValue (Get-ClaudeTokenTotal -Payload $AttemptPayload)
            } catch {
                $Result.Success = $false
                $Result | Add-Member -NotePropertyName FailureKind -NotePropertyValue "ClaudeMalformedJson"
                $Result | Add-Member -NotePropertyName OutputParseError -NotePropertyValue $_.Exception.Message
                continue
            }
        } else {
            $Result | Add-Member -NotePropertyName ResultOutputPath -NotePropertyValue $AttemptReviewPath
            $Result | Add-Member -NotePropertyName ClaudeTokensSpent -NotePropertyValue $null
        }

        $ParsedAttemptResult = Get-ClaudeReviewResult -ReviewPath $AttemptReviewPath
        $Result | Add-Member -NotePropertyName Greenlit -NotePropertyValue $ParsedAttemptResult.Greenlit
        $Result | Add-Member -NotePropertyName Result -NotePropertyValue $ParsedAttemptResult.Result
        $Result | Add-Member -NotePropertyName ResultLine -NotePropertyValue $ParsedAttemptResult.ResultLine
        $Result | Add-Member -NotePropertyName ResultSource -NotePropertyValue $ParsedAttemptResult.ResultSource
        $Result | Add-Member -NotePropertyName OutcomeKind -NotePropertyValue $ParsedAttemptResult.OutcomeKind
        $Result | Add-Member -NotePropertyName FailureKind -NotePropertyValue $ParsedAttemptResult.FailureKind

        if ($ParsedAttemptResult.OutcomeKind -ne "ClaudeMalformedResult") {
            $SuccessfulAttempt = $Result
            break
        }
    }
}

if (-not $SuccessfulAttempt) {
    $Summary = $AttemptResults | ForEach-Object {
        "attempt stdout=$($_.StdoutPath) stderr=$($_.StderrPath) timeout=$($_.TimedOut) exit=$($_.ExitCode) stdoutSnippet='$(Get-FileSnippet -Path $_.StdoutPath)' stderrSnippet='$(Get-FileSnippet -Path $_.StderrPath)'"
    }
    $FailureKind = "ClaudeProcessFailed"
    foreach ($AttemptResult in $AttemptResults) {
        if ((Get-ClaudeAttemptFailureKind -AttemptResult $AttemptResult) -eq "ClaudeUnavailable") {
            $FailureKind = "ClaudeUnavailable"
            break
        }
        if ($AttemptResult.PSObject.Properties.Name -contains "FailureKind" -and $AttemptResult.FailureKind -eq "ClaudeMalformedJson") {
            $FailureKind = "ClaudeMalformedJson"
        }
        if ($AttemptResult.PSObject.Properties.Name -contains "OutcomeKind" -and $AttemptResult.OutcomeKind -eq "ClaudeMalformedResult") {
            $FailureKind = "ClaudeMalformedResult"
        }
    }
    $FailureTokensSpent = $null
    foreach ($AttemptResult in $AttemptResults) {
        if ($AttemptResult.PSObject.Properties.Name -contains "ClaudeTokensSpent" -and $null -ne $AttemptResult.ClaudeTokensSpent) {
            $FailureTokensSpent = $AttemptResult.ClaudeTokensSpent
            continue
        }

        $AttemptPayload = Get-ClaudePayloadFromStdout -StdoutPath $AttemptResult.StdoutPath -ClaudeOutputFormat $ClaudeOutputFormat
        $AttemptTokens = Get-ClaudeTokenTotal -Payload $AttemptPayload
        if ($null -ne $AttemptTokens) {
            $FailureTokensSpent = $AttemptTokens
        }
    }
    $FailureTokenSummaryPath = Join-Path $RunDir "claude_tokens.json"
    ([ordered]@{
        ClaudeTokensSpent = $FailureTokensSpent
        Model = $Model
        Mode = $Mode
        FailureKind = $FailureKind
        CreatedAt = $Stamp
    } | ConvertTo-Json -Depth 8) | Set-Content -LiteralPath $FailureTokenSummaryPath -Encoding UTF8
    throw "FailureKind=$FailureKind; Claude review failed after $Attempts fresh attempt(s). Prompt artifact: $PromptPath. $($Summary -join '; ')"
}

$SuccessfulOutputPath = if ($SuccessfulAttempt.PSObject.Properties.Name -contains "ResultOutputPath") { $SuccessfulAttempt.ResultOutputPath } else { $SuccessfulAttempt.StdoutPath }
Copy-Item -LiteralPath $SuccessfulOutputPath -Destination $ReviewPath -Force
$ParsedResult = Get-ClaudeReviewResult -ReviewPath $ReviewPath
$ClaudePayload = Get-ClaudePayloadFromStdout -StdoutPath $SuccessfulAttempt.StdoutPath -ClaudeOutputFormat $ClaudeOutputFormat
$ClaudeTokensSpent = if ($SuccessfulAttempt.PSObject.Properties.Name -contains "ClaudeTokensSpent") { $SuccessfulAttempt.ClaudeTokensSpent } else { Get-ClaudeTokenTotal -Payload $ClaudePayload }
$TokenSummaryPath = Join-Path $RunDir "claude_tokens.json"
([ordered]@{
    ClaudeTokensSpent = $ClaudeTokensSpent
    Model = $Model
    Mode = $Mode
    ClaudeUsage = if ($ClaudePayload -and ($ClaudePayload.PSObject.Properties.Name -contains "usage")) { $ClaudePayload.usage } else { $null }
    ClaudeModelUsage = if ($ClaudePayload -and ($ClaudePayload.PSObject.Properties.Name -contains "modelUsage")) { $ClaudePayload.modelUsage } else { $null }
    CreatedAt = $Stamp
} | ConvertTo-Json -Depth 8) | Set-Content -LiteralPath $TokenSummaryPath -Encoding UTF8

[pscustomobject]@{
    Mode = $Mode
    ReviewPath = $ReviewPath
    PromptPath = $PromptPath
    OriginalPromptPath = $ResolvedOriginalPromptPath
    PacketPath = $ResolvedPacketPath
    IndependentAnswerPath = $ResolvedIndependentAnswerPath
    Pass = $Pass
    MaxTurns = $MaxTurns
    AttemptsRequested = $Attempts
    TimeoutSeconds = $TimeoutSeconds
    Model = $Model
    Effort = $Effort
    ClaudeOutputFormat = $ClaudeOutputFormat
    ClaudePath = $ClaudePath
    ClaudeVersion = $ClaudeVersion
    SuccessfulStdoutPath = $SuccessfulAttempt.StdoutPath
    SuccessfulStderrPath = $SuccessfulAttempt.StderrPath
    ClaudeTokensSpent = $ClaudeTokensSpent
    TokenSummaryPath = $TokenSummaryPath
    Greenlit = $ParsedResult.Greenlit
    Result = $ParsedResult.Result
    ResultLine = $ParsedResult.ResultLine
    ResultSource = $ParsedResult.ResultSource
    OutcomeKind = $ParsedResult.OutcomeKind
    FailureKind = $ParsedResult.FailureKind
}
