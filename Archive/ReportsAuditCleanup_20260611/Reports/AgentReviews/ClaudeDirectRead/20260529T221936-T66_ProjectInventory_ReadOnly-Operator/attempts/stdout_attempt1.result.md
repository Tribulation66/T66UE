I have a comprehensive, evidence-backed picture. Here is the complete Operator inventory packet for Codex to validate.

---

`Operator Packet: T66 Project Inventory`

# Operator Packet: T66 Project Inventory

## 1. Working Task And Validation Depth

- **Task contract:** READ-ONLY discovery inventory of the live T66 / Chadpocalypse project (main game + explicitly-requested minigames). Deliverable is one two-section Markdown document — (1) plain-language inventory, (2) technical inventory — describing what exists, not its quality.
- **Operator:** Claude (read-only profile). **Validator/Finisher:** Codex.
- **Validation depth:** Full (comprehensive inventory; broad coverage; a wrong roster would cause meaningful rework). Targeted anchor checks are practical because every count below cites a live source.
- **Tier:** Read-only discovery packet. No proof-bearing work; no build/cook/capture/editor automation performed or required.
- **No-assessment scope:** No quality, performance, soundness, TODO, or recommendation content included (see No Assessment Statement).

## 2. Roles And Tool Profile

- Operator model: Claude `claude-opus-4-8`, ReadOnly Operator profile (this session, direct read tools: Read, Glob, Grep, Bash used only for read/list/search — `ls`, `cat`, `cut`, `grep`, `wc`, `head`).
- Validator model: Codex.
- Helper command: invoked as direct-read operator; no FullOperator run, no Codex approval artifact required (read-only, non-mutating).
- Operator run directory / manifest: n/a (in-conversation read-only packet; no helper manifest produced).

## 3. User Constraints And Out Of Scope

- **Constraints:** Read-only; no code/data/config/content/save changes; no broad git status/diff over binary content; no editor/build/cook/capture; describe-only (no evaluation/critique/optimization/flagging/next-steps).
- **Mini/minigame status:** IN SCOPE — user explicitly named minigames. The four standalone minigame modules (Mini, TD, Idle, Deck) and the in-run arcade/casino games are inventoried.
- **Out of scope:** Quality/performance/soundness assessment; problem lists; recommendations; per-creature stat dumps (categories/counts only); the actual writing of the final document (Codex owns that after validation); any file edit.

## 4. Applicable Instructions Read

| Path | Why it applies | One-line takeaway |
|---|---|---|
| `AGENTS.md` | Root process router | Derive task contract; read folder routers; Mini in-scope only when explicitly named (it is here); cite live sources. |
| `OPERATOR_VALIDATOR_PROTOCOL.md` | Operator/Validator stack authority | Operator produces complete packet; Validator runs Packet Completeness Gate first; read-only packet can plan but not produce proof. |
| `Reports/AGENTS.md` | Report artifact routing | Durable packets live under `Reports/AgentReviews/<TaskSlug>/`; raw runs expire 15 days. |
| `Content/Data/pending_issues_Data.md` (present) | Folder pending-issues discipline | Read before working in `Content/Data`; not consumed for inventory content (out-of-scope problem notes). |

## 5. Section 1 Draft — Plain-Language Inventory

**Headline counts:** 12 playable heroes · 16 companions · ~60 standard enemies · 23 boss characters across 20 boss fights · 1 special "Backrooms" stalker · 3 world NPCs + a debt-collector · 20 main stages across 5 themed worlds · 12 element idols · 30 shop items · 192 hero weapons · 11 negative status effects · 13 in-run arcade games · 4 casino gambling games · 4 separate side-game modes · 2 core playable maps (+9 effects-demo/test maps) · ~150 sound assets with 5 music tracks. Current version label: alpha-0.8, shipping with a restricted Demo build alongside the full game.

**Maps / levels / environments.** Two core playable environments: a front-end menu world and a single gameplay world that is themed and re-dressed per stage. Gameplay is organized as a 20-stage descent across 5 worlds — Dungeon, Forest, Ocean, Martian, and Hell — four stages per world, each ending in a boss, with the world theme escalating difficulty (Easy → Medium → Hard → Very Hard → Impossible). Special spaces the player can enter include a Backrooms room, a casino, and shop/vendor and altar areas. Nine additional maps exist as effects/benchmark demo levels from art packs rather than playable content.

**Characters.** 12 playable heroes ("Chad" archetypes — e.g. Founding Chad, Boxer Chad, Robo Chad, Rabbit Chad), each built around one of four attack styles (Pierce, AOE, Bounce, Damage-over-Time), and each having alternate "Chad/Stacy" portrait skins. 16 unlockable female companions (4 available from the start). Enemies number about 60 standard creatures grouped into four behavior roles — melee, ranged, rushing, and flying — and reskinned per world theme. Bosses total 23 named characters spanning 20 encounters: one boss per stage, a demonic "Fallen Chad" capstone per world, the Four Horsemen as a simultaneous multi-boss fight, and a final Great Dragon. There is one unique scripted pursuer (the Backrooms Stalker) and three NPCs (a Casino host, a Saint, and Ouroboros), plus a Loan Shark debt-collector that hunts indebted players.

**Game modes / experiences.** The main experience is the themed tower-descent roguelike run. There is a gated Demo build (a limited set of heroes, one difficulty, a few companions, and a subset of arcade/casino games). Additional experiences include a practice/"Lab" mode, a Daily Climb mode, a Versus/Arcade mode, the Backrooms room, and the casino. Four fully separate side games ship as their own modules: an arcade-roguelike mini battler, a tower-defense game, an idle/clicker game, and a card-based deckbuilder.

**Economy / progression.** Currency is gold (with run materials and debt tracked). Loot uses a four-tier rarity ladder (Black, Red, Yellow, White). Players buy from 30 stat items (damage, attack-speed, attack-scale, accuracy/crit, evasion, armor, luck, plus special items like a Backrooms quick-revive and a vendor token) and collect 12 element idols (curse/lava/poison, electric/ice/shadow, earth/water/storm, light/steel/wood). Each hero has 16 unlockable weapons (192 total) tiered by the same four rarities and the four attack branches. Heroes level up in-run via XP. Companions unlock over time. Leaderboards track both score targets and speedrun times, broken out by difficulty, party size (Solo/Duo/Trio/Quad), and stage.

**Systems the player experiences.** Shop/vendor purchasing, idol and weapon altars, a casino with four gambling games (coin flip, rock-paper-scissors, blackjack, find-the-ball), 13 in-run arcade games (whack-a-mole, gold miner, crystal dash, bomb sorter, blade sweep, and more), stage/start/boss gates, a "Cowardice" gate, a tower-descent hole, chests/crates/loot-wheel/loot-bag pickups, fountains, boosts, rideable vehicles, a creeping miasma boundary, traps, and the debt/loan-shark mechanic.

**Backend / online.** Player-facing online features include online leaderboards and a daily climb, account status/login, party invites (multiplayer), run submission with anti-cheat/run-integrity validation, a skill-rating system, achievements, and local save slots. Steam is the online platform, with separate Steam app IDs for the demo and full game. Telemetry captures damage and performance/lag data.

**Audio / music / UI.** Roughly 150 sound assets and 5 music tracks (main theme, gameplay theme, survival theme, and hero-specific themes), driven by ~96 named audio events covering UI feedback, per-hero attacks, and arcade sounds. The UI comprises about 21 main-game screens — main menu, hero/companion selection and grids, pause, run summary, game over, power-ups, save slots, settings, language select, challenges, minigame hub, versus arcade, daily climb, achievements, account status, plus confirmation/invite/bug-report modals — and an in-run HUD with floating combat text. Each side-game has its own menu and battle screens.

**Other content.** World visual props (themed lamps, decor), vehicle and arcade interactable definitions, character-visual/skin variants (133 visual entries), and pixel/retro and stylized VFX.

## 6. Section 2 Draft — Technical Inventory

**Core architecture.** Unreal Engine 5.7, C++. The project (`T66.uproject`) declares six modules: `T66` (main runtime), `T66Mini`, `T66TD`, `T66Idle`, `T66Deck` (runtime minigames), and `T66Editor`. Enabled plugins include `OnlineSubsystemSteam` + `SocketSubsystemSteamIP`, `MovieRenderPipeline`, `AnimToTexture`, `ElectraPlayer`, `ProceduralMeshComponent`, and editor-only `PythonScriptPlugin`, `EditorScriptingUtilities`, `ModelingToolsEditorMode`.

Main module is subsystem-driven. Core persistence/state lives in `Source/T66/Core` (e.g. `UT66GameInstance`, `UT66RunStateSubsystem` — the runtime heart: hearts/gold/inventory/idols/weapons/score/timers, `UT66SessionSubsystem`, `UT66StageProgressionSubsystem`, `UT66ActorRegistrySubsystem`). Gameplay actors/components live in `Source/T66/Gameplay`. The frontend uses `AT66FrontendGameMode`; gameplay uses `AT66GameMode`, whose behavior is split across `Gameplay/GameMode/` partials: `_Tower`, `_Lab`, `_Backrooms`, `_BossFlow`, `_MainMap`, `_Spawning`, `_WorldInteractables`, `_TestRoom`, `_Bootstrap`.

**Data layer (`Content/Data`).** Source CSV/JSON paired with imported `DT_*.uasset` DataTables:
- `Heroes.csv` (12), `Companions.csv` (16), `Weapons.csv` (192), `Idols.csv` (12), `Items.csv` (30), `StatusEffects.csv` (11+None), `CharacterVisuals.csv` (133), `MobVertexAnimations.csv`.
- `Enemies.csv` (60; RoleID = Melee 20 / Ranged 16 / Rush 13 / Flying 11), `Bosses.csv` (23), `BossEncounters.csv` (20; EncounterType SingleBoss/FinaleChad/MultiBoss/ApocalypseFinale), `BossEncounterMembers.csv`, `UniqueEnemies.csv` (BackroomsChaser), `NPCs.csv` (3), `LoanShark.csv`.
- `Stages.csv` (20; ThemeID Dungeon/Forest/Ocean/Martian/Hell; DifficultyID Easy→Impossible; per-stage enemy slots A–L, boss, stage-effect type).
- `Leaderboard_ScoreTargets.csv` (by difficulty × party size) and `Leaderboard_SpeedrunTargets.csv` (by difficulty × stage).
- `CombatVFXBindings.csv`, `AudioEvents.json` (96 events).
- JSON-driven: `PlayerExperience.json` (per-difficulty XP thresholds, loot-bag/chest drop and rarity weights Black/Red/Yellow/White), `DifficultyTuning.json`, `ArcadeInteractables.json` (13 arcade game types + machine container), `VehicleInteractables.json`, `WorldVisualProps.json`.

**Per-domain components (main module).**
- *Characters:* `AT66HeroBase`, `BP_HeroBase`, `UT66CharacterVisualSubsystem`, `UT66SkinSubsystem`, `UT66CompanionUnlockSubsystem`; enemies via `AT66EnemyDirector`, `UT66MobManagerSubsystem`, `UT66EnemyPoolSubsystem`, enemy role classes under `Gameplay/Enemies`, `AT66ChestMimicEnemy`.
- *Economy/progression:* `UT66RunStateSubsystem`, `UT66WeaponManagerSubsystem` + `AT66WeaponAltar`, `UT66IdolManagerSubsystem` + `AT66IdolAltar`, `UT66BuffSubsystem`, `UT66DifficultyTuningSubsystem`.
- *Interactions:* `AT66VendorInteractable`, `AT66CasinoInteractable`/`AT66CasinoNPC`, `AT66StartGate`/`AT66StageGate`/`AT66BossGate`/`AT66CowardiceGate`/`AT66TutorialGate`, `AT66TowerDescentHole`, `AT66ChestInteractable`/`AT66CrateInteractable`/`AT66LootWheelInteractable`/`AT66BoostInteractable`/`AT66FountainInteractable`, `AT66VehicleInteractable`, `AT66ArcadeInteractableBase`/`AT66ArcadeMachineInteractable`, `AT66BackroomsDoorInteractable`, `AT66MiasmaManager`, `UT66TrapSubsystem` + `Gameplay/Traps`, `UT66InteractionPromptSubsystem`.
- *Projectiles/combat:* `UT66CombatComponent`, `AT66HeroProjectile`, `AT66EnemyProjectileBase`/`AT66SpitProjectile`, `UT66ProjectileManagerSubsystem`, `UT66CombatHitZoneComponent`, `AT66HeroOneAttackVFX`, floating combat text (`UT66FloatingCombatTextSubsystem` + pool + actor), `UT66PixelVFXSubsystem`, `UT66RetroFXSubsystem`, `UT66ToonOutlineViewSubsystem`.
- *Casino/arcade UI games:* `UT66CasinoGamblerTabWidget` with sub-views Cards/CoinFlip/RPS(RockPaperScissors)/FindBall, `UI/Gambler/T66CoinFlipGameWidget`, `T66BlackJackGameWidget`, and a `T66WidgetGames::Registry` descriptor system.
- *UI:* `Source/T66/UI/T66UIManager` (enum `ET66ScreenType` registry, ShowScreen/back-history/modals) routing ~21 screens in `UI/Screens` (MainMenu, HeroSelection, HeroGrid, CompanionSelection/Grid, PauseMenu, RunSummary, GameOver, PowerUp, SaveSlots/SavePreview, Settings, LanguageSelect, Challenges, Minigames, VersusArcade, DailyClimb, Achievements, AccountStatus, PlayerSummaryPicker; modals Quit/PartyInvite/ReportBug).
- *Audio:* `UT66AudioSubsystem` (data-driven event router) + `UT66MusicSubsystem`.
- *Backend/online:* `UT66BackendSubsystem` with `Core/Backend` APIs (`T66BackendAccountApi`, `T66BackendLeaderboardApi`, `T66BackendPartyApi`, `T66BackendRunApi`, `T66BackendDailyClimbJson`, `T66BackendRunSerializer`, `T66BackendRunSummaryParser`); `UT66LeaderboardSubsystem`, `UT66SaveSubsystem`, `UT66RunIntegritySubsystem` (anti-cheat), `UT66SkillRatingSubsystem`, `UT66AchievementsSubsystem`, `UT66SteamHelper`; telemetry via `UT66DamageLogSubsystem`, `UT66LagTrackerSubsystem`, and `T66RunStateSubsystem_ScoreTelemetry`. Demo gating via `T66DemoModeSettings` (`Config/DefaultDemoMode.ini`).
- *Performance:* `PerformanceSystem/UT66PerformanceSubsystem`.

**Minigame modules (all follow Frontend/Data/Save subsystem pattern).**
- `T66Mini` (~40 files): arcade-roguelike "circus" battler. `UT66MiniRuntimeSubsystem`, `UT66MiniRunStateSubsystem`, `UT66MiniFrontendStateSubsystem`, `UT66MiniDataSubsystem`, `UT66MiniCircusSubsystem` (market/debt/anger/transmute), `UT66MiniVisualSubsystem`, `UT66MiniLeaderboardSubsystem`; saves `UT66MiniRunSaveGame`/`UT66MiniProfileSaveGame`; 9 screens; 12 CSVs under `Content/Mini/Data`.
- `T66TD` (~19 files): lane tower-defense. `UT66TDFrontendStateSubsystem`, `UT66TDDataSubsystem`, `UT66TDVisualSubsystem`; saves Run/Profile; 3 screens; 9 data files (incl. `T66TD_Layouts` JSON) under `Content/TD/Data`.
- `T66Idle` (~11 files): idle/clicker. `UT66IdleFrontendStateSubsystem`, `UT66IdleDataSubsystem`; `UT66IdleProfileSaveGame` (offline progress); 1 screen; 8 CSVs under `Content/Idle/Data`.
- `T66Deck` (~11 files): roguelike deckbuilder. `UT66DeckFrontendStateSubsystem`, `UT66DeckDataSubsystem`; `UT66DeckRunSaveGame` (act/floor/node route, deck, relics); 1 screen; 10 CSVs under `Content/Deck/Data`. Design docs under `Gameplay/Minigames/{Mini,TD,Idle,Deck}` + `MINIGAMES_AGENTS.md`.

**Maps.** 11 `.umap`: `Content/Maps/FrontendLevel.umap`, `Content/Maps/GameplayLevel.umap`, plus 9 demo/benchmark maps under `Content/Stylized_VFX_StPack/MAP` and `Content/UE5RFX/Levels`.

**Content pipeline.** Source art → generation (Trellis2, QuadRetro, Pixal3D via RunPod/Blender MCP) → Blender canonicalization/QA → Python import commandlets in `Scripts/` (`ImportHeroPortraits`, `ImportStaticMeshes`, `RunImport*AndExit`, QuadRetro hero/boss/enemy visual imports, LOD generation) → `.uasset`. Data flow: source CSV/JSON → `Setup*DataTable`/`Import*DataTable` commandlets → `DT_*.uasset`. Theme kits processed via split→import→recolor→optimize→verify scripts. `Tools/ArtPipeline` handles minigame sprites/UI; `Video Generation` runs LTX-B frontend videos; `MovieRenderPipeline` plugin supports capture. Runtime asset contract guarded by `Scripts/GuardT66RuntimeAssetContract.ps1`.

## 7. Evidence Index (counts → source)

| Claim | Source (inspected read-only) |
|---|---|
| Engine 5.7; 6 modules; plugins | `T66.uproject:2,6-84` |
| 12 heroes; Pierce/AOE/Bounce/DOT; Chad/Stacy portraits | `Content/Data/Heroes.csv` (rows Hero_1–Hero_12) |
| 16 companions (4 default) | `Content/Data/Companions.csv` (`bUnlockedByDefault`) |
| 60 enemies; roles Melee 20/Ranged 16/Rush 13/Flying 11 | `Content/Data/Enemies.csv` col `RoleID` (uniq -c) |
| 23 bosses; Four Horsemen + Fallen Chads | `Content/Data/Bosses.csv` |
| 20 boss encounters; encounter types | `Content/Data/BossEncounters.csv` |
| Backrooms Stalker unique enemy | `Content/Data/UniqueEnemies.csv` |
| 3 NPCs; Loan Shark | `Content/Data/NPCs.csv`, `LoanShark.csv` |
| 20 stages; 5 themes; difficulty ladder; stage effects | `Content/Data/Stages.csv` |
| 12 idols; 4 element categories; 4 color tiers | `Content/Data/Idols.csv` |
| 30 items; 8 stat groups; specials | `Content/Data/Items.csv` |
| 192 weapons = 12×16; rarities Black/Red/White/Yellow ×48; branches AOE/Bounce/DOT/Pierce ×48 | `Content/Data/Weapons.csv` (col HeroID/Rarity/Branch, uniq -c) |
| 11 status effects | `Content/Data/StatusEffects.csv` |
| 133 character-visual rows | `Content/Data/CharacterVisuals.csv` (wc) |
| Leaderboards: score (difficulty×party) + speedrun (difficulty×stage) | `Leaderboard_ScoreTargets.csv`, `Leaderboard_SpeedrunTargets.csv` |
| Rarity weights / XP thresholds / chests-per-stage | `Content/Data/PlayerExperience.json` |
| 13 arcade games | `Content/Data/ArcadeInteractables.json` (ArcadeID / RandomGameTypes) |
| 4 casino games (CoinFlip/RPS/BlackJack/FindBall) | `Source/T66/UI/T66CasinoGamblerTabWidget.cpp:344-365` |
| Demo gating; version alpha-0.8; Steam app IDs | `Config/DefaultDemoMode.ini`, `Config/DefaultGame.ini` |
| Main game modes (Tower/Lab/Backrooms/BossFlow/MainMap/TestRoom) | `Source/T66/Gameplay/GameMode/` file list |
| Backend APIs (Account/DailyClimb/Leaderboard/Party/Run) | `Source/T66/Core/Backend/` file list |
| Main subsystem/class roster | `Source/T66/Core` + `Source/T66/Gameplay` (Explore pass) |
| 4 minigame modules' subsystems/saves/screens/data | `Source/T66Mini|T66TD|T66Idle|T66Deck` + `Content/{Mini,TD,Idle,Deck}/Data` (Explore pass) |
| 11 .umap; 2 core + 9 demo | `Content/**/*.umap` (Explore glob) |
| ~150 sounds, 5 OST, 96 audio events | `Content/Audio` + `Content/Data/AudioEvents.json` (Explore pass) |
| ~21 UI screens + UIManager registry | `Source/T66/UI/T66UIManager` + `UI/Screens` (Explore pass) |
| Content pipeline scripts | `Scripts/`, `Model Generation/`, `Tools/ArtPipeline`, `Video Generation` (Explore pass) |

*Note for Validator:* counts tagged "(Explore pass)" were gathered by read-only Explore subagents rather than a single literal command; these are the recommended anchors for targeted re-verification (file-list/glob/grep). All CSV/JSON/config counts were taken from direct file reads in this session.

## 8. Verification Plan

- This is a read-only discovery packet; no build/run/capture is in scope or was performed.
- Recommended Validator anchor checks (targeted): re-run `uniq -c` on `Enemies.csv` RoleID and `Weapons.csv` HeroID/Rarity/Branch; `wc -l` on each CSV; confirm `T66.uproject` module list; confirm `Gameplay/GameMode/` and `Core/Backend/` file lists; glob `Content/**/*.umap`. Expected pass markers: counts in §7 match.
- Verification intentionally skipped: compile/cook/editor/runtime — not requested and not possible under the read-only profile. If a current-verification request arises later, that requires a FullOperator run; this profile cannot satisfy it.

## 9. Token Routing

```text
TOKEN ROUTING
OperatorModel: claude-opus-4-8
OperatorTokensSpent: Unavailable
OperatorRunDir: n/a (in-conversation read-only packet)
OperatorManifest: n/a
CodexApprovalPath: n/a (read-only, non-mutating; no approval gate required)
ExpectedValidatorDepth: targeted
ValidatorBudgetHint: Spot-check §7 anchors — CSV row counts, Weapons/Enemies uniq distributions, uproject module list, GameMode/Backend file lists, umap glob. Deepen only if a cited count contradicts the file.
```

## 10. Operator Position And Open Decisions

- **Operator position:** The inventory is complete and ready for Codex to validate and convert into the final two-section document. Section 1 carries no file paths/class names; Section 2 carries paths and named systems; Evidence Index links the two.
- **Caveats:** "(Explore pass)" counts (audio asset count ~150, screen count ~21, umap=11, minigame file counts) are approximate roster counts gathered via subagent search, not exhaustive literal enumerations — flagged for targeted re-count if Codex wants exactness. The 23-boss vs 20-encounter relationship is intentional (Four Horsemen = 4 boss rows under one MultiBoss encounter). "Collector" and "Lab" run modes are referenced in `DefaultDemoMode.ini` gating flags; Lab is confirmed by a GameMode partial, Collector appears as a gating flag only.
- **Decisions only the user can make:** None required for a describe-only inventory.
- **Decisions the Validator should challenge:** Whether any "(Explore pass)" approximate count must be converted to an exact literal count before publication.

## 11. No Assessment Statement

This packet contains only a descriptive inventory of what exists in the project. It includes no quality, performance, soundness, or balance evaluation; no critique; no optimization or readiness judgment; no TODO/problem list; and no recommendations or next-steps. Where current live data was impractical to enumerate exhaustively, roster counts were approximated and labeled, not assessed.

## Verification Statement (Claude)

No files were created, edited, deleted, moved, or otherwise mutated by Claude. No shell mutations, Unreal Python, editor automation, build/cook/capture, or production asset writes were performed. All inspection used read-only operations (Read, Glob, Grep, and read-only Bash: `ls`/`cat`/`cut`/`grep`/`wc`/`head`). No git status/diff over binary content was run. This is an Operator artifact for Codex validation, not a greenlight.

**Claude Tokens Spent:** Unavailable

---

Packet complete and ready for Codex's Packet Completeness Gate and validation. Two `.umap`s are the only playable maps; the headline rosters (12 heroes, 16 companions, ~60 enemies, 23 bosses, 20 stages, 192 weapons, 12 idols, 30 items, 4 standalone minigames, 13 arcade + 4 casino games) are all anchored to live sources in the Evidence Index.
