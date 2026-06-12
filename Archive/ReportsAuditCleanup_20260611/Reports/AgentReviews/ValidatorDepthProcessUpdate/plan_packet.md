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
- `Scripts\Invoke-ClaudePlanReview.ps1:378-408` currently has a fixed concise prompt with exactly six headings. The implementation should keep those headings and fold risk/oversight content into them.
- `AGENTS.md:168` still says the Validator performs targeted anchor checks by default and may deepen only under protocol escalation triggers; it does not mention a deliberate risk-review mode.
- `Scripts\Test-ClaudeReviewVerdictParser.ps1` exercises `Invoke-ClaudePlanReview.ps1 -ParseReviewPathOnly`; the parser currently checks the strict first verdict line and does not enforce heading shape.

## PPF And Process Gates

PPF/process gates: exempt. This is a text/tooling workflow change, not visual/media/VFX/import/UI fidelity production work.

## Proposed Patch Approach

1. `OPERATOR_VALIDATOR_PROTOCOL.md`
- Treat risk-focused review as the practical meaning of existing `deepened` validation, not a third validation concept.
   - Clarify that broad multi-system implementation plans, cleanup/deprecation plans, process changes, and high-rework-risk packets should use deepened risk review.
   - Expand `ValidatorBudgetHint` guidance so packets list risks, anchors, assumptions, missing verification, stale-doc/live-code checks, and decisions to challenge.
   - Expand the validator output template with risk/oversight review and assumptions challenged.
   - Rollback note: revert protocol text only.

2. `Scripts\Invoke-ClaudePlanReview.ps1`
- Add `-ReviewDepth targeted|deepened` and a convenience `-RiskReview` switch. `-RiskReview` is only shorthand for `-ReviewDepth deepened`; it is not a separate mode.
- When `ReviewDepth` is `deepened`, augment the Claude prompt with explicit risk/oversight instructions while preserving the strict first-line verdict and exact six-heading output contract. Risk/oversight content must be reported inside the existing headings, not as new headings.
- Preserve existing `-Effort` behavior. Deepened review changes the review prompt and risk checks only; callers can pass `-Effort high`, `-Effort xhigh`, or `-Effort max` explicitly when more model thinking is warranted.
- Preserve the existing Anthropic API key billing guard exactly.
- Include review-depth/risk-review fields in the helper output object.
- Rollback note: revert script parameter and prompt/output changes.

3. `AGENTS.md`
   - Update the routing wording so `AGENTS.md:168` no longer reads as targeted-only by default for all normal reviews. It should state that deepened/risk-focused validation is available through the protocol/helper for broad or high-rework-risk packets while preserving targeted anchor checks for normal cases.
   - Rollback note: revert the small routing text change.

## Verification Plan

- Run PowerShell parser check on `Scripts\Invoke-ClaudePlanReview.ps1`.
- Run `Invoke-ClaudePlanReview.ps1 -ParseReviewPathOnly` against a known review file to ensure existing parser mode still works.
- Run the updated helper on the final change packet with `-RiskReview` to prove the new prompt path works and Claude can return a valid verdict with the strict verdict-line-first contract and the canonical six headings: Blockers, Major Issues, Minor Issues, Clarifying Questions, Required Verification, Rationale.
- Run `Scripts\Test-ClaudeReviewVerdictParser.ps1` and a direct `-ParseReviewPathOnly` against the produced review; expected result is `OutcomeKind=ClaudeValidVerdict` and the correct verdict line.
- Run prompt-generation checks for deepened review through both `-RiskReview` and `-ReviewDepth deepened`; effort should remain whatever the caller supplied or the existing default.
- Inspect/grep the produced deepened prompt to confirm it contains risk/oversight instructions and still says "exactly these headings" with the same six heading names.
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
- Known caveats: changing helper output fields is low risk but downstream scripts that consume object fields should tolerate additive fields; the exact review depth should remain packet-level, not persisted in `.t66\operator-state.json`, unless the user later requests persistence. Internalizing the new mode for this chat means I will use `ReviewDepth deepened` / `-RiskReview` for substantive change packets in this thread; it does not create hidden state beyond the repo docs/helper.
- Decisions only the user can make: whether to enforce deepened review automatically for broad packets in the helper instead of making it an opt-in/default-by-protocol convention.
- Decisions the Validator should challenge: whether the plan preserves one concept (`deepened` equals risk-focused review), whether explicit `-Effort low` is honored, whether the verdict/parser contract is preserved, and whether `AGENTS.md` needs a routing update or protocol-only change is enough.

## Anti-Lookalike Discriminator

Not applicable. This is not a visual/media/VFX/UI fidelity task.
