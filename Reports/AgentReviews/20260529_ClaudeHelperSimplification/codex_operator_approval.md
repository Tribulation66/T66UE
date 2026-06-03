Codex Approval: APPROVE

# Codex Operator Approval: Claude Helper Simplification

## Approval

Codex approves Claude Operator implementation for the helper/process simplification pass.

## Goal

Implement the agreed simplifications for the Claude helper and T66 process docs:

- Keep a timeout safety guard by default, but make long or intentionally unbounded Claude runs explicit and discoverable.
- Make the effective Claude tool surface clear in helper manifests.
- Make read-only Operator usage obvious and approval-free.
- Make explicit current verification requirements harder to accidentally satisfy with stale or recent evidence.
- Make helper-produced Claude token data available to the completion artifact/user-facing accounting flow.
- Clarify host/native goal state as telemetry only for T66 process control.
- Clarify proof/evidence preservation before deleting or recommending deletion of raw proof folders.

## Operator

- Operator: Claude
- Validator/Finisher: Codex
- Tool profile: FullOperator
- Permission mode: non-plan full operator

## Approved Write Scope

- `C:\UE\T66\Scripts\Invoke-ClaudeDirectRead.ps1`
- `C:\UE\T66\Scripts\Invoke-ClaudeReadOnlyOperator.ps1` if a wrapper script is useful
- `C:\UE\T66\OPERATOR_VALIDATOR_PROTOCOL.md`
- `C:\UE\T66\AGENTS.md` only where directly related to the above simplifications
- `C:\UE\T66\Reports\AgentReviews\20260529_ClaudeHelperSimplification\*`

## Out Of Scope

- Unreal gameplay, assets, Niagara, Blender, staged builds, and production content.
- Any broad refactor of the Operator/Validator protocol unrelated to the approved simplifications.
- Changing model selection, API billing policy, `.t66\operator-state.json`, or usage denominator inference.
- Removing the full Operator approval gate for mutating work.
- Removing the API-key billing guard.
- Removing the plan-mode prohibition for full Operator work.

## Required Implementation Behavior

1. Timeout behavior
   - Do not remove the default wall-clock guard.
   - Raise the normal full Operator default to a realistic value, preferably `1200` seconds.
   - Allow an explicit unbounded run with `-TimeoutSeconds 0` or a clear equivalent switch.
   - Make bounds/defaults discoverable without running Claude, preferably with a `-Preflight` mode.

2. Preflight
   - Add a helper preflight path that prints effective mode/profile, approval requirement, timeout policy, permission mode, model, tool surface, and add-dir roots.
   - Preflight must not invoke Claude.
   - If feasible, preflight should not require a real prompt file.

3. Tool profile reporting
   - FullOperator manifests must not misleadingly imply only `Read,Grep,Glob` are effective.
   - Add clear fields such as `EffectiveToolSurface`, `MutatingCapability`, and `ApprovalRequired`.
   - Preserve existing fields if compatibility is useful, but make the effective capability unambiguous.

4. Read-only Operator
   - Make approval-free read-only Operator runs easy to invoke and document.
   - Either add a small wrapper script or an obvious helper parameter/path.

5. Verification freshness
   - Add a hard rule to the helper prompt/docs: when the user explicitly asks for current verification, the Operator must attempt that exact current verification unless physically impossible; recent evidence cannot satisfy it.

6. Token accounting
   - The helper should expose Claude token usage after the run in a way the final report can consume.
   - If appending to the Claude output is safe, append a small helper token summary after the run; otherwise create a sidecar token summary artifact and record it in the manifest.

7. Evidence preservation
   - Process docs should list proof classes to preserve before recommending deletion of raw proof/worktree folders: manifests, command logs, stdout/stderr, screenshots, videos, captures, JSON/CSV metrics, decision blocks, completion packets, and other task-specific acceptance evidence.

## Validation Expected From Operator

- Run PowerShell syntax validation on changed scripts.
- Run helper preflight in at least one read-only and one full-operator shape without invoking Claude.
- If a wrapper script is added, validate it with preflight.
- Report exact files changed and commands run.
- Save a concise completion packet to:
  `C:\UE\T66\Reports\AgentReviews\20260529_ClaudeHelperSimplification\claude_completion_packet.md`
