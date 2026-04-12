# Tribulation 66 Project Catalogue

Snapshot date: 2026-03-21

This document is a practical map of how the current project is built, where the important systems live, and where consolidation would most improve maintainability for AI-assisted development.

## 1. High-level build model

The game is mostly built as:

1. Unreal config and a small set of Blueprint bridge assets decide which map, `GameInstance`, `GameMode`, and `PlayerController` boot.
2. C++ owns most runtime behavior.
3. DataTables under `Content/Data` drive hero, companion, item, idol, boss, stage, NPC, prop, and visual definitions.
4. Slate/UMG C++ widgets own most UI layout and logic.
5. Binary assets in `Content` supply meshes, portraits, materials, maps, fonts, and UI art.

The project is not “Blueprint-heavy gameplay.” Blueprints are mostly used as entry-point wrappers and asset bindings, while the game logic is concentrated in C++.

## 2. Boot flow and game flow

### Config roots

- `Config/DefaultEngine.ini`
  - Default map: `/Game/Maps/FrontendLevel`
  - Editor startup map: `/Game/Maps/FrontendLevel`
  - Game instance: `/Game/Blueprints/Core/BP_T66GameInstance`
- `Config/DefaultGame.ini`
  - Backend URL and dev auth values
- `Config/DefaultInput.ini`
  - Legacy input mappings plus Enhanced Input classes
- `Config/DefaultT66Rng.ini`
  - Run RNG and reward tuning

### Primary runtime chain

1. Unreal starts in `FrontendLevel`.
2. `BP_T66GameInstance` instantiates `UT66GameInstance`.
3. `UT66GameInstance` preloads DataTables and menu assets, stores run setup selections, and later transitions into gameplay.
4. Frontend uses `AT66FrontendGameMode` and `AT66PlayerController`.
5. Frontend UI is driven by `UT66UIManager` plus `UT66ScreenBase`-derived screens.
6. When the player enters a run, `UT66GameInstance::TransitionToGameplayLevel()` preloads gameplay assets and opens `GameplayLevel`.
7. `AT66GameMode` assembles the stage, spawns the hero and companion, and wires world systems.
8. `AT66PlayerController` switches into gameplay mode, creates the HUD, and manages overlays and input.
9. `UT66RunStateSubsystem` becomes the authoritative model for the active run.

### Maps

- `Content/Maps/FrontendLevel.umap`
  - Main menu and preview-stage world
- `Content/Maps/GameplayLevel.umap`
  - Normal runtime level, procedural stage assembly
- `Content/Maps/LabLevel.umap`
  - Practice/testing sandbox

## 3. Core runtime architecture

### `Source/T66/Core`

This folder is the project’s service layer. It contains persistent state, profile/save systems, rendering preferences, progression, and helper registries.

### Most important files

- `T66GameInstance.h/.cpp`
  - Persistent app-level state
  - Owns DataTable references and asset preloading
  - Stores selected hero, companion, difficulty, party size, skins, run seed, mode flags, and save/load transition state
  - One of the main roots of the game

- `T66RunStateSubsystem.h/.cpp`
  - Authoritative run-state model
  - Holds HP, gold, debt, difficulty, timers, score, inventory, idols, vendor state, hero progression, status effects, survival state, tutorial state, and dev toggles
  - Broadcasts many delegates to HUD, overlays, and gameplay systems
  - This is the central gameplay state container

- `T66PlayerSettingsSubsystem.h/.cpp`
  - Persistent local settings
  - Gameplay, audio, HUD, graphics, media viewer, retro FX
  - Used by settings UI and world rendering

- `T66LocalizationSubsystem.h/.cpp`
  - Code-based localization layer
  - Huge getter surface for UI/game text
  - Centralized but very large

- `T66CharacterVisualSubsystem.h/.cpp`
  - Stable gameplay ID to skeletal mesh/animation resolver
  - Applies imported visuals from `DT_CharacterVisuals`
  - Lets runtime actors stay data-driven instead of hard-wiring asset refs

- `T66BackendSubsystem.h/.cpp`
  - HTTP integration with the Vercel backend
  - Run submission, ranks, account status, leaderboard fetches, run summaries

- `T66LeaderboardSubsystem.h/.cpp`
  - Local leaderboard model, menu leaderboard assembly, run-summary handoff
  - Bridges local saves and backend responses

- `T66SaveSubsystem.h/.cpp`
  - Slot-based run-save management
  - Maintains slot index metadata

### Supporting subsystems

- `T66AchievementsSubsystem`
  - Profile progression, AC currency, tutorial completion, union progression, lab unlocks
- `T66SkinSubsystem`
  - Unified skin ownership/equip logic for heroes and companions
- `T66CompanionUnlockSubsystem`
  - Persistent companion unlock state
- `T66PowerUpSubsystem`
  - Meta-progression/power-up state
- `T66RngSubsystem`
  - Run-seeded and luck-biased RNG API
- `T66PropSubsystem`
  - Spawns scatter props from `DT_Props`
- `T66ActorRegistrySubsystem`
  - Registry to avoid expensive world scans for common gameplay lookups
- `T66EnemyPoolSubsystem`
  - Reuse/pooling support for enemies
- `T66DamageLogSubsystem`
  - Tracks structured combat/run logs
- `T66FloatingCombatTextSubsystem` and `T66FloatingCombatTextPoolSubsystem`
  - Floating combat text creation and pooling
- `T66HeroSpeedSubsystem`
  - Movement-speed interpretation used by hero/companion motion and animation
- `T66MediaViewerSubsystem`, `T66WebView2Host`, `T66WebImageCache`
  - Embedded short-form media viewer plumbing
- `T66MusicSubsystem`
  - Music state
- `T66RetroFXSubsystem`, `T66PosterizeSubsystem`, `T66PixelationSubsystem`, `T66PixelVFXSubsystem`
  - Retro/post-process presentation stack
- `T66UITexturePoolSubsystem`
  - Keeps UI textures alive for Slate brushes
- `T66LagTrackerSubsystem`, `T66SkillRatingSubsystem`, `T66SteamHelper`
  - Support and integration utilities

### Data definitions

- `Source/T66/Data/T66DataTypes.h`
  - Single major shared data-schema file
  - Defines:
    - `FHeroData`
    - `FCompanionData`
    - `FItemData`
    - `FBossData`
    - `FStageData`
    - `FIdolData`
    - `FHouseNPCData`
    - `FLoanSharkData`
    - `FT66CharacterVisualRow`
    - `FT66PropRow`
    - leaderboard, achievement, skin, inventory, and related structs

This file is effectively the design schema of the game.

## 4. Gameplay architecture

### `Source/T66/Gameplay`

This folder contains world actors, the main run flow, player/avatar logic, enemies, bosses, interactables, and preview/test actors.

### Composition roots

- `T66GameMode.h/.cpp`
  - Main gameplay orchestrator
  - Largest architecture hotspot
  - Responsibilities currently include:
    - run start/reset logic
    - selected hero/companion spawn
    - procedural map generation
    - theme and fog setup
    - stage content spawn order
    - interactable/NPC placement
    - stage effects and tutorial setup
    - boss flow
    - Coliseum/Lab/demo/tutorial branches
    - stage regeneration

- `T66PlayerController.h/.cpp`
  - Second major orchestration root
  - Handles:
    - frontend UI bootstrap
    - gameplay input setup
    - HUD creation
    - pause menu
    - world dialogue
    - vendor/gambler/casino/collector/cowardice/load-preview overlays
    - scoped ultimate behavior
    - media viewer toggles

### Player and companion

- `T66HeroBase`
  - Playable hero base actor
  - Camera boom and follow camera
  - Placeholder and skeletal visuals
  - Safe-zone behavior
  - attack-range rings
  - cooldown widget
  - dash, stage slide, terrain recovery, sky-drop entry, quick-revive visuals

- `T66CombatComponent`
  - Best-separated combat logic unit in the project
  - Auto-attacks, ultimates, target locking, overlap-based enemy tracking, effective stat recompute, VFX/SFX, idol/DOT application

- `T66CompanionBase`
  - Companion follow actor
  - Preview mode and gameplay mode
  - Skeletal/placeholder visuals
  - Healing and union-progression hooks

- `T66RecruitableCompanion`
  - In-run companion replacement actor

- `T66PilotableTractor`
  - Vehicle/mount behavior

### Enemy and boss layer

- `T66EnemyBase`
  - Standard enemy base
  - HP, armor, slows, confusion, flee logic, leash/catch-up logic, stage scaling, difficulty scaling, loot drop, floating combat text

- `T66EnemyDirector`
  - Stage wave spawner
  - Uses timer-driven staggered spawning, alive caps, miniboss chance, goblin thief chance

- `T66EnemyAIController`
  - Supporting AI controller

- `T66BossBase`
  - Dormant-until-awakened chase/projectile boss
  - Can spawn ground AOEs
  - Drives boss UI state through run state

- Specialized enemies and bosses
  - `T66GoblinThiefEnemy`
  - `T66ChestMimicEnemy`
  - `T66UniqueDebuffEnemy`
  - `T66UniqueDebuffProjectile`
  - `T66VendorBoss`
  - `T66GamblerBoss`
  - `T66BossProjectile`
  - `T66BossGroundAOE`

### World/NPC/interactable layer

- `T66WorldInteractableBase`
  - Generic press-`F` interactable base
  - Trigger volume, prompt text, rarity, imported mesh support

- `T66HouseNPCBase`
  - Corner-house NPC base
  - Safe-zone sphere, interaction sphere, name text, optional skeletal visuals

- NPC subclasses
  - `T66VendorNPC`
  - `T66GamblerNPC`
  - `T66SaintNPC`
  - `T66OuroborosNPC`
  - `T66LoanShark`
  - `T66TricksterNPC`
  - `T66LabCollector`

- Interactable subclasses
  - `T66ChestInteractable`
  - `T66CrateInteractable`
  - `T66CasinoInteractable`
  - `T66WheelSpinInteractable`
  - `T66IdolAltar`
  - `T66QuickReviveVendingMachine`
  - `T66FountainOfLifeInteractable`
  - `T66TreeOfLifeInteractable`
  - `T66TeleportPadInteractable`
  - `T66StageCatchUpGoldInteractable`
  - `T66StageCatchUpLootInteractable`

### Stage/world assembly

- `T66ProceduralLandscapeGenerator`
  - Procedural terrain/landscape generation
- `T66ProceduralLandscapeParams`
  - Map-generation parameter structs
- `T66StageEffects`
  - Stage effect actors/logic
- `T66MiasmaManager`, `T66MiasmaTile`, `T66MiasmaBoundary`
  - Miasma zone system
- `T66StartGate`, `T66StageGate`, `T66BossGate`, `T66CowardiceGate`, `T66StageCatchUpGate`
  - Run flow and traversal gates
- `T66SpawnPlateau`
  - Grounded placement helper for NPC/interactable islands
- `T66DifficultyTotem`
  - Difficulty escalation interactable
- `T66MegabonkFarm`
  - Special map/mode support
- `T66QuakeSkyActor`, `T66EclipseActor`, `T66LavaPatch`
  - Presentation and hazard/world-effect actors

### Frontend preview actors

- `T66FrontendGameMode`
  - Frontend world/game flow
  - Positions preview camera for hero vs companion screens

- Preview-stage actors
  - `T66HeroPreviewStage`
  - `T66CompanionPreviewStage`
  - `T66PreviewStageEnvironment`
  - `T66PreviewMaterials`

### Tutorial/testing actors

- `T66TutorialManager`
- `T66TutorialPortal`
- `T66TutorialPromptActor`
- Lab flow is also wired through `T66GameMode` and `T66LabCollector`

## 5. UI architecture

### `Source/T66/UI`

The UI is primarily C++ Slate/UMG, not designer-authored widget graphs. This is important: UI structure lives in code.

### Core UI spine

- `T66UIManager`
  - Screen registration, caching, modal flow, back stack, rebuilds

- `T66ScreenBase`
  - Base class for menu screens
  - Standard lifecycle:
    - `OnScreenActivated`
    - `OnScreenDeactivated`
    - `RefreshScreen`
    - `BuildSlateUI`

- `Style/T66Style`
  - Central design system
  - Colors, spacing, fonts, button/panel/dropdown factories, decorative border treatments, debounce handling
  - One of the cleaner centralization points in the codebase

### Frontend screens

- `Screens/T66MainMenuScreen`
  - Front door, leaderboard panel, navigation to game/settings/etc.
- `Screens/T66PartySizePickerScreen`
  - new/load flow continuation
- `Screens/T66SaveSlotsScreen`
  - save/load slots and preview flow
- `Screens/T66HeroSelectionScreen`
  - hero carousel, body type, skins, preview, difficulty, lab/demo/enter flow
- `Screens/T66CompanionSelectionScreen`
  - companion selection and preview flow
- `Screens/T66HeroGridScreen`
  - alternate hero browsing
- `Screens/T66CompanionGridScreen`
  - alternate companion browsing
- `Screens/T66LobbyScreen`
  - multiplayer/lobby-facing flow scaffolding
- `Screens/T66LanguageSelectScreen`
  - language picker
- `Screens/T66SettingsScreen`
  - large tabbed settings system
- `Screens/T66AchievementsScreen`
  - profile achievements and AC flow
- `Screens/T66PowerUpScreen`
  - meta progression/power-up flow
- `Screens/T66AccountStatusScreen`
  - restriction/appeal status
- `Screens/T66RunSummaryScreen`
  - run result and proof/report viewer
- `Screens/T66ReportBugScreen`
  - bug-report path
- `Screens/T66PlayerSummaryPickerScreen`
  - select which player snapshot to inspect

### Modals

- `T66PauseMenuScreen`
- `T66QuitConfirmationModal`
- `T66LobbyReadyCheckModal`
- `T66LobbyBackConfirmModal`

### In-run HUD and overlays

- `T66GameplayHUDWidget`
  - Main in-run HUD composition root
  - Very large
  - Combines:
    - economy
    - score
    - timers
    - boss bar
    - loot prompt
    - pickup cards
    - tutorial hints
    - scoped overlays
    - quick-revive overlay
    - idol slots
    - portrait and cooldown bar
    - stat panel
    - inventory strip
    - minimap/full map
    - dev toggles
    - media viewer placeholder
    - wheel and crate presentations
    - achievement notifications
    - world dialogue

- Overlay widgets
  - `T66VendorOverlayWidget`
  - `T66GamblerOverlayWidget`
  - `T66CasinoOverlayWidget`
  - `T66CollectorOverlayWidget`
  - `T66LabOverlayWidget`
  - `T66IdolAltarOverlayWidget`
  - `T66WheelOverlayWidget`
  - `T66CrateOverlayWidget`
  - `T66CowardicePromptWidget`
  - `T66LoadPreviewOverlayWidget`
  - `T66LoadingScreenWidget`
  - `T66EnemyHealthBarWidget`
  - `T66FloatingCombatTextWidget`
  - `T66HeroCooldownBarWidget`

### Reusable UI pieces

- `Components/T66LeaderboardPanel`
- `Components/T66LeaderboardFilterButton`
- `Components/T66Button`
- `T66StatsPanelSlate`
- `T66ItemCardTextUtils`
- `T66SlateTextureHelpers`

## 6. How design data is organized and written

### Source of truth

The game’s balancing and catalog data are mostly authored as CSV/DataTable assets:

- `Content/Data/Heroes.csv` and `DT_Heroes`
- `Content/Data/Companions.csv` and `DT_Companions`
- `Content/Data/Items.csv` and `DT_Items`
- `Content/Data/Idols.csv` and `DT_Idols`
- `Content/Data/Bosses.csv` and `DT_Bosses`
- `Content/Data/Stages.csv` and `DT_Stages`
- `Content/Data/HouseNPCs.csv` and `DT_HouseNPCs`
- `Content/Data/LoanShark.csv` and `DT_LoanShark`
- `Content/Data/CharacterVisuals.csv` and `DT_CharacterVisuals`
- `Content/Data/Props.csv` and `DT_Props`
- leaderboard target/friend/streamer tables

### What each table controls

- `Heroes`
  - hero identity, portraits, movement, base stats, growth ranges, attack category, ultimate, passive
- `Companions`
  - companion identity, unlock state, portraits, spawn class
- `Items`
  - item stat lines, icons, economy values
- `Idols`
  - independent attack sources and scaling
- `Bosses`
  - boss class, stats, awaken distance, projectile tuning, point values
- `Stages`
  - stage number, boss, stage effect, enemy roster
- `HouseNPCs`
  - safe-zone size, gambler payout, NPC identity
- `LoanShark`
  - debt collector scaling
- `CharacterVisuals`
  - stable ID to imported mesh and animation set
- `Props`
  - scatter-prop spawn tuning

### Design implication

The project’s “game design” is split between:

- static data in `Content/Data`
- runtime interpretation in `UT66GameInstance`, `UT66RunStateSubsystem`, `AT66GameMode`, `UT66CombatComponent`, and various UI widgets

So if you want to understand balancing or content breadth, inspect `Content/Data`.
If you want to understand how those numbers become behavior, inspect the runtime C++ classes.

## 7. Content catalogue

Top-level `Content` snapshot:

- `Characters` - 467 files
- `UI` - 262 files
- `World` - 249 files
- `UE5RFX` - 289 files
- `Slate` - 135 files
- `Items` - 128 files
- `Idols` - 120 files
- `Data` - 30 files
- `Maps` - 3 files
- plus audio, materials, VFX, localization, blueprint bridge assets, and Unreal external-actor folders

### Important content folders

- `Content/Blueprints/Core`
  - bridge assets for `GameInstance` and `PlayerController`
- `Content/Blueprints/GameModes`
  - frontend and gameplay game-mode bridge assets
- `Content/Blueprints/UI`
  - menu widget blueprints used as wrappers/entry assets

- `Content/Characters/Heroes`
  - hero portraits, meshes, animations, related assets
- `Content/Characters/Companions`
  - companion visuals and related assets
- `Content/Characters/Enemies`
  - enemy visuals
- `Content/Characters/NPCs`
  - NPC visuals

- `Content/UI/Assets`
  - UI chrome textures and shared art
- `Content/UI/Materials`
  - UI materials
- `Content/UI/Sprites`
  - portrait/icon/sprite-heavy UI art
- `Content/UI/MainMenu`
  - menu-specific visual assets

- `Content/World`
  - props, interactables, ground, cliffs, sky, loot bags

- `Content/Slate`
  - fonts and Slate-loaded UI resources

- `Content/Localization/T66`
  - localization outputs

### Special/legacy/vendor content areas

- `Content/UE5RFX`
  - third-party FX/content pack
  - third-party or imported art
- `Content/T66MapAssets`
  - project-specific world assets
- `Content/__ExternalActors__`
  - Unreal World Partition/external actor storage
- `Content/__ExternalObjects__`
  - Unreal external object storage
- `Content/__CodexRetroScratch`
  - scratch/generated work area
- `Content/__Debug`
  - debug content

## 8. Scripts and tooling catalogue

### `Scripts`

Utility scripts are mostly editor/import/build helpers rather than gameplay runtime:

- `Import*`
  - mesh, portrait, idol, terrain, prop, and UI imports
- `Setup*`
  - DataTable setup, material setup, retro UI/material setup
- `Inspect*`
  - editor/material/import inspection helpers
- `Run*`
  - run specific import/verify flows and exit Unreal
- `Fix*`, `Repair*`, `Restore*`, `Flatten*`, `Generate*`, `Build*`, `Create*`, `Dump*`
  - maintenance and content-pipeline repair scripts

This is a content-production toolbox. It is not part of the shipping runtime architecture.

### `Source/T66Editor`

- Editor-only module
- Registers menu tools for dev/editor workflows
- Includes level/setup helpers for demo/lab workflows

## 9. Current organization strengths

- C++ owns the real behavior, which is easier to reason about than scattered Blueprint logic.
- DataTables centralize a lot of game design data.
- `FT66Style` is a real design system, not ad hoc styling.
- `UT66CombatComponent` is a good example of behavior isolated into a reusable component.
- `UT66CharacterVisualSubsystem` cleanly separates gameplay IDs from imported art assets.
- Core service responsibilities are at least mostly grouped in `Source/T66/Core`.

## 10. Main organization problems

### 1. `T66GameMode` is over-centralized

It currently mixes:

- run lifecycle
- world assembly
- map generation
- mode branching
- NPC/interactable placement
- tutorial and lab setup
- boss flow
- special stage logic

This is the biggest maintainability problem in the project.

### 2. `T66PlayerController` is also over-centralized

It mixes:

- frontend screen bootstrap
- gameplay input
- overlay ownership
- world dialogue
- special camera behavior
- media viewer
- HUD setup

This creates a second giant “god file.”

### 3. `T66GameplayHUDWidget` is effectively an entire HUD application in one file

It is handling too many separate concerns in a single widget tree and source file.

### 4. `T66RunStateSubsystem` is a good central model, but it is too broad

It is useful as a single authoritative state surface, but too much business logic is living directly inside it.

### 5. `T66LocalizationSubsystem` is too large

The compile-safe getter approach is fine, but one giant localization class is hard to navigate and hard to extend.

### 6. Vendor/Gambler/Casino flows overlap

There is clear reuse across:

- economy display
- inventory strip
- buyback logic
- stats panel
- anger state
- tab shell

But the implementation is still spread across multiple large overlay files.

### 7. Content folders mix first-party, third-party, scratch, and generated assets

That increases search noise and makes AI suggestions less precise.

## 11. Recommended reorganization plan

These are ordered by impact.

### Priority 1: split `T66GameMode` into domain coordinators

Keep `AT66GameMode` as the entry orchestrator, but move logic into dedicated helpers:

- `T66RunFlowCoordinator`
  - fresh run vs stage transition vs retry vs lab/demo/coliseum decision logic
- `T66WorldAssembler`
  - terrain-ready spawn sequence
- `T66StageContentSpawner`
  - interactables, gates, corner houses, NPCs, catch-up content
- `T66BossFlowCoordinator`
  - boss spawn, awaken/beacon, defeat handling, coliseum debt/owed-boss flow
- `T66SpecialModesCoordinator`
  - Lab, tutorial arena, Coliseum, Megabonk farm

The goal is not more abstraction for its own sake. The goal is to stop one file from owning every stage concern.

### Priority 2: split `T66PlayerController` by mode and overlay role

Recommended slices:

- `T66FrontendUIControllerComponent`
  - screen bootstrap, initial screen flow, menu-only input mode
- `T66GameplayInputControllerComponent`
  - gameplay bindings and input handling
- `T66GameplayOverlayControllerComponent`
  - vendor/gambler/casino/collector/cowardice/load-preview overlays
- `T66WorldDialogueControllerComponent`
  - open-world dialogue and screen-position syncing
- `T66ScopedUltControllerComponent`
  - hero-one scoped view/camera state

If you do not want multiple actor components, at minimum split the `.cpp` into clearly separated private helper files.

### Priority 3: break `T66GameplayHUDWidget` into subwidgets

Suggested subwidgets:

- `T66HudTopBarWidget`
  - economy, stage, score, speedrun
- `T66HudBossWidget`
  - boss bar and boss-state presentation
- `T66HudInventoryWidget`
  - inventory strip, idol slots, pickup cards
- `T66HudPortraitWidget`
  - portrait, cooldown, hero stats, passive/ultimate
- `T66HudMapWidget`
  - minimap/full map
- `T66HudPromptWidget`
  - tutorial hint, loot prompt, world dialogue
- `T66HudPresentationWidget`
  - crate, wheel, achievements, scoped overlays, quick revive
- `T66HudMediaWidget`
  - TikTok/media viewer area

This is one of the highest-value changes for Codex efficiency.

### Priority 4: keep `UT66RunStateSubsystem` as façade, but split its domains

Do not remove the single access point. Keep that.

Internally split logic into state domains or helper structs:

- Economy and Debt
- Inventory and Buyback
- Idols and Idol Stock
- Hero Progression and Derived Stats
- Timers, Score, Stage, Speedrun
- Status Effects and Survival
- Difficulty, Cowardice, Coliseum Debt/Boss Debt
- Tutorial and Dev toggles

That preserves central access while reducing edit risk.

### Priority 5: split localization by domain

Keep the public API style if you like it, but group implementation and declarations into:

- MainMenu
- Settings
- GameplayHUD
- Overlays
- Achievements
- RunSummary
- Shared terms

A generated header/source pass from a localization data file would be even better.

### Priority 6: consolidate NPC overlay infrastructure

Create shared pieces for:

- top bar
- anger widget
- inventory strip
- stats panel
- buyback list
- bank controls

Then keep only game-specific logic in:

- vendor page logic
- gambler minigame logic
- alchemy logic

### Priority 7: make content ownership clearer

Recommended top-level content policy:

- `Content/Game/*`
  - first-party shipping content
- `Content/ThirdParty/*`
  - imported packs like `UE5RFX`
- `Content/Generated/*`
  - script-produced/generated assets
- `Content/Scratch/*`
  - temporary Codex/editor experiments

Even if you do not move everything immediately, adopting this policy for new work will reduce future entropy.

### Priority 8: reduce file-search ambiguity with explicit naming

Prefer names that encode domain and role:

- `T66VendorShopPanel`
- `T66VendorBankPanel`
- `T66CasinoAlchemyPanel`
- `T66GameplayHudBossPanel`

This helps AI retrieve the right file on the first search.

## 12. Practical AI-efficiency recommendations

If the goal is “fewer files and more organization so Codex works faster,” the best version is not literally “put everything into fewer giant files.”

The better target is:

- fewer giant multi-domain files
- more stable composition roots
- fewer places where the same concept is implemented differently
- clear folder ownership
- consistent naming
- small shared UI and gameplay primitives

For AI-assisted iteration, the worst pattern is a single 4000-6000 line file with unrelated responsibilities.
The best pattern is:

- one obvious root file per domain
- a few small helpers under it
- one source of truth for each concept

## 13. Suggested target structure

One workable future structure:

```text
Source/T66
  Core
    App
    Profile
    Save
    Online
    Localization
    Presentation
    RuntimeState
  Gameplay
    Flow
    Player
    Combat
    Enemies
    Bosses
    NPCs
    Interactables
    World
    Modes
    Preview
  UI
    Framework
    Style
    Frontend
    HUD
    Overlays
    Shared
  Data
```

This keeps the number of files reasonable while making ownership much clearer.

## 14. Best next refactors

If you want the highest payoff with the least disruption, do these first:

1. Split `T66GameplayHUDWidget`.
2. Split `T66PlayerController` overlay and input responsibilities.
3. Split `T66GameMode` spawn/world-assembly logic.
4. Break `UT66RunStateSubsystem` into internal domain modules while keeping its public façade.
5. Extract shared vendor/gambler/casino UI pieces.
6. Move non-shipping asset folders under a clearer content ownership scheme.

## 15. Quick file hotspot list

Largest files found during this audit:

- `Source/T66/Gameplay/T66GameMode.cpp`
- `Source/T66/UI/T66GameplayHUDWidget.cpp`
- `Source/T66/UI/T66GamblerOverlayWidget.cpp`
- `Source/T66/Gameplay/T66ProceduralLandscapeGenerator.cpp`
- `Source/T66/Gameplay/T66PlayerController.cpp`
- `Source/T66/Core/T66RunStateSubsystem.cpp`
- `Source/T66/UI/Screens/T66SettingsScreen.cpp`

These are the first files to target when reducing cognitive load.

## 16. Bottom line

Current architecture summary:

- The game boots through a small Blueprint bridge into a mostly C++ runtime.
- `UT66GameInstance`, `AT66GameMode`, `AT66PlayerController`, and `UT66RunStateSubsystem` are the core execution spine.
- UI is mostly code-built Slate/UMG with `FT66Style` as the design-system center.
- Content and balancing are strongly data-driven through `Content/Data` DataTables.
- The main organizational problem is not “too many files.” It is “too many unrelated responsibilities concentrated in a handful of giant files.”

If you optimize for Codex and long-term speed, the best move is selective decomposition around domain boundaries, not flattening everything into fewer god files.
