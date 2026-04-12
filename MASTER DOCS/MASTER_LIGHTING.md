# T66 Master Lighting

**Last updated:** 2026-04-12  
**Scope:** Canonical runtime visual-setup document for gameplay, frontend previews, and the project's Unlit-material contract after the lighting-system purge.  
**Companion docs:** `MASTER DOCS/T66_MASTER_GUIDELINES.md`, `MASTER DOCS/MASTER_MAP_DESIGN.md`, `MASTER DOCS/T66_IMPORT_PIPELINE_GUIDELINES.md`  
**Historical predecessor:** `Docs/Archive/Systems/lighting_historical.md`  
**Maintenance rule:** Update this file whenever runtime visual setup, post-process defaults, or the project's Unlit-material rules change.

## 1. Executive Summary

- T66 no longer has a runtime gameplay lighting stack.
- The game no longer keeps or spawns:
  - `UT66LightingSubsystem`
  - `AT66QuakeSkyActor`
  - `AT66EclipseActor`
  - `UT66DungeonVisionSubsystem`
  - directional lights, sky atmosphere, sky light, or exponential height fog as part of normal runtime setup
- The live runtime path is now one neutral visual cleanup pass plus one unbound post-process volume.
- The visual contract is simple:
  - most gameplay and character surfaces are expected to be Unlit
  - brightness/readability comes from material color and emissive, not scene lighting
  - post-process is used only to keep exposure and baseline screen treatment stable
- Frontend previews now use the same neutral setup as gameplay. There is no separate runtime sky/day-night rig.

## 2. Runtime Ownership

Current owners:

- `Source/T66/Gameplay/T66WorldVisualSetup.h/.cpp`
  - single static runtime cleanup pass
- `Source/T66/Gameplay/T66GameMode.cpp`
  - calls `FT66WorldVisualSetup::EnsureNeutralVisualSetupForWorld(GetWorld())`
- `Source/T66/Gameplay/T66FrontendGameMode.cpp`
  - calls the same cleanup path for hero/companion preview worlds

Removed owners:

- `Source/T66/Gameplay/T66LightingSubsystem.h/.cpp`
- `Source/T66/Gameplay/T66QuakeSkyActor.h/.cpp`
- `Source/T66/Gameplay/T66EclipseActor.h/.cpp`
- `Source/T66/Core/T66DungeonVisionSubsystem.h/.cpp`

## 3. Current Runtime Behavior

`FT66WorldVisualSetup::EnsureNeutralVisualSetupForWorld` now does three things:

1. Removes legacy world-lighting actors if they exist:
   - `ASkyAtmosphere`
   - `ADirectionalLight`
   - `ASkyLight`
   - `AExponentialHeightFog`
   - tagged legacy Quake sky actors
   - legacy `T66QuakeSkyActor` / `T66EclipseActor` instances if found
2. Ensures there is one unbound `APostProcessVolume`.
3. Applies a neutral baseline post-process:
   - fixed non-adaptive exposure using the project's global exposure method
   - auto exposure min/max `1.0`
   - ambient occlusion `0.0`
   - bloom `0.0` with threshold `10.0`
   - neutral color saturation

Gameplay still schedules a few delayed re-runs of this cleanup after terrain/props register so PIE startup or late-spawned legacy actors do not leave stale world-lighting state behind.

## 4. Material Contract

The game is now materially, not physically, lit.

- Gameplay world and character surfaces are expected to render through Unlit masters.
- Common paths include:
  - `/Game/Materials/M_Environment_Unlit`
  - `M_Character_Unlit`
  - `M_GLB_Unlit`
- `FT66VisualUtil::EnsureUnlitMaterials(...)` and related helpers are audit/logging tools, not a live replacement pipeline.

Practical implication:

- If something appears too dark now, the fix is usually the material, texture, or emissive setup.
- The fix is not to reintroduce scene lights, sky capture, or fog.

## 5. Frontend And Side Maps

- Frontend preview worlds use the same neutral runtime cleanup as gameplay.
- Lab, tutorial, and coliseum worlds no longer keep a separate open-sky lighting path.
- There is no supported runtime distinction between "tower lighting" and "side-map lighting" anymore.

## 6. Remaining Related Systems

These still exist, but they are no longer part of core runtime world setup:

- fog-related player settings in `UT66PlayerSettingsSubsystem`
  - still exist in UI/save data
  - no longer drive a live runtime fog actor because runtime fog actors are now removed
- retro FX settings
  - still apply through `UT66RetroFXSubsystem`
  - separate from the deleted sky/light/fog stack

Those items are cleanup candidates if the goal is to remove more obsolete visual controls.

## 7. Asset State

Removed with the runtime purge:

- `/Game/UI/M_PosterizePostProcess`
- `/Game/World/Sky/QuakeCanopy2/*`
- `/Game/World/Sky/Quake/*`
- `/Game/World/Sky/QuakeCanopy/*`
- `/Game/World/Sky/QuakeOverhead/*`
- `/Game/World/Sky/QuakeOverhead2/*`
- `/Game/UI/M_DungeonVisionPostProcess`

The old historical doc `Docs/Archive/Systems/lighting_historical.md` still describes eclipse/dungeon-era behavior. Treat it as background history only, not current runtime truth.

## 8. Renderer Baseline

Relevant project-level renderer settings in `Config/DefaultEngine.ini` still include:

- `r.AllowStaticLighting=False`
- `r.DefaultFeature.AutoExposure=False`
- `r.DefaultFeature.AmbientOcclusion=False`
- `r.DefaultFeature.MotionBlur=False`
- `r.DefaultFeature.LensFlare=False`

So the project baseline remains:

- no baked lighting
- stable exposure
- stylized post-process
- Unlit-first runtime materials

## 9. Current Truths And Caveats

- The repo still contains some historical docs and UI text that talk about fog, eclipse, or older visual presets. They are stale relative to runtime behavior.
- If a future pass removes obsolete fog controls from settings/save data, update this file and the settings docs at the same time.
- If a future visual direction needs atmosphere again, it should be treated as a new system, not a rollback to the deleted eclipse/dungeon preset stack.
