# memory.md — T66 Agent Progress Ledger

This file is the persistent memory for any AI agent working on T66.
It must be kept up-to-date so a new agent can resume work safely without guessing.

## HARD RULE (Non‑Negotiable) — Localize *every* new player-facing string at implementation time

**Any time you add or change player-facing text, you must complete the full localization pipeline as part of that same change-set (no “we’ll translate it later”).**

- **1) Author text correctly (no manual per-language logic)**
  - Use **`LOCTEXT` / `NSLOCTEXT`** and/or **String Tables** (`FText::FromStringTable`).
  - **Never** add `switch(CurrentLanguage)` translation tables or other per-language string logic.
  - **Never** ship new UI strings via `FText::FromString(TEXT("..."))` (except truly non-player-facing debug/dev strings).
- **2) Gather**
  - Run GatherText (source and/or assets as applicable) so the new keys land in `Content/Localization/T66/T66.manifest` and the `.archive` files.
- **3) Translate (all 22 supported cultures)**
  - Populate translations for **all** supported cultures (22 total; `en` + 21 others).
  - Approved approaches:
    - Update `.po` / `.archive` entries manually (human translation), OR
    - Run the offline batch translator (`Scripts/AutoTranslateLocalizationArchives.py`) to populate `.archive` translations as a baseline.
- **4) Compile**
  - Run compile (`Config/Localization/T66_Compile.ini`) to regenerate `Content/Localization/T66/<culture>/T66.locres`.
- **5) Verify**
  - Verify the new text visibly changes when switching culture (and that UI remains event-driven; no tick polling).

**Agent acknowledgement requirement:** After reading `memory.md` and `T66_Cursor_Guidelines.md`, the agent must explicitly acknowledge this rule in its next message before making changes (e.g., “Acknowledged: any new player-facing text will be localized end-to-end: author → gather → translate → compile `.locres` → verify.”).

**Rule:** This is not a brainstorm file. It is an engineering log + state tracker.

**Hard Rule (Localization / Culture-based):** Every **player-facing** string must be localized using Unreal’s **culture-based localization pipeline** (gather → translations → `.locres` at runtime). This includes Slate/UMG text, prompts, tooltips, NPC names, achievement text, and button labels.  
**Do not** add new per-language translation switch statements (eg `switch(CurrentLanguage) { ... }`) for shipping text.  
Use `LOCTEXT` / `NSLOCTEXT` and/or **String Tables** (`FText::FromStringTable`) so `FInternationalization::Get().SetCurrentCulture(...)` drives all translations.  
`UT66LocalizationSubsystem` may still exist as a convenience layer (eg formatting helpers, ID→key indirection), but **it must return culture-localized `FText`** (not manual per-language strings).  
**Never** ship new hardcoded UI strings like `FText::FromString(TEXT("..."))`.

---

## 0) Current state (update whenever it changes)

- **Project:** T66 (Tribulation 66)
- **Repo path:** C:\UE\T66 (Windows) / c:\UE\T66
- **Engine version:** Unreal Engine 5.7
- **Active branch:** main
- **Last known-good commit:** 7b2c180 (official .1 version — sprites/models system + UI hookups)
- **Current milestone:** Official .1 — replace placeholder sprites/models with data-driven assets + automation scripts
- **Build status:** ✅ C++ compiles successfully
- **Sprites pipeline status:** ✅ Imported + wired (data-driven)
  - Source PNGs live under `SourceAssets/Sprites/**`
  - Imported textures live under `/Game/UI/Sprites/**`
  - Import script: `Scripts/ImportSpriteTextures.py` (run in full editor; commandlet imports can crash due to Slate/AssetTools)
- **World models pipeline status:** ✅ Imported + wired (data-driven)
  - Source FBX packs (`.zip`) may live under either:
    - `SourceAssets/Models/**` (preferred), or
    - `SourceAssets/Extracted/Models/**` (current repo layout as of 2026-02-02)
  - Extracted to `<models-root>/Extracted/**` by `Scripts/ImportWorldModels.py`
  - Imported meshes live under `/Game/World/**` and `/Game/Characters/NPCs/**`
  - Import script: `Scripts/ImportWorldModels.py`
  - Important: loot bags import into per-color subfolders (`/Game/World/LootBags/Black|Red|Yellow|White`) to avoid material/texture name collisions (`Material_001`, `Image_0`, etc)
  - Important: Trees/Trucks/Wheels also import into per-color subfolders (`/Game/World/Interactables/Trees|Trucks|Wheels/Black|Red|Yellow|White`) to avoid the same collisions
- **ValidateFast command:** `cmd /c "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66Editor Win64 Development "C:\UE\T66\T66.uproject" -waitmutex`
- **Full project setup (from project root):**
  - **Batch:** `Scripts\RunFullSetup.bat`
  - **Bash:** `./Scripts/RunFullSetup.sh` (chmod +x if needed)
  - **Editor:** Py Execute Script → `Scripts/CreateAssets.py` then `Scripts/FullSetup.py` (or run `T66Setup` after DT_Items exists)
- **Items-only setup:** `Scripts\RunItemsSetup.bat` or Py → `Scripts/SetupItemsDataTable.py`
  - **Official .1 one-shot (full editor):** `Scripts/SetupAllAssetsAndDataTables.py`

---

## 0.1) Keystone audit: Bible reconciliation index

This section exists to prevent “spec drift” between `T66_Bible.md` and the repo. When the repo’s behavior diverges from the Bible, we reconcile the Bible to match the repo and list the updated sections here.

- **Keystone audit checklist**: `T66_Keystone_Audit_Report.md`
- **Bible sections reconciled (doc updates)**:
  - `T66_Bible.md` → **2.2 START AREA + GATE START** (Stage 1 first-run tutorial is a dedicated Tutorial Area + portal teleport into Start Area)
  - `T66_Bible.md` → **2.9 IN-RUN STAGE FLOW** (interval-based enemy spawns; mini-boss + Unique behavior updated)
  - `T66_Bible.md` → **2.11.1 Specials** (removed wave-clear gating language; aligned to interval-based spawns)
  - `T66_Bible.md` → **Loot Bag drops** section (aligned to “1 bag per enemy death when `bDropsLoot` is true” + mini-boss rarity bias)

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

- **Localization guardrail debt** — Resolved. Player-facing strings are culture-based via UE localization gather/translate/compile (`.locres`) using `NSLOCTEXT` (and optional future String Tables). No per-language `switch(CurrentLanguage)` translation tables remain in `Source/T66/`.
- **Optional next improvement**: Migrate ID-keyed content (eg achievements/hero/companion names) to **String Tables** for designer-editable translations.
- **Save Slots** — C++ fallback exists; optional WBP_SaveSlots at `/Game/Blueprints/UI/WBP_SaveSlots` for visual customization.
- **Hearts/Inventory** — Use Slate `BorderImage(WhiteBrush)` for filled squares; dynamic delegates use `AddDynamic`/`RemoveDynamic` (not AddUObject).
- **Coliseum map asset** — `Content/Maps/ColiseumLevel.umap` may be missing in this repo state; code has a safe fallback that reuses `GameplayLevel` for Coliseum-only behavior when needed.

---

## 4) Change log (append-only)

### 2026-02-02 — Skill Rating system (no-damage windows) + run summary wiring

**Goal**
- Implement the v0 **Skill Rating** system based on “time without taking damage” using 5-second windows, mirroring the existing Luck Rating pipeline (compute → persist → display).

**Design**
- Starts at **50** (clamped 0..100).
- Evaluates in non-overlapping **5s windows** while combat time is active (stage timer active).
- Per 5s window:
  - 0 hits: +1
  - 1 hit: -1
  - 2 hits: -5
  - 3 hits: -10
  - 4 hits: -20
  - 5+ hits: -50
- Damage is an **input event** (no polling for damage).

**What changed**
- Added new subsystem:
  - `Source/T66/Core/T66SkillRatingSubsystem.h`
  - `Source/T66/Core/T66SkillRatingSubsystem.cpp`
- Wired damage events:
  - `Source/T66/Core/T66RunStateSubsystem.cpp` calls `UT66SkillRatingSubsystem::NotifyDamageTaken()` when damage actually applies in `ApplyDamage` / `ApplyTrueDamage`.
  - `ResetForNewRun()` resets the Skill Rating subsystem.
- Time driver + tracking gate:
  - `Source/T66/Gameplay/T66GameMode.cpp` ticks the skill subsystem and activates tracking only when stage timer is active and not in last-stand.
- Persist + UI:
  - `Source/T66/Core/T66LeaderboardRunSummarySaveGame.h` schema bumped to 4 and adds `SkillRating0To100`.
  - `Source/T66/Core/T66LeaderboardSubsystem.cpp` snapshots `SkillRating0To100`.
  - `Source/T66/UI/Screens/T66RunSummaryScreen.cpp` displays Skill Rating (live or saved summary).

**Localization**
- No new player-facing runtime strings.

**Verification / proof**
- ValidateFast (UE 5.7) ✅:
  - `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66Editor Win64 Development "C:\UE\T66\T66.uproject" -waitmutex`

### 2026-02-01 — UI cleanup: remove Hero/CompanionGrid WBP loads + preview capture on-demand

**Goal**
- Remove noisy load warnings for `WBP_HeroGrid` / `WBP_CompanionGrid` (these are C++ screens).
- Remove SceneCapture “major inefficiency” warnings in hero/companion preview stages.

**What changed**
- `Source/T66/Gameplay/T66PlayerController.cpp`:
  - `AutoLoadScreenClasses()` no longer attempts to load `WBP_HeroGrid` / `WBP_CompanionGrid` by path.
  - Always registers `UT66HeroGridScreen` / `UT66CompanionGridScreen` as C++ screens if missing.
- `Source/T66Editor/T66UISetupSubsystem.cpp`:
  - Removed `HeroGrid` / `CompanionGrid` WBP mappings from the setup list.
- `Source/T66/Gameplay/T66HeroPreviewStage.cpp`, `Source/T66/Gameplay/T66CompanionPreviewStage.cpp`:
  - Disabled `bCaptureEveryFrame`/`bCaptureOnMovement` and switched to on-demand captures (capture after rotate/zoom).

**Localization**
- No new player-facing runtime strings.

**Verification / proof**
- ValidateFast (UE 5.7) ✅:
  - `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66Editor Win64 Development "C:\UE\T66\T66.uproject" -WaitMutex -FromMsBuild -architecture=x64`
  - `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66 Win64 Development "C:\UE\T66\T66.uproject" -WaitMutex -FromMsBuild -architecture=x64`

### 2026-02-01 — Asset audit + remove unused `WBP_T66Button`

**Goal**
- Run the UI `WBP_*` asset audit and remove proven-unused UI bloat in a tiny batch.

**What changed**
- `Scripts/AssetAudit.py`:
  - Fixed UE5.7 commandlet compatibility (AssetData API differences, dependency options API differences, referencer type handling).
  - Report now lists only `WidgetBlueprint` assets (avoids `WidgetBlueprintGeneratedClass` duplicates).
- Updated `T66_Asset_Audit_Report.md`:
  - `/Game/Blueprints/UI` → `WBP_*` found: **9**
  - Unreferenced (non-optional): **0**
- Removed unused asset:
  - Deleted `Content/Blueprints/UI/Components/WBP_T66Button.uasset` (`/Game/Blueprints/UI/Components/WBP_T66Button`)
- Prevent reintroduction via setup scripts:
  - `Scripts/FullSetup.py` no longer auto-creates `WBP_T66Button`.
  - `Scripts/VerifySetup.py` no longer expects `WBP_T66Button`.
- Build fix (ValidateFast blocker):
  - `Source/T66/Core/T66GameInstance.h` and `Source/T66/Gameplay/T66GameMode.h` forward declare `FStreamableHandle` as `struct` (fixes MSVC `C4099`).

**Localization**
- No new player-facing runtime strings.

**Verification / proof**
- Asset audit ✅:
  - `& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "C:\UE\T66\T66.uproject" -run=pythonscript -script="C:\UE\T66\Scripts\AssetAudit.py" -unattended -nop4 -nosplash -nullrhi`
- VerifySetup ✅:
  - `& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "C:\UE\T66\T66.uproject" -run=pythonscript -script="C:\UE\T66\Scripts\VerifySetup.py" -unattended -nop4 -nosplash -nullrhi`
- ValidateFast (UE 5.7) ✅:
  - `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66Editor Win64 Development "C:\UE\T66\T66.uproject" -WaitMutex -FromMsBuild -architecture=x64`
  - `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66 Win64 Development "C:\UE\T66\T66.uproject" -WaitMutex -FromMsBuild -architecture=x64`

### 2026-02-01 — Keystone docs: audit report + Bible reconciliation + guideline hardening

**Goal**
- Establish a keystone “single checklist” and reduce future spec drift.

**What changed**
- Added `T66_Keystone_Audit_Report.md` (session checklist: divergences + perf/leak risks + bloat strategy).
- Reconciled `T66_Bible.md` to match current repo behavior for:
  - Stage 1 tutorial vs Start Area
  - Interval-based spawns (vs point-budget / wave-clear pacing)
  - Mini-boss + Unique enemy behavior
  - Loot bag drop behavior (including mini-boss rarity bias)
- Hardened `T66_Cursor_Guidelines.md` with:
  - Low-end performance checklist
  - Lifetime safety checklist
  - Optional-by-design WBP override note

**Localization**
- Docs only; no new player-facing runtime strings.

### 2026-02-01 — Vendor/Gambler layouts: reserve left space + 2×2 grids + shorter buttons

**Goal**
- Reserve a large empty area on the **left** side of the Vendor shop and Gambler casino cards view (for a future TikTok panel).
- In Vendor:
  - Shorten overly-long **BUY/STEAL** buttons.
  - Change the shop layout to **2×2** and reorder so Item 1 is under Item 2, and a new Item 4 is under Item 3.
- In Gambler:
  - Reorder the casino “game cards” so game 1 is **below** games 2+3, and push the block right.

**What changed**
- **Vendor shop UI**
  - `Source/T66/UI/T66VendorOverlayWidget.cpp`
    - `ShopSlotCount` **3 → 4**
    - Shop grid changed from a single-row layout to a **2×2 grid** with remapped positions:
      - Slot 1 top-left, Slot 2 top-right, Slot 0 bottom-left, Slot 3 bottom-right
    - **BUY/STEAL button width** set to fixed `WidthOverride(260.f)` and centered (no longer expanding across the tile).
- **Vendor stock generation**
  - `Source/T66/Core/T66RunStateSubsystem.cpp`
    - Vendor stock now generates **4 items** (2 black, 1 red, 1 yellow).
    - Fallback placeholder stock updated to `{ Item_01, Item_02, Item_03, Item_04 }`.
- **Gambler casino cards view**
  - `Source/T66/UI/T66GamblerOverlayWidget.cpp`
    - Replaced the single-row (3-across) cards layout with a right-shifted `SUniformGridPanel`:
      - Top row: Rock Paper Scissors, Find The Ball
      - Bottom row: Coin Flip (left slot), right slot empty
    - Added a left `SSpacer` to push cards to the right (reserve left side).

**Localization**
- No new player-facing strings added.

**Verification / proof**
- Builds ✅ (UE 5.7):
  - `& 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat' T66Editor Win64 Development -Project='C:\UE\T66\T66.uproject' -WaitMutex -FromMsBuild`
  - `& 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat' T66 Win64 Development -Project='C:\UE\T66\T66.uproject' -WaitMutex -FromMsBuild`

### 2026-02-01 — Leaderboard: local best run summary snapshots + clickable “YOU” row + Run Summary stats panel

**Goal**
- Reset and re-earn a local best “Bounty” leaderboard score and have that **local entry be clickable**.
- Clicking the local leaderboard entry opens the **Run Summary** for that best run.
- If a new best score is set later, the saved run summary is **replaced** and clicking opens the **new** best run.
- Add **Level + Stats** to the Run Summary screen.

**What changed**
- **Saved run-summary snapshot (file-per-best-run)**
  - Added `UT66LeaderboardRunSummarySaveGame` (`Source/T66/Core/T66LeaderboardRunSummarySaveGame.h/.cpp`)
  - `UT66LeaderboardSubsystem::SubmitRunBounty()` now writes a snapshot **only when a new best is set** (Practice Mode still blocks submission).
  - Snapshot slot name is deterministic per difficulty/party:
    - `T66_LocalBestBountyRunSummary_<Difficulty>_<PartySize>`
- **Reset local best (for testing)**
  - Removed the in-UI reset button (user request). For local testing, delete `Saved/SaveGames/T66_LocalLeaderboard.sav` to clear the local entry.
- **Clickable local leaderboard row**
  - `ST66LeaderboardPanel` (`Source/T66/UI/Components/T66LeaderboardPanel.*`)
    - The local player row is rendered as a full-row button; it opens the snapshot when one exists.
    - Clicking requests opening the saved summary and opens the Run Summary modal.
  - Implemented a simple transient handshake:
    - `UT66LeaderboardSubsystem::RequestOpenLocalBestBountyRunSummary(...)`
    - `UT66LeaderboardSubsystem::ConsumePendingRunSummaryRequest(...)`
    - `UT66RunSummaryScreen` consumes this request on activation to load the snapshot.
- **Run Summary: Level + Stats + saved-view mode**
  - `UT66RunSummaryScreen` now:
    - Shows a “Base Stats” panel with **LEVEL** + all 7 stats (Damage, Attack Speed, Attack Size, Armor, Evasion, Luck, Speed).
    - Can display either the **current run** or a **saved leaderboard snapshot**.
    - When opened from leaderboard (saved-view), the left button becomes **BACK** (closes the modal).
- **Localization**
  - Added `UT66LocalizationSubsystem::GetText_Level()` (new key: `T66.Common.Level`).
  - Ran Gather → Translate → Compile and verified `.locres` for all 22 cultures.

**Verification / proof**
- ValidateFast (UE 5.7) ✅
  - `Build.bat T66Editor Win64 Development "C:\UE\T66\T66.uproject" -WaitMutex -FromMsBuild -architecture=x64`
  - `Build.bat T66 Win64 Development "C:\UE\T66\T66.uproject" -WaitMutex -FromMsBuild -architecture=x64`
- Localization ✅
  - Source gather:
    - `UnrealEditor-Cmd.exe "C:\UE\T66\T66.uproject" -run=GatherText -config="C:\UE\T66\Config\Localization\T66_Gather_Source.ini" ...`
  - Translate archives:
    - `python -u "C:\UE\T66\Scripts\AutoTranslateLocalizationArchives.py"`
  - Compile:
    - `UnrealEditor-Cmd.exe "C:\UE\T66\T66.uproject" -run=GatherText -config="C:\UE\T66\Config\Localization\T66_Compile.ini" ...`
  - `.locres` count: **22**

### 2026-02-01 — Leaderboard UX follow-up: remove reset UI, fix “YOU row” click, leaderboard-view summary = Back-only

**User request**
- No in-UI “clear/reset” option; instead reset should be done by clearing local data so **YOU** shows rank **11** with score **0** (and speedrun time **0**).
- Clicking the **YOU** row should open the saved leaderboard run summary; in that mode the only button should be **BACK** (no restart).

**What changed**
- **Removed reset UI + APIs**
  - Removed the leaderboard “CLEAR” UI and deleted the reset functions from `UT66LeaderboardSubsystem`.
  - Local leaderboard reset is now achieved by deleting:
    - `Saved/SaveGames/T66_LocalLeaderboard.sav`
- **Fix: clicking “YOU” row not opening modal**
  - Root cause: the Main Menu Slate UI can be constructed before `UT66UIManager` assigns `UIManager` onto the screen, so the leaderboard panel’s `UIManager` pointer was null at runtime.
  - Fix:
    - `ST66LeaderboardPanel` gained `SetUIManager(UT66UIManager*)`
    - `UT66MainMenuScreen::OnScreenActivated_Implementation()` injects `UIManager` into the leaderboard panel after activation.
- **Leaderboard-view Run Summary is Back-only**
  - When `UT66RunSummaryScreen` is opened in saved-leaderboard mode, it now renders only a single **BACK** button (closes modal).
  - In normal run-death mode it still shows **RESTART + MAIN MENU**.
- **Leaderboard display tweaks**
  - Unset times now show **0** (not `--:--`) for the local “YOU” entry.

**Verification / proof**
- Build (UE 5.7) ✅
  - `Build.bat T66Editor Win64 Development "C:\UE\T66\T66.uproject" -WaitMutex -FromMsBuild -architecture=x64`
  - `Build.bat T66 Win64 Development "C:\UE\T66\T66.uproject" -WaitMutex -FromMsBuild -architecture=x64`

### 2026-02-01 — HUD timer top-center + in-world Vendor/Gambler dialogue + NPC face-player rotation

**Goal**
- Move the stage timer to the **top-center of the whole screen** (not inside the minimap).
- Replace the Vendor/Gambler “full-screen dialogue” opening flow with an **in-world 3-option selector**:
  - Option 1: Shop / Gamble
  - Option 2: Teleport me to your brother
  - Option 3: Back
- Make house NPCs **rotate in place to face the player** (Vendor, Gambler, Saint, Ouroboros, Trickster, etc.).

**What changed**
- **HUD timer placement**:
  - `Source/T66/UI/T66GameplayHUDWidget.cpp` — timer moved to a root overlay slot at `HAlign_Center/VAlign_Top` (removed from minimap overlay).
- **In-world dialogue selector (HUD-rendered)**:
  - `Source/T66/UI/T66GameplayHUDWidget.h/.cpp` — added a lightweight 3-row dialogue panel that is positioned using `RenderTransform` (updated only while the dialogue is open).
  - `Source/T66/Gameplay/T66PlayerController.h/.cpp` — added world dialogue state + input handling:
    - Default selection is the top option.
    - Navigate with **W/S** (or controller stick via `MoveForward` axis) with debounce; confirm with **Interact (F)**.
    - Option 1 opens the existing overlays directly to **Shop/Casino pages** (skips the overlay dialogue page).
    - Option 2 teleports to the other NPC (blocked when `RunState->GetBossActive()` is true, matching overlay behavior).
- **Vendor/Gambler interaction change**:
  - `Source/T66/Gameplay/T66VendorNPC.cpp` and `Source/T66/Gameplay/T66GamblerNPC.cpp` now open the in-world dialogue instead of directly opening overlays.
  - `Source/T66/Gameplay/T66GamblerNPC.h` — added `GetWinGoldAmount()` so the overlay keeps the same win gold tuning when opened from the dialogue.
- **Overlay helpers**:
  - `Source/T66/UI/T66VendorOverlayWidget.h/.cpp` — added `OpenShopPage()`.
  - `Source/T66/UI/T66GamblerOverlayWidget.h/.cpp` — added `OpenCasinoPage()`.
- **NPC face-player rotation**:
  - `Source/T66/Gameplay/T66HouseNPCBase.h/.cpp` — enabled ticking at 0.05s interval and smoothly rotates yaw toward the player (distance-gated).

**Localization**
- No new player-facing strings added; re-used existing `UT66LocalizationSubsystem` texts for the options.

**Verification / proof**
- Builds ✅ (UE 5.7):
  - `& 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat' T66Editor Win64 Development -Project='C:\UE\T66\T66.uproject' -WaitMutex -FromMsBuild`
  - `& 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat' T66 Win64 Development -Project='C:\UE\T66\T66.uproject' -WaitMutex -FromMsBuild`

### 2026-02-01 — Bigger main map + fully enclosed walls + corner houses fully inside bounds

**Goal**
- Make the main map bigger.
- Ensure the entire playable area has walls so there is **no way to fall off** the world.
- Ensure corner houses/NPC safe-zones (“bubbles”) are fully inside the map (no safe-zone disc hanging off the edge).

**What changed**
- **Main floor size increased**:
  - `AT66GameMode::SpawnFloorIfNeeded()` — main floor scale **140×140 → 160×160** (14,000 → 16,000).
- **Boundary walls now actually enclose the main square** (not just the far start/boss footprint):
  - `AT66GameMode::SpawnBoundaryWallsIfNeeded()` — kept the global N/S walls, but added **MainEdge** E/W wall segments at \(X=\pm 8000\) for \( |Y| > 3000 \) so the player can’t fall off the main square’s east/west edges.
  - Also changed floor/wall setup to **spawn-or-update by tag** (so existing tagged actors get resized/repositioned when constants change).
- **Corner houses + NPC safe-zones fully inside bounds**:
  - `AT66GameMode::SpawnCornerHousesAndNPCs()` — moved house corners outward for the larger map and moved NPC cylinders **inward (toward map center)** so their `SafeZoneRadius` disc can’t spill outside the floor.
- **Spawn bounds updated for the larger main**:
  - `SpawnWorldInteractablesForStage()` and `SpawnStageEffectTilesForStage()` main extents updated to **7200** (keeps margin from walls and safe-zones).
- **Miasma coverage expanded for bigger map**:
  - `AT66MiasmaManager::CoverageHalfExtent` **6500 → 7800**.

**Build fix (needed for ValidateFast)**
- `Source/T66/Core/T66WebView2Host.cpp` — split the large WebView2 TikTok JS raw-string literals into multiple concatenated raw strings to fix MSVC `C2026: string too big`.

**Verification / proof**
- ValidateFast (UE 5.7) ✅
  - `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66Editor Win64 Development "C:\UE\T66\T66.uproject" -WaitMutex -FromMsBuild -architecture=x64`
  - `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66 Win64 Development "C:\UE\T66\T66.uproject" -WaitMutex -FromMsBuild -architecture=x64`

**Follow-up (PIE warning fix)**
- Fixed PIE warnings like:
  - `StaticMeshComponent0 has to be 'Movable' if you'd like to move`
- `AT66GameMode` now temporarily sets spawned/updated dev geometry (`AStaticMeshActor` floors/walls) to **Movable** while applying transform, then back to **Static**.

**Follow-up (remove placeholder houses)**
- Removed the corner “house cube” visuals and kept only the NPCs + safe-zone discs:
  - `AT66GameMode::SpawnCornerHousesAndNPCs()` no longer spawns `AT66HouseBlock`.
- Deleted unused placeholder house actor class:
  - `Source/T66/Gameplay/T66HouseBlock.h/.cpp`

### 2026-02-01 — Tutorial arena + first-time tutorial flow (on-screen text above crosshair)

**Goal**
- Add a dedicated enclosed **Tutorial Area** inside `GameplayLevel` and make it the spawn for **first-time players**.
- Drive the tutorial experience with **on-screen text above the crosshair** (no world-space prompt actors).
- End tutorial via a **portal teleport** (no level load).
- Tutorial loot bag should drop an item whose **primary stat line matches the hero’s highest stat**.

**What changed**
- **Profile tracking (first-time detection)**
  - `UT66ProfileSaveGame`:
    - Added `bHasCompletedTutorial` (profile lifetime flag).
    - Bumped `SaveVersion` to 3 (safe default clamp).
  - `UT66AchievementsSubsystem`:
    - Added `HasCompletedTutorial()` and `MarkTutorialCompleted()` (persists to `T66_Profile`).
- **Tutorial spawn behavior**
  - `AT66GameMode::SpawnDefaultPawnFor_Implementation()`:
    - Spawns into the tutorial arena if `bForceSpawnInTutorialArea` is true **or** the profile has not completed the tutorial yet.
  - `AT66GameMode::SpawnTutorialIfNeeded()`:
    - Spawns `AT66TutorialManager` only when Stage 1 and tutorial is forced/first-time.
- **On-screen tutorial text (above crosshair)**
  - `UT66RunStateSubsystem`:
    - Added tutorial hint state (`TutorialHintLine1/2`, `bTutorialHintVisible`) + `TutorialHintChanged` delegate.
    - Added tutorial input flags (`bTutorialMoveInputSeen`, `bTutorialJumpInputSeen`) + `TutorialInputChanged` delegate.
  - `UT66GameplayHUDWidget`:
    - Added a tutorial hint panel above the crosshair (event-driven; no per-frame UI polling).
- **Tutorial flow system**
  - `AT66TutorialManager`:
    - Event-driven state machine that shows exact requested text and spawns:
      - 1 enemy → black loot bag (hero-highest-stat tutorial item)
      - 1 “elite” (mini-boss scaled) enemy → white loot bag
      - 10-enemy wave
      - portal spawn when wave cleared
    - Uses `NSLOCTEXT("T66.Tutorial", ...)` for all tutorial hint strings.
  - `AT66TutorialPortal` (new):
    - Interact (F) teleports the player to the normal Stage 1 start area **without loading**.
    - Marks tutorial completed in profile and clears tutorial hint.
  - `AT66PlayerController::HandleInteractPressed()`:
    - Added support for interacting with `AT66TutorialPortal`.
  - `AT66PlayerController` movement/jump handlers:
    - Notify `UT66RunStateSubsystem` the first time Move/Jump inputs occur (for tutorial progression).
- **Tutorial stat-matched item**
  - `UT66GameInstance::GetItemData()`:
    - Added synthetic “tutorial-only” item rows (no content DT changes required):
      - `Item_Tutorial_Damage`, `Item_Tutorial_AttackSpeed`, `Item_Tutorial_AttackSize`, `Item_Tutorial_Armor`, `Item_Tutorial_Evasion`, `Item_Tutorial_Luck`
    - These have `MainStatType/MainStatValue` so they correctly grant the intended primary stat line.

**Localization**
- Ran Gather → Translate → Compile for all tutorial hint strings.

**Verification / proof**
- Source gather:
  - `& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "C:\UE\T66\T66.uproject" -run=GatherText -config="C:\UE\T66\Config\Localization\T66_Gather_Source.ini" ...`
- Translate archives:
  - `python -u "C:\UE\T66\Scripts\AutoTranslateLocalizationArchives.py"`
- Compile:
  - `& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "C:\UE\T66\T66.uproject" -run=GatherText -config="C:\UE\T66\Config\Localization\T66_Compile.ini" ...`
- `.locres` count: **22**

### 2026-02-01 — Idols v1: per-boss altar drops + 10-level stacking + HUD level pips + dev toggles

**Goal**
- Remove idol altar from tutorial + Stage 1 start; instead **drop an idol altar after every boss kill** (every stage).
- Add idol stacking: selecting the same idol again increases its level (**up to 10**).
- Show idol level as **small yellow pips** in each idol slot.
- Add always-visible HUD toggles for:
  - **Immortality** (can take damage, hearts can hit 0, but run never ends)
  - **Power** (auto-attack damage = 999999)

**What changed**
- **Idol altar spawn timing**
  - `AT66GameMode::RestartPlayer()` no longer spawns an altar in Stage 1.
  - `AT66GameMode::HandleBossDefeatedAtLocation()` now spawns `AT66IdolAltar` after **every** boss kill.
  - `SpawnStageGateAtLocation()` and tutorial ground traces updated to be more robust (WorldStatic fallback to Visibility) to avoid slight floating.
- **Idol leveling (runtime)**
  - `UT66RunStateSubsystem`:
    - Added `EquippedIdolLevels` + `MaxIdolLevel = 10`.
    - Added `SelectIdolFromAltar()`:
      - If idol already equipped → level++ (clamped 1..10)
      - Else equip into first empty slot at level 1.
  - `UT66IdolAltarOverlayWidget` now calls `SelectIdolFromAltar()` so the altar supports repeated selections for leveling.
- **HUD idol level pips**
  - `UT66GameplayHUDWidget` draws a 2×5 pip grid inside each idol slot and toggles visibility based on idol level.
- **Idol levels DataTable (data-only for now)**
  - Added `FIdolData` to `T66DataTypes.h` with `Level01Value..Level10Value`.
  - Added `Content/Data/Idols.csv`.
  - Added `Content/Data/DT_Idols.uasset` and `Scripts/SetupIdolsDataTable.py`.
  - `UT66GameInstance` now includes `IdolsDataTable` + `GetIdolData()`.
- **Dev toggles**
  - `UT66RunStateSubsystem`: added `bDevImmortality`, `bDevPower` + `DevCheatsChanged` delegate.
  - Immortality is enforced in `ApplyDamage`, `ApplyTrueDamage`, `KillPlayer`, `EndLastStandAndDie`.
  - Power is enforced in `UT66CombatComponent` by overriding `EffectiveDamagePerShot = 999999`.
  - HUD buttons added near difficulty skulls:
    - `IMMORTALITY: ON/OFF`
    - `POWER: ON/OFF`
  - Keybinds:
    - `1` toggles Immortality (`ToggleImmortality`)
    - `2` toggles Power (`TogglePower`)

**Tutorial grounding polish**
- `AT66LootBagPickup` visuals offset adjusted to sit on the ground.
- `AT66TutorialPortal` visuals offset adjusted to sit on the ground.

**Verification / proof**
- Build (UE 5.7) ✅
  - `Build.bat T66Editor Win64 Development "C:\UE\T66\T66.uproject" -WaitMutex -FromMsBuild -architecture=x64`
- Localization ✅
  - Ran Gather → Translate → Compile; `.locres` count: **22**

### 2026-01-31 — In-run HUD overhaul (map/minimap) + lock-on/crosshair + range ring + wheel HUD spin + TikTok placeholder

**HUD layout + map/minimap**
- Rebuilt `UT66GameplayHUDWidget` layout to match updated spec:
  - Full-screen map overlay toggle via **OpenFullMap** (default **M**) and minimap in top-right.
  - Difficulty squares moved beneath minimap and sized to minimap width.
  - Stage timer moved above stage number.
  - Portrait resized so width matches the hearts row; hearts enlarged.
  - Survival “bar” visuals removed (system retained).
  - Added 2×3 idol slots left of portrait.
  - Level ring moved above stats; ring thickness increased.
  - Stats order updated (SPD moved below LCK); Ultimate block positioned to the right of stats and shifts when stats panel is hidden.
  - Inventory expanded to **2×10 = 20 slots** and moved to bottom-right; Gold/Owe moved above items with matched font size.
- Added `ST66WorldMapWidget` (Slate leaf widget) for world→map projection:
  - Full map draws areas + NPC markers with labels; minimap draws markers without labels.
  - Map/minimap centered on hero; hero/minimap icon clamped to bounds.
  - ESC closes full-map first before pause.

**Combat targeting UX**
- Added centered crosshair (screen-space) and click-to-lock targeting:
  - Clicking an enemy locks them; auto-attacks prioritize locked target if in range.
  - Enemy health bars restored via `UWidgetComponent` and show a red lock indicator above the bar when locked.

**Attack range indicator**
- Added a toggleable (HUD-panels-linked) hero attack range ring:
  - Range scales with Scale stat (same multiplier family).
  - Display ring built as a thin segmented circle to avoid “white floor” artifacts from a filled disk.
  - Base attack range tuned to **1000**.

**Auto-attack behavior**
- Auto-attacks are now “sure fire”: projectiles home to the intended target and ignore other overlaps so they can’t hit the wrong enemy.
- Hero projectile tint is driven by equipped idol color (`UT66RunStateSubsystem::GetIdolColor`), using material param **Color** (with BaseColor fallback).

**Wheel + TikTok HUD additions**
- Wheel interact no longer opens `UT66WheelOverlayWidget`; instead it plays a HUD-side spin animation panel and awards gold on resolve.
- Added TikTok placeholder panel (portrait/phone aspect) under the speedrun block; toggleable via new **ToggleTikTok** action (default **O**) and exposed in Settings rebind list.

**Verification**
- `T66Editor` builds successfully after changes.

### 2026-01-31 — Media Viewer v0 (TikTok via Unreal WebBrowser/CEF): locked to TikTok + persistent login

**Goal**
- Replace the TikTok HUD placeholder with a real, in-game Chromium (CEF) browser panel.
- Prevent access to arbitrary websites (strict allowlist).
- Persist login across sessions (cookie storage), without exposing a general-purpose browser UI.

**What changed**
- Enabled Unreal’s browser stack:
  - `T66.uproject`: enabled plugin `WebBrowserWidget`.
  - `Source/T66/T66.Build.cs`: added `WebBrowser` module dependency.
- `UT66GameplayHUDWidget`:
  - TikTok panel now lazy-creates an `SWebBrowser` instance on first open and mounts it into the existing phone-shaped panel.
  - No controls/address bar; context menu suppressed.
  - Popups blocked (`OnBeforePopup` suppressed).
  - Navigation locked via `OnBeforeNavigation`: only allows TikTok + required TikTok CDNs (no general browsing).
  - Browser context uses a dedicated cookie store:
    - `bPersistSessionCookies=true`
    - `CookieStorageLocation=<Project>/Saved/TikTokBrowser/Cookies`
    - Note: “stay logged in” inherently requires saving site cookies/data for TikTok; we isolate it to a dedicated folder and block non-allowlisted navigation.
- `AT66PlayerController`:
  - Opening the viewer switches to `FInputModeGameAndUI`, shows cursor, and suppresses movement/look input so the page is usable.
  - `Esc` closes the viewer first (back behavior), then proceeds to full-map/pause behavior.
  - Combat mouse actions are suppressed while the viewer is open.

**Verification / proof**
- Built successfully (UE 5.7):
  - `cmd /c "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66Editor Win64 Development "C:\UE\T66\T66.uproject" -WaitMutex -FromMsBuild -architecture=x64` ✅
  - `cmd /c "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66 Win64 Development "C:\UE\T66\T66.uproject" -WaitMutex -FromMsBuild -architecture=x64` ✅

### 2026-01-31 — Media Viewer v1 (Windows-only): WebView2 overlay for real TikTok video playback (H.264/AAC)

**Why**
- UE’s bundled CEF build cannot play TikTok’s typical MP4 streams (H.264/AAC), which caused “having trouble playing this, please refresh”.
- WebView2 uses the installed Edge runtime, so H.264/AAC playback works on Windows.

**What changed**
- Added WebView2 SDK bits to the repo (from NuGet) for deterministic builds:
  - `ThirdParty/WebView2/include/WebView2.h`
  - `ThirdParty/WebView2/include/WebView2EnvironmentOptions.h`
  - `ThirdParty/WebView2/bin/Win64/WebView2Loader.dll`
- `Source/T66/T66.Build.cs`:
  - Adds WebView2 include path on Win64
  - Stages/copies `WebView2Loader.dll` to `Binaries/Win64` via `RuntimeDependencies`
  - Defines `T66_WITH_WEBVIEW2=1` on Win64
- Implemented a minimal WebView2 host:
  - `Source/T66/Core/T66WebView2Host.h/.cpp`
  - Dynamic-loads `WebView2Loader.dll`
  - Uses persistent `UserData` at `Saved/TikTokWebView2/UserData` so login persists across sessions
  - Blocks popups and enforces a TikTok/CDN allowlist on navigation
- Wired WebView2 into the existing Media Viewer toggle:
  - `Source/T66/Core/T66MediaViewerSubsystem.h/.cpp` shows/hides the WebView2 overlay on open/close
  - `Source/T66/UI/T66GameplayHUDWidget.cpp` avoids creating the CEF browser on Win64 when WebView2 is enabled

**Notes**
- This path is intended for Windows-only TikTok support. Non-Windows platforms should not rely on TikTok playback (Steam Deck version will omit the feature).

**Verification / proof**
- ValidateFast (UE 5.7) ✅
  - `cmd /c "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66Editor Win64 Development "C:\UE\T66\T66.uproject" -WaitMutex -FromMsBuild -architecture=x64`
  - `cmd /c "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66 Win64 Development "C:\UE\T66\T66.uproject" -WaitMutex -FromMsBuild -architecture=x64`
  - Verified `WebView2Loader.dll` exists at `Binaries/Win64/WebView2Loader.dll`.

### 2026-01-31 — TikTok/WebView2: fix black/desktop-bleed rendering + add deep runtime logs

**Problem**
- Toggling the TikTok viewer could show a **black window** and/or **desktop bleed** / “hole” effect where WebView2 content should be.

**Root cause**
- WebView2 controller was being hidden via `Ctrl->put_IsVisible(FALSE)` on close, but was **not being re-shown** on subsequent open (`put_IsVisible(TRUE)` missing on show).
- WebView2 default background can be transparent depending on runtime/controller behavior; combined with overlay/windowing, this can look like bleed.

**What changed**
- `Source/T66/Core/T66WebView2Host.cpp`
  - Ensure the controller is re-shown on open (`put_IsVisible(TRUE)` in show path).
  - Force an **opaque** black background via `ICoreWebView2Controller2::put_DefaultBackgroundColor`.
  - Added detailed `[TIKTOK][WEBVIEW2]` logs: owner/host `HWND`, parent/owner/rootOwner, style/exstyle flags, window/client rects, DPI, environment/controller creation outcomes, and show rects.
  - Call `NotifyParentWindowPositionChanged()` after applying bounds.
- `Source/T66/Core/T66MediaViewerSubsystem.cpp`
  - More robust parent `HWND` selection (`FindBestParentWindowHandleForDialogs` fallback).
  - Avoid repeated `MoveWindow` spam by ignoring unchanged rect updates.

**Verification / proof**
- Build (UE 5.7) ✅
  - `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66Editor Win64 Development -Project="C:\UE\T66\T66.uproject" -WaitMutex -FromMsBuild -architecture=x64`
  - `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66 Win64 Development -Project="C:\UE\T66\T66.uproject" -WaitMutex -FromMsBuild -architecture=x64`

### 2026-01-31 — TikTok/WebView2: keep gameplay input + Q/E next/prev + video-only UI

**User request**
- Keep TikTok visible but **do not lock** player movement/camera (no mouse cursor, no input-mode swap).
- Add keyboard-only controls: **Q = previous**, **E = next** TikTok.
- Hide TikTok chrome so **only the video** shows in the phone panel.

**What changed**
- `Source/T66/Gameplay/T66PlayerController.cpp/.h`
  - Removed the MediaViewer/TikTok toggle behavior that switched to `FInputModeGameAndUI` and suppressed movement/look.
  - Added hard key binds:
    - `Q` → previous TikTok (only when viewer is open)
    - `E` → next TikTok (only when viewer is open)
  - Media viewer no longer blocks combat mouse input (`CanUseCombatMouseInput` no longer treats TikTok as a modal overlay).
- `Source/T66/Core/T66MediaViewerSubsystem.h/.cpp`
  - Added `TikTokPrev()` / `TikTokNext()` helpers that forward to the WebView2 host.
- `Source/T66/Core/T66WebView2Host.h/.cpp`
  - Host window no longer steals focus and no longer intercepts mouse clicks (`WS_EX_NOACTIVATE | WS_EX_TRANSPARENT`).
  - Added best-effort CSS injection on navigation completed to hide common TikTok chrome (nav/header/footer/sidebar) so the phone panel shows mostly the video content.
  - Implemented `PrevVideo()` / `NextVideo()` by scrolling the page smoothly (no mouse required).

**Verification / proof**
- Build (UE 5.7) ✅
  - `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66Editor Win64 Development -Project="C:\UE\T66\T66.uproject" -WaitMutex -FromMsBuild -architecture=x64`
  - `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66 Win64 Development -Project="C:\UE\T66\T66.uproject" -WaitMutex -FromMsBuild -architecture=x64`

### 2026-01-31 — TikTok/WebView2: Q/E snap to next/prev video card (deterministic)

**Problem**
- Q/E “next/prev” behaved like small scroll steps and did not reliably advance exactly one video.

**What changed**
- `Source/T66/Core/T66WebView2Host.cpp`
  - Replaced the naive scroll-by approach with a deterministic “card snap”:
    - Find all visible `<video>` elements
    - Pick the current card closest to viewport center
    - Pick the next/prev card by vertical center position
    - Scroll the correct scroll container so the target card is centered (instant snap), then nudge playback

**Verification / proof**
- Build (UE 5.7) ✅
  - `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66Editor Win64 Development -Project="C:\UE\T66\T66.uproject" -WaitMutex -FromMsBuild -architecture=x64`
  - `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66 Win64 Development -Project="C:\UE\T66\T66.uproject" -WaitMutex -FromMsBuild -architecture=x64`

### 2026-01-31 — TikTok/WebView2: stop audio bleed + don't re-navigate on toggle

**Problems**
- After Q/E snap, the **previous video's audio** could continue playing.
- Toggling TikTok off/on would **navigate back to QR login** and effectively “re-login” each time (even though cookies persisted).

**What changed**
- `Source/T66/Core/T66WebView2Host.cpp` / `.h`
  - Q/E scripts now:
    - Pause/mute all videos immediately
    - Snap to next/prev card
    - After scroll settles, pause everything again and **play/unmute only the most-visible video**
  - `Hide()` now executes a pause/mute script before hiding so audio never leaks when the overlay is hidden.
  - `ShowAtScreenRect()` now resumes playback of the most-visible video when re-shown.
  - Added `HasEverNavigated()` so higher-level code can avoid re-navigating every toggle.
- `Source/T66/Core/T66MediaViewerSubsystem.cpp`
  - Only calls `Navigate("https://www.tiktok.com/login/qrcode")` on the **first ever open** (when WebView2 hasn’t navigated yet).
  - Subsequent toggles just show/hide the existing session.

**Verification / proof**
- Build (UE 5.7) ✅
  - `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66Editor Win64 Development -Project="C:\UE\T66\T66.uproject" -WaitMutex -FromMsBuild -architecture=x64`
  - `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66 Win64 Development -Project="C:\UE\T66\T66.uproject" -WaitMutex -FromMsBuild -architecture=x64`

### 2026-02-01 — TikTok/WebView2: “perfect” Q/E next/prev (responsive + no stuck-muted) + fix MSVC string literal limit

**Problems**
- Q/E “next/prev” was laggy/flaky (sometimes only worked once), and sometimes landed on a muted next video / weird audio state.
- `T66Editor` build could fail with: `error C2026: string too big` in `T66WebView2Host.cpp` due to the giant injected JS raw string literal.

**What changed**
- `Source/T66/Core/T66WebView2Host.cpp`
  - Replaced the giant `PrevVideo()` / `NextVideo()` raw JS literals with a shared `MoveVideo(dir)` that **builds JS via `FString` concatenation** (avoids MSVC string literal size limits).
  - Reworked Q/E JS to be fast + deterministic:
    - Uses `document.elementsFromPoint(center)` to identify the “current” video cheaply (fallback to a small bounded scan).
    - Picks the next/prev **neighbor `<video>`** by vertical center (bounded scan) and snaps it to viewport center.
    - Waits for **scroll-idle + stable centered element** (short bounded wait) before enforcing playback.
    - Autoplay hardening: **play muted first**, then **unmute after play starts** with a few-frame retry to avoid “stuck muted”.
    - Ensures one audio source by pausing/muting all other videos after the target begins playing.
  - `ShowAtScreenRect()` resume script hardened similarly (play muted → unmute shortly after) so toggling off/on doesn’t resume into a muted-only state.

**Notes**
- No new player-facing strings were added/changed.

**Verification / proof (ValidateFast, UE 5.7)** ✅
- `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66Editor Win64 Development -Project="C:\UE\T66\T66.uproject" -WaitMutex -FromMsBuild -architecture=x64` ✅
- `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66 Win64 Development -Project="C:\UE\T66\T66.uproject" -WaitMutex -FromMsBuild -architecture=x64` ✅

### 2026-02-01 — TikTok/WebView2: remove “half/half” landings + fix “plays 1s then pauses” + swap Q/E mapping

**Problems**
- Sometimes next/prev would land “between” two videos (split view: bottom of one + top of the next).
- Sometimes after a successful next/prev, the video would play briefly then auto-pause.
- Input preference change: **Q = Next**, **E = Previous**.

**What changed**
- `Source/T66/Gameplay/T66PlayerController.cpp`
  - Swapped hard key bindings: `Q` → next, `E` → previous (still only active while TikTok viewer is open).
- `Source/T66/Core/T66WebView2Host.cpp`
  - Navigation move script now scrolls toward the target with **smooth** scrolling, then **re-centers** the final chosen video after scroll-idle so it lands on a single full video (no “half/half” final state).
  - Added a short “keep playing” watchdog after landing: if TikTok pauses the video shortly after, we re-`play()` (muted-first → unmute) and re-assert single-audio-source.
  - Resume-on-toggle script also retries playback once ~1.1s later if the video auto-pauses after resume.

**Notes**
- No new player-facing strings were added/changed.

**Verification / proof (ValidateFast, UE 5.7)** ✅
- `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66Editor Win64 Development -Project="C:\UE\T66\T66.uproject" -WaitMutex -FromMsBuild -architecture=x64` ✅
- `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66 Win64 Development -Project="C:\UE\T66\T66.uproject" -WaitMutex -FromMsBuild -architecture=x64` ✅

### 2026-02-01 — TikTok/WebView2: hard “no audio leak” guarantee on toggle-off + reduce stutter fighting

**Problems**
- TikTok audio could continue playing after toggling the viewer off (this must never happen).
- After next/prev, playback could stutter/stop due to over-aggressive pause/play fighting.

**What changed**
- `Source/T66/Core/T66WebView2Host.cpp`
  - Added WebView2-level global mute using `ICoreWebView2_8::put_IsMuted(...)`:
    - `Hide()` now calls `SetWebViewMuted(true)` **before** any JS executes (unbreakable mute).
    - `ShowAtScreenRect()` calls `SetWebViewMuted(false)` before resuming playback.
  - `Hide()` JS now:
    - Sets `window.__t66TikTok.hidden=true` and bumps `__moveId` to cancel any in-flight keep-alive loops.
    - Mutes/pauses both `video` **and** `audio` tags.
  - Move JS now respects `TT.hidden` and avoids immediate “pause everything” behavior (muting first, pausing after target is stable) to reduce stutter.

**Notes**
- No new player-facing strings were added/changed.

**Verification / proof (ValidateFast, UE 5.7)** ✅
- `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66Editor Win64 Development -Project="C:\UE\T66\T66.uproject" -WaitMutex -FromMsBuild -architecture=x64` ✅
- `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66 Win64 Development -Project="C:\UE\T66\T66.uproject" -WaitMutex -FromMsBuild -architecture=x64` ✅

### 2026-02-01 — TikTok/WebView2: CapsLock toggle + Q/E navigation no longer pauses/mutes

**User request**
- Fix “play/stop/play/stop” stutter by ensuring navigation does **not** pause or mute videos.
- Remap TikTok toggle to **CapsLock**.

**What changed**
- `Config/DefaultInput.ini`
  - `ToggleTikTok` key changed **O → CapsLock**
  - `Dash` key changed **CapsLock → LeftShift** (to avoid double-binding conflicts)
- `Source/T66/Core/T66WebView2Host.cpp`
  - Q/E navigation JS no longer calls `pause()` or sets `muted=true` (only `Hide()` is allowed to pause/mute).
  - Resume script un-mutes the chosen visible video on show (since hide explicitly mutes/pauses).
- `Source/T66/Gameplay/T66PlayerController.cpp`
  - Updated comment to reflect Dash key moved to `DefaultInput.ini` (now LeftShift by default).

**Notes**
- Audio is still guaranteed to never leak when toggled off via WebView2 global mute (`put_IsMuted(true)`), plus the hide pause/mute script.
- No new player-facing strings were added/changed.

**Verification / proof (ValidateFast, UE 5.7)** ✅
- `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66Editor Win64 Development -Project="C:\UE\T66\T66.uproject" -WaitMutex -FromMsBuild -architecture=x64` ✅
- `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66 Win64 Development -Project="C:\UE\T66\T66.uproject" -WaitMutex -FromMsBuild -architecture=x64` ✅

### 2026-02-01 — TikTok/WebView2: “single active video” guardian (no stutter + no old-audio bleed) + preload on gameplay map load

**Problems**
- Q/E could leave the previous TikTok audio playing.
- Many videos would “play/stop/play/stop” (TikTok page pausing itself), causing stutter and eventually stopping.
- First toggle after entering gameplay could show visible “formatting” and/or login-related work; user wants this done before first toggle.

**What changed**
- `Source/T66/Core/T66WebView2Host.cpp`
  - Replaced the Q/E move script with a persistent in-page **guardian**:
    - While visible, enforces **exactly one active (center-most) video** is **unmuted + playing**.
    - Immediately **mutes + pauses any other visible videos** so old audio cannot bleed and overall load is reduced.
    - Installs prototype hooks so the page cannot pause videos while visible (except when we explicitly allow internal pause for non-active videos).
  - `Hide()` now also stops the guardian (in addition to WebView2-level global mute + pause/mute script).
  - `ShowAtScreenRect()` resume script starts the guardian while visible.
- `Source/T66/Core/T66MediaViewerSubsystem.h/.cpp`
  - Added `PrewarmTikTok()` to create WebView2 + navigate in the background, then keep it hidden/muted.
  - Changed first navigation target to `https://www.tiktok.com/` (instead of the QR login URL) so an already-signed-in session can preload without forcing the QR flow.
- `Source/T66/Gameplay/T66PlayerController.cpp`
  - Calls `UT66MediaViewerSubsystem::PrewarmTikTok()` on gameplay `BeginPlay()` so login/session + CSS injection happen before the player first toggles the panel.

**Notes**
- No new player-facing strings were added/changed.
- WebView2 UserData still stays inside the game folder (`Saved/TikTokWebView2/UserData`).

**Verification / proof (ValidateFast, UE 5.7)** ✅
- `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66Editor Win64 Development -Project="C:\UE\T66\T66.uproject" -WaitMutex -FromMsBuild -architecture=x64` ✅
- `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66 Win64 Development -Project="C:\UE\T66\T66.uproject" -WaitMutex -FromMsBuild -architecture=x64` ✅

### 2026-02-01 — TikTok/WebView2: Q/E “commit to next/prev” (no half-scroll staying on same video)

**Problem**
- With the guardian approach, pressing **Q (next)** could sometimes scroll partially but still remain on the same video.

**What changed**
- `Source/T66/Core/T66WebView2Host.cpp`
  - `moveOnce(dir)` now **commits** to the next/prev target:
    - Finds the correct scroller for the target video.
    - Uses `snapTo(target, scroller)` (non-smooth) and waits for scroll-idle.
    - If the centered video is still the old one, retries with a larger step and finally forces the target as the active video for the guardian.

**Notes**
- No new player-facing strings were added/changed.

**Verification / proof (ValidateFast, UE 5.7)** ✅
- `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66Editor Win64 Development -Project="C:\UE\T66\T66.uproject" -WaitMutex -FromMsBuild -architecture=x64` ✅
- `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66 Win64 Development -Project="C:\UE\T66\T66.uproject" -WaitMutex -FromMsBuild -architecture=x64` ✅

### 2026-02-01 — TikTok/WebView2: force-close viewer on death (auto toggle-off)

**User request**
- TikTok/media viewer should automatically toggle off when the player dies.

**What changed**
- `Source/T66/Gameplay/T66PlayerController.cpp`
  - `OnPlayerDied()` now checks `UT66MediaViewerSubsystem` and calls `SetMediaViewerOpen(false)` before pausing and showing the Run Summary modal.

**Notes**
- No new player-facing strings were added/changed.

**Verification / proof (ValidateFast, UE 5.7)** ✅
- `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66Editor Win64 Development -Project="C:\UE\T66\T66.uproject" -WaitMutex -FromMsBuild -architecture=x64` ✅
- `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66 Win64 Development -Project="C:\UE\T66\T66.uproject" -WaitMutex -FromMsBuild -architecture=x64` ✅

### 2026-01-31 — Localization foundations + UI text migration


- Finalized the supported language list in `ET66Language` and ensured the Language Select UI enumerates `GetAvailableLanguages()`.
- Migrated player-facing hardcoded `FText::FromString(TEXT("..."))` fallbacks across UI and a few gameplay-facing prompts (NPC names/tutorial prompts) to `NSLOCTEXT` (and/or `UT66LocalizationSubsystem::GetText_*()` where applicable).
- Added a hard rule to `T66_Cursor_Guidelines.md` and the top of this `memory.md`: all future player-facing strings must be localized.
- Fixed a build warning in `T66Style.cpp` by switching to a composite-font based `FSlateFontInfo` path (non-deprecated).

**Verification**
- `Build.bat T66Editor Win64 Development ...` ✅
- `Build.bat T66 Win64 Development ...` ✅
- `Scripts\\RunFullSetup.bat` ✅ (0 errors, 0 warnings)

### 2026-01-31 — Localization pipeline verification (GatherText → archives → `.locres`)

**What changed**
- Added `NativeCulture=en` to the localization configs (UE 5.7 GatherText requires a native culture).
- Resolved `NSLOCTEXT` key conflicts (same namespace+key with different English source) so GatherText runs cleanly.
- Ensured runtime loads the target by adding `Config/DefaultGame.ini`:
  - `[Internationalization]`
  - `LocalizationPaths=%GAMEDIR%Content/Localization/T66`

**How to run (PowerShell)**
- **Source-only gather + compile:**
  - `& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "C:\UE\T66\T66.uproject" -run=GatherText -config="C:\UE\T66\Config\Localization\T66_Gather_Source.ini" -unattended -nop4 -nosplash -nullrhi -log="C:\UE\T66\Saved\Logs\T66_Gather_Source.log"`
- **Narrow assets gather + compile:**
  - `& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "C:\UE\T66\T66.uproject" -run=GatherText -config="C:\UE\T66\Config\Localization\T66_Gather_Assets.ini" -unattended -nop4 -nosplash -nullrhi -log="C:\UE\T66\Saved\Logs\T66_Gather_Assets.log"`
- **Compile-only (optional):**
  - `& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "C:\UE\T66\T66.uproject" -run=GatherText -config="C:\UE\T66\Config\Localization\T66_Compile.ini" -unattended -nop4 -nosplash -nullrhi -log="C:\UE\T66\Saved\Logs\T66_Compile.log"`
  - Note: UE may still write to `Saved/Logs/T66.log` (and `T66-backup-*.log`) depending on how it resolves `-log`.

**Artifacts**
- `Content/Localization/T66/T66.manifest`
- `Content/Localization/T66/<culture>/T66.archive`
- `Content/Localization/T66/<culture>/T66.locres`

**Proof**
- GatherText completion is visible in the command output (example: `GatherText completed with exit code 0`).
- Verified `.locres` outputs exist for **exactly 22 cultures**:
  - `Get-ChildItem -Path "C:\UE\T66\Content\Localization\T66" -Recurse -Filter *.locres | % FullName`
- Verified runtime culture override does **not** fall back due to missing localization paths:
  - `T66.exe -culture=fr ...` logs `Overriding language with command-line option (fr).` and does **not** print `No localization for 'fr' exists...`.

### 2026-01-31 — Batch translations authored (offline) + `.locres` regenerated; Language Select UX fixes

**Goal**
- Ensure runtime language switching visibly changes UI text by populating non-English translations (no longer empty archives → English fallback).
- Fix Language Select UX: persistent selection highlight, spacing regression, and ensure language names do not change when culture changes.

**What changed**
- **Offline translation pass (machine baseline)**
  - Added `Scripts/AutoTranslateLocalizationArchives.py`:
    - Reads `Content/Localization/T66/en/T66.archive`
    - Writes `Translation.Text` for every entry into each target culture’s `Content/Localization/T66/<culture>/T66.archive`
    - Uses offline Argos Translate models cached in `Saved/Localization/ArgosModels/`
    - Includes a lockfile (`Saved/Localization/AutoTranslateLocalizationArchives.lock`) to prevent concurrent runs clobbering archives.
  - Populated translations for the full 22-culture set:
    - `en`, `zh-Hans`, `zh-Hant`, `ja`, `ko`, `ru`, `pl`, `de`, `fr`, `es-ES`, `es-419`, `pt-BR`, `pt-PT`, `it`, `tr`, `uk`, `cs`, `hu`, `th`, `vi`, `id`, `ar`
- **Compile `.locres` after translation**
  - Ran compile-only:
    - `UnrealEditor-Cmd.exe ... -run=GatherText -config="Config/Localization/T66_Compile.ini" -log="Saved/Logs/T66_Compile_AfterTranslate.log"`
  - Verified `.locres` exists per-culture at: `Content/Localization/T66/<culture>/T66.locres` (sizes differ from `en`).
- **Language Select UX**
  - `UT66LanguageSelectScreen`:
    - Persistent visual highlight is bound to `PreviewedLanguage` (stays highlighted until another is clicked).
    - Reverted list spacing/centering to the prior feel (after the scrollability work) while keeping highlight.
    - Confirm now applies culture after closing the modal to avoid the selector’s own localized UI visibly switching mid-interaction.
  - `UT66LocalizationSubsystem::GetLanguageDisplayName`:
    - Changed to `FText::AsCultureInvariant(...)` so language names in the selector remain in their native forms and do **not** change when the game culture changes.

**Verification**
- `T66Editor` builds successfully after changes.
- Compile commandlet completed with exit code 0 and wrote `.locres` for all cultures.

### 2026-01-31 — Character visuals pipeline + 3D previews overhaul + stage tiles visibility + boundary/wall fixes

**Imported character visuals (data-driven)**
- Added `FT66CharacterVisualRow` to `Source/T66/Data/T66DataTypes.h` (soft refs: `USkeletalMesh`, optional `UAnimationAsset`, plus offset/rot/scale + grounding flags).
- Added `UT66CharacterVisualSubsystem` (`Source/T66/Core/T66CharacterVisualSubsystem.*`):
  - Loads `DT_CharacterVisuals` (prefers `UT66GameInstance::CharacterVisualsDataTable`, falls back to `/Game/Data/DT_CharacterVisuals`).
  - Applies skeletal mesh + transform to a `USkeletalMeshComponent`, hides placeholder mesh, optional single-node looping animation.
  - Grounding fix: for `ACharacter`, auto-aligns **mesh bottom** to **capsule bottom** (fixes “waist pivot” meshes).
  - Fallback animation lookup: if no explicit looping anim, queries Asset Registry for any `UAnimSequence` compatible with the mesh skeleton (prefers names containing idle/walk/run/loop).
- Updated `T66.Build.cs` to include `AssetRegistry`.
- Added `Content/Data/CharacterVisuals.csv` and scripts:
  - `Scripts/SetupCharacterVisualsDataTable.py` (creates/fills `DT_CharacterVisuals`)
  - `Scripts/ValidateCharacterVisuals.py` (validates CSV asset paths exist)

**Actor wiring to visuals**
- Heroes: `AT66HeroBase` applies visuals on initialize.
- Enemies: `AT66EnemyBase` applies visuals by `CharacterVisualID` (and includes multiple safety fallbacks).
- Bosses: `AT66BossBase` applies visuals by `BossID`.
- Companions: `AT66CompanionBase` gained a dedicated skeletal mesh component; applies visuals; preview mode supported.
- House NPCs: `AT66HouseNPCBase` applies visuals; snaps actor to ground; keeps safe-zone bubble behavior.
- Vendor/Gambler boss rule enforced: boss variants reuse the NPC mesh (separate IDs for transforms).

**Enemy visibility policy changes (per user request)**
- Regular enemies and unique debuff enemies are forced to **use placeholder visuals** (colored spheres) instead of FBX visuals.
  - Regular enemy: `AT66EnemyBase` uses sphere placeholder and skips visuals mapping when `CharacterVisualID == RegularEnemy` (or none).
  - Unique debuff enemy: `AT66UniqueDebuffEnemy` uses a purple sphere placeholder and remains flying.
- `AT66EnemyDirector` made robust to misconfigured `EnemyClass` (falls back to `AT66EnemyBase` for regular spawns).

**3D preview stages (Hero/Companion selection)**
- `AT66HeroPreviewStage` / `AT66CompanionPreviewStage`:
  - Real-time `SceneCapture2D` (smooth orbit; no stutter on drag).
  - Drag input updated to orbit: horizontal rotates yaw; vertical adjusts camera pitch.
  - Pitch clamped and camera Z clamped so you can’t “look under” the platform.
  - Preview platform is stable (orbit framing cached) and characters are forced static (no preview animation).
  - Removed preview extra directional light to avoid forward-shading directional-light competition issues; rely on point/fill/rim lights.
- UI drag widget updates in:
  - `Source/T66/UI/Screens/T66HeroSelectionScreen.cpp`
  - `Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp`
- Pre-warm preview to avoid first-open pop-in: `AT66FrontendGameMode::BeginPlay()` sets initial hero/companion previews.

**Run Summary 3D previews**
- `UT66RunSummaryScreen` now reuses the same preview-stage render targets (hero + companion) so it matches selection screens; displayed side-by-side.

**Stage effects visibility**
- `AT66StageEffectTile` now renders as a thin colored rectangular panel slightly above the ground (no longer buried / z-fighting).

**Map walls**
- `AT66GameMode` now spawns extra edge walls around narrower Start/Boss/connector platforms (`SpawnPlatformEdgeWallsIfNeeded`) so there are no “side gaps” before the global boundary walls.

### 2026-01-31 — Achievements v0: persistent AC wallet + first real achievement (Kill 20 enemies → claim 250 AC)

**User request**
- Create a real Achievement Coins (AC) wallet starting at **0**.
- Make the first Black achievement real: **Kill 20 enemies**.
- Reward should be **250 AC**, claimable after reaching 20 kills, and the claim should update the AC wallet/balance.

**What changed (C++ only; no Content/.uasset edits)**
- Added profile save (lifetime progression, not run-slot):
  - `Source/T66/Core/T66ProfileSaveGame.h` — stores `AchievementCoinsBalance` + per-achievement `FT66AchievementState` + `LifetimeEnemiesKilled`.
- Added achievements/wallet subsystem:
  - `Source/T66/Core/T66AchievementsSubsystem.h/.cpp`
  - Saves to slot: `T66_Profile` (throttled to avoid per-kill disk IO; forced save on unlock/claim).
  - Implemented `ACH_BLK_001`: requirement 20 kills, reward 250 AC, lifetime progress.
- Wired enemy deaths to achievement progress:
  - `Source/T66/Gameplay/T66EnemyBase.cpp` — `OnDeath()` now calls `UT66AchievementsSubsystem::NotifyEnemyKilled(1)`.
- Updated Achievements screen to be real (no placeholder coins/rows):
  - `Source/T66/UI/Screens/T66AchievementsScreen.h/.cpp` — shows real AC balance, real progress, and a **Claim** button when unlocked.
**Note**
- `UT66AchievementsSubsystem` currently uses safe fallback text for achievement name/description. Localization hooks for achievements can be added later in `UT66LocalizationSubsystem` when desired.

**Verification / proof**
- Built successfully (UE 5.7):
  - `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66Editor Win64 Development -Project="C:\UE\T66\T66.uproject" -WaitMutex -FromMsBuild -architecture=x64` ✅
  - `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66 Win64 Development -Project="C:\UE\T66\T66.uproject" -WaitMutex -FromMsBuild -architecture=x64` ✅
  - Note: existing warning in `T66Style.cpp` about `FSlateFontInfo` constructor (pre-existing).

### 2026-01-31 — Stages v1: 66 stages + 66 bosses, stage-scaled boss stats, stage-variant unique enemy, idol altar on 5/0 boss clears

**User request**
- Build out **all 66 stages**, each with its own boss, a per-stage “unique enemy” visual, and stage effects as colored ground tiles.
- After killing bosses on stages ending in **5 or 0**, spawn an **Idol Altar** to pick an idol.
- Bosses need real stats (HP/damage/etc) that **scale up** as stages increase.

**What changed**
- Data expanded to 66 stages / 66 bosses:
  - `Content/Data/Stages.csv` now includes `Stage_01` … `Stage_66` (BossID per stage + effect type/color/strength).
  - `Content/Data/Bosses.csv` now includes `Boss_01` … `Boss_66` (scaled HP/speeds/fire rate/projectile damage).
- Boss fallback now scales even without DT reimport:
  - `Source/T66/Gameplay/T66GameMode.cpp`: fallback `StageData.BossID` is now `Boss_%02d` and fallback `FBossData` scales by stage + uses stage-varying colors.
- Unique enemy is now stage-variant and guaranteed to appear at least once per stage:
  - `Source/T66/Gameplay/T66EnemyDirector.*`: guarantee 1 unique enemy spawn per stage (then additional spawns use the existing chance).
  - `Source/T66/Gameplay/T66UniqueDebuffEnemy.cpp`: per-stage visuals (cube/sphere/cylinder/cone + color) and per-stage stat scaling.
- Idol altar after boss clears on stages ending with 5/0:
  - `Source/T66/Gameplay/T66GameMode.*`: after boss defeat, if `Stage % 5 == 0` spawn an `AT66IdolAltar` near the boss death in addition to the stage gate.
- Idol altar overlay updated to support repeated use across stages:
  - `Source/T66/UI/T66IdolAltarOverlayWidget.cpp`: now equips into the **first empty idol slot** (0..2) and only locks if **all slots are full**.
  - `Source/T66/Core/T66LocalizationSubsystem.cpp`: updated the “Stage 1 already selected” message to “All idol slots are full.” (repurposed text).

**Verification / proof**
- Built successfully (UE 5.7):
  - `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66Editor Win64 Development -Project="C:\UE\T66\T66.uproject" -WaitMutex -FromMsBuild -architecture=x64` ✅
  - `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66 Win64 Development -Project="C:\UE\T66\T66.uproject" -WaitMutex -FromMsBuild -architecture=x64` ✅

### 2026-01-31 — Coliseum v2 (in-map): no ColiseumLevel asset required; all owed bosses spawn at once; exit gate appears when all are dead (event-driven)

**User request**
- Remove dependency on `/Game/Maps/ColiseumLevel`; Coliseum should live inside `GameplayLevel` off to the side (hidden by walls).
- If bosses were skipped before checkpoint stages (5/10/15/…), route to Coliseum before continuing.
- **All owed bosses should spawn at the same time**.
- After all are dead, spawn the stage gate.
- Do this without any Tick polling for performance.

**What changed**
- Coliseum routing no longer attempts to load `/Game/Maps/ColiseumLevel`:
  - `Source/T66/Gameplay/T66StageGate.cpp` and `Source/T66/Gameplay/T66CowardiceGate.cpp` now set `bForceColiseumMode=true` and reload `/Game/Maps/GameplayLevel`.
- Coliseum arena is spawned inside `GameplayLevel`:
  - `Source/T66/Gameplay/T66GameMode.cpp`: `SpawnColiseumArenaIfNeeded()` spawns a dedicated coliseum floor + enclosing walls at an offset location.
  - Player spawn location in forced coliseum mode uses that arena.
- Coliseum boss spawning is now “all at once”:
  - `AT66GameMode::SpawnAllOwedBossesInColiseum()` spawns every owed boss in a ring around the coliseum center and tracks `ColiseumBossesRemaining`.
  - `AT66GameMode::HandleBossDefeatedAtLocation()` decrements the counter on each boss death and spawns the stage gate only when the count hits 0 (event-driven).
- RunState gained `ClearOwedBosses()` for clean completion.
- Setup scripts no longer treat ColiseumLevel as required:
  - `Scripts/CreateAssets.py` no longer auto-creates `/Game/Maps/ColiseumLevel`
  - `Scripts/FullSetup.py` and `Source/T66Editor/T66UISetupSubsystem.cpp` treat ColiseumLevel as optional and do not fail setup when it’s missing.
  - `Source/T66Editor/T66UISetupSubsystem.cpp` logs missing optional widget blueprints (HeroGrid/CompanionGrid/etc) as `[SKIP] Optional` instead of warning spam.

**Verification / proof**
- Built successfully (UE 5.7):
  - `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66Editor Win64 Development -Project="C:\UE\T66\T66.uproject" -WaitMutex -FromMsBuild -architecture=x64` ✅
  - `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66 Win64 Development -Project="C:\UE\T66\T66.uproject" -WaitMutex -FromMsBuild -architecture=x64` ✅

### 2026-01-31 — Vendor shop redesign (4 cards + Buy/Steal + inventory sell) + Gambler design polish

**User request**
- Vendor overlay should show **4 items** like the “Level up” card layout (name + description), with **Buy** and **Steal** under each item.
- **Steal** should trigger the existing quick-time event flow.
- Bottom should show **all owned items**; clicking an owned item opens a **Sell** option showing **sell value**; clicking Sell sells it.
- Keep existing Vendor mechanics: **anger meter**, **borrow/payback**.
- Restyle the Gambler overlay to match the project’s UI design decisions (`FT66Style`).

**What changed (C++ only; no Content/.uasset edits)**
- Vendor inventory selling:
  - `Source/T66/Core/T66RunStateSubsystem.h/.cpp`
    - Added `bool SellInventoryItemAt(int32 InventoryIndex)` so UI can sell a specific owned item (not just “sell first”).
- Vendor overlay UI layout + styling:
  - `Source/T66/UI/T66VendorOverlayWidget.h/.cpp`
    - Rebuilt layout to a **4-card grid** (name, wrapped description, price, Buy/Steal buttons).
    - Added a **bottom inventory strip** (5 slots) + **sell details panel** for selected item.
    - Hooked UI updates to run-state delegates (event-driven): `GoldChanged`, `DebtChanged`, `InventoryChanged`, `VendorChanged`.
    - Kept existing steal timing prompt + boss trigger behavior.
- Gambler overlay design polish:
  - `Source/T66/UI/T66GamblerOverlayWidget.cpp`
    - Applied `FT66Style` tokens/styles to top bar, anger meter, casino panels, cheat prompt, and minigame buttons.
    - Gameplay logic unchanged.

**Notes / tech debt**
- Vendor overlay currently has **some hardcoded UI strings** (e.g. “VENDOR”, “SHOP”, “UPGRADE”, “STEAL”, “YOUR ITEMS”, and a few price labels). Per guardrails, these should be moved into `UT66LocalizationSubsystem::GetText_*()` later.

**Build/verification**
- `T66Editor` build succeeded after changes.
- During the build we hit an unrelated compile break in achievements due to missing `FT66AchievementState` visibility; fixed by including `Core/T66ProfileSaveGame.h` in `Source/T66/Core/T66AchievementsSubsystem.h`.

### 2026-01-31 — Ground visual experiment (reverted)

**User request**
- Attempted a “grass-like” ground look with no imports (roughness/pebbles).

**Outcome**
- Implemented a no-import grass/pebble dressing pass in `AT66GameMode` and then **fully reverted it** due to poor visuals and suspected performance impact.
- Current ground remains the original simple dev floor setup (no extra instancing/dressing).

### 2026-01-30 — SourceAssets model import pipeline (IN PROGRESS; handoff-ready)

**User request**
- User provided zipped character packs (heroes, companions, vendor/gambler/npcs, enemies, bosses), sometimes with animations.
- Vendor/Gambler “boss mode” should reuse the same model as normal mode (no mesh swap needed).

**What is done**
- **Zips are stored here:** `C:\UE\T66\SourceAssets\`
- **Extraction completed** into: `C:\UE\T66\SourceAssets\Extracted\`
  - Created helper script: `Scripts/ExtractSourceAssets.ps1`
  - Output packs include: `Hero1..Hero9`, `Companion1..Companion3`, `Vendor`, `Gambler`, `SaintNPC`, `RegularEnemy`, `GoblinThief`, `Leprachaun`, `StageBoss`
  - Confirmed ~31 `.fbx` across packs (many “Meshy_AI_*” exports); some packs include “Animation_Walking_withSkin.fbx” etc.
- Created import script (not yet executed): `Scripts/ImportSourceAssetsModels.py`
  - Intended to import into `/Game/Characters/<Category>/<PackName>/`
  - Heuristics:
    - Prefer `*Character_output.fbx` for the mesh
    - Import additional FBX that contain `animation`/`walking`/`running` as animations and bind to the skeleton created by the primary mesh import

**What is NOT done yet (next agent work)**
1) **Run the import** (creates actual UE assets under `/Game/Characters/...`):
   - Command:
     - `UnrealEditor-Cmd.exe "C:\UE\T66\T66.uproject" -run=pythonscript -script="C:\UE\T66\Scripts\ImportSourceAssetsModels.py"`
   - Then verify imported assets exist (SkeletalMesh, Skeleton, AnimSequence, Materials/Textures).
2) **Create runtime binding** so actual actors use the imported meshes:
   - **Heroes:** In `AT66HeroBase` (or Hero DataTable), add a mapping from `HeroID` → `TSoftObjectPtr<USkeletalMesh>` (and optionally an Idle `UAnimationAsset`) and set it on a `USkeletalMeshComponent` (hide placeholder mesh).
   - **Enemies/Bosses/NPCs/Companions:** same approach (DataTable fields or class defaults), but keep `AT66VendorBoss` and `AT66GamblerBoss` using the same mesh as their NPC versions.
   - **Animations:** If an Idle animation exists, play it (or use the first imported anim). For best results, prefer an AnimBP later; for now a single looping AnimSequence is fine.
3) **Grounding / scaling pass**:
   - Imported meshes often have different pivots/scales; add per-character offsets/scales (data-driven) so they sit on ground and look correct in both gameplay and preview stages.
4) **Build + smoke test**
   - `Build.bat T66Editor ...` and run PIE to verify meshes/animations load and no sync-load spam or missing asset logs.

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

### 2026-01-30 — World Interactables v0 (Tree/Truck/Wheel/Totem) + tiered Hearts/Skulls UI

- **Goal:** Add core world interactables with a shared 4-rarity system and simple, readable placeholder 3D shapes. Support “5-slot compression” color tiers for Hearts and Difficulty (Skulls) when values exceed 5.
- **Commit:** *(not yet committed)*

**Shared rarity + tier utilities**
- Added `ET66Rarity` + helper functions in:
  - `Source/T66/Core/T66Rarity.h`
  - `Source/T66/Core/T66Rarity.cpp`
- **Default rarity probabilities** (tunable later): Black 70%, Red 20%, Yellow 9%, White 1%.
- **Tier color order** (for 5-slot compression): **Red → Blue → Green → Purple → Gold → Cyan(5+)**.

**Run state**
- `UT66RunStateSubsystem`:
  - Added `DifficultySkulls` (float) + `GetDifficultySkulls()` + `AddDifficultySkulls(float)`
  - `DifficultyTier` is now a cached int = `floor(DifficultySkulls)` for existing enemy scaling.
  - Added `AddMaxHearts(int32)` (increases MaxHearts and also grants the new hearts immediately).
  - `ResetForNewRun()` now resets `MaxHearts` back to 5 and `DifficultySkulls` back to 0.

**HUD**
- `UT66GameplayHUDWidget` now renders:
  - **Hearts** using the tier-color + 5-slot compression based on current hearts.
  - **Difficulty** using tier-color + 5-slot compression based on `DifficultySkulls`, with half-skulls shown as a half-bright square.

**World Interactables**
- Added base class: `AT66WorldInteractableBase` (`Source/T66/Gameplay/T66WorldInteractableBase.*`)
  - Has `TriggerBox`, `VisualMesh`, `ET66Rarity Rarity`, and `bConsumed`.
- Added interactables:
  - `AT66TreeOfLifeInteractable` — cylinder trunk + sphere crown; **adds MaxHearts** by rarity:
    - Black +1, Red +3, Yellow +5, White +10
  - `AT66CashTruckInteractable` — cube body + 4 wheels; **either grants gold** (Black 50, Red 150, Yellow 300, White 600) or **spawns a mimic** (20% chance) and is consumed.
  - `AT66WheelSpinInteractable` — wheel on a sphere; opens a non-pausing popup and awards rarity-scaled gold.
  - `AT66DifficultyTotem` upgraded to inherit `AT66WorldInteractableBase` and now adds skulls by rarity (Bible locked):
    - Black +0.5, Red +1, Yellow +3, White +5
- Mimic enemy:
  - `AT66CashTruckMimicEnemy` — spawns on truck mimic interact, chases the player for **5 seconds** (`InitialLifeSpan=5`), deals heavy touch damage, drops nothing.

**Spawning**
- `AT66GameMode::SpawnWorldInteractablesForStage()` spawns **1 Tree, 1 Truck, 1 Wheel, 1 Totem** per normal stage:
  - Placed in the **Main map square** around \(X=0, Y=0\) within ±4200 uu.
  - Avoids NPC safe bubbles + avoids clustering with other interactables.

**Verification**
- Built successfully with UE 5.7:
  - `Build.bat T66Editor Win64 Development "C:\UE\T66\T66.uproject" -waitmutex` ✅
  - `Build.bat T66 Win64 Development "C:\UE\T66\T66.uproject" -waitmutex` ✅

### 2026-01-30 — Bigger main map, 3x interactables, boundary walls, totem growth

- **Goal:** Make the **Main Area** larger (Start/Boss unchanged), spawn **3 of each** world interactable, add **boundary walls** to prevent falling off the map, and make difficulty totems **one-use but visually grow taller** by activation order.
- **Commit:** *(not yet committed)*

**Map**
- `AT66GameMode::SpawnFloorIfNeeded()`:
  - Main floor scale increased **100×100 → 140×140** (Main is now 14,000×14,000).
  - Start/Boss floors remain **60×60** (6,000×6,000).
- Corner houses moved outward to match bigger main: `Corner = 6000` (was 4200).
- Added `AT66GameMode::SpawnBoundaryWallsIfNeeded()` called from `EnsureLevelSetup()`:
  - Spawns 4 collision walls around the whole playable footprint (covers Start+Main+Boss).

**World Interactables**
- `AT66GameMode::SpawnWorldInteractablesForStage()` now spawns **3 Trees, 3 Trucks, 3 Wheels, 3 Totems**.
- Placement bounds updated to match larger main area (±6200) and spacing increased.

**Totems**
- `UT66RunStateSubsystem` now tracks `TotemsActivatedCount` + `RegisterTotemActivated()`.
- `AT66DifficultyTotem` no longer destroys on interact:
  - it becomes non-interactable (collision disabled) and grows taller:
    - activation #1: base height
    - activation #2: taller
    - activation #3: taller again (etc.)

**Miasma**
- `AT66MiasmaManager::CoverageHalfExtent` increased **4500 → 6500** to better cover the larger main area.

**Verification**
- Built successfully with UE 5.7:
  - `Build.bat T66Editor Win64 Development "C:\UE\T66\T66.uproject" -waitmutex` ✅
  - `Build.bat T66 Win64 Development "C:\UE\T66\T66.uproject" -waitmutex` ✅

### 2026-01-30 — Items v1: loot bag rarities, item effects, non-blocking loot prompt, pause modal fix

- **Goal:** Make items actually affect gameplay, add rarity-based loot bags (Black/Red/Yellow/White), ensure loot UI does **not** pause/blackout the screen, and fix the “game freezes after rebinding + leaving pause menu” issue.
- **Commit:** *(pending)*

**Items data + effects framework**
- `FItemData` expanded in `Source/T66/Data/T66DataTypes.h`:
  - Added `ET66ItemRarity`, `ET66ItemEffectType`
  - Added `ItemRarity`, `BuyValueGold`, `PowerGivenPercent`, `EffectType`, `EffectMagnitude`, `EffectLine1-3`
- `Content/Data/Items.csv` updated to include new columns and now includes a **White** item (`Item_04`) so White loot bags can drop something.
- `Scripts/RunItemsSetup.bat` / `Scripts/SetupItemsDataTable.py` should be run after editing `Items.csv` to re-import `DT_Items`.

**RunState: derived tuning from inventory**
- `UT66RunStateSubsystem` now computes derived multipliers from inventory:
  - `GetItemDamageMultiplier()`, `GetItemAttackSpeedMultiplier()`, `GetDashCooldownMultiplier()`, `GetItemScaleMultiplier()`
  - Recomputed on inventory changes (event-driven, no tick).
- Inventory is capped at **5** slots to match HUD (guards against unbounded growth).

**Combat hooks**
- `UT66CombatComponent` reads RunState multipliers and applies:
  - auto-attack **damage**
  - auto-attack **fire interval** (attack speed)
  - projectile **scale**
- `AT66HeroBase::DashForward()` applies RunState dash cooldown multiplier.
- `AT66HeroProjectile` supports a scale multiplier and bosses now take the projectile’s `Damage` value (not a hardcoded 20).

**Loot bags**
- Enemy deaths spawn `AT66LootBagPickup` (instead of direct pickup grants).
- Loot bags have a fixed rarity: **Black / Red / Yellow / White** (`ET66Rarity`) and are colored in-world by rarity tint.
- Item selection matches the bag rarity:
  - `UT66GameInstance::GetRandomItemIDForLootRarity(ET66Rarity)` selects from cached item pools filtered by `FItemData.ItemRarity`.

**Loot UI: no blackout, keep moving**
- Removed the blocking/full-screen “loot popup” flow.
- Loot is shown as a **top-of-HUD prompt** while in proximity (non-blocking):
  - **Accept:** `F` (Interact)
  - **Reject:** `Right Mouse Button` (only rejects if a loot bag is nearby; otherwise it performs attack unlock as usual)
- HUD prompt is built into `UT66GameplayHUDWidget` and reads `AT66PlayerController::GetNearbyLootBag()`.

**Pause menu / settings freeze fix**
- UIManager uses a single active modal. Opening `Settings` or `Report Bug` from Pause replaces the Pause Menu modal.
- Closing `Settings`/`Report Bug` while paused now reopens the Pause Menu automatically:
  - `UT66SettingsScreen::OnCloseClicked()`
  - `UT66ReportBugScreen::OnCancelClicked()` / `OnSubmitClicked()`

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

### 2026-02-02 — Luck Rating wired to central RNG + Run Summary ratings panel (Skill Rating rename + cheat-prob placeholder)

**Goal**
- Remove the “Luck Rating is a placeholder” state and make Luck Rating a real post-run metric driven by the centralized RNG work.
- Update Run Summary layout to match the new direction:
  - Inventory moved to the **bottom half** next to Idols.
  - Add a ratings column to the right of Base Stats: **Luck Rating**, **Skill Rating**, **Probability of Cheating**.
- Canonical rename: **Dodge Rating → Skill Rating** (calculation TBD; separate agent will implement).

**What shipped (committed)**
- Commit: `11bc868` — `Gameplay: central RNG + luck-biased outcomes`
  - Added `UT66RngSubsystem` + config-backed tuning (`UT66RngTuningConfig`) and migrated luck-affected events to use it.
  - Fixed Run Summary first-open snapshot rendering and Event Log toggle refresh.

**What is currently implemented (may be uncommitted depending on repo state)**
- **Luck Rating system (authoritative, per-run)** — `UT66RunStateSubsystem`
  - Added an internal per-run tracker for Luck Rating:
    - Tracks **Quantity** outcomes (normalized within \[Min..Max\] or bool 0/1).
    - Tracks **Quality** outcomes (rarity mapped linearly: Black=0, Red=1/3, Yellow=2/3, White=1).
    - Aggregation uses **“average of category averages”** so huge volumes (eg many kills) don’t dominate the score.
  - Public getters:
    - `GetLuckRating0To100()`
    - `GetLuckRatingQuantity0To100()`
    - `GetLuckRatingQuality0To100()`
  - Tracker resets on `ResetForNewRun()`.

- **Luck Rating feed points (recorded events)**
  - **Enemy loot bags** — `AT66EnemyBase::OnDeath`
    - Quantity: `EnemyLootBagDrop` (true/false).
    - Quality: `EnemyLootBagRarity` (bag rarity).
  - **World interactables** — `AT66GameMode::SpawnWorldInteractablesForStage`
    - Quantity: `TreesPerStage`, `TrucksPerStage`, `WheelsPerStage`.
    - Quality: `TreeRarity`, `TruckRarity`, `WheelRarity`.
    - Note: location is intentionally not luck-affected (per spec).
  - **Special enemies** — `AT66EnemyDirector::SpawnWave`
    - Quantity: `GoblinCountPerWave`, `LeprechaunCountPerWave` (including 0).
    - Quality: `GoblinRarity`, `LeprechaunRarity`.
  - **Wheel payouts**
    - Quantity: `WheelGold` recorded for both HUD wheel (`UT66GameplayHUDWidget`) and wheel overlay (`UT66WheelOverlayWidget`), normalized within the configured rarity payout range.
  - **Vendor steal**
    - Quantity: `VendorStealSuccess` (success = item granted + **no anger increase**; failure = anger increases).
  - **Gambler cheat**
    - Quantity: `GamblerCheatSuccess` (success = forced win + **no anger increase**; failure = anger increases and normal outcome RNG applies).
  - **Level-up gains**
    - Quantity: `LevelUp_*Gain` per stat gain roll (ranges unchanged; Luck biases high-end outcomes).

- **Leaderboard snapshot schema update**
  - `UT66LeaderboardRunSummarySaveGame` bumped **SchemaVersion to 2** and stores:
    - `LuckRating0To100`
    - `LuckRatingQuantity0To100`
    - `LuckRatingQuality0To100`
  - `UT66LeaderboardSubsystem::SaveLocalBestBountyRunSummarySnapshot` now writes these fields from `UT66RunStateSubsystem`.
  - Run Summary shows **N/A** when opening older snapshots with SchemaVersion < 2.

- **Run Summary UI layout + ratings column**
  - File: `Source/T66/UI/Screens/T66RunSummaryScreen.cpp`
  - Layout now:
    - **Top row**: Hero preview | Base Stats | Ratings column
    - **Bottom row**: Idols | Inventory (side-by-side)
  - Ratings column:
    - **LUCK RATING** is now wired to real data (`RunState->GetLuckRating0To100()` or snapshot).
    - **SKILL RATING** is the canonical rename from Dodge Rating — still a placeholder until implemented.
    - **INTEGRITY** box contains placeholder text: `Probability of Cheating: 0%` (wiring TBD).

**Skill Rating rename (coordination note)**
- Canonical terminology: **Skill Rating** replaces “Dodge Rating” everywhere going forward.
- Another agent will own the Skill Rating system implementation. This branch should not attempt to define Skill Rating math until that work lands.

**Localization note (must not be skipped)**
- New player-facing labels were added for ratings panels and integrity text.
- These must go through the full localization pipeline (GatherText → translate all 22 cultures → compile `.locres` → verify).

**Build / verification**
- `T66` target builds successfully after Luck Rating wiring.
- `T66Editor` may fail to rebuild if hot-reload DLLs are locked by a running editor instance (file lock / hot-reload artifact).

**Local data reset (developer action, not committed)**
- To reset the local “YOU” high score so the next run becomes best:
  - Deleted:
    - `Saved/SaveGames/T66_LocalLeaderboard.sav`
    - `Saved/SaveGames/T66_LocalBestBountyRunSummary_Easy_Solo.sav`

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

### Model pipeline (handoff)

1. Run `Scripts/ImportSourceAssetsModels.py` via `UnrealEditor-Cmd.exe` to import meshes + animations.
2. Add data-driven mesh binding (HeroID/NPCID/BossID/etc → SkeletalMesh + optional Idle anim).
3. Apply grounding/scaling tweaks (data-driven offsets).
4. Build + verify.

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

- **Languages (UI selector list):**
  - English
  - Simplified Chinese
  - Traditional Chinese
  - Japanese
  - Korean
  - Russian
  - Polish
  - German
  - French
  - Spanish (Spain)
  - Spanish (Latin America)
  - Portuguese (Brazil)
  - Portuguese (Portugal)
  - Italian
  - Turkish
  - Ukrainian
  - Czech
  - Hungarian
  - Thai
  - Vietnamese
  - Indonesian
  - Arabic
- **Subsystem:** `UT66LocalizationSubsystem`
- **Usage:** `GetText_*()` for all UI strings; screens subscribe to OnLanguageChanged where needed.

---

### 2026-01-31 — Hero foundational stats v1 + Companion Union progression (healing tiers) + localized UI

**Hero stats (foundational)**
- Replaced v0 “each stat == HeroLevel” behavior with:
  - 6 foundational stats: **Damage, Attack Speed, Attack Size, Armor, Evasion, Luck**
  - **Speed** remains foundational but is treated specially: **+1 Speed per level**
- Added per-hero base stats (1–5) and per-level random gain ranges (hero-archetype driven):
  - Central tuning: `UT66GameInstance::GetHeroStatTuning(HeroID, BaseStats, PerLevelGains)`
  - Runtime storage + derived multipliers: `UT66RunStateSubsystem` (stats are no longer just `HeroLevel`)

**UI**
- Gameplay HUD stat panel: **Speed removed from the displayed stat lines** (still affects movement).
- Hero Selection: hero description panel now appends a localized **Base Stats** section showing each hero’s base stats.
- Companion Selection: added a localized **Union** bar with labeled checkpoints: **Basic Healing, Good Healing, Medium Healing, Hyper Healing**.

**Companion Union (gameplay + persistence)**
- Union increases by clearing stages with a companion:
  - Implemented on `AT66StageGate::AdvanceToNextStage` (normal stage clears).
- Union boosts healing received (tuned):
  - Union now controls **companion heal interval** (1 heart per interval):
    - **Basic**: 10s
    - **Good**: 5s
    - **Medium**: 3s
    - **Hyper**: 1s
- Persistent storage:
  - `UT66ProfileSaveGame` now stores `CompanionUnionStagesClearedByID` (CompanionID → stages cleared).

**Verification / proof**
- **ValidateFast (UE 5.7)** ✅
  - `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66Editor Win64 Development "C:\UE\T66\T66.uproject" -WaitMutex -FromMsBuild -architecture=x64`
  - `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66 Win64 Development "C:\UE\T66\T66.uproject" -WaitMutex -FromMsBuild -architecture=x64`
- **GatherText (source)** ✅
  - `& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "C:\UE\T66\T66.uproject" -run=GatherText -config="C:\UE\T66\Config\Localization\T66_Gather_Source.ini" -unattended -nop4 -nosplash -nullrhi -log="C:\UE\T66\Saved\Logs\T66_Gather_Source.log"`
- **Translate archives (22 cultures)** ✅
  - `python -u "C:\UE\T66\Scripts\AutoTranslateLocalizationArchives.py"`
- **Compile `.locres`** ✅
  - `& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "C:\UE\T66\T66.uproject" -run=GatherText -config="C:\UE\T66\Config\Localization\T66_Compile.ini" -unattended -nop4 -nosplash -nullrhi -log="C:\UE\T66\Saved\Logs\T66_Compile.log"`
- **Artifact check** ✅
  - `(Get-ChildItem -Path "C:\UE\T66\Content\Localization\T66" -Recurse -Filter *.locres).Count` → **22**

**Follow-up tuning (same change-set)**
- Hero Selection description text made larger.
- Removed **Speed** from:
  - hero selection base-stats text
  - in-run stat panel
- In-run stat panel now shows **raw stat numbers** (not \(x\) multipliers / %).
- Items now apply a single **main stat line** (one of the 6 foundational stats; no Speed):
  - Implemented as flat stat points in `UT66RunStateSubsystem` (tooltip shows `{Stat}: +{N}`).
  - Vendor UI now shows the same single stat line (shop + sell panels).
- Companion healing nerf:
  - Removed any “multiply healing amount” behavior; healing is rate-based via Union tier intervals.

**Hero Selection UI refresh (2026-01-31)**
- Layout:
  - Removed **THE LAB** button from this screen.
  - Moved **Difficulty** selector + **ENTER THE TRIBULATION** under the **left (Skins) panel**.
  - Moved **Body Type (A/B)** buttons directly under the character platform; moved **CHOOSE COMPANION** under body type.
  - Video preview placeholder made taller (square-ish).
- Hero carousel:
  - Fixed portrait-swatch carousel so the colored squares update as you cycle heroes.
- Lore:
  - Lore now opens as a same-size panel on the right side (no modal).
  - Existing localized hero description text now lives in the Lore panel.
  - Hero info panel now shows a short per-hero **quote** (new localized strings).
- Preview stage:
  - Dragging the preview rotates the hero (no camera orbit) so the platform stays visually stable.

**Verification / proof (rerun after these edits)**
- ValidateFast ✅ (same commands as above)
- GatherText → Translate → Compile ✅
- `.locres` count: **22**

---

## Hero Selection UI polish (2026-01-31)

- **Side panels height**: Left (Skins) and Right (Hero Info) panels are now slightly shorter via bottom padding so they end above the bottom control row.
- **AC balance placement/style**: AC balance moved to the **top-right** of the Skins panel header and styled to match the Achievements page (Panel2 chip + gold text, Bold 22).
- **Skins readability**: Skin name/price font sizes increased for better use of space.
- **Default skin price**: `Default` skin now costs **0 AC** (other skins remain 250 AC in the hero screen placeholder data).
- **Controls**:
  - **HERO GRID** button moved to the **left of** the left carousel arrow.
  - **CHOOSE COMPANION** moved to the **right of TYPE B** (same row).

---

## Preview zoom + Vendor/Gambler UI refresh (2026-01-31)

- **3D preview zoom (Hero + Companion selection)**:
  - Mouse wheel now **zooms in** on the preview (default framing is the **max zoom-out**).
  - Implemented via new `PreviewZoomMultiplier` applied on top of `CameraDistanceMultiplier`:
    - `AT66HeroPreviewStage::AddPreviewZoom()`
    - `AT66CompanionPreviewStage::AddPreviewZoom()`
  - Wired mouse wheel in:
    - `ST66DragRotatePreview` (hero selection)
    - `ST66DragRotateCompanionPreview` (companion selection)

- **Vendor UI cleanup + layout**
  - Shop item cards no longer show **UPGRADE** or **PRICE:** lines (price stays on buy button).
  - **Gold / Owe** moved next to **YOUR ITEMS** header (bottom).
  - **Anger** bar replaced by a **color-changing circle** (white → red) while keeping the same anger logic.
  - **Bank** section moved to the **bottom** of the right panel.
  - Vendor now opens with a **dialogue prompt** (Shop vs Teleport to brother) like the gambler.

- **Gambler UI layout (vendor-like)**
  - Casino hub now shows **3 horizontal game cards** (Coin Flip / RPS / Find the Ball).
  - Selecting a game swaps the center area to that game (cards disappear) and shows **Bet** controls below.
  - Added **inventory strip + sell panel** (same placement/behavior as vendor).
  - Replaced anger fill bar with the same **anger circle** and moved bank into the **right panel**.

- **Theme assets**
  - Added `T66.Brush.Circle` to `FT66Style` (rounded-box brush with large radius) for the anger indicator.

- **Localization**
  - New vendor dialogue strings added:
    - `T66.Vendor.IWantToShop`
    - `T66.Vendor.VendorDialoguePrompt`
  - GatherText → AutoTranslate → Compile rerun; `.locres` count remains **22**.

### 2026-01-31 — Vendor/Gambler overlays: Inventory header + fixed slot sizing + white circle above Bank

- **Inventory header text**
  - Both Vendor and Gambler overlays now show **INVENTORY** (localized via `UT66LocalizationSubsystem::GetText_YourItems()`).
  - Fallback text when `Loc == nullptr` is also **INVENTORY** (so it can’t regress to “YOUR ITEMS”).
  - **Gold / Owe** sits next to the Inventory header in both overlays.
- **Inventory slot sizing**
  - Slots are consistently large (no “small then big” behavior on selection); selection only tints.
- **Bank marker**
  - Added a filled **white circle** above the **BANK** section in both overlays (uses `T66.Brush.Circle`).
- **Gambler label cleanup**
  - Removed the “GAMES” label from the gambler casino panel.
- **Verification**
  - `T66Editor` build succeeded (UE 5.7).

### 2026-02-01 — Stage flow hardening + special arenas + High Score rename + HUD/Run Summary refresh

**Goal**
- Fix “Stage 2 trapped spawn” by ensuring **normal stages always reuse `GameplayLevel`** and stage-transition flags don’t leak.
- Officialize the three special arenas (Boost / Coliseum / Tutorial) as **contained areas inside `GameplayLevel`**.
- Rename “Bounty” → **High Score** (player-facing + internal identifiers) while keeping **SaveGame backward compatibility**.
- Apply requested HUD/menus/run-summary UX tweaks and keep UI **event-driven**.

**What changed**
- **Stage transitions / map reuse**
  - `Source/T66/Gameplay/T66StageGate.cpp`:
    - Normal stages now always `OpenLevel("/Game/Maps/GameplayLevel")` (no more re-opening the current level name).
    - Defensive: clear `bStageBoostPending` during normal stage advance so Boost cannot “leak” into later stages.
  - `Source/T66/Gameplay/T66GameMode.cpp`:
    - Tutorial spawn is restricted so we **don’t spawn into the enclosed tutorial arena on later stages** (prevents trap cases).

- **Special stages (Boost / Coliseum / Tutorial) as enclosed arenas**
  - `Source/T66/Gameplay/T66GameMode.cpp`:
    - Special arena row at **Y=+7500**:
      - Boost \((-5000, 7500)\), Coliseum \((0, 7500)\), Tutorial \((5000, 7500)\)
    - Ensure level setup now spawns/maintains these arenas and their walls as needed.
    - Added no-spawn filters so random gameplay spawns don’t pollute these enclosed arenas.

- **Start Area no-spawn rule**
  - `Source/T66/Gameplay/T66GameMode.cpp`:
    - Added an explicit “no-spawn zone” so **no world interactables** and **no stage-effect tiles** can spawn in the **Start Area** (6000×6000 centered at \((-10000, 0)\)).

- **Boss gate corridor walls**
  - `Source/T66/Gameplay/T66GameMode.cpp`:
    - Added boss-area entry corridor walls so the boss area is only reachable through the intended opening near the boss gate pillars.

- **HUD / UI changes**
  - `Source/T66/UI/T66GameplayHUDWidget.cpp`:
    - Wheel spin HUD moved to the **right side**.
    - Swapped difficulty skull row to be **above** the immortality toggle.
    - “SR” label expanded to **“Speed Run”**.
    - Stage label now supports special stage labels: **Boost / Coliseum / Tutorial**.
  - `Source/T66/UI/Screens/T66PauseMenuScreen.cpp`:
    - Un-squished pause menu buttons (larger sizing + consistent spacing).

- **Run Summary improvements**
  - `Source/T66/UI/Screens/T66RunSummaryScreen.cpp`, `Source/T66/UI/Screens/T66RunSummaryScreen.h`:
    - Event log is now a toggleable panel (hidden by default).
    - Displays **High Score** in the “Stage Reached” line.
    - Submits high score to leaderboard when appropriate.

- **Vendor / Gambler fullscreen overlay behavior**
  - `Source/T66/UI/T66VendorOverlayWidget.cpp`, `Source/T66/UI/T66GamblerOverlayWidget.cpp`:
    - “Back” now exits gameplay overlays cleanly (no longer bouncing to dialogue pages).

- **Companion friendliness (“Union”)**
  - `Source/T66/Core/T66AchievementsSubsystem.h`:
    - Union tier thresholds updated to **10 / 20 / 30** stages cleared.
  - `Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp`:
    - Added a Friendliness (Union) progress UI (text + progress bar) using Achievements data.

- **Bounty → High Score (internal + player-facing) with SaveGame back-compat**
  - `Source/T66/Core/T66LeaderboardSubsystem.h/.cpp`:
    - `SubmitRunHighScore(...)` and High Score entry building.
    - Legacy wrappers maintained where needed for back-compat.
    - Run-summary snapshot slot naming supports legacy “Bounty” slot names.
  - `Source/T66/Core/T66LocalLeaderboardSaveGame.h`, `Source/T66/Core/T66LeaderboardRunSummarySaveGame.h`:
    - Save fields renamed to High Score naming.
  - `Config/DefaultEngine.ini`:
    - Added `[CoreRedirects]` property redirects for SaveGame backward compatibility.
  - `Source/T66/Core/T66LocalizationSubsystem.h/.cpp`:
    - Updated localized strings: “Stage: {0}”, “High Score:”, “Stage Reached … | High Score …”.
    - Kept deprecated wrappers for old Blueprint function names.
  - `Source/T66/Gameplay/T66EnemyBase.h`:
    - Comment updated: “Bounty score” → “High Score”.
  - `Source/T66/UI/Components/T66LeaderboardPanel.cpp`:
    - UI now displays “High Score” instead of “Bounty”.

**Localization**
- Completed pipeline for the new/changed player-facing strings:
  - Gather: `Config/Localization/T66_Gather.ini`
  - Translate: `Scripts/AutoTranslateLocalizationArchives.py` (baseline machine translations)
  - Compile: `Config/Localization/T66_Compile.ini`
- Outputs updated under `Content/Localization/T66/` (`.manifest`, `.archive`, `.locres`).

**Verification / proof**
- Builds ✅ (UE 5.7):
  - `T66Editor` Win64 Development
  - `T66` Win64 Development

---

### 2026-02-01 — Difficulty totems: skull-scalar difficulty + score multiplier + companion UI polish

**Goal**
- Replace the old rarity-based / fractional / single-use difficulty totems with the new **infinite-use skull** system.
- Make skulls affect **enemy HP, enemy damage, enemy count, enemy points**, and also **boss HP, boss damage, boss points** (boss count unchanged).
- Remove remaining player-facing “Bounty” and show **High Score + score multiplier** on HUD.
- Remove tutorial-only items to stop log noise and use normal items instead.
- Update companion selection: remove medals, add friendliness checkpoints and match companion skins styling to hero skins.

**What changed**
- **RunState difficulty is now integer skulls + tier scalar**
  - `Source/T66/Core/T66RunStateSubsystem.h/.cpp`:
    - `DifficultySkulls` is now `int32` (no half-skulls).
    - Added tier math + `GetDifficultyScalar()` (Tier 0 => 1.0x, Tier 1 => 1.1x, Tier 2 => 1.2x, ...).
- **Difficulty totems are infinite-use + global growth**
  - `Source/T66/Gameplay/T66DifficultyTotem.h/.cpp`:
    - Interact always adds **+1 skull**.
    - Totems do **not** consume; you can interact repeatedly.
    - Every interaction updates **all totems** to grow taller together (with a safe clamp).
- **Enemy scaling + enemy count scaling**
  - `Source/T66/Gameplay/T66EnemyBase.h/.cpp`:
    - Added `ApplyDifficultyScalar(float)` (preserves HP percent on mid-fight scaling).
    - Enemy score award is scaled by difficulty scalar at death time.
  - `Source/T66/Gameplay/T66EnemyDirector.h/.cpp`:
    - Scales `EnemiesPerWave` and `MaxAliveEnemies` from cached base counts using the difficulty scalar (with caps).
  - `Source/T66/Gameplay/T66GameMode.cpp`:
    - `HandleDifficultyChanged()` now applies scalar scaling to all active enemies.
- **Boss scaling + boss score**
  - `Source/T66/Data/T66DataTypes.h`: added `FBossData::PointValue` (optional; defaulted conservatively in code when missing).
  - `Source/T66/Core/T66RunStateSubsystem.h/.cpp`: added `RescaleBossMaxHPPreservePercent()` for boss bar scaling without healing to full.
  - `Source/T66/Gameplay/T66BossBase.h/.cpp`:
    - Added boss `PointValue` and `ApplyDifficultyScalar(float)` (HP + projectile damage; preserves boss HP percent via RunState).
  - `Source/T66/Gameplay/T66GameMode.h/.cpp`:
    - Boss death handling now receives the boss pointer, awards scaled boss points, and continues existing gate/coliseum logic.
    - `HandleDifficultyChanged()` also scales bosses.
- **HUD / UI**
  - `Source/T66/UI/T66GameplayHUDWidget.h/.cpp`:
    - Difficulty skull HUD no longer supports half-skulls.
    - Top-left score label now shows **High Score** and displays a **score multiplier** (e.g. `x1.2`).
  - `Source/T66/Core/T66LocalizationSubsystem.h/.cpp`:
    - Updated player-facing strings to say **High Score** instead of **Bounty** (legacy function names retained where needed).
  - `Source/T66/UI/Components/T66LeaderboardPanel.cpp`:
    - Leaderboard type dropdown now displays **High Score** instead of **Bounty**.
  - `Source/T66/UI/Screens/T66RunSummaryScreen.cpp`:
    - Run summary line now displays **High Score** in fallback formatting.
- **Tutorial items removed**
  - `Source/T66/Gameplay/T66TutorialManager.cpp`:
    - Tutorial now selects a real `Item_0X` item (tries to match the hero’s strongest stat by scanning known items).
  - `Source/T66/Core/T66GameInstance.cpp`:
    - Removed synthetic `Item_Tutorial_*` item generation.
- **Companion selection**
  - `Source/T66/UI/Screens/T66CompanionSelectionScreen.h/.cpp`:
    - Removed medal UI.
    - Added Friendliness (Union) UI with checkpoint lines (5/10/20), stages text, and healing type (1–4).
    - Rebuilt companion skins list to match hero skins styling (FT66Style + AC balance header).

**Localization**
- Completed the required end-to-end pipeline for new/changed player-facing strings:
  - Gather: `Config/Localization/T66_Gather_Source.ini`
  - Translate baseline: `Scripts/AutoTranslateLocalizationArchives.py`
  - Compile: `Config/Localization/T66_Compile.ini`

**Verification / proof**
- ValidateFast builds ✅ (UE 5.7):
  - `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66Editor Win64 Development "C:\UE\T66\T66.uproject" -WaitMutex -FromMsBuild -architecture=x64`
  - `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66 Win64 Development "C:\UE\T66\T66.uproject" -WaitMutex -FromMsBuild -architecture=x64`

---

### 2026-02-02 — Gameplay: ensure blue mid-day sky at runtime (SkyAtmosphere)

**Goal**
- GameplayLevel is largely constructed at runtime (floors/walls/lighting). Ensure the in-game world has a **natural blue mid-day sky** (not just ambient tint), without relying on editor-authored sky actors.

**What changed**
- `Source/T66/Gameplay/T66GameMode.cpp`
  - Extended `SpawnLightingIfNeeded()` to:
    - Spawn `ASkyAtmosphere` when missing (provides the actual sky rendering).
    - Ensure the DirectionalLight drives the sky atmosphere (`bAtmosphereSunLight=true`, `AtmosphereSunLightIndex=0`).
    - Keep SkyLight color neutral (white) and `RecaptureSky()` after atmosphere + sun are configured so ambient lighting picks up the blue sky.

**Localization**
- No new player-facing runtime strings.

**Verification / proof**
- Runtime visual check ✅: user confirmed GameplayLevel now shows a blue sky in-game.
- ValidateFast builds ✅ (UE 5.7):
  - `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66Editor Win64 Development "C:\UE\T66\T66.uproject" -WaitMutex -FromMsBuild -architecture=x64`
  - `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66 Win64 Development "C:\UE\T66\T66.uproject" -WaitMutex -FromMsBuild -architecture=x64`

**Commit**
- `3ceda6a6370e1eaae3a2a620b3a75631d0f6bb68` — Gameplay: spawn runtime SkyAtmosphere for blue midday sky

---

### 2026-02-01 — Official .1 release: data-driven sprites + world models + UI hookups

**Goal**
- Replace placeholder colored-square sprites and placeholder meshes with real PNG/FBX assets, while keeping a **repeatable system** so assets can be swapped later without rewriting UI/gameplay.

**What changed (high-signal)**
- **Data types (soft references)**
  - `Source/T66/Data/T66DataTypes.h`:
    - `FItemData.Icon` (`TSoftObjectPtr<UTexture2D>`)
    - `FIdolData.Icon` (`TSoftObjectPtr<UTexture2D>`)
    - `FHeroData.Portrait` (`TSoftObjectPtr<UTexture2D>`)
- **CSV-driven content**
  - `Content/Data/Items.csv`: regenerated to match `SourceAssets/Sprites/Items/*` counts (Black/Red/Yellow/White), includes `Icon`, and assigns deterministic rarity-scaled main stats (no Speed).
  - `Content/Data/Idols.csv`: canonical IdolIDs + `Icon` paths.
  - `Content/Data/Heroes.csv`: `Portrait` paths.
  - `Content/Data/Bosses.csv`: added `PointValue` column and set `Boss_01=100` … `Boss_66=6600` (+100 per stage).
- **UI now renders real icons/portraits**
  - Vendor / Gambler / HUD / Run Summary / Hero carousel & grid / Idol altar tiles all use `SImage` + `FSlateBrush` bound to the new soft texture references (with safe fallbacks).
  - Gambler inventory strip now matches vendor behavior (icons instead of `Item_Red_09` style text).
- **World models**
  - Loot bags, gates, wheel/tree/truck interactables, idol altar, difficulty totem: switched to soft mesh overrides with safe fallbacks.
  - Loot bags: fixed “all bags share one material” by importing loot bags into per-color subfolders (avoids `Material_001` / `Image_0` collisions).
  - Grounding: bags and gates snap to ground at runtime; trace uses object query for both `WorldStatic` and `WorldDynamic`.
- **Difficulty totem growth**
  - Totem interaction now **stacks mesh segments** instead of scaling the base mesh.
- **NPC visuals**
  - Trickster/Ouroboros use `DT_CharacterVisuals` (and force `NPCID` in `BeginPlay` to handle already-placed instances).
- **Automation scripts (run in full editor)**
  - `Scripts/ImportSpriteTextures.py`: imports PNGs under `/Game/UI/Sprites/**` with UI texture settings.
  - `Scripts/ImportWorldModels.py`: extracts/imports model zips under `/Game/World/**` and NPC skeletal meshes under `/Game/Characters/NPCs/**`.
  - `Scripts/SetupAllAssetsAndDataTables.py`: one-shot orchestrator (sprites + models + DataTables).

**Notes**
- `Scripts/__pycache__/` should remain untracked (don’t commit).

**Commit**
- `7b2c180` — `official .1 version`

---

### 2026-02-02 — ImportWorldModels: support `SourceAssets/Extracted/Models` + reimport per-color interactables

**What changed**
- `Scripts/ImportWorldModels.py` now scans both `SourceAssets/Models/**` and `SourceAssets/Extracted/Models/**` for `.zip` packs (some repo layouts relocate model zips under `Extracted/Models`).
- Re-ran the importer successfully (unattended editor `-ExecutePythonScript`) and confirmed per-color subfolders exist on disk for:
  - `Content/World/Interactables/Trees/Black|Red|Yellow|White/`
  - `Content/World/Interactables/Trucks/Black|Red|Yellow|White/`
  - `Content/World/Interactables/Wheels/Black|Red|Yellow|White/`

---

### 2026-02-01 — Replace placeholder enemy models: Goblin/Leprechaun + Stage 1/2 mobs/unique/boss

**Goal**
- Replace placeholder visuals for:
  - Goblin Thief (all 4 rarities)
  - Leprechaun (all 4 rarities)
  - Stage 1 mobs + unique + boss
  - Stage 2 mobs + unique + boss
- Constraint: only **walk loop** animations required for now; **Stage 2 Unique** may be **static**.

**What changed**
- **Extracted the new zips to `SourceAssets/Extracted/` with stable pack names**
  - Goblin: `GoblinThief_Black|Red|Yellow|White`
  - Leprechaun: `Leprechaun_Black|Red|Yellow|White`
  - Stage 1: `Mob_Stage01_Mob1`, `Mob_Stage01_Mob2`, `Unique_Stage01`, `Boss_01`
  - Stage 2: `Mob_Stage02_Mob1`, `Mob_Stage02_Mob2`, `Unique_Stage02`, `Boss_02`
- **Importer improvements**
  - `Scripts/ImportSourceAssetsModels.py`:
    - Added optional pack filter: `-T66ImportPacks=PackA,PackB,...` (prevents reimporting everything).
    - Added category routing for the new packs (Enemies/Stages, Bosses/Stages, etc.).
    - Stage 2 Unique imports as **StaticMesh** with stable name:
      - `/Game/Characters/Enemies/Stages/Unique_Stage02/SM_Unique_Stage02`
    - Imports the *mesh FBX without animations* first, then imports explicit animation FBXs (more stable with Interchange).
- **Data-driven stage roster**
  - `Content/Data/Stages.csv`:
    - Stage 1: `EnemyA/B/C` now use `Mob_Stage01_Mob1` and `Mob_Stage01_Mob2` (no more `Mob_EmberImps`, etc.).
    - Stage 2: `EnemyA/B/C` now use `Mob_Stage02_Mob1` and `Mob_Stage02_Mob2`.
- **Character visuals table**
  - `Content/Data/CharacterVisuals.csv`:
    - Boss: `Boss_01` and new `Boss_02` now point at imported meshes + walk loops.
    - Goblin: `GoblinThief` now points at the black variant; added `GoblinThief_Black|Red|Yellow|White`.
    - Leprechaun: `Leprechaun` now points at the black variant; added `Leprechaun_Black|Red|Yellow|White`.
    - Stage mobs: added `Mob_Stage01_Mob1`, `Mob_Stage01_Mob2`, `Mob_Stage02_Mob1`, `Mob_Stage02_Mob2`.
    - Stage 1 unique: added `Unique_Stage01` (skeletal mesh + walk loop).
- **Gameplay wiring**
  - `Source/T66/Gameplay/T66EnemyBase.cpp`:
    - Stage mobs now set `CharacterVisualID = MobID` so `DT_CharacterVisuals` can drive their mesh/animation (safe fallback to placeholders if row missing).
  - `Source/T66/Gameplay/T66GoblinThiefEnemy.cpp`:
    - Uses rarity-specific visual IDs (`GoblinThief_Black|Red|Yellow|White`) and reapplies visuals on `SetRarity()`.
    - Stops tinting the placeholder cone when an imported mesh is in use.
  - `Source/T66/Gameplay/T66LeprechaunEnemy.cpp`:
    - Uses rarity-specific visual IDs (`Leprechaun_Black|Red|Yellow|White`) and reapplies visuals on `SetRarity()`.
    - Stops tinting placeholder parts when an imported mesh is in use; hides the head sphere when using the imported mesh.
  - `Source/T66/Gameplay/T66UniqueDebuffEnemy.cpp`:
    - Stage 1 Unique uses `Unique_Stage01` imported skeletal mesh + walk loop (via `DT_CharacterVisuals`).
    - Stage 2 Unique uses static mesh `/Game/Characters/Enemies/Stages/Unique_Stage02/SM_Unique_Stage02` (no animation).
- **DataTables updated**
  - Refilled `DT_CharacterVisuals` + `DT_Stages` from CSV.

**Localization**
- No new player-facing runtime strings were added/changed.

**Verification / proof**
- Extracted packs (PowerShell) ✅:
  - `Expand-Archive` into `C:\UE\T66\SourceAssets\Extracted\{PackName}\`
- Imported meshes/animations ✅ (full editor, headless):
  - `UnrealEditor.exe "C:\UE\T66\T66.uproject" -nop4 -nosplash -unattended -nullrhi -log="T66_ImportEnemyModels_Editor2.log" "-T66ImportPacks=..." -ExecCmds="py C:/UE/T66/Scripts/ImportSourceAssetsModels.py"`
- Reimported DataTables ✅:
  - `UnrealEditor-Cmd.exe "C:\UE\T66\T66.uproject" -run=pythonscript -script="C:\UE\T66\Scripts\SetupCharacterVisualsDataTable.py" -unattended -nop4 -nosplash -nullrhi -log="T66_SetupCharacterVisuals.log"`
  - `UnrealEditor-Cmd.exe "C:\UE\T66\T66.uproject" -run=pythonscript -script="C:\UE\T66\Scripts\SetupStagesDataTable.py" -unattended -nop4 -nosplash -nullrhi -log="T66_SetupStages.log"`
- ValidateFast builds ✅ (UE 5.7):
  - `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66Editor Win64 Development "C:\UE\T66\T66.uproject" -WaitMutex -FromMsBuild -architecture=x64`
  - `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66 Win64 Development "C:\UE\T66\T66.uproject" -WaitMutex -FromMsBuild -architecture=x64`

---

### 2026-02-01 — Loot bags: replace snap-to-ground with gravity drop

**User report**
- Loot bags were sometimes floating (common when spawned from enemies that die above the floor).

**Goal**
- Remove the explicit “snap to ground” traces and instead make loot bags **fall naturally with gravity**, which also looks correct for flying enemies.

**What changed**
- `Source/T66/Gameplay/T66LootBagPickup.h/.cpp`:
  - Removed the BeginPlay line-trace “snap to ground”.
  - Added a small physics/collision root sphere (`PhysicsRoot`) that blocks `WorldStatic` + `WorldDynamic`.
  - Added a `UProjectileMovementComponent` (`FallMovement`) with `ProjectileGravityScale=1` so loot bags fall without enabling full physics simulation.
  - Kept the large overlap sphere (`SphereComponent`, radius 120) for pickup detection.
  - Updated visuals positioning so the bag mesh sits on the bottom of `PhysicsRoot` (no “hovering” due to collision radius).

**Localization**
- No new player-facing runtime strings.

**Verification / proof**
- ValidateFast builds ✅ (UE 5.7):
  - `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66Editor Win64 Development "C:\UE\T66\T66.uproject" -WaitMutex -FromMsBuild -architecture=x64`
  - `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66 Win64 Development "C:\UE\T66\T66.uproject" -WaitMutex -FromMsBuild -architecture=x64`

---

### 2026-02-01 — Fix invisible non-biped enemies (pivot/bounds offset): auto-center imported meshes

**User report**
- Some imported enemies (notably quadruped-style packs) were “invisible but hittable” (health bar visible, collisions/damage work, but mesh not seen).

**Root cause (most likely)**
- Some FBX exports have a **bounds origin / pivot far from the actual geometry**, so the skeletal mesh is effectively placed far away from the capsule after attachment (appearing invisible).

**What changed**
- `Source/T66/Core/T66CharacterVisualSubsystem.cpp`:
  - `ApplyCharacterVisual(...)` now auto-centers **XY** by subtracting the (rotated) bounds origin when the origin is non-trivial.
  - This is gated by a small threshold so normal meshes (near-zero origin) are unaffected.
- `Source/T66/Gameplay/T66UniqueDebuffEnemy.cpp`:
  - Stage 2 Unique static mesh (`SM_Unique_Stage02`) is also recentered using its static-mesh bounds origin so it appears at the actor location.

**Localization**
- No new player-facing runtime strings.

**Verification / proof**
- ValidateFast builds ✅ (UE 5.7):
  - `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66Editor Win64 Development "C:\UE\T66\T66.uproject" -WaitMutex -FromMsBuild -architecture=x64`
  - `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66 Win64 Development "C:\UE\T66\T66.uproject" -WaitMutex -FromMsBuild -architecture=x64`
