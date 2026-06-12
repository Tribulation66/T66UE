# Review Packet: Review Process Stress Test

## Working Goal

Stress-test the revised goal and Claude-review routing process with representative random scenarios, including delegated read-only agent checks, parser/helper tests, and a final conclusions report.

## User Request

The user asked to "deploy some agents and do some tests to see if this new process works" using random realistic goals and scenarios that would stress-test the goal / Claude review infrastructure, then report conclusions.

## Clarified Assumptions

- This is a validation/reporting pass, not a request to change gameplay systems, item data, build outputs, or release state.
- Mini/minigame scope is excluded.
- It is acceptable to create review/test artifacts under `Reports/AgentReviews/20260528_ReviewProcessStressTest`.
- It is acceptable to spawn read-only sub-agents for independent scenario design and process analysis.
- It is acceptable to use local helper parse modes and mocked review artifacts to test deterministic routing without forcing Claude to produce every possible verdict in live review.

## Applicable Instructions

- `AGENTS.md` root process router.
- `Reports/AGENTS.md` for review artifact routing.
- `Scripts/pending_issues_Scripts.md` was checked; listed issues are unrelated to review helpers.
- `Scripts/README.md` documents the revised four-verdict helper behavior.

## PPF Check

Skipped. This is process/tooling validation, not visual, media, animation, VFX, import, UI fidelity, generated-content, or comparable production work.

## Proposed Plan

1. Inspect live `AGENTS.md`, `Reports/AGENTS.md`, `Scripts/pending_issues_Scripts.md`, and review helper interfaces.
2. Spawn read-only agents for:
   - random realistic scenario design across `APPROVE`, `REVISE`, `NEEDS_HUMAN_DECISION`, and `BLOCK`;
   - helper/parser stress command suggestions;
   - goal/continuation failure-mode analysis.
3. Run deterministic parser/helper tests:
   - `Scripts/Test-ClaudeReviewVerdictParser.ps1`;
   - direct `Invoke-ClaudePlanReview.ps1 -ParseReviewPathOnly` fixtures for all four verdicts plus malformed cases;
   - direct `Invoke-CodexPlanReview.ps1 -ParseReviewPathOnly` fixtures for the same routing.
4. Run targeted repo text checks for stale old-gate wording and for the new verdict semantics in `AGENTS.md`, helper scripts, and `Scripts/README.md`.
5. Optionally run one or more live Claude reviews against scenario packets if the local Claude CLI is available and `ANTHROPIC_API_KEY` is not set.
6. Write a concise final report summarizing:
   - what passed;
   - what the tests prove;
   - what they do not prove, especially host-level automatic continuation behavior;
   - any recommended follow-up.

## Out Of Scope

- No gameplay/data implementation.
- No staged standalone build.
- No broad Git/LFS status over binary asset folders.
- No changes to Mini/minigame systems.
- No attempt to simulate or mutate host-level native goal state beyond observing current available goal-tool behavior.

## Risks

- Parser tests can prove helper behavior, but cannot fully prove the host will pause every future automatic continuation unless the host itself exposes a way to simulate a queued continuation against a `NEEDS_HUMAN_DECISION` state.
- Live Claude reviews may naturally choose a different verdict than the expected scenario route; deterministic fixture tests are therefore the primary proof for parser/routing acceptance.
- Sub-agents are read-only and may suggest more tests than are worth running in this pass.

## Verification Evidence Required

- Paths of spawned-agent findings or summarized final messages.
- Parser test output showing pass/fail and fixture count.
- Direct parser outputs for all four verdicts and malformed cases for both Claude and Codex helpers.
- Targeted `rg` output or absence of stale gate wording.
- Any live Claude review artifact path, if run.
- Clear distinction between proven script/process behavior and host-level behavior that remains only contract-level.
