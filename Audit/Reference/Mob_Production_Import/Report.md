# Mob Production Import Report

Date: 2026-05-14
Workspace: `C:\UE\T66`

## Status

Production roster migration is complete for authored data, imported assets, schema, stage slots, legacy regular-mob asset deletion, and staged standalone gallery verification.

The staged gameplay smoke proves `GameplayLevel` starts, stage 1 bootstrap completes, the stage 1 boss initializes, and the floor 1 start gallery creates 50 enemy display actors. Main-board enemy wave engagement was not manually walked because the current standalone automation surface has no hook to move the player from the start area into the board and wait for enemy director spawns; this is tracked in `Source/T66/Gameplay/pending_issues_Gameplay.md`.

## Difficulty Themes

| DifficultyID | StageTag / ThemeID |
|---|---|
| Easy | Dungeon |
| Medium | Forest |
| Hard | Ocean |
| VeryHard | Martian |
| Impossible | Hell |

## Import Manifest

Source manifests:

| Source | Produced | Imported | Skipped | Failures |
|---|---:|---:|---:|---:|
| `Model Generation/Production/Roster_v1/AgentA/manifest.csv` | 25 | 25 | 0 | 0 |
| `Model Generation/Production/Roster_v1/AgentB/manifest.csv` | 25 | 25 | 0 | 0 |
| Total | 50 | 50 | 0 | 0 |

Destination convention applied:

| Asset type | Convention |
|---|---|
| Folder | `/Game/Characters/Mobs/<MobName>/` |
| Static mesh | `/Game/Characters/Mobs/<MobName>/SM_<MobName>.SM_<MobName>` |
| Texture | `/Game/Characters/Mobs/<MobName>/T_<MobName>.T_<MobName>` |

Unreal's headless GLB `AssetImportTask` path failed to materialize assets in repeated import attempts. The import script was extended to convert each GLB to FBX plus extracted PNG through Blender 5.1, then import the generated FBX/PNG through Unreal. The production output remains in the requested `/Game/Characters/Mobs/<MobName>/` namespace.

Imported roster:

| StageTag | Count | EnemyIDs |
|---|---:|---|
| Dungeon | 10 | Slime, BoneWalker, RatPack, CaveBat, HexSlinger, TombSpider, StoneSentinel, MimicLure, BoneConjurer, CryptWraith |
| Forest | 10 | MushroomBrute, TreantSapling, ThornImp, TuskerBoar, HiveWasp, TreantAncient, ForestWraith, SporeBomb, VineStrangler, MyconidDruid |
| Ocean | 10 | CrabGuard, DrownedSailor, JellyHover, ReefShark, GhostRay, AnglerfishStalker, CoralMortar, SeaMine, BrineStrafer, DrownedPriestess |
| Martian | 10 | DroneGrunt, CrystalCrawler, PlasmaSpitter, RocketLeaper, SaucerDrone, PlasmaSentinel, MindSlug, CrystalBomber, SandTunneler, CyberLich |
| Hell | 10 | PitImp, BoneKnight, FireSkull, Hellhound, Gargoyle, DemonSentinel, BrimstoneMortar, SinEater, PlagueCultist, HellWyrm |

Validation evidence:

| Check | Result |
|---|---|
| `Saved/QuadRetroEnemyVisualImportReport.json` | `success=true`, `count=50`, `skipped=0`, `failures=0` |
| `Saved/EnemyQuadRetroUnrealValidationReport.json` | `success=true`, `expected_count=50`, `checked_count=50`, `failures=0` |
| Filesystem check | 50 mob directories, 100 mob `.uasset` files |

## Data Migration

| File | Result |
|---|---|
| `Content/Data/Enemies.csv` | Legacy 25 regular-mob rows replaced by 50 production rows with extended schema. |
| `Content/Data/CharacterVisuals.csv` | Legacy 25 regular-mob visual rows removed; 50 `/Game/Characters/Mobs/` rows added; non-enemy rows preserved. |
| `Content/Data/Stages.csv` | Enemy slots expanded to `EnemyA` through `EnemyJ`; all 20 stages populated by the requested 7/8/9/10 slot gating. |
| `/Game/Data/DT_Enemies` | Reloaded from CSV with `Scripts/SetupCombatRosterDataTables.py`. |
| `/Game/Data/DT_CharacterVisuals` | Reloaded and saved by the import script after mesh scale normalization. |
| `/Game/Data/DT_Stages` | Reloaded from CSV with `Scripts/SetupCombatRosterDataTables.py`. |

Counts:

| Table | Count |
|---|---:|
| `Enemies.csv` enemy rows | 50 |
| `CharacterVisuals.csv` mob visual rows under `/Game/Characters/Mobs/` | 50 |
| `Stages.csv` rows | 20 |

Stage slot samples:

| Stage row | EnemyA-J |
|---|---|
| `Stage_01` | Slime, BoneWalker, RatPack, CaveBat, HexSlinger, TombSpider, StoneSentinel, None, None, None |
| `Stage_04` | Slime, BoneWalker, RatPack, CaveBat, HexSlinger, TombSpider, StoneSentinel, MimicLure, BoneConjurer, CryptWraith |
| `Stage_17` | PitImp, BoneKnight, FireSkull, Hellhound, Gargoyle, DemonSentinel, BrimstoneMortar, None, None, None |
| `Stage_20` | PitImp, BoneKnight, FireSkull, Hellhound, Gargoyle, DemonSentinel, BrimstoneMortar, SinEater, PlagueCultist, HellWyrm |

## Schema And Code Changes

| Area | Files |
|---|---|
| `FT66EnemyData` extended with `Archetype`, `Feeling`, `Rarity`, `StageTag`, `PrimaryColor`, `SecondaryColor` | `Source/T66/Data/T66DataTypes.h` |
| `FStageData` expanded from `EnemyA`-`EnemyE` to `EnemyA`-`EnemyJ` | `Source/T66/Data/T66DataTypes.h` |
| Stage preload/read consumers updated for A-J | `Source/T66/Core/T66GameInstance.cpp`, `Source/T66/Gameplay/GameMode/T66GameMode_Bootstrap.cpp`, `Source/T66/Gameplay/T66EnemyDirector.cpp` |
| New production mob IDs mapped to fallback families | `Source/T66/Gameplay/Enemies/T66EnemyFamilyResolver.cpp` |
| `/Game/Characters/Mobs/` treated as QuadRetro static visual namespace for runtime shared-material DMI texture override | `Source/T66/Core/T66CharacterVisualSubsystem.cpp` |
| Legacy sample/runtime references moved from old regular IDs to production IDs | `Source/T66/Gameplay/T66GoblinThiefEnemy.cpp`, `Source/T66/Gameplay/T66UniqueDebuffEnemy.cpp`, `Source/T66/UI/T66CollectorOverlayWidget.cpp`, `Source/T66/UI/T66LabOverlayWidget.cpp`, `Source/T66/Gameplay/GameMode/T66GameMode_WorldInteractables.cpp`, `Source/T66/Core/Backend/T66BackendSubsystem.cpp` |
| Pending issues convention added | `AGENTS.md` |

Build-blocking unrelated compatibility fix:

| File | Change |
|---|---|
| `Source/T66/Core/T66RetroFXSettings.h` | Changed Blueprint-exposed `UIChromeEdgeDistortionSeed` from `uint32` to `int32` because UHT rejects Blueprint `uint32`. |
| `Source/T66/UI/Style/T66FlatStyle.cpp` | Cast the `int32` seed back to `uint32` when hashing, preserving the existing bit-pattern behavior. |

## Legacy Deletion Manifest

| Path | Result |
|---|---|
| `/Game/Characters/Enemies/Regular/` | Deleted by `EditorAssetLibrary.delete_directory`; filesystem path `Content/Characters/Enemies/Regular` no longer exists. |
| `/Game/Characters/_Legacy/MaterialInstances_QuadRetro/` | Deleted by `EditorAssetLibrary.delete_directory`; filesystem path `Content/Characters/_Legacy/MaterialInstances_QuadRetro` no longer exists. |

Reference scan after migration found no source/script/data references to the 25 legacy regular mob IDs. Remaining matches are boss IDs such as `Ocean_DrownedCaptain`, the import script's legacy deletion constant, and the pending issue note about old save-game unlock IDs.

## Pending Issues Created

| File | Topic |
|---|---|
| `Source/T66/Data/pending_issues_Data.md` | `FamilyID` + `RoleID` + `Archetype` redundancy and future deprecation. |
| `Source/T66/Gameplay/Enemies/pending_issues_Enemies.md` | Missing production archetype classes: Exploder, Strafer, Stutterer, Turret, Burrower, Necromancer. |
| `Source/T66/Gameplay/pending_issues_Gameplay.md` | Director still uses fallback-family behavior; Hell has no Core Ranged mob; no automated main-board enemy wave smoke hook. |
| `Content/Data/pending_issues_Data.md` | Empty stage slots in stages 1-3; all new mobs have `StatusEffectOnHit=None`. |
| `Source/T66/Core/pending_issues_Core.md` | Existing save games may retain legacy lab unlock IDs. |

## Verification

| Step | Command / Evidence | Result |
|---|---|---|
| Python syntax | `python -m py_compile Scripts\ImportQuadRetroEnemyVisuals.py Scripts\QuadRetroCharacterPipelineDefaults.py Scripts\ValidateEnemyBossRosterData.py Scripts\ValidateImportedEnemyVisualsAndExit.py Scripts\SetCharacterTextureStreamingDefaults.py Scripts\GenerateCharacterMeshLODs.py` | Passed |
| CSV/data rules | `python Scripts\ValidateEnemyBossRosterData.py` | Passed: 20 stages, 50 enemies, 20 encounters, 23 boss rows |
| Unreal import | `UnrealEditor-Cmd.exe ... -ExecutePythonScript=Scripts\RunImportQuadRetroEnemyVisualsAndExit.py` | Passed: 50 imported, 0 skipped, 0 failures |
| DataTable reload | `UnrealEditor-Cmd.exe ... -run=pythonscript -script=Scripts/SetupCombatRosterDataTables.py` | Passed |
| Unreal asset validation | `UnrealEditor-Cmd.exe ... -run=pythonscript -script=Scripts/ValidateImportedEnemyVisualsAndExit.py` | Passed: 50 checked, 0 failures |
| Full build/cook/stage | `powershell -NoProfile -ExecutionPolicy Bypass -File .\Scripts\StageStandaloneBuild.ps1` | Passed |
| Shortcut target | `C:\UE\T66\T66 Standalone.lnk` and pinned taskbar shortcut | Both target `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe` |
| Standalone frontend launch | Staged exe with `-T66AutoScreenshot` | Exit code 0; screenshot `Saved/StandaloneLogs/MobProduction_FrontendSmoke.png` |
| Standalone gameplay launch | Staged exe direct `/Game/Maps/GameplayLevel` with `-T66GameplayAutoScreenshot` | Exit code 0; screenshot `Saved/StandaloneLogs/MobProduction_Gameplay_NoListen.png` |
| Floor 1 gallery | `Saved/StandaloneLogs/MobProduction_Gameplay_NoListen.log` | `[StartGallery] Spawned inert showcase: heroes=24, enemies=50, bosses=23, npcs=3, interactables=25, traps=4.` |
| Stage 1 startup | `Saved/StandaloneLogs/MobProduction_Gameplay_NoListen.log` | Stage 1 altar spawned, tower content spawned, `Dungeon_SewerSlimeKing` boss spawned, no fatal/error lines in the no-listen smoke extract. |

Notes:

- A first standalone BuildCookRun attempt failed before cooking because an unrelated dirty `uint32` Blueprint property in `FT66RetroFXSettings` was invalid for UHT. This was fixed narrowly and the rerun passed.
- A direct `/Game/Maps/GameplayLevel?listen` smoke also spawned the 50-enemy gallery, but local SteamSockets listen initialization failed without Steam/dev ticket and returned to frontend. The accepted gameplay smoke used `/Game/Maps/GameplayLevel` without `?listen`.
- The gameplay screenshot confirms a live staged gameplay frame; the gallery count is log-verified rather than manually counted visually from the screenshot.

## Judgment Calls

| Decision | Reason |
|---|---|
| Converted GLB to FBX/PNG via Blender before Unreal import | Required because the repeated headless GLB import path did not create assets. |
| Kept unsupported archetypes on fallback `FamilyID` classes | Required by prompt until dedicated archetype classes exist. |
| Did not force a Hell Core Ranged mob | Prompt marks Hell's no-Ranged Core design as intentional. |
| Extended the director to consume all non-empty A-J slots instead of keeping only A-E | Low-risk path that preserves existing fallback-family spawning while making all authored stage slots available. |
| Reported main-board wave smoke as unverified rather than overstating it | Existing automation can verify staged map/gallery/bootstrap, but not drive the player into board combat. |
