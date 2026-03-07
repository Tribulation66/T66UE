"""
Import enemy models from SourceAssets/Enemies.

Expected source layout:
  SourceAssets/Enemies/
    RoostWalk.fbx
    CowWalk.fbx
    GoatWalk.fbx
    PigWalk.fbx

Each FBX contains a skeletal mesh with a walk animation.
Uses the legacy FBX pipeline (FbxImportUI) — same as ImportCompanions.py —
so that materials expose DiffuseColorMap for emissive fill lighting.

Run INSIDE Unreal Editor:
  Tools -> Execute Python Script -> Scripts/ImportEnemies.py
"""

import os
import unreal

ENEMY_NAMES = ["Roost", "Cow", "Goat", "Pig"]


def _proj_dir():
    try:
        return unreal.SystemLibrary.get_project_directory().replace("\\", "/")
    except Exception:
        return os.path.abspath(os.path.join(os.getcwd(), "..")).replace("\\", "/")


def _ensure_game_dir(game_path):
    if not unreal.EditorAssetLibrary.does_directory_exist(game_path):
        unreal.EditorAssetLibrary.make_directory(game_path)


def _delete_dir_if_exists(game_dir):
    try:
        if unreal.EditorAssetLibrary.does_directory_exist(game_dir):
            unreal.EditorAssetLibrary.delete_directory(game_dir)
    except Exception:
        pass


def _load_asset(path):
    try:
        return unreal.EditorAssetLibrary.load_asset(path)
    except Exception:
        return None


def _asset_class_name(asset):
    try:
        return asset.get_class().get_name()
    except Exception:
        return ""


def _rename_asset(old_path, new_path):
    if not old_path or old_path == new_path:
        return new_path
    if unreal.EditorAssetLibrary.does_asset_exist(new_path):
        unreal.EditorAssetLibrary.delete_asset(new_path)
    if unreal.EditorAssetLibrary.rename_asset(old_path, new_path):
        return new_path
    unreal.log_warning(f"[ImportEnemies] Failed to rename: {old_path} -> {new_path}")
    return old_path


def _import_fbx(fbx_path, dest_dir, dest_name,
                import_animations=True, import_mesh=True):
    """Legacy FBX import (FbxImportUI) — same pipeline as ImportCompanions.py."""
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
    opts.import_mesh = bool(import_mesh)
    opts.import_textures = bool(import_mesh)
    opts.import_materials = bool(import_mesh)
    opts.import_as_skeletal = True
    opts.import_animations = bool(import_animations)
    try:
        opts.set_editor_property("create_physics_asset", False)
    except Exception:
        pass

    task.options = opts
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    return list(task.imported_object_paths or [])


def main():
    proj = _proj_dir()
    enemies_root = os.path.join(proj, "SourceAssets", "Enemies")
    if not os.path.isdir(enemies_root):
        unreal.log_error("[ImportEnemies] SourceAssets/Enemies not found")
        return

    unreal.log("=== ImportEnemies: START (legacy FBX pipeline) ===")
    success_count = 0

    for enemy_name in ENEMY_NAMES:
        fbx_filename = f"{enemy_name}Walk.fbx"
        fbx_path = os.path.join(enemies_root, fbx_filename)
        if not os.path.isfile(fbx_path):
            unreal.log_warning(
                f"[ImportEnemies] Skip {enemy_name}: {fbx_filename} not found"
            )
            continue

        dest_dir = f"/Game/Characters/Enemies/{enemy_name}"
        mesh_name = f"SK_{enemy_name}"
        walk_anim_name = f"AM_{enemy_name}_Walk"

        unreal.log(f"[ImportEnemies] === {enemy_name} ===")

        # Clean destination to remove old Interchange-created materials
        _delete_dir_if_exists(dest_dir)
        _ensure_game_dir(dest_dir)

        # Import mesh + materials + textures + walk animation in one shot
        paths = _import_fbx(fbx_path, dest_dir, mesh_name,
                             import_animations=True, import_mesh=True)
        unreal.log(f"[ImportEnemies]   Import created {len(paths)} assets")

        # Legacy FBX may create the anim as <dest_name>_Anim; rename to expected name
        found_anim = False
        for p in paths or []:
            clean = str(p).split(".")[0]
            asset = _load_asset(clean)
            if asset and _asset_class_name(asset) in ("AnimSequence", "AnimationAsset"):
                _rename_asset(clean, f"{dest_dir}/{walk_anim_name}")
                unreal.log(f"[ImportEnemies]   Walk anim: OK")
                found_anim = True
                break

        if not found_anim:
            anim_guess = f"{dest_dir}/{mesh_name}_Anim"
            if _load_asset(anim_guess):
                _rename_asset(anim_guess, f"{dest_dir}/{walk_anim_name}")
                unreal.log(f"[ImportEnemies]   Walk anim (_Anim fallback): OK")
            else:
                unreal.log_warning(f"[ImportEnemies]   Walk anim: NOT FOUND")

        success_count += 1
        unreal.log(f"[ImportEnemies]   {enemy_name} done.")

    unreal.log(f"=== ImportEnemies: DONE ({success_count}/{len(ENEMY_NAMES)} enemies) ===")


if __name__ == "__main__":
    main()
