# T66 Visual Systems Audit - Three-Layer Comprehensive

Working goal: establish the current visual data flow from Pixal3D source assets through Unreal materials, lighting, camera, color, and Retro FX, then identify conflicts before the visual lock pass.

Audit status: read-only inspection. No source assets, code, data tables, or settings files were changed. Runtime screenshots were captured from the staged standalone executable without `?listen`.

Evidence generated during this audit:

- `Saved/VisualSystemsAudit/unreal_asset_probe.json` - Unreal asset/material/texture/map probe.
- `Saved/VisualSystemsAudit/blender_mesh_probe.json` - Blender 5.1 GLB/FBX mesh and texture probe.
- `Saved/VisualSystemsAudit/image_stats.json` - source/converted texture size, color, and luminance probe.
- `Audit/Reference/Visual_Systems_Audit/screenshots/` - ground-truth staged screenshots.

## Layer 1 - Materials, Lighting, Camera, Color

### Materials

The production mob runtime path currently uses `UT66CharacterVisualSubsystem` to identify `/Game/Characters/Mobs/` static meshes as Quad Retro static visuals. For each active mob it creates a dynamic material instance from `/Game/Materials/MI_GLB_Unlit_Character_Shared`, assigns the imported mob texture to the texture aliases `EmissiveTexture`, `BaseColorTexture`, and `DiffuseColorMap`, and sets `Brightness=1`, `Tint=(1,1,1,1)`, `EmissiveFactor=(1,1,1,1)`, and `BaseColorFactor=(0,0,0,1)`.

The requested asset `/Game/Materials/M_GLB_Unlit_Character_Shared` is not present as a material asset in the current project probe. The active shared asset is the material instance `/Game/Materials/MI_GLB_Unlit_Character_Shared`, parented to `/Game/Materials/M_GLB_Unlit`.

| Material | Current role | Domain / shading | Parameters found | Current value | Tunable notes |
|---|---|---|---|---|---|
| `/Game/Materials/M_Character_Unlit` | Legacy/simple character material | Surface, Unlit, Masked, two-sided | `Brightness`, `DiffuseColorMap` | `Brightness=1`, default engine texture | Can still tune brightness if assigned, but production mobs use the GLB shared instance path. |
| `/Game/Materials/MI_GLB_Unlit_Character_Shared` | Active production mob shared material instance | Parent: `/Game/Materials/M_GLB_Unlit` | `Brightness`, `EmissiveFactor`, `BaseColorFactor`, `Tint` | `1`, white emissive/tint, black base factor | Runtime DMI texture overrides are the active visual binding point for all 50 mobs. |
| `/Game/Materials/M_GLB_Unlit` | Parent for active shared mob material | Surface, Unlit, Opaque, two-sided | `Brightness`, `Tint`, `BaseColorTexture` | `Brightness=1`, `Tint=white`, default texture | Main production material behavior is texture-driven and light-independent. |
| `/Game/Materials/M_Environment_Unlit` | Environment unlit material | Surface, Unlit, Opaque, two-sided | `Brightness`, `Tint`, `DiffuseColorMap` | `Brightness=1`, `Tint=white`, default texture | Environment brightness/color is material-authored, not world-light-authored. |
| `/Game/Materials/M_FBX_Unlit` | FBX fallback/import material | Surface, Unlit, Opaque | no active production mob override found | default | Still relevant as fallback for older imported meshes. |
| `/Game/Materials/M_GLB_ViewSpaceLit_Character` | Track 2 parked character material | Surface, Unlit, Opaque, emissive cel lighting | `Brightness`, `Tint`, `LightDirection_ViewSpace`, `ShadowTint`, `MidtoneTint`, `HighlightTint`, `RampStep1`, `RampStep2`, `RimStrength`, `RimColor`, `RimPower`, texture aliases | See knob table below | Built to add view-space 3-band cel response and rim without depending on stripped world lights. Not active on production mobs. |
| `/Game/Materials/Retro/M_T66_OutlinePostProcess` | Track 2 parked/runtime outline post-process | Post Process, Unlit | `OutlineColor`, `OutlineThickness`, `OutlineOpacity`, `CharacterStencilValue` | black, `1.5`, `1.0`, `2.0` | Slotted in the Retro FX post-process stack and enabled by `bEnableCharacterOutline`. |
| `/Game/Materials/Retro/M_RetroChromaticAberrationPostProcess` | Optional runtime chromatic/distortion pass | Post Process, Unlit | `ChromaticStrength`, `DistortionAmount` | `0`, `0` by default settings | In stack only becomes visible when settings drive nonzero values. |
| `/Game/Materials/Retro/M_RetroGeometry_*` | Runtime geometry replacement materials | Surface, Unlit, Opaque | driven by Retro geometry material parameter collection | inactive by default | Used for world/character vertex snap, noise, and affine effect when geometry settings are enabled. |

### Lighting

Current gameplay rendering is intentionally unlit-first. `Source/T66/Gameplay/T66WorldVisualSetup.cpp` removes these actors at runtime:

- `ASkyAtmosphere`
- `ADirectionalLight`
- `ASkyLight`
- `AExponentialHeightFog`
- actors tagged `T66QuakeSky`, `QuakeSky`, or `T66LegacyLighting`
- legacy class-name matches for `T66QuakeSkyActor` and `T66EclipseActor`

It then creates or reuses an unbound neutral post-process volume and locks exposure behavior with `AutoExposureMinBrightness=1.0` and `AutoExposureMaxBrightness=1.0`. It also disables ambient occlusion, disables bloom, raises bloom threshold, and sets saturation to `(0.95,0.95,0.95,1.0)`.

`Config/DefaultEngine.ini` has `r.DefaultFeature.AutoExposure=False`, so auto exposure is off by default. The neutral post-process settings reinforce a fixed 1/1 exposure range when the volume is applied.

The prompt's locked design context mentions an Eclipse dusk look via Sky Atmosphere and Rayleigh scattering. That is not the current gameplay state. The current source and `Gameplay/World/T66_LIGHTING_REFERENCE.md` state that the runtime gameplay path strips sky/atmosphere/fog/light actors and has no supported runtime sky/day-night rig. Frontend and lab maps still contain authored lights in the probe, but `/Game/Maps/GameplayLevel` did not contain the selected authored sky/light actors in the current load, and any matching actors would be stripped for gameplay.

### Camera

Gameplay camera is built on `AT66HeroBase` with a spring arm and camera component:

- Default spring arm length: `1440`.
- Spring arm relative location: `(0,0,60)`.
- Spring arm uses pawn control rotation.
- Camera is attached to the spring arm and does not use pawn control rotation directly.
- Spring arm collision test is disabled.
- Probe size/channel are configured, but collision test is off.
- No explicit default camera FOV is set on the component in the inspected constructor, so the UE camera default applies unless controller logic changes it.

`AT66PlayerController` adds gameplay camera controls:

- `T66.Camera.GameplayPreset` default `0`.
- Locked chase preset `1`: pitch `-30`, arm length `1150`, pivot height `145`, forward offset `35`.
- Gameplay pitch clamp: `-72` to `-4`.
- Ctrl/Command + mouse wheel adjusts pitch by `2.5` degrees per step.
- Normal mouse wheel zoom adjusts spring arm length by `120` per step, clamped `350` to `2800`.
- Scoped/ultimate camera uses FOV `18`, clamped `8` to `30`, with `2` degree step tuning, and temporarily changes boom setup while hiding hero meshes.

The staged screenshots show that gallery mobs are readable when framed in the gallery wing, but stage combat visibility is strongly affected by camera distance, top-down angle, walls, and small on-screen mob size.

### Color and Tone Mapping

The project uses sRGB working color space. There is no inspected project setting or runtime code path that swaps the gameplay tone mapper to Reinhard or linear. UE's default filmic tone mapping remains the practical baseline unless a post-process material overrides the final image.

Color saturation is controlled by the neutral visual setup and stage progression:

- Neutral gameplay setup: `(0.95,0.95,0.95,1.0)`.
- Stage 1: `(0.95,0.95,0.95,1.0)`.
- Stage 2: `(1.00,0.90,0.88,1.0)`.
- Stage 3: `(1.05,0.86,0.82,1.0)`.
- Stage 4: `(1.10,0.81,0.76,1.0)`.

Bloom and ambient occlusion are explicitly neutralized by the runtime visual setup. Any scene contrast currently comes primarily from authored textures/materials, map geometry, Retro FX post-process materials, and fixed exposure.

### Character Texture Handling

Sample production mob textures inspected in Unreal:

- `/Game/Characters/Mobs/Slime/T_Slime`
- `/Game/Characters/Mobs/MushroomBrute/T_MushroomBrute`
- `/Game/Characters/Mobs/BoneWalker/T_BoneWalker`
- `/Game/Characters/Mobs/Hellhound/T_Hellhound`

All sampled textures are:

- `2048x2048`
- `TEXTUREGROUP_CHARACTER`
- `TF_NEAREST`
- `MipGenSettings=FROM_TEXTURE_GROUP`
- `NeverStream=false`
- `VirtualTextureStreaming=false`
- `MaxTextureSize=0`
- `LODBias=0`
- `sRGB=true`

This matches the current crisp-pixel convention while keeping streaming and mips available.

### Layer 1 Knob Table

| Knob | Layer | File / location | Current value | What it controls | Sensible tuning range |
|---|---|---|---|---|---|
| Mob texture binding | 1 | `Source/T66/Core/T66CharacterVisualSubsystem.cpp` | `EmissiveTexture`, `BaseColorTexture`, `DiffuseColorMap` all set from mob texture | Which imported texture drives production mob material appearance | Keep aliases synchronized unless parent material is simplified. |
| Active mob material brightness | 1 | `/Game/Materials/MI_GLB_Unlit_Character_Shared` runtime DMI | `1.0` | Overall mob emissive/unlit brightness | `0.75-1.5` for visual lock; above that risks flattening color. |
| Active mob tint | 1 | `/Game/Materials/MI_GLB_Unlit_Character_Shared` runtime DMI | white | Global color multiplier for mobs | Mostly keep white; use only for controlled family/theme passes. |
| View-space light direction | 1 | `/Game/Materials/M_GLB_ViewSpaceLit_Character` | `(0.4,0.4,1.0)` | Direction of fake cel lighting in view space | `unknown - needs experimentation` per camera angle. |
| View-space cel ramp step 1 | 1 | `/Game/Materials/M_GLB_ViewSpaceLit_Character` | `0.4` | Shadow-to-midtone threshold | `0.25-0.55`. |
| View-space cel ramp step 2 | 1 | `/Game/Materials/M_GLB_ViewSpaceLit_Character` | `0.75` | Midtone-to-highlight threshold | `0.6-0.9`. |
| Shadow tint | 1 | `/Game/Materials/M_GLB_ViewSpaceLit_Character` | `(0.55,0.55,0.65)` | Fake shadow color for cel bands | `0.45-0.8` value, slightly cool for readability. |
| Midtone tint | 1 | `/Game/Materials/M_GLB_ViewSpaceLit_Character` | `(0.85,0.85,0.9)` | Fake midtone color for cel bands | `0.75-1.0`. |
| Highlight tint | 1 | `/Game/Materials/M_GLB_ViewSpaceLit_Character` | `(1,1,1)` | Fake highlight color | `0.9-1.2`, avoid washout. |
| Rim strength | 1 | `/Game/Materials/M_GLB_ViewSpaceLit_Character` | `0.35` | Rim-light silhouette reinforcement | `0.1-0.6`; higher risks white halos. |
| Rim power | 1 | `/Game/Materials/M_GLB_ViewSpaceLit_Character` | `2.5` | Rim falloff sharpness | `1.5-5.0`. |
| Outline thickness | 1/3 | `/Game/Materials/Retro/M_T66_OutlinePostProcess` | `1.5` | Screen-space outline width from stencil/Sobel | `1-4` pixels. |
| Outline opacity | 1/3 | `/Game/Materials/Retro/M_T66_OutlinePostProcess` | `1.0` | Screen-space outline strength | `0.4-1.0`. |
| Character stencil value | 1/3 | `/Game/Materials/Retro/M_T66_OutlinePostProcess` and Retro FX subsystem | `2` | Which custom stencil value receives character outline | Keep `2` unless stencil taxonomy changes. |
| Runtime stripped light classes | 1 | `Source/T66/Gameplay/T66WorldVisualSetup.cpp` | sky atmosphere, directional light, sky light, height fog | Removes world-authored lighting from gameplay | Binary policy; only revisit if moving away from unlit-first rendering. |
| Auto exposure min/max | 1 | `Source/T66/Gameplay/T66WorldVisualSetup.cpp` | `1.0 / 1.0` | Prevents camera exposure pumping | Keep locked for retro readability; test `0.9-1.1` only if needed. |
| Neutral saturation | 1 | `Source/T66/Gameplay/T66WorldVisualSetup.cpp` | `(0.95,0.95,0.95,1.0)` | Global saturation in neutral gameplay setup | `0.9-1.1`; avoid muting source color identity. |
| Stage saturation | 1 | `Source/T66/Core/T66StageProgressionTuningConfig.cpp` | Stage-specific values listed above | Progression color mood | Tune per stage but verify mobs remain legible. |
| Default spring arm length | 1 | `Source/T66/Gameplay/T66HeroBase.cpp` | `1440` | Base camera distance | `900-1600` for combat readability; lower increases detail. |
| Chase preset arm length | 1 | `Source/T66/Gameplay/T66PlayerController.cpp` | `1150` | Camera distance for locked chase preset | `800-1300`. |
| Chase preset pitch | 1 | `Source/T66/Gameplay/T66PlayerController.cpp` | `-30` | View angle and silhouette readability | `-25` to `-45`; steeper reduces faces/details. |
| Gameplay zoom clamp | 1 | `Source/T66/Gameplay/T66PlayerController_Input.cpp` | `350-2800` | Player-controlled distance range | Current wide range is useful; visual lock should define recommended default. |
| Scoped FOV | 1 | `Source/T66/Gameplay/T66PlayerController.cpp` | `18` | Special zoom/aim view | `15-25`; too low makes staging unusable for visual QA. |
| Character texture group | 1 | `Scripts/SetCharacterTextureStreamingDefaults.py` and current assets | `TEXTUREGROUP_CHARACTER` | LOD/mip behavior category | Keep for production mobs. |
| Character texture filter | 1 | `Scripts/SetCharacterTextureStreamingDefaults.py` and current assets | `TF_NEAREST` | Pixel-crisp sampling | Keep nearest for target style. |
| Character texture max size | 1 | current texture assets | `0` | No per-texture max clamp | `1024-2048` if memory pressure requires; current source detail is 2048. |

## Layer 2 - Pipeline

### Scripts Present

`Model Generation/Scripts/Core/QuadRetro/` currently contains:

- `RunQuadRetroCharacterPipeline.ps1`
- `t66_quad_retro_character_pipeline.py`
- `__pycache__/`

The requested standalone script names `retopo_and_bake.py` and `pixelate.py` are not present as separate files. Their functionality is combined into `t66_quad_retro_character_pipeline.py`.

Additional helper scripts inspected outside that folder:

- `Scripts/SetCharacterTextureStreamingDefaults.py`
- `Scripts/GenerateCharacterMeshLODs.py`
- `Scripts/MigrateQuadRetroMaterialAssignment.py`
- `Scripts/QuadRetroCharacterPipelineDefaults.py`

### Quad Retro Orchestrator

`t66_quad_retro_character_pipeline.py` is the full Blender-side retro processing orchestrator. It can:

- import source GLB/mesh data;
- retopologize through Quad Remesher when available;
- bake source material color to a diffuse texture;
- optionally normalize texture luminance;
- dilate transparent edge pixels;
- pixelate/downsample;
- quantize palette;
- apply ordered/noise dither;
- export a `_QuadRetro.glb`;
- write a pipeline report with mesh counts and texture parameters.

Direct Python CLI defaults are aggressive relative to the current cel-shaded source:

- `target_quads=5000`
- `texture_size=256`
- `palette_mode=kmeans`
- `palette_size=24`
- `dither_type=bayer4`
- `dither_strength=0.85`
- `normalize_luminance=true`
- `target_luminance=0.50`
- `max_scaling_factor=4.0`
- `bake_size=1024`
- `dilate=20`

The PowerShell wrapper defaults are the safer current baseline:

- `TargetQuads=12000`
- `TextureSize=512`
- `PaletteMode=none`
- `PaletteSteps=256`
- `DitherType=none`
- `DitherStrength=0`
- `NormalizeLuminance=true`
- `TargetLuminance=0.50`
- `MaxScalingFactor=4.0`
- `SaturationBoost=1.0`
- `BakeSize=1024`

Track 1 added luminance normalization after diffuse baking and transparent-pixel dilation, before pixelation. Track 2 left the view-space cel material and outline material parked until better source models existed.

### Unreal Helper Scripts

`Scripts/SetCharacterTextureStreamingDefaults.py` applies the production texture convention:

- `TEXTUREGROUP_CHARACTER`
- `TF_NEAREST`
- streaming enabled when available
- virtual texture streaming disabled
- mip generation from texture group if missing
- no forced max size or LOD bias

`Scripts/GenerateCharacterMeshLODs.py` applies the shared LOD ladder from `Scripts/QuadRetroCharacterPipelineDefaults.py`:

- LOD0: `1.0`, screen size `1.0`
- LOD1: `0.4`, screen size `0.6`
- LOD2: `0.15`, screen size `0.25`
- LOD3: `0.05`, screen size `0.1`

`Scripts/MigrateQuadRetroMaterialAssignment.py` migrates character meshes to the shared material instance and texture conventions.

### Did The 50 Production Mobs Go Through Quad Retro?

Conclusion: **Not processed at all by the Blender Quad Retro pipeline.**

They were Pixal3D-generated/exported, then format-converted through Blender 5.1 for Unreal compatibility, then imported into Unreal with shared material assignment, texture defaults, and UE LOD generation. They were not passed through `t66_quad_retro_character_pipeline.py` for Quad Remesher retopology, bake, pixelate, dither, palette quantization, or `_QuadRetro.glb` export.

Specific evidence:

- `Audit/Reference/Mob_Production_Import/Report.md` states the import path used Blender 5.1 to convert each source GLB to FBX and extract a PNG because headless Unreal GLB import failed. It does not report running `t66_quad_retro_character_pipeline.py`.
- `Saved/QuadRetroEnemyVisualImportReport.json` records per-mob `source_glb`, `converted_fbx`, `converted_texture`, import destination, generated Unreal LODs, and `MI_GLB_Unlit_Character_Shared` assignment. It does not record Quad Retro pipeline outputs such as `_QuadRetro.glb`, `_Pixelated_512.png`, bake images, dither settings, palette settings, or pipeline reports.
- Blender probe showed sample GLB and converted FBX geometry counts match exactly:
  - Slime: GLB `29,647` triangles, FBX `29,647` triangles.
  - MushroomBrute: GLB `28,955`, FBX `28,955`.
  - BoneWalker: GLB `28,119`, FBX `28,119`.
  - Hellhound: GLB `28,261`, FBX `28,261`.
- The import report's generated UE LOD0 counts match the converted/raw mesh scale, with lower LODs generated by Unreal after import.
- Converted textures are `2048x2048`, not the wrapper default `512` or Python default `256`.
- Texture color probes show high unique color counts inconsistent with baked 24-color quantization:
  - Slime converted texture: more than `300,000` unique RGBA colors.
  - BoneWalker converted texture: `177,973` unique RGBA colors.
  - Hellhound converted texture: `60,240` unique RGBA colors.
  - MushroomBrute converted texture: `47,149` unique RGBA colors.
- No sampled production texture showed evidence of baked ordered dither, forced low-res downsample, or small fixed palette output.

The import manifest notes Pixal3D export decimation/remesh settings around `30000` triangles. That is generator/export-side processing, not this repo's Quad Retro Blender pipeline.

### Visibility Loss Diagnosis

The prior visibility loss was tied to old TRELLIS source assets and aggressive texture treatment. Track 1 showed very low source luminance before normalization and added luminance normalization with `TargetLuminance=0.50` and `MaxScalingFactor=4.0`. Track 2 parked a view-space cel material and stencil outline because the old textures were not good enough to evaluate final readability.

The riskiest Quad Retro knobs for the new cel-shaded source are:

- Low `texture_size`, especially `256`, because the source readability is encoded in hard outlines, facial marks, and 2-color fills.
- `palette_mode=kmeans` with `palette_size=24`, because it may collapse intentionally separated cel colors.
- `dither_type=bayer4` with `dither_strength=0.85`, because it can add visible pattern noise over already-flat fills and outlines.
- Luminance normalization with high scaling on already-authored cel images, because it can flatten intended contrast if pushed too hard.
- Very low retopo target counts if silhouettes or single facial features are simplified away.

For the current 50 mobs, the evidence supports preserving the clean 2048 cel source and applying retro treatment selectively at runtime or with a much softer bake profile.

### Layer 2 Knob Table

| Knob | Layer | File / location | Current value | What it controls | Sensible tuning range |
|---|---|---|---|---|---|
| Target quads | 2 | `RunQuadRetroCharacterPipeline.ps1` | `12000` wrapper, `5000` Python default | Retopo mesh density | `8000-18000` for cel mobs; lower only after silhouette check. |
| Adaptive size | 2 | `RunQuadRetroCharacterPipeline.ps1` | `50` | Quad Remesher adaptive detail preservation | `30-70`; higher can preserve feature density. |
| Use materials | 2 | `RunQuadRetroCharacterPipeline.ps1` | `true` | Whether material/texture data is considered during retopo/bake | Keep true for textured mobs. |
| Use normals | 2 | `RunQuadRetroCharacterPipeline.ps1` | `false` | Whether normals guide remesh | `unknown - needs experimentation`. |
| Auto-detect hard edges | 2 | `RunQuadRetroCharacterPipeline.ps1` | `true` | Preserves hard visual/geometry edges during remesh | Keep true for hard-outline source. |
| Bake size | 2 | `RunQuadRetroCharacterPipeline.ps1` | `1024` | Intermediate texture bake resolution | `1024-2048`; use 2048 if preserving source facial marks. |
| Texture size | 2 | `RunQuadRetroCharacterPipeline.ps1` | `512` wrapper, `256` Python default | Final downsampled texture resolution | `512-1024` for visual lock tests; `256` likely too destructive. |
| Normalize luminance | 2 | `RunQuadRetroCharacterPipeline.ps1` | `true` | Raises dark baked textures toward target luminance | Keep available; test off/on for current cel source. |
| Target luminance | 2 | `RunQuadRetroCharacterPipeline.ps1` | `0.50` | Desired average luminance after normalization | `0.40-0.60`; avoid overbrightening dark archetypes. |
| Max scaling factor | 2 | `RunQuadRetroCharacterPipeline.ps1` | `4.0` | Upper bound on luminance gain | `1.5-3.0` for current source; `4.0` was for old dark assets. |
| Saturation boost | 2 | `RunQuadRetroCharacterPipeline.ps1` | `1.0` | Saturation after luminance normalization | `0.9-1.2`. |
| Palette mode | 2 | `RunQuadRetroCharacterPipeline.ps1` | `none` wrapper, `kmeans` Python default | Color quantization algorithm | Keep `none` or test controlled `uniform`; avoid `kmeans` until compared. |
| Palette size | 2 | `t66_quad_retro_character_pipeline.py` | `24` Python default | Number of colors for quantization | `32-96` if enabled; `24` can be too small for facial/detail retention. |
| Palette steps | 2 | `RunQuadRetroCharacterPipeline.ps1` | `256` | Channel quantization steps when no palette | `64-256`; current `256` preserves source. |
| Dither type | 2 | `RunQuadRetroCharacterPipeline.ps1` | `none` wrapper, `bayer4` Python default | Ordered/noise dither pattern | Keep `none` for current source unless runtime dither is off. |
| Dither strength | 2 | `RunQuadRetroCharacterPipeline.ps1` | `0` wrapper, `0.85` Python default | Intensity of baked dither | `0-0.25` for cel source; `0.85` likely too noisy. |
| Transparent dilation | 2 | `t66_quad_retro_character_pipeline.py` | `20` | Fills transparent edge pixels to avoid halos | `8-32`; keep if baking alpha-edge textures. |
| Texture group default | 2 | `Scripts/SetCharacterTextureStreamingDefaults.py` | `TEXTUREGROUP_CHARACTER` | Imported texture LOD behavior | Keep. |
| Sampler filter default | 2 | `Scripts/SetCharacterTextureStreamingDefaults.py` | `TF_NEAREST` | Pixel crispness | Keep. |
| LOD reduction ladder | 2 | `Scripts/QuadRetroCharacterPipelineDefaults.py` | `1.0/0.4/0.15/0.05` | Mesh simplification by LOD | Current ladder is reasonable; inspect facial-feature survival at distance. |

## Layer 3 - Retro FX Runtime

### Subsystems And Settings

Runtime Retro FX is managed by:

- `Source/T66/Core/T66RetroFXSubsystem.cpp`
- `Source/T66/Core/T66RetroFXSubsystem.h`
- `Source/T66/Core/T66PixelationSubsystem.cpp`
- `Source/T66/Core/T66PixelationSubsystem.h`
- `Source/T66/Core/T66RetroFXSettings.h`

`UT66RetroFXSubsystem` creates a high-priority unbound post-process volume (`Priority=5000`) for Retro FX blendables. `UT66PixelationSubsystem` manages a separate pixelation post-process material path `/Game/UI/M_PixelationPostProcess`.

Default `UT66RetroFXSettings` values matter because several effects are enabled or partially weighted even when their obvious "blend" slider is zero:

- `bEnableRetroFXMaster=true`
- `PS1BlendPercent=0`
- `PS1DitheringPercent=100`
- `PS1ColorLUTPercent=100`
- `PS1ColorBoostPercent=25`
- `PS1FogPercent=100`
- `PS1FogDensityPercent=35`
- `TargetResolutionHeightPercent=60`
- `T66PixelationPercent=0`
- `WorldPixelationPercent=0`
- `CharacterPixelationPercent=0`
- `bEnableCharacterOutline=true`
- `bEnableWorldGeometry=false`
- `bEnableCharacterGeometry=false`
- all world/character vertex snap, vertex noise, and affine blend percents default `0`

Because `ApplyBlendableWeights` uses the max of PS1 blend and fog contribution, the default fog settings can give the PS1 post-process material a nonzero effective weight even when `PS1BlendPercent=0`. That means runtime dither/color/fog should be audited visually from the staged save/settings state before assuming the PS1 stack is fully off.

### Custom Depth / Stencil

`UT66RetroFXSubsystem::ApplyPixelationStencilMasks` enables custom depth with `r.CustomDepth 3` when needed. It assigns:

- world stencil value `1`
- character stencil value `2`

Character meshes are classified by asset path prefix `/Game/Characters/`. Because `bEnableCharacterOutline=true` by default, character custom depth/stencil can remain enabled even when character pixelation is zero.

`M_T66_OutlinePostProcess` consumes stencil value `2` by default.

### Vertex Snap And Affine Geometry

No separate vertex-snap material function was found as the authoritative control point in this audit. The implemented path is through Retro geometry replacement materials plus material parameter collection values applied by `UT66RetroFXSubsystem::ApplyGeometryCollection` and `ApplyGeometryMaterials`.

The current default state disables both world and character geometry effects:

- `bEnableWorldGeometry=false`
- `bEnableCharacterGeometry=false`
- snap percent `0`
- vertex noise percent `0`
- affine blend percent `0`

When enabled, the subsystem maps settings to material parameter collection values:

- snap strength: safe nonzero floor then up to about `4`
- snap target resolution: inverse mapping from about `1080` to `120`
- vertex noise amplitude: `0-8`
- affine blend: `0-1`
- affine distance controls: near/mid/far ranges mapped from percent settings

### Runtime Stack Order

Practical runtime order:

1. Gameplay neutral post-process from `T66WorldVisualSetup`.
2. Retro FX high-priority post-process volume with PS1, N64, outline, and chromatic blendables.
3. Pixelation subsystem post-process material when pixelation levels are nonzero.
4. Real low-resolution mode through `r.ScreenPercentage` only if `bUseRealLowResolution=true`.

The exact final composite order between multiple post-process volumes/materials still depends on Unreal's blendable insertion and material domain order, but the subsystem intentionally creates the Retro FX volume at high priority and places the outline material in that stack.

### Layer 3 Knob Table

| Knob | Layer | File / location | Current value | What it controls | Sensible tuning range |
|---|---|---|---|---|---|
| Retro FX master | 3 | `T66RetroFXSettings.h` | `true` | Global runtime retro stack enable | Binary; useful for A/B screenshots. |
| PS1 blend | 3 | `T66RetroFXSettings.h` | `0%` | Direct PS1 post-process blend | `0-40%` for readable gameplay. |
| PS1 dithering | 3 | `T66RetroFXSettings.h` / `ApplyPs1Parameters` | `100%`, maps to strength `3` | Runtime dither intensity | `0-50%` if baked dither is absent; lower if baked dither returns. |
| PS1 Bayer dithering | 3 | `T66RetroFXSettings.h` | `0%` | Bayer-specific ordered dither amount | `0-40%`; avoid with baked Bayer. |
| PS1 color LUT | 3 | `T66RetroFXSettings.h` | `100%` | Runtime palette/color lookup effect | `0-100%`; verify against cel source colors. |
| PS1 color boost | 3 | `T66RetroFXSettings.h` | `25%`, maps about `1.75` | Runtime color saturation/boost in PS1 material | `0-35%`; too high can posterize. |
| PS1 fog | 3 | `T66RetroFXSettings.h` | `100%` | Enables fog contribution to PS1 material weight | `0-60%` for combat readability. |
| PS1 fog density | 3 | `T66RetroFXSettings.h` | `35%` | Fog strength and effective PS1 stack weight | `0-35%`; higher can wash out distant mobs. |
| Target resolution height | 3 | `T66RetroFXSettings.h` / `GetTargetResolutionHeight` | `60%`, maps about `504` px | Runtime low-resolution target height for resolution effects | `360-720` px equivalent. |
| Real low resolution | 3 | `T66RetroFXSettings.h` / `ApplyResolutionRuntime` | `false` | Uses `r.ScreenPercentage` instead of fake material low-res | Usually false; test true only for final platform profile. |
| N64 blur blend | 3 | `T66RetroFXSettings.h` | `0%` | N64 blur pass weight | `0-25%`; blur fights crisp cel edges. |
| N64 blur steps | 3 | `T66RetroFXSettings.h` | `35%`, maps up to 12 steps when active | Blur sampling count | Low values only if enabled. |
| Chromatic strength | 3 | `T66RetroFXSettings.h` | `0%` | RGB channel offset | `0-10%`; higher harms fine facial marks. |
| Chromatic distortion | 3 | `T66RetroFXSettings.h` | `0%` | Screen distortion amount | `0-10%`; avoid in combat readability pass. |
| T66 pixelation | 3 | `T66RetroFXSettings.h` / `T66PixelationSubsystem` | `0%` | Legacy/global pixelation level | `0-30%`; prefer world/character-specific controls. |
| World pixelation | 3 | `T66RetroFXSettings.h` / `T66PixelationSubsystem` | `0%` | Pixelates world stencil group | `0-40%`. |
| Character pixelation | 3 | `T66RetroFXSettings.h` / `T66PixelationSubsystem` | `0%` | Pixelates character stencil group | `0-25%`; keep low for mob readability. |
| Pixel grid size | 3 | `T66PixelationSubsystem.cpp` | level 1=`680`, level 10=`320`; weight off at 0 | Effective pixelation resolution | Keep characters closer to 680 than 320 if enabled. |
| Character outline toggle | 3 | `T66RetroFXSettings.h` | `true` | Enables stencil outline post-process | Keep true for visual lock A/B; compare against source outlines. |
| Outline thickness | 3 | `M_T66_OutlinePostProcess` | `1.5` | Screen-space outline width | `1-4` px. |
| Outline color | 3 | `M_T66_OutlinePostProcess` | black | Outline color | black or very dark cool tone. |
| World geometry enable | 3 | `T66RetroFXSettings.h` | `false` | Enables world retro geometry replacement | Off for visual audit baseline. |
| Character geometry enable | 3 | `T66RetroFXSettings.h` | `false` | Enables character retro geometry replacement | Off until silhouette/facial feature A/B is run. |
| Character vertex snap | 3 | `T66RetroFXSettings.h` / Retro geometry MPC | `0%` | PSX-style position quantization for characters | `0-20%`; high values can break cel silhouettes. |
| Character vertex noise | 3 | `T66RetroFXSettings.h` / Retro geometry MPC | `0%` | Vertex jitter/noise | `0-10%`; likely off for discernable target. |
| Character affine blend | 3 | `T66RetroFXSettings.h` / Retro geometry MPC | `0%` | Perspective-incorrect texture/geometry style | `0-20%`; verify on faces. |

## Cross-Layer Synthesis

### Data Flow

1. **Source - Pixal3D**
   - Pixal3D generated cel-shaded concept/source textures and 3D meshes for the 50-mob roster.
   - Pixal3D export notes include decimation/remesh around 30k triangles, which explains current source mesh density.
   - Primary knobs: Pixal3D prompt/source generation, export decimation/remesh settings, source texture resolution.

2. **Pipeline - Layer 2**
   - The full Quad Retro Blender pipeline exists but was not used on the 50 production mobs.
   - Actual current path was Blender 5.1 format conversion from GLB to FBX plus PNG extraction.
   - Primary knobs if enabled later: `TargetQuads`, `TextureSize`, luminance normalization, palette mode/size, dither type/strength.

3. **Import**
   - Unreal imports `/Game/Characters/Mobs/<MobName>/SM_<MobName>` and `/Game/Characters/Mobs/<MobName>/T_<MobName>`.
   - Texture defaults are applied: `TEXTUREGROUP_CHARACTER`, `TF_NEAREST`, mips from texture group, streaming eligible.
   - UE LOD ladder is generated after import.
   - Primary knobs: texture group/filter/mips, LOD reduction ladder, material assignment.

4. **Materials - Layer 1**
   - Runtime code applies `MI_GLB_Unlit_Character_Shared` through per-mob DMI texture override.
   - Current production mobs are light-independent because the material path is unlit.
   - Track 2 material `M_GLB_ViewSpaceLit_Character` is available but not active.
   - Primary knobs: active parent material, brightness, tint, texture aliases, view-space cel/rim settings if switched.

5. **Lighting / Camera / Color - Layer 1**
   - Runtime strips sky, lights, atmosphere, fog, and legacy sky actors.
   - Exposure is fixed; bloom and ambient occlusion are neutralized.
   - Camera distance/pitch/zoom strongly determines how much mob detail survives on screen.
   - Primary knobs: camera arm length/pitch/FOV, saturation, stage color grading, stripped-light policy.

6. **Retro FX - Layer 3**
   - Retro FX post-process stack can apply PS1 dither/color/fog, N64 blur, chromatic distortion, stencil outline, fake/real low resolution, and geometry effects.
   - Pixelation subsystem can pixelate global/world/character masks but defaults to off.
   - Outline defaults to on through character stencil value `2`.
   - Primary knobs: PS1 fog/dither/color values, outline thickness/opacity, target resolution height, world/character pixelation, vertex snap/noise/affine values.

7. **Output**
   - Final pixel is currently dominated by high-res cel source texture, unlit material, fixed exposure, camera framing, neutral saturation, and any active Retro FX save/settings state.

### Conflicts

| Conflict | Current state | Risk | Responsible layer(s) | Visual-lock implication |
|---|---|---|---|---|
| Baked dither plus runtime dither | Baked dither is absent on the 50 production mobs; runtime PS1 dithering defaults are high in settings, with effective visibility dependent on stack weight/save state. | Future baked dither would stack with runtime dither and create competing patterns. | Layer 2 and Layer 3 | Keep Layer 2 dither off until runtime dither target is chosen, or disable runtime dither for baked-texture tests. |
| Baked pixelation plus runtime pixelation | Baked downsample is absent; runtime T66/world/character pixelation defaults are `0`. | Future `512` or `256` baked textures plus runtime pixelation can over-reduce facial features. | Layer 2 and Layer 3 | Pick one primary pixelation layer. For current cel mobs, runtime-only is safer for A/B because it is reversible. |
| Unlit material plus stripped lighting | Current setup is internally consistent: no lights, unlit mobs. | It preserves source colors but provides no in-engine cel light separation or rim cues. | Layer 1 | Track 2 view-space lit material is now testable and may improve silhouette/readability without restoring world lights. |
| Source outlines plus post-process outline | Source mobs already have hard black outlines; runtime outline can add a second screen-space edge. | Helps small silhouettes but can over-thicken or muddy clustered mobs. | Layer 1 and Layer 3 | A/B `OutlineThickness=1.0-2.0` against source-only closeups before locking. |
| Runtime fog/color effects plus discernability target | PS1 fog defaults can contribute nonzero stack weight even when PS1 blend is zero. | Fog/dither/color LUT can wash out or pattern over already-stylized mobs. | Layer 3 | Verify staged settings and consider making PS1 fog/dither an explicit visual-lock variable. |
| Camera distance versus single-feature mob design | Default/chase camera frames mobs small in combat. | Single facial feature and hard outlines may disappear at gameplay scale even if closeups look good. | Layer 1 | Visual lock must include both gallery closeups and stage combat screenshots. |
| Dusk sky design context versus current runtime | Current gameplay strips Sky Atmosphere and legacy Eclipse actors. | A dusk aesthetic cannot currently be evaluated through sky/lighting; only materials/post-process can carry it. | Layer 1 | Decide whether dusk is still a target. If yes, it conflicts with current unlit-first runtime policy. |

## Current State vs Target Aesthetic

Target: cel-shaded mobs with hard black outlines, flat 2-color fills, 2-tone shading, a single facial feature per mob, and an ULTRAKILL / Megabonk-style "retro but clearly discernable, not broken" presentation.

### Where The Current Setup Matches

- All 50 production mobs are imported in `/Game/Characters/Mobs/`.
- The active source textures are high-resolution cel-shaded textures, not the old dark TRELLIS source.
- Hard source outlines and flat fills are preserved because the import did not run destructive baked dither/downsample/quantization.
- `TF_NEAREST` and `TEXTUREGROUP_CHARACTER` preserve crisp sampling conventions.
- The unlit material avoids the earlier world-light darkness problem.
- The Track 2 screen-space outline material is present and connected to runtime settings.
- Runtime screenshots show distinct colored mobs in the gallery wing, with silhouettes readable at medium distance.

### Where It Diverges

- The 50 production mobs are not actually processed by the Quad Retro Blender pipeline, despite the path/report names using QuadRetro import terminology.
- The active production material is still the simple unlit shared material, not the Track 2 view-space cel/rim material.
- The current gameplay world does not have the prompt's Eclipse dusk sky/atmosphere look; sky/lighting/fog are stripped.
- PS1 runtime fog/dither/color settings can still affect final output and may compete with the clean cel source.
- Gameplay camera framing can make mobs too small for single-feature readability, especially in combat.
- Closeups prove the source is more readable than old assets, but stage screenshots still show readability pressure from distance, walls, and top-down framing.

### Highest-Value Next A/B Tests For Visual Lock

These are audit conclusions, not changes made in this pass:

1. Current unlit shared material versus `M_GLB_ViewSpaceLit_Character` on several of the new production mobs.
2. Outline off/source-only versus outline thickness `1.0`, `1.5`, and `2.0`.
3. Runtime PS1 fog/dither off versus current/default staged settings.
4. Runtime pixelation only, with Layer 2 baked pixelation still off.
5. Camera chase preset arm length/pitch variants for combat readability.

## Ground-Truth Screenshots

All screenshots were captured from:

`C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`

The pinned taskbar shortcut was verified to target that staged executable:

`C:\Users\DoPra\AppData\Roaming\Microsoft\Internet Explorer\Quick Launch\User Pinned\TaskBar\T66 Standalone.lnk`

Capture path used `/Game/Maps/GameplayLevel` with no `?listen`, matching the import report's SteamSockets note. Automation used `-T66GameplayAutoScreenshot` with windowed `1920x1080` output.

### Floor 1 / Gallery

Gallery overview:

![Floor 1 gallery overview](screenshots/gallery_floor1_overview.png)

Enemy gallery wing with production mobs:

![Floor 1 enemy gallery wing](screenshots/gallery_enemies_wing.png)

### Stage 1 Gameplay Frame

Stage gameplay capture used no-listen standalone launch and a camera placement command. The log for this run reported tower terrain activation and initial enemies spawned/alive before the screenshot was captured.

![Stage 1 combat frame](screenshots/stage1_combat.png)

### Closeups

MushroomBrute / cluster-style mob framing:

![MushroomBrute closeup](screenshots/closeup_MushroomBrute.png)

BoneWalker / humanoid-style framing:

![BoneWalker closeup](screenshots/closeup_BoneWalker.png)

Hellhound / quadruped-style framing:

![Hellhound closeup](screenshots/closeup_Hellhound.png)

## Pending Issues Created

None.

No new `pending_issues_<foldername>.md` file was created during this read-only audit. The only audit limitation that affected screenshot capture is already represented by the existing gameplay pending issue for the lack of an automated main-board enemy wave smoke hook. This pass worked around it with staged standalone screenshot commands and log-verified spawn evidence rather than adding a duplicate pending issue.
