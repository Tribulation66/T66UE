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

The first non-empty line of your review must be exactly one of these three lines:
Verdict: APPROVE
Verdict: REVISE
Verdict: BLOCK

After that verdict line, return a concise Markdown review with exactly these headings:
Blockers
Major Issues
Minor Issues
Clarifying Questions
Required Verification
Rationale

Only use Verdict: APPROVE when the reviewed plan/output is safe for Codex to present as greenlit. If implementation still requires user go-ahead under AGENTS.md, APPROVE means safe to present at that go-ahead gate, not permission to skip the gate.

Review scope:
- Packet path: C:\UE\T66\Reports\AgentReviews\20260528_B10_1D_resume5_route_leakage_diagnostic\completion_packet.md
- Output scope: review of the packet below only.

<review_packet>
# B.10.1D Resume5 Completion Packet

## Output Scope

Report completion of the approved route-leakage diagnostic, including implementation, verification, route-attribution findings, and next reviewed scope.

## Implementation Summary

- Added aggregate route attribution counters and one terminal `RouteAttributionSummary` line.
- Instrumented director initial/runtime route decisions and known non-director rich spawn paths.
- Added `RouteDiagnostic` mode to the B.10.1D runner.
- Added opt-in `-AllowHighFpsControlAdvisory` after Claude-approved gate amendment.
- Updated:
  - `PerformanceSystem/2026-05-23_T66_LightweightActor_Plan.md`
  - `PerformanceSystem/B10_1D_ProjectileManager_HISM_ConsolidatedPacket.md`
  - `Source/T66/Gameplay/pending_issues_Gameplay.md`
  - `PerformanceSystem/pending_issues_PerformanceSystem.md`

## Verification

- `Scripts/StageStandaloneBuild.ps1 -ClientConfig Development` passed.
- PowerShell parser check passed for the runner.
- `-RunRouteControlGateSelfTest` passed.
- Final diagnostic command passed:
  - `Saved/Codex/Performance/LightweightActorB10_1D/run_b101d_projectile_manager_validation.ps1 -Mode RouteDiagnostic -HeroHPOverride 20000 -AllowHighFpsControlAdvisory`
- Stable staged SHA: `D1E3235ED789C2596626BF6748F3DE49018B883D99F941B6160D860C535192FF`.
- Final rows/results:
  - `Saved/Codex/Performance/LightweightActorB10_1D/capture_rows.jsonl`
  - `Saved/Codex/Performance/LightweightActorB10_1D/capture_results.json`

## Findings

- Final diagnostic completed one CVar-off control and ten accepted CVar-on route diagnostic rows.
- No rejected captures, no hero deaths, no overhead rejects, no dropped fires, no route-counter mismatches, and stable binary hash.
- Ranged rich-route leak did not reproduce in 10 CVar-on rows:
  - `RangedRoutedLightweightBasic=259`
  - all Ranged rich reason buckets `0`
  - `RichSpawns=0`
  - rich fire attempts `0`
- A planned rich mini-boss route did reproduce once, landing on Flying:
  - `FlyingRoutedRichMiniBossPromotion=1`
- Planned special routes also appeared:
  - `SpecialUnknownRoutedRichSpecialOrMiniBoss=11`
- Source audit shows mini-boss promotion is family-neutral:
  - `ShouldRouteSpawnToLightweightMob` returns false for `bIsMiniBoss`
  - runtime waves choose `MiniBossIndex` before rolling final `MobID`
  - therefore mini-boss promotion can land on Ranged and plausibly explains the earlier Resume4 `RichSpawns=1` route-validity rejects.

## Next Scope

Next reviewed packet should decide the B.10.1D acceptance route contract:

1. Disable special/mini-boss spawns in `enemywaveperf` for pure basic-family lightweight acceptance, or
2. Teach route validity to allow planned rich special/mini-boss routes while still rejecting fallback/lookup/non-director leaks, or
3. Change gameplay routing so migrated-family mini-boss promotions also route lightweight.

No acceptance reattempt was run in this pass.

</review_packet>
