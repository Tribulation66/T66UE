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
- Packet path: C:\UE\T66\Reports\AgentReviews\20260529_OperatorValidatorTier0UsageTray\implementation_review_packet.md
- Output scope: review of the packet below only.

<review_packet>
# Operator/Validator Tier 0 And Usage Tray Update Plan

## Working Goal

Update the T66 AI-process wording and usage tray plan so the stack uses Operator/Validator language, supports only Tier 0 versus Tier 1 routing, verifies current operator behavior, and adds Operator display requirements to the usage tray widget.

## User Constraints

- Consolidate duplicate wording across `AGENTS.md` and `Scripts/README.md`.
- Use only two tiers:
  - Tier 0: simple questions and things that do not need a serious process.
  - Tier 1: the current full process.
- Shift conceptual language from "Claude/Codex" first to "Operator/Validator" first.
- Check whether Claude is already set as operator and whether it is actually doing the heavy lifting.
- Add an `Operator:` display to the usage tray widget plan.
- No Mini/minigame scope.

## Current-State Findings

- `AGENTS.md` has an existing `Claude/Codex Operator Stack` section and accepts `Make Claude operator` / `Make Codex operator`.
- The review section still speaks mostly as if Claude is always the validator and Codex is always the implementer.
- `Scripts/README.md` repeats review/fallback details already present in `AGENTS.md`.
- `Scripts/Invoke-ClaudeDirectRead.ps1` can run Claude as a read-only operator and marks the output as `OperatorArtifactNotGreenlight`.
- There is no durable persisted "current operator" state file yet. The current operator is conversation/task routing, not a machine-readable state that the usage tray can display.
- For the current task, Claude is not globally and durably set as operator. Codex is currently acting as integrator/implementer in this workspace under the active goal. Claude can be made the heavy operator when a task explicitly uses `Make Claude operator` or when the agent invokes the direct-read operator helper under the documented rule.
- The usage tray plan currently shows Claude/Codex weekly and 5-hour usage, but it does not include the current operator.

## Applicable Instructions

- Root `AGENTS.md` process rules apply until the edit lands.
- `Reports/AGENTS.md` routes this review packet under `Reports/AgentReviews`.
- `Tools/README.md` applies to the usage tray if the user later chooses repo-local source.
- Existing review-gate lessons from memory require preserving strict malformed-verdict fail-closed behavior.

## Proposed Edit Scope

### `AGENTS.md`

1. Add a short `Tier 0 / Tier 1 Request Routing` subsection near the planning/review loop.
2. Define Tier 0 as low-risk direct answers/commands that do not require external validator review or report artifacts, while still requiring live state checks when needed.
3. Define Tier 1 as the current full process: goal, folder instructions, plan, applicable PPF, Operator/Validator review, implementation, verification, and report artifacts.
4. Rename `Claude Cross-Review` conceptually to `Validator Review`.
5. Keep the local Claude Code helper as the normal validator helper when Codex is Operator.
6. State that when Claude is Operator, Codex becomes Validator/Integrator in the active Codex workspace unless the user requests a separate Codex CLI validator.
7. Preserve strict first-line verdict semantics and API-key billing guard.
8. Rename `Claude/Codex Operator Stack` to `Operator/Validator Stack`.
9. Reword switch commands so they set roles:
   - `Make Claude operator`: Operator = Claude, Validator = Codex.
   - `Make Codex operator`: Operator = Codex, Validator = Claude.
10. Add that no durable operator-state file exists yet; once the usage tray/state helper is implemented, operator switches should update the machine-readable state used by the tray.

### `Scripts/README.md`

1. Remove duplicated long verdict/fallback wording and point to `AGENTS.md` as canonical.
2. Rename the helper section from `Claude/Codex Stack Helpers` to `Operator/Validator Stack Helpers`.
3. Keep practical command examples.
4. Add that the usage tray/operator state source is planned, not currently implemented.

### `Reports/AgentReviews/20260528_UsageTrayWidgetPlan/implementation_plan_packet.md`

1. Update the widget goal from usage-only to usage plus active operator display.
2. Add visible UI requirement:
   - `Operator: Claude` or `Operator: Codex`
   - Then provider rows with usage remaining.
3. Add `OperatorState` model:
   - `Operator: Claude | Codex | Unknown`
   - `Validator: Claude | Codex | Unknown`
   - `Scope: Global | Thread | Manual`
   - `UpdatedAtLocal`
   - `Source`
4. Add `OperatorStateService` to read a local state file under `%LOCALAPPDATA%\T66UsageTray\operator-state.json`.
5. Add future write path:
   - Tray UI can manually select operator.
   - Future helper or agent workflow can update this state after `Make Claude operator` / `Make Codex operator`.
6. Failure behavior:
   - If no state file exists, display `Operator: Unknown` or `Operator: Codex (current chat)` only when a live integration explicitly provides it.
7. Add verification:
   - Fixture tests for operator-state parse.
   - Manual switch updates visible widget state.
   - State-file write never stores provider tokens.

## Out Of Scope

- Building the usage tray widget now.
- Creating the actual operator-state helper/script now.
- Refactoring the PowerShell review helpers into shared modules.
- Broad script prompt refactors.
- Mini/minigame systems.
- Changing Claude or Codex usage denominator logic.

## Risks And Mitigations

- Risk: Tier 0 could be abused to skip needed validation.
  - Mitigation: Tier 0 is limited to simple questions, direct commands, and low-risk read-only answers. Any code/content/config/tooling/build/asset/process edit is Tier 1.
- Risk: Role wording could obscure the concrete CLI helper being used.
  - Mitigation: docs should state roles first, concrete helper second.
- Risk: Widget displays stale operator state.
  - Mitigation: state includes timestamp/source and can show `Unknown` or stale state when not updated.
- Risk: Weakening strict verdict rules.
  - Mitigation: preserve strict first-line verdict behavior and API-key guard unchanged.

## Verification Plan

- `git diff --check -- AGENTS.md Scripts/README.md Reports/AgentReviews/20260528_UsageTrayWidgetPlan/implementation_plan_packet.md`
- Targeted `rg` checks for:
  - `Tier 0`
  - `Tier 1`
  - `Operator/Validator`
  - `Operator:`
  - `operator-state.json`
- Manual readback of the edited sections.
- No build required because this is documentation/planning-only.

## Review Request

Review whether this plan safely implements Pablo's requested process wording changes and usage tray plan update without over-scoping into helper refactors or widget implementation.

</review_packet>
