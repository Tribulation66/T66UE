# Phase 1B - Blender Pipeline Extension Report

Date: 2026-05-17

## Scope

Workstream C extended the Pixal3D-to-UE path without regenerating Pixal3D assets. Existing raw GLBs were reprocessed through Blender and UE import to produce separate shading and outline static meshes with vertex colors.

Loot Bag Yellow remains deferred because its Pixal3D generation issue is still tracked separately.

## Scripts

Created/refactored:

- `ToonStyle/BlenderScripts/pixal3d_blender_base.py`
- `ToonStyle/BlenderScripts/extract_and_normalize_pixal3d.py`
- `ToonStyle/BlenderScripts/author_vertex_colors.py`
- `ToonStyle/BlenderScripts/transfer_face_normals.py`
- `ToonStyle/BlenderScripts/run_toon_pipeline.py`
- `ToonStyle/Source/ImportPixal3DAsset.py`
- `ToonStyle/Tools/RunPixal3DToUE.ps1`

`extract_and_normalize_pixal3d.py` is now a compatibility wrapper over reusable module functions. `run_toon_pipeline.py` is the new single-session Blender orchestrator.

## Pipeline Order

For each raw GLB:

1. Import GLB.
2. Extract embedded Pixal3D textures as PNG.
3. Join meshes.
4. Normalize spatial transform to 180 UU height.
5. Author vertex colors.
6. Duplicate outline mesh before face-normal transfer.
7. Transfer humanoid face normals on the shading mesh only.
8. Export shading FBX and outline FBX.
9. Import both FBXs into UE.
10. Import textures, create/bind material instances, and write verification JSON.

The FBX unit-scale fix from Phase 1A.2 remains in place:

- `global_scale=0.01`
- `apply_unit_scale=True`
- `apply_scale_options='FBX_SCALE_ALL'`
- `bake_space_transform=False`

## Vertex Colors

Attribute name: `Col`

Layout:

- R: AO-derived threshold offset, using Cycles AO bake where available.
- G: outline width multiplier, default 1.0.
- B: outline depth offset, default 0.0.
- A: outline mask, default 1.0.

AO settings:

- Cycles AO bake
- 64 samples
- AO distance 0.3 after normalization

If baking is unavailable for a mesh, the script has an analytic fallback. The Phase 1B batch evidence shows `cycles_ao` for the processed assets.

## Normal Transfer

Humanoid assets:

- Lu Bu validation
- ARIA
- Gambler

For humanoids, the top 25 percent of the mesh's Z extent is treated as the head region. The script applies proxy-sphere-style custom split normals to that region on the shading mesh only.

The outline mesh is duplicated before transfer and retains the geometric normals used by `VertexNormalWS` in the outline material.

Non-humanoid assets skip face-normal transfer.

## Gates

G3 passed.

- Asset: `lubu_validation`
- Shading mesh: `/Game/ToonStyle/TestAssets/Validation/SM_lubu_validation`
- Outline mesh: `/Game/ToonStyle/TestAssets/Validation/SM_lubu_validation_Outline`
- Height: approximately 180 UU for both.
- Vertex color attribute: `Col`
- Evidence: `SourceAssets/ToonStyle/Pixal3D/Phase1A/LuBu_Matrix/Working/lubu_validation/lubu_validation_ue_verify.json`

G4 passed.

- Validation Lu Bu shading and outline normals are distinct in the sampled head region.
- Normal delta in verification JSON: approximately 0.678.
- This confirms duplication happened before face-normal transfer.

G5 passed.

- Evidence: `Saved/Codex/ToonStyle/Phase1B/gate_g5_verify.json`
- All processed assets imported with shading and outline static meshes.
- All imported meshes have approximately 180 UU height.
- All imported meshes have vertex color metadata.
- No emergency fallback path was used.

## Processed Assets

- `lubu_validation` -> `/Game/ToonStyle/TestAssets/Validation/`
- `aria` -> `/Game/ToonStyle/TestAssets/Lineup/`
- `gambler` -> `/Game/ToonStyle/TestAssets/Lineup/`
- `slime` -> `/Game/ToonStyle/TestAssets/Lineup/`
- `tombspider` -> `/Game/ToonStyle/TestAssets/Lineup/`
- `cavebat` -> `/Game/ToonStyle/TestAssets/Lineup/`
- `idolaltar` -> `/Game/ToonStyle/TestAssets/Lineup/`
- `arcademachine` -> `/Game/ToonStyle/TestAssets/Lineup/`
- `lootchest` -> `/Game/ToonStyle/TestAssets/Lineup/`
- `lootcrate` -> `/Game/ToonStyle/TestAssets/Lineup/`

## Notes

The UE Python import path can still terminate harshly in some headless sessions after saving due the existing editor/Python shutdown issue documented in `Scripts/pending_issues_Scripts.md`. Phase 1B treated verification JSON and saved assets as the gate evidence, not the process exit code alone.
