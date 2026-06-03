# Completion Packet: Claude Helper Simplification

Operator: Claude (FullOperator). Validator/Finisher: Codex.
Approval artifact: `Reports/AgentReviews/20260529_ClaudeHelperSimplification/codex_operator_approval.md` (first line `Codex Approval: APPROVE`, validated by helper as `Valid`).

## Files Changed

1. `Scripts/Invoke-ClaudeDirectRead.ps1` — timeout policy, preflight mode, manifest capability fields, token sidecar, verification-freshness prompt rules.
2. `Scripts/Invoke-ClaudeReadOnlyOperator.ps1` — new wrapper for approval-free read-only Operator runs.
3. `OPERATOR_VALIDATOR_PROTOCOL.md` — timeout/preflight/read-only docs, manifest field docs, verification-freshness invariant, evidence-preservation section.
4. `AGENTS.md` — read-only wrapper + preflight pointer; verification-freshness hard rule (directly related to the approved simplifications only).

## Exact Behavior Implemented

### 1. Timeout behavior
- Default guard retained. `-TimeoutSeconds` default is now the unset sentinel `-1`; effective default resolves to `1200` for FullOperator and `180` for read-only profiles.
- `ValidateRange` changed to `0..86400`. `-TimeoutSeconds 0` is an explicit unbounded run.
- In `Invoke-ClaudeRun`, `TimeoutSeconds -le 0` now calls `WaitForExit()` (blocking, no wall-clock kill) instead of `WaitForExit(0)`, so `0` no longer immediately times out.

### 2. Preflight
- New `-Preflight` switch prints effective config and returns before any Claude/CLI/auth/API-key call.
- Reports: mode, tool profile, approval required, approval-path status, timeout policy, max-turn policy, permission mode, model, effort, effective tool surface, mutating capability, allowed tools, add-dir roots, API-key-billing-blocked, session persistence, attempts, output format.
- Does not require a real prompt file (prompt-file check moved after the preflight return). Approval status is computed non-fatally via `Get-CodexApprovalStatus` (no throw on missing/invalid).

### 3. Manifest clarity
- Added `EffectiveToolSurface`, `MutatingCapability`, `ApprovalRequired` to both success and failure manifests and to the success output object.
- FullOperator now reports the full Claude Code tool surface + `MutatingCapability=True` instead of looking read-only from the legacy `AllowedTools=Read,Grep,Glob` default. `AllowedTools` retained for backward compatibility.

### 4. Read-only Operator ergonomics
- `Scripts/Invoke-ClaudeReadOnlyOperator.ps1` forwards supplied params and forces `-Mode Operator -ToolProfile ReadOnly`. Needs no approval artifact; cannot mutate.

### 5. Verification freshness
- Hard rule added to the FullOperator and read-only Operator prompt headers, to `OPERATOR_VALIDATOR_PROTOCOL.md` (Core Invariants), and to `AGENTS.md`: an explicit current verification request must be attempted now unless physically impossible; recent/prior evidence cannot satisfy it.

### 6. Token accounting
- Sidecar `claude_tokens.json` written on success and failure (carries `ClaudeTokensSpent`, model/mode/profile, usage/modelUsage on success, FailureKind on failure). Path recorded in manifests/output as `TokenSummaryPath`. Claude output is not appended to, preserving the strict first-line verdict contract.

### 7. Evidence preservation
- New "Evidence Preservation Before Deletion" subsection in `OPERATOR_VALIDATOR_PROTOCOL.md` lists proof classes (manifests, command logs, stdout/stderr, screenshots, videos, captures, JSON/CSV metrics, decision blocks, completion packets, token ledgers, task-specific acceptance evidence) that must be preserved before deleting/recommending deletion of raw proof/worktree folders.

## Validation Commands And Results

- Parse check (`[Parser]::ParseFile`) on both `.ps1`: `OK` for both, zero parse errors.
- Preflight A — read-only Operator: `ToolProfile=ReadOnly`, `ApprovalRequired=False`, `MutatingCapability=False`, `TimeoutPolicy=180 s`, `EffectiveToolSurface=Read,Grep,Glob`. No Claude run.
- Preflight B — FullOperator + approval: `ApprovalRequired=True`, `CodexApprovalStatus=Valid (...)`, `TimeoutPolicy=1200 s`, `MutatingCapability=True`, `PermissionMode=bypassPermissions`, full tool surface. No Claude run.
- Preflight C — wrapper `Invoke-ClaudeReadOnlyOperator.ps1 -Preflight`: identical read-only shape to A. No Claude run.
- Edge D — `-TimeoutSeconds 0`: `TimeoutPolicy=Unbounded (no wall-clock guard)`.
- Edge E — FullOperator preflight with no approval: `CodexApprovalStatus=NotProvided`, did not throw.
- Edge F — `-TimeoutSeconds 600` override: `TimeoutPolicy=600 s per attempt`.

No real nested Claude implementation run was performed; preflight proved helper routing without spending Claude tokens.

## Caveats For Codex
- No git commit/push/tag performed (out of scope; awaiting your validation).
- `-TimeoutSeconds` uses default `-1` as an unset sentinel; PowerShell does not apply `ValidateRange` to unbound default values, so `-1` is accepted only as the internal default and never as user input (range is `0..86400`).
- The unbounded path (`WaitForExit()` with no timeout) was validated only via preflight/static review, not by running an actual unbounded Claude process.
- `AGENTS.md` edits were intentionally limited to the verification-freshness rule and the helper preflight/read-only pointer, per the "only where directly related" scope. `Reports/AGENTS.md` was left untouched (not in approved write scope); evidence-preservation guidance went into `OPERATOR_VALIDATOR_PROTOCOL.md`.
