# Blender Lab MCP Tools

This folder is the local `T66` launcher and recovery path for Blender's official Blender Lab MCP server.

Authoritative sources:

- Blender Lab page: https://www.blender.org/lab/mcp-server/
- Source repository: https://projects.blender.org/lab/blender_mcp
- Wiki setup page: https://projects.blender.org/lab/blender_mcp/wiki/Llama.cpp#mcp-server-installation
- Add-on release used here: `mcp-1.0.0.zip`

This replaces the older third-party `ahujasid/blender-mcp` setup. Do not use `uvx blender-mcp` for this project; that resolves the third-party package from PyPI.

## Local Install

The official source clone is kept outside the repo:

```text
C:\Users\DoPra\.codex\tools\blender_mcp_official
```

The MCP executable used by Codex is:

```text
C:\Users\DoPra\.codex\tools\blender_mcp_official\mcp\.venv\Scripts\blender-mcp.exe
```

Codex config points `mcp_servers.blender.command` at that executable.

## Commands

Reinstall/update the official repo, Python venv, and Blender extension:

```powershell
.\Model Generation\Tools\BlenderLabMCP\setup_blender_lab_mcp.ps1
```

Start Blender with the official MCP bridge server:

```powershell
.\Model Generation\Tools\BlenderLabMCP\launch_blender_lab_mcp.ps1
```

Start with a visible Blender window:

```powershell
.\Model Generation\Tools\BlenderLabMCP\launch_blender_lab_mcp.ps1 -Visible
```

The official add-on requires Blender online access for its local socket server, so the launcher starts Blender with `--online-mode`.

## Required Preflight For Blender Work

Always launch or verify the official Blender Lab MCP bridge before Blender inspection, import, render, rigging, cleanup, or user-facing scene-open work. After the launcher reports that `127.0.0.1:9876` is accepting connections, confirm the session with a Blender MCP summary call before proceeding. If the MCP bridge cannot be started or reached, stop and report that concrete blocker instead of silently falling back to a non-MCP path.

When the user asks to open a `.blend`, do not start `blender.exe` with a raw or unquoted file argument. Use one of these paths:

```powershell
.\Model Generation\Scripts\Core\Blender\OpenBlenderScene.ps1 -BlendFile "<path-to-blend>"
```

or, from a connected Blender MCP session:

```python
bpy.ops.wm.open_mainfile(filepath=r"<path-to-blend>")
```

The repo open helper resolves the path, loads the file once in background for verification, and starts the visible Blender process with `ProcessStartInfo.ArgumentList`, which preserves spaces in paths such as `Model Generation`. Verify the final scene by checking the window title, `bpy.data.filepath`, or a Blender MCP object/data-block summary from the intended file. A running Blender process showing only the startup cube is not sufficient evidence that the requested scene opened.

## T66 Unreal/FriendSlop Preview Lighting

When launching Blender on Windows for FriendSlop raw Pixal3D model review, use the T66 Unreal preview setup rather than Blender's startup/default lighting. Scene builders should import and call:

```python
from t66_unreal_friend_slop_preview import apply_unreal_friend_slop_preview

apply_unreal_friend_slop_preview(bpy.context.scene)
```

with `Model Generation/Scripts/Core/Blender` on `sys.path`.

This preview setup mirrors the current Unreal runtime contract for raw FriendSlop assets: `/Game/Materials/M_GLB_Unlit`, generated base-color texture, neutral `Tint`, `Brightness=1`, and `Opacity=1`. In Blender that means the primary material preview is emission/unlit texture color, not default Principled lighting. The helper also installs the locked Hero 1 softbox rig as the standard review environment, so model form is readable while the material response remains Unreal-unlit. Verify the result through MCP by checking the scene custom property `T66_UnrealPreview_Profile`, converted materials with `T66_UnrealPreview`, and lights named `T66_Key_Softbox_L`, `T66_Key_Softbox_R`, `T66_Top_Soft_Fill`, and `T66_Front_Fill`.
