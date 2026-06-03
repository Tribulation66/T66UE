"""
No-spaces runner for the animated ToonStyle hero Unreal import.

Unreal's -ExecutePythonScript path parser can mangle paths containing spaces, so
call this script from Scripts/ and let Python execute the real tool by path.
"""

import runpy
from pathlib import Path

import unreal


PROJECT_DIR = Path(unreal.SystemLibrary.get_project_directory())
TOOL_PATH = (
    PROJECT_DIR
    / "Model Generation"
    / "Rigging and Animation"
    / "Tools"
    / "import_animated_toonstyle_heroes_to_unreal.py"
)

runpy.run_path(str(TOOL_PATH), run_name="__main__")
