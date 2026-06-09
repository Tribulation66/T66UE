# World Runtime Teardown Audit

This is the first-pass matrix for world-runtime systems that may need pre-travel drain or shutdown participation. It is intentionally conservative: rows marked `needs proof` are not confirmed leaks.

Cross-check this matrix against `ShutdownSystem/SHUTDOWN_REGISTRY.md` before adding shutdown participants or lifecycle drain hooks. `native-unreal` in this audit means safe for normal world teardown; it is not a claim that player-quit shutdown ordering owns or bypasses the system.

Pass 1 foundation ownership evidence lives in `FOUNDATION_OWNERSHIP_INVENTORY.md` with generated scan output in `FOUNDATION_OWNERSHIP_SCAN.md` / `FOUNDATION_OWNERSHIP_SCAN.json`. The scan intentionally over-includes `EndPlay` and cleanup references; this audit remains the authority for deciding whether a cleanup hook needs pre-travel drain.

| System | Lifetime | Ticks | Timers | Delegates | Async Loads | Pooled Actors / Components | Current Cleanup Evidence | Pre-Travel Drain Need | Shutdown Registry Link | Risk |
|---|---|---:|---:|---:|---:|---|---|---|---|---|
| `UT66ActorRegistrySubsystem` | `UWorldSubsystem` | no | no | multicast fields only; no external handle found | no | weak actor registries only | Stores `TWeakObjectPtr` arrays and prunes invalid entries on register/unregister/query; no owned actors, components, timers, or async work found. | none; native world teardown is enough | `native-unreal` | low |
| `UT66EnemyPoolSubsystem` | `UWorldSubsystem` | no | no | no | no | weak refs to pooled inactive enemies | `Deinitialize` empties the weak pool; pooled enemies are world-owned actors hidden/disabled by `Release`. No owned timer/delegate/async work found. | none; native world teardown is enough | `native-unreal` | low |
| `UT66TrapSubsystem` | `UWorldSubsystem` | no | no subsystem timers found | no external handle found | no | weak refs to trap actors and managed pressure plates | `ClearManagedTrapActors` destroys managed traps during tower trap spawn/stage refresh; tracked arrays are weak refs. No teardown participant needed for world destruction. | none; existing helper is stage-spawn cleanup, not a world drain coordinator | `native-unreal` | low |
| `UT66ToonOutlineViewSubsystem` | `UTickableWorldSubsystem` | yes | no | no | no | none found | Tick only calls `FT66WorldVisualSetup::UpdateToonOutlineViewParameters(GetWorld())`; no owned actors, components, handles, or cached state found. | none; native world teardown is enough | `native-unreal` | low |
| `UT66FloatingCombatTextPoolSubsystem` | `UWorldSubsystem` | no | yes | no external handle found | no | active/inactive text actors | `Deinitialize` clears all release timers, destroys active and inactive pooled actors, resets arrays, and uses weak timer keys. | owner-local drain is self-contained; keep out of global coordinator unless repeated travel proves timer ordering trouble | owner-local self-contained | low |
| `UT66MobManagerSubsystem` | `UWorldSubsystem`, `FTickableGameObject` | yes | one next-tick release delegate | no external handle found | no | weak active/inactive lightweight mob pools plus vertex-animation runtime state | `Deinitialize` resets active/inactive weak arrays and VAT state; `NotifyMobDying` schedules a weak next-tick release delegate; overflow destroys mobs but normal pooled actors remain world-owned. | owner-local drain candidate; needs repeated travel count proof before declaring native-safe | `planned` | needs proof |
| `UT66MobLootSubsystem` | `UTickableWorldSubsystem` | yes | no subsystem timers found | no external handle found | no async load found | slot arrays, Niagara component, fallback HISM, render host actor | `Deinitialize` deactivates/destroys Niagara, destroys fallback HISM and render host, resets slots/uploads. | owner-local drain candidate; needs actor/component count proof across travel | `planned` | needs proof |
| `UT66ProjectileManagerSubsystem` | `UWorldSubsystem`, `FTickableGameObject` | yes | no subsystem timers found | no external handle found | static Niagara async-load cache/active-load map in helper | active projectile array, Niagara body/trail components, HISM buckets, render host actor | `Deinitialize` sets `bShuttingDown`, cleans projectile trails, destroys HISM components and render host, resets arrays/buckets. Static asset load handles are not world-owned but should be counted before shutdown classification. | owner-local drain candidate; needs projectile/component/async-load count proof across travel | `planned` | needs proof |
| `UT66BossHazardSubsystem` | `UWorldSubsystem`, `FTickableGameObject` | yes | no subsystem timers found | no external handle found | no | active hazards, HISM render buckets, render host actor | `Deinitialize` sets `bShuttingDown`, destroys HISM components and render host, clears hazards/buckets. | owner-local drain candidate; needs hazard/component count proof across travel | `planned` | needs proof |
| `UT66OutgoingTravelerPoolSubsystem` | `UTickableWorldSubsystem` | yes | no subsystem timers found | no external handle found | no async load found | traveler slots, target snapshot, Niagara component, render host actor | `Deinitialize` optionally writes proof manifest, deactivates/destroys Niagara, destroys render host, resets slots/uploads. | owner-local drain candidate; needs traveler/component count proof across travel | `planned` | needs proof |
| `UT66PixelVFXSubsystem` | `UWorldSubsystem` | no subsystem tick | no | no external handle found | async Niagara default-system load handle | spawned auto-destroy Niagara components | `Deinitialize` resets the streamable handle and cached systems; spawned Niagara components are world-owned/auto-destroy, but no aggregate travel proof exists. | async-load owner-local candidate; needs async-handle and spawned-component count proof across travel | `planned` | needs proof |

## Pass 5 Audit Outcome

Pass 5 does not justify a new `UT66WorldRuntimeCoordinatorSubsystem` yet. Current source evidence splits the world-runtime surface into:

- **Native-safe for world teardown:** `UT66ActorRegistrySubsystem`, `UT66EnemyPoolSubsystem`, `UT66TrapSubsystem`, and `UT66ToonOutlineViewSubsystem`.
- **Owner-local self-contained drain:** `UT66FloatingCombatTextPoolSubsystem`; teardown safety depends on its own `Deinitialize` clearing release timers and destroying active/inactive pooled actors, but no global coordinator need is proven.
- **Owner-local drain / proof candidates:** `UT66MobManagerSubsystem`, `UT66MobLootSubsystem`, `UT66ProjectileManagerSubsystem`, `UT66BossHazardSubsystem`, `UT66OutgoingTravelerPoolSubsystem`, and `UT66PixelVFXSubsystem`.

The remaining candidates already have owner-local `Deinitialize` paths or world-owned resources. The missing proof is not an obvious cleanup function; it is repeated world-travel evidence that active actors, render components, timers, and async handles do not survive longer than their world.

## Next Audit Pass

Before implementing a `UT66WorldRuntimeCoordinatorSubsystem`, collect targeted proof for the owner-local candidates above. Use the development-only observer harness first:

- `T66.WorldRuntime.Snapshot Path=<manifest.json> Label=<label>` writes one current-world snapshot.
- `T66.WorldRuntime.ProofTravel Path=<manifest.json> Travels=<count> Delay=<seconds>` alternates frontend/gameplay travel and writes snapshots after each load.
- `T66.WorldRuntime.ProofTravel ... Stress=1 StressCount=<count> StressSettle=<seconds>` first enters gameplay, creates bounded active resources for the six proof candidates through owner APIs, captures `before_travel_stress`, then runs the same repeated travel proof.

Default output is `Saved/WorldRuntimeProof/`. The harness is implemented in `Source/T66/Core/T66WorldRuntimeProofCommands.cpp` and uses owner-local non-shipping snapshots on:

- `UT66MobManagerSubsystem`
- `UT66MobLootSubsystem`
- `UT66ProjectileManagerSubsystem`
- `UT66BossHazardSubsystem`
- `UT66OutgoingTravelerPoolSubsystem`
- `UT66PixelVFXSubsystem`

The manifest is evidence-only. A non-current world resource count is a leak candidate that needs review, not automatic approval for a coordinator.

Collect and compare:

- actor counts before and after repeated world travel
- active timers before and after world travel
- delegate handles left registered after travel
- async load handles still valid during travel
- components or HISM render buckets surviving longer than their world

Only promote this to runtime implementation if multiple systems need ordered drain, bounded pre-travel cleanup, or shared diagnostics that cannot stay owner-local.

## Active Stress Proof Result

The first active stress proof used:

`T66.WorldRuntime.ProofTravel Path=Saved/WorldRuntimeProof/codex_active_stress_travel_proof_6x_20260607.json Travels=6 Delay=0.75 Stress=1 StressCount=6 StressSettle=0.50 ExitOnComplete=1`

The `before_travel_stress` snapshot proved live current-world resources for all six owner-local candidates: 6 mobs, 6 mob-loot drops, 6 managed projectiles, 6 boss hazards, 6 outgoing travelers, and 6 pixel VFX spawns. All six subsequent travels completed with `non_current_world_proof_candidate_resource_count=0`, and owner-local counters returned to baseline on later worlds.

This active-stress evidence does not justify `UT66WorldRuntimeCoordinatorSubsystem`. Keep teardown owner-local/native-safe for now unless a future, heavier gameplay scenario proves cross-owner ordering or shared timeout diagnostics are required.
