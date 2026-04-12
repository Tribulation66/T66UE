# T66 Master Lighting

**Last updated:** 2026-04-11  
**Scope:** Single-source handoff for runtime lighting, atmosphere, fog, post-process, sky rendering, eclipse presentation, and how lighting interacts with the game's mostly-Unlit material pipeline.  
**Companion docs:** `MASTER DOCS/T66_MASTER_GUIDELINES.md`, `MASTER DOCS/MASTER_MAP_DESIGN.md`, `MASTER DOCS/T66_IMPORT_PIPELINE_GUIDELINES.md`  
**Historical predecessor:** `Docs/Systems/lighting.md`  
**Maintenance rule:** Update this file after every material lighting, atmosphere, fog, sky, posterize, eclipse, renderer-config, or preview-world-lighting change.

## 1. Executive Summary

- T66 does not use baked/static gameplay lighting. `Config/DefaultEngine.ini` disables static lighting and runtime worlds are built around movable atmosphere, directional lights, sky light, fog, and post-process.
- The runtime owner is `UT66LightingSubsystem`, which delegates implementation to `FT66WorldLightingSetup`.
- The visual contract is: most gameplay surfaces are intentionally Unlit, while the lighting stack is used mainly for sky, atmosphere, fog, ambient capture, and mood.
- Gameplay now supports two runtime lighting presets:
  - `Eclipse` is the default and preserves the current outdoor eclipse presentation.
  - `Dungeon` is opt-in and keeps materials Unlit while replacing outdoor atmosphere with a player-centered visibility mask.
- Legacy "Light theme" save data still exists only for migration and is forced off in `UT66PlayerSettingsSubsystem`.
- Frontend previews use the same shared world lighting as gameplay; the older preview-specific day/night rig doc is no longer the current source of truth.
- Main-map stages and side-mode maps do not use exactly the same atmosphere stack. Main-map gameplay uses terrain-aware fog tuning and removes the retro Quake canopy; non-main maps can spawn `AT66QuakeSkyActor`.

## 2. Runtime Ownership And Call Flow

- `AT66GameMode::SpawnLightingIfNeeded()` calls `UT66LightingSubsystem::EnsureSharedLightingForWorld(GetWorld())`.
- `AT66FrontendGameMode::BeginPlay()` also calls `EnsureSharedLightingForWorld(World)` so hero/companion previews render under the same ambient setup.
- `HandleSettingsChanged()` in both game modes reapplies:
  - `ApplyThemeToDirectionalLightsForWorld`
  - `ApplyThemeToAtmosphereAndLightingForWorld`
  - retro FX settings
- Gameplay schedules three post-spawn lighting refreshes after terrain/props register:
  - next tick
  - `+0.35s`
  - `+0.65s`

This exists so the sky light recaptures after runtime terrain and stage props appear.

## 3. Shared Runtime Lighting Stack

### 3.1 Actors and components ensured at runtime

If missing, `FT66WorldLightingSetup::EnsureSharedLightingForWorld` spawns:

- `ASkyAtmosphere`
- one primary directional light
- one tagged secondary directional light with tag `T66Moon`
- `ASkyLight`
- `AExponentialHeightFog`
- one unbound `APostProcessVolume`

Everything is forced movable and shadowless for the intended stylized runtime path.

### 3.2 Directional lights

Bootstrap state:

- first light is spawned as warm sunlight, intensity `3.0`, atmosphere index `0`
- second light is spawned as cool moonlight, intensity `0.0`, atmosphere index `1`, tagged `T66Moon`

Active themed state:

- the original sun is disabled:
  - intensity `0.0`
  - `bAtmosphereSunLight = false`
- the tagged moon becomes the active eclipse sun:
  - intensity `5.0`
  - color `(1.0, 0.95, 0.9)`
  - atmosphere index `0`
  - sun disk color scale `(0, 0, 0)`
  - shadows disabled
  - rotation `(-50, 135, 0)`

So the current "sun" in gameplay is actually the re-skinned moon/eclipsed-light path.

### 3.3 Sky atmosphere

Current atmosphere values:

- `RayleighScattering = (0.028, 0.005, 0.004)`
- `RayleighScatteringScale = 0.8`
- `MieScatteringScale = 0.01`
- `MultiScatteringFactor = 0.5`

This keeps the sky in the red/orange eclipse-dusk family even though surface materials are mostly Unlit.

### 3.4 Sky light

Bootstrap state:

- captured scene
- intensity `8.0`
- white lower hemisphere

Themed state:

- captured scene
- intensity `4.0`
- light color `(0.25, 0.2, 0.25)`
- lower hemisphere color `(0.12, 0.1, 0.12)`
- `RecaptureSky()` after application

The sky light is mainly ambient capture and reflection support. It is not the main source of surface shading because the project intentionally avoids light-reactive materials for most gameplay assets.

### 3.5 Fog

Fog is player-setting aware:

- `UT66PlayerSettingsSubsystem` exposes `bFogEnabled`
- `FogIntensityPercent` defaults to `55`
- fog can be disabled entirely by setting intensity to `0`

Default non-main-map fog tuning:

- density base `0.0105`
- start distance `6200`
- height falloff `0.008`
- color `(0.0, 0.53, 0.60)`
- directional color `(0.10, 0.64, 0.74)`

Main-map terrain fog tuning:

- applied only when the map is not Coliseum, Tutorial, or Lab
- derives clear distance from `T66MainMapTerrain` cell size
- density base `0.140`
- height falloff `0.0012`
- color `(0.02, 0.48, 0.54)`
- directional color `(0.03, 0.50, 0.56)`

This means the current gameplay fog is a visibility and scale-control system as much as a mood system. It is not purely "red eclipse fog"; it deliberately uses teal/cyan fog values for readability and distance control on the main-board runtime.

### 3.6 Post-process baseline

The shared unbound post-process volume is used to keep exposure and bloom behavior stable:

- auto exposure min/max forced to `1.0`
- ambient occlusion intensity forced to `0.0`
- color saturation baseline forced near-neutral
- bloom intensity forced to `0.0`
- bloom threshold forced to `10.0`

Project config also disables default auto exposure in `DefaultEngine.ini`, so exposure is intentionally stable.

### 3.7 Posterize

`UT66PosterizeSubsystem` owns the full-screen posterize material:

- material path: `/Game/UI/M_PosterizePostProcess`
- default steps: `10`
- default intensity: `0.85`
- inserted as a weighted blendable into the unbound post-process volume
- enabled by `ApplyThemeToAtmosphereAndLightingForWorld` only for the `Eclipse` preset

This is part of the eclipse presentation stack and is intentionally disabled in the dungeon preset.

### 3.8 Dungeon vision

`UT66DungeonVisionSubsystem` owns the dungeon-only post-process visibility mask:

- material path: `/Game/UI/M_DungeonVisionPostProcess`
- script owner: `Scripts/BuildDungeonVisionPostProcess.py`
- inserted into the unbound post-process volume as a weighted blendable
- tracks the first local player pawn, falling back to camera/view position
- reveals only a world-space XY radius around the player, so floors above/below are not automatically exposed
- fades visibility from full scene color to black outside the configured radius

Current console tuning:

- `t66.DungeonVisionRadius`
- `t66.DungeonVisionFalloff`
- `t66.DungeonVisionAmbientVisibility`

The dungeon preset keeps models flat and shadowless by design. It is a visibility mask, not a physically lit torch system.

### 3.9 Eclipse actor

`AT66EclipseActor` is spawned if none exists:

- material path: `/Game/Lighting/M_EclipseCorona`
- positioned `500000` units along the active atmosphere sun direction
- scaled to `(8000, 8000, 1)`
- billboards toward the camera on tick
- hides itself if the eclipse material asset is missing

## 4. Map-Type Behavior

### 4.1 Main-map gameplay stages

`T66UsesMainMapTerrainStage` returns true when the map is not:

- Coliseum
- Tutorial
- Lab

For main-map stages:

- terrain-aware fog tuning is used
- retro world-geometry fog/snap/affine settings are zeroed out before `RetroFX->ApplySettings(...)`
- `AT66QuakeSkyActor` is removed if present

### 4.2 Non-main maps

For Lab, Tutorial, and Coliseum-style maps:

- shared lighting still exists
- the retro `AT66QuakeSkyActor` can be spawned and kept alive
- these maps do not use the main-board fog heuristic derived from `T66MainMapTerrain`

### 4.3 Frontend

`AT66FrontendGameMode` intentionally uses the same shared lighting stack as gameplay so preview characters live inside the real game atmosphere instead of a separate editor-style showroom.

Important safety rule:

- `UT66GameInstance::GetEffectiveLightingPreset(...)` forces frontend worlds to remain on `Eclipse`, even if gameplay config is set to `Dungeon`

### 4.4 Dungeon preset behavior

When `TribulationLightingPreset=Dungeon` or `t66.TribulationLightingPreset 1` is active for gameplay worlds:

- fog is forced off
- sky atmosphere is hidden and zeroed
- sky light is hidden and zeroed
- directional lights are hidden and zeroed
- posterize is disabled
- `AT66EclipseActor` is removed
- `AT66QuakeSkyActor` is removed
- dungeon vision post-process is enabled

This preserves the project's Unlit-material rule while giving dungeon maps a hard local field of vision and pitch-black space outside the reveal radius.

## 5. Material Interaction: Why Lighting Does Not Drive Most Surfaces

T66 lighting only makes sense when paired with the material rules:

- gameplay world and character surfaces are expected to resolve through the Unlit material path
- the lighting stack is mainly for sky, fog, ambient capture, and background mood
- ground and preview materials commonly resolve from `/Game/Materials/M_Environment_Unlit`
- imported characters and props are expected to use project unlit masters such as:
  - `M_Character_Unlit`
  - `M_Environment_Unlit`
  - `M_GLB_Unlit`

Important current-state correction:

- `FT66VisualUtil::EnsureUnlitMaterials(...)`
- `FT66VisualUtil::EnsureAllWorldMeshesUnlit(...)`

These functions are now audit/log helpers, not automatic material replacement. Older docs that describe them as force-conversion safety nets are stale.

## 6. Preview-Stage Reality

The older `Docs/Systems/T66_Preview_Stage_Handoff.md` describes a custom preview day/night rig. That is no longer the current source of truth.

Current preview behavior:

- preview stages use shared world lighting from `AT66FrontendGameMode`
- preview actors mainly manage:
  - dynamic framing and camera offsets
  - difficulty-based preview ground material
  - optional easy-farm props
- `T66PreviewStageEnvironment` swaps preview ground materials by difficulty but does not build a separate lighting rig

## 7. Renderer And Config Baseline

Relevant current renderer settings in `Config/DefaultEngine.ini`:

- `r.AllowStaticLighting=False`
- `r.DefaultFeature.AutoExposure=False`
- `r.DefaultFeature.AmbientOcclusion=False`
- `r.DefaultFeature.MotionBlur=False`
- `r.DefaultFeature.LensFlare=False`
- `r.SupportSkyAtmosphere=True`
- `r.SupportSkyAtmosphereAffectsHeightFog=True`

So the project is configured around:

- dynamic/runtime lighting
- stable exposure
- sky-atmosphere support
- fog-driven depth
- stylized post-processing rather than realistic physically lit surfaces

## 8. Source Files That Currently Own Lighting

Core runtime owners:

- `Source/T66/Gameplay/T66LightingSubsystem.h/.cpp`
- `Source/T66/Gameplay/T66WorldLightingSetup.h/.cpp`
- `Source/T66/Core/T66DungeonVisionSubsystem.h/.cpp`
- `Source/T66/Core/T66PosterizeSubsystem.h/.cpp`
- `Source/T66/Core/T66PlayerSettingsSubsystem.h/.cpp`

Gameplay entry points:

- `Source/T66/Gameplay/T66GameMode.cpp`
- `Source/T66/Gameplay/T66FrontendGameMode.cpp`
- `Source/T66/Gameplay/T66QuakeSkyActor.cpp`
- `Source/T66/Gameplay/T66EclipseActor.cpp`

Material and visual support:

- `Source/T66/Gameplay/T66VisualUtil.cpp`
- `Source/T66/Gameplay/T66PreviewStageEnvironment.cpp`
- `Source/T66/Gameplay/T66TerrainThemeAssets.cpp`

Historical but still useful:

- `Docs/Systems/lighting.md`

This remains useful for material-pipeline history, but it is no longer the authoritative description of runtime ownership or every current behavior.

## 9. Current Open Truths And Caveats

- The codebase still contains legacy theme terminology, but the live runtime is effectively locked to the eclipse lighting path.
- The new dungeon preset is opt-in only. `Config/DefaultGame.ini` keeps `TribulationLightingPreset=Eclipse` so existing gameplay remains unchanged until explicitly switched.
- Frontend preview lighting docs are partially stale; the current preview path is simpler and shared-world based.
- Old lighting docs still describe some systems as if `T66GameMode` owns all lighting directly. That is no longer true; ownership moved into `UT66LightingSubsystem` plus `FT66WorldLightingSetup`.
- Old docs also describe `EnsureAllWorldMeshesUnlit()` as a replacement pass. The current implementation only audits and logs bad material assignments.
