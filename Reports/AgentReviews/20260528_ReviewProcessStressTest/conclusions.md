# Review Process Stress Test Conclusions

## Scope

This pass stress-tested the revised goal / Claude review infrastructure using read-only scenario agents, live Claude plan reviews, deterministic parser fixtures, and repo text sweeps. It did not change gameplay data, staged builds, release state, or Mini/minigame systems.

## Live Review Behavior Observed

- Initial stress-test plan review:
  - Artifact: `Reports/AgentReviews/20260528_ReviewProcessStressTest/20260528T105926-pass1/claude_review_pass1.md`
  - Verdict: `APPROVE`
  - Result: Codex proceeded directly to the reviewed test implementation without asking for redundant user approval. This validates the intended `APPROVE` control flow in this live run.
- Parser-fix review pass 2:
  - Artifact: `Reports/AgentReviews/20260528_ReviewProcessStressTest/20260528T110203-pass2/claude_review_pass2.md`
  - Verdict: `REVISE`
  - Result: Codex treated it as Codex-owned work, inspected the exact regex/call sites, revised the packet, and reran review without asking the user. This validates the intended `REVISE` loop in this live run.
- Parser-fix review pass 3:
  - Artifact: `Reports/AgentReviews/20260528_ReviewProcessStressTest/20260528T110403-pass3/claude_review_pass3.md`
  - Verdict: `APPROVE`
  - Result: Codex implemented the approved narrow parser fix.

## Agents Deployed

- Scenario agent:
  - Proposed realistic scenario routes for `APPROVE`, `REVISE`, `NEEDS_HUMAN_DECISION`, `BLOCK`, and a PPF exception after an approved finding.
  - Its own answer was Claude-reviewed in a temp workspace with `Verdict: APPROVE`.
- Helper/parser inspection agent:
  - Confirmed the parser and helper structure.
  - Identified gaps: no Codex-helper self-test, missing stale-token checks, no stale wording sweep.
- Goal/continuation agent:
  - Confirmed `NEEDS_HUMAN_DECISION` should be a hard stop: save decision block, ask once, end turn, and on automatic continuation do no work before user input.
  - Important limitation: repo-level tests cannot prove host-level automatic continuation behavior without a host simulation hook.

## Deterministic Test Results

- `Scripts/Test-ClaudeReviewVerdictParser.ps1`
  - Before parser fix: `PASS`, 13 fixtures.
  - After parser fix: `PASS`, 18 fixtures.
  - Added malformed cases for:
    - `Verdict: approve`
    - `verdict: APPROVE`
    - `VERDICT: APPROVE`
    - `Verdict: APPROVED`
    - `Verdict: NEEDS_USER_DECISION`
- `Reports/AgentReviews/20260528_ReviewProcessStressTest/run_parser_matrix.ps1`
  - Before parser fix: `FAIL`, 13 cases / 26 rows / 2 failures.
  - Discovered defect: both Claude and Codex helper parsers accepted `Verdict: approve` as greenlit because PowerShell regex/equality are case-insensitive by default.
  - After parser fix: `PASS`, 17 cases / 34 rows / 0 failures.
  - The 34 rows are 17 Claude-helper rows and 17 Codex-helper rows.
  - Results:
    - `APPROVE` is the only greenlit verdict.
    - `REVISE`, `NEEDS_HUMAN_DECISION`, and `BLOCK` parse as valid but non-greenlit.
    - Missing, prefaced, quoted, heading, indented, lowercase, uppercase-prefix, and stale-token verdicts fail closed.

## Fix Implemented During Stress Test

The stress test found a real parser defect and fixed it:

- `Scripts/Invoke-ClaudePlanReview.ps1`
  - Verdict regex now uses case-sensitive `-cmatch`.
  - Greenlight comparison now uses case-sensitive `-ceq`.
- `Scripts/Invoke-CodexPlanReview.ps1`
  - Same fix.
- `Scripts/Test-ClaudeReviewVerdictParser.ps1`
  - Expanded from 13 to 18 fixtures.
- `Reports/AgentReviews/20260528_ReviewProcessStressTest/run_parser_matrix.ps1`
  - Expanded stress matrix from 13 to 17 cases and made internal string comparisons case-sensitive.

## Text Sweep Results

- No stale old gate wording found in the active process/helper files for:
  - `safe to present at that go-ahead`
  - `APPROVE means safe to present`
  - `not permission to skip the gate`
  - `asking for user go-ahead`
  - `wait for the user's explicit go-ahead`
  - old three-verdict shorthand forms.
- No active stale verdict tokens found in `AGENTS.md`, helper scripts, or `Scripts/README.md` for:
  - `NEEDS_USER_DECISION`
  - `Verdict: APPROVED`
  - lowercase token/prefix examples.

## What This Proves

- The local review helpers now enforce the exact first non-empty verdict line contract case-sensitively.
- Malformed or stale verdicts fail closed and are not greenlights.
- `NEEDS_HUMAN_DECISION` is recognized as a valid verdict but not a greenlight.
- `APPROVE` is the only helper greenlight.
- A live `APPROVE` review no longer leads Codex to ask for a redundant user approval in this run.
- A live `REVISE` review led to Codex-owned revision and rerun, not a user block, in this run.
- `NEEDS_HUMAN_DECISION` and `BLOCK` were proven at parser/fixture level, not as live end-to-end Claude verdict runs.

## What This Does Not Prove

- This repo-level test cannot prove that the Codex host goal system will always pause an active goal after `NEEDS_HUMAN_DECISION`.
- It cannot prove that automatic continuations, compaction resumes, or thread wakeups will always honor a saved `decision_block.md`.
- It cannot prove every future agent will call `update_goal(blocked)` correctly when a human decision gate is reached.

## Dropped Or Deferred Tests

- Did not run destructive or gameplay/content scenarios. The request was process stress, not content implementation.
- Did not force Claude to produce live `NEEDS_HUMAN_DECISION` or `BLOCK` verdicts, because live model verdicts are nondeterministic and fixture/parser tests are the reliable proof for routing acceptance.
- Did not simulate host automatic continuation after a blocked goal; no host-level continuation simulation hook was available from the repo/tooling surface.

## Recommendation

The repo-side process is materially stronger after this pass, and the parser defect found by stress testing is fixed. The remaining gap is host-level, not repo-level: to prove the original loop is impossible, Codex would need a host test harness that can create an active goal, force a `NEEDS_HUMAN_DECISION` route, mark the strongest pause/block state, trigger automatic continuation, and assert that no work proceeds before user input.
