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
- Packet path: C:\UE\T66\Reports\AgentReviews\ValidatorDepthProcessUpdate\plan_packet.md
- Output scope: review of the packet below only.

<review_packet>
## Working Task And Tier

Working task:
Operator: Codex
Validator: Claude
Scope: Update `OPERATOR_VALIDATOR_PROTOCOL.md`, `Scripts\Invoke-ClaudePlanReview.ps1`, and `AGENTS.md` only if necessary so Claude validation can run in a more thoughtful risk/oversight mode for this and future chats. Internalize the new mode for this chat after implementation.
Stop condition: Process/helper changes are implemented, verified, validated by Claude, and reported.

Validation depth: full, because this changes repository workflow rules and a reusable Claude validator helper.
Tier classification: process/tooling workflow edit.
Scope boundaries: no gameplay code, no assets, no data tables, no content, no saves, no Mini/minigame scope.

## Roles And Tool Profile

- Operator model: Codex.
- Validator model: Claude via `Scripts\Invoke-ClaudePlanReview.ps1`.
- Claude tool profile: read-only plan review helper; no file edits or commands by Claude.
- Current role state: `.t66\operator-state.json` names Codex as Operator and Claude as Validator.

## User Constraints And Out Of Scope

- The user wants the Claude validator role to give more thought to potential risks and oversights, not simply become stricter.
- The user explicitly asked to update the process docs/helper and internalize the change in this chat.
- Out of scope: implementation of gameplay changes, full rewrite of the Operator/Validator stack, changes to operator-state persistence unless needed, broad Git/LFS status scans, Mini/minigame systems.

## Applicable Instructions Read

- `AGENTS.md`: task contract, no native goal tools, folder discovery, implementation planning, always-on validation, Claude billing guard, token reporting.
- `.t66\operator-state.json`: Codex Operator, Claude Validator.
- `Reports/AGENTS.md`: review artifacts belong under `Reports/AgentReviews`.
- `Scripts/pending_issues_Scripts.md`: no conflicting issue for this helper change.
- `OPERATOR_VALIDATOR_PROTOCOL.md`: existing `ExpectedValidatorDepth`, `ValidatorBudgetHint`, escalation triggers, validator output contract.
- `Scripts\Invoke-ClaudePlanReview.ps1`: current params and Claude prompt.

## Evidence And Live Findings

- `OPERATOR_VALIDATOR_PROTOCOL.md:396-397` already defines `ExpectedValidatorDepth: targeted | deepened` and `ValidatorBudgetHint`.
- `OPERATOR_VALIDATOR_PROTOCOL.md:485-514` already allows/requires deepened validation under specific triggers, including comprehensive audits/process changes.
- `OPERATOR_VALIDATOR_PROTOCOL.md:558-566` already requires completeness, anchor checks, instruction/scope checks, findings, missing verification, validation depth, and token spend.
- `Scripts\Invoke-ClaudePlanReview.ps1:32-44` exposes `-MaxTurns` range 1-10 with default 10 and `-Effort` values `low`, `medium`, `high`, `xhigh`, and `max`, defaulting to `low`.
- `Scripts\Invoke-ClaudePlanReview.ps1:378-408` currently has a fixed concise prompt with headings that does not include explicit risk/oversight, assumptions-challenged, or deepened-review sections.
- `AGENTS.md:168` still says the Validator performs targeted anchor checks by default and may deepen only under protocol escalation triggers; it does not mention a deliberate risk-review mode.

## PPF And Process Gates

PPF/process gates: exempt. This is a text/tooling workflow change, not visual/media/VFX/import/UI fidelity production work.

## Proposed Patch Approach

1. `OPERATOR_VALIDATOR_PROTOCOL.md`
   - Add a named risk-focused validation depth/mode that still fits the existing targeted/deepened model.
   - Clarify that broad multi-system implementation plans, cleanup/deprecation plans, process changes, and high-rework-risk packets should use deepened risk review.
   - Expand `ValidatorBudgetHint` guidance so packets list risks, anchors, assumptions, missing verification, stale-doc/live-code checks, and decisions to challenge.
   - Expand the validator output template with risk/oversight review and assumptions challenged.
   - Rollback note: revert protocol text only.

2. `Scripts\Invoke-ClaudePlanReview.ps1`
   - Add a parameter such as `-ReviewDepth targeted|deepened` and a convenience `-RiskReview` switch.
   - When risk review/deepened mode is selected, augment the Claude prompt with explicit risk/oversight instructions and extra required headings.
   - Increase effective effort to at least `high` for risk review if the caller left the default `low`, while preserving explicit caller choices.
   - Include review-depth/risk-review fields in the helper output object.
   - Rollback note: revert script parameter and prompt/output changes.

3. `AGENTS.md`
   - Update only if needed to point to the new helper mode and make future chats inherit the expectation that high-risk workflow changes can request deepened risk review.
   - Rollback note: revert the small routing text change.

## Verification Plan

- Run PowerShell parser check on `Scripts\Invoke-ClaudePlanReview.ps1`.
- Run `Invoke-ClaudePlanReview.ps1 -ParseReviewPathOnly` against a known review file to ensure existing parser mode still works.
- Run the updated helper on the final change packet with `-RiskReview` to prove the new prompt path works and Claude can return a valid verdict.
- Inspect the generated prompt to confirm risk-review instructions and headings are present.
- No Unreal build, editor run, capture, or staged standalone verification is needed because no gameplay/runtime files are changed.

## Token Routing

TOKEN ROUTING
OperatorModel: Codex
OperatorTokensSpent: Unavailable until final token helper run
OperatorRunDir: n/a
OperatorManifest: n/a
CodexApprovalPath: n/a
ExpectedValidatorDepth: deepened
ValidatorBudgetHint: Check whether the proposed changes actually make validation more thoughtful without turning Claude into a second implementer; check parameter design, default behavior, backwards compatibility, prompt/output contract, AGENTS/protocol consistency, and whether verification proves the helper works.

## Operator Position And Open Decisions

- Operator recommendation: approve this scoped process/helper update.
- Known caveats: changing helper output fields is low risk but downstream scripts that consume object fields should tolerate additive fields; the exact review depth should remain packet-level, not persisted in `.t66\operator-state.json`, unless the user later requests persistence.
- Decisions only the user can make: whether to enforce deepened review automatically for broad packets in the helper instead of making it an opt-in/default-by-protocol convention.
- Decisions the Validator should challenge: whether `-RiskReview` should auto-raise effort to `high`; whether `AGENTS.md` needs a routing update or protocol-only change is enough.

## Anti-Lookalike Discriminator

Not applicable. This is not a visual/media/VFX/UI fidelity task.

</review_packet>
