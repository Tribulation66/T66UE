You are Claude reviewing a Codex implementation or answer plan for the T66 Unreal project.

Rules:
- Start your response immediately with the verdict line. Do not write any
  preface, summary, confirmation, Markdown rule, or other text before it.
- Do not edit files.
- Do not run commands.
- Do not implement the plan.
- Review only the packet below.
- Be strict about contradictions with repo instructions, missing verification, unsafe scope, and unclear goals.
- Treat Codex as the implementer and you as the reviewer.

The first non-empty line of your review must be exactly one of these four lines:
Verdict: APPROVE
Verdict: REVISE
Verdict: NEEDS_HUMAN_DECISION
Verdict: BLOCK

After that verdict line, return a concise Markdown review with exactly these headings:
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
- Packet path: C:\UE\T66\Reports\AgentReviews\20260528_ClaudeUnrealNiagaraSmoke\review_packet_pass2.md
- Output scope: review of the packet below only.

<review_packet>
# Claude Unreal/Niagara Smoke Review Packet Pass 2

## Working Goal

Confirm whether Claude Code can directly access Unreal Editor/Niagara for T66 by designing and running a safe smoke test, without granting production writes unless a reviewed plan and user approval allow it.

## Pass 1 Review Result

- Artifact: `C:\UE\T66\Reports\AgentReviews\20260528_ClaudeUnrealNiagaraSmoke\20260528T205341-pass1\claude_review_pass1.md`
- Verdict: `REVISE`
- Major concerns:
  - Prove or test that `--allowedTools` remains the hard ceiling under `--permission-mode dontAsk`.
  - Confirm asserted Claude CLI flags exist in the installed version.
- Minor concerns:
  - Ensure wrapper command matches the allowed `Bash(...)` prefix exactly.
  - Add Unreal editor executable preflight.
  - Make failure classifications explicit.

## Current Evidence Added

`claude --help | Select-String -Pattern '--effort|--model|--no-session-persistence|--allowedTools|--tools|--disallowedTools|--permission-mode'` confirms the installed Claude CLI exposes:

- `--allowedTools`
- `--disallowedTools`
- `--effort`
- `--model`
- `--no-session-persistence`
- `--permission-mode`
- `--tools`

Path preflight run by Codex:

- `C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe`: exists.
- `C:\UE\T66\T66.uproject`: exists.

`ANTHROPIC_API_KEY` checked in Process/User/Machine: all unset.

## Revised Plan

### 1. Create report-owned smoke scripts

Create these files under `C:\UE\T66\Reports\AgentReviews\20260528_ClaudeUnrealNiagaraSmoke\`:

- `ClaudeUnrealNiagaraSmoke.py`
- `RunClaudeUnrealNiagaraSmoke.ps1`
- `claude_allowlist_probe_prompt.md`
- `claude_smoke_prompt.md`

The Unreal Python script is read-only against production assets and writes only:

- `unreal_niagara_smoke_report.json`

The wrapper writes only:

- `unreal_smoke_stdout.log`
- `unreal_smoke_stderr.log`
- `unreal_niagara_smoke_report.json`

No `unreal.EditorAssetLibrary.save_asset`, import task, asset creation, package deletion, or `Content/` writes are allowed.

### 2. Run a harmless Claude allowlist containment probe first

Use Claude Code with:

```powershell
--tools "Bash"
--allowedTools "Bash(powershell -NoProfile -Command Write-Output ALLOWED_CLAUDE_TOOL_PROBE*)"
--disallowedTools "Edit,Write"
--permission-mode dontAsk
```

Prompt Claude to do exactly two harmless attempts:

1. Run the allowed command:
   `powershell -NoProfile -Command Write-Output ALLOWED_CLAUDE_TOOL_PROBE`
2. Attempt the disallowed command:
   `powershell -NoProfile -Command Write-Output DISALLOWED_CLAUDE_TOOL_PROBE`

Pass condition:

- Claude stdout contains `ALLOWED_CLAUDE_TOOL_PROBE`.
- Claude stdout does **not** contain `DISALLOWED_CLAUDE_TOOL_PROBE`.
- Claude output clearly indicates the second command was denied, unavailable, refused, or not executed.

Failure classification:

- If `DISALLOWED_CLAUDE_TOOL_PROBE` appears, abort before Unreal and report that the bounded Claude shell profile is not safe to use.
- If the allowed command cannot run, abort before Unreal and report that Claude cannot be confirmed through this CLI profile.
- If Claude exits nonzero or the result is ambiguous, abort before Unreal and report the ambiguity.

This preflight only uses harmless stdout markers and does not touch project assets.

### 3. Run the Unreal/Niagara smoke only if containment passes

Use Claude Code with:

```powershell
--tools "Bash,Read"
--allowedTools "Read,Bash(powershell -NoProfile -ExecutionPolicy Bypass -File C:/UE/T66/Reports/AgentReviews/20260528_ClaudeUnrealNiagaraSmoke/RunClaudeUnrealNiagaraSmoke.ps1*)"
--disallowedTools "Edit,Write"
--permission-mode dontAsk
```

The exact command in the prompt and wrapper pattern both use the same forward-slash path:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File C:/UE/T66/Reports/AgentReviews/20260528_ClaudeUnrealNiagaraSmoke/RunClaudeUnrealNiagaraSmoke.ps1
```

The wrapper performs preflight checks for:

- `UnrealEditor-Cmd.exe` exists.
- `T66.uproject` exists.
- `ClaudeUnrealNiagaraSmoke.py` exists.

The wrapper enforces a timeout and writes `failure_kind` into `unreal_niagara_smoke_report.json` when possible:

- `PreflightMissingEditor`
- `PreflightMissingProject`
- `PreflightMissingScript`
- `UnrealTimedOut`
- `UnrealExitNonZero`
- `MissingReport`
- `InvalidReport`
- `SmokeAssertionsFailed`
- `Success`

### 4. Success definition

Only report success if:

- Claude's run artifact shows it invoked the wrapper.
- Wrapper exit code is zero.
- `unreal_niagara_smoke_report.json` exists.
- JSON says:
  - `success=true`
  - `unreal_python_live=true`
  - `engine_version` is non-empty
  - target asset exists and loads as a Niagara-related class, or Niagara API/registry signals are present if that specific target asset is missing.

Confirmed scope if successful:

- Claude Code can be given a bounded, reviewed profile that invokes Unreal editor commandlet automation.
- Claude can inspect Unreal/Niagara state through Unreal Python and report artifacts.

Still unconfirmed even on success:

- GUI Niagara editor operation.
- Visual inspection equivalent to seeing the Niagara editor viewport.
- Production Niagara/asset modification.
- Any unrestricted shell/editor permission.
- Replacement of Unreal-owned capture/video proof for visual acceptance.

## Why This Addresses Pass 1

- `dontAsk` containment is not assumed; it is tested first with harmless marker commands.
- `--tools "Bash"` and `--disallowedTools "Edit,Write"` shrink the exposed tool surface during the allowlist probe.
- The Unreal smoke only runs after the allowlist ceiling is proven.
- Installed CLI flag existence has been verified from `claude --help`.
- The wrapper command string and the `Bash(...)` allowlist pattern use the same prefix.
- Unreal executable/project/script preflight and failure classifications are explicit.

## Verification Plan

- Run `git diff --check -- Reports/AgentReviews/20260528_ClaudeUnrealNiagaraSmoke`.
- Run the allowlist containment probe and save stdout/stderr under the report folder.
- Only if containment passes, run the Claude Unreal/Niagara smoke and save stdout/stderr under the report folder.
- Inspect `unreal_niagara_smoke_report.json`.
- Run a narrow output inventory under `Reports/AgentReviews/20260528_ClaudeUnrealNiagaraSmoke`.
- Final answer must classify confirmed versus unconfirmed access.

## Review Request

Review whether this revised plan safely addresses the pass 1 objections and is sufficient to run the containment probe and, only if it passes, the Claude-run Unreal/Niagara smoke.

Required first line: `Verdict: APPROVE`, `Verdict: REVISE`, `Verdict: NEEDS_HUMAN_DECISION`, or `Verdict: BLOCK`.

</review_packet>
