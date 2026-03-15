"""
Run ImportStaticMeshes.py in the full editor, but only for the current prop batch,
then request editor shutdown.
"""

import os
import sys
import unreal


SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
if SCRIPT_DIR not in sys.path:
    sys.path.append(SCRIPT_DIR)

import ImportStaticMeshes


def main():
    prop_imports = []
    for entry in ImportStaticMeshes.IMPORTS:
        relative_path, *_ = ImportStaticMeshes._normalize_import_entry(entry)
        if str(relative_path).startswith("Props/"):
            prop_imports.append(entry)

    unreal.log(
        f"[RunImportPropsAndExit] Running ImportStaticMeshes for {len(prop_imports)} prop entries")

    original_imports = list(ImportStaticMeshes.IMPORTS)
    try:
        ImportStaticMeshes.IMPORTS = prop_imports
        ImportStaticMeshes.main()
    finally:
        ImportStaticMeshes.IMPORTS = original_imports

    world = None
    try:
        subsystem = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
        if subsystem:
            world = subsystem.get_editor_world()
    except Exception:
        pass

    try:
        unreal.SystemLibrary.execute_console_command(world, "QUIT_EDITOR")
        unreal.log("[RunImportPropsAndExit] QUIT_EDITOR requested")
    except Exception as exc:
        unreal.log_warning(f"[RunImportPropsAndExit] Failed to request QUIT_EDITOR: {exc}")


if __name__ == "__main__":
    main()
