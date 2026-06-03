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
- Packet path: C:\UE\T66\Reports\AgentReviews\20260528_ReviewProcessStressTest\case_sensitive_fix_packet.md
- Output scope: review of the packet below only.

<review_packet>
# Review Packet: Case-Sensitive Verdict Parser Fix

## Working Goal

Fix the review-helper parser defect discovered during stress testing: lowercase verdict tokens such as `Verdict: approve` currently greenlight because PowerShell regex and equality operators are case-insensitive by default.

## Triggering Evidence

The stress matrix under `Reports/AgentReviews/20260528_ReviewProcessStressTest` produced 2 failures out of 26 rows:

- `Invoke-ClaudePlanReview.ps1 -ParseReviewPathOnly` accepted `Verdict: approve` and returned `Greenlit=True`.
- `Invoke-CodexPlanReview.ps1 -ParseReviewPathOnly` accepted `Verdict: approve` and returned `Greenlit=True`.

This contradicts `AGENTS.md` and the helper prompts, which require the first non-empty line to be exactly one of:

- `Verdict: APPROVE`
- `Verdict: REVISE`
- `Verdict: NEEDS_HUMAN_DECISION`
- `Verdict: BLOCK`

## Applicable Instructions

- `AGENTS.md` root process router.
- `Reports/AGENTS.md` for artifact placement.
- `Scripts/pending_issues_Scripts.md` was checked; listed issues are unrelated to review helpers.

## Proposed Fix

1. Update `Scripts/Invoke-ClaudePlanReview.ps1`:
   - Change verdict regex matching from case-insensitive `-match` to case-sensitive `-cmatch`.
   - Change greenlight comparison from case-insensitive `-eq` to case-sensitive `-ceq`.
2. Update `Scripts/Invoke-CodexPlanReview.ps1` the same way.
3. Update `Scripts/Test-ClaudeReviewVerdictParser.ps1`:
   - Add `Verdict: approve` as malformed.
   - Add stale/invalid tokens such as `Verdict: APPROVED` and `Verdict: NEEDS_USER_DECISION` as malformed.
4. Rerun:
   - `Scripts/Test-ClaudeReviewVerdictParser.ps1`
   - `Reports/AgentReviews/20260528_ReviewProcessStressTest/run_parser_matrix.ps1`
   - targeted stale-wording `rg` checks.

## Out Of Scope

- No game content/data changes.
- No staged standalone build.
- No host-level continuation simulation.
- No Mini/minigame scope.

## Risk

Low. This tightens parser acceptance to the already documented exact verdict lines. The main compatibility impact is intentional: reviewers using lowercase or stale verdict tokens will fail closed instead of being accepted.

## Verification Evidence Required

- Parser self-test passes.
- Stress matrix passes or records only explicitly accepted residual risks.
- Direct evidence that lowercase `Verdict: approve` is malformed in both helpers.

</review_packet>
