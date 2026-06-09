# Codex Draft - FriendSlop Raw Import Repair

## Task Contract

Operator: Codex
Validator: Claude
Scope: create a FriendSlop raw Pixal3D import guideline, preserve GLB-derived textures on raw imports, and replace old-model runtime references with generated FriendSlop models where applicable.
Stop condition: guideline exists, raw imports have material texture bindings, runtime rows point to generated raw meshes, DataTables are reloaded, and staged standalone verification is run or reported.

## Implementation Summary

- Added `Model Generation/Instructions/11_FRIENDSLOP_RAW_PIXAL3D_IMPORT_GUIDELINES.md`.
- Updated Model Generation instruction routing/README to route FriendSlop raw Pixal3D imports to that doc.
- Patched `Scripts/ImportFriendSlopRawPixal3DFBXAndExit.py` so it no longer relies on FBX/MTL auto binding:
  - Imports `Textures/<AssetID>_00_Image_0.png` as `T_<AssetID>_BaseColor`.
  - Creates/updates `MI_SM_<AssetID>` under each target dir.
  - Parents to `/Game/Materials/M_GLB_Unlit`.
  - Sets `BaseColorTexture` and `DiffuseColorMap`.
  - Assigns the material instance to every imported `StaticMesh` slot.
  - Disables FBX auto material/texture import.
- Patched `Scripts/ValidateFriendSlopRawPixal3DAndExit.py` to validate material slots and texture params, not just mesh existence.
- Patched `Scripts/ReloadFriendSlopEasyPixal3DDataTablesAndExit.py` to reload `DT_MobVertexAnimations`.
- Added a small Hero 1 FriendSlop probe manifest at `Model Generation/Runs/Pixal3D/FriendSlopProbe_Hero1Male_20260604_1415/FriendSlopProbe_Hero1Male_20260604_1415_manifest.json`.
- Exported Hero 1 probe GLB to a raw textured FBX bundle.
- Re-imported:
  - 49 FriendSlop Easy raw FBX assets.
  - 1 Hero 1 raw FriendSlop probe asset.
- Updated `Content/Data/CharacterVisuals.csv`:
  - `Hero_1_Chad` and `Hero_1_Chad_DemoSkin` now point to `/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/SM_Hero_1_Chad_Male.SM_Hero_1_Chad_Male`.
  - Skeletal mesh and walk/idle/jump/roll animation fields are blank for those two rows so the runtime does not prefer the old animated ToonStyle skeletal mesh.
- Updated `Content/Data/MobVertexAnimations.csv`:
  - Disabled VAT rows for `Slime`, `CaveBat`, `BoneWalker`, `RatPack`, `TombSpider`, `HexSlinger`, `StoneSentinel`, `MimicLure`, `BoneConjurer`, and `CryptWraith` so the raw static `CharacterVisuals.csv` rows win.
- Documented unrelated staged smoke failure in `Source/T66/Gameplay/pending_issues_Gameplay.md`.

## Key Evidence

- Hero 1 Blender export:
  - Command: `python Model Generation/Pixal3D/Scripts/export_accurig_textured_batch.py --manifest ...FriendSlopProbe_Hero1Male_20260604_1415_manifest.json --run-root ...FriendSlopProbe_Hero1Male_20260604_1415 --output-root ...RawTexturedFBX --force`
  - Result: `errors=[]`, one FBX, two 4096 textures, height 180 cm.
- Hero 1 Unreal import:
  - Report: `Reports/AgentReviews/FriendSlopEasyPixal3D/raw_fbx_unreal_import_report_hero1.json`
  - `ok=true`, `errors=[]`, `asset_count=1`.
  - Material and both texture params bound to `/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/Textures/T_Hero_1_Chad_Male_BaseColor.T_Hero_1_Chad_Male_BaseColor`.
- 49-asset Unreal import:
  - Report: `Reports/AgentReviews/FriendSlopEasyPixal3D/raw_fbx_unreal_import_report.json`
  - `ok=true`, `errors=[]`, `asset_count=49`.
- Raw runtime validation:
  - Report: `Reports/AgentReviews/FriendSlopEasyPixal3D/raw_runtime_reference_validation.json`
  - `ok=true`, `errors=[]`.
  - Validates 49 raw imports load, material slots point to expected material instances, texture assets exist, `BaseColorTexture`/`DiffuseColorMap` match expected base-color textures, both Chad rows point to raw Hero 1 static mesh with skeletal/animations blank, and ten mob VAT rows are disabled.
- DataTable reload:
  - Log: `Reports/AgentReviews/FriendSlopEasyPixal3D/datatable_reload_repair.log`
  - Commandlet exited 0.
- Standalone stage:
  - Log: `Reports/AgentReviews/FriendSlopEasyPixal3D/stage_standalone_repair.log`
  - `BUILD SUCCESSFUL`, staged exe `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
  - Shortcut targets verified for repo and taskbar shortcuts.
- Staged smoke:
  - Log: `C:\UE\T66\Saved\StandaloneLogs\FriendSlopContentCorrectionsSmoke.log`
  - Process exited code 0, but content smoke summary logged `Pass=0` due out-of-scope `SafeZoneVisualBubblePresent`; documented as pending issue.
- Targeted raw visual proof:
  - Easy mob lineup: `Reports/AgentReviews/FriendSlopEasyPixal3D/VisualProof_EasyMobRawStatic/EasyMobRawStatic.mp4`
  - Easy mob contact sheet: `Reports/AgentReviews/FriendSlopEasyPixal3D/VisualProof_EasyMobRawStatic/evidence/contact_sheet.png`
  - Hero 1 Chad: `Reports/AgentReviews/FriendSlopEasyPixal3D/VisualProof_Hero1ChadRawStatic/Hero1ChadRawStatic.mp4`
  - Hero 1 Chad contact sheet: `Reports/AgentReviews/FriendSlopEasyPixal3D/VisualProof_Hero1ChadRawStatic/evidence/contact_sheet.png`
  - Both captures are Unreal-owned gameplay captures. Contact sheets show textured, non-blue raw static meshes in-game.

## Tradeoff

Replacing `Hero_1_Chad` and `Hero_1_Chad_DemoSkin` with the raw FriendSlop Pixal3D static mesh intentionally drops the old skeletal animation fields. Disabling VAT for `Slime`, `CaveBat`, `BoneWalker`, `RatPack`, `TombSpider`, `HexSlinger`, `StoneSentinel`, `MimicLure`, `BoneConjurer`, and `CryptWraith` intentionally drops their old VAT animation path. This follows the user's direct instruction to use the generated models as generated, but it means the current FriendSlop hero and these ten mobs render as static raw meshes until a future raw FriendSlop rig/animation process exists.

## Draft Final Status

FriendSlop raw imports are repaired for texture preservation and runtime references. The game has been staged. Targeted gameplay captures show the Hero 1 and Easy mob raw static replacements textured in-game. The user can enter the staged game and try the updated raw models. The known caveat is that the generic staged content-corrections smoke has an unrelated Safe Zone visual bubble failure.
