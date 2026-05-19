# Phase 0.5 Blender Deep Dive

## Scope

This pass audits the asset pipeline that prepares T66 meshes and textures before Unreal renders them. It starts from the Phase 0 findings and focuses on what the current DCC scripts actually produce, what data they discard, and what would need to change before a ToonStyle Unreal material can pay off.

The central finding is that `Model Generation/Scripts/Core/QuadRetro/t66_quad_retro_character_pipeline.py` is the production character-style post process. The older prompt names (`retro_process.py`, `pixelate.py`, `retopo_and_bake.py`) are not the active pipeline. QuadRetro is currently a retopo, rebake, normalize, pixel-atlas, unlit-GLB pipeline. It does not produce cel-shading support data: no vertex colors, no authored smooth toon normals, no non-pixelated final material branch.

## QuadRetro Entry Points

There are two relevant entry points:

- `Model Generation/Scripts/Core/QuadRetro/t66_quad_retro_character_pipeline.py`
- `Model Generation/Scripts/Core/QuadRetro/RunQuadRetroCharacterPipeline.ps1`

The Python script's built-in example and parser still describe retro defaults: `--texture-size 256`, `--palette-size 24`, `--dither-type bayer4`, and `--dither-strength 0.85` at `t66_quad_retro_character_pipeline.py:7` and parser defaults at `t66_quad_retro_character_pipeline.py:119` through `t66_quad_retro_character_pipeline.py:127`.

The PowerShell wrapper has newer safer defaults: `TextureSize=512`, `PaletteMode=none`, `DitherType=none`, `DitherStrength=0`, `UseNormals=false`, `BakeSize=1024`, and `TargetQuads=12000` at `RunQuadRetroCharacterPipeline.ps1:7` through `RunQuadRetroCharacterPipeline.ps1:24`. The wrapper forwards these values into the Python script at `RunQuadRetroCharacterPipeline.ps1:98` through `RunQuadRetroCharacterPipeline.ps1:115`.

This default mismatch matters. A direct Python invocation can still create Bayer-dithered, palette-quantized output. Wrapper-driven current runs create non-dithered but still downsampled, nearest-sampled pixel textures.

## QuadRetro Walkthrough

### 1. Argument Parsing

`parse_args()` starts at `t66_quad_retro_character_pipeline.py:78`.

Inputs are GLB, GLTF, or FBX (`--input`) and an output directory (`--output-dir`) at `t66_quad_retro_character_pipeline.py:84` through `t66_quad_retro_character_pipeline.py:86`. The script can reuse an existing Quad Remesher FBX via `--retopo-fbx` at `t66_quad_retro_character_pipeline.py:90`.

The important mesh defaults are:

- `--target-quads` default 5000 in Python, usually overridden to 12000 by wrapper.
- `--use-materials=true`, `--use-normals=false`, `--autodetect-hard-edges=true`.
- `--normalize-height=2.0`.
- `--recalculate-normals=true`.
- `--shade-flat=true`.

Those are defined at `t66_quad_retro_character_pipeline.py:91` through `t66_quad_retro_character_pipeline.py:105`.

The important texture defaults are:

- `--bake-size=1024`.
- `--texture-size=256` in Python, usually overridden to 512 by wrapper.
- `--palette-mode=kmeans` in Python, usually overridden to `none`.
- `--dither-type=bayer4` in Python, usually overridden to `none`.

Those are defined at `t66_quad_retro_character_pipeline.py:108` through `t66_quad_retro_character_pipeline.py:127`.

### 2. Scene Reset And Import

`clear_scene()` deletes all existing Blender objects at `t66_quad_retro_character_pipeline.py:141` through `t66_quad_retro_character_pipeline.py:143`.

`import_model()` imports FBX with image search or GLB/GLTF through Blender's glTF importer at `t66_quad_retro_character_pipeline.py:146` through `t66_quad_retro_character_pipeline.py:158`. It returns mesh objects only and fails if no mesh is imported.

Expected source structure is loose: any GLB/GLTF/FBX with at least one mesh. It does not require a skeleton, a specific naming convention, or specific material slots at import time.

### 3. Mesh Collection And Join

`mesh_objects()` and `count_mesh_stats()` collect imported mesh objects and count triangles/quads at `t66_quad_retro_character_pipeline.py:161` through `t66_quad_retro_character_pipeline.py:180`.

`join_meshes()` starts at `t66_quad_retro_character_pipeline.py:198`. If multiple meshes are present, it selects them, runs `bpy.ops.object.join()`, renames the resulting object and mesh data, and returns one object. If only one mesh exists, it only renames it.

There is no explicit parent-clearing in the core QuadRetro script. Some adjacent export scripts do use detach or parent cleanup, but this script's join stage is simpler.

### 4. Height Normalize And Origin Placement

`normalize_to_height()` starts at `t66_quad_retro_character_pipeline.py:217`. It computes world bounds, scales the model to a target height, applies scale, centers X/Y, puts the bottom at Z=0, then applies location, rotation, and scale again.

Output state after this stage is a normalized Blender object. The report records original height, final size, and final min/max bounds.

### 5. Mesh Cleanup And Normal Rewrite

`clean_mesh()` starts at `t66_quad_retro_character_pipeline.py:253`. It enters edit mode, selects all vertices, removes doubles at `merge_distance`, optionally deletes loose geometry, and optionally runs `normals_make_consistent`.

`force_flat_shading()` starts at `t66_quad_retro_character_pipeline.py:272`. It sets every polygon to flat shading by assigning `poly.use_smooth = False`.

This is the first major conflict with ToonStyle. The current default is explicitly flat-shaded. It does not preserve the smooth, artist-guided normal field that Guilty Gear-like toon bands need.

### 6. Optional Quad Remesher Source Decimation

`make_qremesh_source()` starts at `t66_quad_retro_character_pipeline.py:278`. If `--qremesh-source-target-tris` is positive and the source has more triangles than that value, the script duplicates the source and applies a Blender Decimate modifier before sending it to Quad Remesher.

This is optional and defaults to disabled. It is useful for making Quad Remesher cheaper on heavy generated meshes, but it is not a quality-preserving toon-normal stage.

### 7. Quad Remesher Invocation

The default engine path is resolved by `default_qremesh_engine_path()` at `t66_quad_retro_character_pipeline.py:315` through `t66_quad_retro_character_pipeline.py:323`.

`write_qremesh_settings()` starts at `t66_quad_retro_character_pipeline.py:339`. The critical settings:

- `TargetQuadCount` comes from args.
- `UseVertexColorMap=False` is hardcoded at `t66_quad_retro_character_pipeline.py:363`.
- `UseMaterialIds` follows `--use-materials`.
- `UseIndexedNormals` follows `--use-normals`.
- `AutoDetectHardEdges` follows `--autodetect-hard-edges`.

This is a second major ToonStyle conflict. Quad Remesher is explicitly not using vertex colors, and the wrapper default passes `UseNormals=false`.

`run_quad_remesher()` starts at `t66_quad_retro_character_pipeline.py:404`. It exports a temporary FBX, launches `xremesh.exe -s RetopoSettings.txt`, polls the progress file, fails on activation or negative progress codes, then copies the temporary input, output, settings, and progress files into `Working/QuadRemesher`.

### 8. Retopo Import And Axis Fix

`import_retopo()` imports the remeshed FBX and joins meshes at `t66_quad_retro_character_pipeline.py:495` through `t66_quad_retro_character_pipeline.py:503`.

`clear_qremesher_fbx_axis_rotation()` clears the common Quad Remesher FBX Z rotation at `t66_quad_retro_character_pipeline.py:506` through `t66_quad_retro_character_pipeline.py:511`.

### 9. UV Unwrap

`unwrap_smart_project()` starts at `t66_quad_retro_character_pipeline.py:514`. It creates a UV layer if none exists, selects all faces, and runs Blender Smart UV Project with a 66 degree angle limit and 0.012 island margin.

The ToonStyle implication is that UV layout is generated automatically for the rebaked atlas. This is fine for a single diffuse atlas, but not enough for axis-aligned inner-line techniques or deliberately authored color regions.

### 10. Bake Target Material

`create_bake_target_material()` starts at `t66_quad_retro_character_pipeline.py:526`. It creates a new material on the target, creates a target image, wires the image into Principled Base Color, and sets roughness 1 and metallic 0.

### 11. Source Material Preparation

`prepare_source_materials_for_color_bake()` starts at `t66_quad_retro_character_pipeline.py:552`. Its purpose is to make TRELLIS or Pixal3D source materials bake as flat painted color instead of metal or lighting. It sets opaque material behavior, forces diffuse color white, sets Metallic to 0, Roughness to 1, Alpha to 1, and links the first image texture to Base Color if Base Color is not already linked.

This is useful for ToonStyle because it already tries to isolate color from PBR response. It does not, however, simplify the texture into flat regions. Whatever grain, lighting, compression, or generated texture detail exists in the source texture still gets baked.

### 12. Diffuse Bake

`configure_bake_scene()` switches to Cycles, 64 samples, no denoising, and the configured bake margin at `t66_quad_retro_character_pipeline.py:588` through `t66_quad_retro_character_pipeline.py:594`.

`bake_diffuse()` starts at `t66_quad_retro_character_pipeline.py:597`. It creates a 1024 bake target by default, selects source and target, makes target active, and runs a selected-to-active Diffuse bake with pass filter `COLOR` at `t66_quad_retro_character_pipeline.py:616`.

The bake produces the `*_Bake1024.png` texture. This is the cleanest current pipeline texture, but it still inherits any source texture grain from Pixal3D/TRELLIS and any bake/UV sampling artifacts.

### 13. Save And Dilate Texture

`save_image()` writes a PNG at `t66_quad_retro_character_pipeline.py:624` through `t66_quad_retro_character_pipeline.py:628`.

`dilate_transparent_pixels()` fills transparent padding from neighboring pixels at `t66_quad_retro_character_pipeline.py:631` through `t66_quad_retro_character_pipeline.py:675`. This is useful for mip/edge safety and does not intentionally add grain.

### 14. Luminance Normalization

`normalize_texture_luminance()` starts at `t66_quad_retro_character_pipeline.py:821`. It converts the image into an array, builds an alpha/UV mask, computes mean linear luminance, scales mapped pixels toward the configured target luminance, and optionally boosts saturation.

This stage can shift brightness and color balance, but it is not a dither stage. It can make existing noise more visible if scaling increases contrast in dark regions.

### 15. Pixelation, Palette, And Dither

`dither_offset()` starts at `t66_quad_retro_character_pipeline.py:973`. With `dither_type=none` or `dither_strength<=0`, it returns zero. Otherwise it applies Bayer 4x4 or 8x8 ordered offsets.

`make_pixelated_image()` starts at `t66_quad_retro_character_pipeline.py:983`. This stage is always called in `main()`, regardless of dither settings. It creates a new image at `texture_size`, samples the bake using nearest-point source coordinates, optionally builds a k-means palette, optionally applies Bayer dither before palette lookup or per-channel quantization, and returns a new Blender image plus a report.

Important distinction:

- Current wrapper-driven reports show `palette_mode=none`, `dither_type=none`, and `dither_strength=0.0`.
- The Python defaults still enable `kmeans` plus `bayer4`.
- Even with dither off, this function still creates a lower-resolution pixel output if `texture_size < bake_size`.

So the correct clean-output requirement is not just "skip dithering." It is "make clean flat-color output, with pixelation/palette/dither as opt-in post steps."

### 16. Pixel Material Assignment

`assign_pixel_material()` starts at `t66_quad_retro_character_pipeline.py:1033`. It creates a new material named `*_Pixelated_Unlit`, adds one image texture node with `interpolation="Closest"`, and if `unlit_emission_material=true`, wires it through an Emission shader.

This hardwires the final GLB to a nearest-filter pixel texture. It is exactly the opposite of a ToonStyle clean-source material shell.

### 17. GLB Export

`export_glb()` starts at `t66_quad_retro_character_pipeline.py:1060`. It exports only the selected target object as GLB with `export_format="GLB"`, `use_selection=True`, `export_apply=True`, `export_skins=False`, and `export_animations=False`.

The generated sample GLBs I inspected contain `POSITION`, `NORMAL`, and `TEXCOORD_0` attributes only. They do not contain `COLOR_0`. The current export path is therefore not carrying a ToonStyle vertex color payload because the pipeline never authors one.

### 18. QA Renders And JSON Report

`render_qa()` starts at `t66_quad_retro_character_pipeline.py:1113`. It creates simple lit front/right/back/oblique renders.

`main()` starts at `t66_quad_retro_character_pipeline.py:1136`. It creates `Models`, `Textures`, `Renders`, `Reports`, and `Working/QuadRemesher` output folders, then executes the stages above. The hardwired final pixel branch is visible at `t66_quad_retro_character_pipeline.py:1215` through `t66_quad_retro_character_pipeline.py:1218`: create pixelated image, save it, assign pixel material.

The report JSON records raw and retopo counts, all adjustable values, luminance report, Quad Remesher files, and pixel report at `t66_quad_retro_character_pipeline.py:1237` through `t66_quad_retro_character_pipeline.py:1294`.

## Other Blender And DCC Automation

### Generic Blender QA

`Model Generation/Scripts/Core/Blender/blender_glb_qa.py` imports GLB/GLTF/FBX, counts triangles, optionally decimates, renders a QA image, writes metadata, and optionally exports GLB. Key functions are `import_model()` at line 49, `decimate_meshes()` at line 152, and `export_glb()` at line 139.

This is relevant to ToonStyle as a verification harness. It does not author normals, vertex colors, or clean textures.

### Environment Sheet Splitter

`Model Generation/Scripts/Core/Environment/split_theme_module_sheet.py` is a Pillow-based sheet cropper. It crops source concept sheets into per-module images. It does not interact with QuadRetro.

### World NPC / Interactable Retro Batch

`Model Generation/Scripts/Batches/WorldNpcInteractablesRetroBatch01/run_world_npc_interactables_stage02_quad_retro.py` invokes the QuadRetro wrapper per row. Its current command passes `-TextureSize 512`, `-PaletteMode none`, `-DitherType none`, `-DitherStrength 0`, and `-BakeSize 1024` at lines 189 through 208.

`export_world_npc_interactables_retro_batch01_unreal_ready.py` then imports QuadRetro GLBs, detaches parent transforms, applies transforms, joins meshes, normalizes, exports FBX, and copies the pixel texture. It prefers `pixelated_texture` and falls back to `baked_texture` at lines 219 through 226.

This is directly relevant to ToonStyle because it proves downstream tooling assumes a `Pixelated_512` output exists.

### Environment Kit Automation

`Model Generation/Scripts/Batches/Environment/CoherentThemeKit01/export_coherent_themekit_unreal_ready.py` imports generated GLBs, normalizes wall/floor modules, exports both GLB and FBX, and saves the first base-color texture. Key stages are import at line 129, wall/floor normalize at lines 187 and 230, GLB export at line 264, FBX export at line 278, and texture save at line 306.

This path is not QuadRetro. It may be a better environment test path for ToonStyle than character assets because it is closer to generated level modules.

### Weapons And Interactables

`export_weapon_projectiles_unreal_ready.py` and `export_interactable_batch01_unreal_ready.py` import GLBs, normalize transforms, export FBX, and save base-color textures. These are useful examples of non-QuadRetro DCC scripts, but they are not the core character look pipeline.

### Pixal3D Research Pipeline

`Model Generation/Pixal3D/PIXAL3D_PIPELINE_REFERENCE.md` explicitly keeps Pixal3D separate from TRELLIS at lines 4 through 10. The server exposes `/health` and `/generate`, accepts texture size, decimation, and remesh headers, and exports GLBs through worker-isolated fallback at `Pixal3D/Server/pixal3d_server.py:402` through `pixal3d_server.py:580`.

`run_pixal3d_smoke.py` can optionally call QuadRetro after Pixal3D output. The optional path starts around `run_pixal3d_smoke.py:595` and forwards `--retro-texture-size` at `run_pixal3d_smoke.py:622`. This confirms Pixal3D and QuadRetro are separate pipeline stages.

Pixal3D is relevant because its GLBs can already contain textured meshes with source texture grain before QuadRetro touches them. It is not the same system as `UT66PixelationSubsystem`.

## UE5 Import Path

`Scripts/ImportStaticMeshes.py` is the shared static mesh importer. Default static build settings set full precision UVs true, generate lightmap UVs false, recompute normals false, and recompute tangents false at `Scripts/ImportStaticMeshes.py:94` through `Scripts/ImportStaticMeshes.py:100`.

`import_glb()` starts at `Scripts/ImportStaticMeshes.py:233`. For FBX it creates `FbxImportUI`, imports as static mesh, disables material/texture/animation import, combines meshes, disables auto collision, and disables generated lightmap UVs at `Scripts/ImportStaticMeshes.py:248` through `Scripts/ImportStaticMeshes.py:267`. For GLB, it leaves Interchange options implicit.

After import, GLB assets are converted to unlit through `MakeGLBImportsUnlit.convert_glb_imports_unlit()` at `Scripts/ImportStaticMeshes.py:705` through `Scripts/ImportStaticMeshes.py:710`, then build settings are applied at `Scripts/ImportStaticMeshes.py:713`.

`Scripts/QuadRetroCharacterPipelineDefaults.py` applies character texture defaults. It sets the texture LOD group to character, tries to set nearest filtering, tries to keep default streaming, disables `never_stream`, disables virtual texture streaming, and changes `NO_MIPMAPS` back to "from texture group" at `Scripts/QuadRetroCharacterPipelineDefaults.py:137` through `Scripts/QuadRetroCharacterPipelineDefaults.py:232`.

The import path is favorable for custom normals on static meshes because recompute normals/tangents are false. It is not currently favorable for ToonStyle vertex colors because the current GLBs do not contain `COLOR_0`, and no importer path validates or consumes a vertex color layout.

Skeletal imports are separate. `Scripts/ImportCanonicalHeroMeshes.py` imports FBX as skeletal mesh and does not explicitly set normal/tangent import method at lines 48 through 75. The live `Hero_1_Chad` row is now skeletal and animated, so a ToonStyle character test on that row includes more risk than a static QuadRetro NPC.

## Vertex Color Authoring Feasibility

The current QuadRetro pipeline does not preserve or create a ToonStyle vertex color payload. Evidence:

- Quad Remesher settings hardcode `UseVertexColorMap=False` at `t66_quad_retro_character_pipeline.py:363`.
- The sample QuadRetro GLBs contain only `POSITION`, `NORMAL`, and `TEXCOORD_0`, with no `COLOR_0`.
- No project utility currently authors AO-to-vertex-color data for QuadRetro.

Adding vertex color authoring is feasible but it is new work. The cleanest fit is after retopo import and UV unwrap, before GLB export. At that point the final target topology exists, the material bake has not yet forced the final unlit/pixel material, and Blender can bake AO or assign derived per-vertex defaults to the final mesh.

A practical first layout is one RGBA color attribute:

- R: shade threshold offset or AO-derived "shade easier" value.
- G: outline width multiplier.
- B: outline depth/push or joint suppression.
- A: outline mask or region default.

For an AI-driven pipeline, "authored" should initially mean deterministic defaults plus derived values, not hand paint. R can come from AO bake or curvature. G/B/A can start as uniform defaults with optional heuristics for thin parts, face, hands, or joints later.

GLB should preserve vertex colors if Blender exports a mesh with color attributes, but this must be verified with an explicit `COLOR_0` inspection and Unreal import probe. The current pipeline gives no proof because it never creates the attribute.

## Custom Normal Transfer Feasibility

A Data Transfer Modifier with a proxy sphere or simpler proxy mesh should fit after retopo import and before forced flat shading/material export. The current `force_flat_shading()` default at `t66_quad_retro_character_pipeline.py:272` conflicts with this and should become conditional.

The modifier should be applied before GLB export. A GLB will carry final normal data, not an editable Blender modifier stack. Leaving the modifier unapplied is not a safe export strategy.

I found no existing proxy-normal utility in the project. This is not a small one-line patch, but it also does not require a full pipeline rewrite. It is a medium scoped addition:

- Add a toon-normal mode/branch.
- Create or derive a proxy object.
- Add and apply Data Transfer for normals.
- Disable flat-shade rewrite for that branch.
- Export and verify `NORMAL` data in GLB.
- Confirm Unreal static import preserves the normals with recompute disabled.

The harder part is not code volume. The hard part is choosing proxy generation rules that work across generated bodies, robes, hair, props, and non-humanoid mobs.

## Non-Pixelated Branch Feasibility

Phase 0 suggested a non-pixelated output mode. That suggestion is correct but the better framing is "clean flat-color output with optional pixelation as an opt-in." The word "non-pixelated" is too narrow because ToonStyle needs more than disabling Bayer dither:

- Do not palette-quantize unless explicitly requested.
- Do not downsample the bake unless explicitly requested.
- Do not force image texture interpolation to `Closest` unless explicitly requested.
- Do not name and bind the final output as `*_Pixelated_*` in clean mode.
- Preserve or generate normals and vertex colors before export.

A backward-compatible implementation can keep the current default wrapper behavior for retro production assets and add a new mode such as `--style-mode clean-toon` or `--texture-output clean`. Existing assets would not break if the default remains `retro` for old callers and ToonStyle uses an explicit new branch.

The risky part is downstream assumptions. Importers and data rows often look for `Pixelated_512`, for example `ImportQuadRetroHeroVisuals.py:89`, `ImportQuadRetroBossVisuals.py:67`, and `QuadRetroCharacterPipelineDefaults.py:376`. A clean branch needs a new texture naming convention and importer binding path rather than silently replacing `Pixelated_512`.

## Documentation Drift

These docs should be updated after the ToonStyle direction is decided:

- `Gameplay/World/T66_LIGHTING_REFERENCE.md`: Phase 0 already found it stale. It still presents lighting/postprocess state as a living runtime reference, but live renderer state now includes deferred rendering, Substrate, no AA, no Lumen GI, real low-resolution Retro FX defaults, and a decommissioned or neutralized atmosphere path. Its own maintenance rule at line 7 says it should be updated when runtime visual setup changes.
- `Model Generation/Instructions/03_QUAD_RETRO_PIPELINE_INSTRUCTIONS.md`: the run command correctly recommends `PaletteMode none`, `DitherType none`, and `DitherStrength 0`, but the Python script still has dithered defaults. The doc should explicitly warn against direct Python defaults or the script should be changed.
- `Model Generation/Instructions/README.md` and `00_MODEL_GENERATION_ROUTING_INSTRUCTIONS.md`: routing is broadly correct, but once ToonStyle exists, they need a new clean-toon route rather than treating QuadRetro as the only character post-process.
- `Model Generation/Pixal3D/PIXAL3D_PIPELINE_REFERENCE.md`: still correctly treats Pixal3D as separate research. If Pixal3D becomes a ToonStyle source path, it needs a section explaining clean texture requirements and when to run QuadRetro/ToonStyle post-process.
- `Content/Materials/pending_issues_Materials.md`: the retained `M_GLB_ViewSpaceLit_Character` issue should be resolved after Phase 1 decides whether it is a prototype source, an A/B material, or a dead artifact.
- `ToonStyle/Docs/README.md`: should be updated after Pablo chooses the Phase 1 test pair and pipeline branch.

## Bottom Line

QuadRetro is not just "retopo plus texture bake." Its final authored product is an unlit, flat-shaded, nearest-sampled, pixel-atlas GLB. Current wrapper settings disable ordered dither, but the final result is still structurally retro. ToonStyle needs a clean branch in this pipeline before Unreal shader work can produce a high-fidelity cel look.
