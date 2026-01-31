# T66 Cursor Guidelines (Build BibleV1 into the Unreal Project)

## HARD RULE (Non‑Negotiable) — Localize *every* new player-facing string at implementation time

**Any time you add or change player-facing text, you must complete the full localization pipeline as part of that same change-set (no “we’ll translate it later”).**

- **Author**: Use **`LOCTEXT` / `NSLOCTEXT`** and/or **String Tables** (`FText::FromStringTable`).  
  - **Never** add per-language translation logic (eg `switch(CurrentLanguage)` returning strings).  
  - **Never** ship new UI text via `FText::FromString(TEXT("..."))`.
- **Gather**: Run GatherText so new keys land in `Content/Localization/T66/T66.manifest` and `.archive` files.
- **Translate (all supported cultures)**: Ensure translations exist for all 22 supported cultures.  
  - Allowed: edit `.po` / `.archive` by hand (human translation), or run the offline batch translator `Scripts/AutoTranslateLocalizationArchives.py` as a baseline.
- **Compile**: Run `Config/Localization/T66_Compile.ini` to regenerate `Content/Localization/T66/<culture>/T66.locres`.
- **Verify**: Verify the new/changed text visibly updates when switching culture (event-driven; no tick polling).

**Agent acknowledgement requirement:** After reading `memory.md` and this file, the agent must explicitly acknowledge this rule in its next message before making changes (e.g., “Acknowledged: any new player-facing text will be localized end-to-end: author → gather → translate → compile `.locres` → verify.”).

### Required “delta localization” workflow (agent must run this automatically)

When a change-set adds/changes player-facing text, the agent must do the following **as part of the same change-set**. The agent must **run these commands itself** (do not ask the user to copy/paste commands).

- **If you changed C++ (`Source/**`) and/or config (`Config/**`) text**
  - Run source gather:

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "C:\UE\T66\T66.uproject" -run=GatherText -config="C:\UE\T66\Config\Localization\T66_Gather_Source.ini" -unattended -nop4 -nosplash -nullrhi -log="C:\UE\T66\Saved\Logs\T66_Gather_Source.log"
```

- **If you changed assets with text (UMG/Blueprints/Maps/DataTables/etc.)**
  - Run the narrow assets gather:

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "C:\UE\T66\T66.uproject" -run=GatherText -config="C:\UE\T66\Config\Localization\T66_Gather_Assets.ini" -unattended -nop4 -nosplash -nullrhi -log="C:\UE\T66\Saved\Logs\T66_Gather_Assets.log"
```

- **Translations (must cover all 22 cultures)**
  - If human translations are not provided yet, run the offline baseline translator to populate `.archive` entries:

```powershell
python -u "C:\UE\T66\Scripts\AutoTranslateLocalizationArchives.py"
```

- **Compile `.locres` (always after updating translations)**

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "C:\UE\T66\T66.uproject" -run=GatherText -config="C:\UE\T66\Config\Localization\T66_Compile.ini" -unattended -nop4 -nosplash -nullrhi -log="C:\UE\T66\Saved\Logs\T66_Compile.log"
```

- **Fast verification (agent must do at least one)**
  - **Artifact check**: ensure `Content/Localization/T66/<culture>/T66.locres` exists for all cultures.
  - **Runtime check**: launch editor/game, switch to a non-English language and confirm the new/changed text updates.

This document is the operating manual for an AI agent (Cursor or any CLI-capable agent) working inside the **T66** Unreal project. It defines **how** to make changes safely, deterministically, and in a way that stays performant on low-end PCs—while remaining easy for LLMs to extend without breaking the game.

**Required companion file:** `memory.md` (living progress ledger).  
Every meaningful change must be reflected there with proof (commands run, results, and any risks).

---

## 0. Source of truth

**Design source of truth (SSOT):**
- `T66_Bible.md` (converted from BibleV1) — the gameplay/UI spec to implement.

**Supporting design/structure docs (inputs for architecture + naming):**
- `T66_Unreal_UI_V1.docx`
- `T66_Unreal_Gameplay (1).docx`
- `T66_Unreal_Bridge (1).docx`

**Engineering source of truth (project reality):**
- The repository itself (current C++/assets/config).

When there is a conflict, the agent must:
1. Record it in `memory.md` (what conflicts, where).
2. Prefer SSOT for *intended behavior*, but prefer the repo for *what currently exists*.
3. Resolve by making the smallest change that moves the repo toward SSOT **without introducing fragile/temporary systems**.

---

## 1. Prime directive

### Build the final structure first (no throwaway systems)
Do not create “temporary now, real later” systems. Favor scalable, stable foundations.

### Localization is mandatory (hard rule, culture-based)
All **player-facing** strings must be localized using Unreal’s **culture-based** localization system (gather → translate → compile `.locres`).

- **Preferred**: `LOCTEXT` / `NSLOCTEXT` + **String Tables** (`FText::FromStringTable`) so `SetCurrentCulture` drives translations.
- **Allowed**: `UT66LocalizationSubsystem::GetText_*()` only if it returns culture-localized `FText` (eg `NSLOCTEXT`/StringTable-backed), not manual per-language translations.
- **Never**:
  - ship new UI strings via `FText::FromString(TEXT("..."))`
  - add/extend per-language translation tables in code (eg `switch(CurrentLanguage)` returning `FText::FromString` translations). Those must be replaced by `.locres` translations.

**Supported languages (UI selector list):**
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

### Ship-minded stability
Assume decisions made in-service of BibleV1 are “final-at-launch” unless SSOT explicitly says otherwise. Avoid choices that require rewriting core systems later.

### Low-end performance is a feature
Design for weak CPUs/GPUs and low RAM. Avoid patterns that are “fine on dev machines” but hitch/crash on lower-end hardware.

### LLM-friendly = deterministic + schema-first
The project must be structured so an LLM can safely extend it:
- Stable file/folder conventions
- Stable IDs and schemas
- Predictable ownership boundaries
- Tight change batches, validated frequently

---

## 2. Change safety: Green / Yellow / Red operations

The agent **may touch everything** (including `/Content/**` and `.uasset`), but must follow this risk protocol.

### Green (low risk)
Typical: C++ in `/Source/**`, text config in `/Config/**`, data text files (`.csv/.json/.ini/.md`).

Rules:
- Can proceed immediately.
- Must still pass **ValidateFast** before committing.

### Yellow (medium risk)
Typical: editing a small number of `.uasset` files, adding a few assets, small renames.

Rules:
- Work in small batches (1–5 assets at a time).
- ValidateFast after each batch.
- Always log exact asset paths in `memory.md`.

### Red (high risk)
Typical: mass refactors, bulk renames/moves, “rebuild UI layouts”, large folder reorganizations, anything likely to create redirector chaos or widespread asset churn.

Rules:
- Write a **plan** into `memory.md` first (steps, expected impact, rollback plan).
- Commit a clean checkpoint before starting.
- Execute in phases; ValidateFast after each phase.
- Run **ValidateFull** when done.

---

## 3. The “atomic change-set” workflow (non-negotiable)

Every change must follow this loop:

1) **Plan (1–10 lines)**
- What you are changing
- Why (Bible section or engineering need)
- Files/assets expected to change
- What “done” looks like

2) **Implement**
- Keep the batch small and focused.

3) **Validate**
- Run **ValidateFast** (always).
- Run **ValidateFull** when required by Yellow/Red scope.

4) **Commit**
- Commit message describes the change set in plain language.
- No “mega commits” that hide unrelated edits.

5) **Update `memory.md`**
- What changed, how, and proof (commands + outcomes)
- Include commit hash

If any step fails, do not continue stacking changes. Roll back or fix forward immediately.

---

## 4. UI performance non-negotiables (real killers)

Even perfect architecture can be ruined by UI anti-patterns. Avoid these:

### ❌ Killer #1: Widgets doing work every frame
Avoid:
- Per-frame polling
- Constantly recalculating text
- Constantly refreshing lists

✅ Fix:
- Update UI only when values change (event-driven).
  - Example: health changes → push one new value.

### ❌ Killer #2: Too many widgets on screen at once
Low-end cost is dominated by:
- Total UI elements drawn
- Layout complexity

✅ Fix:
- Keep overlays minimal.
- Don’t stack multiple large panels full of text.
- Keep HUD tight.

### ❌ Killer #3: Heavy UI effects
Expensive patterns include:
- Blur
- Transparency stacks everywhere
- Huge animated gradients
- Lots of large images

✅ Fix:
- Effects should be subtle and rare.
- Prefer simple shapes.
- Don’t make “fancy expensive” the default HUD.

### ❌ Killer #4: Spawning/removing large UI repeatedly
Creating complex UI on demand causes stutters.

✅ Fix:
- For frequent surfaces: create once, show/hide.
- For rare screens: lazy-load once, then reuse.

### Option A (recommended): Zero “per-frame UI thinking”
- No Tick logic in UI widgets.
- No looping updates.
- Updates happen only via events (damage taken, gold changed, etc.).
- Keep active UI small (current screen + minimal overlays).
- Tooltip logic must be cheap (no constant rebuilds).
- Keep UI textures reasonable (avoid giant full-screen images everywhere).
- Keep animation counts low (a few key animations are fine; avoid “forever animating 20 things”).

---

## 5. Data architecture for scale (LLM-friendly + stable)

### Option A (recommended): CSV/JSON as source of truth → DataTables
Do **not** create hundreds of one-off DataAssets if the data is tabular.

- Use **DataTables** for:
  - Items, enemies, bosses, stages, achievements, etc.
- Use **DataAssets** for:
  - System-level configuration
  - Rare special cases that are not tabular

### Schema-first rules
For each table:
- Define a stable **RowID** convention (never reused).
- Prefer “explicit columns” over free-form blobs.
- Avoid storing derived values that can be computed reliably.
- Keep “design text” separated from “runtime fields” when possible.

### Change compatibility rules
- Never rename or delete a column without a migration plan logged in `memory.md`.
- Prefer adding new columns with safe defaults.
- Treat IDs as permanent contracts.

---

## 6. Performance-minded engineering rules (beyond UI)

## 6.1 Make systems removable without breakage (deprecation-first)

When you need to change direction later, do it without detonating the project:

- Prefer **deprecating** fields/rows/features before deleting them.
- If something must be removed:
  - Remove call sites first (compile-time safety), then remove the data/asset.
  - Keep redirect/migration steps explicit (record them in `memory.md`).
- Avoid “spooky action at a distance”:
  - Each feature/system should have a clear ownership boundary (one primary module/file group).
  - Avoid scattered logic spread across many unrelated Blueprints.

A good rule: **a feature can be turned off without crashes** (even if the gameplay is disabled).

## 6.1.1 Repo hygiene (prevent “zombie” features)

When removing or replacing a feature, do not leave behind unused code paths that can be accidentally re-enabled later:

- Remove call sites first (compile-time safety), then remove the old classes/files.
- Delete unused C++ files that are no longer referenced (don’t keep “maybe later” actors/widgets around).
- If a UI flow is replaced (e.g., full-screen modal → HUD prompt), remove the old widget + controller hooks.
- If you intentionally keep a deprecated stub, clearly mark it as deprecated and record it in `memory.md`.

## 6.2 Schema versioning (for LLM safety)

If you introduce a breaking data shape change:
- Add a **SchemaVersion** field (table-level or per-row).
- Write an explicit migration step (even if it’s a one-time editor script).
- Record:
  - old schema
  - new schema
  - how existing rows/assets are migrated
  - how you verified success
in `memory.md`.

## 6.3 Input-mode + modal safety (prevents “game frozen” regressions)

Gameplay input bugs are some of the easiest regressions to introduce and the hardest for players to diagnose.
Follow these rules:

- **Always restore gameplay input** after closing gameplay overlays:
  - Prefer using a single helper (e.g., `AT66PlayerController::RestoreGameplayInputMode()`).
- **Assume `UT66UIManager` is single-modal**:
  - Opening a modal from the Pause Menu replaces it (not stacked).
  - If Settings/ReportBug/etc. is opened from Pause, then closing it must return to **Pause Menu** (or explicitly unpause and restore input).
- Only use `FInputModeGameAndUI` / mouse cursor capture when the player truly needs pointer interaction.
  - If you enable cursor/click events, you must disable them on exit.


### Avoid runtime spikes
- Avoid synchronous loads during gameplay.
- Prefer preloading or staged async loads.
- Reuse objects where practical (pooling) instead of repeated spawn/destroy in hot paths.

### Keep assets low-end friendly
- Avoid unnecessarily high-resolution textures for UI and common props.
- Avoid overly complex materials for frequently visible surfaces.
- Use conservative defaults; allow “fancy” as optional upgrades, not baseline.

### Fail safe
- When data is missing or invalid: fail gracefully, log clearly, and avoid crashes.
- Keep error messages actionable (include asset name/RowID).

---

## 7. CLI operations (Windows + Git Bash oriented)

> The agent should work entirely from CLI when possible, using Git Bash on Windows.

### Required environment variables (set once per terminal)
```bash
# Project path (adjust to your machine)
export UPROJECT="/c/UE/T66/T66.uproject"

# Unreal Engine 5.7 — install location (reference for builds and CLI)
# Windows: C:\Program Files\Epic Games\UE_5.7\
export UE_ROOT="/c/Program Files/Epic Games/UE_5.7"

# Build scripts
export UE_BUILD_BAT="$UE_ROOT/Engine/Build/BatchFiles/Build.bat"
export UE_UAT_BAT="$UE_ROOT/Engine/Build/BatchFiles/RunUAT.bat"
export UE_EDITOR_EXE="$UE_ROOT/Engine/Binaries/Win64/UnrealEditor.exe"
export UE_EDITOR_CMD_EXE="$UE_ROOT/Engine/Binaries/Win64/UnrealEditor-Cmd.exe"
```

### Always start with repo sanity
```bash
git status
git diff --stat
```

### ValidateFast (default gate)
Run after every atomic change-set:
```bash
# 1) Build the editor target (names may vary by project; adjust if needed)
"$UE_BUILD_BAT" T66Editor Win64 Development -Project="$(cygpath -w "$UPROJECT")" -WaitMutex -FromMsBuild -architecture=x64

# 2) Build the game target
"$UE_BUILD_BAT" T66 Win64 Development -Project="$(cygpath -w "$UPROJECT")" -WaitMutex -FromMsBuild -architecture=x64
```

### Localization pipeline (culture-based; GatherText → translations → `.locres`)

**Configs (source-controlled):**
- `Config/Localization/T66_Gather_Source.ini` (fast/safe)
- `Config/Localization/T66_Gather_Assets.ini` (narrow assets-only)
- `Config/Localization/T66_Compile.ini` (compile-only)

**Runtime load (required):**
- Ensure `Config/DefaultGame.ini` contains:
  - `[Internationalization]`
  - `LocalizationPaths=%GAMEDIR%Content/Localization/T66`

**Supported cultures (must be exactly 22):**
- `en`, `zh-Hans`, `zh-Hant`, `ja`, `ko`, `ru`, `pl`, `de`, `fr`, `es-ES`, `es-419`, `pt-BR`, `pt-PT`, `it`, `tr`, `uk`, `cs`, `hu`, `th`, `vi`, `id`, `ar`

**Run (Git Bash):**
```bash
"$UE_EDITOR_CMD_EXE" "$(cygpath -w "$UPROJECT")" -run=GatherText -config="$(cygpath -w "$PWD")\\Config\\Localization\\T66_Gather_Source.ini" -unattended -nop4 -nosplash -nullrhi
"$UE_EDITOR_CMD_EXE" "$(cygpath -w "$UPROJECT")" -run=GatherText -config="$(cygpath -w "$PWD")\\Config\\Localization\\T66_Gather_Assets.ini" -unattended -nop4 -nosplash -nullrhi
"$UE_EDITOR_CMD_EXE" "$(cygpath -w "$UPROJECT")" -run=GatherText -config="$(cygpath -w "$PWD")\\Config\\Localization\\T66_Compile.ini" -unattended -nop4 -nosplash -nullrhi
```

**Run (PowerShell):**
```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "C:\UE\T66\T66.uproject" -run=GatherText -config="C:\UE\T66\Config\Localization\T66_Gather_Source.ini" -unattended -nop4 -nosplash -nullrhi
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "C:\UE\T66\T66.uproject" -run=GatherText -config="C:\UE\T66\Config\Localization\T66_Gather_Assets.ini" -unattended -nop4 -nosplash -nullrhi
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "C:\UE\T66\T66.uproject" -run=GatherText -config="C:\UE\T66\Config\Localization\T66_Compile.ini" -unattended -nop4 -nosplash -nullrhi
```

**Expected outputs:**
- `Content/Localization/T66/T66.manifest`
- `Content/Localization/T66/<culture>/T66.archive`
- `Content/Localization/T66/<culture>/T66.locres`

**Verify `.locres` count (PowerShell):**
```powershell
(Get-ChildItem -Path "C:\UE\T66\Content\Localization\T66" -Recurse -Filter *.locres).Count
```

Optional (recommended when touching data systems):
- Run any project validation commandlets / automation tests that exist in the repo.
- If none exist yet, create them early (data validation is a stability multiplier).

### ValidateFull (heavier gate)
Use for Red scope or before major milestones (cook/package):
```bash
"$UE_UAT_BAT" BuildCookRun -project="$(cygpath -w "$UPROJECT")" -noP4 -platform=Win64 -clientconfig=Development -build -cook -stage -pak -archive -archivedirectory="$(cygpath -w "$PWD")/Artifacts"
```

*(If you customize your packaging pipeline, keep the authoritative command in `memory.md`.)*

---

## 8. Asset operations discipline (when touching Content / .uasset)

When making asset moves/renames:
- Do it in small batches.
- Clean up redirectors (Editor action or an approved automation method).
- ValidateFast immediately.
- Log all paths in `memory.md`.

When editing UI Blueprints:
- Enforce “event-driven UI” (no per-frame logic).
- Prefer show/hide over rebuild for frequently used widgets.
- Avoid heavy effects by default.

### Non-blocking micro-interactions (loot, proximity prompts, etc.)

For small, frequent interactions (loot bags, proximity actions, “press key” prompts):

- Prefer a **HUD prompt/banner** over a full-screen blackout/modal.
- Do not pause the game or switch input mode unless necessary.
- Keep the interaction cheap: no per-frame UI rebuild; update only on proximity changes / state changes.
- If the interaction has rarities (Black/Red/Yellow/White), make the rarity **visibly readable** in-world/UI.

### Rarity-locked pools (content consistency)

If a drop has a rarity tier, the contents must come from the matching tier:

- Black drop → Black items pool; Red → Red; Yellow → Yellow; White → White.
- Pools must be safe:
  - If a pool is empty/missing, fall back gracefully (e.g., “all items”) and log clearly (do not crash, do not return NAME_None).
  - Record the fallback behavior in `memory.md`.

---

## 9. Troubleshooting appendix (QA/support oriented)

These are **not** the primary optimization strategy, but they help reproduce and diagnose stutter/crash reports in the wild.
Only apply them when you’re debugging a specific issue and always record what you changed.

### 9.1 Clean environment protocol (debugging baseline)
- Disable overlays that hook rendering: Steam overlay, Discord overlay, Xbox Game Bar, GPU overlays (NVIDIA/AMD).
- Prefer a true “cold boot” for stubborn hitching reports (full shutdown, then boot).
- Keep chipset/GPU drivers current (use vendor installers when possible; Windows Update drivers can be problematic).
  - If Windows keeps replacing GPU drivers, document the mitigation steps you used.

### 9.2 Antivirus / security stutter killers (debugging baseline)
Real-time scanning can cause hitches while shaders/files stream.

- Add exclusions for:
  - The game install folder (or packaged build folder)
  - The game executable
  - Game Saved/Config folders (where applicable)
- Shader cache folders vary by vendor and game. If investigating shader-related stutters, document the exact folders used on that machine.
- Some users disable certain exploit mitigations per-game (advanced / risky). If you do this for debugging, **log it** and revert after testing.

### 9.3 UE-level configuration tuning (only when verified)
UE versions differ. If you apply engine CVars or `.ini` tuning:
- Verify the setting exists in your engine version.
- Change one thing at a time.
- Measure before/after (frametime graph, hitch count) and record results in `memory.md`.

Examples that some UE5 projects use (verify in your version before adopting):
- Shader/PSO precaching features (names vary by version/config).
- “Fast VRAM” resource hints (advanced; verify support and measure).

### 9.4 The nuclear option (only for controlled debugging)
- DDU clean driver reinstall (Safe Mode) → reinstall official driver.
- Disable CPU/GPU overclocks (return to stock) if crashes/hitches are unexplained.
- Clear shader caches to force rebuild (can make first run worse, but may fix corrupted caches).
- Avoid drastic OS tweaks (like disabling virtual memory) unless you’re doing controlled experiments; they can reduce stability.

### 9.5 How to record troubleshooting in `memory.md`
For each troubleshooting experiment, log:
- Machine specs (CPU/GPU/RAM, OS version) if known
- Exact steps taken
- What you expected
- What happened (before/after hitch metrics, crash frequency, logs)
- Whether the change should be kept, reverted, or made optional



## 10. Definition of “done” for any deliverable

A deliverable is only “done” when:
- It matches the relevant Bible section behaviorally.
- It passes ValidateFast.
- It has a clean commit.
- `memory.md` records what changed and how it was verified.
