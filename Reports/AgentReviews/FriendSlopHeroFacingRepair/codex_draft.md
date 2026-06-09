Codex draft for cross-review.

Root cause:
- `CharacterVisuals.csv` is the runtime orientation adapter for static character meshes.
- Legacy/processed heroes use `MeshRelativeRotation=(Pitch=0,Yaw=-90,Roll=0)`.
- Raw Pixal3D mobs use `MeshRelativeRotation=(Pitch=0,Yaw=90,Roll=0)`.
- The new raw FriendSlop Hero 1 mesh used the raw/static forward convention but inherited the legacy processed-hero `-90` yaw, causing the exact back/front inversion the user reported.

Changes made:
- Added a Facing Preservation Rule to `Model Generation/Instructions/11_FRIENDSLOP_RAW_PIXAL3D_IMPORT_GUIDELINES.md`.
- Added reusable manifest-driven helper `Scripts/ApplyFriendSlopRawCharacterVisualRows.py`.
- Added `character_visual_rows` and `character_visual_yaw: 90.0` to `Model Generation/Runs/Pixal3D/FriendSlopProbe_Hero1Male_20260604_1415/FriendSlopProbe_Hero1Male_20260604_1415_manifest.json`.
- Updated `Scripts/ReloadFriendSlopEasyPixal3DDataTablesAndExit.py` so the raw FriendSlop Hero 1 rows are applied from manifest before `DT_CharacterVisuals` is reloaded.
- Updated `Scripts/ValidateFriendSlopRawPixal3DAndExit.py` so raw Hero 1 rows fail validation if they regress from expected yaw 90.
- Updated `Content/Data/CharacterVisuals.csv`:
  - `Hero_1_Chad` yaw changed from `-90.000000` to `90.000000`.
  - `Hero_1_Chad_DemoSkin` yaw changed from `-90.000000` to `90.000000`.

Verification:
- `python -m py_compile Scripts/ApplyFriendSlopRawCharacterVisualRows.py Scripts/ValidateFriendSlopRawPixal3DAndExit.py Scripts/ReloadFriendSlopEasyPixal3DDataTablesAndExit.py` passed.
- `python Scripts/ApplyFriendSlopRawCharacterVisualRows.py "Model Generation/Runs/Pixal3D/FriendSlopProbe_Hero1Male_20260604_1415/FriendSlopProbe_Hero1Male_20260604_1415_manifest.json"` applied two rows at yaw 90.
- Unreal DataTable reload via `UnrealEditor.exe -ExecutePythonScript=Scripts/ReloadFriendSlopEasyPixal3DDataTablesAndExit.py -NullRHI` exited 0. Log evidence in `Saved/Logs/T66-backup-2026.06.05-06.50.28.log` shows:
  - `Applied FriendSlop raw CharacterVisual row count: 2`
  - `=== SetupCharacterVisualsDataTable DONE ===`
  - `=== ReloadFriendSlopEasyPixal3DDataTablesAndExit DONE ===`
- Unreal validator via `UnrealEditor.exe -ExecutePythonScript=Scripts/ValidateFriendSlopRawPixal3DAndExit.py -NullRHI` exited 0.
- Validator report `Reports/AgentReviews/FriendSlopEasyPixal3D/raw_runtime_reference_validation.json` has `ok=true`, `errors=[]`, and both Hero 1 rows have `actual_yaw=90.0`, `expected_yaw=90.0`, `yaw_ok=true`.
- Current Unreal-owned gameplay capture:
  - `Reports/AgentReviews/FriendSlopHeroFacingRepair/VisualProof_Hero1ChadRawFacing/Hero1ChadRawFacing.mp4`
  - `Reports/AgentReviews/FriendSlopHeroFacingRepair/VisualProof_Hero1ChadRawFacing/evidence/contact_sheet.png`
  - Visual inspection: chase-camera view shows Hero 1's back toward the camera while he moves toward the enemies, which is the expected in-game forward orientation.
- `Scripts/StageStandaloneBuild.ps1` succeeded. Log: `Reports/AgentReviews/FriendSlopHeroFacingRepair/stage_standalone.log`
  - `BUILD SUCCESSFUL`
  - staged exe ready at `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`
- Shortcut target verification:
  - `C:\UE\T66\T66 Standalone.lnk` target is staged exe.
  - taskbar `T66 Standalone.lnk` target is staged exe.

Caveats:
- The hero is still raw/static because FriendSlop has no rigging/processing pipeline yet, matching the user's prior instruction.
- The systematic correction is in manifest-driven CharacterVisuals row application, not in baked mesh reorientation. That preserves the raw generated mesh and makes the runtime yaw explicit for future raw heroes.

Proposed final answer:
- Tell the user the hero-facing issue is fixed and staged.
- State the cause: raw FriendSlop hero used raw/static forward convention but inherited legacy processed-hero yaw.
- Mention the reusable manifest/document/validator additions.
- Provide proof paths and staged exe path.
