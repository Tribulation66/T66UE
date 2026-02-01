"""
One-shot setup:
- Import/update sprite textures from SourceAssets into /Game/UI/Sprites
- Reimport core DataTables from Content/Data/*.csv

Run:
& "C:/Program Files/Epic Games/UE_5.7/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" "C:/UE/T66/T66.uproject" -run=pythonscript -script="C:/UE/T66/Scripts/SetupSpritesAndDataTables.py" -unattended -nop4 -nosplash
"""

import os
import sys
import unreal


def main():
    unreal.log("=== SetupSpritesAndDataTables: START ===")
    # Ensure project Scripts/ is importable as a module root.
    scripts_dir = os.path.dirname(os.path.abspath(__file__))
    if scripts_dir not in sys.path:
        sys.path.insert(0, scripts_dir)

    # Safety: commandlet runs don't have Slate/ContentBrowser, and importing assets can crash.
    # Use this script primarily inside the full editor (Tools -> Execute Python Script).
    try:
        if unreal.SystemLibrary.is_running_commandlet():
            unreal.log_warning("Running under commandlet; skipping sprite import to avoid Slate/AssetTools crash.")
    except Exception:
        pass
    try:
        import ImportSpriteTextures
        try:
            if not unreal.SystemLibrary.is_running_commandlet():
                ImportSpriteTextures.main()
        except Exception:
            # If is_running_commandlet isn't available, still attempt import.
            ImportSpriteTextures.main()
    except Exception as e:
        unreal.log_error(f"ImportSpriteTextures failed: {e}")

    try:
        import ImportData
        ImportData.main()
    except Exception as e:
        unreal.log_error(f"ImportData failed: {e}")

    # If running in full editor with -ExecutePythonScript, quit when done.
    try:
        unreal.SystemLibrary.quit_editor()
    except Exception:
        pass

    unreal.log("=== SetupSpritesAndDataTables: DONE ===")


if __name__ == "__main__":
    main()

