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
- Packet path: C:\UE\T66\Reports\AgentReviews\20260528_B10_1D_resume5_route_leakage_diagnostic\plan_packet.md
- Output scope: review of the packet below only.

<review_packet>
# B.10.1D Resume5 Route Leakage Diagnostic - Plan Packet

## Review Request

Reviewer: local Claude Code CLI through `Scripts\Invoke-ClaudePlanReview.ps1`.

Requested verdict format: the first non-empty line must be exactly one of:

```text
Verdict: APPROVE
Verdict: REVISE
Verdict: BLOCK
```

Please review this as a read-only packet. Do not edit files or run implementation commands. Identify flawed assumptions, missing files, unsafe scope, inadequate verification, contradictions with repo instructions, unanswered goal questions, and any lazy or careless reasoning.

## Working Goal

Identify why some Ranged-family mobs route to rich `AT66RangedEnemy` during CVar-on (`T66.Mob.UseLightweight=1`, `T66.Mob.Diagnostics.RouteRangedLightweight=1`) B.10.1D acceptance when all ordinary Dungeon Ranged mobs should route lightweight. Determine whether the leak affects only Ranged or also Melee/Rush/Flying. This pass is diagnostic-only: no routing fix, no rich Ranged behavior fix, and no B.10 acceptance reattempt.

## Current State And Evidence

- B.10.1D Resume4 raised the automation-only hero HP cap to `50000.f`, ran HP20000 smoke, and proved the hero survives saturated rich and lightweight projectile pressure.
- B.10.1D Resume4 CVar-off escalation produced a clean same-binary candidate baseline: 10/10 accepted, median `157.68 FPS`, 0 hero deaths, 2519 manager projectiles fired, 2506 hero hits, max PerformanceSystem overhead `805.9 us`, staged SHA `0A0AC836F224B898353CD7FA59B5A58ECC24D7676F6903DFD765CA9A3D9252EB`.
- B.10.1D Resume4 CVar-on escalation halted after two `RouteValidity` rejects. Rejected rows had `UseLightweight=1`, `RouteRanged=1`, and `LightweightSpawns>0`, but also `RichSpawns=1` and nonzero rich fire attempts. The route-validity gate is correct to reject this.
- User observation: within the same Dungeon Ranged mob type, some instances fire and others do not. The likely mapping is: lightweight-routed instances fire through the manager; leaked rich-routed instances still exhibit the known rich path failure mode from earlier diagnostics.
- User confirmation: all Dungeon Ranged should be basic mobs and route lightweight. Only the boss has a ranged component; standard `enemywaveperf` Dungeon captures should confirm no boss/special is present.

## Applicable Instructions Read

- `C:\UE\T66\AGENTS.md`
  - Working goal required; live repo state required; Mini/minigame scope excluded unless explicitly named; Claude review gate required by default; implementation waits for user go-ahead after review.
  - New review/report artifacts belong under `Reports/` and follow `Reports/AGENTS.md`.
  - Broad Git/LFS status scans over asset folders should be avoided.
- `C:\UE\T66\Reports\AGENTS.md`
  - New review packets belong under `Reports/AgentReviews`.
- `C:\UE\T66\PerformanceSystem\PERFORMANCE_SYSTEM_AGENTS.md`
  - Performance/diagnostic contracts stay under `PerformanceSystem/`; runtime code stays in the `T66` game module.
- `C:\UE\T66\Gameplay\GAMEPLAY_AGENTS.md`
  - Runtime gameplay changes need compile/build verification and staged standalone validation when they affect playable standalone.
- `C:\UE\T66\Source\T66\Gameplay\pending_issues_Gameplay.md`
  - Current B.10.1D blocker is CVar-on rich Ranged route leakage after Resume4.
- `C:\UE\T66\PerformanceSystem\pending_issues_PerformanceSystem.md`
  - PerformanceSystem overhead is not the current blocker; binary hash and Git/LFS provenance remain capture hygiene requirements.

## Live Code Facts From Pre-Plan Inspection

- `Source/T66/Gameplay/T66EnemyDirector.cpp`
  - `T66.Mob.UseLightweight`, `T66.Mob.Diagnostics.RouteRushLightweight`, `RouteFlyingLightweight`, and `RouteRangedLightweight` live in this file.
  - `ShouldRouteSpawnToLightweightMob(...)` returns false when lightweight routing is disabled, when `MobID` is `None`, when a slot is mini-boss, or when the spawn is special. Otherwise it routes Melee, Rush, Flying, and Ranged according to family and diagnostic CVars.
  - Tower initial population path around lines 474-575 resolves `MobID`, family, archetype, class, then either acquires `AT66MobBase` or spawns rich `AT66EnemyBase`/family class.
  - Runtime staggered path around lines 1451-1670 queues `FPendingEnemySpawn` records, then either acquires `AT66MobBase` or falls back to rich enemy pool/spawn.
  - Runtime path logs a warning and falls back to the rich path if lightweight acquisition fails after a spawn was eligible for lightweight routing.
  - Runtime mini-boss selection can choose one regular mob slot and marks it `bIsMiniBoss`; `ShouldRouteSpawnToLightweightMob` forces mini-boss slots rich. If a Dungeon Ranged MobID is selected as a mini-boss during `enemywaveperf`, that would look like same-MobID per-instance rich leakage unless attributed explicitly.
- `Source/T66/Gameplay/T66MobManagerSubsystem.*`
  - Existing B.10.1C-Rerun aggregate diagnostics are held in `FT66RangedPressureDiagnostics`.
  - `T66.Ranged.DiagnosticLogging=1` tracks aggregate counters and emits one terminal `[RangedDecisionSummary]`.
  - Existing route validity in the runner uses `RichSpawns`, `LightweightSpawns`, and fire-attempt counters from this summary.
- `Saved/Codex/Performance/LightweightActorB10_1D/run_b101d_projectile_manager_validation.ps1`
  - Existing runner parses `[RangedDecisionSummary]`, validates route state, records staged binary hash, and rejects route leakage.
  - Current route validity correctly treats CVar-on rows with `RichSpawns>0` or rich fire attempts as invalid.

## Codex Opinion Before External Review

The prompt's diagnostic direction is correct, but the live code makes two additional branch points especially important:

1. `RoutedRich_SpecialOrMiniBoss` must distinguish true special/goblin/boss spawns from mini-boss promotion of a regular stage MobID. A regular Ranged MobID chosen as a mini-boss would be a valid rich route under current code but may violate the `enemywaveperf` measurement contract if mini-bosses are supposed to be absent.
2. `RoutedRich_FallbackBranch` must specifically capture "eligible for lightweight, but `UT66MobManagerSubsystem::AcquireMob` returned null, so the director fell back to rich." This would produce exactly intermittent per-instance rich leakage even within the same MobID.

The pass should not infer the root cause from existing `RichSpawns=1` alone. It needs per-spawn route-attribution counters that separate intentional rich routing, mini-boss promotion, special/goblin routing, family lookup failure, lightweight-acquire failure fallback, and non-director/test/tutorial/lab spawn paths.

## PPF / Process Check

PPF is not applicable as a visual/media/artifact replication task. This is a repo-governed performance/gameplay diagnostic pass. The mandatory process is the T66 plan-packet workflow, Claude review gate, Pablo go-ahead gate, staged standalone verification, and capture hygiene.

## User Constraints

- Diagnostic-only pass.
- Do not apply a routing fix.
- Do not reattempt B.10 acceptance.
- Do not change rich Ranged behavior.
- No per-frame or per-mob spam logging.
- Preserve B.10.1D projectile manager/HISM architecture, rich LOS fix, peer-filter, HP override, and PerformanceSystem I/O mitigation.
- Reuse aggregate-counter pattern from B.10.1C-Rerun.
- Determine whether leakage is Ranged-only or affects all four migrated families.
- Standard capture hygiene remains in force: `PerformanceSystemOverheadMaxUs > 10000` rejects; staged binary hash stable across pass; pre-capture clean environment; avoid Git/LFS contamination for FPS-grade capture. This pass is counter-diagnostic, but provenance should still be recorded.

## Intended Edit Scope

### Runtime Diagnostic Counters

Likely files:

- `Source/T66/Gameplay/T66MobManagerSubsystem.h`
- `Source/T66/Gameplay/T66MobManagerSubsystem.cpp`
- `Source/T66/Gameplay/T66EnemyDirector.h`
- `Source/T66/Gameplay/T66EnemyDirector.cpp`
- `Source/T66/Gameplay/T66PlayerController_Overlays.cpp`

Plan:

1. Add a small route-attribution diagnostic struct, preferably adjacent to the existing ranged aggregate diagnostics in `UT66MobManagerSubsystem`, so the terminal emission and runner parsing use the same subsystem pattern as `[RangedDecisionSummary]`.
2. Gate tracking behind `T66.Ranged.DiagnosticLogging=1` for this diagnostic pass to avoid adding another command-line surface. If reviewer believes a new CVar is safer, revise before implementation.
3. Reset route-attribution counters when autocapture resets ranged diagnostics at `enemywaveperf` start.
4. Emit a single terminal `[RouteAttributionSummary]` line alongside `[RangedDecisionSummary]` during automation quit/death terminal summary. No per-spawn log lines.

### Attribution Model

Represent family buckets:

- Melee
- Rush
- Flying
- Ranged
- Special/Unknown

Represent route result/reason buckets:

- `RoutedLightweight_BasicFamily`
- `RoutedRich_CVarOff`
- `RoutedRich_SpecialOrMiniBoss`
- `RoutedRich_ElitePromotion` or `RoutedRich_MiniBossPromotion` if live code shows only mini-boss promotion and no elite system for regular mobs
- `RoutedRich_FamilyLookupFailed`
- `RoutedRich_FallbackBranch`
- `RoutedRich_NonDirectorPath`

Additional fields to include if cheap:

- `MobID`
- `SpawnChannel` (`InitialPopulation`, `RuntimeTrickle`, `AutomationSmoke`, `Tutorial`, `Lab`, `TestRoom`, `TowerGuardian`, etc.) as aggregate reason/channel counters, not per-ID spam.
- `bIsMiniBoss`
- `bIsSpecialSpawn`
- `bWasLightweightEligibleBeforeFallback`
- `AcquireMobFailed` count by family.

### Director Instrumentation

Instrument both director spawn surfaces:

1. Tower initial population path:
   - Before routing, classify why the slot will route lightweight or rich.
   - Record rich fallback if `ShouldRouteSpawnToLightweightMob` returns false.
   - Record lightweight route when `AcquireMob` succeeds.
   - If a lightweight-eligible path can fail and fall back or skip, record that separately.

2. Runtime staggered path:
   - Record the queued slot's family/channel/mini-boss state.
   - Record lightweight success.
   - Record lightweight-eligible acquire failure fallback separately from intentional rich.
   - Record rich spawn success through pool or fresh spawn.

Avoid changing spawn behavior. If attribution requires refactoring `ShouldRouteSpawnToLightweightMob`, add a non-behavioral helper that computes a route reason and have the existing bool call it, or keep the helper local to instrumentation. The result must not change branch outcomes in this diagnostic pass.

### Non-Director Spawn Path Audit

Audit, but do not necessarily instrument every path unless it can run during `enemywaveperf`:

- `AT66EnemyDirector` initial population
- `AT66EnemyDirector` runtime staggered wave
- `UT66EnemyPoolSubsystem::TryAcquire` reuse path
- `AT66GameMode::SpawnStageEnemyDirector`
- `T66GameMode_Tower.cpp` guardian spawn
- `T66GameMode_TestRoom.cpp` test-room spawns
- `T66GameMode_Lab.cpp` lab spawns
- `AT66TutorialManager::SpawnTutorialEnemyAt`
- `T66PlayerController_Overlays.cpp` automation smoke/debug spawns
- chest mimic / special interactable spawns, if they can appear during the target capture

For `enemywaveperf`, the expected active production path should be director initial/runtime waves. The pass must explicitly confirm whether any non-director spawn path is active in the standard Dungeon capture, instead of assuming.

### Runner / Parser

Likely file:

- `Saved/Codex/Performance/LightweightActorB10_1D/run_b101d_projectile_manager_validation.ps1`

Plan:

1. Parse `[RouteAttributionSummary]` key/value payload.
2. Add fields to capture rows for per-family lightweight vs rich route counts and rich reason buckets.
3. For diagnostic mode only, run 5 CVar-on captures with `T66.Mob.UseLightweight=1`, `T66.Mob.Diagnostics.RouteRangedLightweight=1`, `T66.AutoCapture.HeroHPOverride=20000`, `T66.Ranged.DiagnosticLogging=1`.
4. Keep route-validity rejection policy visible. For this diagnostic set, `RichSpawns` should not abort the entire investigation before evidence is collected; it should be recorded as the target phenomenon unless the reviewed runner mode already has a diagnostic/provenance override. If the current runner cannot do that cleanly, add a reviewed diagnostic mode or preserve rows before halting.
5. Record staged binary hash at pass start, per capture, and pass end. Discard hash-drift rows.

### Documentation

Update:

- `PerformanceSystem/2026-05-23_T66_LightweightActor_Plan.md`
  - Append `Pass B.10.1D Resume5 Route Leakage Diagnostic`.
  - Include route summary tables, spawn path audit, mini-boss/elite finding, family scope, and proposed fix scope.
- `Source/T66/Gameplay/pending_issues_Gameplay.md`
  - Update route leakage issue with root cause and family scope.
- `PerformanceSystem/B10_1D_ProjectileManager_HISM_ConsolidatedPacket.md`
  - Add the Resume5 consolidated packet section, per user preference that reviewed T66 passes have one combined packet.
- `PerformanceSystem/pending_issues_PerformanceSystem.md`
  - Only update if capture hygiene, binary provenance, or Git/LFS state changes materially.

## Out Of Scope

- Routing fix.
- Rich Ranged behavior fix.
- B.10 acceptance reattempt.
- Projectile manager/HISM changes.
- Ranged cadence/balance tuning.
- B.11+ work.
- Mini/minigame systems.
- Broad data/asset scans or Content-wide Git/LFS status.

## Risks And Rollback Considerations

- Risk: adding counters to the routing branch accidentally changes spawn order or branch behavior. Mitigation: use side-effect-free classification and one integer increment after branch outcome is known; no RNG draws, no data loads beyond existing values.
- Risk: terminal summary does not emit if the session ends through an unhandled exit path. Mitigation: follow current `EmitRangedPressureSummary` call sites in `T66PlayerController_Overlays.cpp`; if needed, add route summary emission in the same sites only.
- Risk: route leaks are caused by stale rich enemies from a previous capture/session rather than new spawns. Mitigation: include reset counts, route attribution, rich alive counters, and per-capture fresh process validation; do not rely only on end-state rich spawn counts.
- Risk: diagnostic runner aborts at first/second `RouteValidity` and loses the target data. Mitigation: a Resume5-specific diagnostic mode should record leakage rows as evidence while still flagging route invalidity.
- Risk: mini-boss promotion may explain `RichSpawns=1`; this could be current-code-correct but measurement-contract-wrong. Mitigation: categorize separately and propose a follow-up fix only after data confirms it.
- Rollback: diagnostic counter code is additive and CVar-gated. If it causes build/runtime issues, revert the new route attribution additions and runner parsing; no production behavior should need rollback.

## Verification Plan

Before implementation:

- Confirm `ANTHROPIC_API_KEY` is absent from Process/User/Machine scopes before Claude review.
- Get Claude `Verdict: APPROVE` on this packet or revise until no Blocker/Major objections remain.
- Wait for Pablo go-ahead after the greenlit packet.

Implementation verification:

1. Focused Development build:
   - `C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat T66 Win64 Development C:\UE\T66\T66.uproject -WaitMutex`
2. Stage standalone:
   - `C:\UE\T66\Scripts\StageStandaloneBuild.ps1 -ClientConfig Development -SkipBuild`
3. Verify taskbar shortcut still points to:
   - `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`
4. Clean-environment gate before diagnostic capture:
   - No `RunUAT`, `UnrealEditor-Cmd`, staged `T66.exe`, unexpected `git-lfs`, or long-lived broad `git status` workers.
5. Diagnostic captures:
   - 5 CVar-on captures with HP20000 and route attribution active.
   - Parse exactly one `[RouteAttributionSummary]` and one `[RangedDecisionSummary]` per capture.
   - Record binary hash per row and prove stable hash.
   - Record PerformanceSystem overhead and reject rows above `10000 us`.
6. Analysis gates:
   - Identify rich-route reason buckets for leaked Ranged instances.
   - Confirm whether Melee/Rush/Flying also show rich route reasons when they should be lightweight.
   - Confirm whether mini-boss/special/goblin/director fallback/non-director path is responsible.
   - Confirm whether boss/special is present in standard Dungeon `enemywaveperf`.

## Acceptance Criteria

- `RouteAttributionSummary` captures per-spawn route reasons without per-frame/per-spawn log spam.
- All relevant enemy spawn paths are identified and classified.
- Mini-boss/elite promotion interaction is determined.
- Leak reason is identified with counter evidence.
- Family scope is determined: Ranged-only or shared across Melee/Rush/Flying/Ranged.
- Findings and next fix scope are documented.
- No routing or spawn fix is applied in this pass.

</review_packet>
