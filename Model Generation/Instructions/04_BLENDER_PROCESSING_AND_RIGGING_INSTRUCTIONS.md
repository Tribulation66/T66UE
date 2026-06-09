# Blender Processing And Rigging

Use Blender for deterministic inspection, cleanup, render QA, and export. Use RetopoFlow only for topology work that needs human-authored low-poly structure.

## Durable Helpers

- `Model Generation/Scripts/Core/Blender/blender_glb_qa.py`
- `Model Generation/Scripts/Core/Blender/OpenBlenderScene.ps1`
- `Model Generation/Scripts/Core/Blender/t66_unreal_friend_slop_preview.py`
- `Model Generation/Tools/BlenderLabMCP/launch_blender_lab_mcp.ps1`
- `Model Generation/Tools/BlenderLabMCP/setup_blender_lab_mcp.ps1`

## Policy

- Before any Blender work, start or verify the official Blender Lab MCP bridge with `Model Generation/Tools/BlenderLabMCP/launch_blender_lab_mcp.ps1`. Use `-Visible` when the user asked to see Blender. Confirm MCP connectivity with a Blender MCP summary call before inspecting, editing, importing, rendering, or opening a scene. If MCP cannot be made available, report that blocker instead of silently switching to an unverified non-MCP workflow.
- Do not launch Blender bare for user-facing scene opens. To open an existing `.blend`, use `Model Generation/Scripts/Core/Blender/OpenBlenderScene.ps1 -BlendFile "<path>"`, or open the file through an already-connected Blender MCP session with `bpy.ops.wm.open_mainfile(filepath=r"<path>")`. These paths keep `Model Generation` spaces intact and prevent Blender from falling back to the default cube startup scene.
- When a Blender window is expected, verify the loaded file, not just that Blender is running. Acceptable evidence includes `OpenBlenderScene.ps1` background load verification/window-title confirmation, Blender MCP `bpy.data.filepath`, or a Blender MCP data-block/object summary from the intended file.
- For FriendSlop raw Pixal3D model review on Windows, Blender scenes must launch with the T66 Unreal/FriendSlop preview setup. Use `Model Generation/Scripts/Core/Blender/t66_unreal_friend_slop_preview.py` to set Standard color management, fixed exposure/gamma, white world, the locked Hero 1 softbox review rig, and Blender emission materials that mirror Unreal's `/Game/Materials/M_GLB_Unlit` behavior: generated texture multiplied by neutral `Tint=(1,1,1,1)` and `Brightness=1`. Do not judge raw FriendSlop model color or readability from Blender's default lit Principled/import lighting.
- Treat the unlit material behavior as the primary runtime match. The Hero 1 softbox rig is the standard secondary review environment for form readability; it must not be mistaken for the runtime material response, because `M_GLB_Unlit` does not depend on scene lights.
- Do not promote Decimate output as accepted low-poly topology for deformation-critical characters.
- Decimate is acceptable for diagnostics and throwaway prototypes.
- Rigging work must bake mesh placement, keep feet-origin scale consistent, and preserve actions explicitly so exported animation assets survive.
- Equipment grip quality needs multi-angle validation, especially hands, shoulders, face, and weapon overlap.

## Legacy Notes

Older TypeA/Mike, Arthur, and dungeon-kit prototype scripts were deleted during Alpha 0.1 cleanup after their durable lessons were captured in the current instructions. New production work should promote reusable ideas into `Scripts/Core` instead of reviving one-off prototypes.
