"""
One-shot automation (full editor):
- Import/update ALL sprite textures (including Games) from SourceAssets/Sprites into /Game/UI/Sprites
- Import/update ALL world models from SourceAssets/Models/*.zip into stable /Game paths
- Reimport core DataTables from Content/Data/*.csv (includes CharacterVisuals)

Run (PowerShell):
& "C:/Program Files/Epic Games/UE_5.7/Engine/Binaries/Win64/UnrealEditor.exe" "C:/UE/T66/T66.uproject" -ExecutePythonScript="C:/UE/T66/Scripts/SetupAllAssetsAndDataTables.py" -unattended -nop4 -nosplash
"""

import os
import sys
import unreal


def _safe_call(fn, label: str):
    try:
        unreal.log(f"[SetupAll] {label}...")
        fn()
        unreal.log(f"[SetupAll] {label} done.")
    except Exception as e:
        unreal.log_error(f"[SetupAll] {label} failed: {e}")


def main():
    unreal.log("=== SetupAllAssetsAndDataTables: START ===")

    # Ensure project Scripts/ is importable as a module root.
    scripts_dir = os.path.dirname(os.path.abspath(__file__))
    if scripts_dir not in sys.path:
        sys.path.insert(0, scripts_dir)

    try:
        import ImportSpriteTextures
        _safe_call(ImportSpriteTextures.main, "ImportSpriteTextures")
    except Exception as e:
        unreal.log_error(f"[SetupAll] ImportSpriteTextures import failed: {e}")

    try:
        import ImportModels
        _safe_call(ImportModels.main, "ImportModels")
    except Exception as e:
        unreal.log_error(f"[SetupAll] ImportModels import failed: {e}")

    try:
        import ImportData
        _safe_call(ImportData.main, "ImportData (DataTables)")
    except Exception as e:
        unreal.log_error(f"[SetupAll] ImportData import failed: {e}")

    unreal.log("=== SetupAllAssetsAndDataTables: DONE ===")
    try:
        unreal.SystemLibrary.quit_editor()
    except Exception:
        pass


if __name__ == "__main__":
    main()

