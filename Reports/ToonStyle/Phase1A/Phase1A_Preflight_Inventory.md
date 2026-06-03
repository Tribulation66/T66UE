# Phase 1A Preflight Inventory

This pass is docs-only. It inventories the gameplay/UI seams Pablo named for the proposed ToonStyle TEST room and records what already exists, what is missing, and what will affect the Phase 1A implementation prompt.

## Executive Findings

- There is no existing dedicated test-room map. The project currently has only `Content/Maps/FrontendLevel.umap` and `Content/Maps/GameplayLevel.umap`.
- There is an existing Lab route from hero selection into `GameplayLevel`, and it is the closest reusable infrastructure. It is not the same thing as the requested ToonStyle TEST room because it spawns a large floor and The Collector.
- The simplest viable TEST route is a new test branch that still opens `GameplayLevel`, skips normal procgen, spawns a small cuboid room, places the lineup, and marks the run leaderboard-ineligible.
- The requested ten assets cross several different binding paths: hero, companion, VAT/static mobs, NPC, world interactables, pickup, and display-only prop. Treating them as one uniform "generate a model" task would hide real implementation work.
- "Spider" and "Bat" are not literal production IDs. The Stage 1 rows are `TombSpider` and `CaveBat`.
- The cuboid room is feasible with existing `AStaticMeshActor` plus engine cube patterns, but there is no current six-face texture room system. It needs a small purpose-built spawner and a texture/material binding path.
- Pixal3D has quality headers and batch tools, but it has no obvious "disable grain" switch. If Pixal3D source textures are grainy, the tunable levers are prompt design, resolution, texture size, and texture sampling/guidance settings, not a known post-process flag.

## 1. Hero Selection UI And Game Flow

The hero selection screen is native Slate. The bottom-right button cluster is built in `Source/T66/UI/Screens/HeroSelection/T66HeroSelectionScreen_Build.cpp:717` through `Source/T66/UI/Screens/HeroSelection/T66HeroSelectionScreen_Build.cpp:732`.

Existing buttons:

- `DIFFICULTY`: a dropdown built from playable difficulty values at `T66HeroSelectionScreen_Build.cpp:676` through `T66HeroSelectionScreen_Build.cpp:714`. Choosing an entry calls `SelectDifficulty(Difficulty)`.
- `ENTER`: bottom-right large button at `T66HeroSelectionScreen_Build.cpp:720` through `T66HeroSelectionScreen_Build.cpp:721`. The handler resolves hero/difficulty, sets `SelectedRunCategory = ET66RunCategory::Tower`, applies party context, and calls `TransitionToGameplayLevel()` at `Source/T66/UI/Screens/T66HeroSelectionScreen.cpp:657` through `Source/T66/UI/Screens/T66HeroSelectionScreen.cpp:718`.
- `CHALLENGES`: button at `T66HeroSelectionScreen_Build.cpp:722` through `T66HeroSelectionScreen_Build.cpp:723`. It opens the Challenges modal and selects challenge content via `OpenCommunityContent(false)` at `T66HeroSelectionScreen.cpp:631` through `T66HeroSelectionScreen.cpp:653`.
- `TUTORIAL`: button at `T66HeroSelectionScreen_Build.cpp:724` through `T66HeroSelectionScreen_Build.cpp:725`. The handler mirrors the normal run setup but sets `SelectedRunCategory = ET66RunCategory::Tutorial` and leaderboard-ineligible before transitioning to gameplay at `T66HeroSelectionScreen.cpp:222` through `T66HeroSelectionScreen.cpp:247`.
- `MODS`: button at `T66HeroSelectionScreen_Build.cpp:726` through `T66HeroSelectionScreen_Build.cpp:727`. It opens the same community modal as Challenges, but selects mod content via `OpenCommunityContent(true)`.
- `LAB`: there is already a separate right-column header button at `T66HeroSelectionScreen_Build.cpp:591` through `T66HeroSelectionScreen_Build.cpp:592`. Its handler sets `SelectedRunCategory = ET66RunCategory::Lab`, marks the run leaderboard-ineligible, and transitions to gameplay at `T66HeroSelectionScreen.cpp:193` through `T66HeroSelectionScreen.cpp:219`.

Adding `TEST` below `MODS` is mechanically straightforward but the layout is tight. The current right cluster is `655 x 216`, and `MODS` already starts at `Y=141` with height `48`. A new 48-pixel button below it needs either a taller bottom row, tighter spacing, or a different right-cluster layout. This is a real UI-layout task, not just adding a handler.

Gameplay travel is centralized. `UT66GameInstance::TransitionToGameplayLevel()` always opens `GetGameplayLevelName()` after a preload delay at `Source/T66/Core/T66GameInstance.cpp:1727` through `Source/T66/Core/T66GameInstance.cpp:1754`. The current run category enum has only `Tower`, `Lab`, and `Tutorial` at `Source/T66/Core/T66RunTypes.h:17` through `Source/T66/Core/T66RunTypes.h:22`.

## 2. Test Mode Infrastructure Feasibility

There is no first-class `TestRoom`, `Sandbox`, or `DevRoom` run category today. There is a Lab mode:

- `ET66RunCategory::Lab` exists in `Source/T66/Core/T66RunTypes.h:17` through `Source/T66/Core/T66RunTypes.h:22`.
- `HandleSpecialModeBeginPlay()` detects Lab and calls `HandleLabBeginPlay()` at `Source/T66/Gameplay/GameMode/T66GameMode_Bootstrap.cpp:181` through `Source/T66/Gameplay/GameMode/T66GameMode_Bootstrap.cpp:189`.
- Lab BeginPlay marks the run leaderboard-ineligible, resets run state/damage logs, ensures setup, and schedules cleanup/warmup at `T66GameMode_Bootstrap.cpp:192` through `T66GameMode_Bootstrap.cpp:218`.
- Main map setup branches early for Lab: spawn Lab floor, neutral visual setup, spawn The Collector, spawn player start, then return at `Source/T66/Gameplay/GameMode/T66GameMode_MainMap.cpp:434` through `Source/T66/Gameplay/GameMode/T66GameMode_MainMap.cpp:440`.
- Lab floor uses an engine cube scaled to a large central floor at `Source/T66/Gameplay/GameMode/T66GameMode_Lab.cpp:16` through `Source/T66/Gameplay/GameMode/T66GameMode_Lab.cpp:62`.
- Lab can already spawn mobs, bosses, fountain, chest, idol altar, crate, and quick revive vending at `T66GameMode_Lab.cpp:149` through `T66GameMode_Lab.cpp:278`.

Lab is useful reference infrastructure, but it should not be overloaded blindly. The requested ToonStyle room needs controlled lighting, a cuboid enclosure, deterministic lineup placement, no Collector UI, and retro/post-process overrides for the validation room. The cleanest route is:

1. Add a TEST button handler that mirrors `HandleLabClicked()`.
2. Add a new test-room state. A new `ET66RunCategory::TestRoom` is clearer than reusing Lab, but it touches save/session serialization. A non-persistent `bIsToonStyleTestRoom` flag would be smaller but less durable.
3. Keep using `GameplayLevel`; no new `.umap` is required for the first pass.
4. Branch in GameMode setup before normal tower/procgen, like Lab does, and spawn the test room and lineup.

## 3. Named Asset Inventory

### Lu Bu

Lu Bu is present, but the naming is split. `Content/Data/Heroes.csv:3` has `Hero_2` with display name `Chinese Chad`, while `Source/T66/Core/T66LocalizationSubsystem.cpp` localizes `Hero_2` as Lu Bu and `Content/Data/Weapons.csv:19` through `Content/Data/Weapons.csv:35` explicitly name Lu Bu weapons. The active character-visual rows include `Hero_2_Chad` and `Hero_2_Stacy` in `Content/Data/CharacterVisuals.csv:5` and `Content/Data/CharacterVisuals.csv:82`, both pointing to QuadRetro static meshes and pixelated textures. There are also skeletal Beachgoer variants. For Phase 1A, Lu Bu is not data-only, but the clean test needs a new visual asset row or a test-only override path so production `Hero_2` is not churned while the look is still moving.

### ARIA

ARIA maps to `Companion_01`. `Content/Data/Companions.csv:2` has display name `Aria`, unlocked by default, and UI sprite references. `Content/Data/CharacterVisuals.csv:27` points `Companion_01` to skeletal mesh and animations under `/Game/Characters/Companions/Companion_01/Default`, with a Beachgoer variant at row 28. This is a higher-risk test asset than static QuadRetro props because it enters the skeletal companion path. For the first display-only TEST room pass, ARIA can be shown as a static generated mesh, but a production-equivalent companion validation needs skeletal import or an explicit acceptance that the Phase 1A room tests visual style only.

### Slime

`Slime` is a Stage 1 Dungeon enemy. `Content/Data/Enemies.csv:2` marks it `MeshReady`, and `Content/Data/Stages.csv:2` includes it in Stage 1. `Content/Data/CharacterVisuals.csv:116` maps the static mesh to `/Game/Characters/Mobs/Slime/SM_Slime` and texture `/Game/Characters/Mobs/Slime/T_Slime`. Runtime mob animation is not just that static mesh: `Content/Data/MobVertexAnimations.csv:2` maps Slime to VAT assets under `/Game/Characters/MobsVAT/Slime`. A TEST-room static lineup is easy; production-path evaluation must decide whether ToonStyle needs to support the VAT material path immediately.

### Spider

The production Stage 1 spider is `TombSpider`, display name `Tomb Spider`. `Content/Data/Enemies.csv:7` marks it `MeshReady`, and `Content/Data/Stages.csv:2` includes `TombSpider`. Static visual mapping is `Content/Data/CharacterVisuals.csv:121`, and VAT mapping is `Content/Data/MobVertexAnimations.csv:6`. Do not create a new raw `Spider` ID unless Pablo wants a separate enemy. For this scope, `Spider` should be treated as `TombSpider`.

### Bat

The production Stage 1 bat is `CaveBat`, display name `Cave Bat`. `Content/Data/Enemies.csv:5` marks it `MeshReady`, and `Content/Data/Stages.csv:2` includes `CaveBat`. Static visual mapping is `Content/Data/CharacterVisuals.csv:119`, and VAT mapping is `Content/Data/MobVertexAnimations.csv:3`. Like Spider, this should use the live ID rather than inventing `Bat`.

### Idol Altar

The idol altar is active gameplay infrastructure and also the existing Pixal3D display anchor. `AT66IdolAltar` has placeholder cube stack components plus a `VisualMesh` at `Source/T66/Gameplay/T66IdolAltar.cpp:25` through `Source/T66/Gameplay/T66IdolAltar.cpp:48`. It defaults to `/Game/World/Interactables/IdolAltar/IdolAltar_Pixal3D` at `T66IdolAltar.cpp:50` through `T66IdolAltar.cpp:52`, swaps to the imported mesh in BeginPlay, and hides the placeholder stack at `T66IdolAltar.cpp:71` through `T66IdolAltar.cpp:86`. Lab can spawn an idol altar and then call `SpawnPixalTestDisplayModelsNearIdolAltar()` at `Source/T66/Gameplay/GameMode/T66GameMode_Lab.cpp:245` through `Source/T66/Gameplay/GameMode/T66GameMode_Lab.cpp:278`. This is the safest static world object to include early because the project already treats it as a generated-asset display anchor.

### Arcade Machine

The arcade machine is a live interactable, not just a prop. Its authoritative row is `Arcade_Machine` in `Content/Data/ArcadeInteractables.json:3` through `Content/Data/ArcadeInteractables.json:45`, with display mesh `/Game/World/Interactables/Arcade/Arcade_Machine/Arcade_Machine_QuadRetro`. The runtime base class resolves arcade data and uses `DisplayMesh` as `SingleMesh` before falling back to a cube at `Source/T66/Gameplay/T66ArcadeInteractableBase.cpp:358` through `Source/T66/Gameplay/T66ArcadeInteractableBase.cpp:382`. A test-room display mesh is straightforward, but a production-equivalent arcade test also has popup arcade session behavior and random game routing behind it.

### Loot Chest

The chest is `AT66ChestInteractable`. It defaults to `/Game/World/Interactables/Chests/ChestModel/Chest_QuadRetro` at `Source/T66/Gameplay/T66ChestInteractable.cpp:17` through `Source/T66/Gameplay/T66ChestInteractable.cpp:20`. It has real gameplay behavior: mimic conversion, gold reward rolls, run-state updates, and chest reward HUD at `T66ChestInteractable.cpp:33` through `T66ChestInteractable.cpp:108`. The visual can be swapped as a static mesh, but the test room should either mark it showcase-reusable or avoid consuming/destroying it during eyeball review.

### Loot Bag

The loot bag is `AT66LootBagPickup`, and it has four per-rarity meshes rather than one universal mesh. Defaults are set at `Source/T66/Gameplay/T66LootBagPickup.cpp:92` through `Source/T66/Gameplay/T66LootBagPickup.cpp:97` for Black, Red, Yellow, and White paths under `/Game/World/LootBags/`. Runtime visuals are selected from rarity and grounded/collision-adjusted at `T66LootBagPickup.cpp:337` through `T66LootBagPickup.cpp:378`. The Phase 1A list says "one loot bag"; the implementation prompt should pick one rarity for first validation, probably Yellow or White for readability, then broaden later if needed.

### Loot Crate

There are two concepts to keep separate. `Item_LootCrate` exists as an item/stat concept in `Content/Data/Items.csv:28`. The world object is `AT66CrateInteractable`, which defaults to `/Game/World/Interactables/Crate/Crate_QuadRetro` at `Source/T66/Gameplay/T66CrateInteractable.cpp:10` through `Source/T66/Gameplay/T66CrateInteractable.cpp:14` and opens the crate HUD on interaction at `T66CrateInteractable.cpp:24` through `T66CrateInteractable.cpp:42`. For the test room, "loot crate" should mean the world `Crate` actor unless Pablo explicitly means the item icon/stat row.

### Gambler

Gambler is active gameplay content and a good test NPC surface. `Content/Data/HouseNPCs.csv:2` has the `Gambler` row. `Content/Data/CharacterVisuals.csv:77` maps `Gambler` to `/Game/Characters/NPCs/Gambler/QuadRetro/SM_Gambler_QuadRetro` and a pixelated texture. `AT66GamblerNPC` sets `NPCID = Gambler`, opens casino UI on interaction, and applies the static QuadRetro visual in `Source/T66/Gameplay/T66GamblerNPC.cpp`. GameMode spawn logic includes start/casino/tower gambler paths in `Source/T66/Gameplay/GameMode/T66GameMode_WorldInteractables.cpp`. For Phase 1A, Gambler is a strong early static-character test because it is character-like, gameplay-active, and currently static rather than skeletal.

## 4. Cuboid Plus 2D Texture Room

The project already spawns simple cube geometry in code:

- Lab floor uses the engine cube in `T66GameMode_Lab.cpp:33` through `T66GameMode_Lab.cpp:61`.
- Start-area walls have a disabled code path that would spawn cube walls and set a `BaseColor` material parameter at `Source/T66/Gameplay/GameMode/T66GameMode_MainMap.cpp:557` through `Source/T66/Gameplay/GameMode/T66GameMode_MainMap.cpp:634`.
- The generated tower terrain system uses procedural floor/wall generation and the Coherent Theme Kit, but that is heavier than the requested room.

The Coherent Theme Kit is not the same as Pablo's requested "six 2D textures on cuboids" approach. It contains generated wall/floor modules with their own meshes, base-color textures, and imported materials under `Content/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01`. It is useful as an import/material reference, but it is not a lightweight room-surface system.

Lowest-friction implementation path:

1. Spawn six `AStaticMeshActor` cube surfaces in `GameplayLevel` when the test mode starts.
2. Use one simple material or material instance with a texture parameter.
3. Assign six imported textures: floor, ceiling, north wall, south wall, east wall, west wall.
4. Control UV scale in the material or by mesh scale/texture addressing. The engine cube's default UVs may stretch differently per face, so the first implementation should visibly verify tiling/stretching before judging art quality.

This is a good validation surface, but it needs purpose-built code or editor setup. It is not already available as a named room builder.

## 5. Image Generation Prompt Patterns

There are prompt examples, but most are not ready for this ToonStyle target.

- `Content/Data/Enemies.csv` contains basic concept prompts such as "Generate a clean game enemy concept..." with colors and transparent backgrounds.
- Mini/TD/Idle archives contain prompts optimized for sprites, UI, backgrounds, or minigame readability.
- `Model Generation/Experiments/Style_Lock_V3_Mushroom_ArtStyles/Report.md` and `Model Generation/Experiments/Style_Lock_V4B_Free_Creatures/Report.md` contain closer cel-shaded prompt language: hard black outlines, flat color fills, exactly two tones per color, no painterly texture, full-body front view, no environment, no cast shadow.
- Pixal3D experiments contain useful warnings: simple silhouettes and low detail are more important than ornamental accuracy for image-to-3D.

Phase 1A prompts should not reuse retro/PSX wording. They should target clean cel-friendly reference art: full object, neutral pose, simple silhouette, hard color separation, flat fills, one shadow tone, no gradients, no texture noise, no pixel art, no background clutter.

## 6. Pixal3D Current Capability Audit

Pixal3D is present as a separate research pipeline under `Model Generation/Pixal3D/`. The server is `Model Generation/Pixal3D/Server/pixal3d_server.py`; batch and smoke runners are under `Model Generation/Pixal3D/Scripts/`.

Relevant server controls:

- Export texture size: `X-Texture-Size`, default 2048, clamped 128 to 4096 at `pixal3d_server.py:412`.
- Export decimation: `X-Decimation`, default 200000 at `pixal3d_server.py:413`.
- Export remesh: `X-Remesh`, default enabled at `pixal3d_server.py:414`.
- Export fallback: `X-Export-Fallback`, default enabled, fallback decimation default 30000 at `pixal3d_server.py:417` through `pixal3d_server.py:418`.
- Generation resolution: `X-Resolution`, accepted values 1024 or 1536 at `pixal3d_server.py:420` through `pixal3d_server.py:422`.
- Sampling/guidance controls exist for sparse structure, shape, and texture at `pixal3d_server.py:430` through `pixal3d_server.py:445`.

The batch runner exposes the same headers, including texture guidance and steps at `Model Generation/Pixal3D/Scripts/run_pixal3d_batch.py:287` through `Model Generation/Pixal3D/Scripts/run_pixal3d_batch.py:307`. Defaults there are texture size 2048, decimation 30000, remesh enabled, and export fallback enabled at `run_pixal3d_batch.py:501` through `run_pixal3d_batch.py:525`.

Important limitation: I found no server-side dither, palette quantization, or pixelation toggle in the Pixal3D generation path. If Pixal3D output textures already show grain, that grain is coming from the image-to-3D/texture generation output or from downstream QuadRetro processing, not from an obvious Pixal3D "pixelate" stage. It may be reducible with prompts, higher texture size, 1536 generation resolution, and texture guidance/step experiments, but it is not proven clean yet.

QuadRetro remains relevant only if the Phase 1A asset path uses it for retopo/normalization. If QuadRetro is used before Unreal import, the clean flat-color branch from Phase 0.5 must be built before bulk asset production. Otherwise the pipeline will reintroduce baked pixelation.

## 7. 2D Texture On Cuboid Environment Validation

Pablo's cuboid-room idea is architecturally different from the current generated dungeon visuals. Current runtime tower terrain is a procedural grid with generated kit modules, cube fallbacks, and theme-specific assets. Prior world docs state the generated kit target dimensions around wall length 1300 UU, wall depth 120 UU, wall height 1200 UU, and floor footprint 1300 UU.

The proposed room is closer to a deliberate style test: simple geometry, texture-driven presentation, no need for full 3D generated wall meshes. That is a valid direction and a lower-risk environment test than full environment mesh generation, but it has two hidden requirements:

- The texture import/material path must preserve crisp authored 2D art. Do not route these textures through QuadRetro pixelation.
- The room must be judged with anti-aliasing and retro low-resolution effects disabled. Phase 0 found project AA is currently off, so the test room may need explicit rendering settings before visual conclusions are trusted.

## Hidden Dependencies For Phase 1A

- UI layout: TEST below MODS needs bottom-row layout changes, not just a new callback.
- Run state: a new test category is clean but touches save/session serialization; a transient flag is smaller but less explicit.
- Retro settings: the test room needs an entry-time override for `bUseRealLowResolution` and `bEnableRetroFXMaster` without accidentally changing user-owned settings outside the test flow.
- Asset binding: each requested asset category has a different runtime owner. A display lineup can use static mesh actors first; production-equivalent binding is larger.
- Import path: static mesh imports preserve custom normals better than skeletal imports. Companion and animated hero validation are therefore later/higher risk.
- VAT mobs: production mobs use VAT rows. Static display meshes do not fully validate the live enemy path.
- Material path: ToonStyle material work is Phase 1B/1C, but Phase 1A should leave a clear asset/material swap seam so the test room can adopt those materials without data churn.
