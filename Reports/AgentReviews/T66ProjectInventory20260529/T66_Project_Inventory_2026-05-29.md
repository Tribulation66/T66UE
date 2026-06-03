# T66 / Chadpocalypse Current Project Inventory

This document inventories what currently exists in the project. It does not assess quality, performance, balance, soundness, readiness, or cleanup priority.

## Section 1: What's In The Game

### Counts Up Front

- 1 current project version label: alpha-0.8.
- 11 map files total: 2 core game maps and 9 effects/demo/benchmark maps.
- 20 main stages across 5 themed worlds: Dungeon, Forest, Ocean, Martian, and Hell.
- 12 playable heroes.
- 16 companions, with 4 marked as unlocked by default.
- 60 authored standard enemy rows, grouped by role into 20 melee, 16 ranged, 13 rush, and 11 flying enemies.
- 23 boss characters across 20 boss encounters.
- 1 unique Backrooms enemy, 3 world NPCs, and 1 loan-shark debt collector.
- 192 hero weapons: 16 per hero, split evenly across 4 rarities and 4 attack branches.
- 30 shop/stat/special items.
- 12 idols.
- 12 status-effect entries, including the "None" entry.
- 14 arcade interactable entries: 1 random arcade machine plus 13 playable arcade games.
- 4 casino gambling games.
- 4 separate side-game modes: Mini, Tower Defense, Idle, and Deck Builder.
- 37 visible UI screen or modal entries across the main game and side-game menus.
- 96 data-driven audio events, 149 Unreal audio assets, and 5 loose OGG source audio files.

### Maps, Levels, And Environments

- The project has 2 core maps: a front-end/menu map and the main gameplay map.
- The main run is organized as a 20-stage descent across 5 themes. Each theme has 4 stages, and each theme maps to one difficulty band: Dungeon/Easy, Forest/Medium, Ocean/Hard, Martian/Very Hard, and Hell/Impossible.
- The themed worlds are Dungeon, Forest, Ocean, Martian, and Hell. Each world has stage definitions and a boss encounter path.
- Special player spaces and interactions include the Backrooms room, casino, shop/vendor areas, altar areas, gates, and the tower-descent hole.
- The project also contains 9 effects/demo/benchmark maps from visual-effects packs. These are separate from the two core game maps.

### Characters

- The 12 playable heroes are Founding Chad, Chinese Chad, Boxer Chad, Billy Chad, Yakub Chad, Robo Chad, Rabbit Chad, CS Chad, Goblino Chad, Monotone Chad, Bald Chad, and Roach Chad.
- Hero attack identities are grouped into 4 primary categories: Pierce, AOE, Bounce, and DOT.
- The companion roster has 16 companions, with 4 available by default and 12 marked as locked/unlockable.
- Standard enemies exist as 60 authored rows across 5 world themes. At a high level they are grouped as melee, ranged, rush, and flying enemies.
- The detailed per-creature enemy breakdown is handled by the separate enemy roster review; this inventory stays at category and count level.
- Boss content has 23 boss characters across 20 stage encounters. The difference comes from the Four Horsemen encounter, which uses 4 boss characters in one stage encounter.
- Boss encounter types include single-boss fights, world-finale Chad fights, a multi-boss Horsemen fight, and the final Great Dragon encounter.
- Special nonstandard hostile content includes the Backrooms Stalker and the Loan Shark debt collector.
- World NPC content includes a Casino host, a Saint, and Ouroboros.

### Game Modes And Experiences

- The main experience is the tower-descent roguelike run across the 20 stages.
- Demo-mode gating exists for a restricted build path. The demo configuration allows 5 heroes, Easy difficulty, 4 companions, 4 arcade games, and 3 casino games.
- Practice/Lab and Daily Descent/Climb style experiences are represented in the game-mode and UI structure.
- The Backrooms experience exists as a special room/mode with its own enemy and quick-revive item path.
- The casino experience exists with a casino NPC, gambling UI, casino games, debt, and the loan-shark mechanic.
- The project contains a Minigames hub and 4 separate side-game modes: Mini Chadpocalypse, Chadpocalypse Tower Defense, Chadpocalypse Idle, and Chadpocalypse Deck Builder.
- A Versus Arcade screen entry also exists in the UI flow.

### Economy And Progression

- Gold is the main shop currency visible in item pricing. The run also tracks progression values such as XP, score, timers, inventory, weapons, idols, and debt-related state.
- The item roster has 30 entries. These cover damage, attack speed, attack scale, accuracy, evasion, armor, luck, and special items.
- Special item entries include loot containers, the Backrooms quick-revive item, and a vendor token.
- There are 12 idols, split evenly across DOT, Bounce, AOE, and Pierce categories.
- There are 192 weapons: 12 heroes times 16 weapons each. Weapon entries are evenly split across Black, Red, Yellow, and White rarities, and evenly split across Pierce, AOE, Bounce, and DOT branches.
- The project has 12 status-effect rows, including "None" plus effects such as webbed, poisoned, rooted, thorned, bleeding, armor cracked, cursed, shocked, chilled, burning, and dazed.
- Leaderboard target data exists for score targets and speedrun targets. Score targets are organized by difficulty and party size, and speedrun targets are organized by difficulty and stage.
- Companion unlock state, player profile saves, run saves, settings saves, local leaderboard saves, and side-game saves all exist as part of progression and persistence.

### Player-Facing Systems

- Shop/vendor purchasing exists through vendor interactables and shop item data.
- Idol altars and weapon altars exist as player-facing upgrade/choice systems.
- Gates include start gates, stage gates, boss gates, a Cowardice gate, and tutorial gates.
- The tower-descent hole exists as a run-progression interaction.
- Loot interaction types include chests, crates, loot bags, loot wheels, boosts, and fountains.
- Casino play includes coin flip, rock-paper-scissors, blackjack, and find-the-ball.
- Arcade play includes 13 games: Whack-a-Mole, Topwar, Gold Miner, Rune Swipe, Cart Switcher, Crystal Dash, Potion Pour, Relic Stack, Shield Parry, Mimic Memory, Bomb Sorter, Lantern Leap, and Blade Sweep.
- Other player-facing world systems include vehicles, traps, a miasma boundary, combat hit feedback, floating combat text, pixel/retro visual effects, and interaction prompts.

### Backend And Online

- Online-facing systems include account status, Steam integration, party/invite APIs, leaderboard submission, daily climb data, run submission, run summaries, achievements, and skill rating.
- The project has separate Steam app IDs configured for the full game and demo.
- Local saves exist for profiles, runs, player settings, leaderboards, companion unlocks, buffs, community content, and minigame profiles/runs.
- Run integrity and anti-cheat-style validation exist as player-facing backend support for submitted runs.
- Telemetry-style systems exist for damage logging, score/run telemetry, and lag tracking.

### Audio, Music, And UI

- The audio set includes 96 named audio events, 149 Unreal audio assets, and 5 loose OGG source audio files.
- Sound content is organized around UI feedback, combat sounds, arcade sounds, sound effects, and OST folders.
- The UI exposes 37 visible screen or modal entries across the main game and side games.
- Main UI areas include main menu, save slots, hero selection, companion selection, settings, achievements, minigames, pause, report bug, run summary, power-up, hero grid, companion grid, language select, quit confirmation, party invite, account status, player summary picker, save preview, challenges, daily descent, versus arcade, and game over.
- Side-game UI areas include Mini menus and battle, Tower Defense menus and battle, Idle main menu, and Deck Builder main menu.
- In-run UI includes HUD presentation, floating combat text, pickup/reward presentation, interaction prompts, and combat feedback.

### Other Encountered Content

- The project contains character visual/skin data with 133 visual rows.
- The project contains vehicle interactable definitions, world visual prop definitions, arcade machine definitions, and combat VFX binding data.
- Visual content includes hero portraits, companion portraits, weapon and idol sprites, world props, toon/retro/pixel visual systems, and imported model/mesh assets.
- Video-generation and media-viewer systems exist as part of the broader project content pipeline and frontend/media support.

## Section 2: How It's Built

### Core Architecture

- Engine and language: Unreal Engine 5.7, C++.
- Project descriptor: `T66.uproject`.
- Runtime/editor modules: `T66`, `T66Mini`, `T66TD`, `T66Idle`, `T66Deck`, and `T66Editor`.
- Enabled plugins include `OnlineSubsystemSteam`, `SocketSubsystemSteamIP`, `MovieRenderPipeline`, `AnimToTexture`, `ElectraPlayer`, `ProceduralMeshComponent`, `ModelingToolsEditorMode`, `PythonScriptPlugin`, and `EditorScriptingUtilities`.
- Main game state is subsystem-driven. Core systems live under `Source/T66/Core`.
- Main gameplay classes live under `Source/T66/Gameplay`.
- Main UI classes live under `Source/T66/UI`.
- Front-end gameplay uses `AT66FrontendGameMode`; main gameplay uses `AT66GameMode`.
- `AT66GameMode` is split into domain files under `Source/T66/Gameplay/GameMode`: Backrooms, Bootstrap, BossFlow, Lab, MainMap, Spawning, TestRoom, Tower, and WorldInteractables.

### Core Subsystems And Managers

- `UT66RunStateSubsystem` is the central run-state system. It carries hearts, gold, inventory, idols, weapons, score, timers, economy/inventory state, combat state, and run telemetry.
- `UT66SessionSubsystem`, `UT66StageProgressionSubsystem`, and `UT66ActorRegistrySubsystem` support run/session flow and registered runtime actors.
- `UT66SaveSubsystem` manages save data with save-game classes such as run saves, profile saves, player settings, local leaderboard saves, companion unlock saves, buff saves, and community-content saves.
- `UT66LeaderboardSubsystem`, `UT66SkillRatingSubsystem`, `UT66AchievementsSubsystem`, `UT66BackendSubsystem`, and `UT66RunIntegritySubsystem` support leaderboards, rating, achievements, backend APIs, and run validation.
- `UT66AudioSubsystem` and `UT66MusicSubsystem` handle audio events and music.
- `UT66CharacterVisualSubsystem`, `UT66SkinSubsystem`, and `UT66CompanionUnlockSubsystem` support character visuals, skins, and companion unlock state.
- `UT66WeaponManagerSubsystem`, `UT66IdolManagerSubsystem`, `UT66BuffSubsystem`, `UT66DifficultyTuningSubsystem`, and `UT66PlayerExperienceSubSystem` support progression, buffs, authored difficulty tuning, and XP/reward tuning.
- `UT66TrapSubsystem`, `UT66InteractionPromptSubsystem`, `UT66FloatingCombatTextSubsystem`, `UT66FloatingCombatTextPoolSubsystem`, `UT66PixelVFXSubsystem`, `UT66RetroFXSubsystem`, and `UT66PixelationSubsystem` support traps, prompts, combat text, and visual effects.
- `UT66EnemyPoolSubsystem`, `UT66HeroSpeedSubsystem`, `UT66RngSubsystem`, `UT66RuntimePlatformSubsystem`, `UT66ReleaseVariantSubsystem`, `UT66LocalizationSubsystem`, `UT66MediaViewerSubsystem`, `UT66UITexturePoolSubsystem`, `UT66LagTrackerSubsystem`, and `UT66DamageLogSubsystem` are additional core support subsystems.
- `PerformanceSystem/UT66PerformanceSubsystem` exists as a separate performance-system component.

### Data Layer

- Main authored data lives under `Content/Data` as CSV and JSON sources, paired with imported `DT_*.uasset` assets where applicable.
- Character/content tables include `Heroes.csv` (12), `Companions.csv` (16), `CharacterVisuals.csv` (133), `Weapons.csv` (192), `Idols.csv` (12), `Items.csv` (30), and `StatusEffects.csv` (12 rows including `None`).
- Combat roster tables include `Enemies.csv` (60), `Bosses.csv` (23), `BossEncounters.csv` (20), `BossEncounterMembers.csv`, `UniqueEnemies.csv` (1), `NPCs.csv` (3), and `LoanShark.csv` (1).
- Stage/progression tables include `Stages.csv` (20), `DifficultyTuning.json`, and `PlayerExperience.json`.
- Leaderboard data includes `Leaderboard_ScoreTargets.csv` (20 rows) and `Leaderboard_SpeedrunTargets.csv` (20 rows).
- Interaction/world data includes `ArcadeInteractables.json` (14 entries), `VehicleInteractables.json`, and `WorldVisualProps.json`.
- Audio and VFX data include `AudioEvents.json` (96 entries) and `CombatVFXBindings.csv`.
- Imported DataTable setup and reload scripts live under `Scripts`, including `SetupCombatRosterDataTables.py`, `SetupWeaponsDataTable.py`, `SetupItemsDataTable.py`, `SetupIdolPixelVFX.py`, `SetupAudioEventsDataTable.py`, `SetupArcadeInteractablesDataTable.py`, `SetupVehicleInteractablesDataTable.py`, `SetupWorldVisualPropsDataTable.py`, `SetupPlayerExperienceDataTable.py`, and related import/reload scripts.

### Maps And World Structure

- Core map files are `Content/Maps/FrontendLevel.umap` and `Content/Maps/GameplayLevel.umap`.
- Effects/demo/benchmark map files exist under `Content/Stylized_VFX_StPack/MAP` and `Content/UE5RFX/Levels`.
- Stage layout is data-driven by `Content/Data/Stages.csv`, with 20 stages, 5 themes, 5 difficulty IDs, per-stage enemy slot references, boss references, and boss-only finale markers.
- Boss flow is data-driven by `Content/Data/BossEncounters.csv`, `Content/Data/BossEncounterMembers.csv`, and `Content/Data/Bosses.csv`, and implemented through the `T66GameMode_BossFlow` game-mode split.

### Characters, Enemies, And Combat

- Heroes are represented by `AT66HeroBase`, `BP_HeroBase`, hero data rows, character visual data, portrait fields, and hero-specific stats in `Content/Data/Heroes.csv`.
- Companion unlock/content state is represented by `Content/Data/Companions.csv`, `UT66CompanionUnlockSubsystem`, and companion save data.
- Standard enemies use `Content/Data/Enemies.csv`, role IDs, theme IDs, and enemy gameplay classes under `Source/T66/Gameplay/Enemies`.
- Bosses use `Content/Data/Bosses.csv`, boss encounter tables, and boss-flow code under `Source/T66/Gameplay/GameMode`.
- Unique enemies and NPCs are sourced from `Content/Data/UniqueEnemies.csv`, `Content/Data/NPCs.csv`, and `Content/Data/LoanShark.csv`.
- The detailed enemy roster reference is `Reports/RosterReview/enemy_roster_review.md`.
- Combat code includes `UT66CombatComponent`, `AT66HeroProjectile`, `AT66EnemyProjectileBase`, `AT66SpitProjectile`, `UT66ProjectileManagerSubsystem`, `UT66CombatHitZoneComponent`, `AT66HeroOneAttackVFX`, and the floating combat text subsystem/pool/actor.

### Economy, Progression, And Interactables

- The economy and run inventory are managed primarily through `UT66RunStateSubsystem` and its economy/inventory source files.
- Shop/vendor content uses `Content/Data/Items.csv` and interactable classes such as `AT66VendorInteractable`.
- Weapons use `Content/Data/Weapons.csv`, `UT66WeaponManagerSubsystem`, and `AT66WeaponAltar`.
- Idols use `Content/Data/Idols.csv`, `UT66IdolManagerSubsystem`, and `AT66IdolAltar`.
- Player XP, reward weights, loot-bag/chest tuning, and difficulty-linked progression data live in `Content/Data/PlayerExperience.json`.
- Main interactables include `AT66StartGate`, `AT66StageGate`, `AT66BossGate`, `AT66CowardiceGate`, `AT66TutorialGate`, `AT66TowerDescentHole`, `AT66ChestInteractable`, `AT66CrateInteractable`, `AT66LootWheelInteractable`, `AT66BoostInteractable`, `AT66FountainInteractable`, `AT66VehicleInteractable`, `AT66ArcadeInteractableBase`, `AT66ArcadeMachineInteractable`, `AT66BackroomsDoorInteractable`, and `AT66CasinoInteractable`.
- World hazards and pressure systems include `AT66MiasmaManager` and `UT66TrapSubsystem` with trap classes under `Source/T66/Gameplay/Traps`.

### Casino, Arcade, And Backrooms

- Casino UI is built around `UT66CasinoGamblerTabWidget` plus game widgets for coin flip, blackjack, rock-paper-scissors, and find-the-ball style play.
- The casino NPC and casino interactable classes support the player-facing casino experience.
- Arcade machines are driven by `Content/Data/ArcadeInteractables.json`, with the random arcade machine routing to 13 possible game types.
- Arcade/mini-game popup routing is handled from gameplay/controller overlay code and widget-game registry code.
- Backrooms gameplay is represented in `T66GameMode_Backrooms`, the Backrooms door interactable, the `UniqueEnemies.csv` Backrooms enemy row, and the Backrooms quick-revive item in `Items.csv`.

### UI Architecture

- UI routing is centralized in `Source/T66/UI/T66UIManager`.
- Screen IDs live in `Source/T66/UI/T66UITypes.h` as `ET66ScreenType`.
- The UI enum exposes 37 visible screen/modal entries, including main-game, modal, minigame, side-game, and game-over entries.
- Main screen and modal classes live under `Source/T66/UI/Screens`.
- Front-end screen resolution and registration is handled in `Source/T66/Gameplay/T66PlayerController_Frontend.cpp`.
- In-run presentation and overlays are handled by player-controller UI/overlay code, HUD widgets, gameplay HUD presentation code, floating combat text, pickup/reward presentation, and interaction prompts.

### Backend, Online, Saves, And Telemetry

- Backend code lives under `Source/T66/Core/Backend`.
- Backend API files include `T66BackendAccountApi`, `T66BackendLeaderboardApi`, `T66BackendPartyApi`, `T66BackendRunApi`, `T66BackendDailyClimbJson`, `T66BackendRunSerializer`, and `T66BackendRunSummaryParser`.
- `UT66BackendSubsystem` is the main backend subsystem.
- Steam online support is enabled through `OnlineSubsystemSteam` and `SocketSubsystemSteamIP`.
- Demo/full app IDs are configured in `Config/DefaultDemoMode.ini`.
- Saves are represented by `UT66SaveSubsystem` and save-game classes for run, profile, settings, local leaderboard, companion unlocks, buffs, community content, and minigame saves.
- Run validation and anti-cheat-style support live in `UT66RunIntegritySubsystem`.
- Telemetry-related components include `UT66DamageLogSubsystem`, `UT66LagTrackerSubsystem`, and run-state score/telemetry code.

### Audio And Music

- Audio assets live under `Content/Audio`.
- Audio source/content folders include `Arcade`, `HeltonPixelCombat`, `OSTS`, and `SFX`.
- The project currently has 149 Unreal audio asset files under `Content/Audio`, plus 5 loose OGG source audio files.
- The OST folder contains `MainTheme`, `Survival`, `Theme`, and a hero-specific `OST` source/asset pair.
- `Content/Data/AudioEvents.json` defines 96 audio event rows.
- Runtime audio is handled by `UT66AudioSubsystem` and `UT66MusicSubsystem`.

### Side-Game Modules

- `T66Mini` is the Mini Chadpocalypse module. It has about 40 C++ source/header files and 12 CSV data files under `Content/Mini/Data`. Its systems include runtime, run-state, frontend-state, data, circus, visual, leaderboard, save-game, and screen classes.
- `T66TD` is the Tower Defense module. It has about 19 C++ source/header files and 8 CSV data files plus `T66TD_Layouts.json` under `Content/TD/Data`. Its systems include frontend-state, data, visual, save-game, difficulty, map/layout, stage, and battle screens.
- `T66Idle` is the Idle module. It has about 11 C++ source/header files and 8 CSV data files under `Content/Idle/Data`. Its systems include frontend-state, data, profile save, offline progress, and an idle main menu screen.
- `T66Deck` is the Deck Builder module. It has about 11 C++ source/header files and 10 CSV data files under `Content/Deck/Data`. Its systems include frontend-state, data, run save, card/relic/encounter/stage data, and a deck main menu screen.
- Shared minigame and side-game design/process docs live under `Gameplay/Minigames`.

### Content Pipeline

- Source content and process folders include `SourceAssets`, `Model Generation`, `Tools/ArtPipeline`, `Video Generation`, and `Scripts`.
- Asset import scripts under `Scripts` cover hero portraits, item sprites, idol sprites, weapon sprites, weapon projectile meshes, static meshes, Backrooms assets, gameplay HUD art, world NPC interactables, and QuadRetro hero/enemy/boss visuals.
- Data import/setup scripts under `Scripts` create or reload the runtime DataTables from CSV/JSON sources.
- Model and visual pipelines include Trellis/RunPod, Pixal3D, QuadRetro, Blender canonicalization, validation scripts, and Unreal import commandlets.
- Runtime loose-asset contracts are guarded by `Scripts/GuardT66RuntimeAssetContract.ps1`.
- Video/frontend media workflow exists under `Video Generation`, with Unreal media support enabled by project plugins.
