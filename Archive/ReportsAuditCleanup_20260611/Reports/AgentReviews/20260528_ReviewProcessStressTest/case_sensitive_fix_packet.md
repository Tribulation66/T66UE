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

## Current Parser Evidence

Current regex lines are already anchored and use a literal mixed-case `Verdict:` prefix:

- `Scripts/Invoke-ClaudePlanReview.ps1:195`: `if ($TrimmedEnd -match '^Verdict:\s*(APPROVE|REVISE|NEEDS_HUMAN_DECISION|BLOCK)\s*$')`
- `Scripts/Invoke-CodexPlanReview.ps1:99`: `if ($TrimmedEnd -match '^Verdict:\s*(APPROVE|REVISE|NEEDS_HUMAN_DECISION|BLOCK)\s*$')`

Current greenlight checks are case-insensitive:

- `Scripts/Invoke-ClaudePlanReview.ps1:198`: `$Greenlit = $Verdict -eq "APPROVE"`
- `Scripts/Invoke-CodexPlanReview.ps1:102`: `$Greenlit = $Verdict -eq "APPROVE"`

Because PowerShell `-match` and `-eq` are case-insensitive by default, the anchored pattern still accepts lowercase token or prefix variants such as `Verdict: approve`, `verdict: APPROVE`, and `VERDICT: APPROVE`. The fix must therefore use case-sensitive operators for both regex acceptance and greenlight comparison.

Repo inspection found no separate shared verdict parser module. The only parser call sites found by targeted `rg` are:

- `Scripts/Invoke-ClaudePlanReview.ps1`
- `Scripts/Invoke-CodexPlanReview.ps1`
- `Scripts/Test-ClaudeReviewVerdictParser.ps1`
- `Reports/AgentReviews/20260528_ReviewProcessStressTest/run_parser_matrix.ps1`

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
   - Keep explicit positive rows for all four canonical lines: `APPROVE`, `REVISE`, `NEEDS_HUMAN_DECISION`, and `BLOCK`.
   - Add `Verdict: approve`, `verdict: APPROVE`, and `VERDICT: APPROVE` as malformed.
   - Add stale/invalid tokens `Verdict: APPROVED` and `Verdict: NEEDS_USER_DECISION` as malformed.
4. Update `Reports/AgentReviews/20260528_ReviewProcessStressTest/run_parser_matrix.ps1` with the same prefix-case and stale-token negative rows so both Claude and Codex helpers are covered by the stress matrix.
5. Rerun:
   - `Scripts/Test-ClaudeReviewVerdictParser.ps1`
   - `Reports/AgentReviews/20260528_ReviewProcessStressTest/run_parser_matrix.ps1`
   - targeted stale-wording `rg` checks.

## Out Of Scope

- No game content/data changes.
- No staged standalone build.
- No host-level continuation simulation.
- No Mini/minigame scope.
- No changes to `AGENTS.md` or helper prompt wording are expected; this fix tightens implementation to match the existing documented exact-line contract.

## Risk

Low. This tightens parser acceptance to the already documented exact verdict lines. The main compatibility impact is intentional: reviewers using lowercase or stale verdict tokens will fail closed instead of being accepted.

## Verification Evidence Required

- Parser self-test passes.
- Stress matrix passes or records only explicitly accepted residual risks.
- Direct evidence that lowercase token and prefix variants are malformed in both helpers.
- `rg "NEEDS_USER_DECISION"` confirms no active docs/templates outside this test artifact use the stale token.
- `rg` over `Scripts/` confirms no remaining verdict parser uses case-insensitive `-match` / `-eq` for review verdict acceptance or greenlighting.
