# Preview Stage Day/Night Environment — Handoff Notes

**Date:** 2026-02-07  
**Branch:** `version-0.5`  
**Goal:** Make hero/companion preview areas look like the character is standing on a grass field with day/night lighting that responds to the Dark/Light theme toggle.

---

## What was done (C++ — compiles and works)

### Files modified

| File | What changed |
|------|-------------|
| `Source/T66/Gameplay/T66HeroPreviewStage.h` | Added `BackdropSphere`, `AmbientSkyLight`, `GroundMID`, `BackdropMID` members. Added `ApplyThemeLighting()`, `OnThemeChanged()`, `EndPlay()`. Forward-declared `UMaterialInstanceDynamic`. |
| `Source/T66/Gameplay/T66HeroPreviewStage.cpp` | Constructor: creates inverted sphere (sky dome) + ambient point light. BeginPlay: loads custom materials from `/Game/UI/Preview/`, creates MIDs, binds to `OnSettingsChanged` delegate. `ApplyThemeLighting()`: sets light colors/intensities and material parameters for day vs night. EndPlay: unbinds delegate. FrameCameraToPreview: positions backdrop sphere and ambient light at orbit center; widened ground disc. |
| `Source/T66/Gameplay/T66CompanionPreviewStage.h` | Same additions as hero (uses `FloorMID` instead of `GroundMID`, `PreviewFloor` instead of `PreviewPlatform`). |
| `Source/T66/Gameplay/T66CompanionPreviewStage.cpp` | Same implementation pattern as hero. |

### File created

| File | Purpose |
|------|---------|
| `Scripts/CreatePreviewMaterials.py` | UE Python script to create `M_PreviewGround` and `M_PreviewSky` material assets in `/Game/UI/Preview/`. |

### Architecture summary

1. **Sky dome:** A `UStaticMeshComponent` using `/Engine/BasicShapes/Sphere.Sphere` at scale `FVector(-50, -50, -50)` — negative scale on all 3 axes inverts the face winding so the camera (inside the sphere) sees the interior faces. Positioned at the orbit center of the character.

2. **Ambient sky light:** A `UPointLightComponent` with large attenuation radius (15000) centered inside the sky dome. Provides uniform backdrop illumination and atmospheric color.

3. **Ground disc:** The existing cylinder platform, widened (scale multiplier 2.5x, Z thickness 0.04) to look more like a field.

4. **Theme reactivity:** Both stages bind to `UT66PlayerSettingsSubsystem::OnSettingsChanged` (dynamic multicast delegate). When the Dark/Light toggle fires, `OnThemeChanged()` → `ApplyThemeLighting()` updates all 4 lights + material parameters. Delegate is unbound in `EndPlay()`.

5. **Material loading (BeginPlay):** Tries `LoadObject<UMaterialInterface>` for `/Game/UI/Preview/M_PreviewGround` and `/Game/UI/Preview/M_PreviewSky`. If found, creates `UMaterialInstanceDynamic` from them and applies to the mesh. If not found (script not yet run), falls back to `CreateDynamicMaterialInstance(0)` from the default engine material.

6. **Day (Light theme) settings:**
   - Key light: warm white (1.0, 0.95, 0.85), intensity 8000
   - Fill: warm (0.95, 0.90, 0.80), intensity 3000
   - Rim: white, intensity 3000
   - Ambient sky: blue (0.55, 0.75, 1.0), intensity 4000
   - Ground MID: bright grass green BaseColor + VariationColor
   - Sky MID: SkyColor blue, HorizonColor light blue, DetailColor white (clouds), DetailIntensity 0.25, DetailScale 4, Sharpness 1.5

7. **Night (Dark theme) settings:**
   - Key light: cool moonlight (0.6, 0.7, 0.95), intensity 3500
   - Fill: deep blue (0.45, 0.55, 0.80), intensity 1200
   - Rim: silver-blue (0.7, 0.75, 0.95), intensity 1800
   - Ambient sky: dark navy (0.12, 0.15, 0.30), intensity 1500
   - Ground MID: dark grass green
   - Sky MID: SkyColor dark navy, HorizonColor slightly lighter, DetailColor warm white (stars), DetailIntensity 1.5, DetailScale 15, Sharpness 10

---

## What is NOT working — the crash

### Problem

The Python script `Scripts/CreatePreviewMaterials.py` crashes the Unreal Editor when executed. Without this script, the material assets (`M_PreviewGround`, `M_PreviewSky`) don't exist in `/Game/UI/Preview/`, so the C++ falls back to the default engine `BasicShapeMaterial` — which **does not have parameterized BaseColor/Color** parameters. This means `SetVectorParameterValue()` calls are no-ops, and the ground stays grey checkerboard and the sky dome stays dark/invisible.

### Likely crash causes

1. **`MaterialExpressionNoise`** — The first version of the script used `unreal.MaterialExpressionNoise`. This expression class may not be exposed to the Python API in UE 5.7, or may require specific rendering subsystems that aren't available in `-run=pythonscript` commandlet mode. Creating an expression with an invalid/unavailable class can crash.

2. **`set_editor_property("shading_model", MSM_UNLIT)`** — In UE 5.1+, the `ShadingModel` property on `UMaterial` was replaced by `ShadingModels` (plural, `FMaterialShadingModelField`). Setting the old singular property may crash in UE 5.7. The rewritten script avoids this entirely by using a **black BaseColor + EmissiveColor trick** (keeps DefaultLit shading model, but EmissiveColor provides 100% of the visible color since BaseColor is black).

3. **`MaterialExpressionPower` / `MaterialExpressionScalarParameter`** — Used in the noise chain; may also have Python API issues.

4. **`mel.connect_material_expressions()` with wrong pin names** — Pin name mismatches (e.g., "Position" vs "" for Noise input) can crash in some UE versions.

### Current state of the script (v2 — simplified)

The rewritten script (`Scripts/CreatePreviewMaterials.py`) is much simpler:

- **M_PreviewGround:** Just a `VectorParameter "BaseColor"` → BaseColor + `Constant 0.85` → Roughness. No noise. Solid green.
- **M_PreviewSky:** `TextureCoordinate` → `ComponentMask(G)` → vertical gradient alpha. `Lerp(HorizonColor, SkyColor, gradient)` → EmissiveColor. Black BaseColor + Roughness 1.0. Extra parameters (DetailColor, DetailIntensity, DetailScale, Sharpness) exist as disconnected nodes for future C++ use.
- Every expression creation and connection is wrapped in `try/except` with logging.
- No Noise nodes, no Power nodes, no shading model changes.

**This v2 script has NOT been tested yet.** It may still crash if there are other UE 5.7 Python API issues. See "How to debug" below.

---

## How to run the script

**In-editor (safest):**
1. Open the UE editor with the T66 project
2. Go to **Tools > Execute Python Script**
3. Browse to `Scripts/CreatePreviewMaterials.py`
4. Check the Output Log for `[PreviewMaterials]` messages

**Command line (PowerShell):**
```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" `
    "C:\UE\T66\T66.uproject" `
    -run=pythonscript -script="C:\UE\T66\Scripts\CreatePreviewMaterials.py" `
    -unattended -nop4 -nosplash
```

> **Important:** Do NOT add `-nullrhi` — material compilation requires rendering.

---

## How to debug if it still crashes

1. **Run in-editor** (not commandlet) so you can see the Output Log in real time.

2. **Check the crash log** at `C:\UE\T66\Saved\Crashes\` or `%LOCALAPPDATA%\UnrealEngine\5.7\Saved\Crashes\`.

3. **Isolate which function crashes** — comment out `create_sky_material()` in `main()` and run only `create_ground_material()`. If ground works, the sky creation is the issue. Then comment out individual node blocks in the sky function.

4. **Test minimal material creation** — try the absolute minimum script to see if the Python material API works at all:
   ```python
   import unreal
   mel = unreal.MaterialEditingLibrary
   at = unreal.AssetToolsHelpers.get_asset_tools()
   mat = at.create_asset("M_Test", "/Game/UI/Preview", unreal.Material, unreal.MaterialFactoryNew())
   if mat:
       p = mel.create_material_expression(mat, unreal.MaterialExpressionVectorParameter, 0, 0)
       unreal.log(f"Expression: {p}")
       mel.recompile_material(mat)
       unreal.EditorAssetLibrary.save_asset("/Game/UI/Preview/M_Test")
       unreal.log("SUCCESS")
   ```

5. **Alternative if Python material creation is completely broken in UE 5.7:**
   - Create `M_PreviewGround` and `M_PreviewSky` **manually in the Material Editor** with the parameters described above
   - Or create them as `MaterialInstanceConstant` from a parent material that has parameters
   - The C++ code will pick them up automatically from `/Game/UI/Preview/`

---

## Alternative approach if Python never works

If the UE 5.7 Python `MaterialEditingLibrary` is fundamentally broken, create the materials by hand in the editor:

### M_PreviewGround (manually)
1. Right-click in Content Browser → Material → name it `M_PreviewGround`, save to `/Game/UI/Preview/`
2. Add a **VectorParameter** node, name it `BaseColor`, default value `(0.15, 0.45, 0.08, 1.0)` (green)
3. Connect it to **Base Color**
4. Add a **Constant** node, value `0.85`, connect to **Roughness**
5. Optionally add another **VectorParameter** named `VariationColor` (not connected, just for the param to exist)
6. Save

### M_PreviewSky (manually)
1. Right-click → Material → name it `M_PreviewSky`, save to `/Game/UI/Preview/`
2. Add a **Constant3Vector** `(0,0,0)` → connect to **Base Color**
3. Add a **Constant** `1.0` → connect to **Roughness**
4. Add a **TextureCoordinate** node
5. Add a **ComponentMask** node (enable only G/green) — connect TexCoord → ComponentMask
6. Add **VectorParameter** `HorizonColor` default `(0.65, 0.78, 0.95)`
7. Add **VectorParameter** `SkyColor` default `(0.35, 0.55, 0.90)`
8. Add a **Lerp** node — A=HorizonColor, B=SkyColor, Alpha=ComponentMask output
9. Connect Lerp → **Emissive Color**
10. Optionally add disconnected params: **VectorParameter** `DetailColor`, **ScalarParameter** `DetailIntensity`, `DetailScale`, `Sharpness`
11. Save

Once either material exists at the right path, the C++ code picks it up automatically on next PIE.

---

## Build status

- **C++ compiles:** Yes (ValidateFast passed, 0 errors)
- **Materials exist:** No (script hasn't run successfully yet)
- **Visual result without materials:** Lighting changes DO work (day/night light colors respond to theme toggle). Ground and sky dome stay default grey because the fallback MID can't color the engine's `BasicShapeMaterial`.

---

## Files to review

- `Source/T66/Gameplay/T66HeroPreviewStage.h` + `.cpp`
- `Source/T66/Gameplay/T66CompanionPreviewStage.h` + `.cpp`
- `Scripts/CreatePreviewMaterials.py`
- This file: `T66_Preview_Stage_Handoff.md`
