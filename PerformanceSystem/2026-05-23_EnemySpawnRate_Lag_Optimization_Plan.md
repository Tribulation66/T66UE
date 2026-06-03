# Enemy Spawn Rate Lag Optimization Plan

Date: 2026-05-23

Project: T66, Unreal Engine 5.7

Related diagnosis: `PerformanceSystem/2026-05-23_EnemySpawnRate_Lag_Optimization_Report.md`

## Decision

T66 should use Unreal MassEntity for the long-term high-density basic mob solution.

The target design is bullet-heaven density: hundreds of live basic enemies without turning every regular mob into a full `ACharacter`. The selected architecture is a hybrid representation:

- basic distant mobs: Mass entities with data-oriented movement and combat fragments
- medium-range basic mobs: Mass entities with MassRepresentation-driven instanced visuals
- nearby or priority basic mobs: promoted or bridged into richer interaction when needed
- elites, bosses, and hero-targeted special cases: current `ACharacter` combat path remains authoritative

This keeps the existing combat work for the cases where rich actor behavior matters, while moving the volume problem onto an Epic-maintained system built for large crowds.

## Rationale

MassEntity maps directly onto the current bottleneck:

- MassRepresentation LOD already solves representation tiering.
- Mass processors batch work over fragments instead of per-actor virtual dispatch.
- Mass movement/spawner/signals avoid a custom swarm framework that would need to be maintained locally.
- Unreal 5.7 keeps Mass current with engine improvements.
- The existing `ACharacter` enemies are still useful for elites, bosses, promoted targets, and debugging.

The immediate lag does not require Mass scaffolding in Phase A. Phase A should first separate obvious debug/HUD/log overhead from true actor-model cost, then Phase B should measure the actor ceiling before Phase C begins.

## Rejected Alternatives

### Custom `UT66EnemySwarmSubsystem`

Rejected for now.

Reasons:

- would recreate Mass LOD, movement, spawning, signals, and representation
- risks accumulating solo-maintained data-oriented infrastructure
- would need its own editor/debug tooling
- likely delays gameplay validation before proving it is better than Mass

### Hybrid Actor Manager Only

Rejected as the long-term solution.

Reasons:

- can reduce tick cost but still pays actor/component/collision overhead
- still keeps basic mobs inside the `ACharacter` cost envelope
- useful as Phase B triage, not as the final hundreds-of-enemies architecture

## Phase A: Quick Wins

Goal: reduce obvious Development-build overhead and add the minimum context needed to correlate hitches with board saturation.

Implementation items:

- default `T66.Combat.DebugView` to `0` in non-shipping builds
- default `T66.Combat.DebugLabels` to `0` in non-shipping builds
- preserve all console/key paths that re-enable combat visualization
- log runtime debug CVar changes so re-enabling visibility appears in session logs
- make HUD map enemy markers dirty via `UT66ActorRegistrySubsystem` enemy register/unregister signals
- skip map refresh entirely when both full map and minimap are not visible
- throttle visible map refresh to a maximum of 10 Hz
- keep the minimap 48-enemy marker cap
- add a timed scope around actual map rebuild work
- demote routine ranged fire/blocked-shot logs to `VeryVerbose`
- demote routine enemy projectile fire/rejected-impact logs to `VeryVerbose`
- keep actual hero-hit projectile logs at their current level
- sample once per second in `UT66LagTrackerSubsystem`:
  - live regular enemies
  - pending spawns
  - active enemy projectiles
- attach those counters to `SingleFrameHitch` PerformanceSystem events

Acceptance criteria:

- average FPS is at least 40 on the same Dungeon floor scenario, with 50+ as the target
- no `GameplayHUD::RefreshMapData` or map rebuild stall above 16 ms
- combat debug defaults are verified off in non-shipping builds
- console re-enable still restores combat hitbox/projectile visualization
- PerformanceSystem hitch events contain live enemy, pending spawn, and active enemy projectile metadata

## Phase B: Instrumentation And Actor-Model Triage

Goal: measure the actor-model ceiling before Mass migration scope is finalized.

Deferred implementation items:

- per-family enemy counters
- grouped enemy tick timing in PerformanceSystem
- grouped projectile tick/collision timing
- hero combat candidate counts and attack-resolution timing
- debug filter modes:
  - nearest N enemies
  - projectiles only
  - traps only
  - selected/locked target only
  - every N frames
- projectile pooling audit
- hit zone collision audit
- widget component visibility/tick audit
- optional significance-tier prototype for current actors

Acceptance criteria:

- hitches identify whether debug, HUD, projectiles, hero targeting, or enemy tick dominates
- a measured live-enemy ceiling exists for the current actor architecture
- Phase C migration can be sized against measured actor-model cost rather than guesses

## Phase C: Mass Migration

Goal: support 300+ live basic enemies at 60 FPS.

Architecture:

- basic mobs become Mass entities
- Mass fragments store lightweight combat state, movement state, target state, and visual identity
- Mass processors batch movement, steering, target acquisition, damage commands, and lifecycle updates
- MassRepresentation LOD controls visual tier:
  - low detail: lightweight entity only or coarse representation
  - medium detail: instanced visual representation
  - high detail or priority: bridge/promote to actor-compatible interaction
- existing `ACharacter` enemies remain for:
  - elites
  - bosses
  - special scripted enemies
  - hero-targeted promoted enemies
  - diagnosis and debug samples

Required bridges:

- spawn budget bridge from `AT66EnemyDirector` / stage progression into Mass spawning
- combat bridge from Mass damage commands into `UT66CombatComponent` expectations
- hitbox/debug bridge so invisible damage stays diagnosable
- projectile bridge so existing hero, enemy, trap, and idol projectile visuals stay readable
- loot/death bridge so Mass deaths still produce existing rewards and run-state effects
- PerformanceSystem bridge so Mass entity counts and processor costs appear in reports

Acceptance criteria:

- 300+ live basic enemies on the Dungeon floor scenario
- average FPS at or above 60 on target Development test conditions
- 1 percent low remains playable and materially better than the full-actor baseline
- elites and bosses still use the existing rich actor combat path
- debug visibility remains available without becoming the default cost

## Measurement Methodology

Use the same Dungeon floor scenario described in the diagnosis report.

Compare sessions using:

- average FPS
- average frame time
- 1 percent low FPS
- 0.1 percent low FPS
- `GameplayHUD::RefreshMapData` / map rebuild stall distribution
- `SingleFrameHitch` events
- live regular enemy count at hitches
- pending spawn count at hitches
- active enemy projectile count at hitches

The key comparison is not only "does it feel smoother." The required proof is whether hitches still correlate with board saturation once combat debug, map rebuilds, and routine logging are reduced.

## Open Questions After Phase A

- With debug off, is the primary remaining cost enemy tick, projectile actors, HUD/map work, or hero combat resolution?
- Does the Development standalone reach the 90 live-enemy cap in the captured scenario?
- Are active enemy projectiles a meaningful part of hitches after ranged logs are demoted?
- Does the HUD map still stall when the full map is closed and only the minimap is visible?
- What live enemy cap can the current actor model support before Phase C becomes mandatory?
- Which regular enemy families should be migrated to Mass first?
- What exact promotion rule should move a Mass mob into the rich actor path?

## Phase A Results

Captured from the staged Development standalone with `-T66Entry=Run:Tower` and the development-only `T66GameplayAutoCapture=enemywaveperf` hook. The hook enters the first Dungeon gameplay floor, triggers the tower descent path, and starts the enemy director so this is no longer a start-gallery-only HUD run.

- PerformanceSystem session: `Saved/StagedBuilds/Windows/T66/Saved/PerformanceSystem/Sessions/20260524T005400Z_aCphoUGGtCXdFZ63E0v-Nw`
- Standalone log: `Saved/StandaloneLogs/T66_PhaseA_EnemyWavePerfRun_Final.log`
- Unreal viewport screenshot: `Saved/Codex/Performance/PhaseA/enemywaveperf_final.png`
- Average FPS: 156.59
- Average frame: 6.39 ms
- 1 percent low FPS: 114.55
- 0.1 percent low FPS: 87.31
- `GameplayHUD::RefreshMapData` / map rebuild stalls: 0 events in the final session. The closed-minimap path no longer reports a map rebuild stall.
- Largest remaining `SingleFrameHitch`: none recorded.
- Remaining project stall: `EnemyDirector::SpawnRuntimeTrickleWave` took 53.25 ms at game time 3.016 seconds.
- Board saturation at that stall: live regular enemies 3, pending spawns 0, active enemy projectiles 0, sample age 0.751 seconds.
- Board saturation at later frame stutter events:
  - game time 5.415 seconds: live regular enemies 17, pending spawns 17, active enemy projectiles 0
  - game time 10.431 seconds: live regular enemies 46, pending spawns 18, active enemy projectiles 0
- Acceptance result: FPS target passed; `RefreshMapData` stall target passed; board-saturation metadata is visible on frame pacing and project operation events.
- Combat debug defaults verified: Development standalone starts with `T66.Combat.DebugView=0` and `T66.Combat.DebugLabels=0` by code default.
- Console debug re-enable verified: staged smoke logs `T66_PhaseA_CombatDebugToggle.log` and `T66_PhaseA_CombatDebugLabelsToggle.log` show runtime CVar changes to `DebugView=3` and `DebugLabels=1`.

Follow-up for Phase B: the remaining measured stall is no longer HUD map rebuild work. It is enemy director wave materialization, so Phase B should prioritize spawn batching details, pooled projectile/enemy actor acquisition cost, and per-family spawn/tick timing before changing collision or combat representation.

## Phase A Saturation Validation

Captured from the staged Development standalone with the same `-T66Entry=Run:Tower` and `T66GameplayAutoCapture=enemywaveperf` path, extended to a 168.62 second gameplay window. This pass did not change spawn tuning: 30-enemy waves, 90 live cap, 5 second wave interval, and 5 second stagger remained intact.

- PerformanceSystem session: `Saved/StagedBuilds/Windows/T66/Saved/PerformanceSystem/Sessions/20260524T011558Z_ghvr1U4kxVtVwXeF7Ah3fw`
- Standalone log: `Saved/StandaloneLogs/T66_PhaseA_SaturationValidation.log`
- Unreal viewport screenshot: `Saved/Codex/Performance/PhaseA/enemywaveperf_saturation.png`
- Board saturation samples: `Saved/StagedBuilds/Windows/T66/Saved/PerformanceSystem/Sessions/20260524T011558Z_ghvr1U4kxVtVwXeF7Ah3fw/board_saturation_samples.jsonl`

Full gameplay window:

- Samples: 26,291
- Duration: 168.62 gameplay seconds
- Average FPS: 155.54
- Average frame: 6.43 ms
- 1 percent low FPS: 122.75
- 0.1 percent low FPS: 84.56

Saturated portion, defined as live regular enemies `>= 60`:

- Samples: 23,915
- Duration: 155.20 gameplay seconds
- Average FPS: 154.08
- Average frame: 6.49 ms
- 1 percent low FPS: 122.89
- 0.1 percent low FPS: 87.59

High-saturation band, live regular enemies `>= 80`:

- Samples: 23,183
- Duration: 151.19 gameplay seconds
- Average FPS: 153.32
- Average frame: 6.52 ms
- 1 percent low FPS: 122.75
- 0.1 percent low FPS: 88.36

Board saturation:

- Peak live regular enemies: 90
- Peak reached at gameplay time 19.448 seconds
- First `>= 60` sample: gameplay time 13.425 seconds
- First `>= 80` sample: gameplay time 17.437 seconds
- Duration at `>= 80`: 151.19 gameplay seconds
- Active enemy projectiles remained 0 in sampled board metadata during this run.

Stalls and hitch events:

- `SpawnRuntimeTrickleWave` project-operation stalls: 0 events in this saturation session.
- Other `ProjectOperationStall` events above 16 ms: 0 events.
- `SingleFrameHitch` events: 0 events.
- Configured `SingleFrameHitch` threshold: 50 ms in `UT66PerformanceSystemSettings`.
- `FrameVarianceStutter` events: 3 early ramp-up events at gameplay times 0.005, 5.410, and 10.421 seconds, with board samples 3/0/0, 17/17/0, and 46/18/0 for live enemies, pending spawns, and enemy projectiles.
- `PerformanceSystemOverhead` events: 34 events, with framework cost ranging from 554 us to 5,144 us. These are measurement overhead warnings from the validation pass and should be watched if the per-frame saturation sample stream remains enabled beyond diagnostics.

Raw frame samples above one frame budget existed even though no `SingleFrameHitch` event was emitted:

- gameplay time 0.405 seconds: 795.31 ms, live enemies 3, pending spawns 0
- gameplay time 3.818 seconds: 310.28 ms, live enemies 7, pending spawns 27
- gameplay time 3.111 seconds: 99.54 ms, live enemies 3, pending spawns 0
- gameplay time 151.794 seconds: 63.24 ms, live enemies 90, pending spawns 0

This resolves the earlier contradiction: `SingleFrameHitch` is currently checked by the `FramePacingDetector` on a 1 second detector cadence, so a bad frame can appear in raw samples without becoming a `SingleFrameHitch` event. That detector-cadence gap is documented as a pending PerformanceSystem issue and should be fixed before relying on hitch-event counts as the only stall source.

Validation answer: with combat debug off and the HUD map throttled, the current full-actor enemy model held the 80-90 live-enemy range comfortably in this scenario. The measured FPS floor for the saturated `>= 80` band was 153.32 average FPS, 122.75 1 percent low FPS, and 88.36 0.1 percent low FPS. Mass migration remains the correct long-term architecture for hundreds of basic enemies, but this capture does not support treating Mass as the next immediate blocker for the current 90-enemy cap. The next Phase B scope should prioritize spawn/load attribution, single-frame hitch instrumentation accuracy, per-family timing, and projectile-pooling evidence before starting Mass scaffolding.

## Mass Migration Baseline Capture

Captured after the Mass-foundation instrumentation pass from the staged Development standalone. This is the fixed-instrumentation actor-model baseline that future Mass migration sessions should compare against.

- PerformanceSystem session: `Saved/StagedBuilds/Windows/T66/Saved/PerformanceSystem/Sessions/20260524T025550Z_f2RLT0IfeX1V1E-KrSKWKA`
- Standalone log: `Saved/StandaloneLogs/T66_MassMigration_Baseline.log`
- Unreal viewport screenshot: `Saved/Codex/Performance/MassBaseline/enemywaveperf_mass_baseline.png`
- Parsed metrics artifact: `Saved/Codex/Performance/MassBaseline/mass_baseline_metrics.json`
- Debug projectile screenshot: `Saved/Codex/Performance/MassBaseline/enemywaveperf_debug_projectiles.png`
- Debug projectile metrics artifact: `Saved/Codex/Performance/MassBaseline/debug_projectile_metrics.json`

Full gameplay window:

- Samples: 28,358
- Duration: 174.688 gameplay seconds
- Average FPS: 162.34
- Average frame: 6.160 ms
- 1 percent low FPS: 71.23
- 0.1 percent low FPS: 17.07
- Max frame: 899.386 ms
- Raw frames over 50 ms: 4

Saturated portion, defined as live regular enemies `>= 60`:

- Samples: 26,012
- Duration: 160.892 gameplay seconds
- Average FPS: 161.67
- Average frame: 6.185 ms
- 1 percent low FPS: 107.34
- 0.1 percent low FPS: 70.40
- Max frame: 28.108 ms
- Raw frames over 50 ms: 0

High-saturation band, live regular enemies `>= 80`:

- Samples: 25,353
- Duration: 156.883 gameplay seconds
- Average FPS: 161.60
- Average frame: 6.188 ms
- 1 percent low FPS: 107.57
- 0.1 percent low FPS: 70.27
- Max frame: 28.108 ms
- Raw frames over 50 ms: 0

Board saturation:

- Peak live regular enemies: 90
- Peak reached at gameplay time 19.318 seconds
- Duration at `>= 80`: 156.883 gameplay seconds
- Max pending spawns: 29
- Max active hostile projectiles in board samples: 2
- Active projectile samples: 27,666
- First nonzero active projectile sample: gameplay time 3.805 seconds, live regular enemies 6, pending spawns 28, active hostile projectiles 1

Instrumentation validation:

- `SingleFrameHitch` threshold: 50 ms
- Raw all-map frames over 50 ms: 6
- `SingleFrameHitch` all-map events: 6
- Raw GameplayLevel frames over 50 ms: 4
- `SingleFrameHitch` GameplayLevel events: 4
- Result: the fixed per-frame detector matched raw frame samples above threshold.
- `PerformanceSystemOverhead` events: 34
- Max framework cost: 1,210.403 us
- 95th percentile framework cost among overhead events: 952.400 us
- Result: framework overhead is now below the 2 ms acceptance threshold.

Projectile counter validation:

- Dungeon Stage 01 includes ranged mob rows (`HexSlinger`, `StoneSentinel`) and the debug run spawned `T66RangedEnemy` actors, but enemy-ranged projectile fire did not occur in the debug capture. The capture position is inside the safe zone and the ranged enemy behavior intentionally suppresses firing while the hero is safe.
- The same debug run did produce visible hostile trap-arrow projectiles. The log recorded 15 `Type=TrapProjectile` fire events, and the corrected board-saturation stream sampled active hostile projectiles up to 2.
- `ActiveEnemyProjectiles` is now used as the legacy field name for active hostile projectile actor load: enemy spit projectiles, unique debuff projectiles, and trap arrow projectiles. If Phase B needs source-specific attribution, split this into `ActiveEnemyProjectiles` and `ActiveTrapProjectiles` instead of changing the legacy field semantics again.

Project operations:

- `ProjectOperationStall` events: 0 in the final baseline session.
- `SpawnRuntimeTrickleWave` stalls: 0 in the final baseline session.
- `RefreshMapData` stalls: 0 in the final baseline session.

Baseline answer: with fixed hitch detection, lightweight PerformanceSystem snapshots, corrected hostile-projectile counting, combat debug off, and HUD map throttled, the actor model is still comfortable at the current 90 live-enemy cap. The saturated `>= 80` band is the number to compare against Mass: 161.60 average FPS, 107.57 1 percent low FPS, 70.27 0.1 percent low FPS, and no frames above the 50 ms hitch threshold.

## Actor Baseline at 300 Cap

Captured before any Mass plugin/module/scaffolding changes. The run used a non-shipping console override, `T66.EnemyDirector.MaxAliveOverride 300`, so no gameplay tuning data or cooked DataTables were edited. The CVar defaults back to `0`, which restores the normal data-driven cap. Raising `RuntimeEnemiesPerWave` was not needed because the board reached `>= 250` live enemies within 60 gameplay seconds.

- PerformanceSystem session: `Saved/StagedBuilds/Windows/T66/Saved/PerformanceSystem/Sessions/20260524T033922Z__dBCe0kd2O2c1_e1abfa8Q`
- Standalone log: `Saved/StandaloneLogs/T66_PhaseC1_Actor300Baseline.log`
- Unreal viewport screenshot: `Saved/Codex/Performance/MassC1/enemywaveperf_actor300_baseline.png`
- Parsed metrics artifact: `Saved/Codex/Performance/MassC1/actor300_baseline_metrics.json`
- Override approach: non-shipping `T66.EnemyDirector.MaxAliveOverride` CVar
- Wave size bumped: no

Full PerformanceSystem window:

- Average FPS: 57.22
- 1 percent low FPS: 46.47
- 0.1 percent low FPS: 41.63
- `SingleFrameHitch` events: 11
- Worst frame: 1308.996 ms
- Raw frames at or above 50 ms: 11
- Max `PerformanceSystemOverhead`: 4,456.799 us

Gameplay-only window:

- Samples: 14,434
- Duration: 218.107 gameplay seconds
- Average FPS: 66.18
- 1 percent low FPS: 44.76
- 0.1 percent low FPS: 34.72
- Worst frame: 1308.996 ms

Saturation band, live regular enemies `>= 80`:

- Samples: 12,034
- Duration: 199.926 gameplay seconds
- Average FPS: 60.19
- 1 percent low FPS: 44.52
- 0.1 percent low FPS: 36.88
- Worst frame: 173.971 ms

Saturation band, live regular enemies `>= 200`:

- Samples: 10,153
- Duration: 178.829 gameplay seconds
- Average FPS: 56.77
- 1 percent low FPS: 43.96
- 0.1 percent low FPS: 36.50
- Worst frame: 173.971 ms

Saturation band, live regular enemies `>= 250`:

- Samples: 9,614
- Duration: 170.761 gameplay seconds
- Average FPS: 56.30
- 1 percent low FPS: 43.91
- 0.1 percent low FPS: 36.22
- Worst frame: 173.971 ms

Board saturation:

- First `>= 80` sample: 84 live enemies at gameplay time 17.280 seconds
- First `>= 200` sample: 205 live enemies at gameplay time 38.386 seconds
- First `>= 250` sample: 251 live enemies at gameplay time 46.453 seconds
- Peak live regular enemies: 300
- Peak reached at gameplay time 55.515 seconds
- Max pending spawns: 29
- Peak active hostile projectiles: 2

Baseline answer: at the 300 live-enemy actor cap, the current ACharacter model is playable but no longer comfortable. The `>= 250` band held 56.30 average FPS, 43.91 1 percent low FPS, and 36.22 0.1 percent low FPS, with hitches still present. This gives Phase C a real actor-at-300 comparison point for Mass.

## Mass Rollback Verification

Captured after removing the Phase C.1 Mass plugin enablement, Mass module dependencies, Mass source folder, spawn-bridge shadow path, and Mass placeholder PerformanceSystem counters. Two unsuppressed long captures exited abruptly before `PerformanceSystem` finalization, so the final comparison capture suppressed only the high-volume `LogCharacterMovement` warning category with `-LogCmds="LogCharacterMovement off"`. Gameplay tuning, enemy cap, wave size, and spawn cadence were unchanged.

- PerformanceSystem session: `Saved/StagedBuilds/Windows/T66/Saved/PerformanceSystem/Sessions/20260524T062352Z_ZXhPCUVjTtHWOUmg9cQ1uA`
- Standalone log: `Saved/StandaloneLogs/T66_MassRollback_EnemyWavePerf_Final170_QuietMove.log`
- Unreal viewport screenshot: `Saved/Codex/Performance/MassRollback/enemywaveperf_mass_rollback_final170_quietmove.png`
- Parsed metrics artifact: `Saved/Codex/Performance/MassRollback/mass_rollback_final170_quietmove_metrics.json`
- SchemaVersion: 3
- ExitReason in PerformanceSystem report: `SubsystemDeinitialize`

High-saturation band, live regular enemies `>= 80`:

- Samples: 21,337
- Duration: 157.071 gameplay seconds
- Average FPS: 135.84
- Average frame: 7.361 ms
- 1 percent low FPS: 70.94
- 0.1 percent low FPS: 31.07
- Worst frame: 283.985 ms
- Raw frames over 50 ms: 1

Comparison to the fixed-instrumentation baseline:

- Baseline `>= 80` average FPS: 161.60
- Rollback `>= 80` average FPS: 135.84
- Delta: -15.94 percent
- Result: failed the within-5-percent recovery target. The rollback removed Mass plugin/code references, but this capture did not restore the old FPS floor.

Board saturation:

- Peak live regular enemies: 90
- Peak reached at gameplay time 18.456 seconds
- Duration at `>= 80`: 157.071 gameplay seconds
- Max pending spawns: 27
- Peak active hostile projectiles: 2
- `SingleFrameHitch` events: 5

Runtime plugin mount scan:

- `MassGameplay`: 0 mount lines
- `ZoneGraph`: 0 mount lines
- `ZoneGraphAnnotations`: 0 mount lines
- `SmartObjects`: 0 mount lines
- `GameplayCameras`: `LogPluginManager: Mounting Engine plugin GameplayCameras`
- `StateTree`: `LogPluginManager: Mounting Engine plugin StateTree`

`StateTree` did not come from Mass in this rollback build. The pre-Mass fixed baseline log also mounted `GameplayCameras` and `StateTree`, and local engine plugin inspection showed `GameplayCameras.uplugin` is enabled by default and depends on `StateTree`. An attempted explicit disable of `GameplayCameras`/`CameraShakePreviewer` compiled but caused repeat staging cook failures, so that optional plugin suppression was reverted.

Schema and output validation:

- `board_saturation_samples.jsonl`, `session_summary.json`, `session_summary.md`, and `events.jsonl` all emitted schema v3.
- The final session output contained no Mass placeholder fields: no `MassEnemy`, `MassProcessor`, `PromotedActor`, `MassProjectile`, `PromotionsPerSecond`, or `DemotionsPerSecond` fields.

PerformanceSystem overhead:

- Final long rollback capture `PerformanceSystemOverhead` events: 35
- Max framework cost: 1,091.100 us
- Result: the final long capture is below the 2 ms overhead target.
- Caveat: an earlier clean rollback capture still reached 2,965.599 us, so `Source/T66/PerformanceSystem/pending_issues_PerformanceSystem.md` remains open and was updated with both rollback measurements.

## Phase A Diagnostic: Baseline Recovery Investigation

Purpose: determine whether the Mass-rollback `>= 80` saturated-band result of 135.84 average FPS was a deterministic regression or run-to-run measurement variance before scoping the Lightweight Actor pass.

Methodology:

- Ran 5 consecutive staged Development standalone captures with no code or tuning changes between runs.
- Used the same `-T66Entry=Run:Tower` and `-T66GameplayAutoCapture=enemywaveperf` path as the rollback verification.
- Kept the same 90 live-enemy cap, 30 enemies per wave, 5 second wave interval, and 5 second stagger.
- Used `-LogCmds="LogCharacterMovement off"` for all 5 captures, matching the finalized rollback comparison run and preventing the high-volume stuck-movement log category from preventing long capture finalization.
- Parsed each run from `board_saturation_samples.jsonl` for frames where `LiveRegularEnemies >= 80`.
- Aggregate metrics artifact: `Saved/Codex/Performance/VarianceCharacterization/variance_characterization_summary.json`

| Run | PerformanceSystem session | `>= 80` avg FPS | `>= 80` 1% low | `>= 80` 0.1% low | Max PerformanceSystem overhead |
| --- | --- | ---: | ---: | ---: | ---: |
| 1 | `Saved/StagedBuilds/Windows/T66/Saved/PerformanceSystem/Sessions/20260524T064509Z_kiZfz019_AAr3puK0RDAOQ` | 147.04 | 77.59 | 30.53 | 16,659.997 us |
| 2 | `Saved/StagedBuilds/Windows/T66/Saved/PerformanceSystem/Sessions/20260524T065008Z_O3xo1k589R8CwS66gnjthw` | 144.39 | 87.95 | 39.65 | 1,032.602 us |
| 3 | `Saved/StagedBuilds/Windows/T66/Saved/PerformanceSystem/Sessions/20260524T065354Z_X4zQYkTI1dj6ubmImHyUOw` | 155.02 | 88.53 | 37.66 | 1,233.302 us |
| 4 | `Saved/StagedBuilds/Windows/T66/Saved/PerformanceSystem/Sessions/20260524T065738Z_b1SUuExSGIxauneMT143hg` | 160.89 | 94.14 | 37.70 | 797.100 us |
| 5 | `Saved/StagedBuilds/Windows/T66/Saved/PerformanceSystem/Sessions/20260524T070122Z_SoY3_kvJj59c0dqDeNwggw` | 151.37 | 84.99 | 35.87 | 806.797 us |

Computed `>= 80` average-FPS distribution:

- Fixed-instrumentation baseline comparator: 161.60 average FPS
- Within-5-percent band for baseline: 153.52 to 169.68 average FPS
- Min: 144.39
- Max: 160.89
- Range: 16.50
- Median: 151.37
- Mean: 151.74
- Population standard deviation: 5.84
- Sample standard deviation: 6.53

Conclusion: mixed variance, not a proven deterministic 135 FPS regression.

This result lands between the prompt's two clear branches. The median, 151.37, is below the 5 percent recovery band for the 161.60 baseline, so it does not prove that rollback fully restored the old fixed-instrumentation baseline. However, the five-run range is 16.50 FPS, one run reached 160.89 FPS, and the data does not match the deterministic-regression criterion of "median around 135 and range below 10 FPS." Task 2 commit-culprit analysis was therefore not run in this pass.

Recommendation: do not accept 135.84 as the new working baseline from a single run. Use this five-run median of 151.37 FPS as the current practical rollback comparator, and treat the remaining 6.33 percent gap versus 161.60 as unresolved measurement/runtime variance. Before making Lightweight Actor performance claims, compare future captures against a five-run median rather than a single session, and keep `Source/T66/PerformanceSystem/pending_issues_PerformanceSystem.md` open because run 1 recorded a 16.66 ms PerformanceSystem framework-cost spike while the other four runs stayed near 0.8-1.2 ms.
