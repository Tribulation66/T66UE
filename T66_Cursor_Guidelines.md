# T66 Cursor Guidelines (Build BibleV1 into the Unreal Project)

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

# Unreal install root (adjust to your machine)
export UE_ROOT="/c/Program Files/Epic Games/UE_5.7"

# Build scripts
export UE_BUILD_BAT="$UE_ROOT/Engine/Build/BatchFiles/Build.bat"
export UE_UAT_BAT="$UE_ROOT/Engine/Build/BatchFiles/RunUAT.bat"
export UE_EDITOR_EXE="$UE_ROOT/Engine/Binaries/Win64/UnrealEditor.exe"
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
