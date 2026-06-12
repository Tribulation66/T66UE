# Pending Issues - Gameplay

## Moving Lift Platforms V1 Edge Behaviors

- Severity tag: [Minor]
- What's wrong: The 2026-06-10 Phase 2 lift pass (`AT66TowerLiftPlatform`, design contract `Gameplay/World/T66_MAP_DESIGN_REFERENCE.md` section 1.4) ships three deliberate v1 edge behaviors: (1) a descending slab over a hero resolves by depenetration shove (the 30uu bottom rest height prevents pinning, but the shove is unpolished); (2) lift cycle phase restarts on save/load floor re-entry (same transient class as the per-floor lava clocks); (3) enemies neither ride nor avoid lifts (no nav impact, hero-only feature, same scope class as enemies ignoring lava).
- Why it's out of scope now: The pass scoped to the moving-collision/riding contract, seeded placement, and the no-softlock audit extension; crush polish, persistence, and enemy interaction are separate behavior passes.
- What fixing it would entail: (1) add a pre-descent hero-overlap check that pauses the descent or applies a gentle outward launch; (2) persist per-lift cycle time in floor state alongside a future lava-clock persistence fix; (3) decide the enemy rule (avoidance vs riding) together with the enemy lava rule.

## Resolved: Staged Readiness Lifecycle Stress Expects Mob Loot While Mob Loot Is Shelved

- Severity tag: [Resolved - Major]
- What's wrong: The 2026-06-08 staged readiness run at `Saved/StagedBuildReadiness/20260608_122548` passed staging, frontend smoke, durable-save smoke, and repeated tower room proof markers, but the lifecycle gate reported `BUILD_CONFIG_UNSUPPORTED`/FAIL because `stress_population.mob_loot_spawned` was `0` instead of `6`. The child manifest shows `T66.WorldRuntime.ProofTravel` completed all 6 travels and spawned mobs/projectiles/hazards/travelers/pixel VFX, while the current dirty `UT66MobLootSubsystem` has `T66.MobLoot.Enabled` defaulted to `0` and is additionally gated by `FT66ShelvedFeatureGate::IsMobLootEnabled()`, so the lifecycle stress harness cannot satisfy its mob-loot expectation.
- Why it's out of scope now: The current pass is scoped to tower floor 2/3 room count, tile size, room sizes, room traps, room content, and per-floor vendor rules. Unshelving or changing the mob-loot lifecycle stress contract would touch separate loot-system/product-readiness policy.
- What fixing it would entail: Decide whether mob loot should be unshelved for Development readiness, whether `RunLifecycleTransitionSmokeGate.ps1` should skip `mob_loot_spawned` while the feature is shelved, or whether `T66PopulateWorldRuntimeStress` should use a non-shelved resource for that assertion; then rerun `Scripts/RunStagedBuildReadinessGate.ps1` until the full suite passes.
- Resolution: Resolved 2026-06-09: `T66PopulateWorldRuntimeStress` now reports `mob_loot_enabled` (`UT66MobLootSubsystem::IsEnabled`) in the manifest and `RunLifecycleTransitionSmokeGate.ps1` expects `StressCount` only when enabled, `0` while shelved. Gate rerun pending the next staged readiness pass.

## TestRoom Default Hero 2 Active-Ragdoll Init Lacks Physics Asset

- Severity tag: [Major]
- What's wrong: A staged standalone TestRoom smoke on 2026-06-07 logged `LogT66HeroPhysics: Warning: Init Failed Hero=BP_HeroBase_C_2147482135 Reason=MissingPhysicsAsset Mesh=SK_Hero_2_Chad`. The side-room trap placement proof still verified trap spawning and the stationary center arm, but default TestRoom Hero 2 cannot prove active-ragdoll trap reactions until its mesh has the expected PhysicsAsset or TestRoom forces a hero with one.
- Why it's out of scope now: The current pass only places the four obstacle trap actors into TestRoom side rooms and freezes the existing center wipeout arm. Changing hero selection, skeletal mesh assets, or PhysicsAsset assignment would broaden this into hero-physics content ownership.
- What fixing it would entail: Decide whether TestRoom should force Hero 1 for trap/active-ragdoll proof or give `SK_Hero_2_Chad` a valid PhysicsAsset, then rerun a staged TestRoom obstacle-contact proof that shows `UT66HeroPhysicsComponent` initializes and receives trap reactions.

## Hero Active-Ragdoll Proof Cameras Need Better Framing

- Severity tag: [Minor]
- What's wrong: The current `heroactiveragdollproof` and `heromovementqa` capture routes provide useful engine logs for the Stage 3 authority model, but their camera framing is not yet strong subjective-feel evidence. The TestRoom wipeout-arm proof camera can be occluded by the arm or let the shoved hero exit the best view, while MovementQA can show the hero late or partially out of frame.
- Why it's out of scope now: The Stage 3 authority rebuild needed to establish runtime ownership, active reaction routing, and no runtime body-teleport/resync loop. Rebuilding the capture camera into a following/observer rig is a proof-harness pass, not the core runtime authority fix.
- What fixing it would entail: Add a dedicated active-ragdoll proof camera that tracks both the wipeout arm and the hero capsule/mesh through contact and recovery, add a movement-only active-ragdoll framing path that applies any requested automation HP override before enemies can kill the test pawn, then rerun Unreal-owned MP4 captures until the frames clearly show wobble, impact, displacement, and recovery.

## Leap Migration Still Has Compatibility Aliases And Dash Audio

- Severity tag: [Minor]
- What's wrong: Stage 2 replaced the primary player movement ability with Leap and the active input/data row now uses `Leap`, but C++ compatibility wrappers still keep `RollForward`, `TryRollForward`, and `TryDashInWorldDirection` routed to Leap. Leap also still plays the existing `Hero.Movement.Dash` audio event because `Content/Data/AudioEvents.json` has no `Hero.Movement.Leap` event yet.
- Why it's out of scope now: The current pass intentionally preserved legacy wrappers while moving Hero 1 Chad to the new physics-first rig/animation foundation and forward-up Leap movement. Removing wrappers or renaming audio/data assets would broaden this Stage 2 implementation into a compatibility cleanup and audio authoring pass.
- What fixing it would entail: Add/author a `Hero.Movement.Leap` audio event, update the movement component to call it, audit Blueprint/input/UI/data references for any remaining primary Roll/Dash ability naming, remove the deprecated wrappers once no callers remain, compile, reload affected DataTables if needed, and rerun HeroMovementQA plus staged standalone verification.

## Superseded: TestRoom Skeletal Chad Pure Chaos Ragdoll Was The Previous Hero Direction

- Severity tag: [Superseded - Major]
- Resolution: The 2026-06-06 pure-Chaos/PAC-off policy superseded the earlier PAC-stabilization experiment for the legacy `UT66KnockbackComponent` path. As of the 2026-06-07 Stage 3 authority rebuild, Hero 1 active ragdoll is no longer defined by that legacy path: `UT66HeroPhysicsComponent` is the current Hero 1 target, with capsule gameplay authority, simulated pelvis/body chain, hip-anchor constraint, child-body PAC muscle drive, and bounded capsule shove. The old pure-Chaos result remains fallback/historical evidence only.
- Evidence: Focused `T66Editor Win64 Development` build passed after the hero PAC disable policy edit. The earlier PAC proof at `Saved/AgentReviews/FriendSlopPACStabilization/testragdoll_friendslop_pac_hero1_observer_delay05.mp4` remains historical evidence only, not the active hero direction.
- Follow-up: Subjective feel remains intentionally untuned. The live knobs to tune with design review are active-ragdoll drive strengths, hip-anchor stiffness/limits, capsule shove fraction/cap, launch XY/Z, knockdown/recovery timing, wipeout-arm speed/height/length, and proof/gameplay camera framing. Do not tune the old `UT66KnockbackComponent` PAC experiment as the final Hero 1 feel unless the active-ragdoll direction is explicitly reverted.
- 2026-06-06 supersede note: The user explicitly reopened hero physics architecture and approved moving toward a broad `Gameplay/Physics` ownership layer with Hero 1 Chad as the always-on active-ragdoll MVP. The pure-Chaos/PAC-off setup remains historical/prototype scaffolding and fallback evidence only; it is no longer the standing hero feel direction. Do not tune this old path as the final target unless a future task explicitly reverts the active-ragdoll direction.

## Resolved: TestRoom Wipeout-Arm Ragdoll Capture Does Not Visually Frame The Actor Action

- Severity tag: [Resolved - Minor]
- Resolution: The `testragdoll`/`testragdollproof` automation mode now spawns an inside-the-room proof camera below the TestRoom ceiling, hides the gameplay HUD, and frames the wipeout-arm lane from the live pawn location. The 2026-06-06 proof capture at `Saved/AgentReviews/FriendSlopUnrealRagdollImport/testragdoll_friendslop_passive_final.mp4` visibly shows the FriendSlop skeletal Chad before impact, the yellow wipeout arm contact, the ragdoll pose on the floor, and later recovery/control restoration.
- Follow-up: None for capture framing. Future PAC proof still needs its own accepted capture after PAC tuning is stable.

## Resolved: Content Corrections Smoke Safe Zone Bubble Visual Check Fails

- Severity tag: [Resolved - Major]
- What's wrong: A staged standalone `-T66GameplayAutoCapture=contentcorrectionssmoke` run on 2026-06-05 exited after logging `[ContentCorrectionsSmokeSummary] Terminal=1 Pass=0 Failures=1 FailedChecks=SafeZoneVisualBubblePresent`. The specific failing check was `SafeZoneVisualBubblePresent` with `Components=0 Visuals=0`; the rest of the content-corrections smoke checks logged PASS. The run also emitted the existing deliberate missing-visual fallback errors for `Smoke_MissingVisualBoss` / `Boss`, but the summary failure was only the Safe Zone bubble check.
- Why it's out of scope now: The current pass is scoped to FriendSlop raw Pixal3D model imports, texture preservation, and runtime visual references. Fixing Safe Zone bubble authoring or smoke expectations would touch unrelated gameplay visual code.
- What fixing it would entail: Inspect the Safe Zone actor/component setup used by `RunContentCorrectionsSmoke`, decide whether the bubble component should be spawned or the smoke expectation updated, then rerun the staged content-corrections smoke until `SafeZoneVisualBubblePresent` passes.
- Resolution: Resolved 2026-06-09: root cause was `/Engine/BasicShapes` never being cooked (`FT66VisualUtil::GetBasicShapeSphere` loads it by string at runtime), so `SafeZoneVisualComponent` had a null mesh in staged builds. Added `+DirectoriesToAlwaysCook=(Path="/Engine/BasicShapes")` to `DefaultGame.ini`; also made the `MobLootHasVisibleGroundMarker` check shelve-aware (passes-with-note while mob loot is shelved). Staged content-corrections smoke rerun pending.

## Resolved: Vendor Boss Has No CharacterVisuals Row

- Severity tag: [Resolved - Minor]
- What's wrong: The staged vendor/shop corrections proof spawned and defeated `AT66VendorBoss`, but the runtime log emitted `LogT66CharacterVisuals: Error: [MESH] ResolveVisual: NO ROW for VisualID='VendorBoss' in DT_CharacterVisuals`. The proof still passed because boss identity, defeat, token drop, and vendor-token stack math were valid, but the boss visual lookup is unresolved.
- Why it's out of scope now: The approved pass explicitly leaves CharacterVisuals consolidation/data alone and only changes vendor-token, steal-trigger, Daily Descent naming, and retired side-mode teardown surfaces.
- What fixing it would entail: Decide whether `VendorBoss` should receive a dedicated `CharacterVisuals.csv` row or move into a future per-entity visual table, author/import the visual row through the owning data-table pipeline, then rerun a staged vendor-boss smoke to confirm the visual error is gone.
- Resolution: Resolved 2026-06-09: the `VendorBoss` row already existed in `CharacterVisuals.csv` (line ~40, Patriot skeletal mesh) — the `DT_CharacterVisuals` uasset was stale. Reimported via `SetupCharacterVisualsDataTable.py` (editor-closed commandlet, log: script executed successfully, DT re-saved 21:18). Staged vendor-boss smoke re-check pending the next staged build.

## Resolved: Hero 1 Axe AOE VFX Lab Uses Deprecated Niagara Emitter Readiness API

- Severity tag: [Resolved - Minor]
- What's wrong: Focused UI build and standalone stage for the LootWheel cleanup emitted `C4996` from `Source/T66/Gameplay/T66Hero1AxeAOEVFXLabActor.cpp` because `FNiagaraEmitterInstance::IsReadyToRun` is deprecated.
- Why it's out of scope now: The current pass only removes LootWheel UI marks and verifies the staged LootWheel capture. Updating Niagara lab readiness handling would touch unrelated Gameplay/VFX code.
- What fixing it would entail: Audit the lab actor's emitter readiness check against the UE 5.7 Niagara execution-path API, replace the deprecated call, then run a focused Gameplay/VFX compile and any existing Hero 1 axe lab capture checks.
- Resolution: Resolved 2026-06-09 (pending-cleanup program): replaced the deprecated `FNiagaraEmitterInstance::IsReadyToRun` call at `T66Hero1AxeAOEVFXLabActor.cpp:716` (and the same call in `T66Editor/T66NiagaraIsolationCaptureCommandlet.cpp:433`) with the versioned emitter data's `IsReadyToRun`. Verified: `T66Editor` Development compile clean with zero `C4996`.

## Resolved: TrySpawnBoundWeaponBaseSlashVFX ImpactAnchored Bounce Branch Is Now Unreached

- Severity tag: [Resolved - Minor]
- What's wrong: `Source/T66/Gameplay/T66CombatComponent.cpp` `TrySpawnBoundWeaponBaseSlashVFX` still contains the `bImpactAnchoredCarrier` (`AttackCategory == ET66AttackCategory::Bounce`) branch that placed a static slash at each chain impact point. The Hero 1 Bounce projectile-travel fix replaced Bounce presentation with `StageBounceProjectileChain`/`SpawnBounceLinkProjectile`, so `PerformBounce` no longer calls `TrySpawnBoundWeaponBaseSlashVFX`. LineTarget and Slash are the only remaining callers, so the ImpactAnchored Bounce branch is currently dead.
- Why it's out of scope now: The approved task is the Bounce moving-projectile behavior/proof; removing the branch would touch interdependent locals shared with the former lane PathAnchored and radial AOE/Slash carriers in the same function and broaden the diff beyond the corrective fix.
- What fixing it would entail: Decide whether Bounce will ever reuse a bound Niagara impact slash as a static accent. The Hero1BounceProjectileTravelFix revision now renders the authored `NS_Hero1AxeBounce_MeshSlash` as the *moving* carrier silhouette (resolved via `ResolveCombatVFXBinding` in `PerformBounce`, attached through `AT66HeroProjectile::SetPrimaryCarrierNiagara`), so the `bImpactAnchoredCarrier` static-slash branch is still unreached on the Bounce path. If a static impact accent is never wanted, remove the `bImpactAnchoredCarrier` branch and its references (`SpawnLocation`, `VisualScaleVec`, `VisualAnchorModel`), then run a focused compile and the combat VFX binding validator. The `Hero1Axe_Bounce_Base` binding row remains authored.
- Resolution: Resolved 2026-06-09: removed the unreached `bImpactAnchoredCarrier` branch and simplified the `SpawnLocation` select (only the LineTarget flow ~line 2947 and Slash flow ~line 3234 call this function; `PerformBounce` drives link visuals through `SpawnBounceLinkProjectile`). Verified by focused compile.

## Legacy MainMapTerrain Dirt Grass And Rock Names Are Dormant But Confusing

- Severity tag: [Minor]
- What's wrong: `Source/T66/Gameplay/T66MainMapTerrain.cpp` still contains legacy names and helper paths for dirt, grass, rock decoration, and farm decor/supports, even though `bRenderFarmGrass`, `bRenderFarmDecor`, and `bRenderFarmSupports` are false and the loaded grass/rock meshes are null. These names can mislead future cleanup work into thinking there are active map clutter assets when the current runtime path is disabled/dormant.
- Why it's out of scope now: The current cleanup removes the live placed rock actors/assets and disables the active tower runtime rock/deco spawner. Renaming or deleting dormant terrain internals is a separate legacy-code cleanup with broader regression risk.
- What fixing it would entail: Audit `T66MainMapTerrain.cpp` for dormant decor/support code, decide whether to remove the unused paths or rename them to neutral terrain-surface terminology, compile, and verify classic/tower terrain generation still builds correctly.

## Resolved: Ranged Autocapture Acceptance Closed On Lightweight-Only Baseline

- Severity tag: [Resolved - Blocker]
- What's wrong: Pass B.10 moved Ranged-family mobs onto `AT66MobBase` and proved the projectile path in smoke, but the CVar-on `enemywaveperf` acceptance set halted after two non-zero exits. Both non-zero exits were real hero deaths from lightweight `EnemyProjectile` hits, not shutdown crashes. Example fatal lines show `SourceID=StoneSentinel Delivery=EnemyProjectile SourceActor=T66MobBase_* HeroHP=20.0->0.0` at world times `17.58` and `28.56`. A clean CVar-on attempt also had to be rejected because `PerformanceSystemOverheadMaxUs=268567.0`.
- B.10.1 update: route diagnostics landed and the partial same-binary comparison points at a lightweight Ranged parity gap. RA (`UseLightweight=1`, Rush/Flying lightweight, Ranged rich) produced 6/6 route-valid survivals, 0 projectile hits, and 0 projectile damage before the set halted on two PerformanceSystem overhead rejections. A post-patch full-lightweight attribution rerun produced 5/5 route-valid captures, 21 projectile hits, 420 HP of projectile damage, and 3/5 hero deaths. A single post-patch RB preflight also showed full-lightweight Ranged landing a projectile hit while the hero survived at 80 HP. This is not enough for B.10 acceptance, but it strongly favors "lightweight Ranged is more aggressive than rich Ranged in practice" over a pure measurement-contract issue.
- B.10.1B update: after the PerformanceSystem write-queue fix, the clean 10x10 same-binary diagnostic showed a route-dependent split in observed Ranged outcomes without overhead rejects. RA completed 10/10 route-valid captures with 0 hero deaths, 0 projectile hits, and 0 projectile damage. RB completed 10/10 route-valid captures with 4 hero deaths, 31 projectile hits, and 620 projectile damage. B.10 remains blocked on Ranged parity rather than a measurement-contract change in this pass; the next reviewed packet needs to explain RA's zero projectile activity and make lightweight Ranged match the rich path's effective pressure.
- B.10.1C-freeze update: the reviewed short freeze matrix showed that the explicit `T66.AutoCapture.HeroHPOverride=500` path is not the freeze source and is needed for ranged-active diagnostics. No-override full-lightweight runs can still kill the stationary hero within 30s (`5` projectile hits / `100` HP damage at `WorldTime=24.68` in Config1 attempt 1; `5` hits / `100` HP damage at `WorldTime=26.84` in Config3 attempt 1). HP override runs applied `500` HP cleanly and did not mutate staged saves. Ranged parity remains unresolved; this update only confirms the measurement-survivability behavior.
- B.10.1C-Rerun update: aggregate-counter diagnostics identified the rich-Ranged zero-hit root cause. RA-D completed 10/10 accepted, route-valid, HP500 captures on staged hash `6212558B842942338D3071098D094703542CB473C9DAADDCF5B96C49FFC71ACA`. Rich Ranged reached the fire path (`RichLOSPassed=221`, `RichProjectilesDispatched=221`) but every dispatch failed to spawn (`RichProjectilesSpawned=0`, `RichSpawnFailed=221`), producing 0 hero hits. RB-D on the same binary reached lightweight dispatch 183 times, spawned 130 projectiles, and hit the hero 73 times. This makes the immediate B.10.1D target rich `AT66RangedEnemy` projectile spawn failure, not lightweight pressure reduction.
- B.10.1D update: the rich `SpawnActor` failure is resolved by architecture replacement, not by patching the old actor spawn path. `UT66ProjectileManagerSubsystem` now owns enemy projectiles as flat-array data and renders them through HISM. Focused staged smoke proved rich and lightweight Ranged both fire through the manager, render projectiles, hit the hero, and preserve `SourceID=HexSlinger`/`Delivery=EnemyProjectile` attribution (`T66_B101D_RichRangedSmoke_Isolated_Final.log` and `T66_B101D_LightweightRangedSmoke_Isolated_Final.log`). However, acceptance remains blocked. The final clean CVar-off rerun on hash `CF70BC921BA930115837775F96F2277040A3A78D09BC6C44FBBB6A730D62F20B` halted after two rejects: run 1 killed the HP500 hero with 34 rich projectile hits / 680 projectile damage by `WorldTime=52.11`, and run 3 completed with `NoProjectilesFired` because rich LOS never passed (`RichLOSBlocked=3840`, `RichLOSPassed=0`). Manager capacity/rendering was healthy in the same rows (`DroppedFires=0`, `ManagerTickMaxUs<=709.6 us`, `HISMUpdateMaxUs<=630.0 us`), so this is now a measurement-contract / rich LOS stability blocker rather than a projectile manager implementation failure.
- B.10.1D resume update: the reviewed no-source-change HP2000 rerun halted immediately on the same rich LOS/no-fire pattern. The command line and route flags requested `T66AutoCaptureHeroHPOverride=2000`, but the run-state automation path applied only `1000.0` due a hard cap. The single CVar-off row completed with `NoProjectilesFired`, `RichDistancePassed=103`, `RichLOSBlocked=103`, `RichLOSPassed=0`, `ProjectileManagerFired=0`, and `PerfSystemOverheadMaxUs=689.0`. This proves HP alone cannot be the next step: the follow-up must both make the measurement HP override capable of applying the reviewed value and fix or at least attribute rich Ranged LOS starvation with bounded aggregate evidence.
- B.10.1D Resume2 update: bounded LOS blocker attribution now identifies the no-fire path. The reviewed CVar-off probe on staged hash `5FB9EECF1C2B25B79DDFC5CA07D221AA1C4BE986636FA732409AD011B7A13589` reproduced `NoProjectilesFired` in run 3 with `RichDistancePassed=815`, `RichLOSBlocked=815`, `RichLOSPassed=0`, and `ProjectileManagerFired=0`; all `815` blocked LOS checks were classified as `RichEnemy`. Accepted probe rows showed the same dominant blocker pattern (`391/403` and `28/28` rich LOS blocks caused by `RichEnemy`). The blocker is therefore peer rich enemy capsule self-occlusion on the visibility LOS trace, not PerformanceSystem overhead, HP cap behavior, projectile manager capacity, or SpawnActor/spawn failure.
- B.10.1D Resume3 update: the reviewed follow-up raised the automation-only HP cap to `2000`, made rich Ranged LOS ignore registered rich enemy actors, and made enemy projectiles pass through rich/lightweight peer bodies while preserving hero/world collision. Smoke passed for both rich and lightweight manager projectiles on staged hash `782F98AAA4D5269D450A94FBA7345E54B335C91F53BFDD61D324258569962C9D`. The mandatory CVar-off acceptance attempt no longer showed rich LOS starvation (`RichLOSBlocked=0`, `RichLOSBlockerRichEnemy=0` in all six rows), but it halted after two `HeroDeath` rejects before CVar-on could run. The rejected rows recorded `194` and `262` hero projectile hits under HP2000. PerformanceSystem overhead stayed under `936.6 us`, no Git/LFS overlap was recorded during acceptance rows, dropped fires stayed `0`, and binary hash stayed stable.
- B.10.1D Resume4 update: the reviewed measurement-contract pass raised the automation-only HP cap to `50000` and ran HP20000 smoke/acceptance. HP20000 survived both rich and lightweight saturated projectile pressure in smoke and all attempted acceptance rows; no `HeroDeath` occurred after the cap increase. The 10-capture escalation produced a clean CVar-off baseline candidate on staged hash `0A0AC836F224B898353CD7FA59B5A58ECC24D7676F6903DFD765CA9A3D9252EB` with 10/10 accepted rows, median `157.68 FPS`, 0 hero deaths, 2519 projectiles fired, 2506 hero hits, and max overhead `805.9 us`. Acceptance still halted in CVar-on after two `RouteValidity` rejects: the rejected rows had `UseLightweight=1`, `RouteRanged=1`, and `LightweightSpawns>0`, but also `RichSpawns=1` with nonzero rich fire attempts. This means the HP/hero-death measurement blocker is resolved, but CVar-on routing can still leak rich Ranged activity during lightweight acceptance.
- B.10.1D Resume5 update: the reviewed route-attribution diagnostic added aggregate `RouteAttributionSummary` counters and ran one CVar-off control plus 10 CVar-on route diagnostic captures on stable staged hash `D1E3235ED789C2596626BF6748F3DE49018B883D99F941B6160D860C535192FF`. The specific Ranged rich-route leak did not reproduce in those 10 CVar-on captures: `RangedRoutedLightweightBasic=259`, all Ranged rich reason buckets were `0`, `RichSpawns=0`, and rich fire attempts were `0`. The diagnostic did reproduce one planned rich mini-boss route, landing on Flying (`FlyingRoutedRichMiniBossPromotion=1`), and 11 planned special routes (`SpecialUnknownRoutedRichSpecialOrMiniBoss=11`). Source audit shows mini-boss promotion is family-neutral: `ShouldRouteSpawnToLightweightMob` returns false for `bIsMiniBoss`, and runtime waves choose the mini-boss slot before rolling the final `MobID`. If that promotion lands on a Ranged `MobID`, it explains the earlier `RichSpawns=1` CVar-on route-validity rejects without requiring a routing race, fallback branch, family lookup failure, lightweight acquire failure, or second spawn path.
- Placed miniboss update: the reviewed placed-encounter pass removed random per-wave miniboss promotion from `AT66EnemyDirector` and moved minibosses to deliberate tower descent-hole encounters. A fixed rich `Slime` miniboss now spawns on entering floors `2`, `3`, and `4` in normal tower stages and gates that floor's exit hole until defeated. This resolves the family-neutral random miniboss promotion source that could inject planned rich routes into basic-mob captures; B.10 acceptance still needs a follow-up capture pass to confirm the route contract and close this larger Ranged acceptance issue.
- Boss projectile manager update: boss projectiles now share `UT66ProjectileManagerSubsystem` with basic enemy projectiles through `FireBossProjectile(...)`. This unifies enemy-to-hero projectile storage/rendering/damage attribution for basic mobs and bosses, but it did not itself rerun or close the B.10 basic-mob acceptance blocker.
- B.11/B.12 lightweight-only closure update: the rich-vs-lightweight A/B contract is retired for basic mobs. Basic Melee/Rush/Flying/Ranged now route lightweight unconditionally, while minibosses, specials, guardians, and bosses remain intentionally rich. The rich basic-mob path is deprecated dead code pending cleanup, and its intermittent CMC/Ranged delivery is recorded as a known limitation of the retired path rather than a blocker.
- Resolution: Phase 1 lightweight baseline on staged SHA `86EDE7D6F2533614D9E0525305230BC06CD468F1547642A47C6A2B5C1613C9F5` accepted 3/3 captures with median `192.80 FPS`, max PerformanceSystem overhead `1011.4 us`, zero hero deaths, zero rejected rows, and basic Ranged routing/lightweight projectile hits present. Phase 3 after the VAT-state/workstream pass accepted 3/3 captures on staged SHA `BD1F3BFB6AE27000684F0980FB5EA4FB356D2B94C3941278F57CAA43826B6E33` with median `200.99 FPS`, max overhead `911.8 us`, zero hero deaths, zero rejected rows, and stable executable hash. B.10 is closed on the lightweight-only measurement contract.
- Follow-up: Cleanup pass should delete deprecated rich-basic routing branches and inert routing CVars after B.13/B.14 validates the final rendering path.

## Resolved: Ranged Diagnostic VeryVerbose Logging Can Stall Or Distort Autocapture

- Severity tag: [Resolved - Major]
- What's wrong: The pre-final B.10.1C diagnostic path enabled legacy `LogT66MobManager VeryVerbose` and emitted per-frame `CooldownBlocked` `[RangedFireDecision]` lines under `-forcelogflush`. Two diagnostic-on smoke logs produced `46415` and `65556` decision lines, mostly `CooldownBlocked`, and no terminal `RangedPressureSummary`, matching the frozen-window symptom. The current post-mitigation dedicated category no longer hard-freezes in the 30s matrix, but `LogT66RangedDiagnostics VeryVerbose` still slowed HP500 full-lightweight capture from `43.94s` wall for 30.04s world time to `53.82s` wall for 30.02s world time.
- Resolution: B.10.1C-Rerun removed per-frame/per-mob Ranged fire-decision log emission and replaced it with aggregate counters plus one terminal `[RangedDecisionSummary]` line per capture. RA-D and RB-D both completed 10/10 accepted captures with exactly one summary line per capture, zero legacy `RangedFireDecision` lines, zero `RangedPressureDiagnostic` lines, and max framework overhead under `1300 us`.
- Follow-up: If future diagnostics need example traces, add a separate reviewed sampling/throttle mechanism. Do not reintroduce unbounded cooldown-blocked logs under `-forcelogflush`.

## Resolved: Backrooms GameMode Handler Declarations Blocked Fresh Link

- Severity tag: [Resolved - Blocker]
- What's wrong: A fresh B.10 Development link exposed stale `AT66GameMode` Backrooms handler declarations that had no definitions, plus private `UPROPERTY` declarations for placeholder Backrooms classes that were not visible to UHT in the current source state. This was unrelated to lightweight Ranged, but it blocked the required clean build.
- Resolution: The private Backrooms actor storage in `AT66GameMode` now uses `TObjectPtr<AActor>` and the declared Backrooms handlers have defensive no-op definitions that log if invoked. This restores build/link integrity without implementing or enabling Backrooms gameplay.
- Follow-up: Backrooms gameplay remains unimplemented/disabled. A future Backrooms pass should replace the defensive no-ops with real door/chaser flow and concrete actor classes or remove the stale declarations entirely.

## Resolved: Trap Projectile Fire Logs Demoted

- Severity tag: [Resolved - Major]
- What's wrong: B.7 lightweight actor validation still surfaced routine `LogT66TrapProjectile` projectile-fire lines during gameplay captures. Projectile fire is a per-spawn/per-projectile hot path, and normal `Log`/`Display` output under `-forcelogflush` can materially distort performance captures, as seen with the lightweight mob routing logs in B.6.1.
- Resolution: The B.11/B.12 multi-workstream pass demoted routine trap arrow fire and impact telemetry in `Source/T66/Gameplay/Traps/T66TrapArrowProjectile.cpp` to `VeryVerbose` while preserving warnings/errors. Live editor build passed after integration.
- Follow-up: None for the routine trap-arrow fire/impact log issue. Broader trap projectile manager migration remains out of scope.

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

## Resolved: Direct-Entry Companion Spawn-Seam Fallback

- Severity tag: [Resolved - Minor]
- What's wrong: Confirm Companion writes the run selection to `UT66GameInstance::SelectedCompanionID`, but the gameplay spawn helper preferred `AT66SessionPlayerState::GetSelectedCompanionID()` whenever a session player state existed. In standalone/direct gameplay entry the fresh gameplay player state can still hold `None` at companion-spawn time, so `AT66GameMode::SpawnCompanionForPlayer` skipped the confirmed companion.
- Resolution: `T66GetSelectedCompanionID` now keeps a non-empty session companion authoritative, but lets the local controller fall back to the game-instance run selection when the fresh gameplay player state has not received its lobby profile yet. Remote controller `None` still stays `None` so a host/local game-instance selection is not reused for another player.
- Verification: Runtime target build passed, standalone staging refreshed `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe` with `Scripts\StageStandaloneBuild.ps1 -SkipBuild`, and both standalone shortcuts target that staged exe. End-to-end UI proof `C:\UE\T66\Saved\Codex\CompanionConfirmEnterSmoke\ManualUI_Companion_01_alpha08_trimmed3\ManualUI_Companion_01_alpha08_trimmed3.log` launched Companion Selection, clicked Confirm Companion, clicked Enter, logged the patched fallback with `GICompanion=Companion_01`, and logged `Spawned companion: Light-Skinned Black Rap Vixen`; `ManualUI_Companion_01_alpha08_trimmed3.png` was written by the Unreal viewport capture path. Positive staged direct-entry proof `C:\UE\T66\Saved\Codex\CompanionConfirmEnterSmoke\DirectEntry_Companion_01_alpha08_trimmed\DirectEntry_Companion_01_alpha08_trimmed.log` entered the tower with `-T66Companion=Companion_01`, logged the patched fallback with `GICompanion=Companion_01`, and logged `Spawned companion: Light-Skinned Black Rap Vixen`. Negative staged direct-entry proof `C:\UE\T66\Saved\Codex\CompanionConfirmEnterSmoke\DirectEntry_None_alpha08_trimmed\DirectEntry_None_alpha08_trimmed.log` entered with `-T66Companion=None`, logged the patched fallback with `GICompanion=None`, and did not log any `Spawned companion:` line.
- Follow-up: The UI proof used deterministic client-coordinate clicks against the staged window after bringing it topmost; remote multiplayer fallback behavior was not smoke-tested.

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

## Resolved: Player Experience Tuning Can Be Requested Before DataTable Is Available

- Severity tag: [Resolved - Minor]
- What's wrong: The Easy mob VAT staged gameplay smoke logged `PlayerExperience tuning requested by GetDifficultyStartStage before DataTable '/Game/Data/DT_PlayerExperience.DT_PlayerExperience' was available; returning empty tuning.`
- Why it's out of scope now: The VAT pass only changed enemy visual animation data and did not change the player-experience subsystem initialization order.
- What fixing it would entail: Audit the player-experience subsystem load sequence, make the tuning table available before gameplay difficulty startup queries, and add a staged smoke assertion that this warning no longer appears.
- Resolution: Resolved 2026-06-09: replaced the async preload with a synchronous `TryLoad` in `UT66PlayerExperienceSubSystem::Initialize` (five-row table; the async load raced gameplay difficulty queries). Removed the now-dead async handler/member. Staged smoke assertion that the warning no longer appears rides the next staged run.

## Inter-Walkable-Box Floor Seams Remain Possible

- Severity tag: [Major]
- What's wrong: `Source/T66/Gameplay/T66TowerMapTerrain.cpp` now emits one generated floor rectangle per walkable source box, which removes internal subdivision seams, but seams can still appear where separate walkable boxes or drop-hole carve-out rectangles meet.
- Why it's out of scope now: Eliminating those joins requires changing floor-box generation/merging semantics beyond the requested visual assembly fix.
- What fixing it would entail: Merge each mob floor's compatible walkable rectangles into a single visual surface, or author a dedicated runtime mesh for the floor footprint so there are no visual boundaries between adjacent boxes.

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

## Resolved: NPC Class Names Still Used HouseNPC

- Severity tag: [Resolved - Minor]
- Resolution: The C++ base actor and data struct were renamed to `AT66NPCBase` and `FT66NPCData`; registry, gameplay, HUD, and NPC subclass references now use the neutral symbols, with CoreRedirects preserving old class/struct references.
- Follow-up: None for the C++ symbol cleanup.

## Vehicle Class Names Still Use Tractor

- Severity tag: [Minor]
- What's wrong: The authored data row, prompts, damage source token, and interaction row lookup now use `Vehicle`, but the inherited C++ implementation still uses `AT66PilotableTractor`, `T66PilotableTractor.*`, and enum/action names such as `PilotTractor`, subobject names such as `TractorRoot`, and local variable names such as `ClosestTractor` in player interaction code.
- Why it's out of scope now: The current pass is scoped to data/schema/loader cleanup and avoids broad class/file renames that would touch generated headers, includes, actor references, and Blueprint bindings.
- What fixing it would entail: Rename the C++ class/files to a neutral vehicle name, update includes and player-controller interaction variables, add Unreal redirects if required, compile, and verify existing spawned/interactable vehicle references still resolve.

## Enemies Ignore The Rising Lava Hazard

- Severity tag: [Major]
- What's wrong: The 2026-06-10 bounce-course pass added the rising-lava mode to `Source/T66/Gameplay/T66MiasmaManager.cpp`, but lava damage is hero-only (matching the legacy miasma contract). Grounded enemies path through fully flooded floors unharmed and visually wade through the lava sheet, which undercuts the hazard fantasy and lets melee pressure continue from inside lava.
- Why it's out of scope now: The bounce-course pass scoped hazard damage to the existing hero-only miasma damage contract; enemy-vs-hazard rules touch enemy balance, the lightweight mob manager, and AI routing ownership.
- What fixing it would entail: Decide the enemy rule (damage, slow, despawn, or platform-aware routing), implement it for both rich enemies and `UT66MobManagerSubsystem` lightweight mobs, and rerun staged tower gameplay smoke to confirm enemy pressure on flooded floors stays fair.

## Bounce Ramp Visuals Are Plain Rotated Cubes

- Severity tag: [Minor]
- What's wrong: `T66SpawnBounceCourseForFloor` in `Source/T66/Gameplay/T66TowerMapTerrain.cpp` renders bounce platforms with baffle-tube decks and inflated side skirts, but ramps use a rotated themed cube slab because the baffle tube placement helpers are axis-aligned.
- Why it's out of scope now: The bounce-course pass prioritized the traversal guarantee and platform/arch visuals; sloped baffle layout needs its own transform math and seam handling.
- What fixing it would entail: Add a sloped baffle-tube layout (tubes running across the ramp, stepped along the slope), apply it in the ramp branch with the cube slab as fallback, and verify the read in a staged capture.

## Resolved: TutorialGate Class Remains After Tutorial Exit Uses StageGate

- Severity tag: [Resolved - Minor]
- What's wrong: `Source/T66/Gameplay/T66TutorialGate.*` and the `AT66TutorialGate` interaction path in `Source/T66/Gameplay/T66PlayerController_Combat.cpp` still exist, but `Source/T66/Gameplay/T66TutorialManager.cpp` now spawns `AT66StageGate` for the tutorial exit.
- Why it's out of scope now: This pass was intentionally constrained to swapping the tutorial end to the regular Stage Gate without broad class/file deletion or reference cleanup.
- What fixing it would entail: Prove there are no Blueprint, asset, map, or automation references to `AT66TutorialGate`, then remove the class, includes, player-controller branch, and any stale generated/API references with a focused compile and content reference check.
- Resolution: Resolved 2026-06-09: proven zero Blueprint/map/asset references (full Content binary scan) and zero spawn sites; removed `T66TutorialGate.h`/`.cpp`, the 7 player-controller includes, and the dead closest-gate interact branch in `T66PlayerController_Combat.cpp`. Verified by full `T66Editor` compile. (TestRoom's `TutorialGate_Pixal3D` display-mesh row and TutorialManager's tag-name strings are unrelated to the class and remain.)

