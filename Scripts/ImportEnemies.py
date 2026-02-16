"""
Import enemy and boss skeletal meshes from SourceAssets/Enemies/.

Expects: Enemy1.fbx, Enemy2.fbx, Enemy3.fbx, Boss.fbx
Imports to: /Game/Characters/Enemies/Enemy1/SK_Enemy1, etc.

Run INSIDE Unreal Editor: Tools -> Execute Python Script -> Scripts/ImportEnemies.py
Then run Scripts/ImportData.py (or reimport CharacterVisuals.csv into DT_CharacterVisuals).
"""

import os
import unreal


def _proj_dir():
    try:
        return unreal.SystemLibrary.get_project_directory().replace("\\", "/")
    except Exception:
        return os.path.abspath(os.path.join(os.getcwd(), "..")).replace("\\", "/")


def _ensure_game_dir(game_path):
    if not unreal.EditorAssetLibrary.does_directory_exist(game_path):
        unreal.EditorAssetLibrary.make_directory(game_path)


def _import_fbx_skeletal(fbx_path, dest_dir, dest_name):
    _ensure_game_dir(dest_dir)
    task = unreal.AssetImportTask()
    task.automated = True
    task.save = True
    task.replace_existing = True
    task.replace_existing_settings = True
    task.filename = fbx_path
    task.destination_path = dest_dir
    task.destination_name = dest_name

    opts = unreal.FbxImportUI()
    opts.import_mesh = True
    opts.import_textures = True
    opts.import_materials = True
    opts.import_as_skeletal = True
    opts.import_animations = True
    try:
        opts.set_editor_property("create_physics_asset", False)
    except Exception:
        pass
    task.options = opts

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    paths = list(task.imported_object_paths or [])
    unreal.log(f"[ImportEnemies] {dest_name}: imported {len(paths)} asset(s) -> {paths}")
    return paths


def main():
    proj = _proj_dir()
    src = os.path.join(proj, "SourceAssets", "Enemies")

    if not os.path.isdir(src):
        unreal.log_error(f"[ImportEnemies] Source folder not found: {src}")
        return

    # Map source filename (no ext) -> (Content folder name, asset name)
    entries = [
        ("Enemy1", "Enemy1", "SK_Enemy1"),
        ("Enemy2", "Enemy2", "SK_Enemy2"),
        ("Enemy3", "Enemy3", "SK_Enemy3"),
        ("Boss", "Boss", "SK_Boss"),
    ]

    unreal.log("=== ImportEnemies: START ===")
    for fbx_base, folder_name, asset_name in entries:
        fbx_path = os.path.join(src, fbx_base + ".fbx")
        if not os.path.isfile(fbx_path):
            unreal.log_warning(f"[ImportEnemies] Skip {fbx_base}: not found at {fbx_path}")
            continue
        dest_dir = f"/Game/Characters/Enemies/{folder_name}"
        _import_fbx_skeletal(fbx_path, dest_dir, asset_name)
    unreal.log("=== ImportEnemies: DONE ===")
    unreal.log("[ImportEnemies] Next: reimport CharacterVisuals.csv into DT_CharacterVisuals (ImportData.py or editor).")


if __name__ == "__main__":
    main()
