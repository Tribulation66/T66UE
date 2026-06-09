# FriendSlop Raw Pixal3D Import Guidelines

FriendSlop gameplay assets use the raw Pixal3D mesh and texture output unless the user explicitly asks for a later processing pass. Do not run ToonStyle, Quad Retro, outline, tint, inner-line, or processed-GLB steps for this visual approach.

## Source Rule

- Source artifact: the generated Pixal3D `.glb`.
- If Unreal GLB Interchange imports zero usable `StaticMesh` assets, export a textured FBX bundle from the GLB with `Model Generation/Pixal3D/Scripts/export_accurig_textured_batch.py`.
- The helper and its reports may use the historical name `AccuRig_Textured`. In FriendSlop raw imports, that name means "textured FBX export from the generated GLB" only. It does not mean the archived AccuRig / Animated ToonStyle hero demo lineup is current or approved for FriendSlop.
- The FBX bundle must contain:
  - `<AssetID>_Textured.fbx`
  - `Textures/<AssetID>_00_Image_0.png`
  - export report evidence showing nonzero FBX and texture bytes

## Texture Preservation Rule

Do not rely on FBX/MTL auto texture binding. The generated MTL can point at paths Unreal does not resolve during import, which produces valid meshes with generic blue materials.

For every raw FriendSlop import:

1. Import `Textures/<AssetID>_00_Image_0.png` as `/Game/.../Textures/T_<AssetID>_BaseColor`.
2. Create or update `/Game/.../Materials/MI_SM_<AssetID>` with parent `/Game/Materials/M_GLB_Unlit`.
3. Set both material texture parameters when available:
   - `BaseColorTexture`
   - `DiffuseColorMap`
4. Set neutral material controls:
   - `Tint = (1,1,1,1)`
   - `Brightness = 1.0`
   - `Opacity = 1.0`
5. Assign that material instance to every imported `StaticMesh` material slot.

The blue-material fallback is a failed import, even if the mesh exists and the DataTable reference is correct.

## Facing Preservation Rule

`CharacterVisuals.csv` is the runtime orientation adapter for static character meshes. Do not let raw FriendSlop humanoid heroes inherit the legacy processed-hero yaw by default.

Observed convention:

- Legacy processed/skeletal heroes use `MeshRelativeRotation=(Pitch=0,Yaw=-90,Roll=0)`.
- Raw Pixal3D mobs use `MeshRelativeRotation=(Pitch=0,Yaw=90,Roll=0)`.
- Raw FriendSlop humanoid hero GLBs generated from the current front-facing Pixal3D image workflow match the raw mob convention, so their `CharacterVisuals.csv` rows must use `Yaw=90` unless a four-view/facing proof shows a different generated source convention.

For future raw FriendSlop heroes, add `character_visual_rows` and `character_visual_yaw` to the manifest and apply rows through `Scripts/ApplyFriendSlopRawCharacterVisualRows.py`. The validator must fail if a raw humanoid hero row points at the correct mesh but carries the wrong yaw; this is still a failed import because gameplay front/back is inverted.

## Runtime Reference Rule

Raw FriendSlop meshes must win the same runtime path the player sees:

- `CharacterVisuals.csv` rows using a raw static mesh must not keep a preferred `SkeletalMesh` or animation set when the static mesh is intended to render.
- Mob rows with enabled entries in `MobVertexAnimations.csv` will try VAT visuals before `CharacterVisuals.csv`. Disable the VAT row when replacing that enemy with a raw FriendSlop static mesh.
- Pets, NPCs, vehicles, and world props must point their display/capture mesh fields at the imported raw `StaticMesh`.

## Required Validation

Before accepting a raw FriendSlop import:

1. Verify every imported mesh loads as a `StaticMesh`.
2. Verify every mesh material slot points to the expected `MI_SM_<AssetID>`.
3. Verify `MI_SM_<AssetID>` has `BaseColorTexture` and `DiffuseColorMap` bound to `T_<AssetID>_BaseColor`.
4. Reload the affected Unreal DataTables from source CSV/JSON.
5. If playable content changes, refresh the staged standalone build and verify the shortcut target still points at the staged executable.

## Blender Preview Rule

For Blender review of raw FriendSlop Pixal3D assets, use `Model Generation/Scripts/Core/Blender/t66_unreal_friend_slop_preview.py` after import and before rendering or showing the scene. The runtime material contract is `/Game/Materials/M_GLB_Unlit`, so Blender previews must use emission/unlit texture output with neutral `Tint=(1,1,1,1)`, `Brightness=1`, and `Opacity=1` instead of judging the asset under Blender's default Principled lighting. The helper also applies the locked Hero 1 softbox rig as the standard secondary review environment for shape readability.

Current raw batch tooling:

- Import: `Scripts/ImportFriendSlopRawPixal3DFBXAndExit.py`
- CharacterVisual rows: `Scripts/ApplyFriendSlopRawCharacterVisualRows.py`
- Reload: `Scripts/ReloadFriendSlopEasyPixal3DDataTablesAndExit.py`
- Validate: `Scripts/ValidateFriendSlopRawPixal3DAndExit.py`

Current raw source runs:

- `Model Generation/Runs/Pixal3D/FriendSlopEasyBatch_20260604_1532`
- `Model Generation/Runs/Pixal3D/FriendSlopProbe_Hero1Male_20260604_1415`
