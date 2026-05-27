# Pending Issues - Gameplay

## Hero 1 Axe AOE VFX Lab Uses Deprecated Niagara Emitter Readiness API

- Severity tag: [Minor]
- What's wrong: Focused UI build and standalone stage for the LootWheel cleanup emitted `C4996` from `Source/T66/Gameplay/T66Hero1AxeAOEVFXLabActor.cpp` because `FNiagaraEmitterInstance::IsReadyToRun` is deprecated.
- Why it's out of scope now: The current pass only removes LootWheel UI marks and verifies the staged LootWheel capture. Updating Niagara lab readiness handling would touch unrelated Gameplay/VFX code.
- What fixing it would entail: Audit the lab actor's emitter readiness check against the UE 5.7 Niagara execution-path API, replace the deprecated call, then run a focused Gameplay/VFX compile and any existing Hero 1 axe lab capture checks.

## Legacy MainMapTerrain Dirt Grass And Rock Names Are Dormant But Confusing

- Severity tag: [Minor]
- What's wrong: `Source/T66/Gameplay/T66MainMapTerrain.cpp` still contains legacy names and helper paths for dirt, grass, rock decoration, and farm decor/supports, even though `bRenderFarmGrass`, `bRenderFarmDecor`, and `bRenderFarmSupports` are false and the loaded grass/rock meshes are null. These names can mislead future cleanup work into thinking there are active map clutter assets when the current runtime path is disabled/dormant.
- Why it's out of scope now: The current cleanup removes the live placed rock actors/assets and disables the active tower runtime rock/deco spawner. Renaming or deleting dormant terrain internals is a separate legacy-code cleanup with broader regression risk.
- What fixing it would entail: Audit `T66MainMapTerrain.cpp` for dormant decor/support code, decide whether to remove the unused paths or rename them to neutral terrain-surface terminology, compile, and verify classic/tower terrain generation still builds correctly.

## Lightweight Ranged Autocapture Can Kill The Stationary EnemyWavePerf Hero

- Severity tag: [Blocker]
- What's wrong: Pass B.10 moved Ranged-family mobs onto `AT66MobBase` and proved the projectile path in smoke, but the CVar-on `enemywaveperf` acceptance set halted after two non-zero exits. Both non-zero exits were real hero deaths from lightweight `EnemyProjectile` hits, not shutdown crashes. Example fatal lines show `SourceID=StoneSentinel Delivery=EnemyProjectile SourceActor=T66MobBase_* HeroHP=20.0->0.0` at world times `17.58` and `28.56`. A clean CVar-on attempt also had to be rejected because `PerformanceSystemOverheadMaxUs=268567.0`.
- B.10.1 update: route diagnostics landed and the partial same-binary comparison points at a lightweight Ranged parity gap. RA (`UseLightweight=1`, Rush/Flying lightweight, Ranged rich) produced 6/6 route-valid survivals, 0 projectile hits, and 0 projectile damage before the set halted on two PerformanceSystem overhead rejections. A post-patch full-lightweight attribution rerun produced 5/5 route-valid captures, 21 projectile hits, 420 HP of projectile damage, and 3/5 hero deaths. A single post-patch RB preflight also showed full-lightweight Ranged landing a projectile hit while the hero survived at 80 HP. This is not enough for B.10 acceptance, but it strongly favors "lightweight Ranged is more aggressive than rich Ranged in practice" over a pure measurement-contract issue.
- B.10.1B update: after the PerformanceSystem write-queue fix, the clean 10x10 same-binary diagnostic showed a route-dependent split in observed Ranged outcomes without overhead rejects. RA completed 10/10 route-valid captures with 0 hero deaths, 0 projectile hits, and 0 projectile damage. RB completed 10/10 route-valid captures with 4 hero deaths, 31 projectile hits, and 620 projectile damage. B.10 remains blocked on Ranged parity rather than a measurement-contract change in this pass; the next reviewed packet needs to explain RA's zero projectile activity and make lightweight Ranged match the rich path's effective pressure.
- Why it's out of scope now: Fixing this requires a reviewed gameplay parity packet after the PerformanceSystem overhead fix unblocks clean RA/RB capture sets. Adding hero safety or parking the hero remains out of scope and would hide the regression signal.
- What fixing it would entail: Drill into rich-vs-lightweight Ranged movement efficiency, LOS/fire cadence, projectile spawn failure, and distance-band behavior. Then either make lightweight Ranged match rich pressure or intentionally rebalance the Ranged data with Pablo go-ahead, followed by a clean 10x10 RA/RB diagnostic and the original B.10 acceptance rerun.

## Resolved: Backrooms GameMode Handler Declarations Blocked Fresh Link

- Severity tag: [Resolved - Blocker]
- What's wrong: A fresh B.10 Development link exposed stale `AT66GameMode` Backrooms handler declarations that had no definitions, plus private `UPROPERTY` declarations for placeholder Backrooms classes that were not visible to UHT in the current source state. This was unrelated to lightweight Ranged, but it blocked the required clean build.
- Resolution: The private Backrooms actor storage in `AT66GameMode` now uses `TObjectPtr<AActor>` and the declared Backrooms handlers have defensive no-op definitions that log if invoked. This restores build/link integrity without implementing or enabling Backrooms gameplay.
- Follow-up: Backrooms gameplay remains unimplemented/disabled. A future Backrooms pass should replace the defensive no-ops with real door/chaser flow and concrete actor classes or remove the stale declarations entirely.

## Trap Projectile Fire Logs Are Still Normal-Level Hot-Path Telemetry

- Severity tag: [Major]
- What's wrong: B.7 lightweight actor validation still surfaced routine `LogT66TrapProjectile` projectile-fire lines during gameplay captures. Projectile fire is a per-spawn/per-projectile hot path, and normal `Log`/`Display` output under `-forcelogflush` can materially distort performance captures, as seen with the lightweight mob routing logs in B.6.1.
- Why it's out of scope now: Pass B.7 is scoped to lightweight mob pooling plus HUD/minimap widening. Changing trap projectile logging would touch separate trap/projectile diagnostics and should be verified with its own focused smoke.
- What fixing it would entail: Audit `Source/T66/Gameplay/T66TrapProjectile.*` and related trap fire paths, demote routine fire/impact telemetry to `VeryVerbose`, preserve warnings/errors at their current levels, then rerun a short staged gameplay capture with `-forcelogflush` to confirm the log stream stays quiet.

## Mixed Rich/Lightweight Enemy Capsules Trigger CharacterMovement Stuck Logs

- Severity tag: [Major]
- What's wrong: Pass B.9 CVar-on capture logs showed rich `AT66RangedEnemy` actors repeatedly emitting `LogCharacterMovement` stuck messages while colliding with `AT66MobBase` capsules, for example `Actor:T66MobBase_* Component:MobCapsule`. These are normal-level engine logs under `-forcelogflush`, and they can distort performance captures the same way other hot-path telemetry has.
- B.9.1 diagnostic update: the `-LogCmds="LogCharacterMovement Off"` isolation set removed all stuck-log lines and improved the full-B.9 median from `175.81` to `180.22`, but the `+4.41 FPS` delta was below the `12.74 FPS` 2x-stdev significance threshold. This is still a real log hygiene/collision issue, but it did not explain the earlier B.9 parity miss.
- Why it's out of scope now: Pass B.9.1 is diagnostic-only. Changing rich/lightweight collision response could affect combat contact, body blocking, and future Ranged migration behavior.
- What fixing it would entail: Decide whether rich enemies should block, overlap, or ignore lightweight mob capsules; update the `AT66MobBase` capsule collision profile or rich enemy movement/collision response accordingly; then rerun mixed rich/lightweight staged captures to verify no stuck-log spam and no touch-damage or targeting regressions.

## Resolved: Lightweight Flying Routing Missed B.9 CVar-On Parity Target

- Severity tag: [Resolved - Major]
- What's wrong: Pass B.9 functionally migrated Flying mobs to `AT66MobBase`, but the final clean CVar-on ten-capture tiebreaker landed at `162.80` median FPS against the required `169.88` floor from the B.8.1 Melee+Rush comparator. The same clean CVar-off ten-capture set passed at `156.62` median FPS, so lightweight routing remains faster than rich-only gameplay, but Flying reduced the previous Melee+Rush gain. Full tables are in `PerformanceSystem/2026-05-23_T66_LightweightActor_Plan.md`.
- Resolution: Pass B.9.1 added diagnostic CVars and ran same-binary 10-capture isolation sets. The full B.9 state landed at `175.81` median FPS, above the `169.88` B.9 floor, and the B.8-equivalent Melee+Rush set landed at `179.85`, matching prior accepted data. The full-state delta was `-4.05 FPS` against a `15.60 FPS` 2x-stdev threshold, so the B.9 miss did not reproduce as a statistically distinguishable regression.
- Follow-up: Proceed to B.10 Ranged. Keep `T66.Mob.Diagnostics.RouteFlyingLightweight=1` and `T66.Mob.Diagnostics.UseTouchDamageOverlap=1` as default production behavior until final cleanup. Flying is still the expensive family inside the manager profiler (`29.225 us` median per Flying sample versus `0.833 us` for Melee in saturated-like windows), so revisit it before any 300-cap experiment or if a later same-binary diagnostic shows a repeatable regression.

## Resolved: Lightweight Rush Routing Missed B.8 CVar-On Parity Target

- Severity tag: [Resolved - Major]
- What's wrong: Pass B.8 functionally migrated Rush-family mobs to `AT66MobBase`, but the CVar-on performance gate did not recover to the B.7 CVar-on comparator. The first CVar-on set landed at `163.48` median FPS and the confirmation set landed at `168.75`, below the required `169.65` floor. Details are in `PerformanceSystem/2026-05-23_T66_LightweightActor_Plan.md`.
- Resolution: Pass B.8.1 added diagnostic grouped timing and ran same-binary 10-capture comparisons. Melee+Rush lightweight routing landed at `178.82` median FPS; Melee-only landed at `179.22`, a `0.40 FPS` delta against a `2x stdev` threshold of `8.18 FPS`. The data supports Outcome Z: the B.8 miss was measurement variance, not a statistically distinguishable Rush regression.
- Follow-up: Proceed to B.9 Flying. Keep `T66.Mob.ManagerTickProfileEnabled=0` by default; the diagnostic hook can be reused if a later family migration shows a repeatable performance miss.

## Resolved: Lightweight Mob Perf Capture Touch Damage Parity

- Severity tag: [Resolved - Major]
- What's wrong: With `T66.Mob.UseLightweight=1`, `-T66GameplayAutoCapture=enemywaveperf` left the direct-entry hero stationary long enough for manager-driven `AT66MobBase` touch damage to kill the hero before the delayed screenshot/exit timer fired. In the B.6 CVar-on parity set, attempts 3 and 4 both reached 90 live enemies but timed out after repeated `SourceID=T66MobBase Delivery=EnemyTouch` damage drove HeroHP to 0.
- Resolution: Pass B.6.1 changed `UT66MobManagerSubsystem::ApplyMobTouchDamageIfNeeded` to match `AT66EnemyBase::OnCapsuleBeginOverlap` semantics: damage fires on the transition from out-of-range to in-range, remains disarmed while the hero stays in range, and rearms only after the mob leaves damage range. The `enemywaveperf` hero now survives the unmodified CVar-on five-capture set without adding a safety mode or changing `AT66EnemyBase`.
- Follow-up: None for this specific issue. A future gameplay design pass can still choose range-driven repeated touch damage, but that should change rich and lightweight enemies together.

## Direct-Entry Companion Selection Does Not Reach Spawn Path

- Severity tag: [Minor]
- What's wrong: A staged smoke run launched with `-T66Entry=Run:Tower -T66Companion=Companion_01` sets `UT66GameInstance::SelectedCompanionID`, but `T66GetSelectedCompanionID` prefers `AT66SessionPlayerState::GetSelectedCompanionID()` when a session player state exists. In direct-entry runs that player-state value remains `None`, so `AT66GameMode::SpawnCompanionForPlayer` does not spawn the requested companion.
- Why it's out of scope now: The current pass imports animated humanoid visuals and wires companion animation mirroring; changing direct-entry/session loadout authority could affect broader party and multiplayer flow.
- What fixing it would entail: Add a targeted direct-entry bridge that applies requested hero/companion loadout into the local `AT66SessionPlayerState`, or make `T66GetSelectedCompanionID` fall back to the game instance only for direct-entry automation when the session value is empty; then add a staged companion-spawn smoke capture.

## Combat Damage Sources Are Not All Geometric Damage Volumes

- Severity tag: [Major]
- What's wrong: The combat debug view can draw current `Hurtbox` and `DamageVolume` primitives, but several health-loss paths still apply damage from direct rules rather than a drawable combat volume. Examples include `Source/T66/Gameplay/T66MiasmaBoundary.cpp`, `Source/T66/Gameplay/T66MiasmaManager.cpp`, `Source/T66/Gameplay/T66LoanShark.cpp`, and `Source/T66/Gameplay/T66TutorialManager.cpp`. These paths now emit `[CombatDamage]` provenance in the Unreal log when HP is actually removed, but that does not yet make every abstract damage rule visually inspectable in-world.
- Why it's out of scope now: The current pass adds semantic debug visibility over existing combat and hazard primitives without redesigning abstract/global damage rules.
- What fixing it would entail: Route direct damage through a shared combat-damage event contract with optional debug providers, or give each abstract damage rule a clear world-space debug representation and source label.

## Enemy Touch Damage Intent Is Split Across Old Overlap And Hero Proximity Logic

- Severity tag: [Major]
- What's wrong: `Source/T66/Gameplay/T66EnemyBase.cpp` now filters touch damage to the hero capsule, rejects far overlap events, and uses `TouchDamageHearts`, but `Source/T66/Gameplay/T66HeroBase.cpp` still has a separate enemy proximity check that currently handles bounce behavior. The old bug where the hero's large attack-range sphere could trigger `Delivery=EnemyTouch` from thousands of units away is guarded, but the long-term authority split remains.
- Why it's out of scope now: The current pass fixes the false-overlap damage path and projectile visibility without redesigning the separate hero-side proximity/bounce behavior.
- What fixing it would entail: Pick one authoritative touch-damage path, use `TouchDamageHearts` consistently, remove the stale overlap/proximity split, and add a gameplay smoke test that confirms melee contact damage and cooldown behavior.

## Spawn Director Still Uses Fallback-Family Behavior

- Severity tag: [Major]
- What's wrong: `Source/T66/Gameplay/T66EnemyDirector.cpp` now reads up to 10 stage slots, but production archetype selection is still routed through `FamilyID` fallback classes. The director does not yet account for the remaining non-ranged special archetype mix as distinct mechanics. Ranged subsections were intentionally collapsed into the single `Ranged` class path for now.
- Why it's out of scope now: The current pass must keep the stage spawn path working while the new archetype classes are not implemented.
- What fixing it would entail: Refactor the director to choose from weighted archetype quotas, consume `Archetype` directly, and add deterministic handling for empty stage slots and unsupported archetypes.

## Hell Core Has No Ranged Mob

- Severity tag: [Minor]
- What's wrong: Hell stages intentionally use core mobs `PitImp`, `BoneKnight`, `FireSkull`, `Hellhound`, and `Gargoyle`, which contain no `Ranged` fallback family. Any old logic expecting a guaranteed ranged slot could behave differently in Hell.
- Why it's out of scope now: The roster explicitly marks this as intentional and says not to force a ranged substitution.
- What fixing it would entail: Verify Hell-stage spawn variety after the archetype-aware director refactor and add explicit no-ranged handling if design needs it.

## Player Experience Tuning Can Be Requested Before DataTable Is Available

- Severity tag: [Minor]
- What's wrong: The Easy mob VAT staged gameplay smoke logged `PlayerExperience tuning requested by GetDifficultyStartStage before DataTable '/Game/Data/DT_PlayerExperience.DT_PlayerExperience' was available; returning empty tuning.`
- Why it's out of scope now: The VAT pass only changed enemy visual animation data and did not change the player-experience subsystem initialization order.
- What fixing it would entail: Audit the player-experience subsystem load sequence, make the tuning table available before gameplay difficulty startup queries, and add a staged smoke assertion that this warning no longer appears.

## Inter-Walkable-Box Floor Seams Remain Possible

- Severity tag: [Major]
- What's wrong: `Source/T66/Gameplay/T66TowerMapTerrain.cpp` now emits one generated floor rectangle per walkable source box, which removes internal subdivision seams, but seams can still appear where separate walkable boxes or drop-hole carve-out rectangles meet.
- Why it's out of scope now: Eliminating those joins requires changing floor-box generation/merging semantics beyond the requested visual assembly fix.
- What fixing it would entail: Merge each gameplay floor's compatible walkable rectangles into a single visual surface, or author a dedicated runtime mesh for the floor footprint so there are no visual boundaries between adjacent boxes.

## Drop-Hole Rectangle Floors Are Split Around The Opening

- Severity tag: [Minor]
- What's wrong: `Source/T66/Gameplay/T66TowerMapTerrain.cpp` now uses the rectangle-plus-texture path for generated dungeon floors, and drop-hole floors are split into up to four rectangles around the opening. This preserves layout and avoids new material work, but it can still leave visible seams at the hole boundary if the texture reads across the split.
- Why it's out of scope now: The map transition intentionally ships the first-pass test-room rectangle approach with no masked-material or procedural-cutout extension until Pablo reviews the live result.
- What fixing it would entail: Add either a masked floor material with per-spawn hole parameters or a procedural floor cutout mesh so each drop-hole floor can render as one continuous surface with a geometric opening.

## Non-Dungeon Theme Atmosphere Specs Need Authoring

- Severity tag: [Minor]
- What's wrong: `Source/T66/Gameplay/T66ThemeAtmosphereData.cpp` now contains the first Dungeon atmosphere spec, but Forest, Ocean, Martian, and Hell return neutral lighting/fog/grading values.
- Why it's out of scope now: Atmosphere Iteration 01 is explicitly Dungeon-only so Pablo can validate the foundation before extending vibe-setter values to other themes.
- What fixing it would entail: Author and tune per-theme sky light, fog, and color-grading specs for Forest, Ocean, Martian, and Hell, then validate them in staged gameplay screenshots.

## NPC Class Names Still Use HouseNPC

- Severity tag: [Minor]
- What's wrong: The data/setup/runtime table seam now uses `NPCs.csv`, `DT_NPCs`, `NPCsDataTable`, and `GetNPCData`, but the underlying C++ actor/data symbols still include `AT66HouseNPCBase`, `FHouseNPCData`, and related subclass/include names in `Source/T66/Gameplay` and registry consumers.
- Why it's out of scope now: The current cleanup is constrained to data/loader/source naming and avoids broad C++ class renames that would touch generated headers, includes, Blueprint references, and asset class bindings.
- What fixing it would entail: Rename the C++ base actor and data struct to neutral NPC names, add Unreal redirects if needed, update all includes/subclasses/registry references, compile, and verify existing Blueprint/class references still resolve.

## Vehicle Class Names Still Use Tractor

- Severity tag: [Minor]
- What's wrong: The authored data row, prompts, damage source token, and interaction row lookup now use `Vehicle`, but the inherited C++ implementation still uses `AT66PilotableTractor`, `T66PilotableTractor.*`, and enum/action names such as `PilotTractor`, subobject names such as `TractorRoot`, and local variable names such as `ClosestTractor` in player interaction code.
- Why it's out of scope now: The current pass is scoped to data/schema/loader cleanup and avoids broad class/file renames that would touch generated headers, includes, actor references, and Blueprint bindings.
- What fixing it would entail: Rename the C++ class/files to a neutral vehicle name, update includes and player-controller interaction variables, add Unreal redirects if required, compile, and verify existing spawned/interactable vehicle references still resolve.

## TutorialGate Class Remains After Tutorial Exit Uses StageGate

- Severity tag: [Minor]
- What's wrong: `Source/T66/Gameplay/T66TutorialGate.*` and the `AT66TutorialGate` interaction path in `Source/T66/Gameplay/T66PlayerController_Combat.cpp` still exist, but `Source/T66/Gameplay/T66TutorialManager.cpp` now spawns `AT66StageGate` for the tutorial exit.
- Why it's out of scope now: This pass was intentionally constrained to swapping the tutorial end to the regular Stage Gate without broad class/file deletion or reference cleanup.
- What fixing it would entail: Prove there are no Blueprint, asset, map, or automation references to `AT66TutorialGate`, then remove the class, includes, player-controller branch, and any stale generated/API references with a focused compile and content reference check.
