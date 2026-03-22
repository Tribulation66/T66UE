"""
Run ImportStaticMeshes.py for the staged SourceAssets/Import root models added for
the current gameplay batch, then request editor shutdown.
"""

import os
import sys
import unreal


SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
if SCRIPT_DIR not in sys.path:
    sys.path.append(SCRIPT_DIR)

import ImportStaticMeshes


TARGET_RELATIVE_PATHS = {
    "Cow.glb",
    "FullBody.glb",
    "RoboCow.glb",
    "TeleportPad.glb",
}


def main():
    batch_imports = []
    for entry in ImportStaticMeshes.IMPORTS:
        relative_path, *_ = ImportStaticMeshes._normalize_import_entry(entry)
        if str(relative_path) in TARGET_RELATIVE_PATHS:
            batch_imports.append(entry)

    unreal.log(
        f"[RunImportNewSourceAssetsAndExit] Running ImportStaticMeshes for {len(batch_imports)} staged SourceAssets/Import entries")

    original_imports = list(ImportStaticMeshes.IMPORTS)
    try:
        ImportStaticMeshes.IMPORTS = batch_imports
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
        unreal.log("[RunImportNewSourceAssetsAndExit] QUIT_EDITOR requested")
    except Exception as exc:
        unreal.log_warning(f"[RunImportNewSourceAssetsAndExit] Failed to request QUIT_EDITOR: {exc}")


if __name__ == "__main__":
    main()
