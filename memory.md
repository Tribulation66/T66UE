# memory.md — T66 Agent Progress Ledger

This file is the persistent memory for any AI agent working on T66.
It must be kept up-to-date so a new agent can resume work safely without guessing.

**Rule:** This is not a brainstorm file. It is an engineering log + state tracker.

---

## 0) Current state (update whenever it changes)

- **Project:** T66 (Tribulation 66)
- **Repo path:** C:\UE\T66 (Windows) / c:\UE\T66
- **Engine version:** Unreal Engine 5.7
- **Active branch:** main
- **Last known-good commit:** 5758aa0 (Gameplay progression, Coliseum fallback, and map layout)
- **Current milestone:** Phase 3+ — Stage progression + bosses + miasma + NPCs + debt + Coliseum + map layout
- **Build status:** ✅ C++ compiles successfully
- **ValidateFast command:** `cmd /c "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66Editor Win64 Development "C:\UE\T66\T66.uproject" -waitmutex`
- **Full project setup (from project root):**
  - **Batch:** `Scripts\RunFullSetup.bat`
  - **Bash:** `./Scripts/RunFullSetup.sh` (chmod +x if needed)
  - **Editor:** Py Execute Script → `Scripts/CreateAssets.py` then `Scripts/FullSetup.py` (or run `T66Setup` after DT_Items exists)
- **Items-only setup:** `Scripts\RunItemsSetup.bat` or Py → `Scripts/SetupItemsDataTable.py`

---

## 1) Guardrails (always true)

- Make changes in **atomic change-sets**.
- ValidateFast after every change-set.
- Keep commits small and descriptive.
- If a change is risky (mass asset changes), write a plan first and checkpoint the repo.
- UI must remain **event-driven** (no per-frame UI thinking).
- **ALL UI text MUST be localized** — Use `UT66LocalizationSubsystem::GetText_*()` for every user-visible string. Never hardcode display text in C++ or Blueprints.

---

## 2) Open questions / blockers

- **WBP_LanguageSelect / WBP_Achievements** — Optional. C++ screens work without them; create in editor if you want Blueprint overrides (parent: T66LanguageSelectScreen, T66AchievementsScreen). Re-run `T66Setup` after creating.
- **Leaderboard data** — Placeholder only; will connect when Steam integration begins. Expected.
- **Nav Mesh** — Enemies move toward player in **Tick** (no nav required). If you add nav later, place Nav Mesh Bounds Volume in GameplayLevel for pathfinding.

---

## 3) Known issues / tech debt

- **Enemy health bar** — WidgetComponent has no Widget Class set; health bar is placeholder. Optional: create WBP_EnemyHealthBar and assign to HealthBarWidget.
- **Save Slots** — C++ fallback exists; optional WBP_SaveSlots at `/Game/Blueprints/UI/WBP_SaveSlots` for visual customization.
- **Hearts/Inventory** — Use Slate `BorderImage(WhiteBrush)` for filled squares; dynamic delegates use `AddDynamic`/`RemoveDynamic` (not AddUObject).
- **Localization** — New HUD strings added recently (stage number, stage timer, boss HP bar) are currently hardcoded in C++ Slate; must be moved to `UT66LocalizationSubsystem` per guardrails/Bible.
- **Coliseum map asset** — `Content/Maps/ColiseumLevel.umap` may be missing in this repo state; code has a safe fallback that reuses `GameplayLevel` for Coliseum-only behavior when needed.

---

## 4) Change log (append-only)

### 2026-01-29 — Gameplay progression, Coliseum fallback, and map layout

- **Commit:** 5758aa0
- **Goal:** Stabilize stage flow + bosses + miasma + debt systems, add difficulty totems + gambling cheating boss, implement Start/Main/Boss area layout with gates, and make Coliseum robust even if the Coliseum map asset is missing.

**Map layout**
- Gameplay map is now treated as three zones:
  - **Start Area** centered at \(X=-10000\)
  - **Main Area** centered at \(X=0\)
  - **Boss Area** centered at \(X=+10000\)
  - Connector floors at \(X=-6000\) and \(X=+6000\)
- **Start Gate pillars** are placed at \(X=-6000\). Walking through starts the stage timer (timer stays frozen before this).
- **Boss Gate pillars** are placed at \(X=+6000\). Walking through awakens the stage boss (forces awaken).
- **Trickster + Cowardice Gate** spawn right before the Boss Gate pillars (main-side).
- **Spawning guarantees**
  - Gameplay hero spawn is forced into the Start Area even if a PlayerStart exists elsewhere.
  - Stage boss spawns are forced into the Boss Area (fixes Stage 1 spawning at old/center locations).

**Coliseum behavior**
- Coliseum countdown begins immediately on spawn and still has miasma.
- When `ColiseumLevel` map asset is missing, the game falls back to using `GameplayLevel` in a forced Coliseum mode:
  - **Only** owed bosses + player + timer/miasma (no houses, no waves/NPCs, no gates).
- A Stage Gate spawns after the final owed boss dies and returns to Gameplay checkpoint stage **without incrementing** stage.

**Enemy spawning rule**
- Regular enemy waves only spawn after the stage timer starts (i.e., after Start Gate). Implemented by gating `AT66EnemyDirector::SpawnWave()` behind `RunState->GetStageTimerActive()`.

**Difficulty**
- Difficulty tier is tracked in `UT66RunStateSubsystem` and shown on HUD as placeholder squares.
- Enemy HP/touch-damage scale from tier and updates on difficulty changes.

**Gambler cheating boss**
- Added anger meter + cheat prompt in gambler overlay.
- If anger reaches 100%, Gambler NPC is replaced by a Gambler Boss (1000 HP, shoots pellets).

### 2026-01-29 — Data-driven stages + boss encounter + boss-gated stage exit

- **Goal:** Make stages data-driven; add a stage boss that is dormant until proximity, then awakens (chase + ranged projectiles). Show boss HP bar on awaken. Spawn Stage Gate only after boss dies, at boss death location. Reduce regular enemy spawn rate and increase hero auto-attack speed.
- **Commit:** 7b612fc

**Data (DT-backed)**
- Added `FBossData` and `FStageData` to `Source/T66/Data/T66DataTypes.h`.
- `UT66GameInstance` now supports `BossesDataTable` + `StagesDataTable` and helpers `GetBossData()` / `GetStageData()`.
- New CSV stubs:
  - `Content/Data/Bosses.csv`
  - `Content/Data/Stages.csv`
- Setup scripts updated to create/import/wire DTs:
  - `Scripts/CreateAssets.py` creates `/Game/Data/DT_Bosses` and `/Game/Data/DT_Stages`
  - `Scripts/ImportData.py` imports Bosses/Stages CSV into those DTs
  - `Scripts/FullSetup.py` wires `BossesDataTable` + `StagesDataTable` onto `BP_T66GameInstance`
  - `T66Editor/T66UISetupSubsystem.cpp` (`T66Setup`) also wires both DTs

**Boss gameplay**
- New boss actor: `AT66BossBase` (very large sphere) in `Source/T66/Gameplay/T66BossBase.*`
  - Dormant until within `AwakenDistance`
  - On awaken: sets boss UI state, starts firing ranged projectiles, chases player
  - On death: hides boss UI state, spawns Stage Gate at death location
- New projectile: `AT66BossProjectile` in `Source/T66/Gameplay/T66BossProjectile.*`
  - Overlap hero → `RunState->ApplyDamage(1)` then destroy

**HUD**
- `UT66RunStateSubsystem` now tracks boss UI state (active + HP) and broadcasts `BossChanged`.
- `UT66GameplayHUDWidget` now shows a top-center red boss HP bar (hidden until awaken), updates to `HP/MaxHP`.

**Stage flow**
- `AT66GameMode` no longer spawns Stage Gate at BeginPlay. It spawns a boss for the current stage instead.
- Stage Gate is spawned only after boss death via `SpawnStageGateAtLocation()`.
- On stage transition, timer is reset to full and boss UI state is reset/hidden.

**Combat + tuning**
- Hero projectiles deal **20 damage** per hit to an awakened boss (5 hits to kill at 100 HP).
- Hero auto-attack now prefers an awakened boss target.
- EnemyDirector spawn interval reduced (slower spawns): `10s → 20s` default.
- Hero auto-attack interval increased (faster shots): `10s → 1s` default.
- Boss chase fix: boss is AI-possessed so `AddMovementInput` applies (`AIControllerClass` + `AutoPossessAI`).

**Verification / proof**
- Built successfully with UE 5.7:
  - `Build.bat T66Editor Win64 Development -Project=c:\UE\T66\T66.uproject ...` ✅
  - `Build.bat T66 Win64 Development -Project=c:\UE\T66\T66.uproject ...` ✅

### 2026-01-30 — Start/Stage gates, 60s countdown timer, visible gates, UE 5.7 in guidelines

- **Goal:** Two distinct gates (Start Gate = walk-through two poles, starts timer; Stage Gate = big 3D rectangle, interact F to next stage). Timer frozen at 60s until start gate; countdown from 60; timer always visible on HUD. Stage transition keeps bounty, gold, progress; timer resets to 60 for new stage. Document UE 5.7 install path in Cursor guidelines.
- **Commit:** cb31d26

**Cursor guidelines**
- **T66_Cursor_Guidelines.md:** Section 7 — Added explicit note: Unreal Engine 5.7, install location `C:\Program Files\Epic Games\UE_5.7\` (UE_ROOT env var already pointed there).

**Run state (timer)**
- **UT66RunStateSubsystem:** `StageTimerDurationSeconds = 60.f`, `StageTimerSecondsRemaining` (frozen at 60 until start gate). `GetStageTimerSecondsRemaining()`, `TickStageTimer(DeltaTime)` (called from GameMode Tick; counts down when active; broadcasts `StageTimerChanged` at most once per second). `ResetStageTimerToFull()` — sets timer to 60 and freezes (used when entering next stage). `ResetForNewRun()` and `ResetStageTimerToFull()` both set `StageTimerSecondsRemaining = 60`, `bStageTimerActive = false`.

**GameMode**
- **AT66GameMode:** `PrimaryActorTick.bCanEverTick = true`. `Tick(DeltaTime)` calls `RunState->TickStageTimer(DeltaTime)`. On level load: if `bIsStageTransition` → clear flag and `RunState->ResetStageTimerToFull()` (keep progress, reset timer for new stage); else `RunState->ResetForNewRun()`. `SpawnStartGateForPlayer()` after RestartPlayer (near hero). `SpawnStageGate()` in BeginPlay at `StageGateSpawnOffset` (default 2500, 0, 200).

**Gates**
- **AT66StartGate:** Two pole meshes (Engine cylinder, left/right, ~100 uu apart), thin trigger between them (extent 60×80×180). Walk through (overlap) → `SetStageTimerActive(true)`, timer unfreezes and counts down from 60. No F interaction. Spawned by GameMode in RestartPlayer at hero + (150, 0, 0).
- **AT66StageGate:** Big 3D rectangle (cube mesh scaled 2×4×3 = 200×400×300). Overlap + F → `AdvanceToNextStage()`: increment stage, `bIsStageTransition = true`, reload current level. Progress (gold, bounty, items, hearts) kept; timer reset to 60 for new stage. Spawned in GameMode BeginPlay at `StageGateSpawnOffset`.

**HUD**
- **UT66GameplayHUDWidget:** Timer text always visible (e.g. 1:00), green, below stage number. Subscribes to `StageTimerChanged`; `RefreshHUD()` formats `GetStageTimerSecondsRemaining()` as M:SS. Frozen at 60 until start gate crossed; then countdown.

**Verification**
- C++ compiles; no new linter errors. Gates and timer driven by existing spawn/event flow.

---

### 2026-01-29 — Gameplay loop: HUD, combat, enemies, items, run summary, setup scripts

- **Goal:** In-run HUD, hero auto-attack projectile, enemy spawn/chase/touch damage, items (red ball), vendor, run summary on death, save/load, pause menu, full setup automation.
- **Commit:** 7a2d3de

**Core**
- **Run state:** `UT66RunStateSubsystem` — CurrentHearts (5), MaxHearts, CurrentGold, Inventory, EventLog, bHUDPanelsVisible; ApplyDamage (i-frames 0.75s), AddItem, SellFirstItem, ResetForNewRun, ToggleHUDPanels; delegates HeartsChanged, GoldChanged, InventoryChanged, LogAdded, PanelVisibilityChanged, OnPlayerDied.
- **Save system:** `UT66RunSaveGame` (run data), `UT66SaveIndex` (slot metadata), `UT66SaveSubsystem` (save/load). GameInstance: CurrentSaveSlotIndex, PendingLoadedTransform, bApplyLoadedTransform, bIsNewGameFlow.
- **Items:** `FItemData` (ItemID, PlaceholderColor, SellValueGold). GameInstance: ItemsDataTable, GetItemsDataTable(), GetItemData().
- **Data:** DT_Items, Content/Data/Items.csv. FullSetup.py and T66UISetupSubsystem set ItemsDataTable on BP_T66GameInstance.

**Gameplay**
- **Hero:** `UT66CombatComponent` on AT66HeroBase — AttackRange 4000, FireIntervalSeconds 10, DamagePerShot 999 (insta kill). Spawns `AT66HeroProjectile` toward closest enemy; projectile overlaps enemy capsule, TakeDamageFromHero(Damage), Destroy.
- **Projectile:** `AT66HeroProjectile` — SphereComponent (overlap), ProjectileMovementComponent (1200 u/s), red mesh. SetTargetLocation() sets velocity; OnSphereOverlap → enemy TakeDamageFromHero, ignore owner (hero).
- **Enemies:** `AT66EnemyBase` — Character with VisualMesh (red cylinder), HealthBarWidget. Tick: AddMovementInput toward player (no nav mesh). Capsule: ECR_Overlap for Pawn (touch hero) and WorldDynamic (projectile hit). OnCapsuleBeginOverlap(AT66HeroBase) → RunState->ApplyDamage(1) with cooldown. OnDeath → NotifyEnemyDied, spawn AT66ItemPickup (random ItemID), Destroy.
- **Enemy Director:** `AT66EnemyDirector` — SpawnIntervalSeconds 10, EnemiesPerWave 3, MaxAliveEnemies 12; SpawnMinDistance 400, SpawnMaxDistance 1000. First wave delay 2s. SpawnWave() traces ground, spawns AT66EnemyBase, sets OwningDirector.
- **Vendor:** `AT66VendorNPC` — Big blue cylinder (BasicShapes/Cylinder, scale 2,2,3). SphereComponent 150 radius. TrySellFirstItem() → RunState->SellFirstItem(). **Spawned by GameMode** in RestartPlayer (SpawnVendorForPlayer) near hero at hero+300,0,0.
- **Item pickup:** `AT66ItemPickup` — Red sphere mesh, SetItemID sets red material. F interact: PlayerController finds closest vendor or pickup, vendor→SellFirstItem, pickup→RunState->AddItem + Destroy pickup.
- **GameMode:** BeginPlay: RunState->ResetForNewRun(), Spawn AT66EnemyDirector. RestartPlayer: SpawnCompanionForPlayer, SpawnVendorForPlayer. SpawnDefaultPawnFor: apply PendingLoadedTransform if bApplyLoadedTransform.

**UI**
- **Gameplay HUD:** `UT66GameplayHUDWidget` — Hearts (5 filled red squares, grey when damaged; WhiteBrush), gold text, inventory slots (red when item, grey empty), minimap panel, inventory panel. T key toggles panel visibility via RunState->ToggleHUDPanels(). Delegates: AddDynamic(RefreshHUD) on Hearts/Gold/Inventory/PanelVisibility; RemoveDynamic in NativeDestruct.
- **Run Summary:** `UT66RunSummaryScreen` — Modal on death. Shows event log, RESTART (ResetForNewRun, unpause, open GameplayLevel), MAIN MENU (reset, open FrontendLevel). PlayerController subscribes to RunState->OnPlayerDied → pause, ShowModal(RunSummary), UI input mode.
- **Pause menu:** `UT66PauseMenuScreen` — Resume, Save and Quit, Restart, Settings, Report Bug. Esc closes sub-modals then pause. T66ReportBugScreen for bug text.
- **Save slots:** `UT66SaveSlotsScreen` — 3x3 grid, load from UT66SaveSubsystem; New Game sets bIsNewGameFlow, Load Game → PartySizePicker routes to SaveSlots.
- **Slate fixes:** SOverlay include `Widgets/SOverlay.h`; SScrollBox `Widgets/Layout/SScrollBox.h`. SBorder: no WidthOverride (use SBox wrapper). Dynamic delegates: AddDynamic + UFUNCTION for bound function; RemoveDynamic for unbind.

**Input**
- T = Toggle HUD panels
- F = Interact (vendor sell or pickup)
- Escape = Pause / back from modals
- P = Pause

**Scripts**
- `Scripts/RunFullSetup.bat` — Runs CreateAssets.py then FullSetup.py via UnrealEditor-Cmd (set UE_ROOT if needed).
- `Scripts/RunFullSetup.sh` — Same for bash; defaults UE_ROOT by OS (macOS/Linux/Git Bash).
- `Scripts/SetupItemsDataTable.py` — Creates DT_Items if missing, fills from Items.csv, runs T66Setup. Run from editor Py or RunItemsSetup.bat.
- `Scripts/FullSetup.py` — Now includes ItemsDataTable in configure_game_instance(), Items CSV import, DT_Items in verify_all_assets().

**Build**
- T66.Build.cs: PublicDependencyModuleNames include Slate, SlateCore, UMG, AIModule, NavigationSystem.
- New: T66RunStateSubsystem, T66SaveSubsystem, T66RunSaveGame, T66SaveIndex, T66CombatComponent, T66EnemyBase, T66EnemyAIController, T66EnemyDirector, T66HeroProjectile, T66ItemPickup, T66VendorNPC, T66GameplayHUDWidget, T66PauseMenuScreen, T66ReportBugScreen, T66RunSummaryScreen, T66CompanionGridScreen, T66HeroGridScreen, etc.

---

### 2025-01-28 — Localization System + Bible-Compliant UI Restructure

*(Previous entry — see original content in history. Summary: LocalizationSubsystem, Main Menu two-panel + leaderboard, Language Select, Hero/Companion selection restructure, FLeaderboardEntry, FSkinData, ET66LeaderboardFilter.)*

---

### 2025-01-28 — Programmatic UI Construction + Editor Subsystem

*(Previous entry — T66Editor, T66UISetupSubsystem, RunFullSetup/Configure*, BuildUI() on all screens, T66Setup console command.)*

---

### 2025-01-28 — Initial UI Framework + Asset Creation

*(Previous entry — T66.Build.cs, T66DataTypes, T66GameInstance, T66UIManager, T66ScreenBase, all screens, T66HeroBase, T66GameMode, T66FrontendGameMode, T66PlayerController, CreateAssets.py, DT_Heroes/DT_Companions, Blueprints, levels.)*

---

## 5) Working queue (short, prioritized)

### Full project setup (recommended)

1. **Build:** Compile T66Editor in Visual Studio (or Build.bat).
2. **Data + config:** From project root run:
   - **Windows:** `Scripts\RunFullSetup.bat`
   - **Bash:** `./Scripts/RunFullSetup.sh`
   - This runs CreateAssets.py (DataTables, levels) then FullSetup.py (Game Instance, PlayerController, GameModes, CSV import, verify). Items (DT_Items, Items.csv) are included in FullSetup.
3. **Editor:** Open project, optionally add Nav Mesh Bounds Volume in GameplayLevel (enemies move without it; useful for future). Vendor **does not** need to be placed — it spawns when the hero spawns.

### Optional manual steps

- Create WBP_LanguageSelect / WBP_Achievements if you want Blueprint overrides; re-run T66Setup.
- Create WBP_EnemyHealthBar and assign to enemy HealthBarWidget for health bar visuals.
- Create WBP_SaveSlots at `/Game/Blueprints/UI/WBP_SaveSlots` for Save Slots screen customization.

### T66Setup (editor)

- In Output Log: `T66Setup` — Configures BP_T66GameInstance (DataTables including DT_Items), BP_T66PlayerController (ScreenClasses), GameModes, level overrides. Run after creating new assets that need wiring.

---

## 6) Architecture summary

### File structure (current)

```
Source/T66/
├── T66.h / T66.cpp
├── T66.Build.cs                    # Core, Slate, SlateCore, UMG, AIModule, NavigationSystem
├── Data/
│   └── T66DataTypes.h               # FHeroData, FCompanionData, FItemData, FLeaderboardEntry, FSkinData, enums
├── Core/
│   ├── T66GameInstance.h/.cpp       # DataTables (Heroes, Companions, Items), save slot/transform, bIsNewGameFlow
│   ├── T66LocalizationSubsystem.h/.cpp
│   ├── T66RunStateSubsystem.h/.cpp  # Hearts, gold, inventory, event log, HUD visibility, i-frames, OnPlayerDied
│   ├── T66SaveSubsystem.h/.cpp      # Save/load run data
│   ├── T66RunSaveGame.h             # Run snapshot (hero, companion, map, transform, etc.)
│   └── T66SaveIndex.h               # Slot metadata
├── UI/
│   ├── T66UITypes.h                 # ET66ScreenType (includes RunSummary, PauseMenu, ReportBug)
│   ├── T66UIManager.h/.cpp          # Screen stack, ShowModal, GetCurrentModalType
│   ├── T66ScreenBase.h/.cpp
│   ├── T66GameplayHUDWidget.h/.cpp  # In-run HUD: hearts, gold, inventory, minimap; T toggle
│   ├── Screens/
│   │   ├── T66MainMenuScreen, T66PartySizePickerScreen, T66HeroSelectionScreen, T66CompanionSelectionScreen
│   │   ├── T66HeroGridScreen, T66CompanionGridScreen, T66SaveSlotsScreen
│   │   ├── T66SettingsScreen, T66QuitConfirmationModal, T66LanguageSelectScreen, T66AchievementsScreen
│   │   ├── T66PauseMenuScreen, T66ReportBugScreen, T66RunSummaryScreen
│   └── Components/
│       ├── T66Button.h/.cpp
│       └── T66LeaderboardPanel.h/.cpp
└── Gameplay/
    ├── T66HeroBase.h/.cpp           # Character, UT66CombatComponent
    ├── T66CombatComponent.h/.cpp    # Timer: TryFire every 10s, spawn AT66HeroProjectile toward closest enemy
    ├── T66HeroProjectile.h/.cpp      # Overlap enemy → TakeDamageFromHero(999), Destroy
    ├── T66EnemyBase.h/.cpp          # Character, red cylinder, Tick move toward player, touch damage, OnDeath spawn pickup
    ├── T66EnemyAIController.h/.cpp  # Optional SimpleMoveToActor (nav); movement also in enemy Tick
    ├── T66EnemyDirector.h/.cpp      # Timer: SpawnWave 3 enemies every 10s (first wave 2s), 400–1000 uu from player
    ├── T66ItemPickup.h/.cpp         # Red sphere, SetItemID(red), F to collect
    ├── T66VendorNPC.h/.cpp          # Big blue cylinder, F to sell first item, spawned by GameMode
    ├── T66CompanionBase.h/.cpp      # Companion pawn, follow hero
    ├── T66CompanionPreviewStage.h/.cpp
    ├── T66HeroPreviewStage.h/.cpp
    ├── T66StartGate.h/.cpp          # Two poles + trigger; walk-through starts timer (SetStageTimerActive)
    ├── T66StageGate.h/.cpp          # Big 3D rectangle; F to AdvanceToNextStage (reload level, keep progress)
    ├── T66GameMode.h/.cpp           # Tick→TickStageTimer; RestartPlayer: SpawnCompanion, SpawnVendor, SpawnStartGate; BeginPlay: spawn StageGate, RunState reset or ResetStageTimerToFull
    ├── T66FrontendGameMode.h/.cpp
    └── T66PlayerController.h/.cpp   # Frontend vs Gameplay init, HUD, Interact (F), Pause (Esc), OnPlayerDied → RunSummary

Source/T66Editor/
├── T66Editor.Build.cs, T66Editor.h/.cpp
└── T66UISetupSubsystem.h/.cpp       # RunFullSetup, Configure* (GameInstance with DT_Items, PlayerController, GameModes, levels)

Scripts/
├── CreateAssets.py                  # DataTables (DT_Heroes, DT_Companions, DT_Items), levels
├── FullSetup.py                     # Button widget, Game Instance (Heroes, Companions, Items), PlayerController, GameModes, CSV import, verify
├── SetupItemsDataTable.py           # Create DT_Items, fill from Items.csv, run T66Setup
├── ImportData.py                    # Fill DataTables from CSV (Heroes, Companions, Items)
├── RunFullSetup.bat / RunFullSetup.sh
└── RunItemsSetup.bat

Content/
├── Blueprints/Core/                 # BP_T66GameInstance, BP_T66PlayerController, BP_HeroBase
├── Blueprints/GameModes/            # BP_FrontendGameMode, BP_GameplayGameMode
├── Blueprints/UI/                   # WBP_*, WBP_T66Button in Components
├── Data/                            # DT_Heroes, DT_Companions, DT_Items, *.csv
└── Maps/                            # FrontendLevel, GameplayLevel
```

### Data flow (gameplay)

1. **Frontend:** Main Menu → New Game / Load Game → PartySizePicker → HeroSelection → CompanionSelection → (Enter Tribulation or pick save slot) → GameplayLevel.
2. **Gameplay start:** GameMode BeginPlay → RunState->ResetForNewRun() or (if bIsStageTransition) ResetStageTimerToFull(); spawn EnemyDirector, SpawnStageGate(). RestartPlayer → spawn hero, SpawnCompanionForPlayer, SpawnVendorForPlayer, SpawnStartGateForPlayer().
3. **In run:** RunState holds hearts, gold, inventory, log, stage number, stage timer (60s frozen until start gate). Walk through Start Gate (two poles) → timer starts counting down. HUD shows "Stage number: x" and timer (1:00 → 0:59…). HUD subscribes to RunState delegates; T toggles panels. F at Stage Gate (big rectangle) → next stage (reload level, keep progress, timer reset to 60). Hero CombatComponent fires projectile every 10s at closest enemy (insta kill). Enemies spawn 3 every 10s, chase in Tick, touch = 1 heart damage (i-frames). Enemy death → red ball pickup; F to collect (inventory slot red) or F at vendor to sell.
4. **Death:** RunState->ApplyDamage leads to CurrentHearts 0 → OnPlayerDied → PlayerController shows RunSummary modal, pause. Restart or Main Menu resets RunState and loads level.

### Screen navigation (current)

```
MainMenu
  ├── New Game → PartySizePicker → HeroSelection → CompanionSelection → [Enter] → GameplayLevel
  ├── Load Game → PartySizePicker → SaveSlots → (load) → GameplayLevel
  ├── Settings (modal)
  ├── Achievements (modal)
  ├── Language → LanguageSelect (modal)
  └── Quit → QuitConfirmation (modal)

Gameplay: T = HUD toggle, F = Interact, Esc = Pause
  └── PauseMenu → Resume / Save and Quit / Restart / Settings / Report Bug
  └── On death → RunSummary (Restart / Main Menu)
```

### Localization

- **Languages:** English, Português (Brasil), 繁體中文
- **Subsystem:** `UT66LocalizationSubsystem`
- **Usage:** `GetText_*()` for all UI strings; screens subscribe to OnLanguageChanged where needed.
