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
