# Review Packet: Review Verdict Routing

## Working Goal

Implement the repo process and helper changes so Claude/Codex review verdicts distinguish ordinary Codex revisions from user-only human decision gates, and so a normal `APPROVE` authorizes implementation without asking the user for redundant approval.

The active native goal is still blocked on the earlier item-taxonomy task. The user has explicitly redirected this turn to process/tooling infrastructure.

## User Request

The user approved the structural change discussed in-chat:

- Do not overload `REVISE` for both "Codex should improve the plan" and "human decision required".
- Add a distinct human-decision verdict.
- Update the Claude review prompt/helper and related scripts/process docs.
- Fix the separate problem where Claude accepts a plan and Codex still asks for manual user approval even when there is no user-only vision/product decision left.

## Applicable Instructions

- Root `AGENTS.md`: process/tooling changes require planning and Claude review by default.
- Root `AGENTS.md`: a valid Claude greenlight currently authorizes implementation after reporting caveats unless the user marked the work planning-only, asked Codex to stop, or the reviewed packet contains a user-only decision.
- Root `AGENTS.md`: if a decision gate is needed, ask it once and save `Reports/AgentReviews/<TaskSlug>/decision_block.md`; on continuations, reference the saved gate instead of repeating questions.
- `Reports/AGENTS.md`: review packets belong under `Reports/AgentReviews`.
- No `Scripts/*_AGENTS.md` exists.

## Live Evidence

- `AGENTS.md` line area around Claude Cross-Review already says a valid Claude greenlight authorizes implementation without separate manual approval unless a hold condition exists.
- `Scripts/Invoke-ClaudePlanReview.ps1` currently accepts only:
  - `Verdict: APPROVE`
  - `Verdict: REVISE`
  - `Verdict: BLOCK`
- `Scripts/Invoke-ClaudePlanReview.ps1` prompt currently says `APPROVE` may mean "safe to present at that go-ahead gate, not permission to skip the gate", which conflicts with the desired default auto-implementation behavior after a real greenlight.
- `Scripts/Invoke-CodexPlanReview.ps1` has the same three-verdict parser/prompt shape.
- `Scripts/Test-ClaudeReviewVerdictParser.ps1` verifies the Claude helper's strict first-line verdict parsing but has no `NEEDS_HUMAN_DECISION` case.
- `Scripts/README.md` documents the review helpers but not the new verdict semantics.

## Planned Changes

### Verdict vocabulary

Use these four first-line verdicts:

- `Verdict: APPROVE`
- `Verdict: REVISE`
- `Verdict: NEEDS_HUMAN_DECISION`
- `Verdict: BLOCK`

### Semantics

- `APPROVE`: the reviewed plan/output is safe for Codex to proceed to implementation under the reviewed scope. Codex should report the review conclusion briefly, then implement without asking the user for redundant approval unless the user explicitly marked the work planning-only, asked Codex to stop, the packet itself contains an unresolved user-only decision, or AGENTS/PPF requires explicit approval for a method substitution.
- `REVISE`: Codex can resolve the issue by improving the plan/output, inspecting more repo state, tightening verification, changing implementation approach, or otherwise doing more Codex-owned work. Codex should revise and rerun review. This should not block the goal.
- `NEEDS_HUMAN_DECISION`: the plan/output depends on product direction, vision, risk acceptance, scope choice, or another decision only the user can make. Codex should save `decision_block.md`, ask once, end the turn, and on automatic continuation before the user answers, reference the saved block and use the strongest available pause state.
- `BLOCK`: the plan/output cannot safely proceed because of a hard blocker, missing prerequisite, external-state issue, unavailable credential/context, or contradiction that is not solved by normal Codex revision.

### Files to edit

- `AGENTS.md`
  - Replace `REVISE or BLOCK` wording with verdict-specific routing.
  - Add `NEEDS_HUMAN_DECISION` as the explicit user-decision gate.
  - Clarify that `APPROVE` means proceed to implementation without manual user approval unless a concrete hold condition exists.
  - Update the accepted process registry row.
- `Scripts/Invoke-ClaudePlanReview.ps1`
  - Parser accepts `NEEDS_HUMAN_DECISION`.
  - Prompt lists four verdicts.
  - Prompt defines each verdict and removes the "APPROVE means go-ahead gate" wording.
- `Scripts/Invoke-CodexPlanReview.ps1`
  - Mirror the parser/prompt verdict changes for fallback reviews.
- `Scripts/Test-ClaudeReviewVerdictParser.ps1`
  - Add a valid `NEEDS_HUMAN_DECISION` parser case.
  - Keep malformed output fail-closed.
- `Scripts/README.md`
  - Document the four verdicts and routing at a high level.

## Out Of Scope

- No changes to the item-taxonomy implementation itself.
- No changes to Claude/Codex authentication or fallback availability policy.
- No weakening of strict first-line verdict parsing.
- No automatic fallback from malformed Claude output to Codex.
- No broad review-helper rewrite beyond verdict parsing/prompt semantics.

## Risks

- Parser regex changes could accidentally treat malformed output as valid.
- Helper prompt wording could weaken the greenlight gate if `APPROVE` is too broad.
- AGENTS wording could accidentally make every review approval auto-implement even when the user explicitly requested planning-only or when PPF/method substitution requires explicit approval.

## Verification Plan

1. Run `Scripts\Test-ClaudeReviewVerdictParser.ps1`.
2. Add or run an equivalent direct parse check for `Scripts\Invoke-CodexPlanReview.ps1` with `Verdict: NEEDS_HUMAN_DECISION` if no existing test exists.
3. Use narrow `rg` checks to confirm:
   - both helper parsers accept `APPROVE|REVISE|NEEDS_HUMAN_DECISION|BLOCK`;
   - prompts list all four verdicts;
   - stale "safe to present at that go-ahead gate" wording is removed;
   - `AGENTS.md` contains explicit `APPROVE`, `REVISE`, `NEEDS_HUMAN_DECISION`, and `BLOCK` routing.
4. No Unreal build is required because this pass changes only Markdown and PowerShell review helpers.

