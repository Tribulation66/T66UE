# Pixal3D ToonStyle Production Import

Use this doc for any Pixal3D model that will replace or enter playable T66
content. This is the production-cleared path for interactables, props, mobs,
and ToonStyle production assets.

Do not use legacy Pixal3D import scripts, one-off ToonStyle imports, or manual
material assignment for production assets. The manifest-driven wrapper is the
canonical entrypoint.

## Required Read Order

1. `00_MODEL_GENERATION_ROUTING_INSTRUCTIONS.md`
2. `02_SOURCE_IMAGE_RULES_INSTRUCTIONS.md`
3. `../Pixal3D/PIXAL3D_PIPELINE_REFERENCE.md`
4. This document
5. `07_PIXAL3D_RUNPOD_SETUP_INSTRUCTIONS.md`
6. `08_PIXAL3D_TROUBLESHOOTING_INSTRUCTIONS.md` if generation/export fails

## Canonical Entrypoint

Production Pixal3D replacement work is manifest-driven:

```powershell
python "Model Generation/Pixal3D/Scripts/run_pixal3d_toonstyle_production_import.py" validate `
  --manifest "Model Generation/Pixal3D/production_asset_replacement_manifest.json"
```

The checked-in manifest intentionally contains one template row. `validate`
without `--allow-template` must fail until that template row is replaced or
removed. Use `--allow-template validate` only to sanity-check the repository
template itself; do not use `--allow-template` for a real production run.

The wrapper owns the whole path:

1. validate the replacement manifest
2. stage source images into a run folder
3. call `run_pixal3d_batch.py` with production settings
4. run `ToonStyle/BlenderScripts/run_toon_pipeline.py --enable-foundation-tools`
5. import through `ToonStyle/Source/ImportPixal3DAsset_Phase1C.py`
6. verify hard ToonStyle bindings and per-asset readbacks
7. write a production import report

Agents may run individual phases for debugging, but the final accepted pass must
end with wrapper `verify` and a report.

Before staging source images, apply
`02_SOURCE_IMAGE_RULES_INSTRUCTIONS.md`, including the Source Image Stop Rule.
If a source image fails luminance, single-subject, background, alpha,
contact-sheet, readability, or visual-quality requirements, stop before staging
or generation and report the failing image and gate to Pablo. Do not manually
edit, crop, brighten, clean up, split, or regenerate the source unless Pablo
explicitly approves a separate correction/regeneration pass.

## Production Generation Settings

Default production request:

```text
X-Seed: 1337
X-Resolution: 1536
X-Texture-Size: 4096
X-Decimation: 200000
X-Remesh: 1
X-Export-Fallback: 1
X-Fallback-Decimation: 80000
X-Safe-Fill-Holes-Fallback: 1
X-SS-Steps: 25
X-SS-Guidance: 7.5
X-Shape-Steps: 25
X-Shape-Guidance: 7.5
X-Tex-Steps: 25
X-Tex-Guidance: 4.0
```

`X-Decimation=200000` is the face target Pablo requested for production
replacement assets. Lower values, no-remesh, or safe-fill-holes output are
fallback states, not silent success states. If any fallback export path is used,
record it in the manifest/report and decide whether the asset is acceptable or
needs source-image regeneration.

## Replacement Manifest

Use `Model Generation/Pixal3D/production_asset_replacement_manifest.json` as the
working manifest and keep it under source control. Each asset row must include:

- `asset_id`: stable lowercase id used for files and UE assets.
- `display_name`: human-readable name.
- `category`: `interactable`, `prop`, `mob`, `environment`, or `hero`.
- `asset_class`: `humanoid`, `creature`, `prop`, or `accepted-limitation`.
- `source_image`: absolute path or repo-relative path to the PNG source image.
- `target_dir`: Unreal content destination such as `/Game/World/Interactables/...`.
- `replaces`: existing Unreal asset path being replaced, or empty for a new asset.
- `target_height`: normalized Unreal height in UU.
- `collision_policy`: `simple`, `complex_as_simple`, `custom_proxy`, or `none`.
- `outline_color`: RGBA values that should be preserved on the outline MID.
- `gameplay_owner`: data table, C++ class, or placement system that owns the runtime reference.
- `production_status`: `planned`, `generated`, `processed`, `imported`, `verified`, or `blocked`.

Do not put source images directly into runtime content folders. Keep source
images in `SourceAssets/...` or `Model Generation/...` and import only the
validated Unreal assets into `/Game/...`.

## Blender ToonStyle Processing

Every production Pixal3D GLB must run:

```powershell
& "C:\Program Files\Blender Foundation\Blender 5.1\blender.exe" --background `
  --python "ToonStyle/BlenderScripts/run_toon_pipeline.py" -- `
  --input "<run>/Outputs/<asset_id>.glb" `
  --working-dir "SourceAssets/ToonStyle/Pixal3D/Production/<asset_id>/Working" `
  --asset-name "<asset_id>" `
  --target-height <target_height> `
  --asset-class <asset_class> `
  --enable-foundation-tools
```

Use `--accepted-limitation` only when the manifest class is
`accepted-limitation`, and explain why. For humanoids, pass `--is-humanoid`.

The Blender stage is responsible for:

- normalized shading mesh
- inverted-hull outline mesh
- production channel layout
- texture flattening and UV-aware padding
- Tint texture generation
- close-the-gap B authoring on outline mesh
- inner-line texture bake
- manifest readbacks

## Unreal Import

Use only:

```text
ToonStyle/Source/ImportPixal3DAsset_Phase1C.py
```

The importer must create or update:

- shading static mesh
- outline static mesh
- `MI_<asset_id>`
- `MI_<asset_id>_Outline`
- BaseColor texture
- Tint texture
- InnerLine texture or `T_InnerLines_DefaultBlack` for accepted-limitation assets

Do not manually assign `M_Toon_Character`, `M_Toon_Character_Outline`, Tint,
or InnerLine parameters in the editor.

## Hard Validators

The wrapper `verify` phase must fail for any non-accepted-limitation asset if:

- generated GLB is missing or zero bytes
- generation headers/status do not record the requested or fallback export path
- Blender manifest is missing
- `foundation_pass.enabled` is not true
- close-the-gap readback has `B_max <= 0` or `B_max > 1`
- humanoid/creature `B_nonzero_fraction <= 0.05`
- prop `B_nonzero_fraction <= 0.01`
- inner-line texture is missing
- inner-line coverage is `<= 0.005` or `>= 0.5`
- Tint texture is missing
- UE verify JSON is missing
- `InnerLineTexture` is unbound or points to the default black texture
- target UE folder differs from the manifest

Accepted-limitation assets must still import cleanly and bind
`/Game/ToonStyle/Textures/T_InnerLines_DefaultBlack` if no inner lines are
authored.

## Runtime Promotion

Replacing the imported mesh in gameplay is separate from importing the asset.
The manifest row must identify the owner:

- data table row
- C++ soft-object path
- Blueprint actor or placement system
- world generation visual prop table

Do not change gameplay references without updating the owning data/code and
recording the change in the production import report.

## Verification Before Done

For production replacement assets, done means:

1. manifest row is `verified`
2. production import report exists
3. all hard validators pass
4. build succeeds if code/data changed
5. staged standalone is refreshed if playable output changed
6. `T66 Standalone.lnk` and the pinned taskbar shortcut point to the staged exe

Visual tuning is a separate pass. Do not block this production import workflow
on subjective line strength, light direction, or HSV tuning unless the asset is
structurally broken.
