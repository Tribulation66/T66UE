<#
.SYNOPSIS
Runs a local Codex CLI review of a Codex plan packet.

.DESCRIPTION
This helper is the fallback review path when Claude Code is unavailable,
rate-limited, or otherwise blocked and the user has approved continuing with a
Codex CLI reviewer. It sends a read-only review packet to `codex exec` and
writes the review artifacts under Saved\AgentReviews by default.

.EXAMPLE
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\Invoke-CodexPlanReview.ps1 `
  -PacketPath C:\UE\T66\Saved\AgentReviews\current_plan.md
#>

[CmdletBinding()]
param(
    [string] $PacketPath = "",

    [string] $OutputRoot = "C:\UE\T66\Saved\AgentReviews",

    [int] $Pass = 1,

    [ValidateRange(1, 3)]
    [int] $Attempts = 2,

    [ValidateRange(15, 900)]
    [int] $TimeoutSeconds = 240,

    [string] $CodexExe = "",

    [AllowEmptyString()]
    [string] $Model = "",

    [string] $ParseReviewPathOnly = ""
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Resolve-CodexCliPath {
    param([string] $RequestedPath)

    $Candidates = @()

    if (-not [string]::IsNullOrWhiteSpace($RequestedPath)) {
        $Candidates += $RequestedPath
    }

    foreach ($Name in @("codex.cmd", "codex.exe", "codex.ps1", "codex")) {
        $Command = Get-Command $Name -ErrorAction SilentlyContinue
        if ($Command) {
            $Candidates += $Command.Source
        }
    }

    foreach ($Candidate in ($Candidates | Where-Object { -not [string]::IsNullOrWhiteSpace($_) } | Select-Object -Unique)) {
        if (Test-Path -LiteralPath $Candidate -PathType Leaf) {
            return (Resolve-Path -LiteralPath $Candidate).Path
        }
    }

    throw "Codex CLI was not found. Install or log in to the local Codex CLI before using the fallback reviewer."
}

function Get-CodexReviewVerdict {
    param([Parameter(Mandatory = $true)][string] $ReviewPath)

    if (-not (Test-Path -LiteralPath $ReviewPath -PathType Leaf)) {
        throw "Review artifact not found: $ReviewPath"
    }

    $Raw = Get-Content -LiteralPath $ReviewPath -Raw
    $VerdictLine = $null
    $Verdict = $null
    $Greenlit = $false

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

    return [pscustomobject]@{
        Greenlit = $Greenlit
        Verdict = $Verdict
        VerdictLine = $VerdictLine
    }
}

if (-not [string]::IsNullOrWhiteSpace($ParseReviewPathOnly)) {
    $Parsed = Get-CodexReviewVerdict -ReviewPath $ParseReviewPathOnly
    [pscustomobject]@{
        ReviewPath = (Resolve-Path -LiteralPath $ParseReviewPathOnly).Path
        Greenlit = $Parsed.Greenlit
        Verdict = $Parsed.Verdict
        VerdictLine = $Parsed.VerdictLine
    }
    return
}

if ([string]::IsNullOrWhiteSpace($PacketPath) -or -not (Test-Path -LiteralPath $PacketPath -PathType Leaf)) {
    throw "Review packet not found: $PacketPath"
}

$CodexPath = Resolve-CodexCliPath -RequestedPath $CodexExe
$ResolvedPacketPath = (Resolve-Path -LiteralPath $PacketPath).Path
$ResolvedOutputRoot = $OutputRoot
if (-not [System.IO.Path]::IsPathRooted($ResolvedOutputRoot)) {
    $ResolvedOutputRoot = Join-Path (Get-Location).Path $ResolvedOutputRoot
}

$Stamp = Get-Date -Format "yyyyMMddTHHmmss"
$RunDir = Join-Path $ResolvedOutputRoot "$Stamp-codex-pass$Pass"
$null = New-Item -ItemType Directory -Force -Path $RunDir

$Packet = Get-Content -LiteralPath $ResolvedPacketPath -Raw
$Prompt = @"
You are a separate Codex CLI reviewer checking a Codex implementation or answer plan for the T66 Unreal project.

Rules:
- Do not edit files.
- Do not implement the plan.
- Use only read-only inspection if you need to inspect the repository.
- Review only the packet below and the live repo facts needed to validate it.
- Be strict about contradictions with repo instructions, missing verification, unsafe scope, unclear goals, and careless or lazy reasoning by the implementing Codex agent.
- Treat the current chat Codex as the implementer and this Codex CLI run as the reviewer.

The first non-empty line of your review must be exactly one of:
Verdict: APPROVE
Verdict: REVISE
Verdict: NEEDS_HUMAN_DECISION
Verdict: BLOCK

After that verdict line, return a concise Markdown review with these headings:
Blockers
Major Issues
Minor Issues
Clarifying Questions
Required Verification
Rationale

Verdict meanings:
- APPROVE: the reviewed plan/output is safe for Codex to proceed to implementation under the reviewed scope. Codex should not ask for redundant manual user approval after APPROVE unless the user explicitly marked the work planning-only, asked Codex to stop before implementation, the packet has an unresolved user-only decision, or AGENTS/PPF requires explicit approval for a method substitution.
- REVISE: Codex can resolve the issue by improving the plan/output, inspecting more repo state, tightening verification, changing implementation approach, or otherwise doing more Codex-owned work. Codex should revise and rerun review.
- NEEDS_HUMAN_DECISION: the plan/output depends on product direction, vision, risk acceptance, scope choice, or another decision only the user can make. Codex should save a decision block, ask once, and stop until the user answers.
- BLOCK: the plan/output cannot safely proceed because of a hard blocker, missing prerequisite, external-state issue, unavailable credential/context, or contradiction that is not solved by normal Codex revision.

Review scope:
- Packet path: $ResolvedPacketPath
- Output scope: review of the packet below only.

<review_packet>
$Packet
</review_packet>
"@

$PromptPath = Join-Path $RunDir "codex_review_prompt_pass$Pass.md"
$ReviewPath = Join-Path $RunDir "codex_review_pass$Pass.md"
$AttemptRoot = Join-Path $RunDir "attempts"
$null = New-Item -ItemType Directory -Force -Path $AttemptRoot
Set-Content -LiteralPath $PromptPath -Value $Prompt -Encoding UTF8

function ConvertTo-ProcessArgument {
    param([Parameter(Mandatory = $true)][string] $Argument)

    if ($Argument -notmatch '[\s"]') {
        return $Argument
    }

    return '"' + ($Argument -replace '"', '\"') + '"'
}

function Invoke-CodexAttempt {
    param(
        [Parameter(Mandatory = $true)][string] $Executable,
        [Parameter(Mandatory = $true)][string] $ReviewPrompt,
        [Parameter(Mandatory = $true)][int] $Attempt,
        [Parameter(Mandatory = $true)][int] $TimeoutSeconds,
        [Parameter(Mandatory = $true)][string] $AttemptRoot,
        [AllowEmptyString()]
        [string] $Model = ""
    )

    $StdoutPath = Join-Path $AttemptRoot "stdout_attempt$Attempt.txt"
    $StderrPath = Join-Path $AttemptRoot "stderr_attempt$Attempt.txt"
    $LastMessagePath = Join-Path $AttemptRoot "last_message_attempt$Attempt.md"

    $StartInfo = [System.Diagnostics.ProcessStartInfo]::new()
    $Arguments = @()
    if ($Executable.EndsWith(".ps1", [StringComparison]::OrdinalIgnoreCase)) {
        $StartInfo.FileName = "powershell.exe"
        $Arguments += "-NoProfile"
        $Arguments += "-ExecutionPolicy"
        $Arguments += "Bypass"
        $Arguments += "-File"
        $Arguments += $Executable
    } else {
        $StartInfo.FileName = $Executable
    }

    $Arguments += "-C"
    $Arguments += "C:\UE\T66"
    $Arguments += "-s"
    $Arguments += "read-only"
    $Arguments += "-a"
    $Arguments += "never"
    $Arguments += "exec"
    $Arguments += "--color"
    $Arguments += "never"
    $Arguments += "--ephemeral"
    $Arguments += "--output-last-message"
    $Arguments += $LastMessagePath
    if (-not [string]::IsNullOrWhiteSpace($Model)) {
        $Arguments += "--model"
        $Arguments += $Model
    }
    $Arguments += "-"
    $StartInfo.Arguments = ($Arguments | ForEach-Object { ConvertTo-ProcessArgument $_ }) -join " "

    $StartInfo.UseShellExecute = $false
    $StartInfo.RedirectStandardInput = $true
    $StartInfo.RedirectStandardOutput = $true
    $StartInfo.RedirectStandardError = $true
    $StartInfo.CreateNoWindow = $true

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
            LastMessagePath = $LastMessagePath
        }
    }

    $Stdout = $StdoutTask.Result
    $Stderr = $StderrTask.Result
    Set-Content -LiteralPath $StdoutPath -Value $Stdout -Encoding UTF8
    Set-Content -LiteralPath $StderrPath -Value $Stderr -Encoding UTF8

    return [pscustomobject]@{
        Success = $Process.ExitCode -eq 0 -and (Test-Path -LiteralPath $LastMessagePath -PathType Leaf)
        TimedOut = $false
        ExitCode = $Process.ExitCode
        StdoutPath = $StdoutPath
        StderrPath = $StderrPath
        LastMessagePath = $LastMessagePath
    }
}

$SuccessfulAttempt = $null
$AttemptResults = @()
for ($Attempt = 1; $Attempt -le $Attempts; ++$Attempt) {
    $Result = Invoke-CodexAttempt `
        -Executable $CodexPath `
        -ReviewPrompt $Prompt `
        -Attempt $Attempt `
        -TimeoutSeconds $TimeoutSeconds `
        -AttemptRoot $AttemptRoot `
        -Model $Model
    $AttemptResults += $Result

    if ($Result.Success) {
        $SuccessfulAttempt = $Result
        break
    }
}

if (-not $SuccessfulAttempt) {
    $Summary = $AttemptResults | ForEach-Object {
        "attempt stdout=$($_.StdoutPath) stderr=$($_.StderrPath) last=$($_.LastMessagePath) timeout=$($_.TimedOut) exit=$($_.ExitCode)"
    }
    throw "Codex review failed after $Attempts fresh attempt(s). Prompt artifact: $PromptPath. $($Summary -join '; ')"
}

Copy-Item -LiteralPath $SuccessfulAttempt.LastMessagePath -Destination $ReviewPath -Force
$ParsedVerdict = Get-CodexReviewVerdict -ReviewPath $ReviewPath

[pscustomobject]@{
    ReviewPath = $ReviewPath
    PromptPath = $PromptPath
    Pass = $Pass
    AttemptsRequested = $Attempts
    TimeoutSeconds = $TimeoutSeconds
    CodexPath = $CodexPath
    SuccessfulStdoutPath = $SuccessfulAttempt.StdoutPath
    SuccessfulStderrPath = $SuccessfulAttempt.StderrPath
    SuccessfulLastMessagePath = $SuccessfulAttempt.LastMessagePath
    Greenlit = $ParsedVerdict.Greenlit
    Verdict = $ParsedVerdict.Verdict
    VerdictLine = $ParsedVerdict.VerdictLine
}
