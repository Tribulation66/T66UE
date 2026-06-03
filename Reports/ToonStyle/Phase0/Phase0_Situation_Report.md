# Phase 0 Situation Report

This report audits the live T66 rendering, retro FX, material, atmosphere, and mesh-pipeline state as of Phase 0. It is intentionally focused on what affects the upcoming ToonStyle work.

## Executive Findings

1. The prompt's `Source/T66/Rendering` path is stale. The retro and pixelation subsystems live under `Source/T66/Core/`.
2. The current project is deferred, not forward-rendered: `Config/DefaultEngine.ini:84` has `r.ForwardShading=False`.
3. Anti-aliasing is currently disabled at the project level: `Config/DefaultEngine.ini:108` has `r.AntiAliasingMethod=0`. This is a direct blocker for judging clean cel bands.
4. Lumen GI is not enabled by the current config: `r.DynamicGlobalIlluminationMethod=0`; reflections are set to method `2`.
5. Substrate is enabled (`Config/DefaultEngine.ini:167`). Any Phase 1 material plan should verify UE5.7 material behavior against this project setting before assuming older UE5 material-node behavior.
6. There is no registered project shader directory and no project `.ush` or `.usf` files found. The proposed `#include "/Project/ToonStyle/..."` path will need `.uproject` registration work before it can compile.
7. `Gameplay/World/T66_LIGHTING_REFERENCE.md` is stale relative to live source. It describes a neutral-only lighting baseline, while `T66WorldVisualSetup.cpp` now creates/applies theme SkyLight, fog, post-process, ambient cubemap, carry-light overrides, and Dungeon torch lighting.
8. The current mesh pipeline hardwires pixelated texture generation and flat/recalculated normals. It is not ready for ToonStyle vertex colors or proxy-transferred smooth normals without changes.
9. The material-instance count has drifted from memory: `M_Environment_Lit` has 47 dependent assets, but `M_GLB_Unlit` currently has 63 dependent assets, not 32.
10. `python_unrealengine` is not enabled. UE's built-in `PythonScriptPlugin` is enabled, which is the automation path available in this project.

## Instruction And Pending-Issue Context

Relevant folder routers and process docs were read before writing this report:

- Root `AGENTS.md`
- `Source/T66/Gameplay/GAMEPLAY_AGENTS.md`
- `Source/T66/Gameplay/World/WORLD_AGENTS.md`
- `Model Generation/MODEL_GENERATION_AGENTS.md`
- `Model Generation/Pixal3D/PIXAL3D_AGENTS.md`
- `Gameplay/README.md`
- `Gameplay/World/T66_LIGHTING_REFERENCE.md`
- `Gameplay/World/T66_MAP_DESIGN_REFERENCE.md`
- `Model Generation/Instructions/README.md`
- `Model Generation/Instructions/00_MODEL_GENERATION_ROUTING_INSTRUCTIONS.md`
- `Model Generation/Pixal3D/PIXAL3D_PIPELINE_REFERENCE.md`
- `Scripts/README.md`

Relevant pending issue files were also read:

- `Source/T66/Gameplay/pending_issues_Gameplay.md`
- `Content/Materials/pending_issues_Materials.md`
- `Scripts/pending_issues_Scripts.md`
- `Source/T66/Core/pending_issues_Core.md`

The most relevant pending issues are:

- Non-Dungeon atmosphere specs still need authoring in `T66ThemeAtmosphereData`.
- `/Game/Materials/M_GLB_ViewSpaceLit_Character` is retained but not visually locked; the active production mob path is still `/Game/Materials/MI_GLB_Unlit_Character_Shared`.
- The shared GLB import scripts are still active dependencies and should not be removed.
- Headless Interchange import paths can crash after saves; future material/asset automation should prefer the safer editor scripting paths already documented by the repo.

`ToonStyle/` did not exist before this pass, so there was no folder-specific router or pending issue file to read. This README is the first durable ToonStyle doc.

## 1. Retro / Pixelation Audit

The runtime retro gate is `FT66RetroFXSettings::bEnableRetroFXMaster` in `Source/T66/Core/T66RetroFXSettings.h:20`. The saved player setting mirrors this through `UT66PlayerSettingsSaveGame::bRetroFXMasterEnabled` in `Source/T66/Core/T66PlayerSettingsSaveGame.h:234`.

Important defaults:

- `bEnableRetroFXMaster = true` in `T66RetroFXSettings.h`.
- Most actual effect percentages default to zero, including PS1 blend, dithering, color LUT, fog, and all pixelation percentages.
- UI CRT defaults are separately enabled in the settings struct.

The effective runtime behavior is controlled in `UT66RetroFXSubsystem::BuildEffectiveSettings` (`Source/T66/Core/T66RetroFXSubsystem.cpp:399`). If the master gate is false, the subsystem zeros PS1, pixelation, UI, CRT, geometry, and related effect settings before applying anything.

With the gate off, code still touches the post-process path for cleanup and consistency:

- `ApplySettings` still resolves the active world and calls `EnsureBlendablesInWorld`.
- It still applies blendable weights, resolution runtime settings, geometry collection state, stencil masks, and geometry material restoration.
- It still calls `UT66PixelationSubsystem::SetPixelationLevels` with zeroed levels.
- Existing post-process dynamic materials are weighted to zero rather than always destroyed.

The pixelation subsystem itself is lazy. `UT66PixelationSubsystem::SetPixelationLevels` (`Source/T66/Core/T66PixelationSubsystem.cpp:80`) returns after applying a zero weight if both levels are zero. It does not create its post-process volume/material unless a nonzero world or character level requires it. The material path is `/Game/UI/M_PixelationPostProcess.M_PixelationPostProcess`.

Runtime toggling flows through the player settings subsystem. `UT66PlayerSettingsSubsystem::SetRetroFXSettings` persists the struct and mirrored master flag, while `AT66GameMode::HandleSettingsChanged` reapplies settings by calling `UT66RetroFXSubsystem::ApplySettings`.

PIXAL3D is not the same system as `T66PixelationSubsystem`. No `Pixal3D` or `PixalSubsystem` C++ runtime subsystem was found. Pixal3D exists under `Model Generation/Pixal3D/` as a separate research model-generation pipeline with server and batch scripts. Its outputs are namespaced under Pixal3D run/output folders and should not be treated as the runtime pixelation system.

The requested Blender scripts `retro_process.py`, `pixelate.py`, and `retopo_and_bake.py` were not found by name in the repo. The current equivalent appears to be the QuadRetro pipeline at `Model Generation/Scripts/Core/QuadRetro/t66_quad_retro_character_pipeline.py`.

## 2. Global Lighting And Rendering Audit

Project-level renderer settings in `Config/DefaultEngine.ini`:

- Deferred renderer: `r.ForwardShading=False`.
- Dynamic GI method: `r.DynamicGlobalIlluminationMethod=0`.
- Reflection method: `r.ReflectionMethod=2`.
- Virtual Shadow Maps: `r.Shadow.Virtual.Enable=1`.
- Nanite project setting: `r.Nanite.ProjectEnabled=True`.
- Ray tracing: `r.RayTracing=False`.
- Mesh distance fields: `r.GenerateMeshDistanceFields=False`.
- Static lighting disabled: `r.AllowStaticLighting=False`.
- Anti-aliasing: `r.AntiAliasingMethod=0`.
- Substrate: `r.Substrate=True`.
- Default RHI: DX12 with SM6 targeted.

The live runtime lighting path is mostly owned by `Source/T66/Gameplay/T66WorldVisualSetup.cpp`, not by static map lighting:

- `EnsureNeutralVisualSetupForWorld` removes SkyAtmosphere, DirectionalLight, untagged SkyLight, ExponentialHeightFog, and old Quake/Eclipse actors.
- `EnsureAtmosphereForWorld` applies theme SkyLight, fog, post-process, and hero carry-light settings.
- The strip allowlist tag is `T66_AtmosphereSpared`.

DirectionalLight state: live source removes all `ADirectionalLight` actors during neutral setup. No current runtime setup was found for a DirectionalLight mobility, atmosphere sun index, or cast-shadow policy.

SkyLight state: `EnsureAtmosphereSkyLightForWorld` creates or reuses a tagged SkyLight, but the Dungeon spec sets intensity to zero. The SkyLight has shadows disabled and realtime capture disabled. The prompt memory that SkyLight was decommissioned is directionally correct for lighting contribution, but the actor still exists as part of the atmosphere setup.

Theme post-process: confirmed. `EnsureAtmospherePostProcessForWorld` creates an unbound volume tagged `T66_ThemeAtmospherePostProcess`, sets priority `1000`, applies color grading, and assigns an ambient cubemap. Dungeon currently uses `/Engine/MapTemplates/Sky/DaylightAmbientCubemap.DaylightAmbientCubemap` with intensity `2.5`.

Sky atmosphere: the prompt's Rayleigh/eclipse-dusk memory is not live. `EnsureNeutralVisualSetupForWorld` destroys `ASkyAtmosphere` actors, and no live creation path was found.

Procedural torches: `Source/T66/Gameplay/T66TowerLighting.cpp` spawns Dungeon-only torch lights for supported room roles. The lights are movable point lights, tagged `T66_AtmosphereSpared`, have shadows disabled, and use the current theme torch spec.

Stage progression visuals: `Source/T66/Gameplay/T66StageProgressionVisuals.cpp` is effectively neutralized. Its local guard returns false, so the old ColorSaturation writes are not active.

## 3. Material System Audit

`Content/Materials/M_Environment_Lit.uasset` is a Default Lit, opaque surface material. Binary inspection shows `MSM_DefaultLit`, `MD_Surface`, `BLEND_Opaque`, and parameter names including `DiffuseColorMap`, `BaseColor`, `Metallic`, `Roughness`, `Specular`, and `Tint`. The graph appears to be a simple texture/tint/PBR-parameter wiring material, not a custom HLSL material.

`Content/Materials/M_GLB_Unlit.uasset` is an Unlit, opaque surface material. Binary inspection shows `MSM_Unlit`, `MD_Surface`, `BLEND_Opaque`, and both `BaseColorTexture` and `DiffuseColorMap` texture parameter names. This confirms the aliasing pattern used by imported GLB assets.

Material reference counts:

- `M_Environment_Lit`: 47 dependent `.uasset` files under `Content/`, excluding the master itself.
- `M_GLB_Unlit`: 63 dependent `.uasset` files under `Content/`, excluding `M_GLB_Unlit` and `M_GLB_Unlit_RetroGeometry`. Split by top folder: Characters 4, Materials 1, Weapons 12, World 46.

Existing Material Parameter Collection assets by filename:

- `Content/Materials/Retro/MPC_T66_RetroGeometry.uasset`
- `Content/UE5RFX/Materials/UE5RFX_MaterialParameterCollection.uasset`
- `Content/UE5RFX/VertexLights/Essentials/UE5RFX_VertexLightMaterialParameterCollection.uasset`

No project shader source directory was found. A repo-wide search found no `.ush` or `.usf` files and no `Shaders/` folder before this pass. `T66.uproject` contains no shader-directory registration. Phase 1 must add and verify the registration before Custom-node includes can target `/Project/ToonStyle/...`.

No evidence of Surface ForwardShading usage was found by text/binary scan. The project-level renderer is deferred. If Phase 1 wants Surface ForwardShading lighting mode inside materials, it must be validated in UE5.7 with the current deferred/Substrate setup rather than assumed.

Existing Custom HLSL usage was not conclusively established from binary-only scans. Some material assets contain generic external-code metadata strings, but this is not enough to prove active Custom nodes. A future editor-side material dump is the right way to inventory actual Custom node usage.

## 4. C++ Atmosphere And Parameter Plumbing Audit

`FT66ThemeAtmosphereSpec` currently drives actors and post-process settings, not material parameters. It contains sky, fog, ambient cubemap, color grading, torch, and carry-light fields. The path is:

- Theme selection resolves a `FT66ThemeAtmosphereSpec`.
- `T66WorldVisualSetup` applies it to SkyLight, ExponentialHeightFog, the theme post-process volume, Dungeon torch lights, and hero carry lights.
- No Material Parameter Collection or MID fan-out was found in this atmosphere path.

The cleanest Phase 1 integration point is still `T66WorldVisualSetup`, but the plan should be precise: adding cel parameters to `FT66ThemeAtmosphereSpec` is not enough. A new material-parameter delivery path must be created, likely by either:

- applying parameters to known runtime material instances/components during visual setup, or
- adding a ToonStyle Material Parameter Collection and updating it from the theme spec.

The `T66_AtmosphereSpared` strip allowlist works as described. The helper that destroys atmosphere-related actor classes skips actors with that tag.

Per-character parameter delivery exists only in a loose sense. Characters already have component/material seams and shared character material paths, especially `UT66CharacterVisualSubsystem` and `/Game/Materials/MI_GLB_Unlit_Character_Shared`. There is not currently a general per-character toon-parameter architecture matching the proposed character/environment split.

Hero carry lights are created in `AT66HeroBase`, but the Dungeon atmosphere spec sets carry-light intensity to zero. Carry lights have shadows disabled.

## 5. Mesh Pipeline Audit

The named scripts from the prompt were not present:

- `retro_process.py` - not found.
- `pixelate.py` - not found.
- `retopo_and_bake.py` - not found.

The current character-processing equivalent is `Model Generation/Scripts/Core/QuadRetro/t66_quad_retro_character_pipeline.py`.

Current QuadRetro responsibilities:

- Imports the source model.
- Joins imported mesh parts with `bpy.ops.object.join`.
- Normalizes scale.
- Cleans mesh data and, by default, recalculates normals.
- Prepares TRELLIS source materials for flat color baking by forcing opaque material state and neutralizing metallic/roughness/alpha.
- Generates a Quad Remesher source and imports/reuses a retopo FBX.
- Smart-unwraps the retopo mesh.
- Force-flat-shades the retopo mesh by default.
- Bakes diffuse color.
- Normalizes luminance.
- Pixelates and dithers the baked image.
- Assigns the pixelated texture to a material.
- Exports a GLB.

The pixelation stage is conceptually separable, but it is not currently an optional stage. `make_pixelated_image`, `save_image(pixel_path)`, and `assign_pixel_material(retopo, pixel_image, ...)` are hardwired in `main`.

Vertex colors are not ready for ToonStyle. The Quad Remesher settings explicitly write `UseVertexColorMap=False`, and no current copy/preserve path for source vertex colors to the retopo/export mesh was found. Any GG Xrd-style threshold and outline channel plan needs a new vertex-color authoring and preservation step.

Vertex normals are also not ready. The current pipeline defaults are `--use-normals false`, `--recalculate-normals true`, and `--shade-flat true`. Unreal import defaults preserve imported normals by setting `recompute_normals=False` and `recompute_tangents=False`, but the imported normals are currently generated by a retro/flat pipeline, not a proxy-transferred toon pipeline.

Unreal static mesh import defaults in `Scripts/ImportStaticMeshes.py`:

- `use_full_precision_u_vs=True`
- `generate_lightmap_u_vs=False`
- `recompute_normals=False`
- `recompute_tangents=False`

Those import settings are good for preserving authored normals once the Blender pipeline creates them, but they do not solve the current upstream normal-generation problem.

Nanite state is mixed. The project enables Nanite, and several optimization scripts explicitly apply or verify Nanite for generated environment kit meshes. Character and imported GLB paths need per-asset verification before assuming Nanite behavior.

## 6. Codex Access Audit

Codex can create and write under `ToonStyle/`; this pass created the requested scaffold.

The project has UE's `PythonScriptPlugin` enabled in `T66.uproject`. The legacy plugin name `python_unrealengine` was not found. Future prompts should ask for UE Python automation, not `python_unrealengine`.

There are no observed filesystem permission problems with the proposed ToonStyle folder structure.

The main build-system constraint is shader registration. `ToonStyle/Shaders/` existing on disk is not sufficient for material Custom nodes to include `.ush` files. Phase 1 needs an explicit `.uproject` shader-directory registration step and an editor compile check.

Git will not track the empty scaffold folders until files are added under them. This pass intentionally did not add `.gitkeep` files because the requested write list only named markdown reports.
