"""
Run a rigging/animation Python tool whose path may contain spaces.

Unreal's Python command-line runners can misparse direct script paths that live
under folders such as "Model Generation/Rigging and Animation". Set
T66_RIGGING_ANIMATION_TOOL_SCRIPT to the real script path and run this wrapper
from the no-space Scripts folder.
"""

import os
import runpy
from pathlib import Path

import unreal


def main():
    target = os.environ.get("T66_RIGGING_ANIMATION_TOOL_SCRIPT", "").strip()
    if not target:
        raise RuntimeError("T66_RIGGING_ANIMATION_TOOL_SCRIPT is not set")

    script_path = Path(target)
    if not script_path.is_file():
        raise RuntimeError(f"Rigging animation tool script does not exist: {script_path}")

    unreal.log(f"[RunRiggingAnimationToolAndExit] running {script_path}")
    try:
        runpy.run_path(str(script_path), run_name="__main__")
        unreal.log("[RunRiggingAnimationToolAndExit] done")
    finally:
        if os.environ.get("T66_RIGGING_ANIMATION_TOOL_QUIT_EDITOR", "").strip().lower() in {"1", "true", "yes"}:
            unreal.log("[RunRiggingAnimationToolAndExit] quitting editor by request")
            unreal.SystemLibrary.quit_editor()


if __name__ == "__main__":
    main()
