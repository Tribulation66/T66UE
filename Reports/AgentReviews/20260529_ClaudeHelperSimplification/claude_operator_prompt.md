# Claude Operator Prompt: Claude Helper Simplification

You are Claude acting as the FullOperator for this approved implementation task. Codex is Validator/Finisher.

Codex approval artifact:

`C:\UE\T66\Reports\AgentReviews\20260529_ClaudeHelperSimplification\codex_operator_approval.md`

## Task

Implement the helper/process simplifications approved in the Codex approval artifact. Keep this scoped. Do not redesign the full Operator/Validator process.

## Files To Inspect First

- `C:\UE\T66\Scripts\Invoke-ClaudeDirectRead.ps1`
- `C:\UE\T66\OPERATOR_VALIDATOR_PROTOCOL.md`
- `C:\UE\T66\AGENTS.md`
- `C:\UE\T66\Reports\AGENTS.md`

## Required Changes

1. Timeout behavior
   - Do not remove the default timeout guard.
   - Raise the normal FullOperator default timeout to a realistic value, preferably `1200` seconds.
   - Allow explicit unbounded runs with `-TimeoutSeconds 0` or a clear equivalent switch.
   - Ensure `0` does not immediately time out.

2. Preflight
   - Add a helper preflight path that prints effective settings and exits without invoking Claude.
   - It should show at least: mode, tool profile, approval required, approval path status, timeout policy, max-turn policy, permission mode, model, effective tool surface, mutating capability, add-dir roots, and whether API-key billing is blocked.
   - If feasible, preflight should not require a real prompt file.

3. Manifest clarity
   - Add unambiguous manifest fields such as `EffectiveToolSurface`, `MutatingCapability`, and `ApprovalRequired`.
   - FullOperator should no longer appear read-only just because `AllowedTools` defaults to `Read,Grep,Glob`.
   - Keep backward-compatible fields when reasonable.

4. Read-only Operator ergonomics
   - Make approval-free read-only Operator use obvious. A small `Scripts\Invoke-ClaudeReadOnlyOperator.ps1` wrapper is acceptable if it forwards to `Invoke-ClaudeDirectRead.ps1 -Mode Operator -ToolProfile ReadOnly`.

5. Verification freshness
   - Add a hard prompt/doc rule: if the user explicitly asks for current compile/run/capture/test/editor verification, the Operator must attempt that exact current verification unless physically impossible; recent evidence cannot satisfy the requirement.

6. Token accounting
   - Make helper-produced `ClaudeTokensSpent` available after a run in the completion/report flow. If appending to the Claude output could break strict verdict parsing, create a sidecar token summary file and record it in the manifest.

7. Evidence preservation
   - Clarify in process docs that before deleting or recommending deletion of raw proof/worktree folders, durable evidence must preserve applicable proof classes: manifests, command logs, stdout/stderr, screenshots, videos, captures, JSON/CSV metrics, decision blocks, completion packets, and any task-specific acceptance evidence.

## Constraints

- Do not touch Unreal gameplay/assets/content.
- Do not remove the full Operator approval requirement for mutating work.
- Do not remove the API-key billing guard.
- Do not use Claude plan permission mode.
- Do not change `.t66\operator-state.json`.
- Keep edits small and direct.

## Validation

Run focused validation:

- PowerShell parse/syntax validation for every changed `.ps1`.
- Preflight for a read-only Operator shape.
- Preflight for a FullOperator shape with the approval artifact.
- If a wrapper script is added, preflight through the wrapper.

Do not run a real nested Claude implementation run unless needed; preflight should prove the helper routing without spending extra Claude tokens.

## Completion Packet

Write:

`C:\UE\T66\Reports\AgentReviews\20260529_ClaudeHelperSimplification\claude_completion_packet.md`

Include:

- Files changed.
- Exact behavior implemented.
- Validation commands and results.
- Any caveats for Codex.
