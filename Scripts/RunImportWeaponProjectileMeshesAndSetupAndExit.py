"""
Run ImportWeaponProjectileMeshesAndSetup.py in the editor, then request shutdown.
"""

import os
import sys

import unreal


SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
if SCRIPT_DIR not in sys.path:
    sys.path.append(SCRIPT_DIR)

import ImportWeaponProjectileMeshesAndSetup


def main():
    exit_code = 0
    world = None
    try:
        ImportWeaponProjectileMeshesAndSetup.main()
    except Exception as exc:
        exit_code = 1
        unreal.log_error(f"[RunImportWeaponProjectileMeshesAndSetupAndExit] Failed: {exc}")
    finally:
        try:
            subsystem = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
            if subsystem:
                world = subsystem.get_editor_world()
        except Exception:
            pass

        try:
            unreal.SystemLibrary.execute_console_command(world, "QUIT_EDITOR")
            unreal.log("[RunImportWeaponProjectileMeshesAndSetupAndExit] QUIT_EDITOR requested")
        except Exception as exc:
            unreal.log_warning(
                f"[RunImportWeaponProjectileMeshesAndSetupAndExit] Failed to request QUIT_EDITOR: {exc}"
            )

    if exit_code != 0:
        raise SystemExit(exit_code)


if __name__ == "__main__":
    main()
