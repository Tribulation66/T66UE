# T66 Procedural Hills Landscape — Detailed Plan & Checklist

**Goal:** Procedural rolling-hills terrain that regenerates each time the player presses "Enter the Tribulation." Same map size; different hill layout per run (seed-controlled). Editor tool for authoring/testing; runtime generation wired to the Enter button.

**Engine:** UE 5.7 · **Project:** T66

---

## 1. Architecture Overview

| Layer | Responsibility |
|-------|----------------|
| **T66 (game module)** | Shared heightfield algorithm (noise + smooth), parameter struct, and runtime terrain application. No editor-only APIs. |
| **T66Editor** | "T66 Tools" menu, "Generate Procedural Hills Landscape (Dev)" action, and editor-only Landscape create/edit (write height data into level). |
| **Game Instance** | Hold run seed for procedural terrain (set when entering tribulation). |
| **GameMode (GameplayLevel)** | On level init: create or find Landscape, generate heightfield from seed, apply to Landscape. |

**Data flow (runtime):**  
Enter the Tribulation → set `UT66GameInstance::ProceduralTerrainSeed` (e.g. random) → `OpenLevel(GameplayLevel)` → GameMode runs terrain generation using seed → Landscape appears with new hills.

**Data flow (editor):**  
User runs "Generate Procedural Hills Landscape (Dev)" → tool uses same algorithm + current params → creates/updates Landscape in the currently open level.

---

## 2. Checklist Summary

- [ ] **Phase A:** Shared core (T66): noise + smooth algorithm, params, height buffer API
- [ ] **Phase B:** Editor tool (T66Editor): T66 Tools menu, generator action, create/update Landscape in level
- [ ] **Phase C:** Runtime: seed in Game Instance, GameMode generates terrain on GameplayLevel load
- [ ] **Phase D:** Wire "Enter the Tribulation" to set seed and (optional) document flow
- [ ] **Phase E:** Validate, memory.md, commit

---

## 3. Phase A — Shared Core (T66 Module)

**Purpose:** One implementation of the procedural hills algorithm and parameters usable by both editor and game. No editor-only or Landscape module dependency in this phase for the *algorithm*; Landscape dependency only where we apply heights at runtime.

### 3.1 Add Landscape dependency to T66 (for runtime)

- [ ] In `Source/T66/T66.Build.cs`: add `"Landscape"` to `PublicDependencyModuleNames` (needed for runtime Landscape creation/editing).

### 3.2 Parameter struct (editable in code)

- [ ] Create `Source/T66/Gameplay/T66ProceduralLandscapeParams.h` (and optional .cpp if needed).
- [ ] Define a struct (e.g. `FT66ProceduralLandscapeParams`) or USTRUCT with:
  - `int32 Seed`
  - `float HillAmplitude` (default ~600)
  - `float VeryLargeScaleMeters` (default ~900)
  - `float LargeScaleMeters` (default ~350)
  - `float MediumScaleMeters` (default ~140)
  - `int32 SmoothPasses` (default 4, range 3–6)
  - `float SmoothStrength` (default 0.45, range 0.35–0.6)
  - `bool bCarveRiverValley` (default false)
  - `float RiverWidthMeters` (if river, keep gentle)
  - `float RiverDepthUU` (if river)
- [ ] Add size preset enum: e.g. `ET66LandscapeSizePreset::Small` (~1 km × 1 km), `Large` (~2–4 km × 2–4 km). Map presets to component counts / dimensions (document in code).

### 3.3 Deterministic noise (T66)

- [ ] Implement or use a deterministic 2D noise function (e.g. seeded Perlin/simplex) in T66. Options:
  - Wrap `FMath::PerlinNoise2D` with a seed (if UE exposes seed), or
  - Implement a small seeded RNG + noise (e.g. value noise or Perlin) so same seed ⇒ same sequence.
- [ ] Expose a function like `float SampleNoise2D(int32 Seed, float X, float Y, float Scale)` where Scale is in world units (meters) so we can derive frequency from VeryLargeScaleMeters etc.

### 3.4 Height formula and smoothing (T66)

- [ ] Implement height generation into a **height buffer** (TArray<float> or TArray<uint16> and scale):
  - Grid dimensions from preset (e.g. Small = N×N vertices at Landscape resolution).
  - For each vertex (i, j) → world X, Z:
    - `H = HillAmplitude * (0.65f * Noise(Seed, X, Z, VeryLargeScaleMeters) + 0.25f * Noise(..., LargeScaleMeters) + 0.10f * Noise(..., MediumScaleMeters))`
  - Optional: if `bCarveRiverValley`, subtract a smooth valley (e.g. Gaussian or smoothstep along one axis, width/height from params).
- [ ] Implement multi-pass smoothing (e.g. 3×3 or 5×5 box average, blend by SmoothStrength, repeat SmoothPasses times). No clamp that creates flat plateaus.
- [ ] Expose API: e.g. `GenerateHeightfield(FT66ProceduralLandscapeParams, int32 SizeX, int32 SizeY, TArray<float>& OutHeights)` (and optionally a version that returns uint16 for Landscape).

### 3.5 Landscape size from preset

- [ ] Helper: given `ET66LandscapeSizePreset`, return:
  - World size in UU (e.g. Small 1000 m → 100000 uu if 1 m = 100 uu, or project convention).
  - Number of Landscape components (e.g. 1 component = 1 quads × 1 quads subsection; derive from UE Landscape rules: component size, quads per section).
- [ ] Document in code: UE Landscape uses components of fixed quads; total size = f(components, quads per component, scale).

---

## 4. Phase B — Editor Tool (T66Editor Module)

**Purpose:** "T66 Tools" menu with "Generate Procedural Hills Landscape (Dev)" that creates or reuses one Landscape in the current level and applies the generated heightfield.

### 4.1 T66Editor build dependencies

- [ ] In `Source/T66Editor/T66Editor.Build.cs`: add `"Landscape"`, `"LandscapeEditor"` (or equivalent for UE 5.7; confirm module name in Engine) so we can create/edit Landscape in editor.

### 4.2 T66 Tools menu

- [ ] In T66Editor module startup: register an editor menu (e.g. "Window" or "Tools") with submenu "T66 Tools".
- [ ] Add menu entry: "Generate Procedural Hills Landscape (Dev)".
- [ ] On click: run the procedural landscape generator for the current level.

### 4.3 Editor generator class / lambda

- [ ] Implement editor-side flow:
  1. Get current world (GWorld or from context).
  2. Build params: use defaults; optionally load from config or leave hardcoded for now.
  3. Call shared T66 API: `GenerateHeightfield(Params, SizeX, SizeY, OutHeights)`.
  4. Find or create Landscape in level:
     - If Landscape exists: reuse it (ensure size matches or handle resize; prefer "replace height data" for same dimensions).
     - If not: create one Landscape actor with correct component layout and size from preset (Small by default).
  5. Write `OutHeights` into the Landscape's height data (editor Landscape API).
  6. Notify Landscape to update (e.g. Reimport heightmap or equivalent).
- [ ] Use full-file C++ for new editor files; no partial snippets.

### 4.4 Default preset in editor

- [ ] Editor tool uses Small preset by default (e.g. ~1 km × 1 km) so it runs quickly. Option to switch to Large in code/params later.

---

## 5. Phase C — Runtime Terrain on GameplayLevel Load

**Purpose:** When GameplayLevel loads (after "Enter the Tribulation"), generate procedural hills and apply them to the level’s Landscape (create if missing).

### 5.1 Game Instance seed

- [ ] In `UT66GameInstance`: add `UPROPERTY()` e.g. `int32 ProceduralTerrainSeed`. Set when the player commits to entering the tribulation (see Phase D).

### 5.2 When to generate (GameMode)

- [ ] Choose hook: `InitGame` (earlier) or `BeginPlay`. Prefer **InitGame** or very start of **BeginPlay** so terrain exists before other level setup (e.g. spawns, miasma). If Landscape must exist before certain logic, generate first.
- [ ] Only run for main gameplay (not for Coliseum-only or Stage Boost if those use the same level but different layout; if they share the same Landscape, still generate once per level load with the same seed).

### 5.3 GameMode terrain flow

- [ ] In GameMode (e.g. at start of BeginPlay, or in InitGame):
  1. Get `ProceduralTerrainSeed` from `UT66GameInstance`.
  2. Build `FT66ProceduralLandscapeParams` (use defaults; optionally read from Game Mode or config).
  3. Call shared `GenerateHeightfield(...)` to fill a height buffer.
  4. Find existing Landscape in level, or create one (runtime Landscape creation API).
  5. Apply height buffer to Landscape (runtime path: may require Landscape proxy or edit interface; confirm UE 5.7 runtime API).
  6. If creation fails or is unsupported, fall back gracefully (e.g. keep level as-is, log warning).

### 5.4 Runtime Landscape API (UE 5.7)

- [ ] Confirm in UE 5.7: how to create a new `ALandscape` at runtime (e.g. `ALandscape::Create` or equivalent) and how to set height data at runtime. If runtime height editing is not supported, fallback: level always contains a "template" Landscape and we only replace height data via the available runtime API.
- [ ] Document in `memory.md`: chosen approach (create vs update), any limitations.

---

## 6. Phase D — Wire "Enter the Tribulation" to Seed

**Purpose:** Each time the user presses Enter the Tribulation, a new seed is used so the next GameplayLevel load gets new hills.

### 6.1 Set seed before opening level

- [ ] In `UT66HeroSelectionScreen::OnEnterTribulationClicked()` (or immediately before `OpenLevel`):
  - Get `UT66GameInstance`.
  - Set `ProceduralTerrainSeed` to a new value: e.g. `FMath::Rand()` or `FDateTime::Now().GetTicks()` or a run ID if you have one. Ensure it’s different enough per click (no need to persist across sessions unless desired).
- [ ] Keep existing logic (selection storage, HideAllUI, OpenLevel).

### 6.2 Optional: pass preset at runtime

- [ ] If you want runtime to use a different size than editor default, add e.g. `ET66LandscapeSizePreset ProceduralTerrainSize` to Game Instance and set it when entering (default Small).

---

## 7. Phase E — Validation & Documentation

### 7.1 Validate

- [ ] **ValidateFast:** Build T66 + T66Editor, no errors.
- [ ] **Smoke test (editor):** Open GameplayLevel, run "Generate Procedural Hills Landscape (Dev)", confirm one Landscape with rolling hills; run again with different seed (if exposed in editor), confirm different layout.
- [ ] **Smoke test (runtime):** From frontend, press "Enter the Tribulation" twice (or enter, leave, enter again); confirm terrain layout changes (or same if same seed is reused; then change seed source and confirm variation).

### 7.2 memory.md

- [ ] Add short "Procedural terrain" note: where params live, where seed is set, that "Enter the Tribulation" triggers new terrain via seed, editor menu name, and any runtime Landscape creation caveat.

### 7.3 Commit

- [ ] Single atomic change-set (or one per phase if preferred). Commit message: e.g. "Procedural hills landscape: shared core, editor tool, runtime generation wired to Enter the Tribulation."

---

## 8. Algorithm Details (Reference)

- **Height:**  
  `Height = A * (0.65 * N(very_large) + 0.25 * N(large) + 0.10 * N(medium))`  
  then optional river carve, then smoothing. No plateau/clamp.
- **Noise scale:** Pass scale in meters; convert to noise frequency (e.g. wavelength in world units) so "very large" dominates.
- **Smoothing:** Multiple passes; kernel blend by SmoothStrength; avoid sharp edges.

---

## 9. File Checklist (New/Modified)

| File | Action |
|------|--------|
| `Source/T66/T66.Build.cs` | Add Landscape dependency |
| `Source/T66/Gameplay/T66ProceduralLandscapeParams.h` | New: params struct, preset enum |
| `Source/T66/Gameplay/T66ProceduralLandscapeGenerator.h` | New: generator API (noise + height + smooth) |
| `Source/T66/Gameplay/T66ProceduralLandscapeGenerator.cpp` | New: implementation |
| `Source/T66Editor/T66Editor.Build.cs` | Add Landscape, LandscapeEditor (or correct module names) |
| `Source/T66Editor/T66Editor.cpp` / `.h` | Register "T66 Tools" menu and "Generate Procedural Hills Landscape (Dev)" |
| `Source/T66Editor/T66ProceduralLandscapeEditorTool.h` | New: editor tool declaration (or inline in module) |
| `Source/T66Editor/T66ProceduralLandscapeEditorTool.cpp` | New: create/update Landscape, call generator |
| `Source/T66/Core/T66GameInstance.h` | Add ProceduralTerrainSeed (and optional preset) |
| `Source/T66/Gameplay/T66GameMode.cpp` | Terrain generation on GameplayLevel load |
| `Source/T66/Gameplay/T66GameMode.h` | Optional: terrain params or helper ref |
| `Source/T66/UI/Screens/T66HeroSelectionScreen.cpp` | Set ProceduralTerrainSeed before OpenLevel |
| `memory.md` | Note procedural terrain, seed, editor menu |

---

## 10. Risk & Fallback

- **Runtime Landscape creation not available in UE 5.7:** Fallback: GameplayLevel always contains one Landscape actor; at runtime we only *replace* its height data via whatever API exists (e.g. landscape proxy height edit). If no runtime height edit, document and keep editor-only generation; runtime uses a default saved terrain until engine supports it.
- **Packaged build:** `GetLandscapeExtent` and `FHeightmapAccessor` are editor-only. Terrain apply is wrapped in `#if WITH_EDITOR`; in packaged builds `GenerateProceduralTerrainIfNeeded()` does nothing (level uses saved landscape). For shipped builds: run the editor tool on GameplayLevel and save, or accept one fixed terrain per package.
- **Performance:** Generation runs once per level load; keep SmoothPasses and grid size reasonable (Small preset first). If hitches, consider async generation and apply when ready, or reduce resolution for runtime.

End of plan.
