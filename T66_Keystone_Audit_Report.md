# T66 Keystone Audit Report
Date: 2026-02-01

This file is the **single checklist** for the keystone audit session. It captures:
- **Bible vs repo reality** divergences (repo wins; Bible will be updated to match)
- **Performance** risks for low-end PCs (ranked)
- **Lifetime / leak** risks (delegates, timers, widget lifetimes)
- **Bloat / duplication** candidates and the safe cleanup sequence

---

## 1) Bible divergences (repo wins; update `T66_Bible.md`)

### 1.1 Tutorial vs Start Area (Stage 1, first-time only)
- **Bible**: “larger Tutorial Start Area” inside the Start Area flow (see `T66_Bible.md` → “2.2 START AREA + GATE START”, esp. “First-Run Tutorial Behavior (Stage 1 Only)”).
- **Repo reality (per `memory.md`)**:
  - Dedicated enclosed **Tutorial Area** inside `GameplayLevel`.
  - First-time / forced tutorial spawns into Tutorial Area.
  - Tutorial ends via **portal teleport** into the normal Stage 1 Start Area (no level load).
  - Tutorial is driven by **HUD text above crosshair** (event-driven; no per-frame UI polling).
- **Action**: Update Bible wording to reflect a dedicated Tutorial Area + portal teleport into Start Area, while keeping the “Gate Start” rule intact.

### 1.2 Tutorial loot / enemy sequence
- **Bible**: tutorial demonstrates a **White** loot bag / White item proc.
- **Repo reality (per `memory.md`)**:
  - Tutorial manager spawns tutorial enemies and forces item/proc demonstration with specific drops (includes “black loot bag” and an “elite (mini-boss scaled)” tutorial enemy with a White bag in the log).
- **Action**: Update Bible tutorial section to match the implemented tutorial sequence and keep the “teach White proc” intent.

### 1.3 Mini-bosses and Unique enemies (definitions + loot rules)
- **Bible** (examples): mini-boss chance/cap per wave; Unique enemies don’t drop loot bags; “all non-boss enemies are melee-only”.
- **Repo reality**: needs reconciliation against current code paths (wave director + enemy classes) before editing Bible text beyond the already-known tutorial deltas.
- **Action**: During Bible pass, align *only what we can prove from repo state* and explicitly mark any remaining “DECIDE LATER” items as such.

---

## 2) Performance risks (ranked, low-end focused)

These are the highest ROI targets identified from code audit:

1) **Synchronous asset/data loads** during init/spawn (hitch risk)
   - Likely hotspots: `Source/T66/Core/T66GameInstance.cpp`, `Source/T66/Gameplay/T66GameMode.cpp`
2) **Heavy HUD refresh work** triggered by many state change events
   - Hotspot: `Source/T66/UI/T66GameplayHUDWidget.cpp` (large `RefreshHUD()` path)
3) **High-frequency UI timers** (60Hz) during wheel/steal bar animations
   - Hotspots: `Source/T66/UI/T66GameplayHUDWidget.cpp`, `Source/T66/UI/T66VendorOverlayWidget.cpp`
4) **Actor iteration in gameplay hot paths**
   - Hotspots: `Source/T66/Gameplay/T66EnemyBase.cpp`, `Source/T66/Gameplay/T66EnemyDirector.cpp`
5) **Per-frame GameMode tick doing subsystem work**
   - Hotspot: `Source/T66/Gameplay/T66GameMode.cpp`

---

## 3) Lifetime / leak risks to validate

- **UMG/Slate delegate bindings**: ensure every `AddDynamic`/`AddUObject`/`AddLambda` has a matching unbind path (destruct/deactivate/endplay).
- **Timer handles**: ensure timers are cleared on widget destruction and state exit (no orphan timers).
- **UI manager caching**: ensure cached screens are not holding strong references that prevent garbage collection.

---

## 4) Bloat / duplication candidates (do last)

### 4.1 Optional-by-design UI Blueprints (not “bloat”)
Per `memory.md`, these may be intentionally optional overrides (C++ fallback exists):
- `WBP_LanguageSelect`
- `WBP_Achievements`
- `WBP_SaveSlots`

### 4.2 Actual bloat candidates (needs AssetRegistry proof)
- Unreferenced `WBP_*` assets not used as optional overrides
- Duplicate data sources for the same concept (e.g., multiple tables/lists that drift)
- Legacy/unused actors/widgets left after C++-driven UI replacements

### 4.3 Safe cleanup sequence (never skip)
1) Remove code/asset references first (compile-time safety when possible)
2) Delete in tiny batches
3) Fix redirectors (editor)
4) ValidateFast after each batch
5) Log exact paths and results in `memory.md`

---

## 5) Session checklist

- [ ] Phase 1: Update Bible to match repo tutorial + start area + miniboss/unique definitions we can prove
- [ ] Phase 2: Improve guidelines + memory index so future work is safer
- [ ] Phase 3: Perf hardening (sync loads, HUD refresh granularity, hot-path iterators, timers)
- [ ] Phase 4: Asset audit script + generate unused/optional classification report
- [ ] Phase 5: Cleanup batches (only what audit proves is safe)

