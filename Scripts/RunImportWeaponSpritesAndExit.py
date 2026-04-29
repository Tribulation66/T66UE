"""
Run ImportWeaponSpritesAndSetup.py in the full editor, then request editor shutdown.
"""

import os
import sys
import unreal


SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
if SCRIPT_DIR not in sys.path:
    sys.path.append(SCRIPT_DIR)

import ImportWeaponSpritesAndSetup


def main():
    world = None
    try:
        ImportWeaponSpritesAndSetup.main()
    finally:
        try:
            subsystem = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
            if subsystem:
                world = subsystem.get_editor_world()
        except Exception:
            pass

        try:
            unreal.SystemLibrary.execute_console_command(world, "QUIT_EDITOR")
            unreal.log("[RunImportWeaponSpritesAndExit] QUIT_EDITOR requested")
        except Exception as exc:
            unreal.log_warning(f"[RunImportWeaponSpritesAndExit] Failed to request QUIT_EDITOR: {exc}")


if __name__ == "__main__":
    main()
