# T66 Mob System And Model Pipeline Report

Date: 2026-05-15

This report describes the current regular mob roster, runtime spawn system, model generation and import path, Retro FX tooling, and first-floor mob display order as found in the live project files.

## Source Files Checked

| Area | Current source |
| --- | --- |
| Regular mob data | `Content/Data/Enemies.csv` |
| Stage mob order | `Content/Data/Stages.csv` |
| Runtime visual mapping | `Content/Data/CharacterVisuals.csv` |
| Runtime mob VAT mapping | `Content/Data/MobVertexAnimations.csv` |
| Production Pixal3D batches | `Model Generation/Production/Roster_v1/AgentA/manifest.csv`, `Model Generation/Production/Roster_v1/AgentB/manifest.csv` |
| Production batch reports | `Model Generation/Production/Roster_v1/AgentA/Report.md`, `Model Generation/Production/Roster_v1/AgentB/Report.md` |
| Enemy import script | `Scripts/ImportQuadRetroEnemyVisuals.py` |
| Import verification | `Saved/QuadRetroEnemyVisualImportReport.json` |
| VAT verification | `Saved/EasyMobVATVerifyReport.json` |
| Spawn/runtime code | `Source/T66/Gameplay/T66EnemyDirector.cpp`, `Source/T66/Gameplay/T66EnemyBase.cpp`, `Source/T66/Gameplay/Enemies/T66EnemyFamilyResolver.cpp` |
| Visual application code | `Source/T66/Core/T66CharacterVisualSubsystem.cpp` |
| Data structs | `Source/T66/Data/T66DataTypes.h` |
| Retro FX runtime | `Source/T66/Core/T66RetroFXSettings.h`, `Source/T66/Core/T66RetroFXSubsystem.cpp`, `Source/T66/Core/T66PixelationSubsystem.cpp` |

## Executive Snapshot

- Current regular mob roster: 50 mobs.
- Data state: all 50 `Enemies.csv` rows have `ModelStatus=MeshReady`.
- Stage structure: 20 stages, broken into 5 difficulties with 4 stages each.
- Per-difficulty roster size: 10 mobs per difficulty/theme.
- Runtime visual state: 50 regular mobs have `CharacterVisuals.csv` rows that point to static meshes and textures under `/Game/Characters/Mobs/<EnemyID>/`.
- Current animation state: the 10 Easy/Dungeon mobs also have enabled VAT rows under `/Game/Characters/MobsVAT/<EnemyID>/`.
- Runtime behavior classes currently implemented for regular mobs: `Melee`, `Ranged`, `Rush`, and `Flying`.
- Richer production archetypes exist in data, but several still fall back through those four family classes at runtime.
- Model generation evidence exists for 50 Pixal3D production-roster GLBs across Agent A and Agent B.
- First-floor display/order source is `Stages.csv` stage 1, slots `EnemyA` through `EnemyJ`.

## Runtime Mob System

The mob system is data-driven at the identity and roster level, then resolved to a smaller set of C++ runtime behavior families.

1. `UT66GameInstance` owns the default table paths for stages and enemies and exposes `GetStageData(...)` and `GetEnemyData(...)`.
2. `AT66EnemyDirector` asks the game instance for the current stage, reads the stage's `EnemyA` through `EnemyJ` slots, and filters out `None`.
3. Initial tower population uses the current stage's non-None mob list and spawns a target of 4 initial enemies per gameplay floor.
4. Wave spawning uses the same stage mob list and randomly chooses a `MobID` for each pending spawn.
5. Class resolution first looks up the `Enemies.csv` row and maps `FamilyID` to one of the implemented C++ classes:
   - `Melee` -> `AT66MeleeEnemy`
   - `Ranged` -> `AT66RangedEnemy`
   - `Rush` -> `AT66RushEnemy`
   - `Flying` -> `AT66FlyingEnemy`
6. `AT66EnemyBase::ConfigureAsMob(...)` stores the stage `MobID`, sets `CharacterVisualID` to the same value, applies stage scaling, and then tries visuals in this order:
   - enabled mob VAT row from `DT_MobVertexAnimations`
   - static visual row from `DT_CharacterVisuals`
   - runtime placeholder shape/color fallback
7. Stage scaling is applied in `AT66EnemyBase`; the base health and armor use stage and difficulty multipliers, and score award is frozen after spawn.

The important distinction is that `Archetype` is the richer design/mechanical label, while `FamilyID` is still the runtime class selector. For example, `StoneSentinel` has `Archetype=Turret`, but it currently resolves through `FamilyID=Ranged`.

## Data Shape

`FT66EnemyData` currently carries:

- `EnemyID`
- `DisplayName`
- `DifficultyID`
- `ThemeID`
- `FamilyID`
- `RoleID`
- `StatusEffectOnHit`
- `VisualConcept`
- `ImagePrompt`
- `ModelStatus`
- `Archetype`
- `Feeling`
- `Rarity`
- `StageTag`
- `PrimaryColor`
- `SecondaryColor`

`FStageData` carries `EnemyA` through `EnemyJ`, so every stage can author up to 10 regular mobs. Current stage rows use a progressive reveal pattern: local stage 1 uses 7 mobs, local stage 2 adds the first late mob, local stage 3 adds the second late mob, and local stage 4 lists all 10 mobs while also being marked as the boss-only finale.

## Roster Counts

### By Difficulty

| Difficulty | Theme | Stage range | Mob count |
| --- | --- | ---: | ---: |
| Easy | Dungeon | 1-4 | 10 |
| Medium | Forest | 5-8 | 10 |
| Hard | Ocean | 9-12 | 10 |
| VeryHard | Martian | 13-16 | 10 |
| Impossible | Hell | 17-20 | 10 |
| Total | All | 1-20 | 50 |

### By FamilyID

| FamilyID | Count | Current runtime class |
| --- | ---: | --- |
| Melee | 17 | `AT66MeleeEnemy` |
| Ranged | 16 | `AT66RangedEnemy` |
| Rush | 11 | `AT66RushEnemy` |
| Flying | 6 | `AT66FlyingEnemy` |
| Total | 50 | 4 classes |

### By Rarity

| Rarity | Count |
| --- | ---: |
| Core | 25 |
| Rare | 10 |
| Late | 15 |
| Total | 50 |

### By Archetype

| Archetype | Count | Runtime support today |
| --- | ---: | --- |
| Melee | 10 | Implemented through melee family |
| Rush | 6 | Implemented through rush family |
| Ranged | 5 | Implemented through ranged family |
| Flying | 6 | Implemented through flying family |
| Exploder | 5 | Falls back through `FamilyID` |
| Necromancer | 5 | Falls back through `FamilyID` |
| Turret | 4 | Falls back through `FamilyID` |
| Stutterer | 4 | Falls back through `FamilyID` |
| Burrower | 3 | Falls back through `FamilyID` |
| Strafer | 2 | Falls back through `FamilyID` |
| Total | 50 |  |

## Mob Roster By Difficulty

### Easy / Dungeon

| EnemyID | Display name | FamilyID | Archetype | Rarity | Feeling | Model status |
| --- | --- | --- | --- | --- | --- | --- |
| `Slime` | Slime | Melee | Melee | Core | MowDown | MeshReady |
| `BoneWalker` | Bone Walker | Melee | Melee | Core | Pressure | MeshReady |
| `RatPack` | Rat Pack | Rush | Rush | Core | MowDown | MeshReady |
| `CaveBat` | Cave Bat | Flying | Flying | Core | Pressure | MeshReady |
| `HexSlinger` | Hex Slinger | Ranged | Ranged | Core | DodgeThreat | MeshReady |
| `TombSpider` | Tomb Spider | Melee | Melee | Rare | MiniBossFeel | MeshReady |
| `StoneSentinel` | Stone Sentinel | Ranged | Turret | Rare | DodgeThreat | MeshReady |
| `MimicLure` | Mimic Lure | Rush | Exploder | Late | Disruptor | MeshReady |
| `BoneConjurer` | Bone Conjurer | Ranged | Necromancer | Late | Specialist | MeshReady |
| `CryptWraith` | Crypt Wraith | Melee | Stutterer | Late | Disruptor | MeshReady |

### Medium / Forest

| EnemyID | Display name | FamilyID | Archetype | Rarity | Feeling | Model status |
| --- | --- | --- | --- | --- | --- | --- |
| `MushroomBrute` | Mushroom Brute | Melee | Melee | Core | MowDown | MeshReady |
| `TreantSapling` | Treant Sapling | Melee | Melee | Core | Pressure | MeshReady |
| `ThornImp` | Thorn Imp | Ranged | Ranged | Core | DodgeThreat | MeshReady |
| `TuskerBoar` | Tusker Boar | Rush | Rush | Core | Pressure | MeshReady |
| `HiveWasp` | Hive Wasp | Flying | Flying | Core | DodgeThreat | MeshReady |
| `TreantAncient` | Treant Ancient | Melee | Melee | Rare | MiniBossFeel | MeshReady |
| `ForestWraith` | Forest Wraith | Ranged | Strafer | Rare | DodgeThreat | MeshReady |
| `SporeBomb` | Spore Bomb | Rush | Exploder | Late | Disruptor | MeshReady |
| `VineStrangler` | Vine Strangler | Melee | Burrower | Late | Disruptor | MeshReady |
| `MyconidDruid` | Myconid Druid | Ranged | Necromancer | Late | Specialist | MeshReady |

### Hard / Ocean

| EnemyID | Display name | FamilyID | Archetype | Rarity | Feeling | Model status |
| --- | --- | --- | --- | --- | --- | --- |
| `CrabGuard` | Crab Guard | Melee | Melee | Core | Pressure | MeshReady |
| `DrownedSailor` | Drowned Sailor | Melee | Melee | Core | MowDown | MeshReady |
| `JellyHover` | Jelly Hover | Ranged | Ranged | Core | DodgeThreat | MeshReady |
| `ReefShark` | Reef Shark | Rush | Rush | Core | DodgeThreat | MeshReady |
| `GhostRay` | Ghost Ray | Flying | Flying | Core | Pressure | MeshReady |
| `AnglerfishStalker` | Anglerfish Stalker | Melee | Stutterer | Rare | MiniBossFeel | MeshReady |
| `CoralMortar` | Coral Mortar | Ranged | Turret | Rare | DodgeThreat | MeshReady |
| `SeaMine` | Sea Mine | Rush | Exploder | Late | Disruptor | MeshReady |
| `BrineStrafer` | Brine Strafer | Ranged | Strafer | Late | Disruptor | MeshReady |
| `DrownedPriestess` | Drowned Priestess | Ranged | Necromancer | Late | Specialist | MeshReady |

### VeryHard / Martian

| EnemyID | Display name | FamilyID | Archetype | Rarity | Feeling | Model status |
| --- | --- | --- | --- | --- | --- | --- |
| `DroneGrunt` | Drone Grunt | Ranged | Ranged | Core | DodgeThreat | MeshReady |
| `CrystalCrawler` | Crystal Crawler | Melee | Melee | Core | Pressure | MeshReady |
| `PlasmaSpitter` | Plasma Spitter | Ranged | Ranged | Core | DodgeThreat | MeshReady |
| `RocketLeaper` | Rocket Leaper | Rush | Rush | Core | MowDown | MeshReady |
| `SaucerDrone` | Saucer Drone | Flying | Flying | Core | DodgeThreat | MeshReady |
| `PlasmaSentinel` | Plasma Sentinel | Ranged | Turret | Rare | MiniBossFeel | MeshReady |
| `MindSlug` | Mind Slug | Melee | Stutterer | Rare | Disruptor | MeshReady |
| `CrystalBomber` | Crystal Bomber | Rush | Exploder | Late | Disruptor | MeshReady |
| `SandTunneler` | Sand Tunneler | Melee | Burrower | Late | Disruptor | MeshReady |
| `CyberLich` | Cyber Lich | Ranged | Necromancer | Late | Specialist | MeshReady |

### Impossible / Hell

| EnemyID | Display name | FamilyID | Archetype | Rarity | Feeling | Model status |
| --- | --- | --- | --- | --- | --- | --- |
| `PitImp` | Pit Imp | Rush | Rush | Core | MowDown | MeshReady |
| `BoneKnight` | Bone Knight | Melee | Melee | Core | MiniBossFeel | MeshReady |
| `FireSkull` | Fire Skull | Flying | Flying | Core | DodgeThreat | MeshReady |
| `Hellhound` | Hellhound | Rush | Rush | Core | Pressure | MeshReady |
| `Gargoyle` | Gargoyle | Flying | Flying | Core | Pressure | MeshReady |
| `DemonSentinel` | Demon Sentinel | Melee | Stutterer | Rare | MiniBossFeel | MeshReady |
| `BrimstoneMortar` | Brimstone Mortar | Ranged | Turret | Rare | DodgeThreat | MeshReady |
| `SinEater` | Sin Eater | Rush | Exploder | Late | Disruptor | MeshReady |
| `PlagueCultist` | Plague Cultist | Ranged | Necromancer | Late | Specialist | MeshReady |
| `HellWyrm` | Hell Wyrm | Melee | Burrower | Late | Disruptor | MeshReady |

## Stage Breakdown

| Stage | Difficulty | Theme | Local stage | Boss-only finale | Authored mob order |
| ---: | --- | --- | ---: | --- | --- |
| 1 | Easy | Dungeon | 1 | false | Slime, BoneWalker, RatPack, CaveBat, HexSlinger, TombSpider, StoneSentinel, None, None, None |
| 2 | Easy | Dungeon | 2 | false | Slime, BoneWalker, RatPack, CaveBat, HexSlinger, TombSpider, StoneSentinel, MimicLure, None, None |
| 3 | Easy | Dungeon | 3 | false | Slime, BoneWalker, RatPack, CaveBat, HexSlinger, TombSpider, StoneSentinel, MimicLure, BoneConjurer, None |
| 4 | Easy | Dungeon | 4 | true | Slime, BoneWalker, RatPack, CaveBat, HexSlinger, TombSpider, StoneSentinel, MimicLure, BoneConjurer, CryptWraith |
| 5 | Medium | Forest | 1 | false | MushroomBrute, TreantSapling, ThornImp, TuskerBoar, HiveWasp, TreantAncient, ForestWraith, None, None, None |
| 6 | Medium | Forest | 2 | false | MushroomBrute, TreantSapling, ThornImp, TuskerBoar, HiveWasp, TreantAncient, ForestWraith, SporeBomb, None, None |
| 7 | Medium | Forest | 3 | false | MushroomBrute, TreantSapling, ThornImp, TuskerBoar, HiveWasp, TreantAncient, ForestWraith, SporeBomb, VineStrangler, None |
| 8 | Medium | Forest | 4 | true | MushroomBrute, TreantSapling, ThornImp, TuskerBoar, HiveWasp, TreantAncient, ForestWraith, SporeBomb, VineStrangler, MyconidDruid |
| 9 | Hard | Ocean | 1 | false | CrabGuard, DrownedSailor, JellyHover, ReefShark, GhostRay, AnglerfishStalker, CoralMortar, None, None, None |
| 10 | Hard | Ocean | 2 | false | CrabGuard, DrownedSailor, JellyHover, ReefShark, GhostRay, AnglerfishStalker, CoralMortar, SeaMine, None, None |
| 11 | Hard | Ocean | 3 | false | CrabGuard, DrownedSailor, JellyHover, ReefShark, GhostRay, AnglerfishStalker, CoralMortar, SeaMine, BrineStrafer, None |
| 12 | Hard | Ocean | 4 | true | CrabGuard, DrownedSailor, JellyHover, ReefShark, GhostRay, AnglerfishStalker, CoralMortar, SeaMine, BrineStrafer, DrownedPriestess |
| 13 | VeryHard | Martian | 1 | false | DroneGrunt, CrystalCrawler, PlasmaSpitter, RocketLeaper, SaucerDrone, PlasmaSentinel, MindSlug, None, None, None |
| 14 | VeryHard | Martian | 2 | false | DroneGrunt, CrystalCrawler, PlasmaSpitter, RocketLeaper, SaucerDrone, PlasmaSentinel, MindSlug, CrystalBomber, None, None |
| 15 | VeryHard | Martian | 3 | false | DroneGrunt, CrystalCrawler, PlasmaSpitter, RocketLeaper, SaucerDrone, PlasmaSentinel, MindSlug, CrystalBomber, SandTunneler, None |
| 16 | VeryHard | Martian | 4 | true | DroneGrunt, CrystalCrawler, PlasmaSpitter, RocketLeaper, SaucerDrone, PlasmaSentinel, MindSlug, CrystalBomber, SandTunneler, CyberLich |
| 17 | Impossible | Hell | 1 | false | PitImp, BoneKnight, FireSkull, Hellhound, Gargoyle, DemonSentinel, BrimstoneMortar, None, None, None |
| 18 | Impossible | Hell | 2 | false | PitImp, BoneKnight, FireSkull, Hellhound, Gargoyle, DemonSentinel, BrimstoneMortar, SinEater, None, None |
| 19 | Impossible | Hell | 3 | false | PitImp, BoneKnight, FireSkull, Hellhound, Gargoyle, DemonSentinel, BrimstoneMortar, SinEater, PlagueCultist, None |
| 20 | Impossible | Hell | 4 | true | PitImp, BoneKnight, FireSkull, Hellhound, Gargoyle, DemonSentinel, BrimstoneMortar, SinEater, PlagueCultist, HellWyrm |

## First Floor Mob Display Order

For floor/stage 1, the authored order is the stage 1 `EnemyA` through `EnemyJ` order in `Content/Data/Stages.csv`.

| Slot | EnemyID | Display name | FamilyID | Archetype | Rarity |
| --- | --- | --- | --- | --- | --- |
| EnemyA | `Slime` | Slime | Melee | Melee | Core |
| EnemyB | `BoneWalker` | Bone Walker | Melee | Melee | Core |
| EnemyC | `RatPack` | Rat Pack | Rush | Rush | Core |
| EnemyD | `CaveBat` | Cave Bat | Flying | Flying | Core |
| EnemyE | `HexSlinger` | Hex Slinger | Ranged | Ranged | Core |
| EnemyF | `TombSpider` | Tomb Spider | Melee | Melee | Rare |
| EnemyG | `StoneSentinel` | Stone Sentinel | Ranged | Turret | Rare |
| EnemyH | `None` | Empty slot | - | - | - |
| EnemyI | `None` | Empty slot | - | - | - |
| EnemyJ | `None` | Empty slot | - | - | - |

Runtime spawning does not currently walk this order as a deterministic queue. `AT66EnemyDirector` preserves this order when building the stage mob list, then randomly chooses among the non-None entries for initial spawns and waves. The tutorial helper is a special case: `AT66TutorialManager::PickStage1MobID()` returns the first available stage 1 slot from `EnemyA`, `EnemyB`, then `EnemyC`, so it currently resolves to `Slime`.

The Lab and Collector mob display order is separate and hardcoded to a partial testing/unlock list: `Slime`, `TombSpider`, `TuskerBoar`, `JellyHover`, `Gargoyle`, `GoblinThief`, `UniqueEnemy`. Do not use that UI list as the authoritative first-floor order.

## How The Current Mob Models Are Represented In Game

Every regular mob has a `CharacterVisuals.csv` row keyed by the same `EnemyID` as `Enemies.csv`.

The current row shape for normal mobs is:

- `SkeletalMesh`: empty
- `StaticMesh`: `/Game/Characters/Mobs/<EnemyID>/SM_<EnemyID>.SM_<EnemyID>`
- `PixelatedTextureAssetPath`: `/Game/Characters/Mobs/<EnemyID>/T_<EnemyID>.T_<EnemyID>`
- `LoopingAnimation`, `AlertAnimation`, `RunAnimation`, `RollAnimation`: empty for the regular static-mesh fallback path
- `MeshRelativeLocation`: `(X=0,Y=0,Z=0)`
- `MeshRelativeRotation`: yaw 90 degrees
- `MeshRelativeScale`: per-mob scalar computed by the import script to normalize mesh size
- `bLoopAnimation`: false
- `bAutoGroundToActorOrigin`: true

At runtime, `UT66CharacterVisualSubsystem::ApplyCharacterVisual(...)` loads that row, applies the static mesh to the enemy's visual mesh component, binds the shared retro material path and pixelated texture where applicable, grounds the visual to the actor origin/capsule, and hides the skeletal mesh component for static visuals.

## Mob Vertex Animation Status

The Easy/Dungeon roster currently has the first production VAT pass wired:

| Item | Current value |
| --- | --- |
| CSV | `Content/Data/MobVertexAnimations.csv` |
| Data table | `/Game/Data/DT_MobVertexAnimations.DT_MobVertexAnimations` |
| Enabled rows | 10 |
| Covered mobs | Slime, BoneWalker, RatPack, CaveBat, HexSlinger, TombSpider, StoneSentinel, MimicLure, BoneConjurer, CryptWraith |
| Runtime mesh root | `/Game/Characters/MobsVAT/<EnemyID>/` |
| Material instances | `/Game/Characters/MobsVAT/<EnemyID>/MI_EasyMobVAT_<EnemyID>` |
| Position textures | `/Game/Characters/MobsVAT/<EnemyID>/TX_EasyMobVAT_<EnemyID>_Position` |
| Normal textures | `/Game/Characters/MobsVAT/<EnemyID>/TX_EasyMobVAT_<EnemyID>_Normal` |
| Frame count | 195 |
| Sample rate | 30 |
| Rows per frame | 4 |
| Clips | Idle 0-59, Move 60-99, AttackCue 100-129, HitReact 130-149, Death 150-194 |

`AT66EnemyBase` tries `DT_MobVertexAnimations` before the static `CharacterVisuals.csv` fallback. If a row is enabled and all referenced assets load, the enemy uses the VAT static mesh/material, hides the skeletal mesh component, and plays clip frames through per-enemy dynamic material parameters. Gameplay movement, collision, touch damage, hit reactions, death handling, and pooling stay owned by the enemy actor and director; VAT is a visual presentation layer.

The Easy VAT docs and verification reports still carry two important caveats:

- Python verification could not prove static mesh UV channel 2 even though the VAT textures and material parameters were valid.
- Pixal3D source GLBs and derived runtime assets are tracked through the generated-model validation path.

## Pixal3D Model Generation Process

The current Pixal3D path is repo-native and separate from Trellis. The authoritative docs are under `Model Generation/Instructions` and `Model Generation/Pixal3D`.

### Pipeline Contract

1. Start from approved source concept images.
2. Use the Pixal3D RunPod server instead of editing Trellis scripts.
3. Health-check the pod through `/health`.
4. Generate GLBs through `/generate`.
5. Save outputs under Pixal3D-specific namespaces such as `Raw/Pixal3D/...` and `QA/Pixal3DFront/...`.
6. Verify nonzero GLB bytes, status logs, response headers, and Blender importability.
7. Run Blender QA/contact sheets before promotion.
8. Use Quad Retro/Unreal import tooling after visual review selects the models worth promoting.
9. Stage Pixal3D-derived content through the standard standalone validation path.

### Current Production Roster v1 Evidence

The current 50-mob source-generation pass is split across two manifests:

| Batch | Rows | Success | Failure | Notes |
| --- | ---: | ---: | ---: | --- |
| Agent A | 25 | 25 | 0 | Detached Pixal3D batch on an A40 pod, 25 Blender import checks passed |
| Agent B | 25 | 25 | 0 | One transient `TreantSapling` CuMesh export failure was retried and succeeded |
| Total | 50 | 50 | 0 | All per-mob GLBs and concept PNGs are present in the production roster folders |

Both Agent A and Agent B used the locked baseline:

| Header/settings item | Value |
| --- | --- |
| `X-Seed` | 1337 |
| `X-Resolution` | 1024 |
| `X-Texture-Size` | 2048 |
| `X-Decimation` | 30000 |
| `X-Remesh` | 1 |
| `X-Image-Resolution` | 512 |
| `X-Max-Num-Tokens` | 49152 |
| `X-SS-Steps` | 12 |
| `X-Shape-Steps` | 12 |
| `X-Tex-Steps` | 12 |
| `X-Tex-Guidance` | 1.0 |

The Unreal project currently contains imported mob meshes under `/Game/Characters/Mobs`; release readiness depends on the normal runtime, visual, and standalone QA gates.

### Unreal Import Path For The 50 Mob Models

`Scripts/ImportQuadRetroEnemyVisuals.py` is the current production enemy import script. It:

1. Reads both production manifests from `Model Generation/Production/Roster_v1/AgentA` and `AgentB`.
2. Requires the expected 50 rows in `Enemies.csv`.
3. Requires manifest rows to have `Status=Success`.
4. Converts each source GLB to FBX through Blender.
5. Extracts the largest embedded image as `<EnemyID>_Texture.png`.
6. Imports the FBX as a static mesh under `/Game/Characters/Mobs/<EnemyID>/SM_<EnemyID>`.
7. Imports/renames the texture as `/Game/Characters/Mobs/<EnemyID>/T_<EnemyID>`.
8. Applies static-mesh build settings, LOD defaults, and the shared character material defaults.
9. Computes a visual scale from a target max dimension of 180 cm.
10. Upserts `CharacterVisuals.csv` with the imported static mesh, texture, yaw 90, scale, and grounding flags.
11. Marks imported enemy rows in `Enemies.csv` as `MeshReady`.
12. Reloads `/Game/Data/DT_CharacterVisuals` and `/Game/Data/DT_Enemies`.
13. Writes `Saved/QuadRetroEnemyVisualImportReport.json`.

The current saved import report says:

- `success=true`
- `count=50`
- `expected_count=50`
- `skipped_count=0`
- `destination_root=/Game/Characters/Mobs`

## Retro Post-Processing And Visual Tools

T66 has several overlapping retro presentation layers. They are related, but they do not all do the same job.

### Settings Surface

The current user-facing settings checklist says the Settings screen intentionally exposes two master toggles:

- `Frontend Retro FX`
- `Gameplay Retro FX`

Older per-effect rows are kept hidden for save compatibility and possible future advanced controls. The actual persistent struct is much more detailed than the current UI surface.

### Gameplay Retro FX Subsystem

`UT66RetroFXSubsystem` owns the main gameplay retro stack. Its settings are stored through `FT66RetroFXSettings` and applied through a transient unbound post-process volume.

Major controls and effect groups include:

- Master enable/disable.
- PS1 material stack blend.
- PS1 dithering, Bayer dithering, color LUT, color boost, and fog.
- Real/fake low-resolution behavior.
- N64 blur and replace-tonemapper path.
- Chromatic aberration and optional invert.
- World and character pixelation levels.
- Character outline.
- World/character retro geometry controls, including vertex snapping, resolution, noise, and affine settings.
- UI chrome/text/background treatment settings.
- Fullscreen UI CRT settings.

Important material and collection paths include:

- `/Game/Materials/Retro/PS1/MI_T66_PS1_C*_S*_B*`
- `/Game/Materials/Retro/M_T66_OutlinePostProcess`
- `/Game/Materials/Retro/M_RetroChromaticAberrationPostProcess`
- `/Game/Materials/Retro/MPC_T66_RetroGeometry`
- `/Game/Materials/Retro/M_Character_Unlit_RetroGeometry`
- `/Game/Materials/Retro/M_Environment_Unlit_RetroGeometry`
- `/Game/Materials/Retro/M_FBX_Unlit_RetroGeometry`
- `/Game/Materials/Retro/M_GLB_Unlit_RetroGeometry`

### Pixelation Subsystem

`UT66PixelationSubsystem` owns the post-process pixelation path and uses `/Game/UI/M_PixelationPostProcess`. It can separate world and character pixelation through custom stencil masks and receives final world/character levels from the Retro FX subsystem. Current grid sizing makes level 1 the least pixelated and level 10 the most pixelated.

### Frontend UI CRT

`UT66FrontendUIRootWidget` applies a fullscreen UI CRT material through a retainer widget:

- `/Game/UI/Materials/M_T66_UI_CRTPostProcess`

That UI path has independent values for scanline strength, phosphor mask, bloom, chromatic aberration, barrel distortion, vignette, color quantization, reference resolution, and viewport size.

### World Visual Baseline

`AT66WorldVisualSetup` creates/reuses a neutral unbound post-process volume for baseline world presentation. It disables ambient occlusion and bloom, sets restrained saturation, and removes legacy lighting actors so the retro stack has a controlled baseline.

### Pixel VFX

`UT66PixelVFXSubsystem` is separate from full-screen post processing. It manages pixel-style combat VFX using Niagara/fallback paths such as `/Game/VFX/NS_PixelParticle` and budget controls like `T66.PixelVFX.LowBudget`, `T66.PixelVFX.MediumBudget`, `T66.PixelVFX.HighBudget`, `T66.PixelVFX.UseEffectsScalability`, and `T66.PixelVFX.BudgetScale`.

## Current Gaps And Caveats

1. Full archetype behavior is not implemented yet. `Exploder`, `Strafer`, `Stutterer`, `Turret`, `Burrower`, and `Necromancer` are production data labels, but they still route through fallback family classes.
2. `FamilyID`, `RoleID`, and `Archetype` overlap. Existing pending data notes already call out that `RoleID` is mostly compatibility duplication and `FamilyID` still drives runtime class selection.
3. The Hell/Impossible core set intentionally has no normal ranged core mob; its core mix is rush, melee, flying, rush, flying.
4. Local stage 4 rows list all 10 mobs but are also marked `bBossOnlyFinale=true`; do not assume those rows mean the stage is a normal full-wave mob floor.
5. Pixal3D production roster outputs and derived assets still need final visual and runtime acceptance before being treated as final mob art.
6. The first-floor authored order is not a spawn queue. Runtime wave selection is randomized from the non-empty stage list.
