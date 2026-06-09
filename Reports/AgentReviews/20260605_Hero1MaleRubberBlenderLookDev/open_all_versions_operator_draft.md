Operator draft:

Codex corrected the visible Blender launch to show all versions side by side, not just raw plus V04.

Actions performed:
- Added `open_all_versions_scene.py`, which selects the existing `Rubber_Comparison_Grid` scene and `Camera_Comparison_Ortho` when the look-dev `.blend` opens.
- Verified the `.blend` contains `Rubber_Comparison_Grid` with labels for `Reference image`, `Raw Pixal3D GLB`, `V01 Soft Satin`, `V02 Rubber Pop`, `V03 Vinyl Bounce`, `V04 Candy Rubber`, `V05 Matte Gum`, and `V06 Toy Vinyl Gloss`.
- Closed the previous two-model Blender process and reopened the look-dev file with quoted paths and the all-versions opener.

Visible Blender process:
- PID: `24196`
- Responding: `True`
- Command includes: `Hero_1_Chad_Male_RubberLookDev.blend --python open_all_versions_scene.py`

Scope:
- Blender-only.
- No Unreal work.
- No material-look changes.

