import bpy
from pathlib import Path


LOG_PATH = Path(r"C:\UE\T66\Model Generation\Tools\Trellis2\start_blender_mcp.log")


def log(message):
    print(message)
    with LOG_PATH.open("a", encoding="utf-8") as handle:
        handle.write(f"{message}\n")


def start_mcp_server():
    LOG_PATH.write_text("", encoding="utf-8")
    log("[BLENDER_MCP] timer fired")
    try:
        bpy.ops.preferences.addon_enable(module="blender_mcp_addon")
        log("[BLENDER_MCP] addon enabled")
    except Exception as exc:
        log(f"[BLENDER_MCP] Failed to enable addon: {exc}")

    try:
        import blender_mcp_addon
        log("[BLENDER_MCP] addon module imported")

        if (
            not hasattr(bpy.types, "blendermcp_server")
            or not bpy.types.blendermcp_server
        ):
            bpy.types.blendermcp_server = blender_mcp_addon.BlenderMCPServer(
                port=bpy.context.scene.blendermcp_port
            )
        log(
            f"[BLENDER_MCP] starting server on port {bpy.context.scene.blendermcp_port}"
        )
        bpy.types.blendermcp_server.start()
        bpy.context.scene.blendermcp_server_running = True
        log("[BLENDER_MCP] server started")
    except Exception as exc:
        log(f"[BLENDER_MCP] Failed to start server: {exc}")

    return None


bpy.app.timers.register(start_mcp_server, first_interval=1.0)
