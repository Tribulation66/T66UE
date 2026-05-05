from pathlib import Path


canonical_script = (
    Path(__file__).resolve().parents[1]
    / "BlenderLabMCP"
    / "start_blender_lab_mcp.py"
)

exec(
    compile(canonical_script.read_text(encoding="utf-8"), str(canonical_script), "exec"),
    globals(),
)
