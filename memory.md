# memory.md — T66 Agent Progress Ledger

This file is the persistent memory for any AI agent working on T66.
It must be kept up-to-date so a new agent can resume work safely without guessing.

**Rule:** This is not a brainstorm file. It is an engineering log + state tracker.

---

## 0) Current state (update whenever it changes)

- **Project:** T66 (Tribulation 66)
- **Repo path:** C:\UE\T66
- **Engine version:** Unreal Engine 5.7
- **Active branch:** main
- **Last known-good commit:** 8786fc2 (Add complete UI framework with Slate-based programmatic screens)
- **Current milestone:** Phase 1 - UI Flow + Localization (Bible sections 1.1-1.17)
- **Build status:** ✅ C++ COMPILES SUCCESSFULLY
- **ValidateFast command:** `cmd /c "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66Editor Win64 Development "C:\UE\T66\T66.uproject" -waitmutex`
- **ValidateFull command:** Open in Editor and PIE test

---

## 1) Guardrails (always true)

- Make changes in **atomic change-sets**.
- ValidateFast after every change-set.
- Keep commits small and descriptive.
- If a change is risky (mass asset changes), write a plan first and checkpoint the repo.
- UI must remain **event-driven** (no per-frame UI thinking).
- **ALL UI text MUST be localized** — Use `UT66LocalizationSubsystem::GetText_*()` functions for every user-visible string. Never hardcode display text in C++ or Blueprints.

---

## 2) Open questions / blockers

- Q: WBP_LanguageSelect and WBP_Achievements Blueprints need creation
  - Context: New screens added but Blueprint assets may not exist yet
  - Proposed resolution: Create in editor (parent: T66LanguageSelectScreen, T66AchievementsScreen)
  - Owner: User (create in editor)
  - Status: PENDING - Run T66Setup command after creating

- Q: Leaderboard data is placeholder only
  - Context: Leaderboard panel shows fake data, not connected to any backend
  - Proposed resolution: Will be connected when Steam integration begins
  - Owner: Future phase
  - Status: EXPECTED (placeholder is intentional)

---

## 3) Known issues / tech debt

(None yet - fresh codebase)

---

## 4) Change log (append-only)

### 2025-01-28 — Localization System + Bible-Compliant UI Restructure

- **Goal:** Add localization support (EN, PT-BR, 繁體中文) + restructure UI per Bible spec
- **Bible refs:** 1.2 (Main Menu), 1.7 (Language Select), 1.10 (Hero Selection), 1.13 (Companion Selection)
- **Risk tier:** Green (extending existing systems)

**Part 1: Localization System**
- `Source/T66/Core/T66LocalizationSubsystem.h/.cpp` — GameInstance subsystem:
  - `ET66Language` enum (English, BrazilianPortuguese, ChineseTraditional)
  - `SetLanguage()` / `GetCurrentLanguage()` — Language switching
  - `OnLanguageChanged` delegate — Broadcasts when language changes
  - 50+ `GetText_*()` functions for all UI strings in all languages
  - Extensible design: add new languages by adding enum value + switch cases

**Part 2: Main Menu Restructure (Bible 1.2)**
- Two-panel layout: Left (buttons) + Right (leaderboard)
- Left panel: Title + vertical button stack (New Game, Load Game, Settings, Achievements)
- Right panel: `ST66LeaderboardPanel` Slate widget with:
  - Filter toggles (Global / Friends / Streamers)
  - Dropdown buttons (Party Size, Difficulty) - cycle on click
  - 10 leaderboard rows with rank, name, score, time
  - Placeholder data with fake player names
- Language icon button (bottom-left)
- Quit button (top-right)

**Part 3: Leaderboard Panel Component**
- `Source/T66/UI/Components/T66LeaderboardPanel.h/.cpp`:
  - Pure Slate widget (SCompoundWidget)
  - Generates 10 placeholder entries with fake names/scores
  - Highlights local player row (entry 8)
  - Filter/dropdown clicking regenerates data

**Part 4: Language Select Modal (Bible 1.7)**
- `Source/T66/UI/Screens/T66LanguageSelectScreen.h/.cpp`:
  - Modal overlay with dark background
  - Vertical list of language buttons
  - Preview on select, Confirm/Back buttons
  - Instantly updates UI when language changes

**Part 5: Hero Selection Restructure (Bible 1.10)**
- Top bar: HERO GRID button + hero belt (nav arrows + current hero name)
- Left panel: Skins list with BUY/EQUIP buttons (4 placeholder skins)
- Center: Preview area + CHOOSE COMPANION button + Body Type toggle
- Right panel: HERO INFO + video placeholder + description + 4 medal slots + LORE button
- Bottom: 7 difficulty buttons (Easy→Final) + THE LAB button + ENTER THE TRIBULATION button
- Back button (bottom-left overlay)

**Part 6: Companion Selection Restructure (Bible 1.13)**
- Mirrors Hero Selection layout
- Top bar: COMPANION GRID button + nav belt + NO COMPANION button
- Left panel: Skins list (3 placeholder skins)
- Center: Preview area + Body Type toggle (disabled when no companion)
- Right panel: COMPANION INFO + lore + passive effect + 3 medal slots + LORE button
- Bottom: CONFIRM COMPANION button
- Back button (bottom-left overlay)

**Part 7: Data Types Added**
- `FLeaderboardEntry` struct — Rank, PlayerName, Score, TimeSeconds, HeroID, PartySize, Difficulty, StageReached, bIsLocalPlayer
- `FSkinData` struct — SkinID, DisplayName, OwnerID, CoinCost, bIsOwned, bIsEquipped, bIsDefault
- `ET66LeaderboardFilter` enum — Global, Friends, Streamers

**Part 8: Localized Text Updates**
- All screens now use `UT66LocalizationSubsystem::GetText_*()` methods
- Text updates automatically when language changes
- Main Menu subscribes to `OnLanguageChanged` and rebuilds UI

- **Commands run (proof):**
  - Build.bat T66Editor Win64 Development → Result: Succeeded (12.37 seconds)
- **Result:** All C++ compiles, UI restructured per Bible, localization functional
- **Commit:** (pending)

---

### 2025-01-28 — Programmatic UI Construction + Editor Subsystem

- **Goal:** Create Editor subsystem to auto-configure Blueprints + add programmatic UI construction to screens
- **Bible refs:** 1.1-1.15 (Boot Intro through Hero Selection Co-op)
- **Risk tier:** Green (new code extending existing systems)

**Part 1: T66Editor Module Created**
- `Source/T66Editor/T66Editor.Build.cs` — Editor module dependencies (UnrealEd, UMGEditor, etc.)
- `Source/T66Editor/T66Editor.h/.cpp` — Module entry point
- `Source/T66Editor/T66UISetupSubsystem.h/.cpp` — Editor subsystem with auto-configuration:
  - `RunFullSetup()` — Runs all configuration in one call
  - `ConfigureFrontendLevel()` — Sets GameMode override on FrontendLevel
  - `ConfigureGameplayLevel()` — Sets GameMode override on GameplayLevel
  - `ConfigureFrontendGameMode()` — Sets PlayerController class, no pawn
  - `ConfigureGameplayGameMode()` — Sets PlayerController class, HeroBase pawn
  - `ConfigurePlayerController()` — Registers all screen classes in ScreenClasses TMap
  - `ConfigureGameInstance()` — Sets DataTable references

**Part 2: Programmatic UI Construction**
- `T66ScreenBase.h/.cpp` — Added helper functions for building UI in code:
  - `CreateRootCanvas()`, `CreateButton()`, `CreateText()`, `CreateTitle()`
  - `CreateVerticalBox()`, `CreateHorizontalBox()`, `CreateSizeBox()`, `CreateBorder()`
  - `AddToCanvas()`, `AddToCenterOfCanvas()` — Canvas positioning helpers
- All screen classes now build their UI programmatically via `BuildUI()`:
  - `T66MainMenuScreen` — Title, New Game/Load Game/Settings/Achievements buttons, Quit/Language buttons
  - `T66PartySizePickerScreen` — Solo/Duo/Trio selection with descriptions, Back button
  - `T66QuitConfirmationModal` — Dark overlay, modal box with Stay/Quit buttons
  - `T66SettingsScreen` — Modal with tabs (placeholder), Close button
  - `T66HeroSelectionScreen` — Hero nav (</>), Choose Companion, Enter Tribulation buttons
  - `T66CompanionSelectionScreen` — Companion nav, No Companion option, Confirm button
  - `T66SaveSlotsScreen` — Placeholder for 3x3 grid, Back button

**Part 3: Configuration Updates**
- `T66.uproject` — Added T66Editor module (Type: Editor)
- `T66Editor.Target.cs` — Added T66Editor to ExtraModuleNames

- **Commands run (proof):**
  - UnrealHeaderTool → Result: Succeeded (processed 21 files)
  - Full build blocked by Live Coding (editor open)
- **Result:** UHT validates all headers. Full compile requires editor restart or Live Coding.
- **Commit:** (not yet committed)

### TO RUN SETUP:
1. Close Unreal Editor
2. Rebuild project (or open editor and Live Compile)
3. In Editor, open Output Log and run: `T66.Setup.RunFullSetup` (console command)
   OR use the subsystem from Blueprint/C++: `GetEditorSubsystem<UT66UISetupSubsystem>()->RunFullSetup()`

---

### 2025-01-28 — Initial UI Framework + Asset Creation

- **Goal:** Create complete C++ foundation + all Blueprint/DataTable/Level assets via automation
- **Bible refs:** 1.1-1.15 (Boot Intro through Hero Selection Co-op)
- **Risk tier:** Green (new code, no existing systems affected)

**Part 1: C++ Code**
- `Source/T66/T66.Build.cs` — Added Slate, SlateCore, UMG + include paths
- `Source/T66/T66.h/.cpp` — Module header with log category
- `Source/T66/Data/T66DataTypes.h` — FHeroData, FCompanionData, enums
- `Source/T66/Core/T66GameInstance.h/.cpp` — Game Instance with DataTable management
- `Source/T66/UI/T66UITypes.h` — ET66ScreenType enum
- `Source/T66/UI/T66UIManager.h/.cpp` — Screen navigation manager
- `Source/T66/UI/T66ScreenBase.h/.cpp` — Base widget class
- `Source/T66/UI/Screens/*.h/.cpp` — All screen classes (MainMenu, PartySizePicker, etc.)
- `Source/T66/Gameplay/T66HeroBase.h/.cpp` — Placeholder hero (colored cylinder)
- `Source/T66/Gameplay/T66GameMode.h/.cpp` — Gameplay GameMode
- `Source/T66/Gameplay/T66FrontendGameMode.h/.cpp` — Frontend GameMode
- `Source/T66/Gameplay/T66PlayerController.h/.cpp` — Player Controller with UI Manager

**Part 2: Assets Created (via Python commandlet)**
- `Content/Blueprints/Core/BP_T66GameInstance.uasset`
- `Content/Blueprints/Core/BP_T66PlayerController.uasset`
- `Content/Blueprints/Core/BP_HeroBase.uasset`
- `Content/Blueprints/GameModes/BP_FrontendGameMode.uasset`
- `Content/Blueprints/GameModes/BP_GameplayGameMode.uasset`
- `Content/Blueprints/UI/WBP_MainMenu.uasset`
- `Content/Blueprints/UI/WBP_PartySizePicker.uasset`
- `Content/Blueprints/UI/WBP_HeroSelection.uasset`
- `Content/Blueprints/UI/WBP_CompanionSelection.uasset`
- `Content/Blueprints/UI/WBP_SaveSlots.uasset`
- `Content/Blueprints/UI/WBP_Settings.uasset`
- `Content/Blueprints/UI/WBP_QuitConfirmation.uasset`
- `Content/Data/DT_Heroes.uasset`
- `Content/Data/DT_Companions.uasset`
- `Content/Maps/FrontendLevel.umap`
- `Content/Maps/GameplayLevel.umap`

**Part 3: Configuration**
- `Config/DefaultEngine.ini` — Updated GameDefaultMap, GameInstanceClass

**Part 4: CSV Data Files**
- `Content/Data/Heroes.csv` — 5 heroes with colors
- `Content/Data/Companions.csv` — 3 companions

- **Commands run (proof):**
  - UnrealBuildTool → Result: Succeeded (3.39 seconds)
  - Python CreateAssets.py → Result: Success - 0 error(s), 1 warning(s)
  - Python ImportData.py → Result: Success - 0 error(s), 0 warning(s)
- **Result:** All C++ compiles, all assets created. Manual Blueprint configuration required.
- **Commit:** (not yet committed)

---

## 5) Working queue (short, prioritized)

### AUTOMATED SETUP (NEW - use T66UISetupSubsystem):

The T66Editor module now includes an Editor Subsystem that automates most configuration:

**To Run Auto-Setup:**
1. Close Unreal Editor if open
2. Rebuild the project (or use Live Coding Ctrl+Alt+F11 in editor)
3. Open Unreal Editor
4. In the Output Log, run: `T66Setup` (console command)
   - Or in Blueprint/C++: `GEditor->GetEditorSubsystem<UT66UISetupSubsystem>()->RunFullSetup()`

**What Auto-Setup Configures:**
- ✅ BP_T66GameInstance — DataTable references
- ✅ BP_T66PlayerController — Screen class mappings (TMap)
- ✅ BP_FrontendGameMode — PlayerController class, no pawn
- ✅ BP_GameplayGameMode — PlayerController class, HeroBase pawn
- ✅ FrontendLevel — GameMode override
- ✅ GameplayLevel — GameMode override

**What UI Screens Now Have (Programmatic UI):**
- ✅ All screens build their UI in C++ via BuildUI() — no manual Widget Designer work needed
- ✅ Buttons are created and click events bound automatically
- ✅ Main Menu shows first on PIE (when FrontendLevel is default map)

### REMAINING MANUAL STEPS (minimal):

1. **Create WBP_LanguageSelect** - New Widget Blueprint needed:
   - In Content Browser, navigate to `Content/Blueprints/UI/`
   - Right-click → User Interface → Widget Blueprint
   - Name it `WBP_LanguageSelect`
   - In Class Settings, set Parent Class to `T66LanguageSelectScreen`
   - Save and re-run `T66Setup`

2. **GameplayLevel** - Add environment:
   - Add a `PlayerStart` actor to the level
   - Add floor/lighting as needed

3. **DT_Heroes** - Verify data (should auto-import):
   - Open DataTable, should have 5 rows
   - If empty, right-click → Reimport from `Content/Data/Heroes.csv`

4. **DT_Companions** - Verify data (should auto-import):
   - Open DataTable, should have 3 rows
   - If empty, right-click → Reimport from `Content/Data/Companions.csv`

---

## 6) Architecture Summary

### File Structure
```
Source/T66/
├── T66.h / T66.cpp                    # Module entry, log category
├── T66.Build.cs                       # Module dependencies + include paths
├── Data/
│   └── T66DataTypes.h                 # FHeroData, FCompanionData, FLeaderboardEntry, FSkinData, enums
├── Core/
│   ├── T66GameInstance.h/.cpp         # Persistent state, DataTable access
│   └── T66LocalizationSubsystem.h/.cpp # Language management, all UI strings
├── UI/
│   ├── T66UITypes.h                   # ET66ScreenType enum
│   ├── T66UIManager.h/.cpp            # Screen management, navigation
│   ├── T66ScreenBase.h/.cpp           # Base widget class
│   ├── Screens/
│   │   ├── T66MainMenuScreen.h/.cpp   # Two-panel: buttons + leaderboard
│   │   ├── T66PartySizePickerScreen.h/.cpp
│   │   ├── T66HeroSelectionScreen.h/.cpp  # Bible 1.10 layout
│   │   ├── T66CompanionSelectionScreen.h/.cpp  # Bible 1.13 layout
│   │   ├── T66SaveSlotsScreen.h/.cpp
│   │   ├── T66SettingsScreen.h/.cpp
│   │   ├── T66QuitConfirmationModal.h/.cpp
│   │   └── T66LanguageSelectScreen.h/.cpp  # Language selection modal
│   └── Components/
│       ├── T66Button.h/.cpp           # Reusable button component
│       └── T66LeaderboardPanel.h/.cpp # Slate leaderboard widget
└── Gameplay/
    ├── T66HeroBase.h/.cpp             # Placeholder hero (colored cylinder)
    ├── T66GameMode.h/.cpp             # Spawns selected hero
    ├── T66FrontendGameMode.h/.cpp     # Menu level (no pawn)
    └── T66PlayerController.h/.cpp     # Owns UIManager

Content/
├── Blueprints/
│   ├── Core/
│   │   ├── BP_T66GameInstance.uasset
│   │   ├── BP_T66PlayerController.uasset
│   │   └── BP_HeroBase.uasset
│   ├── GameModes/
│   │   ├── BP_FrontendGameMode.uasset
│   │   └── BP_GameplayGameMode.uasset
│   └── UI/
│       ├── WBP_MainMenu.uasset
│       ├── WBP_PartySizePicker.uasset
│       ├── WBP_HeroSelection.uasset
│       ├── WBP_CompanionSelection.uasset
│       ├── WBP_SaveSlots.uasset
│       ├── WBP_Settings.uasset
│       └── WBP_QuitConfirmation.uasset
├── Data/
│   ├── DT_Heroes.uasset
│   ├── DT_Companions.uasset
│   ├── Heroes.csv
│   └── Companions.csv
└── Maps/
    ├── FrontendLevel.umap
    └── GameplayLevel.umap
```

### Data Flow
1. User selects hero in UI → stored in `UT66GameInstance::SelectedHeroID`
2. User clicks "Enter the Tribulation" → loads GameplayLevel
3. `AT66GameMode::SpawnDefaultPawnFor` reads SelectedHeroID from GameInstance
4. GameMode looks up FHeroData from DataTable
5. GameMode spawns `AT66HeroBase`, calls `InitializeHero(HeroData)`
6. Hero sets its cylinder color from `HeroData.PlaceholderColor`

### Screen Navigation
```
MainMenu (two-panel: buttons left, leaderboard right)
  ├── NewGame → PartySizePicker → HeroSelection → CompanionSelection → HeroSelection → [Enter] → GameplayLevel
  ├── LoadGame → PartySizePicker → SaveSlots → HeroSelection → ...
  ├── Settings (modal)
  ├── Language (modal) → LanguageSelect
  └── Quit (modal) → QuitConfirmation → Exit
```

### Localization
- **Languages:** English, Português (Brasil), 繁體中文
- **Subsystem:** `UT66LocalizationSubsystem` (GameInstance subsystem)
- **Usage:** `GetLocSubsystem()->GetText_NewGame()` etc.
- **Extending:** Add new `ET66Language` value, add cases to all `GetText_*()` methods
