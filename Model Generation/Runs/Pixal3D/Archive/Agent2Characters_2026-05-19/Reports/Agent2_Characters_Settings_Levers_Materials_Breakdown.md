# Agent 2 Character Pixal3D ToonStyle Settings, Levers, And Materials Breakdown

Date: 2026-05-19

Purpose: document the exact settings and material pipeline used to generate, process, and import the Agent 2 Pixal3D ToonStyle production character models.

This file is informational only. It does not request generation, import, staging, build, or data rewiring.

## Evidence Files

- Production manifest: `C:\UE\T66\Model Generation\Pixal3D\production_asset_replacement_manifest.json`
- Generation status log: `C:\UE\T66\Model Generation\Runs\Pixal3D\Agent2Characters_2026-05-19\Logs\pixal3d_generation_status.jsonl`
- Production import verifier: `C:\UE\T66\Model Generation\Runs\Pixal3D\Agent2Characters_2026-05-19\Reports\Pixal3D_ToonStyle_Production_Import_Report.json`
- Runtime/data validator: `C:\UE\T66\Saved\Codex\Agent2CharacterVisualsValidation.json`
- Representative Blender manifest: `C:\UE\T66\SourceAssets\ToonStyle\Pixal3D\Production\Hero_1_Chad\Working\Hero_1_Chad_manifest.json`
- Representative Unreal verify JSON: `C:\UE\T66\SourceAssets\ToonStyle\Pixal3D\Production\Hero_1_Chad\Working\Hero_1_Chad_ue_verify.json`
- Blender pipeline script: `C:\UE\T66\ToonStyle\BlenderScripts\run_toon_pipeline.py`
- Unreal importer script: `C:\UE\T66\ToonStyle\Source\ImportPixal3DAsset_Phase1C.py`

## Pixal3D Generation

These manifest settings applied to all 26 Agent 2 character assets.

| Setting | Value |
|---|---:|
| `seed` | `1337` |
| `resolution` | `1536` |
| `image_resolution` | `1536` |
| `texture_size` | `4096` |
| `decimation` | `200000` |
| `fallback_decimation` | `80000` |
| `remesh` | `true` |
| `export_fallback` | `true` |
| `safe_fill_holes_fallback` | `true` |
| `ss_steps` | `25` |
| `ss_guidance` | `7.5` |
| `shape_steps` | `25` |
| `shape_guidance` | `7.5` |
| `tex_steps` | `25` |
| `tex_guidance` | `4.0` |
| `batch_generate_timeout` | `7200` |
| `batch_poll_interval` | `60` |
| `batch_wait_timeout` | `43200` |

Actual server headers in `pixal3d_generation_status.jsonl`:

- `X-Pixal3D-Export-Decimation=200000`
- `X-Pixal3D-Export-Remesh=1`
- `X-Pixal3D-Export-Label=requested`
- `X-Pixal3D-Export-Attempts=1`
- `X-Pixal3D-Export-CPU-UV-Unwraps=0`
- `X-Pixal3D-Export-Safe-Fill-Holes=0`
- `X-Pixal3D-Export-Fill-Holes-Skipped=0`

Result:

- Output format: one `.glb` per asset.
- Output folder: `C:\UE\T66\Model Generation\Runs\Pixal3D\Agent2Characters_2026-05-19\Outputs`
- Fallbacks used: none.

## Blender ToonStyle Processing

The production wrapper called `run_toon_pipeline.py` for each generated GLB.

Common process settings:

- Asset class: `humanoid`
- `--is-humanoid`: enabled
- Target height: `180.0`
- Foundation tools: enabled
- Accepted limitation: not used
- No-remesh export: not used
- Texture flatten color count: `flatten_k=6`
- Highlight cap: `0.85`
- Inner-line texture size: `4096`

Processing sequence:

1. Import Pixal3D GLB.
2. Extract source textures from the GLB.
3. Join source meshes into a single shading mesh.
4. Normalize the model to the manifest target height of `180 cm`.
5. Dump UV triangle data for texture post-processing.
6. Flatten extracted diffuse textures using six-color palette snapping.
7. Run texture post-processing:
   - palette snap
   - speckle cleanup
   - UV padding
   - tint texture generation
8. Strip material texture references from the mesh.
9. Author vertex colors on attribute `Col`.
10. Apply humanoid normal handling.
11. Duplicate the shading mesh into an outline mesh.
12. Reverse outline winding while preserving outward vertex normals.
13. Run ToonStyle foundation tools:
   - close-the-gap B-channel curvature authoring
   - inner-line segment extraction
   - inner-line texture bake
14. Transfer/edit normals for humanoid head/face presentation.
15. Export two FBXs:
   - `<AssetID>.fbx`
   - `<AssetID>_outline.fbx`
16. Write `<AssetID>_manifest.json`.

Representative `Hero_1_Chad` Blender outputs:

- Main FBX: `C:\UE\T66\SourceAssets\ToonStyle\Pixal3D\Production\Hero_1_Chad\Working\Hero_1_Chad.fbx`
- Outline FBX: `C:\UE\T66\SourceAssets\ToonStyle\Pixal3D\Production\Hero_1_Chad\Working\Hero_1_Chad_outline.fbx`
- Flattened base texture: `C:\UE\T66\SourceAssets\ToonStyle\Pixal3D\Production\Hero_1_Chad\Working\Hero_1_Chad_0_flattened.png`
- Tint texture: `C:\UE\T66\SourceAssets\ToonStyle\Pixal3D\Production\Hero_1_Chad\Working\Hero_1_Chad_Tint.png`
- Inner-line texture: `C:\UE\T66\SourceAssets\ToonStyle\Pixal3D\Production\Hero_1_Chad\Working\Hero_1_Chad_InnerLines.png`
- Vertex color attribute: `Col`
- Vertex color domain/type: `CORNER`, `BYTE_COLOR`
- Vertex color mode: `cycles_ao`

Representative texture post-process levers:

- Palette size after flattening: `6`
- Speckle cleanup method: `3x3_local_median_high_contrast_pixel_replacement`
- Speckle cleanup diff threshold: `42.0`
- UV padding: `16 px`
- Tint generation saturation scale: `1.1`
- Tint generation value scale: `0.6`

Representative normal/outline levers:

- Normal transfer method: `sphere_proxy_split_normals`
- Outline winding method: reverse faces after smoothing, then restore captured outward vertex normals as custom split normals.
- For `Hero_1_Chad`, normal transfer changed `136287` loops.

Production validation ranges across all 26 assets:

- Close-the-gap `B_nonzero_fraction`: `0.29998960534978` to `0.30000286754796`
- Inner-line coverage: `0.0823230147361755` to `0.204690158367157`

## Unreal Import

The production wrapper called `ImportPixal3DAsset_Phase1C.py` for every processed asset.

### Static Mesh Import Settings

FBX import settings:

- Import type: static mesh
- Import as skeletal: `false`
- Import mesh: `true`
- Import materials: `false`
- Import textures: `false`
- Import animations: `false`
- Combine meshes: `true`
- Auto-generate collision: `false`
- Generate lightmap UVs: `false`
- Normal import method: import normals and tangents
- Vertex color import option: replace
- Replace existing asset: `true`
- Replace existing settings: `true`

Imported assets per model:

- Main mesh: `<TargetFolder>/SM_<AssetID>.SM_<AssetID>`
- Outline mesh: `<TargetFolder>/SM_<AssetID>_Outline.SM_<AssetID>_Outline`

Height validation:

- Expected height: `180.0`
- Height tolerance: importer verified bounds against expected height.

### Texture Import Settings

Imported texture assets:

- Base texture: `<TargetFolder>/Textures/T_<AssetID>_0`
- Tint texture: `<TargetFolder>/Textures/T_<AssetID>_Tint`
- Inner-line texture: `<TargetFolder>/Textures/T_<AssetID>_InnerLines`

Texture settings:

- Texture group: `TEXTUREGROUP_CHARACTER`
- `sRGB=true`
- Filter: default
- Mip generation: from texture group
- LOD bias: `0`
- Compression settings: default

### Material Setup

Parent materials:

- Character parent material: `/Game/ToonStyle/Materials/M_Toon_Character`
- Outline parent material: `/Game/ToonStyle/Materials/M_Toon_Character_Outline`
- Default black inner-line texture: `/Game/ToonStyle/Textures/T_InnerLines_DefaultBlack`

Created material instances per asset:

- Main material: `<TargetFolder>/Materials/MI_<AssetID>.MI_<AssetID>`
- Outline material: `<TargetFolder>/Materials/MI_<AssetID>_Outline.MI_<AssetID>_Outline`

Main material parameter bindings:

<!-- Parameter list: derived at generation time from ImportPixal3DAsset_Phase1C.py:_TOON_CHARACTER_PARAMETERS. Do not hand-edit. -->

| Parameter | Binding |
|---|---|
| `BaseColorTexture` | Imported base texture |
| `TintTexture` | Imported tint texture |
| `InnerLineTexture` | Imported inner-line texture |

Outline material parameter binding:

| Parameter | Binding |
|---|---|
| `OutlineColor` | Manifest/import outline color when provided |

Stale importer bindings removed after the standard-process bulletproofing pass:
`DiffuseColorMap`, scalar `Brightness`, and vector `Tint`. They were no-op
entries because the live `M_Toon_Character` master does not expose them.

Mesh material assignment:

- Slot 0 on main mesh: `MI_<AssetID>`
- Slot 0 on outline mesh: `MI_<AssetID>_Outline`

Important material note:

- `T_InnerLines_DefaultBlack` exists as a safe default, but these 26 Agent 2 assets used generated per-asset inner-line textures, not the default black placeholder.

## Runtime And Data Wiring

Files/systems updated in the import pass:

- `C:\UE\T66\Content\Data\CharacterVisuals.csv`
- `Content/Data/DT_CharacterVisuals`, reloaded from CSV
- `C:\UE\T66\Source\T66\Gameplay\T66CompanionBase.cpp`
- `C:\UE\T66\Source\T66\Gameplay\T66CompanionBase.h`
- `C:\UE\T66\Model Generation\Pixal3D\production_asset_replacement_manifest.json`
- `C:\UE\T66\Model Generation\Pixal3D\Scripts\run_pixal3d_toonstyle_production_import.py`

Runtime behavior:

- Hero and companion visuals are already connected through `CharacterVisuals.csv` and `DT_CharacterVisuals`.
- Hero and companion identity/unlock data remains separate from cosmetic visual data.
- Hero demo skin runtime ID remains `Beachgoer`; UI displays it as `Demo`.
- No Hero 2 demo skin rows/assets were added.
- Companion static visuals rely on the runtime bridge added in `T66CompanionBase`.

Placement guidance:

- For static Hero Selection test-room placement, use the imported static meshes and outline meshes directly.
- Use `180 cm` as the placement height baseline.
- Companion CSV scale `(X=0.588235,Y=0.588235,Z=0.588235)` is runtime compensation for the companion actor scale multiplier; it is not necessarily the correct static lineup scale.
