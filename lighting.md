# Chadpocalypse Lighting & Material System

This document describes the complete lighting, material, and visual pipeline for Chadpocalypse's "Dark mode" (Eclipse Dusk), the design decisions behind it, the problems encountered during development, and the rules for importing new assets.

---

## 1. Design Goal

The visual target is inspired by **ULTRAKILL** and **Megabonk**: a retro pixelated 3D aesthetic where every object in the game displays its texture at full brightness from every angle, with no directional shading, shadows, or light-dependent color variation. The sky has a posterized red/orange dusk gradient with an eclipse visual (dark mass + ring of fire), while the game world is clean and uniformly lit.

### Key visual principles

- **No bright side / dark side** on any mesh. A character or wall looks identical regardless of camera angle relative to the light.
- **No bloom glare** on the game world. The eclipse effect stays in the sky.
- **Natural surface colors preserved** — green grass reads as green, not washed red.
- **Posterize banding on the sky only** — ground and characters are clean.
- **Eclipse dusk atmosphere** — the scene reads as "dimmed afternoon during an eclipse," not nighttime.

---

## 2. Why Everything Is Unlit (The Problem That Got Us Here)

### The original approach (and why it failed)

The initial lighting setup was a "blood-red moon night sky" using traditional Lit materials:

1. **Lit materials respond to directional light direction.** Characters had a bright side and a dark side. Rotating around an NPC showed their back completely dark.
2. **Per-character UPointLightComponents were added as a bandaid** — every hero, enemy, and companion had a point light at intensity 5000. With 100+ characters on screen this was expensive, and still produced directional falloff.
3. **Emissive bake was added as another bandaid** — `EmissiveColorMapWeight = 0.16` on all character material instances. This added subtle glow but didn't eliminate directional shading.
4. **Red directional light color washed all surface colors to monochrome red** — you couldn't tell green grass from brown dirt.
5. **The scene was framed as nighttime** when what we actually wanted was an eclipse dusk.

### The solution

**Make every material in the game Unlit.** The base color texture is the final pixel color. No material responds to any light source. The directional light, sky light, and fog exist only to drive the Sky Atmosphere rendering (the red eclipse sky dome). They have zero effect on Unlit materials.

This is exactly what Megabonk does: characters and world geometry are Unlit, the sky has posterized banding, and the world is clean.

---

## 3. Material Architecture

### Master materials

All material instances in the project must inherit from one of these Unlit master materials:

| Master Material | Asset Path | Texture Param | Use For |
|---|---|---|---|
| **M_Character_Unlit** | `/Game/Materials/M_Character_Unlit` | `DiffuseColorMap` | Heroes, enemies, companions, NPCs — any character/creature (FBX imports) |
| **M_Environment_Unlit** | `/Game/Materials/M_Environment_Unlit` | `DiffuseColorMap` | Ground, terrain, structures, ramps, procedural geometry, solid-color surfaces |
| **M_FBX_Unlit** | `/Game/Materials/M_FBX_Unlit` | `DiffuseColorMap` | Alternate character master — mirrors `FBXLegacyPhongSurfaceMaterial` param layout |
| **M_GLB_Unlit** | `/Game/Materials/M_GLB_Unlit` | `BaseColorTexture` | Props and interactables imported via GLB (Interchange uses different param names) |

> **Why two different texture parameter names?** FBX imports create MICs with `DiffuseColorMap`. GLB imports create MICs with `BaseColorTexture`. The master materials must match the parameter name used by the import pipeline so textures carry over during reparenting.

### M_Character_Unlit

- **Shading Model:** Unlit
- **Material Domain:** Surface
- **Two Sided:** true
- **Parameters:**
  - `DiffuseColorMap` (TextureSampleParameter2D) — the baked character texture
  - `Brightness` (Scalar, default 1.0, range 0.5–2.0) — global brightness multiplier
- **Graph:** `DiffuseColorMap.RGB * Brightness` → Emissive Color

### M_Environment_Unlit

- **Shading Model:** Unlit
- **Material Domain:** Surface
- **Two Sided:** true
- **Parameters:**
  - `DiffuseColorMap` (TextureSampleParameter2D) — the base color texture
  - `Brightness` (Scalar, default 1.0, range 0.5–2.0) — global brightness multiplier
  - `Tint` (VectorParameter, default white `1,1,1`) — color tint for solid-color surfaces or reusing textures with different hues
- **Graph:** `DiffuseColorMap.RGB * Brightness * Tint` → Emissive Color

### Ground atlas materials

The procedural map floor uses 4 rotation variants of a ground atlas texture:

| Material | Asset Path | Shading Model |
|---|---|---|
| M_GroundAtlas_2x2_R0 | `/Game/World/Ground/M_GroundAtlas_2x2_R0` | Unlit |
| M_GroundAtlas_2x2_R90 | `/Game/World/Ground/M_GroundAtlas_2x2_R90` | Unlit |
| M_GroundAtlas_2x2_R180 | `/Game/World/Ground/M_GroundAtlas_2x2_R180` | Unlit |
| M_GroundAtlas_2x2_R270 | `/Game/World/Ground/M_GroundAtlas_2x2_R270` | Unlit |

These are generated by `Scripts/ImportGroundAtlas.py`. The script connects the texture to `MP_EMISSIVE_COLOR` (not `MP_BASE_COLOR`) and sets the shading model to `MSM_UNLIT`.

### Procedural map geometry material split

Platform walls and ramps use `UProceduralMeshComponent` with **two mesh sections** for visual variety:

| Section | Material | Used For |
|---|---|---|
| Section 0 | Ground atlas (`FloorMat`) | Platform top faces, ramp slope (walkable surface) |
| Section 1 | Brown side material (MID from `M_Environment_Unlit`, Tint `0.35, 0.2, 0.08`) | Wall vertical faces, wall bottom, ramp back wall, ramp underside, ramp side caps |

The brown side material is created at runtime as a `UMaterialInstanceDynamic` from `M_Environment_Unlit`.

### Runtime Unlit enforcement

`FT66VisualUtil::EnsureAllWorldMeshesUnlit(World)` runs after map generation and iterates all actors in the world. Any mesh component using an `/Engine/` material (e.g., `BasicShapeMaterial`, `DefaultMaterial`) gets replaced with an MID of `M_Environment_Unlit` with a neutral grey tint. This is a safety net for any geometry that slips through the conversion pipeline.

---

## 4. Dark Mode Lighting Values

All values are set programmatically in `T66GameMode.cpp` when Dark theme is active. Light mode remains unchanged.

### Directional Light (Eclipsed Sun)

The "moon" directional light is reframed as the eclipsed sun:

| Property | Value |
|---|---|
| Intensity | 5.0 |
| Light Color | `(1.0, 0.95, 0.9)` — warm white |
| Atmosphere Sun Light | true |
| Atmosphere Sun Light Index | 0 |
| Atmosphere Sun Disk Color Scale | `(0, 0, 0)` — hides built-in sun disk |
| Cast Shadows | false |
| Rotation | `(-50, 135, 0)` — sun 50 degrees above horizon |

The original sun directional light is disabled (intensity 0, atmosphere sun light false).

### Sky Atmosphere

| Property | Value |
|---|---|
| Rayleigh Scattering | `(0.028, 0.005, 0.004)` — red-shifted |
| Rayleigh Scattering Scale | 0.8 |
| Mie Scattering Scale | 0.01 |
| Multi Scattering Factor | 0.5 |

### Sky Light

| Property | Value |
|---|---|
| Source Type | Captured Scene |
| Intensity | 4.0 |
| Light Color | `(0.25, 0.2, 0.25)` — neutral purple tint |
| Lower Hemisphere Is Black | false |
| Lower Hemisphere Color | `(0.12, 0.1, 0.12)` |

### Exponential Height Fog

| Property | Value |
|---|---|
| Fog Inscattering Color | `(0, 0, 0)` — black, inherits tone from atmosphere |
| Volumetric Fog | Off |

### Post Process Volume (Unbound)

| Property | Value |
|---|---|
| Color Saturation | `(1.0, 1.0, 1.0, 1.0)` — fully neutral |
| Color Gamma Override | false (default) |
| Color Gain Override | false (default) |
| Bloom Intensity | 0.0 — completely off |
| Bloom Threshold | 10.0 |

### Posterize Post-Process (`M_PosterizePostProcess`)

| Parameter | Value |
|---|---|
| Steps | 10 — number of discrete color bands |
| Intensity | 0.85 — lerp blend strength |
| SkyDepthThreshold | 500000 — pixels beyond this depth get posterized |

The posterize effect uses a `SceneDepth` mask so it only applies to sky pixels (effectively infinite depth). Ground and characters pass through unaffected.

### Eclipse Actor (`AT66EclipseActor`)

| Property | Value |
|---|---|
| Position | 500,000 units along moon directional light's forward vector |
| Scale | `(8000, 8000, 1)` |
| Material | `M_EclipseCorona` |

Eclipse material (`M_EclipseCorona`) parameters:

| Parameter | Value |
|---|---|
| CoronaIntensity | 3.0 |
| CoronaColor | `(1.0, 0.4, 0.05)` — deep orange |
| InnerRadius | 0.38 — black eclipsing body |
| OuterRadius | 0.45 — thin ring of fire |

The eclipse billboard always faces the camera (oriented per-tick via `OrientTowardCamera()`).

---

## 5. What the Lighting Actually Does

Since all game materials are Unlit, the lighting stack exists **solely to render the sky and atmosphere**:

- **Directional Light** → drives the Sky Atmosphere's sun position and scatter. No Unlit material responds to it.
- **Sky Atmosphere** → renders the red dusk sky dome via Rayleigh/Mie scattering.
- **Sky Light** → captures the sky dome for ambient reflections. Has minimal visual impact since everything is Unlit.
- **Exponential Height Fog** → adds atmospheric depth/haze using atmosphere-derived color.
- **Post Process Volume** → disables bloom and keeps color grading neutral.
- **Posterize** → sky-only depth-masked color banding for the retro look.
- **Eclipse Actor** → visual eclipse effect positioned at the sun disk location.

---

## 6. Problems Encountered and How They Were Solved

### Problem: Characters had bright side / dark side

**Cause:** Lit materials respond to directional light.
**Solution:** Converted all 52+ character material instances to inherit from `M_Character_Unlit` (Unlit shading model). Characters now display their baked textures at full brightness from all angles.

### Problem: Per-character fill lights were expensive and ineffective

**Cause:** `UPointLightComponent` at intensity 5000 on every hero, enemy, and companion. Still produced directional falloff and was GPU-expensive with 100+ characters.
**Solution:** Removed all `UPointLightComponent` fill lights from `AT66HeroBase`, `AT66EnemyBase`, and `AT66CompanionBase`. Unnecessary once materials are Unlit.

### Problem: Red directional light washed all surface colors

**Cause:** Deep crimson light color `(0.8, 0.1, 0.05)` made everything monochrome red.
**Solution:** Changed to warm white `(1.0, 0.95, 0.9)` and decoupled sky color (Rayleigh scattering) from surface lighting color. Since surfaces are now Unlit, the directional light color only affects the sky atmosphere.

### Problem: Platform wall sides were completely black

**Cause:** Walls were built using `UHierarchicalInstancedStaticMeshComponent` (HISM) with the engine's `Cube.Cube` static mesh. The engine's `BasicShapeMaterial` is Lit, and `SetMaterial()` on HISM was not reliably overriding it — material assignments were being ignored or lost.
**Solution:** Replaced HISM with `UProceduralMeshComponent` (PMC) using procedurally generated box geometry (`AppendBoxFace` / `AppendWallBox`). PMC gives full control over material assignment per mesh section with no engine default material interference.

### Problem: Ground atlas materials were still Lit after script runs

**Cause:** `ImportGroundAtlas.py` was connecting texture to `MP_BASE_COLOR` (Lit pipeline) instead of `MP_EMISSIVE_COLOR` (Unlit pipeline).
**Solution:** Updated `ImportGroundAtlas.py` to set `shading_model = MSM_UNLIT`, connect texture to `MP_EMISSIVE_COLOR`, remove `MP_ROUGHNESS` connection, and set `two_sided = True`. Added detailed logging and multiple retry attempts for the shading model change.

### Problem: Interactables and world actors still using engine materials

**Cause:** Actors spawned at runtime using `CreateAndSetMaterialInstanceDynamic(0)` from `BasicShapeMaterial` or other Lit engine materials.
**Solution:** Created `FT66VisualUtil::EnsureAllWorldMeshesUnlit()` which iterates all world actors and replaces any `/Engine/` material with an MID of `M_Environment_Unlit`. This runs after map generation as a safety net. Also explicitly applied `FloorMat` (ground atlas) to boss/start area walls instead of using engine material defaults.

### Problem: Hot-reload crash after code changes

**Cause:** Stale cached data in `HotReloadState.bin` and `UnrealEditor.modules` after class layout changes.
**Solution:** Delete `Intermediate/Build/Win64/x64/UnrealEditor/Development/T66/HotReloadState.bin` and `Binaries/Win64/UnrealEditor.modules`, then do a full rebuild.

### Problem: Eclipse corona bloom washing over the game world

**Cause:** `CoronaIntensity` was 15.0, causing massive bloom bleed.
**Solution:** Reduced `CoronaIntensity` to 3.0, set `BloomIntensity` to 0.0 in the post-process volume, and raised `BloomThreshold` to 10.0.

### Problem: Newly imported FBX characters appeared completely black

**Cause:** UE5's Interchange pipeline created MICs parented to `FBXLegacyPhongSurfaceMaterial` but **did not bind the textures** to the `DiffuseColorMap` parameter. The textures existed as separate `Texture2D` assets in the same directory (`Image_0`, `Image_0_003`, etc.) but the MICs' `DiffuseColorMap` was empty. Combined with the dim eclipse lighting (designed for Unlit materials), a Lit material with no texture rendered as pure black.
**Solution:** Created `Scripts/MakeCharacterMaterialsUnlit.py` with smart texture discovery — when `DiffuseColorMap` is empty, the script searches the same directory for `Texture2D` assets matching the MIC's naming convention (`Material_0_003` → `Image_0_003`), then reparents to `M_Character_Unlit` and sets the texture.

### Problem: GLB-imported props had wrong material (grey/untextured)

**Cause:** GLB imports use a different engine parent (`MI_Default_Opaque` → `M_Default` at `/InterchangeAssets/gltf/M_Default`) and a different texture parameter name (`BaseColorTexture` instead of `DiffuseColorMap`). Early conversion scripts assumed `DiffuseColorMap` for everything, so GLB textures were lost during reparenting. Additionally, `M_Default` is an engine material and cannot be modified in-place.
**Solution:** Created `M_GLB_Unlit` with `BaseColorTexture` as its parameter, and `Scripts/MakeGLBImportsUnlit.py` to reparent GLB MICs specifically.

### Problem: Engine materials cannot be modified

**Cause:** Both `FBXLegacyPhongSurfaceMaterial` (FBX imports) and `M_Default` (GLB imports) are engine-level materials. MICs inherit their shading model from their parent, and there is no per-MIC shading model override. Attempting to modify engine materials either fails silently or breaks on engine updates.
**Solution:** Create project-specific Unlit master materials (`M_Character_Unlit`, `M_GLB_Unlit`) that mirror the engine material's parameter layout but use Unlit shading. Reparent MICs to these project materials instead of modifying the engine materials.

---

## 7. Importing New Characters (FBX)

### The current workflow

1. **Import** — use `Scripts/ImportSkeletalMeshes.py`
2. **Auto-convert** — the importer immediately runs `Scripts/MakeCharacterMaterialsUnlit.py` logic on the imported destination folder

The helper script still exists as a repair/re-run tool, but the normal workflow is now integrated into the importer.

### What happens during FBX import

UE5's Interchange pipeline creates:
- A `SkeletalMesh` asset
- `MaterialInstanceConstant` (MIC) assets named like `Material_0`, `Material_0_003`, etc.
- `Texture2D` assets named like `Image_0`, `Image_0_003`, etc.

The MICs are parented to `FBXLegacyPhongSurfaceMaterial` — an **engine material** that is Lit. You cannot modify engine materials, so the MICs must be reparented to a project Unlit material.

### The texture binding problem

The Interchange pipeline sometimes **fails to bind textures** to the MICs it creates. The `DiffuseColorMap` parameter on the MIC is left empty, even though the corresponding texture assets exist in the same directory. This causes the character to render completely black (no texture + dim eclipse lighting = black).

### How MakeCharacterMaterialsUnlit.py solves this

The script has smart texture discovery:

1. Scans all MICs under `/Game/Characters/` (Heroes, Companions, Enemies, NPCs)
2. Skips anything already parented to `M_Character_Unlit` or `M_FBX_Unlit`
3. For MICs parented to `FBXLegacyPhongSurfaceMaterial`:
   - **First** tries reading `DiffuseColorMap` from the MIC (works for imports that bound textures)
   - **If empty**, searches the same directory for `Texture2D` assets and matches by naming convention: `Material_0_003` → `Image_0_003`
   - **Fallback**: if only one texture exists in the directory, uses that
4. Reparents to `M_Character_Unlit`
5. Sets `DiffuseColorMap` to the found texture
6. Saves

Safe to re-run any time. Use it when an already-imported character folder needs repair or a manual import bypassed the project importer.

### Step-by-step for importing a new character

1. Place your `.fbx` file(s) in `SourceAssets/Import/`
2. Edit `Scripts/ImportSkeletalMeshes.py` — update the `IMPORTS` list with your new entry
3. Run `ImportSkeletalMeshes.py` in the editor
4. Verify the mesh imported correctly (check Content Browser for the assets)
5. Check the import log — the unlit pass should run automatically and report converted/already-ok counts
6. If the visual is data-driven, update `Content/Data/CharacterVisuals.csv`
7. Run `Scripts/SetupCharacterVisualsDataTable.py`
8. Play the game and verify the character displays textures correctly from all angles

---

## 8. Importing New Props / Interactables (GLB)

### The workflow

Props and interactables imported via GLB use a different material pipeline:
- Parent material: `MI_Default_Opaque` → `M_Default` (engine material at `/InterchangeAssets/gltf/M_Default`)
- Texture parameter: `BaseColorTexture` (not `DiffuseColorMap`)

`Scripts/ImportStaticMeshes.py` now imports the GLB and immediately runs the GLB unlit conversion on that import root. The repair script is still available if needed. The GLB conversion pass:
1. Creates or safely recreates `M_GLB_Unlit` if it doesn't exist or is missing required params/flags (Unlit, `BaseColorTexture * Tint * Brightness` → Emissive Color, `used_with_instanced_static_meshes = true`, `used_with_nanite = true`)
2. Finds all MICs parented to `MI_Default_Opaque` or `M_Default`
3. Reads `BaseColorTexture` before reparenting, reparents to `M_GLB_Unlit`, re-sets it after
4. Re-applies the gameplay static-mesh build policy for GLB assets: full-precision UVs, no generated lightmap UVs, preserve imported normals/tangents
5. Optionally applies per-entry material overrides (`Brightness`, `Tint`) and texture-parameter overrides from the import manifest
5. Saves

### Why two separate scripts?

FBX and GLB imports use different parent materials and different texture parameter names. The project keeps two dedicated conversion helpers, but the importers call the correct one automatically:

| Import Format | Parent Material (engine) | Texture Param | Conversion Script |
|---|---|---|---|
| FBX (characters) | `FBXLegacyPhongSurfaceMaterial` | `DiffuseColorMap` | `ImportSkeletalMeshes.py` → `MakeCharacterMaterialsUnlit.py` |
| GLB (props/interactables) | `MI_Default_Opaque` / `M_Default` | `BaseColorTexture` | `ImportStaticMeshes.py` → `MakeGLBImportsUnlit.py` |

---

## 9. Rules

- **Never** use `DefaultLit`, `Lit`, or any other shading model
- **Never** leave `FBXLegacyPhongSurfaceMaterial` or `M_Default` as a parent material on shipped assets
- **Never** add `UPointLightComponent` fill lights to characters
- **Never** set `EmissiveColorMapWeight` as a lighting workaround
- **Never** try to modify engine materials in-place — they're read-only and changes break on engine updates
- If a material needs to be visible from both sides, ensure `Two Sided = true` (all master materials have this enabled by default)
- Always use the project importers so the unlit pass happens during import
- If an asset was imported manually or before this workflow existed, run the matching repair script on that folder
- If a GLB atlas smears colors at gameplay distance, fix it in the importer manifest with a texture-parameter override (for example `BaseColorTexture` → `TMGS_NO_MIPMAPS`) so the repair path can reproduce it

### Creating materials from scratch in C++

If you need a new solid-color material at runtime (e.g., for a procedurally spawned prop):

```cpp
UMaterialInterface* EnvUnlit = LoadObject<UMaterialInterface>(
    nullptr, TEXT("/Game/Materials/M_Environment_Unlit.M_Environment_Unlit"));
UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(EnvUnlit, this);
MID->SetVectorParameterValue(FName("Tint"), FLinearColor(0.35f, 0.2f, 0.08f));
MeshComponent->SetMaterial(0, MID);
```

---

## 10. Relevant Source Files

### C++ source

| File | Role |
|---|---|
| `Source/T66/Gameplay/T66GameMode.cpp` | All Dark/Light theme lighting values, procedural map generation (platform tops, walls, ramps), ground atlas material loading |
| `Source/T66/Gameplay/T66VisualUtil.cpp` | `EnsureAllWorldMeshesUnlit()` (log-only safety net), `ApplyT66Color()`, `EnsureUnlitMaterials()` |
| `Source/T66/Gameplay/T66EclipseActor.cpp` | Eclipse billboard positioning, camera orientation, corona material parameter control |

### Python scripts (run in-editor via Tools > Execute Python Script)

| Script | Purpose |
|---|---|
| `Scripts/ImportSkeletalMeshes.py` | FBX importer for characters. Imports assets and immediately runs the FBX unlit conversion for that destination folder. |
| `Scripts/MakeCharacterMaterialsUnlit.py` | Repair/re-run tool for FBX character materials already in the project. |
| `Scripts/MakeGLBImportsUnlit.py` | Repair/re-run tool for GLB prop/interactable materials already in the project. |
| `Scripts/ImportGroundAtlas.py` | Generates the 4 Unlit ground atlas materials from source PNG |
| `Scripts/ImportStaticMeshes.py` | GLB importer for props/interactables. Imports, flattens Interchange output, and immediately runs the GLB unlit conversion. |
| `Scripts/FlattenInterchangeAssets.py` | Flattens nested Interchange directory structure after import |

---

## 11. Quick Reference Cheat Sheet

**"My new FBX character is completely black"** → First import through `Scripts/ImportSkeletalMeshes.py`. If the asset already exists or was imported manually, run `Scripts/MakeCharacterMaterialsUnlit.py`. The usual root cause is the character master losing `used_with_skeletal_mesh = true`; the repair script now re-enforces that flag.

**"My new GLB prop is grey/untextured"** → First import through `Scripts/ImportStaticMeshes.py`. If the asset already exists or was imported manually, run `Scripts/MakeGLBImportsUnlit.py`.

**"My new character model has dark sides"** → Same fix path: use the project importer or re-run `MakeCharacterMaterialsUnlit.py` on the existing folder.

**"My new prop has dark faces or muddy atlas colors"** → Use the GLB importer or re-run `MakeGLBImportsUnlit.py` for GLB imports. If the source atlas smears at distance, add a texture-parameter override in `ImportStaticMeshes.py` (for example `BaseColorTexture` no-mips) so the fix survives reimport/repair.

**"My GLB looks correct in Blender but muddy in Unreal"** → First suspect stale Unreal import artifacts before blaming the source file. A clean raw GLB import may be correct while an existing gameplay destination still points at old nested textures/materials from a previous import. The project fix is to run `ImportStaticMeshes.py`, which now deletes the old GLB subtree before reimporting. After that, if fidelity is still off, check the GLB mesh build settings path: gameplay GLBs preserve source mesh data with full-precision UVs and disabled generated lightmap UVs, and those writes must run through `UnrealEditor.exe -ExecutePythonScript=...`; `UnrealEditor-Cmd.exe` does not reliably expose `StaticMeshEditorSubsystem` for them.

**"A procedural mesh is grey/dark"** → Make sure you're calling `SetMaterial()` with either `FloorMat` or an MID of `M_Environment_Unlit`, not using `CreateAndSetMaterialInstanceDynamic(0)` from an engine material.

**"The sky posterization stopped working"** → Check that `UT66PosterizeSubsystem::SetEnabled(true)` is being called for Dark theme in `ApplyThemeToAtmosphereAndLightingForWorld`.

**"Everything has bloom glare"** → Verify `BloomIntensity = 0.0` in the post-process volume Dark mode block.

**"The eclipse is too bright/washed out"** → Reduce `CoronaIntensity` on the eclipse material (currently 3.0).

**"The editor crashes on open after code changes"** → Delete `HotReloadState.bin` and `UnrealEditor.modules` from `Intermediate/Build/Win64/` and `Binaries/Win64/` respectively, then rebuild.
