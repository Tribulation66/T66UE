Original user message:
This is all I see.

Context:
The user screenshot shows the Blender startup cube after Codex claimed the Hero 1 male side-by-side scene was open.

Working task:
Operator: Codex
Validator: Claude
Scope: Fix the visible Blender launch so the saved look-dev blend opens on `SideBySide_Raw_vs_V04`, with raw Pixal3D on the left and `V04 Candy Rubber` on the right. Stay Blender-only; no Unreal or material-look changes.
Stop condition: The visible Blender window is on the requested side-by-side scene/camera, or the exact blocker is reported.

Suspected cause:
The previous `Start-Process` command passed the `.blend` path without quoting spaces, so Blender opened the default startup scene.

