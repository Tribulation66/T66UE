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

Review depth: deepened.
Deepened validation is the risk-focused review mode. It is not a second implementation pass.
Keep the exact output headings below. Put risk/oversight content inside those headings; do not add headings.

For deepened review, actively look for:
- Hidden coupling, stale-doc/live-code mismatch, and assumptions that could make the plan incomplete.
- Unsafe cleanup, deprecation, deletion, migration, data reload, asset cook, or runtime/source mismatch consequences.
- Scope bleed, especially into explicitly excluded systems.
- Verification gaps, weak pass markers, rollback gaps, and evidence that would fail to prove the user's stated goal.
- The weakest implementation that could appear to pass while missing the real intent.

In Clarifying Questions, ask only user-owned decisions that block safe progress.
In Required Verification, name exact verification gaps and expected pass markers.
In Rationale, summarize the main assumption or oversight risk you challenged.

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
- Packet path: C:\UE\T66\Reports\AgentReviews\ValidatorDepthProcessUpdate\final_change_packet.md
- Output scope: review of the packet below only.

<review_packet>
## Working Task And Tier

Working task:
Operator: Codex
Validator: Claude
Scope: Update `OPERATOR_VALIDATOR_PROTOCOL.md`, `Scripts\Invoke-ClaudePlanReview.ps1`, and `AGENTS.md` as needed so Claude validation can run in a more thoughtful risk/oversight mode for this and future chats. Internalize the new mode for this chat.
Stop condition: Changes are implemented, verified, validated by Claude, and reported.

Validation depth: full/deepened, because this changes repository workflow rules and a reusable validator helper.
Tier classification: process/tooling workflow edit.
Scope boundaries: no gameplay code, assets, data tables, content, saves, Unreal runtime files, or Mini/minigame scope.

## Roles And Tool Profile

- Operator model: Codex.
- Validator model: Claude.
- Claude validator helper: `Scripts\Invoke-ClaudePlanReview.ps1`.
- Current role state: `.t66\operator-state.json` names Codex as Operator and Claude as Validator.

## User Constraints And Out Of Scope

- User asked to update the protocol/helper/AGENTS if necessary and internalize the behavior in this chat.
- The change should make validation more thoughtful around risk and oversight, not simply more strict.
- Out of scope: automatic enforcement for every broad packet, persistence in `.t66\operator-state.json`, gameplay implementation, and any Mini/minigame work.

## Applicable Instructions Read

- `AGENTS.md`
- `.t66\operator-state.json`
- `Reports/AGENTS.md`
- `Scripts/pending_issues_Scripts.md`
- `OPERATOR_VALIDATOR_PROTOCOL.md`
- `Scripts\Invoke-ClaudePlanReview.ps1`
- `Scripts\Test-ClaudeReviewVerdictParser.ps1`

## Evidence And Live Findings

- Plan review pass 3 approved the scoped plan: `Reports/AgentReviews/ValidatorDepthProcessUpdate/ClaudeValidation/20260531T004018-pass3/claude_review_pass3.md`.
- `Scripts\Invoke-ClaudePlanReview.ps1` now accepts `-ReviewDepth targeted|deepened` and `-RiskReview`.
- `-RiskReview` is shorthand for `-ReviewDepth deepened` and errors if combined with explicit `-ReviewDepth targeted`.
- Deepened review is one concept: the risk-focused form of existing `deepened` validation, not a third mode.
- Deepened review preserves the strict first-line verdict and the existing six heading names: Blockers, Major Issues, Minor Issues, Clarifying Questions, Required Verification, Rationale.
- Deepened review changes the review prompt and risk checks only; it does not silently change Claude effort.
- If the caller explicitly passes `-Effort low`, deepened review honors that explicit choice.
- The existing Anthropic API key guard remains before Claude invocation and was not changed.
- `OPERATOR_VALIDATOR_PROTOCOL.md` now documents deepened validation as risk-focused review and lists the risk/oversight checks.
- `OPERATOR_VALIDATOR_PROTOCOL.md` now distinguishes validation process class (`quick` / `full`) from Validator review depth (`targeted` / `deepened`).
- `AGENTS.md` now routes broad/high-rework-risk/process/migration/deletion/runtime-source-sync packets to deepened risk-focused validation, mentions `-ReviewDepth deepened` / `-RiskReview`, and recommends explicit higher effort for broad/high-risk packets without changing the helper default.

## PPF And Process Gates

PPF/process gates: exempt. This is a process/tooling text/script update, not visual/media/VFX/import/UI fidelity production work.

## Proposed Patch Approach

Implemented:

1. `Scripts\Invoke-ClaudePlanReview.ps1`
   - Added `-ReviewDepth targeted|deepened`.
   - Added `-RiskReview` convenience switch.
   - Added deepened-review prompt instructions that explicitly look for risk, oversight, stale evidence, hidden coupling, scope bleed, weak verification, rollback gaps, and weak implementations that could appear to pass.
   - Preserved the exact existing review headings and instructed Claude not to add headings.
   - Preserved existing `Effort` behavior; review depth does not rewrite the caller's effort choice.
   - Added helper output fields: `ReviewDepth` and `RiskReview`.

2. `OPERATOR_VALIDATOR_PROTOCOL.md`
   - Clarified that `deepened` validation is risk-focused review, not a third role and not a second implementation pass.
   - Expanded `ValidatorBudgetHint`.
   - Added the `Deepened Risk-Focused Validation` section.
   - Clarified that risk/oversight and challenged-assumption findings stay inside existing review sections.
   - Clarified that `quick/full` are validation process classes while `targeted/deepened` are Validator review depths.
   - Added an explicit effort recommendation for broad/high-risk deepened reviews.

3. `AGENTS.md`
   - Updated routing so future broad/high-rework-risk/process/migration/deletion/runtime-source-sync packets should use deepened risk-focused validation.
   - Added the helper flags and explicit effort recommendation to the validator review registry row.

Rollback:
- Revert the three edited files if this process change is rejected.

## Verification Plan

Verification performed:

- PowerShell parser check on `Scripts\Invoke-ClaudePlanReview.ps1`: PASS.
- `Scripts\Test-ClaudeReviewVerdictParser.ps1 -OutputRoot Reports\AgentReviews\ValidatorDepthProcessUpdate\ParserFixturesFinal`: PASS, 18 verdict fixtures and 6 auth fixtures.
- Smoke run with `-RiskReview` and no explicit `-Effort`: PASS. Output reported `ReviewDepth=deepened`, `RiskReview=True`, `Effort=low`, and `OutcomeKind=ClaudeValidVerdict`.
- Smoke run with `-RiskReview -Effort low`: PASS. Output reported `ReviewDepth=deepened`, `RiskReview=True`, `Effort=low`, and `OutcomeKind=ClaudeValidVerdict`.
- Negative flag guard test with `-RiskReview -ReviewDepth targeted`: PASS. The helper exited nonzero and emitted the expected `-RiskReview is shorthand for -ReviewDepth deepened` error.
- Accepted combined alias test with `-RiskReview -ReviewDepth deepened -Effort low`: PASS. Output reported `ReviewDepth=deepened`, `RiskReview=True`, `Effort=low`, and `OutcomeKind=ClaudeValidVerdict`.
- Direct primary flag test with `-ReviewDepth deepened`: PASS. Output reported `ReviewDepth=deepened`, `RiskReview=True`, `Effort=low`, and `OutcomeKind=ClaudeValidVerdict`.
- Targeted default test with no review-depth flags: PASS. Output reported `ReviewDepth=targeted`, `RiskReview=False`, `Effort=low`, and `OutcomeKind=ClaudeValidVerdict`.
- Produced deepened prompt inspection: PASS. Prompt contains `Review depth: deepened`, `risk-focused review mode`, `do not add headings`, and the canonical six heading names.
- Verdict-contract prompt inspection: PASS. The produced deepened prompt still contains the strict first-line instruction and all four exact verdict lines.
- Targeted prompt inspection: PASS. The targeted prompt contains `Review depth: targeted` and does not contain `risk-focused review mode`.
- Direct `-ParseReviewPathOnly` against the produced smoke review: PASS, `Verdict=APPROVE`, `OutcomeKind=ClaudeValidVerdict`.
- Canonical heading name check against the produced smoke review: PASS.
- `git diff --check -- AGENTS.md`: PASS, with only the existing CRLF warning.
- Cross-reference check: PASS. `AGENTS.md`, `OPERATOR_VALIDATOR_PROTOCOL.md`, and the helper use the same `-ReviewDepth deepened` and `-RiskReview` spellings.
- Vocabulary coherence check: PASS. `OPERATOR_VALIDATOR_PROTOCOL.md` now defines `quick/full` as process class and `targeted/deepened` as Validator review depth; helper flags use only review depth; `AGENTS.md` routes to deepened risk-focused validation without redefining the taxonomy.
- Caller-compatibility check: PASS. Active `Scripts` call sites use named parameters for the helper (`-PacketPath`, `-ParseReviewPathOnly`, `-ParseAuthStatusJsonOnly`) or are examples; no active script invocation depends on positional arguments that the new optional parameters could intercept.
- Default-path parity check: PASS. Compared the pre-change default prompt from `ClaudeValidation/20260531T004018-pass3/claude_review_prompt_pass3.md` to the post-change targeted prompt from `SmokeFinalTargeted/20260531T005029-pass1/claude_review_prompt_pass1.md`; 19 substantive instructions were retained, including no-edit/no-command/no-implementation rules, strict first-line verdict, all four verdict lines, the six heading names, and contradiction/scope/verification checks.
- Deepened/high-effort flawed-fixture comparison: PASS. On `flawed_cleanup_fixture.md`, targeted/default review returned `REVISE` with no Blockers and general deletion-safety issues; deepened `-ReviewDepth deepened -Effort high` returned `REVISE` with Blockers for undefined deletion target and active-packet self-deletion risk, plus additional recoverability/keep-list/depth-contradiction findings. This proves the deepened/high path surfaced risk findings the targeted/default path did not.
- Depth-isolation flawed-fixture comparison: PASS. On the same fixture at equal low effort, targeted/default returned `REVISE` with deletion-safety findings; deepened `-ReviewDepth deepened -Effort low` returned `REVISE` with additional risk-focused findings including explicit irreversible-deletion/no-rollback framing, no durable keep-list, stale-doc/live-code mismatch, weak pass marker, depth/routing contradiction, and user-owned irreversibility/cutoff decisions. This proves the deepened prompt itself adds risk/oversight coverage even without higher effort.
- Repo-wide taxonomy grep: PASS for active docs/scripts. A grep across `*.md`, `*.ps1`, `*.py`, and `*.json` found active taxonomy references only in `AGENTS.md`, `OPERATOR_VALIDATOR_PROTOCOL.md`, and `Scripts\Invoke-ClaudePlanReview.ps1`; those now agree on `quick/full` as process class and `targeted/deepened` as review depth. Historical `Reports/` and `Audit/` artifacts retain old prompts/reviews by design and were treated as immutable evidence, not active process instructions.

Verification intentionally skipped:
- Unreal build/editor/capture/staged standalone checks, because no gameplay/runtime files changed.

## Token Routing

TOKEN ROUTING
OperatorModel: Codex
OperatorTokensSpent: Unavailable until final token helper run
OperatorRunDir: n/a
OperatorManifest: n/a
CodexApprovalPath: n/a
ExpectedValidatorDepth: deepened
ValidatorBudgetHint: Check whether the change really improves risk/oversight validation while preserving one concept (`deepened`), the strict verdict/parser contract, the six-heading review shape, explicit effort choices, billing guard behavior, AGENTS/protocol/helper consistency, and verification adequacy.

## Operator Position And Open Decisions

- Operator recommendation: approve the change.
- Known caveats: automatic enforcement for all broad packets was not implemented; the new behavior is protocol-directed and helper-supported via `-ReviewDepth deepened` or `-RiskReview`. Deepened validation does not silently raise helper effort; docs now recommend `-ReviewDepth deepened -Effort high` as the normal starting point for broad/high-risk packets, with `xhigh` or `max` for the highest-risk packets and lower effort only when intentional.
- Decisions only the user can make: whether to later make deepened review mandatory/enforced in the helper for broad packets rather than protocol-directed.
- Decisions the Validator should challenge: whether the helper should have changed parser behavior; whether preserving explicit/default effort is the right cost posture; whether the `AGENTS.md` routing text is sufficient.

## Anti-Lookalike Discriminator

Not applicable.

</review_packet>
