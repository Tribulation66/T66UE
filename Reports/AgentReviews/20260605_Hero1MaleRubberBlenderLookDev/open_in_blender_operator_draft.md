Operator draft:

Codex opened the existing male Hero 1 rubber look-dev `.blend` in a visible Blender window.

Actions performed:
- Created and saved a dedicated scene named `SideBySide_Raw_vs_V04` inside `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\LookDev\Hero_1_Chad_Male_Rubber_20260605\Hero_1_Chad_Male_RubberLookDev.blend`.
- The scene places the raw Pixal3D GLB on the left and `V04 Candy Rubber` on the right, using the same fixed soft Blender lighting rig from the look-dev pass.
- Launched visible Blender with `open_side_by_side_scene.py`, which switches the window to `SideBySide_Raw_vs_V04` and camera view.

Verification:
- Blender background save reported `T66_SIDE_BY_SIDE_SCENE_READY=SideBySide_Raw_vs_V04`.
- Visible Blender process started: `blender`, PID `29744`, `Responding=True`.

Scope:
- No Unreal work.
- No rigging/jiggle/runtime import work.
- Only Blender look-dev/report helper scripts and the look-dev `.blend` were touched.

