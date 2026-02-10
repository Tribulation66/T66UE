"""
Import leaderboard filter icon textures from SourceAssets/Icons into Unreal Content.

Source files:
  SourceAssets/Icons/Globe.png   -> /Game/UI/Leaderboard/T_LB_Global
  SourceAssets/Icons/Friends.png -> /Game/UI/Leaderboard/T_LB_Friends
  SourceAssets/Icons/Streamers.png -> /Game/UI/Leaderboard/T_LB_Streamers

Run from **inside the Editor** (Output Log → Python console) or via:
& "C:/Program Files/Epic Games/UE_5.7/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" "C:/UE/T66/T66.uproject" -run=pythonscript -script="C:/UE/T66/Scripts/ImportLeaderboardIcons.py" -unattended -nop4 -nosplash
"""

import os
import unreal


def _proj_dir() -> str:
    try:
        return unreal.SystemLibrary.get_project_directory()
    except Exception:
        return os.path.abspath(os.path.join(os.getcwd(), ".."))


def _ensure_dir(game_path: str):
    if not unreal.EditorAssetLibrary.does_directory_exist(game_path):
        unreal.EditorAssetLibrary.make_directory(game_path)


def _import_texture(src_file: str, dest_game_dir: str, dest_asset_name: str) -> str:
    """Import a single texture file using AutomatedAssetImportData (commandlet-safe)."""
    _ensure_dir(dest_game_dir)

    import_data = unreal.AutomatedAssetImportData()
    import_data.set_editor_property("destination_path", dest_game_dir)
    import_data.set_editor_property("filenames", [src_file])
    import_data.set_editor_property("replace_existing", True)
    import_data.set_editor_property("skip_read_only", True)

    factory = unreal.TextureFactory()
    import_data.set_editor_property("factory", factory)

    imported = unreal.AssetToolsHelpers.get_asset_tools().import_assets_automated(import_data)
    if not imported or len(imported) == 0:
        unreal.log_warning(f"[LB Icons] Import produced no assets: {src_file}")
        return ""

    obj = imported[0]
    asset_path = obj.get_path_name()
    # Strip the object suffix (e.g. ".T_LB_Global") to get the package path
    if "." in asset_path:
        asset_path = asset_path.rsplit(".", 1)[0]
    unreal.log(f"[LB Icons] Imported: {asset_path}")

    # Rename to target asset name if needed
    base_name = os.path.splitext(os.path.basename(src_file))[0]
    if base_name != dest_asset_name:
        dest_full = f"{dest_game_dir}/{dest_asset_name}"
        if unreal.EditorAssetLibrary.does_asset_exist(dest_full):
            unreal.EditorAssetLibrary.delete_asset(dest_full)
        success = unreal.EditorAssetLibrary.rename_asset(asset_path, dest_full)
        if success:
            asset_path = dest_full
            unreal.log(f"[LB Icons] Renamed to: {asset_path}")
        else:
            unreal.log_warning(f"[LB Icons] Rename failed for {asset_path} -> {dest_full}")

    return asset_path


def _apply_ui_texture_settings(asset_path: str):
    """Apply UI-friendly texture settings (no mipmaps, UI compression)."""
    if not asset_path:
        return
    tex = unreal.EditorAssetLibrary.load_asset(asset_path)
    if not tex or not isinstance(tex, unreal.Texture2D):
        return

    changed = False
    try:
        if tex.get_editor_property("mip_gen_settings") != unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS:
            tex.set_editor_property("mip_gen_settings", unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
            changed = True
    except Exception:
        pass

    try:
        if tex.get_editor_property("compression_settings") != unreal.TextureCompressionSettings.TC_EDITOR_ICON:
            tex.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_EDITOR_ICON)
            changed = True
    except Exception:
        pass

    if changed:
        try:
            if hasattr(tex, "modify"):
                tex.modify()
        except Exception:
            pass
        try:
            unreal.EditorAssetLibrary.save_loaded_asset(tex)
        except Exception:
            pass


def main():
    proj = _proj_dir()
    icons_dir = os.path.join(proj, "SourceAssets", "Icons")
    dest_dir = "/Game/UI/Leaderboard"

    mapping = {
        "Globe.png":     "T_LB_Global",
        "Friends.png":   "T_LB_Friends",
        "Streamers.png": "T_LB_Streamers",
    }

    imported = 0
    for filename, asset_name in mapping.items():
        src = os.path.join(icons_dir, filename)
        if not os.path.isfile(src):
            unreal.log_warning(f"[LB Icons] Source file not found: {src}")
            continue
        result = _import_texture(src, dest_dir, asset_name)
        if result:
            _apply_ui_texture_settings(result)
            imported += 1

    unreal.log(f"[LB Icons] Done. Imported {imported}/{len(mapping)} textures.")


if __name__ == "__main__":
    main()
else:
    main()
