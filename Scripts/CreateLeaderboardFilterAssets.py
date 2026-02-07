"""
Create leaderboard filter button assets: import Global/Friends/Streamers textures,
create sprite-sheet material M_LeaderboardStreamersSprite, and create WBP_LeaderboardFilterButton.

Prerequisites:
1. Run:  python Scripts/LeaderboardGifToSpriteSheet.py  (creates Streamers_SpriteSheet.png)
2. Ensure SourceAssets/Leaderboard/ has Global.png, Friends.png, Streamers_SpriteSheet.png

Run from Unreal Editor: Tools -> Execute Python Script -> Scripts/CreateLeaderboardFilterAssets.py
Or: UnrealEditor-Cmd.exe T66.uproject -run=pythonscript -script="Scripts/CreateLeaderboardFilterAssets.py"
"""

import os
import unreal


def _proj_dir():
    try:
        return unreal.SystemLibrary.get_project_directory()
    except Exception:
        return os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))


def _ensure_dir(game_path: str):
    if not unreal.EditorAssetLibrary.does_directory_exist(game_path):
        unreal.EditorAssetLibrary.make_directory(game_path)


def _import_texture(src_path: str, dest_dir: str, dest_name: str) -> str:
    if not os.path.isfile(src_path):
        unreal.log_warning(f"Source not found: {src_path}")
        return ""
    _ensure_dir(dest_dir)
    task = unreal.AssetImportTask()
    task.automated = True
    task.save = True
    task.replace_existing = True
    task.replace_existing_settings = True
    task.filename = src_path
    task.destination_path = dest_dir
    task.destination_name = dest_name
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    if not task.imported_object_paths:
        return ""
    return task.imported_object_paths[0]


def _create_sprite_sheet_material(dest_path: str, num_columns: int, num_rows: int) -> bool:
    """Create M_LeaderboardStreamersSprite with scalar 'Frame' and texture param. UV math must be wired in Material Editor."""
    if unreal.EditorAssetLibrary.does_asset_exist(dest_path):
        unreal.log(f"Material already exists: {dest_path}")
        return True
    parent = unreal.load_object(None, "/Script/Engine.Material")
    if not parent:
        unreal.log_error("Could not load Material class")
        return False
    factory = unreal.MaterialFactoryNew()
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    package_path = dest_path.rsplit(".", 1)[0]
    asset_name = "M_LeaderboardStreamersSprite"
    pkg_path = "/Game/UI/Leaderboard"
    mat = asset_tools.create_asset(asset_name, pkg_path, unreal.Material, factory)
    if not mat:
        unreal.log_error("Failed to create Material")
        return False
    # Add scalar parameter 'Frame' (0..FrameCount-1) and texture param for sprite sheet.
    try:
        mat_edit_lib = unreal.MaterialEditingLibrary
        frame_param = mat_edit_lib.create_material_expression(mat, unreal.MaterialExpressionScalarParameter, -384, -100)
        if frame_param:
            frame_param.set_editor_property("parameter_name", "Frame")
            frame_param.set_editor_property("default_value", 0.0)
        tex_param = mat_edit_lib.create_material_expression(mat, unreal.MaterialExpressionTextureSampleParameter2D, -384, 0)
        if tex_param:
            tex_param.set_editor_property("parameter_name", "SpriteSheet")
        # Connect texture sample to emissive so the material displays (user can refine in editor)
        if tex_param:
            mat_edit_lib.connect_material_property(tex_param, "RGB", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    except Exception as e:
        unreal.log_warning(f"Material expressions could not be fully set up: {e}. Open M_LeaderboardStreamersSprite and add sprite-sheet UV logic (Frame, NumColumns, NumRows).")
    unreal.EditorAssetLibrary.save_asset(dest_path)
    unreal.log(f"Created material: {dest_path}. Set NumColumns={num_columns}, NumRows={num_rows} in the material if needed.")
    return True


def _create_filter_button_blueprint() -> bool:
    parent_class = unreal.load_object(None, "/Script/T66.T66LeaderboardFilterButton")
    if not parent_class:
        unreal.log_error("T66LeaderboardFilterButton not found. Rebuild the T66 module.")
        return False
    pkg_path = "/Game/UI/Leaderboard"
    asset_name = "WBP_LeaderboardFilterButton"
    full_path = f"{pkg_path}/{asset_name}"
    if unreal.EditorAssetLibrary.does_asset_exist(full_path):
        unreal.log(f"Widget Blueprint already exists: {full_path}")
        return True
    factory = unreal.WidgetBlueprintFactory()
    factory.set_editor_property("parent_class", parent_class)
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    created = asset_tools.create_asset(asset_name, pkg_path, None, factory)
    if not created:
        unreal.log_error("Failed to create WBP_LeaderboardFilterButton")
        return False
    unreal.EditorAssetLibrary.save_asset(full_path)
    unreal.log(f"Created {full_path}. Open it and add a Button (name: FilterButton) and an Image (name: IconImage) as the button's content; set button style to border-only if desired.")
    return True


def main():
    unreal.log("=== Create Leaderboard Filter Assets ===")
    proj = _proj_dir()
    src = os.path.join(proj, "SourceAssets", "Leaderboard")
    dest = "/Game/UI/Leaderboard"
    _ensure_dir(dest)

    # Import textures
    _import_texture(os.path.join(src, "Global.png"), dest, "T_Leaderboard_Global")
    _import_texture(os.path.join(src, "Friends.png"), dest, "T_Leaderboard_Friends")

    # Import Streamers GIF frames (individual PNGs: Streamers_Frame_00.png, _01.png, ...)
    frame_idx = 0
    while True:
        frame_file = os.path.join(src, f"Streamers_Frame_{frame_idx:02d}.png")
        if not os.path.isfile(frame_file):
            break
        _import_texture(frame_file, dest, f"T_Leaderboard_Streamers_Frame_{frame_idx:02d}")
        frame_idx += 1
    if frame_idx == 0:
        unreal.log_warning("No Streamers frame PNGs found. Run: python Scripts/LeaderboardGifToFrames.py")
    else:
        unreal.log(f"Imported {frame_idx} Streamers GIF frames.")

    # Sprite sheet dimensions (run LeaderboardGifToSpriteSheet.py first; typical output is 1x3 or 4x5)
    num_frames = 3
    cols = 1
    rows = 3
    _create_sprite_sheet_material(f"{dest}/M_LeaderboardStreamersSprite", cols, rows)
    _create_filter_button_blueprint()

    unreal.log("=== Create Leaderboard Filter Assets: done ===")


if __name__ == "__main__":
    main()
