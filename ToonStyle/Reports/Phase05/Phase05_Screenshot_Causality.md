# Phase 0.5 Screenshot Causality

## Conclusion

The visible grain on current generated characters is primarily asset-side, not a PS1 dither pass in Unreal.

There are three separate contributors:

1. Pixal3D/TRELLIS source textures can already contain high-frequency generated texture noise before QuadRetro runs.
2. QuadRetro always writes a downsampled `*_Pixelated_512.png` style atlas and binds it through a nearest-filter unlit material. Current wrapper-driven runs do not Bayer-dither, but they still preserve and often amplify source texture noise by rebaking and nearest downsampling.
3. Runtime Retro FX fresh defaults do not enable PS1 dither, PS1 blend, world pixelation, or character pixelation weights. However, fresh defaults do enable real low resolution when the Retro FX master is true, which can make existing texture noise look rougher on screen.

If Pablo saw a true ordered Bayer/checker dither pattern, that came either from an older/direct Python QuadRetro invocation using the script defaults, or from a saved runtime/user setting that enabled PS1 dither/Bayer. The current wrapper reports I inspected do not support "current wrapper adds Bayer dither" as the cause.

## Evidence From QuadRetro Code

The Python script still has retro defaults:

- `palette_mode=kmeans` at `Model Generation/Scripts/Core/QuadRetro/t66_quad_retro_character_pipeline.py:120`.
- `palette_size=24` at `t66_quad_retro_character_pipeline.py:121`.
- `dither_type=bayer4` at `t66_quad_retro_character_pipeline.py:125`.
- `dither_strength=0.85` at `t66_quad_retro_character_pipeline.py:126`.

The wrapper overrides those defaults:

- `PaletteMode="none"` at `Model Generation/Scripts/Core/QuadRetro/RunQuadRetroCharacterPipeline.ps1:20`.
- `DitherType="none"` at `RunQuadRetroCharacterPipeline.ps1:23`.
- `DitherStrength=0` at `RunQuadRetroCharacterPipeline.ps1:24`.

The current World NPC batch runner also passes `-PaletteMode none`, `-DitherType none`, and `-DitherStrength 0` at `Model Generation/Scripts/Batches/WorldNpcInteractablesRetroBatch01/run_world_npc_interactables_stage02_quad_retro.py:189` through `run_world_npc_interactables_stage02_quad_retro.py:208`.

The pixel branch itself is still hardwired:

- `make_pixelated_image()` starts at `t66_quad_retro_character_pipeline.py:983`.
- It always creates a new target image at `texture_size`.
- It nearest-samples from the bake.
- It only applies ordered dither if `dither_type` is not `none` and `dither_strength > 0`; that guard is in `dither_offset()` at `t66_quad_retro_character_pipeline.py:973`.
- `main()` always calls `make_pixelated_image`, `save_image(pixel_path)`, and `assign_pixel_material()` at `t66_quad_retro_character_pipeline.py:1215` through `t66_quad_retro_character_pipeline.py:1218`.
- `assign_pixel_material()` forces image interpolation to `Closest` at `t66_quad_retro_character_pipeline.py:1043`.

So current wrapper-driven QuadRetro output is not Bayer-dithered, but it is still a pixel-texture output.

## Evidence From Current QuadRetro Reports

I inspected two current QuadRetro reports:

- `Model Generation/Runs/Pixal3D/HeroArthur01/Post/QuadRetro/arthur_royal_chad/Reports/arthur_royal_chad_QuadRetro_report.json`
- `Model Generation/Experiments/Pixal3D_Goblin_Characterization/PostProcessed/QuadRetro/Variant_A/Reports/Variant_A_QuadRetro_report.json`

Both reports show:

- `texture_size: 512`
- `palette_mode: none`
- `palette_size: 0`
- `dither_type: none`
- `dither_strength: 0.0`
- `pixel_report.palette_count: 0`

Arthur's report still writes `arthur_royal_chad_QuadRetro_Pixelated_512.png`. Variant A still writes `Variant_A_QuadRetro_Pixelated_512.png`. The artifact source is therefore not ordered dither in these runs; it is the baked/source texture content plus the hardwired pixel atlas step.

## Pixel-Level Texture Evidence

I inspected image files directly with Pillow and parsed embedded GLB textures without writing any extracted files.

### Pixal3D Hi-Fi Rush Slime Run

Source image:

- Path: `Model Generation/Runs/Pixal3D/PIXALSLIME_HIFIRUSH_2026-05-16/Sources/PIXALSLIME_HIFIRUSH.png`
- Size: 1024 x 1024
- Unique RGB colors: 122,572
- Mean neighbor RGB delta: 3.206

Embedded GLB color texture:

- GLB: `Model Generation/Runs/Pixal3D/PIXALSLIME_HIFIRUSH_2026-05-16/Outputs/PIXALSLIME_HIFIRUSH.glb`
- Embedded image 0 MIME: `image/webp`
- Size: 2048 x 2048
- Unique RGB colors: 242,662
- Mean neighbor RGB delta: 4.534
- Neighbor deltas over 2: 47.74 percent

This proves the sampled Pixal3D output already contains high-frequency texture variation inside the GLB. QuadRetro is not required for that run to have texture noise.

### Arthur QuadRetro Run

Bake texture:

- Path: `Model Generation/Runs/Pixal3D/HeroArthur01/Post/QuadRetro/arthur_royal_chad/Textures/arthur_royal_chad_QuadRetro_Bake1024.png`
- Size: 1024 x 1024
- Unique RGB colors: 161,148
- Mean neighbor RGB delta: 7.140

Pixelated texture:

- Path: `Model Generation/Runs/Pixal3D/HeroArthur01/Post/QuadRetro/arthur_royal_chad/Textures/arthur_royal_chad_QuadRetro_Pixelated_512.png`
- Size: 512 x 512
- Unique RGB colors: 77,099
- Mean neighbor RGB delta: 12.330

Embedded GLB texture:

- GLB: `Model Generation/Runs/Pixal3D/HeroArthur01/Post/QuadRetro/arthur_royal_chad/Models/arthur_royal_chad_QuadRetro.glb`
- Embedded image name: `arthur_royal_chad_QuadRetro_Pixelated_512`
- Size: 512 x 512
- Same stats as the PNG above.

The exported GLB stores the pixelated atlas. Unreal is not reconstructing this grain at runtime; it is in the asset texture being bound.

### Variant A QuadRetro Run

Bake texture:

- Path: `Model Generation/Experiments/Pixal3D_Goblin_Characterization/PostProcessed/QuadRetro/Variant_A/Textures/Variant_A_QuadRetro_Bake1024.png`
- Size: 1024 x 1024
- Unique RGB colors: 35,369
- Mean neighbor RGB delta: 3.708

Pixelated texture:

- Path: `Model Generation/Experiments/Pixal3D_Goblin_Characterization/PostProcessed/QuadRetro/Variant_A/Textures/Variant_A_QuadRetro_Pixelated_512.png`
- Size: 512 x 512
- Unique RGB colors: 16,841
- Mean neighbor RGB delta: 6.132

Embedded GLB texture:

- GLB: `Model Generation/Experiments/Pixal3D_Goblin_Characterization/PostProcessed/QuadRetro/Variant_A/Models/Variant_A_QuadRetro.glb`
- Embedded image name: `Variant_A_QuadRetro_Pixelated_512`
- Size: 512 x 512
- Same stats as the PNG above.

Again, the final GLB carries the pixel atlas.

## Unreal Binding Evidence

Runtime character binding uses the pixelated texture path directly.

`FT66CharacterVisualRow` contains `PixelatedTextureAssetPath` at `Source/T66/Data/T66DataTypes.h:1747`.

For static QuadRetro characters, `T66ApplyQuadRetroStaticMaterialOverrides()` creates dynamic materials and binds the texture to `EmissiveTexture`, `BaseColorTexture`, and `DiffuseColorMap` at `Source/T66/Core/T66CharacterVisualSubsystem.cpp:533` through `Source/T66/Core/T66CharacterVisualSubsystem.cpp:560`.

For static character rows, `ApplyCharacterVisual()` loads `PixelatedTextureAssetPath` and applies the shared QuadRetro material at `T66CharacterVisualSubsystem.cpp:1404` through `T66CharacterVisualSubsystem.cpp:1415`.

For VAT mobs, `ApplyMobVertexAnimationVisual()` also loads `PixelatedTextureAssetPath` and binds it alongside animation textures at `T66CharacterVisualSubsystem.cpp:900` through `T66CharacterVisualSubsystem.cpp:977`.

The data table confirms production rows point at pixel textures. Examples:

- `Hero_1_Chad` points at `/Game/Characters/Heroes/Hero_1/Chad/QuadRetro/RoyalChad_QuadRetro/Textures/RoyalChad_QuadRetro_Pixelated_512` in `Content/Data/CharacterVisuals.csv:2`.
- `Gambler` points at `/Game/Characters/NPCs/Gambler/QuadRetro/Textures/T_SM_Gambler_QuadRetro_Pixelated_512_Normalized` in `Content/Data/CharacterVisuals.csv:77`.

## Runtime Retro FX Evidence

Fresh settings defaults are not visually silent.

`FT66RetroFXSettings` defaults:

- Master enabled: `bEnableRetroFXMaster=true` at `Source/T66/Core/T66RetroFXSettings.h:20`.
- PS1 blend/dither/Bayer/color/fog values are 0 at `T66RetroFXSettings.h:23` through `T66RetroFXSettings.h:49`.
- World and character pixelation values are 0 at `T66RetroFXSettings.h:86` through `T66RetroFXSettings.h:92`.
- Real low resolution is enabled by default at `T66RetroFXSettings.h:53`.
- Target resolution height percent is 40 at `T66RetroFXSettings.h:62`.

`BuildEffectiveSettings()` only zeros everything when master is false. If master is true, it returns the settings unchanged at `Source/T66/Core/T66RetroFXSubsystem.cpp:399` through `T66RetroFXSubsystem.cpp:407`.

`HasPs1StackEnabled()` only turns on the PS1 post-process when PS1 blend/dither/Bayer/LUT/fog values are nonzero at `T66RetroFXSubsystem.cpp:361` through `T66RetroFXSubsystem.cpp:373`. Fresh defaults therefore do not create a PS1 dither blendable.

`ApplyPixelationStencilMasks()` disables world/character pixelation if the pixelation values are zero at `T66RetroFXSubsystem.cpp:1174` through `T66RetroFXSubsystem.cpp:1196`.

`UT66PixelationSubsystem::SetPixelationLevels()` receives 0/0 from fresh defaults and calls `ApplyLevelToBlendable()` without ensuring a blendable at `Source/T66/Core/T66PixelationSubsystem.cpp:80` through `T66PixelationSubsystem.cpp:95`. If a blendable exists, its weight is set to 0 at `T66PixelationSubsystem.cpp:196` through `T66PixelationSubsystem.cpp:219`.

But `ApplyResolutionRuntime()` is unconditional in `ApplySettings()` at `T66RetroFXSubsystem.cpp:770`. Because `bUseRealLowResolution` defaults true, it writes `r.ScreenPercentage` based on target height at `T66RetroFXSubsystem.cpp:1025` through `T66RetroFXSubsystem.cpp:1059`.

So fresh defaults do not add dither or pixelation weights, but they can still render the scene at reduced screen percentage while master is true. That can make asset-side texture grain look harsher.

## Character Asset Taxonomy

### Animated Hero

`Hero_1_Chad` currently uses a skeletal QuadRetro UAL QA mesh and animations in `Content/Data/CharacterVisuals.csv:2`. It still points at the original QuadRetro pixel texture. This is important, but it is not the lowest-risk Phase 1 test because it mixes material, skeletal import, animation, and row compatibility.

### Static QuadRetro Heroes

Most other Chad/Stacy rows use static QuadRetro meshes and normalized `Pixelated_512_Normalized` textures in `Content/Data/CharacterVisuals.csv`. They are easier than animated hero, but changing them risks player-facing hero selection and runtime body presentation.

### Static QuadRetro NPCs

Saint, Ouroboros, and Gambler use static QuadRetro meshes and pixel textures at `Content/Data/CharacterVisuals.csv:75` through `Content/Data/CharacterVisuals.csv:77`. They are a low-risk character-like test surface because they use the production `FT66CharacterVisualRow` path but do not require animation or VAT.

### VAT Mobs

VAT mobs use static meshes plus position/normal animation textures through `FT66MobVertexAnimationRow`. Their binding path is more complex (`T66CharacterVisualSubsystem.cpp:876` through `T66CharacterVisualSubsystem.cpp:994`). They are not a good first ToonStyle test because the animation textures and material expectations are extra variables.

### Pixal3D Display-Only Assets

Pixal3D display assets are spawned around the idol altar/game mode path and are explicitly separate research/display surfaces. They are useful for visual research but should not be the first production ToonStyle test unless Pablo explicitly wants a research-only validation.

### Environment Modules

Generated environment modules under the Coherent Theme Kit path use a different export/import path than QuadRetro. They are the right environment-side test if Phase 1 wants to validate a clean toon environment material separately from character texture cleanup.

## Recommended Phase 1 Test Pair

Recommendation, subject to Pablo approval:

- Character test: `Gambler` static QuadRetro NPC (`Content/Data/CharacterVisuals.csv:77`, `Content/Characters/NPCs/Gambler/QuadRetro/SM_Gambler_QuadRetro.uasset`).
- Environment test: one generated Coherent Theme Kit wall/floor module, preferably a simple dungeon wall/floor that already runs through `M_Environment_Lit` or the static mesh import path.

Why: Gambler exercises the active static character material binding path without skeletal or VAT complexity. A generated wall/floor exercises the environment material path without the character-specific pixel texture assumptions. If the goal is specifically to compare old QuadRetro vs clean ToonStyle on the same DCC source, then a `Crate_QuadRetro` or another world interactable from the World NPC/Interactables batch is also acceptable.

## M_GLB_ViewSpaceLit_Character Evaluation

`Content/Materials/pending_issues_Materials.md:6` says `/Game/Materials/M_GLB_ViewSpaceLit_Character` is retained as a Track 2 master for future A/B testing, while production mob path uses `/Game/Materials/MI_GLB_Unlit_Character_Shared`.

Binary inspection of `Content/Materials/M_GLB_ViewSpaceLit_Character.uasset` shows it is not unrelated. It is an unlit surface material with a Custom expression and toon-like parameters:

- Shading model markers include `MSM_Unlit`, `MD_Surface`, and `BLEND_Opaque`.
- It contains `MaterialExpressionCustom` and `MaterialExpressionPixelNormalWS`.
- Parameters include `LightDirection_ViewSpace`, `NormalWS`, `RampStep1`, `RampStep2`, `ShadowTint`, `MidtoneTint`, `HighlightTint`, `RimColor`, `RimPower`, `RimStrength`, `BaseColorTexture`, `DiffuseColorMap`, and `EmissiveTexture`.
- The embedded Custom HLSL computes `normalVS = normalize(TransformWorldVectorToView(normalize(NormalWS)))`, dots it with `LightDirection_ViewSpace`, selects between shadow/midtone/highlight tints using `RampStep1` and `RampStep2`, adds a rim term, and returns lit color.

Assessment: this is a partial material-contained toon prototype or useful reference, not a dead-end artifact. It should be evaluated in Phase 1 before building an entirely new character master. It should not be promoted directly without solving the DCC issues, shader-source ownership, AA, and parameter delivery questions.

## Final Causality Statement

For current wrapper-generated QuadRetro assets, the phrase "dithered grain" should be split into more precise causes:

- Ordered dither: not present in the inspected current wrapper reports.
- Pixel atlas: present and hardwired.
- Source/generated texture grain: present in sampled Pixal3D output and preserved through QuadRetro.
- Runtime PS1 dither/pixelation: off by fresh scalar defaults.
- Runtime real low resolution: on by fresh defaults when master is true.

The immediate fix is not an Unreal shader tweak. The immediate fix is a clean DCC output branch and a runtime/defaults decision that stops low-resolution presentation from masking whether the source asset is clean.
