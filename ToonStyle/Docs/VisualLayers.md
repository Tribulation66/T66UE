# T66 Visual Layers

Purpose: this document is the technical map of the T66 visual stack. New agents should read this before changing ToonStyle materials, imports, stage atmosphere, difficulty tinting, lighting, outlines, or Retro FX.

This is not a mood-board overview. It names the actual T66 layers, source files, runtime entry points, asset roots, material masters, parameters, and application rules used by the current implementation.

The asset root is `/Game/ToonStyle/` in Unreal, backed by `Content/ToonStyle/` on disk. The process/doc/shader root is `ToonStyle/` on disk.

## Core Rule

Playable heroes keep their authored identity materials. Difficulty atmosphere must not recolor playable hero body or outline materials.

Difficulty atmosphere is allowed to affect the world, tower surfaces, enemies, traps, props, fog, post-process grading, and other stage-owned visuals. If a future pass wants companions to inherit difficulty atmosphere, Pablo should approve that explicitly; by default companions should be treated like hero-identity characters.

## Layer Order

| Order | Layer | Owner | Applies to | Must not apply to |
| --- | --- | --- | --- | --- |
| 1 | Source/import foundation | Pixal3D + Blender ToonStyle pipeline | Character/prop source meshes and generated textures before Unreal import | Runtime-only tuning |
| 2 | Toon material masters | `/Game/ToonStyle/Materials` + `ToonStyle/Shaders` | Toon character, environment, and outline material behavior | Stage-specific art direction policy |
| 3 | Character identity layer | Character visual data + imported MIs | Playable heroes, companions, hero-selection/test-room review models | Difficulty atmosphere recolor |
| 4 | Inverted-hull outline layer | `_Outline` sidecar meshes + `M_Toon_Character_Outline` | ToonStyle characters that have sidecar outline meshes | Retro FX post-process outline |
| 5 | Environment surface layer | `/Game/ToonStyle/Environment/<Theme>/Materials` | Tower walls, floors, ceilings, shell walls, doorway headers | Hero body materials |
| 6 | Difficulty Atmosphere Layer | `FT66ThemeAtmosphereSpec` runtime application | Stage world mood, environment ramp colors, enemy/trap/prop theme color where registered | Playable hero identity materials |
| 7 | Local data tint layer | Gameplay data rows and local dynamic materials | Specific props, vehicles, traps, VFX, HUD markers | Global stage color grading policy |
| 8 | Runtime lighting/fog/post-process layer | World visual setup + tower lighting | Sky light, fog, torches, ambient cubemap, color grading | Hero material ramp recolor unless explicitly routed |
| 9 | Retro FX layer | `UT66RetroFXSubsystem` + settings UI | Optional late overlay: pixelation, PS1/N64 passes, UI chrome treatment | Default ToonStyle review path |
| 10 | UI/HUD presentation layer | Native Slate/WBP UI code | HUD, menus, minimap, panels, typography | World atmosphere and Toon material ramps |

## Layer Details

### 1. Source/Import Foundation

Technical name: ToonStyle Pixal3D production import foundation.

This is the offline asset-generation layer. It runs before Unreal runtime and produces the mesh, texture, material-instance, vertex-color, and outline-sidecar inputs consumed by gameplay.

Primary docs:

- `ToonStyle/Docs/PipelineSpec.md`
- `Model Generation/Instructions/09_PIXAL3D_TOONSTYLE_PRODUCTION_IMPORT_INSTRUCTIONS.md`

Current responsibilities:

- Pixal3D production source generation through `Model Generation/Pixal3D/Scripts/run_pixal3d_toonstyle_production_import.py`.
- Blender ToonStyle processing through `ToonStyle/BlenderScripts/run_toon_pipeline.py`.
- Texture post-process through `ToonStyle/BlenderScripts/texture_postprocess.py`: K-means flatten, highlight cap, speckle cleanup, UV padding, and Tint generation.
- Vertex color authoring through `ToonStyle/BlenderScripts/author_vertex_colors.py`.
- Outline sidecar creation through the duplicate/reverse-winding path in `run_toon_pipeline.py`.
- Close-the-gap B-channel authoring through `ToonStyle/BlenderScripts/curvature_analysis.py`.
- Inner-line bake through `ToonStyle/BlenderScripts/inner_line_bake.py`.
- Humanoid face/head normal transfer is deprecated and off in production. It remains only behind the explicit deprecated flag documented in `PipelineSpec.md` for a future revisit pass.

Runtime outputs:

- Shading mesh: `SM_<AssetName>`.
- Outline mesh: `SM_<AssetName>_Outline`.
- Base color texture: flattened/padded production texture.
- Tint texture: HSV-derived darker color-control texture.
- Inner-line texture: high-contrast line mask.
- Toon material instance: `MI_<AssetName>`.
- Outline material instance: `MI_<AssetName>_Outline`.
- Manifest/readback JSON used for validation and traceability.

This layer should not make stage/difficulty art-direction decisions. It prepares assets so the runtime material stack can render them consistently.

### 2. Toon Material Masters

Technical name: ToonStyle master material and shader layer.

This is the runtime shader/material behavior layer. It owns shader math, master material parameters, and generator-authored Unreal material assets.

Primary docs:

- `ToonStyle/Docs/MaterialSpec.md`
- `ToonStyle/Shaders/Public/ToonShadingCommon.ush`
- `ToonStyle/Source/SetupPhase1CToonMaterials.py`

Current masters:

- `/Game/ToonStyle/Materials/M_Toon_Character`
- `/Game/ToonStyle/Materials/M_Toon_Environment`
- `/Game/ToonStyle/Materials/M_Toon_Character_Outline`

This layer owns shader math and exposed parameters. It does not decide which stage theme is active.

Generation entry point:

- `ToonStyle/Source/SetupPhase1CToonMaterials.py`

Shader entry point:

- `ToonStyle/Shaders/Public/ToonShadingCommon.ush`

Key character parameters:

- `BaseColorTexture`
- `TintTexture`
- `InnerLineTexture`
- `InnerLineColor`
- `InnerLineStrength`
- `LightDirection`
- `RampStep1`
- `RampStep2`
- `ShadeColor`
- `MidtoneColor`
- `LitColor`
- `RimColor`
- `RimPower`
- `RimStrength`

Key environment parameters:

- `BaseColorTexture`
- `UVTileU`
- `UVTileV`
- `bUseWorldSpaceUVs`
- `ProjectionAxes`
- `WorldSpaceTileSize`
- `LightDirection`
- `RampStep1`
- `RampStep2`
- `EnvShadeColor`
- `EnvMidtoneColor`
- `EnvLitColor`

Key outline parameters:

- `OutlineColor`
- `OutlineBaseWidth`
- `OutlineReferenceDistance`
- `OutlineFOVTanHalf`
- `OutlineReferenceFOVTanHalf`
- `OutlineDepthOffsetScalar`
- `OutlineWidth` legacy compatibility alias

### 3. Character Identity Layer

Technical name: playable character visual identity layer.

Playable heroes and companion-style identity characters should read from their authored textures, Tint textures, inner-line masks, and neutral ToonStyle material defaults.

Policy:

- Heroes should stay visually identifiable across Dungeon, Forest, Ocean, Martian, and Hell.
- Difficulty atmosphere must not overwrite hero `ShadeColor`, `MidtoneColor`, `LitColor`, `RimColor`, or outline color in live gameplay.
- Hero material changes should be made through the import/material pipeline or a hero-specific visual tuning pass, not through stage difficulty.

Relevant code/data:

- `Source/T66/Core/T66CharacterVisualSubsystem.cpp`
- `Source/T66/Data/T66DataTypes.h` field `OutlineStaticMesh`
- `Content/Data/CharacterVisuals.csv`

Technical inputs:

- `StaticMesh`
- `SkeletalMesh`
- `OutlineStaticMesh`
- `PixelatedTextureAssetPath` for older QuadRetro/static visual paths
- Material slots created or rebuilt by `UT66CharacterVisualSubsystem`

Technical rule:

- Character identity materials can use `M_Toon_Character`, `M_Toon_Character_Outline`, and imported per-asset MIs.
- Live playable hero material instances should not be registered as `ET66ToonMaterialKind::Character` for difficulty-atmosphere recoloring.
- If a hero needs a temporary combat/status color, implement it as an explicit status-effect/material overlay path, not as the Difficulty Atmosphere Layer.

Current implementation note:

- `FT66WorldVisualSetup::RegisterToonMaterial()` can technically register `Character`, `Outline`, or `Environment` material kinds.
- Live gameplay should not register playable hero materials into the Difficulty Atmosphere Layer. Test-room review code may register review models for controlled diagnostics, but that is not the live gameplay policy.

### 4. Inverted-Hull Outline Layer

Technical name: ToonStyle inverted-hull sidecar outline layer.

T66's live character outline path is the imported `_Outline` sidecar mesh plus `M_Toon_Character_Outline`.

Policy:

- Use sidecar outline meshes for ToonStyle characters.
- Keep Retro FX character outline disabled unless Pablo explicitly reopens it near end-of-development.
- Outline tuning belongs to `M_Toon_Character_Outline`, sidecar vertex colors, and the character visual application path.

Relevant references:

- `ToonStyle/Docs/MaterialSpec.md`, `M_Toon_Character_Outline`
- `Source/T66/Data/T66DataTypes.h`, `OutlineStaticMesh`
- `Config/DefaultEngine.ini`, `r.AntiAliasingMethod=1` for FXAA

Technical implementation:

- The Blender pipeline creates `SM_<AssetName>_Outline` by duplicating the shading mesh, reversing winding, and preserving outward custom normals.
- `M_Toon_Character_Outline` is one-sided opaque and renders the inverted-hull shell through WPO.
- Vertex color channels:
  - R: lighting threshold offset for character material.
  - G: outline width multiplier/mask.
  - B: outline depth offset amount.
  - A: unused in production.
- The live runtime data seam is `FCharacterVisualRow::OutlineStaticMesh`.
- FXAA is enabled through `Config/DefaultEngine.ini` with `r.AntiAliasingMethod=1`.

### 5. Environment Surface Layer

Technical name: ToonStyle environment rectangle surface layer.

This is the tower wall/floor/ceiling visual layer.

Current runtime route:

- Difficulty resolves to a tower theme in `Source/T66/Gameplay/T66TowerMapTerrain.cpp`.
- `Source/T66/Gameplay/T66TowerThemeVisuals.cpp` resolves materials under `/Game/ToonStyle/Environment/<Theme>/Materials`.
- `Source/T66/Gameplay/T66TowerMapTerrain.cpp` spawns large cube rectangles for walls, floors, ceilings, shell walls, and doorway headers.

Current content route:

- `/Game/ToonStyle/Environment/<Theme>/Materials/MI_<Theme>_Wall_XZ`
- `/Game/ToonStyle/Environment/<Theme>/Materials/MI_<Theme>_Wall_YZ`
- `/Game/ToonStyle/Environment/<Theme>/Materials/MI_<Theme>_Floor`
- `/Game/ToonStyle/Environment/<Theme>/Materials/MI_<Theme>_Ceiling`

Current texture policy:

- All themes intentionally reuse the existing test-room wall/floor textures for now.
- The per-theme material paths remain production paths so later texture replacement does not require map-spawn code changes.

Technical implementation:

- Geometry primitive: `/Engine/BasicShapes/Cube.Cube`.
- Spawn helper: `T66SpawnEnvironmentRectangle()` in `Source/T66/Gameplay/T66TowerMapTerrain.cpp`.
- Material resolver: `T66TowerThemeVisuals::ResolveEnvironmentSurfaceMaterial()`.
- Surface enum: `T66TowerThemeVisuals::EEnvironmentSurfaceType`.
- Wall orientation:
  - `Wall_XZ` uses `ProjectionAxes=(1,0,1,0)`.
  - `Wall_YZ` uses `ProjectionAxes=(0,1,1,0)`.
- Floor/ceiling projection uses `ProjectionAxes=(1,1,0,0)`.
- Current world-space scale: `WorldSpaceTileSize=300.0`.
- The current live rectangle path deliberately makes the 40-module `CoherentThemeKit01` model output obsolete for wall/floor/ceiling rendering.

### 6. Difficulty Atmosphere Layer

Technical name: `FT66ThemeAtmosphereSpec` runtime layer.

This is the stage mood layer. Use this exact name in future prompts and reports.

Owner:

- `Source/T66/Gameplay/T66ThemeAtmosphereData.cpp`
- `Source/T66/Gameplay/T66ThemeAtmosphereData.h`
- `Source/T66/Gameplay/T66WorldVisualSetup.cpp`

Difficulty-to-theme route:

- Easy -> Dungeon
- Medium -> Forest
- Hard -> Ocean
- VeryHard -> Martian
- Impossible -> Hell

Current responsibilities:

- Theme cel parameters: `LightDirection`, ramp steps, shade/midtone/lit colors, rim color, outline color, and environment ramp colors.
- World setup: sky light, fog, ambient cubemap, post-process grading, and hero carry light data.
- Applies runtime Toon cel parameters to registered Toon materials.

Technical application:

- `AT66GameMode::BeginPlay`/world setup calls `FT66WorldVisualSetup::EnsureAtmosphereForWorld()`.
- `EnsureAtmosphereForWorld()` resolves `T66ThemeAtmosphereData::GetSpecForTheme(Theme)`.
- It applies:
  - `T66ApplyAtmosphereSkyLight()`
  - `T66ApplyAtmosphereFog()`
  - `T66ApplyThemePostProcess()`
  - `ApplyAtmosphereToHeroCarryLights()`
  - `ApplyToonCelAtmosphereToRegisteredMaterials()`
- `ApplyToonCelAtmosphereToRegisteredMaterials()` writes parameters into registered dynamic material instances.

Registered material kinds:

- `ET66ToonMaterialKind::Environment`
- `ET66ToonMaterialKind::Character`
- `ET66ToonMaterialKind::Outline`

Policy constraint:

- `Environment` registration is valid for tower/world surfaces.
- `Character`/`Outline` registration is valid for test-room diagnostics and non-hero stage-owned actors only.
- Live playable heroes should not receive Difficulty Atmosphere Layer `Character` or `Outline` parameter writes.

Application policy:

- Apply to environment materials.
- Apply to enemies, mobs, bosses, traps, props, and VFX only when those assets are intended to belong to the stage mood.
- Do not apply to playable hero body materials or playable hero outline sidecars.
- Do not use this layer as a shortcut for hero readability tuning.

Diagnostic example:

- If the user sees a red/orange wash while difficulty is Impossible, first inspect this layer. Hell currently defines red/orange character and environment ramp colors in `T66ThemeAtmosphereData.cpp`.

### 7. Local Data Tint Layer

Technical name: actor/data-row tint and visual-row selection layer.

This is not the same as Difficulty Atmosphere.

Purpose:

- Per-actor or per-row visual color choices: prop tint, vehicle tint, trap tint, projectile tint, ring color, VFX color.
- Difficulty-specific data row lookup where gameplay assets intentionally swap rows by selected difficulty.

Examples:

- `Source/T66/Gameplay/T66WorldVisualProp.cpp` builds difficulty-specific visual prop row IDs.
- `Source/T66/Gameplay/T66VehicleInteractable.cpp` does the same for vehicles.
- Trap and projectile tints live in trap tuning/runtime code.

Technical signals:

- `Tint`
- `BaseColor`
- `DiffuseColorMap`
- `BaseColorTexture`
- row IDs with difficulty suffixes such as `_Easy`, `_Medium`, `_Hard`, `_VeryHard`, `_Impossible`
- per-actor material instances created locally with `UMaterialInstanceDynamic::Create()` or `CreateAndSetMaterialInstanceDynamic()`

Policy:

- Use this layer for a specific object or effect.
- Do not use it as a global mood pass.
- Do not tint hero identity assets through this layer unless the design explicitly calls for a temporary gameplay status effect.

### 8. Runtime Lighting/Fog/Post-Process Layer

Technical name: world atmosphere actor layer.

This layer creates the actual world atmosphere actors and post-process state.

Owner:

- `Source/T66/Gameplay/T66WorldVisualSetup.cpp`
- `Source/T66/Gameplay/T66TowerLighting.cpp`

Current responsibilities:

- Neutral setup cleanup.
- Theme sky light.
- Exponential height fog.
- Theme post-process volume.
- Ambient cubemap.
- Torch light spawning on tower floors.
- Optional carry-light setup.

Technical actors/components:

- `ASkyLight`
- `AExponentialHeightFog`
- `APostProcessVolume`
- `AT66TowerLightingActor`
- torch light components spawned per eligible floor/wall placement

Important parameters:

- `SkyLightColor`
- `SkyLightIntensity`
- `FogDensity`
- `FogHeightFalloff`
- `FogInscatteringColor`
- `FogStartDistance`
- `FogCutoffDistance`
- `AmbientCubemap`
- `AmbientCubemapIntensity`
- `AmbientCubemapTint`
- `ColorGradeShadowsTint`
- `ColorGradeMidtonesTint`
- `ColorGradeHighlightsTint`
- `ColorGradeSaturation`
- `ColorGradeContrast`
- `ColorGradeGain`
- `TorchIntensity`
- `TorchColor`

Policy:

- This layer can affect the rendered scene globally because it is world-level lighting/post-process.
- It should not be used to solve hero material identity issues.
- If hero readability needs protection from this layer, prefer explicit hero material policy or lighting channel strategy over hidden per-hero hacks.

### 9. Retro FX Layer

Technical name: Retro FX post-process/runtime-overlay layer.

This is the optional late overlay layer.

Owner:

- `Source/T66/Core/T66RetroFXSubsystem.cpp`
- `Source/T66/Core/T66RetroFXSettings.h`
- Settings UI Retro FX screens

Capabilities:

- PS1 blend/dither/Bayer/LUT/fog passes.
- N64 blur/fake resolution passes.
- Real/fake low resolution.
- World/character pixelation masks.
- UI chrome/text treatment.
- Legacy post-process character outline path.

Technical settings:

- `bEnableRetroFXMaster`
- `PS1BlendPercent`
- `PS1DitheringPercent`
- `PS1BayerDitheringPercent`
- `PS1ColorLUTPercent`
- `PS1ColorBoostPercent`
- `PS1FogPercent`
- `PS1SceneDepthFogPercent`
- `bUseRealLowResolution`
- `TargetResolutionHeightPercent`
- `N64BlurBlendPercent`
- `ChromaticAberrationPercent`
- `WorldPixelationPercent`
- `CharacterPixelationPercent`
- `bEnableCharacterOutline`
- `UIChromeTreatmentPercent`
- `UITextTreatmentPercent`

Policy:

- Do not treat Retro FX as part of the default ToonStyle review stack.
- Do not enable Retro FX automatically during unrelated work.
- Do not use Retro FX character outline while T66 is committed to the inverted-hull sidecar outline path.
- When Retro FX is reactivated near end-of-development, verify it composes after ToonStyle without breaking hero readability, sidecar outlines, or UI legibility.

### 10. UI/HUD Presentation Layer

Technical name: UI/HUD visual presentation layer.

UI and HUD visuals are separate from world ToonStyle.

Examples:

- HUD stage area name displays `Hell of Torment` for Impossible.
- Minimap, inventory, health, gold, debt, and text panels are UI presentation, not world materials.
- Retro UI chrome settings are optional overlays, not world atmosphere.

Policy:

- World atmosphere should not recolor UI text/panels.
- UI color changes should follow UI-specific instructions and verification loops.

Technical note:

- UI work follows `UI/Instructions/UI_FIDELITY_LOOP_INSTRUCTIONS.md` when implementing or editing a UI screen from a reference image.
- Native Slate screens and WBP-backed assets must be identified before visual edits.

## Application Matrix

| Visual target | Source/import | Character identity | Outline sidecar | Environment surface | Difficulty Atmosphere | Local data tint | Lighting/fog/post | Retro FX | UI/HUD |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Playable heroes | Yes | Yes | Yes when available | No | No | Only explicit status effects | Scene-level only | Optional late overlay only | No |
| Companions/identity allies | Yes | Yes | Yes when available | No | Default no unless approved | Only explicit status effects | Scene-level only | Optional late overlay only | No |
| Enemies/mobs/bosses | Yes | Optional | Optional | No | Yes when registered/intended | Yes | Yes | Optional late overlay only | No |
| Tower walls/floors/ceilings | No | No | No | Yes | Yes | No | Yes | Optional late overlay only | No |
| Traps/projectiles/VFX | Optional | No | No | No | Theme-appropriate | Yes | Yes | Optional late overlay only | No |
| Props/vehicles/interactables | Optional | No | No | Optional | Theme-appropriate | Yes | Yes | Optional late overlay only | No |
| HUD/menus/minimap | No | No | No | No | No | UI-specific only | No | UI Retro FX only if enabled | Yes |

## New-Agent Checklist

Before changing visuals, answer these questions in the task/report:

1. Which layer owns the symptom?
2. Which visual targets should be affected?
3. Which visual targets must be protected?
4. Is the change source/import, material/shader, runtime atmosphere, local data tint, lighting/post-process, Retro FX, or UI?
5. Does the change affect playable standalone? If yes, follow the standalone verification rule.

## Current Guardrails

- Do not route playable heroes through the Difficulty Atmosphere Layer.
- Do not re-enable Retro FX as a side effect of ToonStyle work.
- Do not replace the inverted-hull outline path with Retro FX outline.
- Keep `/Game/ToonStyle/Environment/<Theme>/...` as the production environment content path.
- Keep `/Game/ToonStyle/TestAssets/...` for test-room-only assets unless a pass intentionally promotes them.
- Keep durable material truths in `MaterialSpec.md` and durable pipeline truths in `PipelineSpec.md`; this document owns layer ownership and application policy.
