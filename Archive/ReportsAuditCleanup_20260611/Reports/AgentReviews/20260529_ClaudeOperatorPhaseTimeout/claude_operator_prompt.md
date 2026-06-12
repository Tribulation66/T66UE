# Claude Operator Prompt: Phase-Bounded FullOperator Timeout Policy

## Working Task

The user approved two process improvements:

1. Update the Operator protocol so broad implementation tasks must be split into bounded Operator phases.
2. Require FullOperator implementation phases to use `-TimeoutSeconds 0` unless the Operator/Validator intentionally chooses a timebox.

You are Claude acting as the T66 Operator. Implement those changes directly inside the approved scope.

## Current Role State

- Operator: Claude
- Validator/Finisher: Codex
- Repo: `C:\UE\T66`
- Approval artifact: `C:\UE\T66\Reports\AgentReviews\20260529_ClaudeOperatorPhaseTimeout\codex_operator_approval.md`

## Files To Inspect

- `C:\UE\T66\AGENTS.md`
- `C:\UE\T66\OPERATOR_VALIDATOR_PROTOCOL.md`
- `C:\UE\T66\Scripts\Invoke-ClaudeDirectRead.ps1`
- `C:\UE\T66\Scripts\Invoke-ClaudeReadOnlyOperator.ps1`
- `C:\UE\T66\Reports\AGENTS.md`

## Required Changes

Implement the smallest coherent repo-native update:

- Add a clear phase-bounded Operator task rule for broad implementation work.
- Define what counts as a broad implementation task: multi-system, Unreal/editor asset work, runtime code plus assets, docs plus runtime plus proof, build/capture harness plus implementation, or any task likely to exceed a single clean Operator completion packet.
- Require a phase plan before approval for broad implementation tasks.
- Define the recommended phase shape without being too rigid. Good examples: design/process packet, runtime/code wiring, asset/editor/commandlet work, proof/capture harness, final validation/completion packet.
- Ensure the rule preserves the target that Claude does the authoring work while Codex stays wrapper/approval/validator.
- Update the FullOperator timeout policy so implementation phases default to unbounded wall-clock runs (`-TimeoutSeconds 0`) unless the task is intentionally timeboxed.
- Keep read-only runs and explicit timeboxed probes able to use shorter guards.
- If you change the helper default, make preflight text truthful.
- Keep `AGENTS.md` as a short router. Put details in `OPERATOR_VALIDATOR_PROTOCOL.md`.

## Constraints

- Do not use Claude plan mode.
- Do not edit unrelated sections.
- Do not touch runtime/gameplay/content/assets/build outputs.
- Do not run Unreal, Blender, Niagara, staged builds, git commit/push/tag/reset/clean, or broad Git/LFS scans.
- Do not use native goal functions.
- Preserve unrelated user changes.

## Verification To Run

- Parse-check any edited PowerShell scripts.
- Run `Scripts\Invoke-ClaudeDirectRead.ps1 -Mode Operator -ToolProfile FullOperator -CodexApprovalPath <approval> -Preflight` and confirm the default FullOperator timeout policy matches the new rule.
- Run a narrow `git diff --check` only for files you touched.

## Required Final Operator Artifact

At the end, write a concise completion packet at:

`C:\UE\T66\Reports\AgentReviews\20260529_ClaudeOperatorPhaseTimeout\claude_completion_packet.md`

It must list:

- Files changed.
- Exact changes made.
- Commands run and pass/fail results.
- Any unverified areas.
- Any remaining blockers or follow-ups.
- The helper run directory or manifest path if available to you.

Your output in the Claude helper response should also summarize the same information. Codex will validate before the user sees a final answer.
