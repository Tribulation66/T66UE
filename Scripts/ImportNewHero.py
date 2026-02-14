"""
Import new Hero 1 TypeA from SourceAssets/NewHero.

Source files:
  - IDLE/IDLE.fbx              -> SK_Hero_1_Default_TypeA (mesh + textures)
                                + AM_Hero_1_Default_TypeA_Alert_Anim (idle animation)
  - WALK/walk-relaxed-2loop-378986.fbx -> AM_Hero_1_Default_TypeA_Walking_Anim (walk loop)

Destination: /Game/Characters/Heroes/Hero_1/Default_TypeA/

Run INSIDE Unreal Editor: Tools -> Execute Python Script -> Scripts/ImportNewHero.py
Then run: Scripts/ImportData.py to reimport CharacterVisuals.csv into DT_CharacterVisuals.
"""

import os
import unreal

DEST_DIR = "/Game/Characters/Heroes/Hero_1/Default_TypeA"


def _proj_dir():
    try:
        return unreal.SystemLibrary.get_project_directory().replace("\\", "/")
    except Exception:
        return os.path.abspath(os.path.join(os.getcwd(), "..")).replace("\\", "/")


def _ensure_game_dir(game_path):
    if not unreal.EditorAssetLibrary.does_directory_exist(game_path):
        unreal.EditorAssetLibrary.make_directory(game_path)


def _import_fbx(fbx_path, dest_dir, dest_name, import_mesh=True, import_animations=True):
    """Import FBX as skeletal mesh and/or animation.
    Use import_mesh=False for animation-only import (avoids bind pose warnings)."""
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
    paths = list(task.imported_object_paths or [])
    unreal.log(f"[ImportNewHero] {dest_name}: imported {len(paths)} asset(s) -> {paths}")
    return paths


def main():
    proj = _proj_dir()
    src = os.path.join(proj, "SourceAssets", "NewHero")

    idle_fbx = os.path.join(src, "IDLE", "IDLE.fbx")
    walk_fbx = os.path.join(src, "WALK", "walk-relaxed-2loop-378986.fbx")

    if not os.path.isfile(idle_fbx):
        unreal.log_error(f"[ImportNewHero] IDLE.fbx not found: {idle_fbx}")
        return
    if not os.path.isfile(walk_fbx):
        unreal.log_error(f"[ImportNewHero] Walk FBX not found: {walk_fbx}")
        return

    _ensure_game_dir(DEST_DIR)
    unreal.log("=== ImportNewHero: START ===")

    # 1. Import mesh from IDLE.fbx (mesh + textures, no animation)
    unreal.log("[ImportNewHero] Step 1/3: Importing skeletal mesh from IDLE.fbx ...")
    _import_fbx(idle_fbx, DEST_DIR, "SK_Hero_1_Default_TypeA",
                import_mesh=True, import_animations=False)

    # 2. Import idle/alert animation from IDLE.fbx (animation only, no mesh)
    unreal.log("[ImportNewHero] Step 2/3: Importing alert/idle animation from IDLE.fbx ...")
    _import_fbx(idle_fbx, DEST_DIR, "AM_Hero_1_Default_TypeA_Alert_Anim",
                import_mesh=False, import_animations=True)

    # 3. Import walk loop animation (animation only, no mesh)
    unreal.log("[ImportNewHero] Step 3/3: Importing walk animation from walk-relaxed-2loop ...")
    _import_fbx(walk_fbx, DEST_DIR, "AM_Hero_1_Default_TypeA_Walking_Anim",
                import_mesh=False, import_animations=True)

    unreal.log("=== ImportNewHero: DONE ===")
    unreal.log("[ImportNewHero] Next: run Scripts/ImportData.py to reimport CharacterVisuals.csv")


if __name__ == "__main__":
    main()
