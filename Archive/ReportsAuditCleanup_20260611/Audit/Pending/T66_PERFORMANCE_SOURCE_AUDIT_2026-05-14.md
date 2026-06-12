# T66 performance-oriented source audit (2026-05-14)

**Location:** `Audit/Pending/T66_PERFORMANCE_SOURCE_AUDIT_2026-05-14.md`
**Scope:** Every `*.cpp` / `*.h` under `Source/T66` (524 translation units).
**Repo edits:** This pass only adds this audit document; no game source or `Config/` changes.

## Executive honesty

1. **File-by-file coverage:** **Appendix A** (keyword presence per file) and **Appendix C** (numeric counts per file after a full read of every file’s text). Both enumerate the same **524** paths under `Source/T66`.
2. This is **not** a human semantic review of every line in every file; runtime cost requires Unreal Insights / `stat` on your scenarios.
3. **Flags are lexical regex hits** and can false-positive (e.g. `PrimaryActorTick.bCanEverTick = false` still matches `TICK_ACTOR`). Use as triage, not automatic guilt.
4. **Prior audit markdown under `Audit/` was not used as source material** for this document.
5. **Out of scope:** Blueprint-only logic, Niagara systems, maps, most `Content/`, most `Config/` (renderer scalability not re-audited here).

## Methodology

| Flag | Intent |
|------|--------|
| `TICK_ACTOR` | Actor/widget tick hooks (`PrimaryActorTick`, `AT66*::Tick`, `UT66*::NativeTick`) |
| `PLAYER_TICK` | `PlayerTick` override |
| `TRACE` | Physics / scene queries |
| `SYNC_LOAD` | `LoadObject` / `LoadSynchronous` / `StaticLoadObject` |
| `ITERATOR` | `TActorIterator` / `TObjectIterator` |
| `HTTP` | HTTP client usage |
| `FILE_IO` | `FFileHelper` / `IFileManager` |
| `NIAGARA` | Niagara types / spawn helpers |
| `NEW_OBJECT` | Runtime `NewObject<` |
| `DYNAMIC_MAT` | Dynamic material instance creation |
| `GET_ALL` | `GetAllActorsOfClass` style scans |
| `DEBUG_DRAW` | DrawDebug helpers |
| `SLATE_INV` | Slate invalidation APIs |
| `RENDER_TARGET` | Render target readback / export |
| `WEBVIEW` | WebView2 integration |
| `SET_TIMER_LOOP` | `SetTimer(..., true)` looping timers |

### Aggregate: files with at least one hit per flag

- `TICK_ACTOR`: **51** files
- `NIAGARA`: **37** files
- `SYNC_LOAD`: **31** files
- `TRACE`: **30** files
- `ITERATOR`: **24** files
- `NEW_OBJECT`: **24** files
- `DYNAMIC_MAT`: **21** files
- `FILE_IO`: **18** files
- `HTTP`: **12** files
- `WEBVIEW`: **6** files
- `PLAYER_TICK`: **2** files
- `SLATE_INV`: **1** files
- `SET_TIMER_LOOP`: **1** files
- `GET_ALL`: **0** files
- `DEBUG_DRAW`: **0** files
- `RENDER_TARGET`: **0** files

**Zero hits in this module for:** `GET_ALL`, `DEBUG_DRAW`, `RENDER_TARGET` (for these exact string patterns).

## Files with four or more flags (triage shortlist)

- **5** `Source/T66/Gameplay/T66PlayerController.cpp` — TRACE, SYNC_LOAD, ITERATOR, NIAGARA, DYNAMIC_MAT
- **5** `Source/T66/Gameplay/T66CombatComponent.cpp` — TRACE, SYNC_LOAD, ITERATOR, NIAGARA, NEW_OBJECT
- **5** `Source/T66/Gameplay/GameMode/T66GameMode_BossFlow.cpp` — TRACE, SYNC_LOAD, ITERATOR, NEW_OBJECT, DYNAMIC_MAT
- **4** `Source/T66/Gameplay/T66VisualUtil.cpp` — TRACE, SYNC_LOAD, ITERATOR, DYNAMIC_MAT
- **4** `Source/T66/Gameplay/T66PlayerController_Combat.cpp` — PLAYER_TICK, TRACE, NIAGARA, NEW_OBJECT
- **4** `Source/T66/Gameplay/T66MiasmaManager.cpp` — TICK_ACTOR, TRACE, SYNC_LOAD, DYNAMIC_MAT
- **4** `Source/T66/Gameplay/T66LavaPatch.cpp` — TICK_ACTOR, TRACE, SYNC_LOAD, DYNAMIC_MAT
- **4** `Source/T66/Gameplay/T66HouseNPCBase.cpp` — TICK_ACTOR, TRACE, ITERATOR, DYNAMIC_MAT
- **4** `Source/T66/Gameplay/T66HeroBase.cpp` — TICK_ACTOR, TRACE, SYNC_LOAD, DYNAMIC_MAT
- **4** `Source/T66/Gameplay/T66GameMode.cpp` — TICK_ACTOR, TRACE, SYNC_LOAD, ITERATOR
- **4** `Source/T66/Gameplay/T66ArcadeMachineInteractable.cpp` — TRACE, SYNC_LOAD, NEW_OBJECT, DYNAMIC_MAT

### Short manual notes (interpretation layer)

- **`T66CombatComponent.cpp`**: Large combat surface; traces + Niagara + soft sync resolves — primary gameplay CPU/GPU driver; profile locked-target and AoE paths.
- **`T66PlayerController.cpp` / `_Combat.cpp`**: `PlayerTick`, camera traces, wall occlusion materials — verify per-frame vs throttled work in Insights.
- **`T66GameMode*.cpp` / `T66GameMode_WorldInteractables.cpp`**: iterators + traces often in **spawn / layout / interactable setup**; confirm not in steady combat tick.
- **`T66TowerMapTerrain.cpp` / `T66MainMapTerrain.cpp`**: Very high line counts + `NewObject` component trees — build-time / streaming hitch risk.
- **`T66CharacterVisualSubsystem.cpp`**: sync material / mesh resolution — first-spawn hitch risk.
- **`T66RetroFXSubsystem.cpp`**: full-world iterator when retro materials refresh — acceptable if rare; dangerous if triggered frequently.
- **`T66MiasmaManager.cpp` / `T66LavaPatch.cpp`**: tick + materials + traces — scales with active hazard coverage.
- **`T66HouseNPCBase.cpp`**: throttled tick exists in code elsewhere; still worth validating NPC count × trace budget.

## Recommended follow-up (outside this document)

1. Unreal Insights capture: **tower combat** + **main menu** + **run summary**.
2. Correlate hot frames with: Niagara spawn storms, sync loads, Slate rebuilds.
3. Re-run a similar automated manifest pass after major refactors (same regex table in **Methodology**).

## Appendix A — complete file-by-file manifest

- `Source/T66/Core/Backend/T66BackendAccountApi.cpp` (473 lines): **HTTP**
- `Source/T66/Core/Backend/T66BackendDailyClimbJson.cpp` (130 lines): **—**
- `Source/T66/Core/Backend/T66BackendDailyClimbJson.h` (14 lines): **—**
- `Source/T66/Core/Backend/T66BackendLeaderboardApi.cpp` (806 lines): **HTTP**
- `Source/T66/Core/Backend/T66BackendPartyApi.cpp` (527 lines): **HTTP, FILE_IO**
- `Source/T66/Core/Backend/T66BackendPrivate.h` (126 lines): **HTTP**
- `Source/T66/Core/Backend/T66BackendRunApi.cpp` (627 lines): **HTTP**
- `Source/T66/Core/Backend/T66BackendRunSerializer.cpp` (573 lines): **—**
- `Source/T66/Core/Backend/T66BackendRunSerializer.h` (26 lines): **—**
- `Source/T66/Core/Backend/T66BackendRunSummaryParser.cpp` (289 lines): **NEW_OBJECT**
- `Source/T66/Core/Backend/T66BackendRunSummaryParser.h` (15 lines): **—**
- `Source/T66/Core/Backend/T66BackendSubsystem.cpp` (434 lines): **HTTP, NEW_OBJECT**
- `Source/T66/Core/PlayerExperience/T66PlayerExperienceSubSystem_Scoring.cpp` (30 lines): **—**
- `Source/T66/Core/PlayerExperience/T66PlayerExperienceSubSystem_Spawning.cpp` (44 lines): **—**
- `Source/T66/Core/PlayerExperience/T66PlayerExperienceSubSystem_Totems.cpp` (43 lines): **—**
- `Source/T66/Core/PlayerExperience/T66PlayerExperienceTypes.h` (157 lines): **—**
- `Source/T66/Core/RunState/T66RunStateSubsystem.cpp` (697 lines): **—**
- `Source/T66/Core/RunState/T66RunStateSubsystem_AntiCheat.cpp` (566 lines): **—**
- `Source/T66/Core/RunState/T66RunStateSubsystem_Combat.cpp` (916 lines): **—**
- `Source/T66/Core/RunState/T66RunStateSubsystem_EconomyInventory.cpp` (1003 lines): **—**
- `Source/T66/Core/RunState/T66RunStateSubsystem_Idols.cpp` (279 lines): **—**
- `Source/T66/Core/RunState/T66RunStateSubsystem_Private.h` (508 lines): **NIAGARA**
- `Source/T66/Core/RunState/T66RunStateSubsystem_ScoreTelemetry.cpp` (121 lines): **—**
- `Source/T66/Core/RunState/T66RunStateSubsystem_Snapshot.cpp` (322 lines): **—**
- `Source/T66/Core/RunState/T66RunStateSubsystem_Stats.cpp` (909 lines): **—**
- `Source/T66/Core/RunState/T66RunStateSubsystem_TimersBoss.cpp` (526 lines): **—**
- `Source/T66/Core/T66AchievementsSubsystem.cpp` (1643 lines): **NEW_OBJECT**
- `Source/T66/Core/T66AchievementsSubsystem.h` (400 lines): **—**
- `Source/T66/Core/T66ActorRegistrySubsystem.cpp` (154 lines): **—**
- `Source/T66/Core/T66ActorRegistrySubsystem.h` (72 lines): **ITERATOR**
- `Source/T66/Core/T66AudioSubsystem.cpp` (567 lines): **—**
- `Source/T66/Core/T66AudioSubsystem.h` (88 lines): **—**
- `Source/T66/Core/T66AudioTypes.h` (63 lines): **—**
- `Source/T66/Core/T66BackendSubsystem.h` (616 lines): **HTTP**
- `Source/T66/Core/T66BuffSaveGame.h` (126 lines): **—**
- `Source/T66/Core/T66BuffSubsystem.cpp` (1312 lines): **NEW_OBJECT**
- `Source/T66/Core/T66BuffSubsystem.h` (219 lines): **—**
- `Source/T66/Core/T66CharacterVisualSubsystem.cpp` (1628 lines): **SYNC_LOAD, DYNAMIC_MAT**
- `Source/T66/Core/T66CharacterVisualSubsystem.h` (137 lines): **—**
- `Source/T66/Core/T66CommunityContentSaveGame.h` (25 lines): **—**
- `Source/T66/Core/T66CommunityContentSubsystem.cpp` (1206 lines): **HTTP, NEW_OBJECT**
- `Source/T66/Core/T66CommunityContentSubsystem.h` (96 lines): **HTTP**
- `Source/T66/Core/T66CommunityContentTypes.h` (264 lines): **—**
- `Source/T66/Core/T66CompanionUnlockSaveGame.h` (28 lines): **—**
- `Source/T66/Core/T66CompanionUnlockSubsystem.cpp` (104 lines): **—**
- `Source/T66/Core/T66CompanionUnlockSubsystem.h` (50 lines): **—**
- `Source/T66/Core/T66CsvUtil.h` (203 lines): **FILE_IO**
- `Source/T66/Core/T66DailyClimbTypes.h` (121 lines): **—**
- `Source/T66/Core/T66DamageLogSubsystem.cpp` (125 lines): **—**
- `Source/T66/Core/T66DamageLogSubsystem.h` (74 lines): **—**
- `Source/T66/Core/T66EnemyPoolSubsystem.cpp` (84 lines): **—**
- `Source/T66/Core/T66EnemyPoolSubsystem.h` (60 lines): **—**
- `Source/T66/Core/T66FloatingCombatTextPoolSubsystem.cpp` (249 lines): **—**
- `Source/T66/Core/T66FloatingCombatTextPoolSubsystem.h` (61 lines): **—**
- `Source/T66/Core/T66FloatingCombatTextSubsystem.cpp` (73 lines): **—**
- `Source/T66/Core/T66FloatingCombatTextSubsystem.h` (68 lines): **—**
- `Source/T66/Core/T66GameContentSettings.cpp` (4 lines): **—**
- `Source/T66/Core/T66GameContentSettings.h` (42 lines): **—**
- `Source/T66/Core/T66GameInstance.cpp` (1840 lines): **—**
- `Source/T66/Core/T66GameInstance.h` (737 lines): **—**
- `Source/T66/Core/T66GameplayLayout.h` (146 lines): **—**
- `Source/T66/Core/T66HeroSpeedSubsystem.cpp` (22 lines): **—**
- `Source/T66/Core/T66HeroSpeedSubsystem.h` (51 lines): **—**
- `Source/T66/Core/T66IdolManagerSubsystem.cpp` (531 lines): **—**
- `Source/T66/Core/T66IdolManagerSubsystem.h` (99 lines): **—**
- `Source/T66/Core/T66InteractionPromptSubsystem.cpp` (220 lines): **—**
- `Source/T66/Core/T66InteractionPromptSubsystem.h` (42 lines): **—**
- `Source/T66/Core/T66LagTrackerSubsystem.cpp` (415 lines): **—**
- `Source/T66/Core/T66LagTrackerSubsystem.h` (134 lines): **—**
- `Source/T66/Core/T66LeaderboardPacingUtils.cpp` (115 lines): **—**
- `Source/T66/Core/T66LeaderboardPacingUtils.h` (17 lines): **—**
- `Source/T66/Core/T66LeaderboardRunSummarySaveGame.cpp` (5 lines): **—**
- `Source/T66/Core/T66LeaderboardRunSummarySaveGame.h` (299 lines): **—**
- `Source/T66/Core/T66LeaderboardSubsystem.cpp` (2324 lines): **—**
- `Source/T66/Core/T66LeaderboardSubsystem.h` (322 lines): **—**
- `Source/T66/Core/T66LegacyRuntimeDataAccess.cpp` (79 lines): **FILE_IO**
- `Source/T66/Core/T66LegacyRuntimeDataAccess.h` (43 lines): **—**
- `Source/T66/Core/T66LegacyRuntimeTextureAccess.cpp` (71 lines): **FILE_IO**
- `Source/T66/Core/T66LegacyRuntimeTextureAccess.h` (20 lines): **—**
- `Source/T66/Core/T66LocalLeaderboardSaveGame.cpp` (5 lines): **—**
- `Source/T66/Core/T66LocalLeaderboardSaveGame.h` (243 lines): **—**
- `Source/T66/Core/T66LocalizationSubsystem.cpp` (2300 lines): **—**
- `Source/T66/Core/T66LocalizationSubsystem.h` (1104 lines): **—**
- `Source/T66/Core/T66MediaViewerSubsystem.cpp` (364 lines): **FILE_IO, WEBVIEW**
- `Source/T66/Core/T66MediaViewerSubsystem.h` (102 lines): **WEBVIEW**
- `Source/T66/Core/T66MusicSubsystem.cpp` (823 lines): **—**
- `Source/T66/Core/T66MusicSubsystem.h` (161 lines): **—**
- `Source/T66/Core/T66PartySubsystem.cpp` (479 lines): **—**
- `Source/T66/Core/T66PartySubsystem.h` (127 lines): **—**
- `Source/T66/Core/T66PixelVFXSubsystem.cpp` (291 lines): **NIAGARA**
- `Source/T66/Core/T66PixelVFXSubsystem.h` (77 lines): **NIAGARA**
- `Source/T66/Core/T66Pixelation.cpp` (52 lines): **—**
- `Source/T66/Core/T66PixelationSubsystem.cpp` (215 lines): **ITERATOR, DYNAMIC_MAT**
- `Source/T66/Core/T66PixelationSubsystem.h` (69 lines): **—**
- `Source/T66/Core/T66PlayerExperienceSubSystem.cpp` (241 lines): **—**
- `Source/T66/Core/T66PlayerExperienceSubSystem.h` (186 lines): **—**
- `Source/T66/Core/T66PlayerSettingsSaveGame.cpp` (5 lines): **—**
- `Source/T66/Core/T66PlayerSettingsSaveGame.h` (240 lines): **—**
- `Source/T66/Core/T66PlayerSettingsSubsystem.cpp` (1169 lines): **—**
- `Source/T66/Core/T66PlayerSettingsSubsystem.h` (311 lines): **—**
- `Source/T66/Core/T66ProfileSaveGame.h` (189 lines): **—**
- `Source/T66/Core/T66Rarity.cpp` (86 lines): **—**
- `Source/T66/Core/T66Rarity.h` (40 lines): **—**
- `Source/T66/Core/T66ReleaseVariantSubsystem.cpp` (227 lines): **—**
- `Source/T66/Core/T66ReleaseVariantSubsystem.h` (69 lines): **—**
- `Source/T66/Core/T66RetroFXSettings.h` (235 lines): **—**
- `Source/T66/Core/T66RetroFXSubsystem.cpp` (1906 lines): **ITERATOR, DYNAMIC_MAT**
- `Source/T66/Core/T66RetroFXSubsystem.h` (213 lines): **—**
- `Source/T66/Core/T66RngSubsystem.cpp` (250 lines): **—**
- `Source/T66/Core/T66RngSubsystem.h` (123 lines): **—**
- `Source/T66/Core/T66RngTuningConfig.cpp` (74 lines): **—**
- `Source/T66/Core/T66RngTuningConfig.h` (107 lines): **—**
- `Source/T66/Core/T66RunIntegritySubsystem.cpp` (322 lines): **FILE_IO**
- `Source/T66/Core/T66RunIntegritySubsystem.h` (40 lines): **—**
- `Source/T66/Core/T66RunIntegrityTypes.h` (97 lines): **—**
- `Source/T66/Core/T66RunSaveGame.h` (678 lines): **—**
- `Source/T66/Core/T66RunStateSubsystem.h` (1898 lines): **—**
- `Source/T66/Core/T66RuntimePlatformSubsystem.cpp` (130 lines): **—**
- `Source/T66/Core/T66RuntimePlatformSubsystem.h` (67 lines): **—**
- `Source/T66/Core/T66SaveIndex.h` (43 lines): **—**
- `Source/T66/Core/T66SaveMigration.h` (43 lines): **—**
- `Source/T66/Core/T66SaveSubsystem.cpp` (268 lines): **NEW_OBJECT**
- `Source/T66/Core/T66SaveSubsystem.h` (52 lines): **—**
- `Source/T66/Core/T66SessionSubsystem.cpp` (2479 lines): **NEW_OBJECT**
- `Source/T66/Core/T66SessionSubsystem.h` (181 lines): **—**
- `Source/T66/Core/T66SkillRatingSubsystem.cpp` (128 lines): **—**
- `Source/T66/Core/T66SkillRatingSubsystem.h` (82 lines): **—**
- `Source/T66/Core/T66SkinSubsystem.cpp` (356 lines): **—**
- `Source/T66/Core/T66SkinSubsystem.h` (118 lines): **—**
- `Source/T66/Core/T66StageProgressionSubsystem.cpp` (142 lines): **—**
- `Source/T66/Core/T66StageProgressionSubsystem.h` (109 lines): **—**
- `Source/T66/Core/T66StageProgressionTuningConfig.cpp` (89 lines): **—**
- `Source/T66/Core/T66StageProgressionTuningConfig.h` (48 lines): **—**
- `Source/T66/Core/T66SteamHelper.cpp` (678 lines): **—**
- `Source/T66/Core/T66SteamHelper.h` (157 lines): **—**
- `Source/T66/Core/T66TrapSubsystem.cpp` (1015 lines): **NIAGARA**
- `Source/T66/Core/T66TrapSubsystem.h` (51 lines): **—**
- `Source/T66/Core/T66TrapTuningConfig.cpp` (379 lines): **NIAGARA**
- `Source/T66/Core/T66TrapTuningConfig.h` (224 lines): **NIAGARA**
- `Source/T66/Core/T66UITexturePoolSubsystem.cpp` (291 lines): **—**
- `Source/T66/Core/T66UITexturePoolSubsystem.h` (94 lines): **—**
- `Source/T66/Core/T66WeaponManagerSubsystem.cpp` (208 lines): **—**
- `Source/T66/Core/T66WeaponManagerSubsystem.h` (58 lines): **—**
- `Source/T66/Core/T66WebImageCache.cpp` (196 lines): **HTTP**
- `Source/T66/Core/T66WebImageCache.h` (52 lines): **HTTP**
- `Source/T66/Core/T66WebView2Host.cpp` (1336 lines): **WEBVIEW**
- `Source/T66/Core/T66WebView2Host.h` (59 lines): **WEBVIEW**
- `Source/T66/Data/T66DataTypes.h` (2328 lines): **—**
- `Source/T66/Gameplay/Enemies/Projectiles/T66EnemyProjectileBase.cpp` (81 lines): **TICK_ACTOR**
- `Source/T66/Gameplay/Enemies/Projectiles/T66EnemyProjectileBase.h` (41 lines): **—**
- `Source/T66/Gameplay/Enemies/Projectiles/T66SpitProjectile.cpp` (22 lines): **—**
- `Source/T66/Gameplay/Enemies/Projectiles/T66SpitProjectile.h` (17 lines): **—**
- `Source/T66/Gameplay/Enemies/T66EnemyFamilyResolver.cpp` (115 lines): **—**
- `Source/T66/Gameplay/Enemies/T66EnemyFamilyResolver.h` (19 lines): **—**
- `Source/T66/Gameplay/Enemies/T66EnemyFamilyTypes.h` (17 lines): **—**
- `Source/T66/Gameplay/Enemies/T66FlyingEnemy.cpp` (43 lines): **—**
- `Source/T66/Gameplay/Enemies/T66FlyingEnemy.h` (35 lines): **—**
- `Source/T66/Gameplay/Enemies/T66MeleeEnemy.cpp` (15 lines): **—**
- `Source/T66/Gameplay/Enemies/T66MeleeEnemy.h` (17 lines): **—**
- `Source/T66/Gameplay/Enemies/T66RangedEnemy.cpp` (112 lines): **—**
- `Source/T66/Gameplay/Enemies/T66RangedEnemy.h` (43 lines): **—**
- `Source/T66/Gameplay/Enemies/T66RushEnemy.cpp` (84 lines): **—**
- `Source/T66/Gameplay/Enemies/T66RushEnemy.h` (38 lines): **—**
- `Source/T66/Gameplay/GameMode/T66GameModePrivate.h` (178 lines): **—**
- `Source/T66/Gameplay/GameMode/T66GameMode_Bootstrap.cpp` (769 lines): **ITERATOR**
- `Source/T66/Gameplay/GameMode/T66GameMode_BossFlow.cpp` (1266 lines): **TRACE, SYNC_LOAD, ITERATOR, NEW_OBJECT, DYNAMIC_MAT**
- `Source/T66/Gameplay/GameMode/T66GameMode_Lab.cpp` (287 lines): **ITERATOR**
- `Source/T66/Gameplay/GameMode/T66GameMode_MainMap.cpp` (1042 lines): **TRACE, ITERATOR**
- `Source/T66/Gameplay/GameMode/T66GameMode_Spawning.cpp` (786 lines): **TRACE**
- `Source/T66/Gameplay/GameMode/T66GameMode_Tower.cpp` (506 lines): **ITERATOR**
- `Source/T66/Gameplay/GameMode/T66GameMode_WorldInteractables.cpp` (2389 lines): **TRACE, SYNC_LOAD, ITERATOR**
- `Source/T66/Gameplay/Movement/T66HeroMovementComponent.cpp` (236 lines): **—**
- `Source/T66/Gameplay/Movement/T66HeroMovementComponent.h` (68 lines): **—**
- `Source/T66/Gameplay/Movement/T66HeroMovementTypes.h` (78 lines): **—**
- `Source/T66/Gameplay/T66ArcadeAmplifierPickup.cpp` (147 lines): **TICK_ACTOR, SYNC_LOAD**
- `Source/T66/Gameplay/T66ArcadeAmplifierPickup.h` (51 lines): **—**
- `Source/T66/Gameplay/T66ArcadeInteractableBase.cpp` (681 lines): **SYNC_LOAD**
- `Source/T66/Gameplay/T66ArcadeInteractableBase.h` (60 lines): **—**
- `Source/T66/Gameplay/T66ArcadeInteractableTypes.h` (216 lines): **—**
- `Source/T66/Gameplay/T66ArcadeMachineInteractable.cpp` (241 lines): **TRACE, SYNC_LOAD, NEW_OBJECT, DYNAMIC_MAT**
- `Source/T66/Gameplay/T66ArcadeMachineInteractable.h` (52 lines): **—**
- `Source/T66/Gameplay/T66ArcadeTruckInteractable.cpp` (213 lines): **SYNC_LOAD**
- `Source/T66/Gameplay/T66ArcadeTruckInteractable.h` (44 lines): **—**
- `Source/T66/Gameplay/T66ArthurSwordVisuals.cpp` (21 lines): **SYNC_LOAD**
- `Source/T66/Gameplay/T66ArthurSwordVisuals.h` (14 lines): **—**
- `Source/T66/Gameplay/T66ArthurUltimateSword.cpp` (101 lines): **TICK_ACTOR**
- `Source/T66/Gameplay/T66ArthurUltimateSword.h` (46 lines): **—**
- `Source/T66/Gameplay/T66BossAttackTypes.h` (17 lines): **—**
- `Source/T66/Gameplay/T66BossBase.cpp` (1593 lines): **TICK_ACTOR, TRACE, NEW_OBJECT**
- `Source/T66/Gameplay/T66BossBase.h` (223 lines): **—**
- `Source/T66/Gameplay/T66BossGate.cpp` (134 lines): **TICK_ACTOR**
- `Source/T66/Gameplay/T66BossGate.h` (46 lines): **—**
- `Source/T66/Gameplay/T66BossGroundAOE.cpp` (352 lines): **TICK_ACTOR, TRACE, NIAGARA**
- `Source/T66/Gameplay/T66BossGroundAOE.h` (75 lines): **NIAGARA**
- `Source/T66/Gameplay/T66BossPartTypes.h` (58 lines): **—**
- `Source/T66/Gameplay/T66BossProjectile.cpp` (385 lines): **TICK_ACTOR, NIAGARA**
- `Source/T66/Gameplay/T66BossProjectile.h` (66 lines): **NIAGARA**
- `Source/T66/Gameplay/T66ChestInteractable.cpp` (144 lines): **—**
- `Source/T66/Gameplay/T66ChestInteractable.h` (31 lines): **—**
- `Source/T66/Gameplay/T66ChestMimicEnemy.cpp` (43 lines): **TICK_ACTOR**
- `Source/T66/Gameplay/T66ChestMimicEnemy.h` (25 lines): **—**
- `Source/T66/Gameplay/T66CombatComponent.cpp` (2661 lines): **TRACE, SYNC_LOAD, ITERATOR, NIAGARA, NEW_OBJECT**
- `Source/T66/Gameplay/T66CombatComponent.h` (229 lines): **NIAGARA**
- `Source/T66/Gameplay/T66CombatHitZoneComponent.cpp` (33 lines): **—**
- `Source/T66/Gameplay/T66CombatHitZoneComponent.h` (36 lines): **—**
- `Source/T66/Gameplay/T66CombatShared.cpp` (119 lines): **TRACE**
- `Source/T66/Gameplay/T66CombatShared.h` (21 lines): **—**
- `Source/T66/Gameplay/T66CombatTargetTypes.h` (64 lines): **—**
- `Source/T66/Gameplay/T66CombatVFX.cpp` (1522 lines): **NIAGARA**
- `Source/T66/Gameplay/T66CompanionBase.cpp` (430 lines): **TICK_ACTOR, TRACE, DYNAMIC_MAT**
- `Source/T66/Gameplay/T66CompanionBase.h` (140 lines): **TRACE**
- `Source/T66/Gameplay/T66CowardiceGate.cpp` (120 lines): **TICK_ACTOR, SYNC_LOAD**
- `Source/T66/Gameplay/T66CowardiceGate.h` (46 lines): **—**
- `Source/T66/Gameplay/T66CrateInteractable.cpp` (44 lines): **—**
- `Source/T66/Gameplay/T66CrateInteractable.h` (25 lines): **—**
- `Source/T66/Gameplay/T66DifficultyTotem.cpp` (187 lines): **TICK_ACTOR, NEW_OBJECT**
- `Source/T66/Gameplay/T66DifficultyTotem.h` (42 lines): **—**
- `Source/T66/Gameplay/T66EnemyAIController.cpp` (27 lines): **TICK_ACTOR**
- `Source/T66/Gameplay/T66EnemyAIController.h` (28 lines): **—**
- `Source/T66/Gameplay/T66EnemyBase.cpp` (1743 lines): **TICK_ACTOR, ITERATOR**
- `Source/T66/Gameplay/T66EnemyBase.h` (333 lines): **—**
- `Source/T66/Gameplay/T66EnemyDirector.cpp` (1310 lines): **TICK_ACTOR, TRACE, ITERATOR**
- `Source/T66/Gameplay/T66EnemyDirector.h` (163 lines): **—**
- `Source/T66/Gameplay/T66FloatingCombatTextActor.cpp` (80 lines): **TICK_ACTOR**
- `Source/T66/Gameplay/T66FloatingCombatTextActor.h` (33 lines): **—**
- `Source/T66/Gameplay/T66FountainInteractable.cpp` (78 lines): **—**
- `Source/T66/Gameplay/T66FountainInteractable.h` (29 lines): **—**
- `Source/T66/Gameplay/T66FrontendGameMode.cpp` (95 lines): **—**
- `Source/T66/Gameplay/T66FrontendGameMode.h` (28 lines): **—**
- `Source/T66/Gameplay/T66GalleryDisplayActor.cpp` (218 lines): **TICK_ACTOR**
- `Source/T66/Gameplay/T66GalleryDisplayActor.h` (70 lines): **—**
- `Source/T66/Gameplay/T66GamblerBoss.cpp` (173 lines): **DYNAMIC_MAT**
- `Source/T66/Gameplay/T66GamblerBoss.h` (22 lines): **—**
- `Source/T66/Gameplay/T66GamblerNPC.cpp` (78 lines): **SYNC_LOAD**
- `Source/T66/Gameplay/T66GamblerNPC.h` (33 lines): **—**
- `Source/T66/Gameplay/T66GameMode.cpp` (1752 lines): **TICK_ACTOR, TRACE, SYNC_LOAD, ITERATOR**
- `Source/T66/Gameplay/T66GameMode.h` (386 lines): **—**
- `Source/T66/Gameplay/T66GoblinThiefEnemy.cpp` (135 lines): **—**
- `Source/T66/Gameplay/T66GoblinThiefEnemy.h` (43 lines): **—**
- `Source/T66/Gameplay/T66HeroBase.cpp` (1196 lines): **TICK_ACTOR, TRACE, SYNC_LOAD, DYNAMIC_MAT**
- `Source/T66/Gameplay/T66HeroBase.h` (310 lines): **—**
- `Source/T66/Gameplay/T66HeroOneAttackVFX.cpp` (439 lines): **TICK_ACTOR, SYNC_LOAD, DYNAMIC_MAT**
- `Source/T66/Gameplay/T66HeroOneAttackVFX.h` (90 lines): **NIAGARA**
- `Source/T66/Gameplay/T66HeroPlagueCloud.cpp` (164 lines): **TICK_ACTOR, TRACE, NIAGARA**
- `Source/T66/Gameplay/T66HeroPlagueCloud.h` (61 lines): **NIAGARA**
- `Source/T66/Gameplay/T66HeroProjectile.cpp` (315 lines): **TICK_ACTOR, NIAGARA**
- `Source/T66/Gameplay/T66HeroProjectile.h` (77 lines): **NIAGARA**
- `Source/T66/Gameplay/T66HouseNPCBase.cpp` (440 lines): **TICK_ACTOR, TRACE, ITERATOR, DYNAMIC_MAT**
- `Source/T66/Gameplay/T66HouseNPCBase.h` (150 lines): **TRACE**
- `Source/T66/Gameplay/T66IdolAltar.cpp` (179 lines): **TICK_ACTOR, SYNC_LOAD**
- `Source/T66/Gameplay/T66IdolAltar.h` (125 lines): **—**
- `Source/T66/Gameplay/T66LabCollector.cpp` (20 lines): **—**
- `Source/T66/Gameplay/T66LabCollector.h` (24 lines): **—**
- `Source/T66/Gameplay/T66LavaPatch.cpp` (520 lines): **TICK_ACTOR, TRACE, SYNC_LOAD, DYNAMIC_MAT**
- `Source/T66/Gameplay/T66LavaPatch.h` (154 lines): **—**
- `Source/T66/Gameplay/T66LavaShared.h` (83 lines): **—**
- `Source/T66/Gameplay/T66LoanShark.cpp` (213 lines): **TICK_ACTOR, DYNAMIC_MAT**
- `Source/T66/Gameplay/T66LoanShark.h` (54 lines): **—**
- `Source/T66/Gameplay/T66LootBagPickup.cpp` (408 lines): **TICK_ACTOR, SYNC_LOAD, ITERATOR**
- `Source/T66/Gameplay/T66LootBagPickup.h` (115 lines): **—**
- `Source/T66/Gameplay/T66MainMapTerrain.cpp` (2679 lines): **SYNC_LOAD, NEW_OBJECT, DYNAMIC_MAT**
- `Source/T66/Gameplay/T66MainMapTerrain.h` (136 lines): **—**
- `Source/T66/Gameplay/T66MainMapTerrainTypes.h` (30 lines): **—**
- `Source/T66/Gameplay/T66MiasmaBoundary.cpp` (328 lines): **TICK_ACTOR, SYNC_LOAD, DYNAMIC_MAT**
- `Source/T66/Gameplay/T66MiasmaBoundary.h` (65 lines): **—**
- `Source/T66/Gameplay/T66MiasmaManager.cpp` (1077 lines): **TICK_ACTOR, TRACE, SYNC_LOAD, DYNAMIC_MAT**
- `Source/T66/Gameplay/T66MiasmaManager.h` (181 lines): **—**
- `Source/T66/Gameplay/T66MiasmaTile.cpp` (95 lines): **TICK_ACTOR**
- `Source/T66/Gameplay/T66MiasmaTile.h` (52 lines): **—**
- `Source/T66/Gameplay/T66OuroborosNPC.cpp` (30 lines): **—**
- `Source/T66/Gameplay/T66OuroborosNPC.h` (24 lines): **—**
- `Source/T66/Gameplay/T66PilotableTractor.cpp` (405 lines): **TICK_ACTOR, TRACE**
- `Source/T66/Gameplay/T66PilotableTractor.h` (92 lines): **—**
- `Source/T66/Gameplay/T66PlayerController.cpp` (1414 lines): **TRACE, SYNC_LOAD, ITERATOR, NIAGARA, DYNAMIC_MAT**
- `Source/T66/Gameplay/T66PlayerController.h` (544 lines): **PLAYER_TICK, NIAGARA**
- `Source/T66/Gameplay/T66PlayerController_Combat.cpp` (1473 lines): **PLAYER_TICK, TRACE, NIAGARA, NEW_OBJECT**
- `Source/T66/Gameplay/T66PlayerController_Frontend.cpp` (1540 lines): **FILE_IO, NIAGARA, NEW_OBJECT**
- `Source/T66/Gameplay/T66PlayerController_Input.cpp` (684 lines): **NIAGARA**
- `Source/T66/Gameplay/T66PlayerController_Movement.cpp` (307 lines): **NIAGARA**
- `Source/T66/Gameplay/T66PlayerController_Overlays.cpp` (1760 lines): **FILE_IO, NIAGARA, NEW_OBJECT**
- `Source/T66/Gameplay/T66PlayerController_ScopedUlt.cpp` (294 lines): **TRACE, NIAGARA**
- `Source/T66/Gameplay/T66PlayerController_WorldDialogue.cpp` (279 lines): **NIAGARA**
- `Source/T66/Gameplay/T66ProceduralLandscapeParams.h` (159 lines): **—**
- `Source/T66/Gameplay/T66QuickReviveVendingMachine.cpp` (114 lines): **—**
- `Source/T66/Gameplay/T66QuickReviveVendingMachine.h` (35 lines): **—**
- `Source/T66/Gameplay/T66RecruitableCompanion.cpp` (137 lines): **TICK_ACTOR, TRACE, DYNAMIC_MAT**
- `Source/T66/Gameplay/T66RecruitableCompanion.h` (65 lines): **—**
- `Source/T66/Gameplay/T66SaintNPC.cpp` (24 lines): **—**
- `Source/T66/Gameplay/T66SaintNPC.h` (21 lines): **—**
- `Source/T66/Gameplay/T66SessionPlayerState.cpp` (62 lines): **—**
- `Source/T66/Gameplay/T66SessionPlayerState.h` (119 lines): **—**
- `Source/T66/Gameplay/T66SpawnPlateau.cpp` (28 lines): **TICK_ACTOR**
- `Source/T66/Gameplay/T66SpawnPlateau.h` (34 lines): **—**
- `Source/T66/Gameplay/T66StageCatchUpGate.cpp` (133 lines): **TICK_ACTOR**
- `Source/T66/Gameplay/T66StageCatchUpGate.h` (42 lines): **—**
- `Source/T66/Gameplay/T66StageEffects.cpp` (171 lines): **TICK_ACTOR, SYNC_LOAD, NIAGARA**
- `Source/T66/Gameplay/T66StageEffects.h` (74 lines): **NIAGARA**
- `Source/T66/Gameplay/T66StageGate.cpp` (145 lines): **TICK_ACTOR, ITERATOR**
- `Source/T66/Gameplay/T66StageGate.h` (43 lines): **—**
- `Source/T66/Gameplay/T66StageProgressionVisuals.cpp` (22 lines): **—**
- `Source/T66/Gameplay/T66StageProgressionVisuals.h` (16 lines): **—**
- `Source/T66/Gameplay/T66StartGate.cpp` (110 lines): **TICK_ACTOR**
- `Source/T66/Gameplay/T66StartGate.h` (54 lines): **—**
- `Source/T66/Gameplay/T66TerrainThemeAssets.cpp` (91 lines): **SYNC_LOAD, DYNAMIC_MAT**
- `Source/T66/Gameplay/T66TerrainThemeAssets.h` (17 lines): **—**
- `Source/T66/Gameplay/T66TowerDescentHole.cpp` (76 lines): **TICK_ACTOR**
- `Source/T66/Gameplay/T66TowerDescentHole.h` (41 lines): **—**
- `Source/T66/Gameplay/T66TowerMapTerrain.cpp` (5873 lines): **TRACE, NEW_OBJECT**
- `Source/T66/Gameplay/T66TowerMapTerrain.h` (190 lines): **—**
- `Source/T66/Gameplay/T66TowerThemeVisuals.cpp` (347 lines): **SYNC_LOAD, DYNAMIC_MAT**
- `Source/T66/Gameplay/T66TowerThemeVisuals.h` (48 lines): **—**
- `Source/T66/Gameplay/T66TutorialGuideCompanion.cpp` (179 lines): **TICK_ACTOR, TRACE**
- `Source/T66/Gameplay/T66TutorialGuideCompanion.h` (54 lines): **—**
- `Source/T66/Gameplay/T66TutorialManager.cpp` (856 lines): **TICK_ACTOR, TRACE, ITERATOR**
- `Source/T66/Gameplay/T66TutorialManager.h` (144 lines): **—**
- `Source/T66/Gameplay/T66TutorialPortal.cpp` (96 lines): **TICK_ACTOR, TRACE**
- `Source/T66/Gameplay/T66TutorialPortal.h` (43 lines): **—**
- `Source/T66/Gameplay/T66TutorialPromptActor.cpp` (67 lines): **TICK_ACTOR**
- `Source/T66/Gameplay/T66TutorialPromptActor.h` (49 lines): **—**
- `Source/T66/Gameplay/T66UniqueDebuffEnemy.cpp` (155 lines): **TICK_ACTOR**
- `Source/T66/Gameplay/T66UniqueDebuffEnemy.h` (39 lines): **—**
- `Source/T66/Gameplay/T66UniqueDebuffProjectile.cpp` (142 lines): **TICK_ACTOR, NIAGARA**
- `Source/T66/Gameplay/T66UniqueDebuffProjectile.h` (54 lines): **NIAGARA**
- `Source/T66/Gameplay/T66VisualUtil.cpp` (419 lines): **TRACE, SYNC_LOAD, ITERATOR, DYNAMIC_MAT**
- `Source/T66/Gameplay/T66VisualUtil.h` (44 lines): **—**
- `Source/T66/Gameplay/T66WhackAMoleArcadeInteractable.cpp` (49 lines): **—**
- `Source/T66/Gameplay/T66WhackAMoleArcadeInteractable.h` (17 lines): **—**
- `Source/T66/Gameplay/T66WorldInteractableBase.cpp` (482 lines): **SYNC_LOAD**
- `Source/T66/Gameplay/T66WorldInteractableBase.h` (107 lines): **—**
- `Source/T66/Gameplay/T66WorldVisualSetup.cpp` (226 lines): **ITERATOR**
- `Source/T66/Gameplay/T66WorldVisualSetup.h` (14 lines): **—**
- `Source/T66/Gameplay/Traps/T66FloorFlameTrap.cpp` (514 lines): **TICK_ACTOR, NIAGARA**
- `Source/T66/Gameplay/Traps/T66FloorFlameTrap.h` (108 lines): **NIAGARA**
- `Source/T66/Gameplay/Traps/T66FloorSpikePatchTrap.cpp` (556 lines): **TICK_ACTOR, NIAGARA**
- `Source/T66/Gameplay/Traps/T66FloorSpikePatchTrap.h` (122 lines): **NIAGARA**
- `Source/T66/Gameplay/Traps/T66TrapArrowProjectile.cpp` (200 lines): **TICK_ACTOR, NIAGARA**
- `Source/T66/Gameplay/Traps/T66TrapArrowProjectile.h` (71 lines): **NIAGARA**
- `Source/T66/Gameplay/Traps/T66TrapBase.cpp` (209 lines): **TICK_ACTOR**
- `Source/T66/Gameplay/Traps/T66TrapBase.h` (118 lines): **—**
- `Source/T66/Gameplay/Traps/T66TrapDamageUtils.cpp` (76 lines): **TRACE**
- `Source/T66/Gameplay/Traps/T66TrapDamageUtils.h` (18 lines): **—**
- `Source/T66/Gameplay/Traps/T66TrapPressurePlate.cpp` (229 lines): **TICK_ACTOR**
- `Source/T66/Gameplay/Traps/T66TrapPressurePlate.h` (89 lines): **—**
- `Source/T66/Gameplay/Traps/T66WallArrowTrap.cpp` (345 lines): **—**
- `Source/T66/Gameplay/Traps/T66WallArrowTrap.h` (82 lines): **—**
- `Source/T66/T66.cpp` (26 lines): **—**
- `Source/T66/T66.h` (20 lines): **—**
- `Source/T66/UI/Components/T66FlatLeaderboardPanel.cpp` (2011 lines): **—**
- `Source/T66/UI/Components/T66FlatLeaderboardPanel.h` (170 lines): **—**
- `Source/T66/UI/Components/T66LeaderboardPanel.cpp` (3287 lines): **HTTP**
- `Source/T66/UI/Components/T66LeaderboardPanel.h` (200 lines): **—**
- `Source/T66/UI/Components/T66MinigameMenuLayout.cpp` (628 lines): **—**
- `Source/T66/UI/Components/T66MinigameMenuLayout.h` (131 lines): **—**
- `Source/T66/UI/Gambler/T66GamblerOverlayWidget_BlackJack.cpp` (637 lines): **—**
- `Source/T66/UI/Gambler/T66GamblerOverlayWidget_Build.cpp` (2186 lines): **—**
- `Source/T66/UI/Gambler/T66GamblerOverlayWidget_ChanceGames.cpp` (505 lines): **—**
- `Source/T66/UI/Gambler/T66GamblerOverlayWidget_Cheat.cpp` (568 lines): **—**
- `Source/T66/UI/Gambler/T66GamblerOverlayWidget_CoinGames.cpp` (244 lines): **—**
- `Source/T66/UI/Gambler/T66GamblerOverlayWidget_Economy.cpp` (968 lines): **—**
- `Source/T66/UI/HUD/T66GameplayHUDWidget.cpp` (419 lines): **TICK_ACTOR**
- `Source/T66/UI/HUD/T66GameplayHUDWidget_Build.cpp` (2425 lines): **—**
- `Source/T66/UI/HUD/T66GameplayHUDWidget_Map.cpp` (520 lines): **ITERATOR**
- `Source/T66/UI/HUD/T66GameplayHUDWidget_Overlays.cpp` (538 lines): **WEBVIEW**
- `Source/T66/UI/HUD/T66GameplayHUDWidget_Presentations.cpp` (94 lines): **—**
- `Source/T66/UI/HUD/T66GameplayHUDWidget_Private.h` (2408 lines): **ITERATOR**
- `Source/T66/UI/HUD/T66GameplayHUDWidget_Refresh.cpp` (1246 lines): **—**
- `Source/T66/UI/HUD/T66HUDPresentationController.cpp` (670 lines): **—**
- `Source/T66/UI/HUD/T66HUDPresentationController.h` (80 lines): **—**
- `Source/T66/UI/Screens/HeroSelection/T66HeroSelectionPreviewController.cpp` (335 lines): **NEW_OBJECT**
- `Source/T66/UI/Screens/HeroSelection/T66HeroSelectionPreviewController.h` (90 lines): **—**
- `Source/T66/UI/Screens/HeroSelection/T66HeroSelectionScreen_Build.cpp` (765 lines): **—**
- `Source/T66/UI/Screens/HeroSelection/T66HeroSelectionScreen_Party.cpp` (331 lines): **—**
- `Source/T66/UI/Screens/HeroSelection/T66HeroSelectionScreen_Preview.cpp` (642 lines): **FILE_IO**
- `Source/T66/UI/Screens/HeroSelection/T66HeroSelectionScreen_Private.h` (1112 lines): **—**
- `Source/T66/UI/Screens/HeroSelection/T66HeroSelectionScreen_RetroFX.cpp` (468 lines): **—**
- `Source/T66/UI/Screens/HeroSelection/T66HeroSelectionScreen_Skins.cpp` (668 lines): **—**
- `Source/T66/UI/Screens/HeroSelection/T66HeroSelectionScreen_Stats.cpp` (412 lines): **NEW_OBJECT**
- `Source/T66/UI/Screens/Settings/T66SettingsScreen_Audio.cpp` (431 lines): **—**
- `Source/T66/UI/Screens/Settings/T66SettingsScreen_Build.cpp` (503 lines): **—**
- `Source/T66/UI/Screens/Settings/T66SettingsScreen_Controls.cpp` (808 lines): **—**
- `Source/T66/UI/Screens/Settings/T66SettingsScreen_Crashing.cpp` (95 lines): **—**
- `Source/T66/UI/Screens/Settings/T66SettingsScreen_Gameplay.cpp` (823 lines): **—**
- `Source/T66/UI/Screens/Settings/T66SettingsScreen_Graphics.cpp` (1046 lines): **SET_TIMER_LOOP**
- `Source/T66/UI/Screens/Settings/T66SettingsScreen_HUD.cpp` (401 lines): **—**
- `Source/T66/UI/Screens/Settings/T66SettingsScreen_Private.h` (729 lines): **—**
- `Source/T66/UI/Screens/Settings/T66SettingsScreen_Rebinding.cpp` (286 lines): **—**
- `Source/T66/UI/Screens/Settings/T66SettingsScreen_RetroFX.cpp` (447 lines): **—**
- `Source/T66/UI/Screens/T66AccountStatusScreen.cpp` (5160 lines): **FILE_IO**
- `Source/T66/UI/Screens/T66AccountStatusScreen.h` (98 lines): **—**
- `Source/T66/UI/Screens/T66AchievementsScreen.cpp` (2364 lines): **—**
- `Source/T66/UI/Screens/T66AchievementsScreen.h` (70 lines): **—**
- `Source/T66/UI/Screens/T66ChallengesScreen.cpp` (2565 lines): **—**
- `Source/T66/UI/Screens/T66ChallengesScreen.h` (103 lines): **—**
- `Source/T66/UI/Screens/T66CompanionGridScreen.cpp` (303 lines): **—**
- `Source/T66/UI/Screens/T66CompanionGridScreen.h` (37 lines): **—**
- `Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp` (1466 lines): **NEW_OBJECT**
- `Source/T66/UI/Screens/T66CompanionSelectionScreen.h` (169 lines): **—**
- `Source/T66/UI/Screens/T66DailyClimbScreen.cpp` (1513 lines): **—**
- `Source/T66/UI/Screens/T66DailyClimbScreen.h` (56 lines): **—**
- `Source/T66/UI/Screens/T66HeroGridScreen.cpp` (249 lines): **—**
- `Source/T66/UI/Screens/T66HeroGridScreen.h` (37 lines): **—**
- `Source/T66/UI/Screens/T66HeroSelectionScreen.cpp` (726 lines): **NEW_OBJECT**
- `Source/T66/UI/Screens/T66HeroSelectionScreen.h` (316 lines): **—**
- `Source/T66/UI/Screens/T66LanguageSelectScreen.cpp` (366 lines): **—**
- `Source/T66/UI/Screens/T66LanguageSelectScreen.h` (51 lines): **—**
- `Source/T66/UI/Screens/T66MainMenuScreen.cpp` (3981 lines): **TICK_ACTOR, NEW_OBJECT**
- `Source/T66/UI/Screens/T66MainMenuScreen.h` (236 lines): **—**
- `Source/T66/UI/Screens/T66MinigamesScreen.cpp` (533 lines): **—**
- `Source/T66/UI/Screens/T66MinigamesScreen.h` (46 lines): **—**
- `Source/T66/UI/Screens/T66PartyInviteModal.cpp` (496 lines): **—**
- `Source/T66/UI/Screens/T66PartyInviteModal.h` (43 lines): **—**
- `Source/T66/UI/Screens/T66PauseMenuScreen.cpp` (231 lines): **—**
- `Source/T66/UI/Screens/T66PauseMenuScreen.h` (56 lines): **—**
- `Source/T66/UI/Screens/T66PlayerSummaryPickerScreen.cpp` (340 lines): **—**
- `Source/T66/UI/Screens/T66PlayerSummaryPickerScreen.h` (31 lines): **—**
- `Source/T66/UI/Screens/T66PowerUpScreen.cpp` (3082 lines): **—**
- `Source/T66/UI/Screens/T66PowerUpScreen.h` (49 lines): **—**
- `Source/T66/UI/Screens/T66QuitConfirmationModal.cpp` (155 lines): **—**
- `Source/T66/UI/Screens/T66QuitConfirmationModal.h` (33 lines): **—**
- `Source/T66/UI/Screens/T66ReportBugScreen.cpp` (332 lines): **FILE_IO**
- `Source/T66/UI/Screens/T66ReportBugScreen.h` (33 lines): **—**
- `Source/T66/UI/Screens/T66RunSummaryScreen.cpp` (4164 lines): **NEW_OBJECT, SLATE_INV**
- `Source/T66/UI/Screens/T66RunSummaryScreen.h` (172 lines): **—**
- `Source/T66/UI/Screens/T66SavePreviewScreen.cpp` (247 lines): **—**
- `Source/T66/UI/Screens/T66SavePreviewScreen.h` (34 lines): **—**
- `Source/T66/UI/Screens/T66SaveSlotsScreen.cpp` (1227 lines): **—**
- `Source/T66/UI/Screens/T66SaveSlotsScreen.h` (74 lines): **—**
- `Source/T66/UI/Screens/T66ScreenSlateHelpers.cpp` (1315 lines): **—**
- `Source/T66/UI/Screens/T66ScreenSlateHelpers.h` (136 lines): **—**
- `Source/T66/UI/Screens/T66SelectionScreenUtils.h` (116 lines): **—**
- `Source/T66/UI/Screens/T66SettingsScreen.cpp` (228 lines): **—**
- `Source/T66/UI/Screens/T66SettingsScreen.h` (235 lines): **—**
- `Source/T66/UI/Style/T66ButtonVisuals.cpp` (19 lines): **—**
- `Source/T66/UI/Style/T66ButtonVisuals.h` (114 lines): **—**
- `Source/T66/UI/Style/T66CasinoAlchemyTabReferenceLayout.generated.h` (35 lines): **—**
- `Source/T66/UI/Style/T66CasinoGamblingTabReferenceLayout.generated.h` (35 lines): **—**
- `Source/T66/UI/Style/T66CasinoVendorTabReferenceLayout.generated.h` (35 lines): **—**
- `Source/T66/UI/Style/T66CollectorOverlayReferenceLayout.generated.h` (35 lines): **—**
- `Source/T66/UI/Style/T66CrateOverlayReferenceLayout.generated.h` (35 lines): **—**
- `Source/T66/UI/Style/T66FlatStyle.cpp` (1515 lines): **—**
- `Source/T66/UI/Style/T66FlatStyle.h` (422 lines): **—**
- `Source/T66/UI/Style/T66FlatWidgetMetadata.h` (47 lines): **—**
- `Source/T66/UI/Style/T66GameplayHUDFullMapReferenceLayout.generated.h` (35 lines): **—**
- `Source/T66/UI/Style/T66GameplayHUDInventoryInspectReferenceLayout.generated.h` (35 lines): **—**
- `Source/T66/UI/Style/T66GameplayHUDReferenceLayout.generated.h` (35 lines): **—**
- `Source/T66/UI/Style/T66HeroSelectionReferenceLayout.generated.h` (53 lines): **—**
- `Source/T66/UI/Style/T66IdolAltarOverlayReferenceLayout.generated.h` (35 lines): **—**
- `Source/T66/UI/Style/T66LabOverlayReferenceLayout.generated.h` (35 lines): **—**
- `Source/T66/UI/Style/T66MainMenuReferenceLayout.generated.h` (84 lines): **—**
- `Source/T66/UI/Style/T66OverlayChromeStyle.cpp` (123 lines): **—**
- `Source/T66/UI/Style/T66OverlayChromeStyle.h` (101 lines): **—**
- `Source/T66/UI/Style/T66ReferenceLayout.h` (96 lines): **—**
- `Source/T66/UI/Style/T66RuntimeUIBrushAccess.cpp` (469 lines): **SYNC_LOAD, FILE_IO**
- `Source/T66/UI/Style/T66RuntimeUIBrushAccess.h` (51 lines): **—**
- `Source/T66/UI/Style/T66RuntimeUIFontAccess.cpp` (150 lines): **FILE_IO**
- `Source/T66/UI/Style/T66RuntimeUIFontAccess.h` (17 lines): **—**
- `Source/T66/UI/Style/T66RuntimeUIHelpers.h` (14 lines): **—**
- `Source/T66/UI/Style/T66RuntimeUITextureAccess.cpp` (496 lines): **SYNC_LOAD, FILE_IO**
- `Source/T66/UI/Style/T66RuntimeUITextureAccess.h` (33 lines): **—**
- `Source/T66/UI/Style/T66Style.cpp` (3029 lines): **SYNC_LOAD, DYNAMIC_MAT**
- `Source/T66/UI/Style/T66Style.h` (635 lines): **—**
- `Source/T66/UI/T66ArcadePopupWidget.cpp` (38 lines): **—**
- `Source/T66/UI/T66ArcadePopupWidget.h` (35 lines): **—**
- `Source/T66/UI/T66ArcadeSelectionWidget.cpp` (565 lines): **—**
- `Source/T66/UI/T66ArcadeSelectionWidget.h` (37 lines): **—**
- `Source/T66/UI/T66CasinoOverlayShared.h` (372 lines): **—**
- `Source/T66/UI/T66CasinoOverlayWidget.cpp` (965 lines): **—**
- `Source/T66/UI/T66CasinoOverlayWidget.h` (127 lines): **—**
- `Source/T66/UI/T66CasinoShopTabWidget.cpp` (2519 lines): **—**
- `Source/T66/UI/T66CasinoShopTabWidget.h` (178 lines): **—**
- `Source/T66/UI/T66CollectorOverlayWidget.cpp` (467 lines): **—**
- `Source/T66/UI/T66CollectorOverlayWidget.h` (44 lines): **—**
- `Source/T66/UI/T66CowardicePromptWidget.cpp` (164 lines): **—**
- `Source/T66/UI/T66CowardicePromptWidget.h` (34 lines): **—**
- `Source/T66/UI/T66CrateOverlayWidget.cpp` (479 lines): **—**
- `Source/T66/UI/T66CrateOverlayWidget.h` (76 lines): **—**
- `Source/T66/UI/T66EnemyLockWidget.cpp` (113 lines): **—**
- `Source/T66/UI/T66EnemyLockWidget.h` (18 lines): **—**
- `Source/T66/UI/T66FloatingCombatTextWidget.cpp` (162 lines): **—**
- `Source/T66/UI/T66FloatingCombatTextWidget.h` (37 lines): **—**
- `Source/T66/UI/T66FrontendTopBarWidget.cpp` (1415 lines): **TICK_ACTOR, FILE_IO**
- `Source/T66/UI/T66FrontendTopBarWidget.h` (81 lines): **—**
- `Source/T66/UI/T66FrontendUIRootWidget.cpp` (328 lines): **SYNC_LOAD**
- `Source/T66/UI/T66FrontendUIRootWidget.h` (81 lines): **—**
- `Source/T66/UI/T66FrontendVideoCatalog.cpp` (356 lines): **FILE_IO**
- `Source/T66/UI/T66FrontendVideoCatalog.h` (21 lines): **—**
- `Source/T66/UI/T66FrontendVideoPlayer.cpp` (263 lines): **FILE_IO, NEW_OBJECT**
- `Source/T66/UI/T66FrontendVideoPlayer.h` (57 lines): **—**
- `Source/T66/UI/T66GamblerOverlayWidget.cpp` (379 lines): **—**
- `Source/T66/UI/T66GamblerOverlayWidget.h` (358 lines): **—**
- `Source/T66/UI/T66GameplayHUDWidget.h` (426 lines): **ITERATOR, WEBVIEW**
- `Source/T66/UI/T66GoldMinerArcadeWidget.cpp` (1083 lines): **—**
- `Source/T66/UI/T66GoldMinerArcadeWidget.h` (137 lines): **—**
- `Source/T66/UI/T66IdolAltarOverlayWidget.cpp` (1035 lines): **—**
- `Source/T66/UI/T66IdolAltarOverlayWidget.h` (70 lines): **—**
- `Source/T66/UI/T66ItemCardTextUtils.cpp` (143 lines): **—**
- `Source/T66/UI/T66ItemCardTextUtils.h` (22 lines): **—**
- `Source/T66/UI/T66LabOverlayWidget.cpp` (404 lines): **—**
- `Source/T66/UI/T66LabOverlayWidget.h` (55 lines): **—**
- `Source/T66/UI/T66LoadingScreenWidget.cpp` (67 lines): **—**
- `Source/T66/UI/T66LoadingScreenWidget.h` (31 lines): **—**
- `Source/T66/UI/T66QuickArcadeWidget.cpp` (1010 lines): **—**
- `Source/T66/UI/T66QuickArcadeWidget.h` (136 lines): **—**
- `Source/T66/UI/T66ScreenBase.cpp` (239 lines): **—**
- `Source/T66/UI/T66ScreenBase.h` (153 lines): **—**
- `Source/T66/UI/T66SlateTextureHelpers.cpp` (113 lines): **—**
- `Source/T66/UI/T66SlateTextureHelpers.h` (55 lines): **SYNC_LOAD**
- `Source/T66/UI/T66StatsPanelSlate.cpp` (1130 lines): **—**
- `Source/T66/UI/T66StatsPanelSlate.h` (74 lines): **—**
- `Source/T66/UI/T66TemporaryBuffUIUtils.cpp` (190 lines): **—**
- `Source/T66/UI/T66TemporaryBuffUIUtils.h` (21 lines): **—**
- `Source/T66/UI/T66TopwarArcadeWidget.cpp` (768 lines): **—**
- `Source/T66/UI/T66TopwarArcadeWidget.h` (90 lines): **—**
- `Source/T66/UI/T66UIDumpCommands.cpp` (171 lines): **—**
- `Source/T66/UI/T66UIManager.cpp` (771 lines): **—**
- `Source/T66/UI/T66UIManager.h` (195 lines): **—**
- `Source/T66/UI/T66UITween.h` (67 lines): **—**
- `Source/T66/UI/T66UITypes.h` (70 lines): **—**
- `Source/T66/UI/T66WhackAMoleArcadeWidget.cpp` (1187 lines): **—**
- `Source/T66/UI/T66WhackAMoleArcadeWidget.h` (166 lines): **—**
- `Source/T66/UI/T66WidgetDumpTargets.cpp` (603 lines): **ITERATOR**
- `Source/T66/UI/T66WidgetDumpTargets.h` (24 lines): **—**
- `Source/T66/UI/T66WidgetTreeWalker.cpp` (451 lines): **FILE_IO**
- `Source/T66/UI/T66WidgetTreeWalker.h` (19 lines): **—**
## Appendix B — largest .cpp files by line count (maintainability / compile-time signal)

Top 25 (approximate line counts):

- `Source/T66/Gameplay/T66TowerMapTerrain.cpp` — **5873** lines
- `Source/T66/UI/Screens/T66AccountStatusScreen.cpp` — **5160** lines
- `Source/T66/UI/Screens/T66RunSummaryScreen.cpp` — **4164** lines
- `Source/T66/UI/Screens/T66MainMenuScreen.cpp` — **3981** lines
- `Source/T66/UI/Components/T66LeaderboardPanel.cpp` — **3287** lines
- `Source/T66/UI/Screens/T66PowerUpScreen.cpp` — **3082** lines
- `Source/T66/UI/Style/T66Style.cpp` — **3029** lines
- `Source/T66/Gameplay/T66MainMapTerrain.cpp` — **2679** lines
- `Source/T66/Gameplay/T66CombatComponent.cpp` — **2661** lines
- `Source/T66/UI/Screens/T66ChallengesScreen.cpp` — **2565** lines
- `Source/T66/UI/T66CasinoShopTabWidget.cpp` — **2519** lines
- `Source/T66/Core/T66SessionSubsystem.cpp` — **2479** lines
- `Source/T66/UI/HUD/T66GameplayHUDWidget_Build.cpp` — **2425** lines
- `Source/T66/Gameplay/GameMode/T66GameMode_WorldInteractables.cpp` — **2389** lines
- `Source/T66/UI/Screens/T66AchievementsScreen.cpp` — **2364** lines
- `Source/T66/Core/T66LeaderboardSubsystem.cpp` — **2324** lines
- `Source/T66/Core/T66LocalizationSubsystem.cpp` — **2300** lines
- `Source/T66/UI/Gambler/T66GamblerOverlayWidget_Build.cpp` — **2186** lines
- `Source/T66/UI/Components/T66FlatLeaderboardPanel.cpp` — **2011** lines
- `Source/T66/Core/T66RetroFXSubsystem.cpp` — **1906** lines
- `Source/T66/Core/T66GameInstance.cpp` — **1840** lines
- `Source/T66/Gameplay/T66PlayerController_Overlays.cpp` — **1760** lines
- `Source/T66/Gameplay/T66GameMode.cpp` — **1752** lines
- `Source/T66/Gameplay/T66EnemyBase.cpp` — **1743** lines
- `Source/T66/Core/T66AchievementsSubsystem.cpp` — **1643** lines

## Appendix C — Per-file numeric pass (all 524 files)

This section was generated by a full-tree read of every Source/T66 *.cpp / *.h file (same inventory as Appendix A). Each row is **one file**.

### Column legend

| Col | Meaning |
|-----|---------|
| L | Line count (approx) |
| trace | Occurrences of line/sweep/overlap query APIs |
| sync | LoadSynchronous / LoadObject / StaticLoadObject occurrences |
| iter | TActorIterator / TObjectIterator occurrences |
| http | Substrings matching HTTP client setup (CreateRequest, etc.) — may include non-runtime text |
| io | FFileHelper / IFileManager references |
| niag | Niagara-related identifiers |
| newo | NewObject< occurrences |
| dmat | Dynamic material instance creation |
| timers | SetTimer / SetTimerForNextTick calls |
| UE_LOG | `UE_LOG(` occurrences |
| tick_fn | AT66*::Tick or UT66*::NativeTick definitions |
| PT_ref | `PrimaryActorTick.` references (includes `bCanEverTick = false`) |
| PT→T | `bCanEverTick = true` assignments |
| PT→F | `bCanEverTick = false` assignments |
| playTick | PlayerTick override definitions |
| UCLASS | `UCLASS(` macro lines |
| UFUNCTION | `UFUNCTION(` macro lines |

| File | L | trace | sync | iter | http | io | niag | newo | dmat | timers | UE_LOG | tick_fn | PT_ref | PT→T | PT→F | playTick | UCLASS | UFUNCTION |
|------|---:|------:|-----:|-----:|-----:|---:|-----:|-----:|-----:|-------:|-------:|--------:|-------:|-----:|-----:|---------:|-------:|----------:|
| `Source/T66/Core/Backend/T66BackendAccountApi.cpp` | 473 | 0 | 0 | 0 | 16 | 0 | 0 | 0 | 0 | 0 | 7 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/Backend/T66BackendDailyClimbJson.cpp` | 130 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/Backend/T66BackendDailyClimbJson.h` | 14 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/Backend/T66BackendLeaderboardApi.cpp` | 806 | 0 | 0 | 0 | 16 | 0 | 0 | 0 | 0 | 0 | 12 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/Backend/T66BackendPartyApi.cpp` | 527 | 0 | 0 | 0 | 9 | 2 | 0 | 0 | 0 | 0 | 13 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/Backend/T66BackendPrivate.h` | 126 | 0 | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/Backend/T66BackendRunApi.cpp` | 627 | 0 | 0 | 0 | 8 | 0 | 0 | 0 | 0 | 0 | 20 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/Backend/T66BackendRunSerializer.cpp` | 573 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/Backend/T66BackendRunSerializer.h` | 26 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/Backend/T66BackendRunSummaryParser.cpp` | 289 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/Backend/T66BackendRunSummaryParser.h` | 15 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/Backend/T66BackendSubsystem.cpp` | 434 | 0 | 0 | 0 | 7 | 0 | 0 | 1 | 0 | 0 | 4 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/PlayerExperience/T66PlayerExperienceSubSystem_Scoring.cpp` | 30 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/PlayerExperience/T66PlayerExperienceSubSystem_Spawning.cpp` | 44 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/PlayerExperience/T66PlayerExperienceSubSystem_Totems.cpp` | 43 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/PlayerExperience/T66PlayerExperienceTypes.h` | 157 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/RunState/T66RunStateSubsystem.cpp` | 697 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/RunState/T66RunStateSubsystem_AntiCheat.cpp` | 566 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/RunState/T66RunStateSubsystem_Combat.cpp` | 916 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/RunState/T66RunStateSubsystem_EconomyInventory.cpp` | 1003 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/RunState/T66RunStateSubsystem_Idols.cpp` | 279 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/RunState/T66RunStateSubsystem_Private.h` | 508 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/RunState/T66RunStateSubsystem_ScoreTelemetry.cpp` | 121 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/RunState/T66RunStateSubsystem_Snapshot.cpp` | 322 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/RunState/T66RunStateSubsystem_Stats.cpp` | 909 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/RunState/T66RunStateSubsystem_TimersBoss.cpp` | 526 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66AchievementsSubsystem.cpp` | 1643 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 0 | 12 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66AchievementsSubsystem.h` | 400 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 70 |
| `Source/T66/Core/T66ActorRegistrySubsystem.cpp` | 154 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 14 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66ActorRegistrySubsystem.h` | 72 | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Core/T66AudioSubsystem.cpp` | 567 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 9 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66AudioSubsystem.h` | 88 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 1 |
| `Source/T66/Core/T66AudioTypes.h` | 63 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66BackendSubsystem.h` | 616 | 0 | 0 | 0 | 5 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 21 |
| `Source/T66/Core/T66BuffSaveGame.h` | 126 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Core/T66BuffSubsystem.cpp` | 1312 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 0 | 12 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66BuffSubsystem.h` | 219 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 39 |
| `Source/T66/Core/T66CharacterVisualSubsystem.cpp` | 1628 | 0 | 9 | 0 | 0 | 0 | 0 | 0 | 3 | 0 | 26 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66CharacterVisualSubsystem.h` | 137 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 7 |
| `Source/T66/Core/T66CommunityContentSaveGame.h` | 25 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Core/T66CommunityContentSubsystem.cpp` | 1206 | 0 | 0 | 0 | 10 | 0 | 0 | 2 | 0 | 0 | 6 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66CommunityContentSubsystem.h` | 96 | 0 | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Core/T66CommunityContentTypes.h` | 264 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66CompanionUnlockSaveGame.h` | 28 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Core/T66CompanionUnlockSubsystem.cpp` | 104 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66CompanionUnlockSubsystem.h` | 50 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 3 |
| `Source/T66/Core/T66CsvUtil.h` | 203 | 0 | 0 | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66DailyClimbTypes.h` | 121 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66DamageLogSubsystem.cpp` | 125 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66DamageLogSubsystem.h` | 74 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 6 |
| `Source/T66/Core/T66EnemyPoolSubsystem.cpp` | 84 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 4 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66EnemyPoolSubsystem.h` | 60 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Core/T66FloatingCombatTextPoolSubsystem.cpp` | 249 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66FloatingCombatTextPoolSubsystem.h` | 61 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Core/T66FloatingCombatTextSubsystem.cpp` | 73 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66FloatingCombatTextSubsystem.h` | 68 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 3 |
| `Source/T66/Core/T66GameContentSettings.cpp` | 4 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66GameContentSettings.h` | 42 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Core/T66GameInstance.cpp` | 1840 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 2 | 16 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66GameInstance.h` | 737 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 54 |
| `Source/T66/Core/T66GameplayLayout.h` | 146 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66HeroSpeedSubsystem.cpp` | 22 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66HeroSpeedSubsystem.h` | 51 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 5 |
| `Source/T66/Core/T66IdolManagerSubsystem.cpp` | 531 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66IdolManagerSubsystem.h` | 99 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Core/T66InteractionPromptSubsystem.cpp` | 220 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66InteractionPromptSubsystem.h` | 42 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Core/T66LagTrackerSubsystem.cpp` | 415 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 4 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66LagTrackerSubsystem.h` | 134 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 1 |
| `Source/T66/Core/T66LeaderboardPacingUtils.cpp` | 115 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66LeaderboardPacingUtils.h` | 17 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66LeaderboardRunSummarySaveGame.cpp` | 5 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66LeaderboardRunSummarySaveGame.h` | 299 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Core/T66LeaderboardSubsystem.cpp` | 2324 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 49 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66LeaderboardSubsystem.h` | 322 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 22 |
| `Source/T66/Core/T66LegacyRuntimeDataAccess.cpp` | 79 | 0 | 0 | 0 | 0 | 3 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66LegacyRuntimeDataAccess.h` | 43 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66LegacyRuntimeTextureAccess.cpp` | 71 | 0 | 0 | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 3 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66LegacyRuntimeTextureAccess.h` | 20 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66LocalLeaderboardSaveGame.cpp` | 5 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66LocalLeaderboardSaveGame.h` | 243 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Core/T66LocalizationSubsystem.cpp` | 2300 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66LocalizationSubsystem.h` | 1104 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 314 |
| `Source/T66/Core/T66MediaViewerSubsystem.cpp` | 364 | 0 | 0 | 0 | 0 | 2 | 0 | 0 | 0 | 0 | 3 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66MediaViewerSubsystem.h` | 102 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 6 |
| `Source/T66/Core/T66MusicSubsystem.cpp` | 823 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 7 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66MusicSubsystem.h` | 161 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 7 |
| `Source/T66/Core/T66PartySubsystem.cpp` | 479 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66PartySubsystem.h` | 127 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 10 |
| `Source/T66/Core/T66PixelVFXSubsystem.cpp` | 291 | 0 | 0 | 0 | 0 | 0 | 24 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66PixelVFXSubsystem.h` | 77 | 0 | 0 | 0 | 0 | 0 | 8 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Core/T66Pixelation.cpp` | 52 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66PixelationSubsystem.cpp` | 215 | 0 | 0 | 2 | 0 | 0 | 0 | 0 | 1 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66PixelationSubsystem.h` | 69 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 5 |
| `Source/T66/Core/T66PlayerExperienceSubSystem.cpp` | 241 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 6 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66PlayerExperienceSubSystem.h` | 186 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 6 |
| `Source/T66/Core/T66PlayerSettingsSaveGame.cpp` | 5 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66PlayerSettingsSaveGame.h` | 240 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Core/T66PlayerSettingsSubsystem.cpp` | 1169 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 5 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66PlayerSettingsSubsystem.h` | 311 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 75 |
| `Source/T66/Core/T66ProfileSaveGame.h` | 189 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Core/T66Rarity.cpp` | 86 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66Rarity.h` | 40 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66ReleaseVariantSubsystem.cpp` | 227 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66ReleaseVariantSubsystem.h` | 69 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 2 | 4 |
| `Source/T66/Core/T66RetroFXSettings.h` | 235 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66RetroFXSubsystem.cpp` | 1906 | 0 | 0 | 2 | 0 | 0 | 0 | 0 | 2 | 0 | 21 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66RetroFXSubsystem.h` | 213 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Core/T66RngSubsystem.cpp` | 250 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66RngSubsystem.h` | 123 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 6 |
| `Source/T66/Core/T66RngTuningConfig.cpp` | 74 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66RngTuningConfig.h` | 107 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66RunIntegritySubsystem.cpp` | 322 | 0 | 0 | 0 | 0 | 3 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66RunIntegritySubsystem.h` | 40 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Core/T66RunIntegrityTypes.h` | 97 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66RunSaveGame.h` | 678 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Core/T66RunStateSubsystem.h` | 1898 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 233 |
| `Source/T66/Core/T66RuntimePlatformSubsystem.cpp` | 130 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66RuntimePlatformSubsystem.h` | 67 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 2 | 6 |
| `Source/T66/Core/T66SaveIndex.h` | 43 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Core/T66SaveMigration.h` | 43 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66SaveSubsystem.cpp` | 268 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 0 | 4 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66SaveSubsystem.h` | 52 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 6 |
| `Source/T66/Core/T66SessionSubsystem.cpp` | 2479 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 0 | 23 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66SessionSubsystem.h` | 181 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Core/T66SkillRatingSubsystem.cpp` | 128 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66SkillRatingSubsystem.h` | 82 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 1 |
| `Source/T66/Core/T66SkinSubsystem.cpp` | 356 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66SkinSubsystem.h` | 118 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 20 |
| `Source/T66/Core/T66StageProgressionSubsystem.cpp` | 142 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66StageProgressionSubsystem.h` | 109 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 4 |
| `Source/T66/Core/T66StageProgressionTuningConfig.cpp` | 89 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66StageProgressionTuningConfig.h` | 48 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66SteamHelper.cpp` | 678 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 16 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66SteamHelper.h` | 157 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 15 |
| `Source/T66/Core/T66TrapSubsystem.cpp` | 1015 | 0 | 0 | 0 | 0 | 0 | 6 | 0 | 0 | 0 | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66TrapSubsystem.h` | 51 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Core/T66TrapTuningConfig.cpp` | 379 | 0 | 0 | 0 | 0 | 0 | 9 | 0 | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66TrapTuningConfig.h` | 224 | 0 | 0 | 0 | 0 | 0 | 7 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66UITexturePoolSubsystem.cpp` | 291 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66UITexturePoolSubsystem.h` | 94 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Core/T66WeaponManagerSubsystem.cpp` | 208 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66WeaponManagerSubsystem.h` | 58 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Core/T66WebImageCache.cpp` | 196 | 0 | 0 | 0 | 4 | 0 | 0 | 0 | 0 | 0 | 4 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66WebImageCache.h` | 52 | 0 | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Core/T66WebView2Host.cpp` | 1336 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 21 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Core/T66WebView2Host.h` | 59 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Data/T66DataTypes.h` | 2328 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/Enemies/Projectiles/T66EnemyProjectileBase.cpp` | 81 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 1 | 0 | 0 | 0 |
| `Source/T66/Gameplay/Enemies/Projectiles/T66EnemyProjectileBase.h` | 41 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 1 |
| `Source/T66/Gameplay/Enemies/Projectiles/T66SpitProjectile.cpp` | 22 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/Enemies/Projectiles/T66SpitProjectile.h` | 17 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Gameplay/Enemies/T66EnemyFamilyResolver.cpp` | 115 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/Enemies/T66EnemyFamilyResolver.h` | 19 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/Enemies/T66EnemyFamilyTypes.h` | 17 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/Enemies/T66FlyingEnemy.cpp` | 43 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/Enemies/T66FlyingEnemy.h` | 35 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Gameplay/Enemies/T66MeleeEnemy.cpp` | 15 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/Enemies/T66MeleeEnemy.h` | 17 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Gameplay/Enemies/T66RangedEnemy.cpp` | 112 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/Enemies/T66RangedEnemy.h` | 43 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Gameplay/Enemies/T66RushEnemy.cpp` | 84 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/Enemies/T66RushEnemy.h` | 38 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Gameplay/GameMode/T66GameModePrivate.h` | 178 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/GameMode/T66GameMode_Bootstrap.cpp` | 769 | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 10 | 18 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/GameMode/T66GameMode_BossFlow.cpp` | 1266 | 8 | 3 | 1 | 0 | 0 | 0 | 3 | 1 | 0 | 11 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/GameMode/T66GameMode_Lab.cpp` | 287 | 0 | 0 | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/GameMode/T66GameMode_MainMap.cpp` | 1042 | 2 | 0 | 6 | 0 | 0 | 0 | 0 | 0 | 0 | 11 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/GameMode/T66GameMode_Spawning.cpp` | 786 | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 13 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/GameMode/T66GameMode_Tower.cpp` | 506 | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 3 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/GameMode/T66GameMode_WorldInteractables.cpp` | 2389 | 12 | 1 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 7 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/Movement/T66HeroMovementComponent.cpp` | 236 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/Movement/T66HeroMovementComponent.h` | 68 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Gameplay/Movement/T66HeroMovementTypes.h` | 78 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66ArcadeAmplifierPickup.cpp` | 147 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 1 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66ArcadeAmplifierPickup.h` | 51 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 2 |
| `Source/T66/Gameplay/T66ArcadeInteractableBase.cpp` | 681 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66ArcadeInteractableBase.h` | 60 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Gameplay/T66ArcadeInteractableTypes.h` | 216 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66ArcadeMachineInteractable.cpp` | 241 | 1 | 1 | 0 | 0 | 0 | 0 | 2 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66ArcadeMachineInteractable.h` | 52 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 2 |
| `Source/T66/Gameplay/T66ArcadeTruckInteractable.cpp` | 213 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66ArcadeTruckInteractable.h` | 44 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Gameplay/T66ArthurSwordVisuals.cpp` | 21 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66ArthurSwordVisuals.h` | 14 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66ArthurUltimateSword.cpp` | 101 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 1 | 1 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66ArthurUltimateSword.h` | 46 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Gameplay/T66BossAttackTypes.h` | 17 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66BossBase.cpp` | 1593 | 1 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 3 | 0 | 1 | 1 | 1 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66BossBase.h` | 223 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 1 |
| `Source/T66/Gameplay/T66BossGate.cpp` | 134 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 1 | 1 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66BossGate.h` | 46 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 1 |
| `Source/T66/Gameplay/T66BossGroundAOE.cpp` | 352 | 2 | 0 | 0 | 0 | 0 | 9 | 0 | 0 | 2 | 0 | 1 | 1 | 1 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66BossGroundAOE.h` | 75 | 0 | 0 | 0 | 0 | 0 | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Gameplay/T66BossPartTypes.h` | 58 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66BossProjectile.cpp` | 385 | 0 | 0 | 0 | 0 | 0 | 11 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 1 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66BossProjectile.h` | 66 | 0 | 0 | 0 | 0 | 0 | 6 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 1 |
| `Source/T66/Gameplay/T66ChestInteractable.cpp` | 144 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66ChestInteractable.h` | 31 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Gameplay/T66ChestMimicEnemy.cpp` | 43 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 1 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66ChestMimicEnemy.h` | 25 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Gameplay/T66CombatComponent.cpp` | 2661 | 3 | 1 | 2 | 0 | 0 | 27 | 1 | 0 | 2 | 7 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66CombatComponent.h` | 229 | 0 | 0 | 0 | 0 | 0 | 12 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 4 |
| `Source/T66/Gameplay/T66CombatHitZoneComponent.cpp` | 33 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66CombatHitZoneComponent.h` | 36 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 2 |
| `Source/T66/Gameplay/T66CombatShared.cpp` | 119 | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66CombatShared.h` | 21 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66CombatTargetTypes.h` | 64 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66CombatVFX.cpp` | 1522 | 0 | 0 | 0 | 0 | 0 | 114 | 0 | 0 | 1 | 19 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66CompanionBase.cpp` | 430 | 3 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 0 | 1 | 1 | 1 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66CompanionBase.h` | 140 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 6 |
| `Source/T66/Gameplay/T66CowardiceGate.cpp` | 120 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 1 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66CowardiceGate.h` | 46 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Gameplay/T66CrateInteractable.cpp` | 44 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66CrateInteractable.h` | 25 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Gameplay/T66DifficultyTotem.cpp` | 187 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 1 | 0 | 1 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66DifficultyTotem.h` | 42 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Gameplay/T66EnemyAIController.cpp` | 27 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 1 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66EnemyAIController.h` | 28 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Gameplay/T66EnemyBase.cpp` | 1743 | 0 | 0 | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 6 | 1 | 1 | 1 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66EnemyBase.h` | 333 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 6 |
| `Source/T66/Gameplay/T66EnemyDirector.cpp` | 1310 | 1 | 0 | 2 | 0 | 0 | 0 | 0 | 0 | 4 | 3 | 0 | 1 | 0 | 1 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66EnemyDirector.h` | 163 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 5 |
| `Source/T66/Gameplay/T66FloatingCombatTextActor.cpp` | 80 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 1 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66FloatingCombatTextActor.h` | 33 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Gameplay/T66FountainInteractable.cpp` | 78 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66FountainInteractable.h` | 29 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Gameplay/T66FrontendGameMode.cpp` | 95 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66FrontendGameMode.h` | 28 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 1 |
| `Source/T66/Gameplay/T66GalleryDisplayActor.cpp` | 218 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 1 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66GalleryDisplayActor.h` | 70 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 2 |
| `Source/T66/Gameplay/T66GamblerBoss.cpp` | 173 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66GamblerBoss.h` | 22 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Gameplay/T66GamblerNPC.cpp` | 78 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66GamblerNPC.h` | 33 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Gameplay/T66GameMode.cpp` | 1752 | 6 | 1 | 6 | 0 | 0 | 0 | 0 | 0 | 0 | 5 | 1 | 1 | 1 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66GameMode.h` | 386 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 12 |
| `Source/T66/Gameplay/T66GoblinThiefEnemy.cpp` | 135 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66GoblinThiefEnemy.h` | 43 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 1 |
| `Source/T66/Gameplay/T66HeroBase.cpp` | 1196 | 3 | 1 | 0 | 0 | 0 | 0 | 0 | 1 | 1 | 10 | 1 | 1 | 1 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66HeroBase.h` | 310 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 15 |
| `Source/T66/Gameplay/T66HeroOneAttackVFX.cpp` | 439 | 0 | 2 | 0 | 0 | 0 | 0 | 0 | 2 | 0 | 8 | 1 | 2 | 1 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66HeroOneAttackVFX.h` | 90 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Gameplay/T66HeroPlagueCloud.cpp` | 164 | 1 | 0 | 0 | 0 | 0 | 1 | 0 | 0 | 2 | 0 | 1 | 1 | 1 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66HeroPlagueCloud.h` | 61 | 0 | 0 | 0 | 0 | 0 | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Gameplay/T66HeroProjectile.cpp` | 315 | 0 | 0 | 0 | 0 | 0 | 4 | 0 | 0 | 0 | 0 | 1 | 1 | 1 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66HeroProjectile.h` | 77 | 0 | 0 | 0 | 0 | 0 | 4 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 1 |
| `Source/T66/Gameplay/T66HouseNPCBase.cpp` | 440 | 3 | 0 | 1 | 0 | 0 | 0 | 0 | 2 | 0 | 0 | 1 | 2 | 1 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66HouseNPCBase.h` | 150 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 6 |
| `Source/T66/Gameplay/T66IdolAltar.cpp` | 179 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 1 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66IdolAltar.h` | 125 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Gameplay/T66LabCollector.cpp` | 20 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66LabCollector.h` | 24 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Gameplay/T66LavaPatch.cpp` | 520 | 2 | 3 | 0 | 0 | 0 | 0 | 0 | 1 | 1 | 0 | 1 | 3 | 1 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66LavaPatch.h` | 154 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 2 |
| `Source/T66/Gameplay/T66LavaShared.h` | 83 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66LoanShark.cpp` | 213 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 0 | 1 | 1 | 1 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66LoanShark.h` | 54 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 1 |
| `Source/T66/Gameplay/T66LootBagPickup.cpp` | 408 | 0 | 1 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 1 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66LootBagPickup.h` | 115 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 12 |
| `Source/T66/Gameplay/T66MainMapTerrain.cpp` | 2679 | 0 | 9 | 0 | 0 | 0 | 0 | 9 | 1 | 0 | 7 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66MainMapTerrain.h` | 136 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66MainMapTerrainTypes.h` | 30 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66MiasmaBoundary.cpp` | 328 | 0 | 3 | 0 | 0 | 0 | 0 | 0 | 1 | 1 | 1 | 0 | 1 | 0 | 1 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66MiasmaBoundary.h` | 65 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Gameplay/T66MiasmaManager.cpp` | 1077 | 1 | 3 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 2 | 1 | 1 | 1 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66MiasmaManager.h` | 181 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Gameplay/T66MiasmaTile.cpp` | 95 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 0 | 1 | 0 | 1 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66MiasmaTile.h` | 52 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 2 |
| `Source/T66/Gameplay/T66OuroborosNPC.cpp` | 30 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66OuroborosNPC.h` | 24 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Gameplay/T66PilotableTractor.cpp` | 405 | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 1 | 1 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66PilotableTractor.h` | 92 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Gameplay/T66PlayerController.cpp` | 1414 | 3 | 1 | 1 | 0 | 0 | 27 | 0 | 1 | 4 | 10 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66PlayerController.h` | 544 | 0 | 0 | 0 | 0 | 0 | 12 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 16 |
| `Source/T66/Gameplay/T66PlayerController_Combat.cpp` | 1473 | 3 | 0 | 0 | 0 | 0 | 3 | 7 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 0 |
| `Source/T66/Gameplay/T66PlayerController_Frontend.cpp` | 1540 | 0 | 0 | 0 | 0 | 5 | 3 | 1 | 0 | 4 | 26 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66PlayerController_Input.cpp` | 684 | 0 | 0 | 0 | 0 | 0 | 7 | 0 | 0 | 0 | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66PlayerController_Movement.cpp` | 307 | 0 | 0 | 0 | 0 | 0 | 2 | 0 | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66PlayerController_Overlays.cpp` | 1760 | 0 | 0 | 0 | 0 | 1 | 3 | 1 | 0 | 6 | 5 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66PlayerController_ScopedUlt.cpp` | 294 | 1 | 0 | 0 | 0 | 0 | 3 | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66PlayerController_WorldDialogue.cpp` | 279 | 0 | 0 | 0 | 0 | 0 | 3 | 0 | 0 | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66ProceduralLandscapeParams.h` | 159 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66QuickReviveVendingMachine.cpp` | 114 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66QuickReviveVendingMachine.h` | 35 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 1 |
| `Source/T66/Gameplay/T66RecruitableCompanion.cpp` | 137 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 0 | 0 | 1 | 0 | 1 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66RecruitableCompanion.h` | 65 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 1 |
| `Source/T66/Gameplay/T66SaintNPC.cpp` | 24 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66SaintNPC.h` | 21 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Gameplay/T66SessionPlayerState.cpp` | 62 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66SessionPlayerState.h` | 119 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 1 |
| `Source/T66/Gameplay/T66SpawnPlateau.cpp` | 28 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 1 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66SpawnPlateau.h` | 34 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Gameplay/T66StageCatchUpGate.cpp` | 133 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 1 | 0 | 1 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66StageCatchUpGate.h` | 42 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 1 |
| `Source/T66/Gameplay/T66StageEffects.cpp` | 171 | 0 | 1 | 0 | 0 | 0 | 3 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 1 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66StageEffects.h` | 74 | 0 | 0 | 0 | 0 | 0 | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 2 |
| `Source/T66/Gameplay/T66StageGate.cpp` | 145 | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 1 | 0 | 1 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66StageGate.h` | 43 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 1 |
| `Source/T66/Gameplay/T66StageProgressionVisuals.cpp` | 22 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66StageProgressionVisuals.h` | 16 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66StartGate.cpp` | 110 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 1 | 1 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66StartGate.h` | 54 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 1 |
| `Source/T66/Gameplay/T66TerrainThemeAssets.cpp` | 91 | 0 | 4 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66TerrainThemeAssets.h` | 17 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66TowerDescentHole.cpp` | 76 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 1 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66TowerDescentHole.h` | 41 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 2 |
| `Source/T66/Gameplay/T66TowerMapTerrain.cpp` | 5873 | 1 | 0 | 0 | 0 | 0 | 0 | 5 | 0 | 0 | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66TowerMapTerrain.h` | 190 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66TowerThemeVisuals.cpp` | 347 | 0 | 9 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66TowerThemeVisuals.h` | 48 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66TutorialGuideCompanion.cpp` | 179 | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 1 | 1 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66TutorialGuideCompanion.h` | 54 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Gameplay/T66TutorialManager.cpp` | 856 | 2 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 1 | 1 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66TutorialManager.h` | 144 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 9 |
| `Source/T66/Gameplay/T66TutorialPortal.cpp` | 96 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 1 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66TutorialPortal.h` | 43 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 1 |
| `Source/T66/Gameplay/T66TutorialPromptActor.cpp` | 67 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 1 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66TutorialPromptActor.h` | 49 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 2 |
| `Source/T66/Gameplay/T66UniqueDebuffEnemy.cpp` | 155 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66UniqueDebuffEnemy.h` | 39 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Gameplay/T66UniqueDebuffProjectile.cpp` | 142 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 1 | 1 | 1 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66UniqueDebuffProjectile.h` | 54 | 0 | 0 | 0 | 0 | 0 | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 1 |
| `Source/T66/Gameplay/T66VisualUtil.cpp` | 419 | 2 | 7 | 1 | 0 | 0 | 0 | 0 | 2 | 0 | 3 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66VisualUtil.h` | 44 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66WhackAMoleArcadeInteractable.cpp` | 49 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66WhackAMoleArcadeInteractable.h` | 17 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Gameplay/T66WorldInteractableBase.cpp` | 482 | 0 | 4 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66WorldInteractableBase.h` | 107 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 2 |
| `Source/T66/Gameplay/T66WorldVisualSetup.cpp` | 226 | 0 | 0 | 5 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/T66WorldVisualSetup.h` | 14 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/Traps/T66FloorFlameTrap.cpp` | 514 | 0 | 0 | 0 | 0 | 0 | 22 | 0 | 0 | 4 | 0 | 1 | 2 | 1 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/Traps/T66FloorFlameTrap.h` | 108 | 0 | 0 | 0 | 0 | 0 | 6 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Gameplay/Traps/T66FloorSpikePatchTrap.cpp` | 556 | 0 | 0 | 0 | 0 | 0 | 9 | 0 | 0 | 6 | 0 | 1 | 2 | 1 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/Traps/T66FloorSpikePatchTrap.h` | 122 | 0 | 0 | 0 | 0 | 0 | 3 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Gameplay/Traps/T66TrapArrowProjectile.cpp` | 200 | 0 | 0 | 0 | 0 | 0 | 10 | 0 | 0 | 0 | 0 | 1 | 1 | 1 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/Traps/T66TrapArrowProjectile.h` | 71 | 0 | 0 | 0 | 0 | 0 | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 1 |
| `Source/T66/Gameplay/Traps/T66TrapBase.cpp` | 209 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 1 | 0 | 0 | 0 |
| `Source/T66/Gameplay/Traps/T66TrapBase.h` | 118 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/Gameplay/Traps/T66TrapDamageUtils.cpp` | 76 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/Traps/T66TrapDamageUtils.h` | 18 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/Traps/T66TrapPressurePlate.cpp` | 229 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 2 | 0 | 0 | 1 | 0 | 1 | 0 | 0 | 0 |
| `Source/T66/Gameplay/Traps/T66TrapPressurePlate.h` | 89 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 2 |
| `Source/T66/Gameplay/Traps/T66WallArrowTrap.cpp` | 345 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/Gameplay/Traps/T66WallArrowTrap.h` | 82 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/T66.cpp` | 26 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/T66.h` | 20 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Components/T66FlatLeaderboardPanel.cpp` | 2011 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Components/T66FlatLeaderboardPanel.h` | 170 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Components/T66LeaderboardPanel.cpp` | 3287 | 0 | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Components/T66LeaderboardPanel.h` | 200 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Components/T66MinigameMenuLayout.cpp` | 628 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Components/T66MinigameMenuLayout.h` | 131 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Gambler/T66GamblerOverlayWidget_BlackJack.cpp` | 637 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 5 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Gambler/T66GamblerOverlayWidget_Build.cpp` | 2186 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Gambler/T66GamblerOverlayWidget_ChanceGames.cpp` | 505 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 3 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Gambler/T66GamblerOverlayWidget_Cheat.cpp` | 568 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 4 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Gambler/T66GamblerOverlayWidget_CoinGames.cpp` | 244 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Gambler/T66GamblerOverlayWidget_Economy.cpp` | 968 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/HUD/T66GameplayHUDWidget.cpp` | 419 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 2 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/HUD/T66GameplayHUDWidget_Build.cpp` | 2425 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/HUD/T66GameplayHUDWidget_Map.cpp` | 520 | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/HUD/T66GameplayHUDWidget_Overlays.cpp` | 538 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 2 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/HUD/T66GameplayHUDWidget_Presentations.cpp` | 94 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/HUD/T66GameplayHUDWidget_Private.h` | 2408 | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/HUD/T66GameplayHUDWidget_Refresh.cpp` | 1246 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/HUD/T66HUDPresentationController.cpp` | 670 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/HUD/T66HUDPresentationController.h` | 80 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Screens/HeroSelection/T66HeroSelectionPreviewController.cpp` | 335 | 0 | 0 | 0 | 0 | 0 | 0 | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Screens/HeroSelection/T66HeroSelectionPreviewController.h` | 90 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/UI/Screens/HeroSelection/T66HeroSelectionScreen_Build.cpp` | 765 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Screens/HeroSelection/T66HeroSelectionScreen_Party.cpp` | 331 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Screens/HeroSelection/T66HeroSelectionScreen_Preview.cpp` | 642 | 0 | 0 | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Screens/HeroSelection/T66HeroSelectionScreen_Private.h` | 1112 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Screens/HeroSelection/T66HeroSelectionScreen_RetroFX.cpp` | 468 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 5 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Screens/HeroSelection/T66HeroSelectionScreen_Skins.cpp` | 668 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Screens/HeroSelection/T66HeroSelectionScreen_Stats.cpp` | 412 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Screens/Settings/T66SettingsScreen_Audio.cpp` | 431 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Screens/Settings/T66SettingsScreen_Build.cpp` | 503 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Screens/Settings/T66SettingsScreen_Controls.cpp` | 808 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Screens/Settings/T66SettingsScreen_Crashing.cpp` | 95 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Screens/Settings/T66SettingsScreen_Gameplay.cpp` | 823 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Screens/Settings/T66SettingsScreen_Graphics.cpp` | 1046 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Screens/Settings/T66SettingsScreen_HUD.cpp` | 401 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Screens/Settings/T66SettingsScreen_Private.h` | 729 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Screens/Settings/T66SettingsScreen_Rebinding.cpp` | 286 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Screens/Settings/T66SettingsScreen_RetroFX.cpp` | 447 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 6 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Screens/T66AccountStatusScreen.cpp` | 5160 | 0 | 0 | 0 | 0 | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Screens/T66AccountStatusScreen.h` | 98 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 1 |
| `Source/T66/UI/Screens/T66AchievementsScreen.cpp` | 2364 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 4 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Screens/T66AchievementsScreen.h` | 70 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 3 |
| `Source/T66/UI/Screens/T66ChallengesScreen.cpp` | 2565 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Screens/T66ChallengesScreen.h` | 103 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/UI/Screens/T66CompanionGridScreen.cpp` | 303 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Screens/T66CompanionGridScreen.h` | 37 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp` | 1466 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Screens/T66CompanionSelectionScreen.h` | 169 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 13 |
| `Source/T66/UI/Screens/T66DailyClimbScreen.cpp` | 1513 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Screens/T66DailyClimbScreen.h` | 56 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/UI/Screens/T66HeroGridScreen.cpp` | 249 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Screens/T66HeroGridScreen.h` | 37 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/UI/Screens/T66HeroSelectionScreen.cpp` | 726 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 0 | 12 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Screens/T66HeroSelectionScreen.h` | 316 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 23 |
| `Source/T66/UI/Screens/T66LanguageSelectScreen.cpp` | 366 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Screens/T66LanguageSelectScreen.h` | 51 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 3 |
| `Source/T66/UI/Screens/T66MainMenuScreen.cpp` | 3981 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 0 | 1 | 1 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Screens/T66MainMenuScreen.h` | 236 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 12 |
| `Source/T66/UI/Screens/T66MinigamesScreen.cpp` | 533 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Screens/T66MinigamesScreen.h` | 46 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 2 |
| `Source/T66/UI/Screens/T66PartyInviteModal.cpp` | 496 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 8 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Screens/T66PartyInviteModal.h` | 43 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/UI/Screens/T66PauseMenuScreen.cpp` | 231 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Screens/T66PauseMenuScreen.h` | 56 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 6 |
| `Source/T66/UI/Screens/T66PlayerSummaryPickerScreen.cpp` | 340 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Screens/T66PlayerSummaryPickerScreen.h` | 31 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/UI/Screens/T66PowerUpScreen.cpp` | 3082 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Screens/T66PowerUpScreen.h` | 49 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/UI/Screens/T66QuitConfirmationModal.cpp` | 155 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Screens/T66QuitConfirmationModal.h` | 33 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 2 |
| `Source/T66/UI/Screens/T66ReportBugScreen.cpp` | 332 | 0 | 0 | 0 | 0 | 2 | 0 | 0 | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Screens/T66ReportBugScreen.h` | 33 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 2 |
| `Source/T66/UI/Screens/T66RunSummaryScreen.cpp` | 4164 | 0 | 0 | 0 | 0 | 0 | 0 | 2 | 0 | 0 | 15 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Screens/T66RunSummaryScreen.h` | 172 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 3 |
| `Source/T66/UI/Screens/T66SavePreviewScreen.cpp` | 247 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Screens/T66SavePreviewScreen.h` | 34 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 2 |
| `Source/T66/UI/Screens/T66SaveSlotsScreen.cpp` | 1227 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Screens/T66SaveSlotsScreen.h` | 74 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 7 |
| `Source/T66/UI/Screens/T66ScreenSlateHelpers.cpp` | 1315 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Screens/T66ScreenSlateHelpers.h` | 136 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Screens/T66SelectionScreenUtils.h` | 116 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Screens/T66SettingsScreen.cpp` | 228 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Screens/T66SettingsScreen.h` | 235 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 4 |
| `Source/T66/UI/Style/T66ButtonVisuals.cpp` | 19 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Style/T66ButtonVisuals.h` | 114 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Style/T66CasinoAlchemyTabReferenceLayout.generated.h` | 35 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Style/T66CasinoGamblingTabReferenceLayout.generated.h` | 35 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Style/T66CasinoVendorTabReferenceLayout.generated.h` | 35 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Style/T66CollectorOverlayReferenceLayout.generated.h` | 35 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Style/T66CrateOverlayReferenceLayout.generated.h` | 35 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Style/T66FlatStyle.cpp` | 1515 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Style/T66FlatStyle.h` | 422 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Style/T66FlatWidgetMetadata.h` | 47 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Style/T66GameplayHUDFullMapReferenceLayout.generated.h` | 35 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Style/T66GameplayHUDInventoryInspectReferenceLayout.generated.h` | 35 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Style/T66GameplayHUDReferenceLayout.generated.h` | 35 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Style/T66HeroSelectionReferenceLayout.generated.h` | 53 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Style/T66IdolAltarOverlayReferenceLayout.generated.h` | 35 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Style/T66LabOverlayReferenceLayout.generated.h` | 35 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Style/T66MainMenuReferenceLayout.generated.h` | 84 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Style/T66OverlayChromeStyle.cpp` | 123 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Style/T66OverlayChromeStyle.h` | 101 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Style/T66ReferenceLayout.h` | 96 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Style/T66RuntimeUIBrushAccess.cpp` | 469 | 0 | 1 | 0 | 0 | 2 | 0 | 0 | 0 | 0 | 5 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Style/T66RuntimeUIBrushAccess.h` | 51 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Style/T66RuntimeUIFontAccess.cpp` | 150 | 0 | 0 | 0 | 0 | 3 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Style/T66RuntimeUIFontAccess.h` | 17 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Style/T66RuntimeUIHelpers.h` | 14 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Style/T66RuntimeUITextureAccess.cpp` | 496 | 0 | 1 | 0 | 0 | 3 | 0 | 0 | 0 | 0 | 8 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Style/T66RuntimeUITextureAccess.h` | 33 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Style/T66Style.cpp` | 3029 | 0 | 2 | 0 | 0 | 0 | 0 | 0 | 1 | 1 | 5 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/Style/T66Style.h` | 635 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/T66ArcadePopupWidget.cpp` | 38 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/T66ArcadePopupWidget.h` | 35 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/UI/T66ArcadeSelectionWidget.cpp` | 565 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/T66ArcadeSelectionWidget.h` | 37 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/UI/T66CasinoOverlayShared.h` | 372 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/T66CasinoOverlayWidget.cpp` | 965 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/T66CasinoOverlayWidget.h` | 127 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 6 |
| `Source/T66/UI/T66CasinoShopTabWidget.cpp` | 2519 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/T66CasinoShopTabWidget.h` | 178 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 5 |
| `Source/T66/UI/T66CollectorOverlayWidget.cpp` | 467 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/T66CollectorOverlayWidget.h` | 44 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/UI/T66CowardicePromptWidget.cpp` | 164 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/T66CowardicePromptWidget.h` | 34 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/UI/T66CrateOverlayWidget.cpp` | 479 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/T66CrateOverlayWidget.h` | 76 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/UI/T66EnemyLockWidget.cpp` | 113 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/T66EnemyLockWidget.h` | 18 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/UI/T66FloatingCombatTextWidget.cpp` | 162 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/T66FloatingCombatTextWidget.h` | 37 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 2 |
| `Source/T66/UI/T66FrontendTopBarWidget.cpp` | 1415 | 0 | 0 | 0 | 0 | 3 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/T66FrontendTopBarWidget.h` | 81 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/UI/T66FrontendUIRootWidget.cpp` | 328 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/T66FrontendUIRootWidget.h` | 81 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/UI/T66FrontendVideoCatalog.cpp` | 356 | 0 | 0 | 0 | 0 | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/T66FrontendVideoCatalog.h` | 21 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/T66FrontendVideoPlayer.cpp` | 263 | 0 | 0 | 0 | 0 | 2 | 0 | 3 | 0 | 0 | 7 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/T66FrontendVideoPlayer.h` | 57 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 2 |
| `Source/T66/UI/T66GamblerOverlayWidget.cpp` | 379 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/T66GamblerOverlayWidget.h` | 358 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 2 |
| `Source/T66/UI/T66GameplayHUDWidget.h` | 426 | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 13 |
| `Source/T66/UI/T66GoldMinerArcadeWidget.cpp` | 1083 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/T66GoldMinerArcadeWidget.h` | 137 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/UI/T66IdolAltarOverlayWidget.cpp` | 1035 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/T66IdolAltarOverlayWidget.h` | 70 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 1 |
| `Source/T66/UI/T66ItemCardTextUtils.cpp` | 143 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/T66ItemCardTextUtils.h` | 22 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/T66LabOverlayWidget.cpp` | 404 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/T66LabOverlayWidget.h` | 55 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/UI/T66LoadingScreenWidget.cpp` | 67 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/T66LoadingScreenWidget.h` | 31 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 1 |
| `Source/T66/UI/T66QuickArcadeWidget.cpp` | 1010 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/T66QuickArcadeWidget.h` | 136 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/UI/T66ScreenBase.cpp` | 239 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/T66ScreenBase.h` | 153 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 11 |
| `Source/T66/UI/T66SlateTextureHelpers.cpp` | 113 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/T66SlateTextureHelpers.h` | 55 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/T66StatsPanelSlate.cpp` | 1130 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/T66StatsPanelSlate.h` | 74 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/T66TemporaryBuffUIUtils.cpp` | 190 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/T66TemporaryBuffUIUtils.h` | 21 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/T66TopwarArcadeWidget.cpp` | 768 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 3 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/T66TopwarArcadeWidget.h` | 90 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/UI/T66UIDumpCommands.cpp` | 171 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 6 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/T66UIManager.cpp` | 771 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 6 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/T66UIManager.h` | 195 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 14 |
| `Source/T66/UI/T66UITween.h` | 67 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/T66UITypes.h` | 70 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/T66WhackAMoleArcadeWidget.cpp` | 1187 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/T66WhackAMoleArcadeWidget.h` | 166 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| `Source/T66/UI/T66WidgetDumpTargets.cpp` | 603 | 0 | 0 | 4 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/T66WidgetDumpTargets.h` | 24 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/T66WidgetTreeWalker.cpp` | 451 | 0 | 0 | 0 | 0 | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `Source/T66/UI/T66WidgetTreeWalker.h` | 19 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
