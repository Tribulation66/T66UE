"""
Run the Arthur HUD-art import and hero portrait import in the full editor, then request shutdown.
"""

import os
import sys
import unreal


SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
if SCRIPT_DIR not in sys.path:
    sys.path.append(SCRIPT_DIR)

import ImportGameplayHUDArt
import ImportHeroPortraits


def main():
    world = None
    try:
        ImportGameplayHUDArt.main()
        ImportHeroPortraits.main()
    finally:
        try:
            subsystem = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
            if subsystem:
                world = subsystem.get_editor_world()
        except Exception:
            pass

        try:
            unreal.SystemLibrary.execute_console_command(world, "QUIT_EDITOR")
            unreal.log("[RunImportGameplayHUDArtAndPortraitsAndExit] QUIT_EDITOR requested")
        except Exception as exc:
            unreal.log_warning(f"[RunImportGameplayHUDArtAndPortraitsAndExit] Failed to request QUIT_EDITOR: {exc}")


if __name__ == "__main__":
    main()
