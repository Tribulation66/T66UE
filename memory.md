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
- **Model pipeline status:** ✅ Imported + wired (data-driven)
  - Source packs extracted to `C:\UE\T66\SourceAssets\Extracted\` (~31 `.fbx` files)
  - Imported assets exist under `/Game/Characters/...` (SkeletalMeshes + Skeletons + AnimSequences where provided)
  - Data-driven visuals mapping implemented via `DT_CharacterVisuals` (backed by `Content/Data/CharacterVisuals.csv`)
  - Runtime wiring implemented (heroes/enemies/bosses/companions/NPCs apply skeletal visuals + optional looping anim at runtime)
  - Note: some characters have **no animation assets** in the provided FBX packs, so they remain static (fallback picks any compatible anim only if it exists)
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

- **Localization guardrail debt** — Some newer gameplay/UI strings are still hardcoded in Slate/UMG and must be moved into `UT66LocalizationSubsystem::GetText_*()` per guardrails/Bible (examples include some HUD labels/prompts).
- **Save Slots** — C++ fallback exists; optional WBP_SaveSlots at `/Game/Blueprints/UI/WBP_SaveSlots` for visual customization.
- **Hearts/Inventory** — Use Slate `BorderImage(WhiteBrush)` for filled squares; dynamic delegates use `AddDynamic`/`RemoveDynamic` (not AddUObject).
- **Coliseum map asset** — `Content/Maps/ColiseumLevel.umap` may be missing in this repo state; code has a safe fallback that reuses `GameplayLevel` for Coliseum-only behavior when needed.

---

## 4) Change log (append-only)

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

- **Languages:** English, Português (Brasil), 繁體中文
- **Subsystem:** `UT66LocalizationSubsystem`
- **Usage:** `GetText_*()` for all UI strings; screens subscribe to OnLanguageChanged where needed.
