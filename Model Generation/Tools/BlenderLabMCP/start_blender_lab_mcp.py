import os
import traceback
from pathlib import Path

import bpy


LOG_PATH = Path(r"C:\UE\T66\Model Generation\Tools\BlenderLabMCP\start_blender_lab_mcp.log")
DEFAULT_HOST = "localhost"
DEFAULT_PORT = 9876
OFFICIAL_MODULE = "bl_ext.user_default.mcp"


def log(message):
    print(message)
    LOG_PATH.parent.mkdir(parents=True, exist_ok=True)
    with LOG_PATH.open("a", encoding="utf-8") as handle:
        handle.write(f"{message}\n")


def get_target_port():
    raw_port = os.environ.get("BLENDER_MCP_PORT", str(DEFAULT_PORT))
    try:
        return int(raw_port)
    except ValueError:
        log(f"[BLENDER_LAB_MCP] Invalid BLENDER_MCP_PORT={raw_port!r}; using {DEFAULT_PORT}")
        return DEFAULT_PORT


def start_mcp_server():
    LOG_PATH.write_text("", encoding="utf-8")
    target_host = os.environ.get("BLENDER_MCP_HOST", DEFAULT_HOST)
    target_port = get_target_port()
    log("[BLENDER_LAB_MCP] timer fired")
    log(f"[BLENDER_LAB_MCP] target endpoint: {target_host}:{target_port}")

    try:
        if not bpy.app.online_access:
            log("[BLENDER_LAB_MCP] Online access is disabled; launch Blender with --online-mode")
            return None

        if OFFICIAL_MODULE not in bpy.context.preferences.addons:
            bpy.ops.preferences.addon_enable(module=OFFICIAL_MODULE)
            log(f"[BLENDER_LAB_MCP] enabled official extension module {OFFICIAL_MODULE}")

        prefs = bpy.context.preferences.addons[OFFICIAL_MODULE].preferences
        prefs.host = target_host
        prefs.port = target_port
        prefs.use_autostart = True
        prefs.use_log = False

        from bl_ext.user_default.mcp import mcp_to_blender_server

        if not mcp_to_blender_server.is_running():
            bpy.ops.blmcp.server_start()
            log("[BLENDER_LAB_MCP] server start operator executed")
        else:
            log("[BLENDER_LAB_MCP] server already running")

        log(f"[BLENDER_LAB_MCP] running: {mcp_to_blender_server.is_running()}")
    except Exception as exc:
        log(f"[BLENDER_LAB_MCP] Failed to start server: {exc}")
        log(traceback.format_exc())

    return None


bpy.app.timers.register(start_mcp_server, first_interval=1.0)
