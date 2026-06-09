# T66 Visual / Model / Material / Difficulty Topology Audit

Date: 2026-06-07  
Project: `C:\UE\T66`, UE 5.7, pure C++  
Provenance: this file packages the previously completed and Claude-validated audit content into a durable markdown report.  
Mode: read-only investigation. No source, asset, config, data, or build-output edits were made during the investigation. Disposable editor readback artifacts were written under `C:\UE\T66\Saved\VisualTopologyAudit`.

Confidence key: `[High]` = live file plus editor/readback or deterministic code path; `[Medium]` = live code/data proof but no map binary readback.

## Executive Conclusion

- `[High]` The strict claim "all current easy content is FriendSlop" is false.
- `[High]` Most easy character-style content is FriendSlop raw Pixal3D: Hero 1 male, the 3 prompt-requested companions, 3 pets, dungeon mobs, dungeon bosses, dungeon NPCs, vehicle, and SkullRemains.
- `[High]` Current easy exceptions: `Hero_1_Stacy`, `Hero_1_Stacy_DemoSkin`, `Companion_04` because it is also `bUnlockedByDefault=true`, `WallLamp_Easy`, `WallTorch_Easy`, all dungeon floor/wall/ceiling environment surfaces, and `BrokenVase_Easy` which is currently missing.
- `[High]` Raw FriendSlop color source is baked UV base-color PNG, not vertex color or a flat material constant. Active raw FriendSlop mesh slots use one master: `/Game/Materials/M_GLB_Unlit.M_GLB_Unlit`, `MSM_UNLIT`, `BLEND_OPAQUE`.

## A. Model-Bearing Data Tables

| Source | Defines | Model/material/texture columns | Runtime meaning | Confidence |
|---|---|---|---|---|
| `Content/Data/CharacterVisuals.csv` | Hero, companion, NPC, mob, boss visual rows | `SkeletalMesh`, `StaticMesh`, `OutlineStaticMesh`, `PixelatedTextureAssetPath`, animations | Main visual resolver via `UT66CharacterVisualSubsystem::ResolveVisual` / `ApplyCharacterVisual` | `[High]` |
| `Content/Data/MobVertexAnimations.csv` | Optional mob VAT override | `StaticMesh`, `Material`, `PixelatedTextureAssetPath`, `PositionTexture`, `NormalTexture` | Checked before `CharacterVisuals`; current easy VAT rows are disabled | `[High]` |
| `Content/Data/Enemies.csv` | 60 mob stat rows | No mesh column | Mobs render by matching `EnemyID` to `CharacterVisuals.csv` | `[High]` |
| `Content/Data/Bosses.csv`, `BossEncounters.csv` | Boss tuning/encounters | `VisualConcept`; no live mesh column | Bosses render by matching `BossID` to `CharacterVisuals.csv` | `[High]` |
| `Content/Data/Companions.csv` | Companion identity/unlock data | Portraits only | Companion mesh comes from `CharacterVisuals.csv` | `[High]` |
| `Content/Data/Pets.csv` | 3 boss-pet rows | `CaptureVisualMesh` | Pets directly reference capture mesh | `[High]` |
| `Content/Data/NPCs.csv` | NPC tuning rows | No mesh column | NPC actors set NPCID; visuals resolve via `CharacterVisuals.csv` | `[High]` |
| `Content/Data/WorldVisualProps.json` | 4 easy prop rows | `PropData.DisplayMesh` | Generic props load mesh, then runtime flat-color tint is applied | `[High]` |
| `Content/Data/VehicleInteractables.json` | Shared and per-difficulty vehicle rows | `VehicleData.DisplayMesh` | Imported vehicle mesh material is preserved when load succeeds | `[High]` |
| `Content/Data/Heroes.csv` | Hero gameplay rows | `AutoAttackProjectileMesh`, portraits, `HeroClass` | Hero body visuals are generated IDs resolved in `CharacterVisuals.csv` | `[High]` |
| `Content/Data/UniqueEnemies.csv` | One unique enemy row | `CharacterVisualID` | `BackroomsChaser` points to visual ID `Slime`; no direct mesh ref | `[High]` |
| `Content/Data/CombatVFXBindings.csv` | 20 combat VFX bindings | `NiagaraSystem` | Separate VFX graph, not model-body topology | `[High]` |

Definitive definitions: mobs are `Enemies.csv` plus `Stages.csv` spawn columns, rendered through `CharacterVisuals.csv`; NPCs are `NPCs.csv` plus C++ NPC actor classes, rendered through `CharacterVisuals.csv`; interactables are split between `WorldVisualProps.json`, `VehicleInteractables.json`, and C++ placeholder/fallback interactable classes.

## B. Easy-Difficulty Roster, Actual Loads

`[High]` Easy is stages 1-4, ThemeID `Dungeon`.

| Entity set | Resolved asset(s) | Material/color source | Family | Confidence |
|---|---|---|---|---|
| Hero 1 male `Hero_1_Chad` | `/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/PhysicsFirst/SK_Hero_1_Chad_PhysicsFirst.SK_Hero_1_Chad_PhysicsFirst` | `MI_SK_Hero_1_Chad_PhysicsFirst`, parent `M_GLB_Unlit`; 4096 PNG from `FriendSlopProbe_Hero1Male_20260604_1415` | FriendSlop raw Pixal3D | `[High]` |
| Hero 1 female `Hero_1_Stacy` | `/Game/Characters/Heroes/Hero_1/Stacy/AnimatedToonStyle/SK_Hero_1_Stacy.SK_Hero_1_Stacy`; row also carries static/outline ToonStyle refs | Parent `M_Toon_Character` | AnimatedToonStyle / Pixal3DToonStyle | `[High]` |
| Companions requested by prompt: `Companion_01`, `Companion_02`, `Companion_03` | `/Game/Characters/Companions/<ID>/Default/Pixal3D/SM_<ID>.SM_<ID>` | One slot, parent `M_GLB_Unlit`; FriendSlop batch 4096 PNG | FriendSlop raw Pixal3D | `[High]` |
| Data caveat: `Companion_04` | `bUnlockedByDefault=true`; `/Game/Characters/Companions/Companion_04/Default/AnimatedToonStyle/SK_Companion_04.SK_Companion_04` | Parent `M_Toon_Character` | AnimatedToonStyle / Pixal3DToonStyle | `[High]` |
| Pets | `Pet_Dungeon_SewerSlimeKing`, `Pet_Dungeon_WebMatriarch`, `Pet_Dungeon_BoneJailer` under `/Game/Characters/Pets/<ID>/Pixal3D/SM_<ID>` | Parent `M_GLB_Unlit`; FriendSlop PNG | FriendSlop raw Pixal3D | `[High]` |
| Dungeon mobs | `Slime`, `BoneWalker`, `RatPack`, `CaveBat`, `HexSlinger`, `TombSpider`, `StoneSentinel`, `CursedCrow`, `FamishedGhoul`, `MimicLure`, `BoneConjurer`, `CryptWraith` under `/Game/Characters/Mobs/<ID>/Pixal3D/SM_<ID>` | Parent `M_GLB_Unlit`; baked UV base-color PNG; VAT disabled | FriendSlop raw Pixal3D | `[High]` |
| Dungeon bosses | `Dungeon_SewerSlimeKing`, `Dungeon_WebMatriarch`, `Dungeon_BoneJailer`, `Dungeon_BaelFallenChad` under `/Game/Characters/Enemies/Bosses/<ID>/Pixal3D/SM_<ID>` | Parent `M_GLB_Unlit`; baked UV base-color PNG | FriendSlop raw Pixal3D | `[High]` |
| Dungeon NPCs | `CasinoNPC`, `Saint`, `Ouroboros`, `VendorNPC`, `LoanShark` under `/Game/Characters/NPCs/<ID>/Pixal3D/SM_<ID>` | Parent `M_GLB_Unlit`; baked UV base-color PNG | FriendSlop raw Pixal3D | `[High]` |
| Vehicle | `/Game/World/Interactables/Vehicles/SM_Vehicle_Pixal3D.SM_Vehicle_Pixal3D` | `MI_SM_Vehicle_Pixal3D`, parent `M_GLB_Unlit`; runtime preserves imported material | FriendSlop raw Pixal3D | `[High]` |
| `SkullRemains_Easy` | `/Game/World/VisualProps/Easy/SM_SkullRemains_Easy_Pixal3D.SM_SkullRemains_Easy_Pixal3D` | Slot parent `M_GLB_Unlit`, but `AT66WorldVisualProp` applies flat runtime tint | FriendSlop asset with runtime flat-color caveat | `[High]` |
| `WallLamp_Easy`, `WallTorch_Easy` | `/Game/World/VisualProps/Easy/SM_WallLamp_Easy_Pixal3D...`, `/SM_WallTorch_Easy_Pixal3D...` | Slot parent `M_Toon_Character`; BaseColor/Tint/InnerLine textures from older ToonStyle source; runtime flat tint likely replaces slot | ToonStyle Pixal3D | `[High]` |
| `BrokenVase_Easy` | `/Game/World/VisualProps/Easy/SM_BrokenVase_Easy_Pixal3D.SM_BrokenVase_Easy_Pixal3D` | Asset failed to resolve; prop code falls back to cylinder + flat tint | Missing/dangling | `[High]` |

## C. Difficulty / Theme System

| Difficulty | Stage range | Theme | Bosses | Enemy set | Confidence |
|---|---:|---|---|---|---|
| Easy | 1-4 | Dungeon | 4 dungeon bosses | 12 dungeon mobs listed above | `[High]` |
| Medium | 5-8 | Forest | `Forest_BrambleTreant`, `Forest_MyconidQueen`, `Forest_ThornHive`, `Forest_BuerVerdantChad` | Mushroom/Treant/Forest set | `[High]` |
| Hard | 9-12 | Ocean | `Ocean_ReefCrabColossus`, `Ocean_AbyssalJellyfish`, `Ocean_DrownedCaptain`, `Ocean_FocalorDrownedChad` | Crab/Drowned/Ocean set | `[High]` |
| VeryHard | 13-16 | Martian | `Martian_RedSandBehemoth`, `Martian_CrystalMantis`, `Martian_PlasmaSaucerPrime`, `Martian_StolasAstralChad` | Drone/Crystal/Martian set | `[High]` |
| Impossible | 17-20 | Hell | `Hell_Horseman_Conquest`, `Hell_FalseProphet`, `Hell_Antichrist`, `Hell_GreatDragon` | Pit/Bone/Hell set | `[High]` |

`DifficultyTuning.json` defines stage bands. `Stages.csv` defines `DifficultyID`, `ThemeID`, boss, encounter, and `EnemyA`-`EnemyL`. `T66EnemyDirector` resolves actual mob spawn IDs from those stage rows. `T66TowerMapTerrain::ResolveGameplayLevelThemeForDifficulty` separately maps difficulty to procedural tower theme: Easy->Dungeon, Medium->Forest, Hard->Ocean, VeryHard->Martian, Impossible->Hell.

## D. FriendSlop Wiring + Naming

- `[High]` Active batch manifest: `Model Generation/Runs/Pixal3D/FriendSlopEasyBatch_20260604_1532/FriendSlopEasyBatch_20260604_1532_manifest.json`, 49 entries.
- `[High]` Import script: `Scripts/ImportFriendSlopRawPixal3DFBXAndExit.py`. It imports FBX static meshes, imports `T_<AssetID>_BaseColor`, creates `MI_SM_<AssetID>` parented to `/Game/Materials/M_GLB_Unlit`, binds `BaseColorTexture` and `DiffuseColorMap`, and assigns that MI to mesh slots.
- `[High]` Data reload/wiring scripts: `Scripts/ReloadFriendSlopEasyPixal3DDataTablesAndExit.py` and `Scripts/ApplyFriendSlopRawCharacterVisualRows.py`.
- `[High]` Exact live references from the 49 batch: `Companion_01`-`03`, 3 pets, 12 dungeon mobs, 4 dungeon bosses, 5 NPCs, `Vehicle_Pixal3D`, and `SkullRemains_Easy_Pixal3D`. Hero 1 male is a separate FriendSlop probe run.
- `[High]` Imported but currently unwired batch props/interactables: 8 boost meshes, `Chest_Pixal3D`, `CompanionCage_Pixal3D`, `CowardiceGate_Pixal3D`, `LootCrate`, `DifficultyTotem_Pixal3D`, `TutorialGate_Pixal3D`, `Fountain_Pixal3D`, `IdolAltar_Pixal3D`, `LootBag_Shared_Pixal3D`, `LootWheel_Pixal3D`, `StageGate_Pixal3D`, `WeaponAltar_Pixal3D`.
- `[High]` No literal `Pixal3D/Easy` path was found in searched live source/data. Rename scope should include `Easy_Pixal3D` rows in `WorldVisualProps.json`, `FriendSlopEasyBatch` / `FriendSlopEasyPixal3D` scripts and docs, and active `/Pixal3D/` folders whose material texture provenance is FriendSlop.

## E. Materials - Authoritative Inventory + Usage Graph

Editor confirmation artifact: `C:\UE\T66\Saved\VisualTopologyAudit\material_readback.json`.

`[High]` An initial readback run crashed with an `AssetData.object_path` API mismatch, logged in `material_readback_error.txt`. The script was corrected and rerun, producing complete JSON: 698 material/material-instance assets, 576 mesh assets, and 1,376 texture assets. In the successful run, `BP_HeroBase_C` and `SM_BrokenVase_Easy_Pixal3D` were the notable unresolved asset refs.

| Group | State | Shading / blend | Uses | Confidence |
|---|---:|---|---|---|
| All material interfaces | 698 | 352 Unlit/Opaque; 125 DefaultLit/Opaque; 110 Unlit/Masked; 43 Unlit/Translucent; 26 Unlit/Additive; others smaller | Full inventory in readback JSON | `[High]` |
| Active easy raw FriendSlop slots | 30 slot MIs in 34-entity easy roster | Parent `M_GLB_Unlit`, `MSM_UNLIT`, `BLEND_OPAQUE` | Hero1 Chad, companions 01-03, pets, mobs, bosses, NPCs, vehicle, SkullRemains slot | `[High]` |
| Active easy ToonStyle exceptions | 3 slot MIs, plus Companion04 if included | Parent `M_Toon_Character`, `MSM_UNLIT`, `BLEND_OPAQUE` | Hero1 Stacy, WallLamp, WallTorch, Companion04 | `[High]` |
| Tower environment | `MI_<Theme>_<Surface>` | Parent `M_Toon_Environment`, `MSM_UNLIT`, `BLEND_OPAQUE` | Floors, walls, ceilings | `[High]` |
| Environment fallback | `M_Environment_Lit` | `MSM_DEFAULT_LIT`, `BLEND_OPAQUE` | Legacy/dynamic difficulty-ground fallback | `[High]` |
| VFX | 37 `/Game/VFX`; 130 `/Game/Stylized_VFX_StPack` | Mostly Unlit Additive/Translucent/Masked; third-party pack includes DefaultLit | Combat/trap/boss/projectile VFX | `[High]` |
| Retro/PS1 | 20 direct Retro hits plus UE5RFX assets | Optional post-process/geometry material stack | Runtime only when RetroFX settings enable it; defaults forced off | `[High]` |
| Rubber/clay | 0 hits | None found | No existing rubber/clay master | `[High]` |

`[High]` Characters are currently unlit, not baked-lit: raw FriendSlop uses `M_GLB_Unlit`; ToonStyle characters use `M_Toon_Character`, also Unlit/Opaque.

## F. World Ground / Wall Materials

- `[High]` Procedural tower surfaces load `/Game/ToonStyle/Environment/<Theme>/Materials/MI_<Theme>_<Surface>.MI_<Theme>_<Surface>` for `Floor`, `Ceiling`, `Wall_XZ`, and `Wall_YZ`.
- `[High]` Dungeon examples: `MI_Dungeon_Floor` and `MI_Dungeon_Wall_XZ` parent to `M_Toon_Environment`, bind flat 1024 environment textures from `SourceAssets/ToonStyle/ImageGen/Phase1A`, and expose tiling/ramp parameters. No material depth/displacement path was found.
- `[High]` `T66TerrainThemeAssets::ResolveDifficultyGroundMaterial` is a legacy/fallback path that builds dynamic `M_Environment_Lit` from TowerForest ground texture for all difficulties, but current tower theme resolution uses ToonStyle environment MIs first.
- `[High]` To migrate dungeon floors/walls/ceilings to rubber, reparent or replace `MI_Dungeon_Floor`, `MI_Dungeon_Wall_XZ`, `MI_Dungeon_Wall_YZ`, and `MI_Dungeon_Ceiling`. A strict color-only rubber master must ignore or remove the current texture inputs.

## G. Safe Archival Topology

- `[High]` Archiving all non-FriendSlop model assets while leaving rows intact would break active/default easy-visible content: `Hero_1_Stacy`, `Companion_04`, `WallLamp_Easy`, `WallTorch_Easy`, and dungeon environment surfaces. `BrokenVase_Easy` is already dangling.
- `[High]` Non-easy assets are not automatically inert. `Stages.csv` defines stages 5-20 and `CharacterVisuals.csv` contains their refs; if those difficulties are selectable, those assets are runtime content.
- `[High]` Cook risk is high if rows remain but assets move. `DefaultGame.ini` always cooks `/Game/Characters`, `/Game/Data`, `/Game/Materials`, `/Game/Maps`, `/Game/Stylized_VFX_StPack`, `/Game/ToonStyle`, `/Game/UE5RFX`, `/Game/VFX`, and `/Game/World`.
- `[High]` Safe rule: migrate or disable rows/systems first, then archive assets. Do not classify by folder label alone; use resolved mesh path plus material/texture provenance.
- `[High]` Hard parts for a single rubber master: ToonStyle female/Companion04/world-prop exceptions, dungeon environment materials, VFX blend/domain differences, RetroFX optional material replacement, runtime `ApplyT66Color` dynamic overrides, and the `BrokenVase_Easy` dangling reference.

## Verification

- Read live routers and instructions: `AGENTS.md`, `OPERATOR_VALIDATOR_PROTOCOL.md`, `Audit/AUDIT_AGENTS.md`, `Gameplay/GAMEPLAY_AGENTS.md`, `Gameplay/World/WORLD_AGENTS.md`, and Pixal3D/model-generation routers.
- Ran editor material/mesh/texture readback to `Saved/VisualTopologyAudit/material_readback.json`.
- Cross-checked CSV/JSON data refs, C++ runtime resolvers, FriendSlop import/reload scripts, stage/difficulty data, tower theme code, tower lighting code, RetroFX settings code, and cook/map config.
- Claude validator was used per protocol for the audit: independent answer `Result: OK`; cross-review `Result: OK`. Valid corrections were incorporated: readback-run narrative, `UniqueEnemies.csv`, and Companion04 default-unlocked exception.
- `[Medium]` Exact placed actor lighting in `FrontendLevel.umap` / `GameplayLevel.umap` was not separately dumped; gameplay procedural tower lighting is code-confirmed, frontend map actor specifics would need a map actor readback.
