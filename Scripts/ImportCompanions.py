"""
Import companion skeletal meshes, animations, and portraits from SourceAssets/Companions.

Expects: SourceAssets/Companions/Companion N/ with:
  - Def_CON/   (Default skin): Meshy_AI_Character_output.fbx, Meshy_AI_Animation_Walking_withSkin.fbx, Meshy_AI_Animation_Alert_withSkin.fbx
  - Beach_CON/ (Beach skin):   same
  - Portrait/  one PNG for the companion portrait

Maps "Companion 1" -> Companion_01, etc. (zero-padded 2 digits).
Outputs to /Game/Characters/Companions/Companion_NN/Default/ and .../Beach/:
  - Mesh: SK_Companion_NN_Default (or SK_Companion_NN_Beach)
  - Animations: AM_Companion_NN_Default_Walking + AM_Companion_NN_Default_Walking_Anim (Unreal creates both; CSV uses _Anim)
Portraits -> /Game/UI/Sprites/Companions/Companion_NN/T_Companion_NN.

Run INSIDE Unreal Editor: Tools -> Execute Python Script -> Scripts/ImportCompanions.py
Then run SetupCharacterVisualsDataTable.py to refresh DT_CharacterVisuals from CharacterVisuals.csv.
"""

import os
import re
import unreal


def _proj_dir():
    try:
        return unreal.SystemLibrary.get_project_directory().replace("\\", "/")
    except Exception:
        return os.path.abspath(os.path.join(os.getcwd(), "..")).replace("\\", "/")


def _ensure_game_dir(game_path):
    if not unreal.EditorAssetLibrary.does_directory_exist(game_path):
        unreal.EditorAssetLibrary.make_directory(game_path)


def _find_fbx_in_dir(root_dir, name_part):
    """Find first FBX under root_dir whose filename contains name_part."""
    for dirpath, _, files in os.walk(root_dir):
        for fn in files:
            if not fn.lower().endswith(".fbx"):
                continue
            if name_part.lower() in fn.lower():
                return os.path.join(dirpath, fn)
    return None


def _find_first_png(root_dir):
    for dirpath, _, files in os.walk(root_dir):
        for fn in files:
            if fn.lower().endswith(".png"):
                return os.path.join(dirpath, fn)
    return None


def _import_fbx_skeletal(fbx_path, dest_dir, dest_name, import_animations=True, import_mesh=True):
    """Import FBX as skeletal mesh and/or animation.
    
    When import_animations=True and import_mesh=False, Unreal creates an animation asset
    with _Anim suffix automatically (e.g., AM_X -> AM_X_Anim.uasset).
    """
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


def _import_texture(png_path, dest_dir, dest_name):
    _ensure_game_dir(dest_dir)
    task = unreal.AssetImportTask()
    task.automated = True
    task.save = True
    task.replace_existing = True
    task.filename = png_path
    task.destination_path = dest_dir
    task.destination_name = dest_name
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    return list(task.imported_object_paths or [])


def main():
    proj = _proj_dir()
    companions_root = os.path.join(proj, "SourceAssets", "Companions")
    if not os.path.isdir(companions_root):
        unreal.log_error("[ImportCompanions] SourceAssets/Companions not found")
        return

    # Discover "Companion N" folders and sort by N
    companion_folders = []
    for name in os.listdir(companions_root):
        match = re.match(r"Companion\s*(\d+)", name, re.IGNORECASE)
        if match:
            num = int(match.group(1))
            path = os.path.join(companions_root, name)
            if os.path.isdir(path):
                companion_folders.append((num, name, path))
    companion_folders.sort(key=lambda x: x[0])

    unreal.log("=== ImportCompanions: START ===")
    imported = 0

    for num, folder_name, base_path in companion_folders:
        companion_id = f"Companion_{num:02d}"
        game_base = f"/Game/Characters/Companions/{companion_id}"
        game_portrait_base = f"/Game/UI/Sprites/Companions/{companion_id}"
        _ensure_game_dir(game_base)
        _ensure_game_dir(game_portrait_base)

        # Portrait (single PNG per companion)
        port_dir = os.path.join(base_path, "Portrait")
        if os.path.isdir(port_dir):
            png = _find_first_png(port_dir)
            if png:
                dest_name = f"T_{companion_id}"
                _import_texture(png, game_portrait_base, dest_name)
                unreal.log(f"[ImportCompanions] Portrait {companion_id}: {png} -> {game_portrait_base}/{dest_name}")
                imported += 1

        # Skins: Def_CON -> Default, Beach_CON -> Beach
        for skin_folder, skin_key in [("Def_CO" + str(num), "Default"), ("Beach_CO" + str(num), "Beach")]:
            src_dir = os.path.join(base_path, skin_folder)
            if not os.path.isdir(src_dir):
                unreal.log_warning(f"[ImportCompanions] Skip {companion_id} {skin_key}: {src_dir} not found")
                continue

            dest_sub = skin_key  # Default or Beach
            game_dest = f"{game_base}/{dest_sub}"
            _ensure_game_dir(game_dest)

            mesh_fbx = _find_fbx_in_dir(src_dir, "Character_output")
            walk_fbx = _find_fbx_in_dir(src_dir, "Walking")
            alert_fbx = _find_fbx_in_dir(src_dir, "Alert")

            # Import mesh first
            if mesh_fbx:
                mesh_name = f"SK_{companion_id}_{dest_sub}"
                _import_fbx_skeletal(mesh_fbx, game_dest, mesh_name, import_animations=False, import_mesh=True)
                unreal.log(f"[ImportCompanions] Mesh {companion_id} {dest_sub}: {mesh_fbx}")
                imported += 1
            
            # Import animations WITH mesh enabled - this ensures Unreal recognizes the skeleton
            # and creates the _Anim.uasset AnimSequence (like it does for heroes)
            if walk_fbx:
                anim_name = f"AM_{companion_id}_{dest_sub}_Walking"
                _import_fbx_skeletal(walk_fbx, game_dest, anim_name, import_animations=True, import_mesh=True)
                unreal.log(f"[ImportCompanions] Anim {companion_id} {dest_sub} Walking: {walk_fbx}")
                imported += 1
            if alert_fbx:
                anim_name = f"AM_{companion_id}_{dest_sub}_Alert"
                _import_fbx_skeletal(alert_fbx, game_dest, anim_name, import_animations=True, import_mesh=True)
                unreal.log(f"[ImportCompanions] Anim {companion_id} {dest_sub} Alert: {alert_fbx}")
                imported += 1

    unreal.log(f"=== ImportCompanions: DONE (imported {imported} assets) ===")


if __name__ == "__main__":
    main()
