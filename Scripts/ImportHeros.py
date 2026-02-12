"""
Import hero skeletal meshes, animations, and portraits from SourceAssets/Heros.

Maps: Knight -> Hero_1, Ninja -> Hero_2, Cowboy -> Hero_3, Wizard -> Hero_4.
For each: Default/Type A, Default/Type B, Beach/Type A, Beach/Type B (mesh + walk + alert + run).
Portraits: Portraits/Type A and Type B PNGs -> /Game/UI/Sprites/Heros/Hero_N/T_Hero_N_TypeA, T_Hero_N_TypeB.

Run INSIDE Unreal Editor: Tools -> Execute Python Script -> Scripts/ImportHeros.py
Then run: SetupCharacterVisualsDataTable.py and reimport Heroes.csv (ImportData.py or FullSetup).

Walking/Alert FBX are imported as animation-only (import_mesh=False) to avoid bind pose/smoothing warnings. You may still see one warning per Character mesh; the engine auto-fixes bind poses and the import succeeds.
"""

import os
import unreal

HERO_FOLDER_MAP = [
    ("Knight", "Hero_1"),
    ("Ninja", "Hero_2"),
    ("Cowboy", "Hero_3"),
    ("Wizard", "Hero_4"),
]


def _proj_dir():
    try:
        return unreal.SystemLibrary.get_project_directory().replace("\\", "/")
    except Exception:
        return os.path.abspath(os.path.join(os.getcwd(), "..")).replace("\\", "/")


def _ensure_game_dir(game_path):
    if not unreal.EditorAssetLibrary.does_directory_exist(game_path):
        unreal.EditorAssetLibrary.make_directory(game_path)


def _find_fbx_in_dir(root_dir, name_part):
    """Find first FBX under root_dir whose filename contains name_part (e.g. 'Character_output', 'Walking')."""
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
    """Import FBX as skeletal mesh and/or animation. Use import_mesh=False for animation-only FBX to avoid bind pose/smoothing warnings."""
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


def _resolve_type_folder(base_path, skin, body_type):
    """base_path = Heros/Knight, skin = Default|Beach, body_type = Type A|Type B."""
    type_dir = os.path.join(base_path, skin, body_type)
    if not os.path.isdir(type_dir):
        return None
    # Type A: e.g. Def_Knight_TA or Beach_Knight_TA (one subfolder with fbxs).
    # Type B: e.g. Def_Knight_TB/Meshy_AI_biped (fbxs inside).
    for root, dirs, files in os.walk(type_dir):
        if _find_fbx_in_dir(root, "Character_output"):
            return root
    return None


def main():
    proj = _proj_dir()
    heros_root = os.path.join(proj, "SourceAssets", "Heros")
    if not os.path.isdir(heros_root):
        unreal.log_error("[ImportHeros] SourceAssets/Heros not found")
        return

    unreal.log("=== ImportHeros: START ===")
    imported = 0

    for folder_name, hero_id in HERO_FOLDER_MAP:
        base_path = os.path.join(heros_root, folder_name)
        if not os.path.isdir(base_path):
            unreal.log_warning(f"[ImportHeros] Skip {folder_name}: not found")
            continue

        # Content base for this hero
        game_hero_base = f"/Game/Characters/Heroes/{hero_id}"
        game_portrait_base = f"/Game/UI/Sprites/Heros/{hero_id}"
        _ensure_game_dir(game_hero_base)
        _ensure_game_dir(game_portrait_base)

        # Portraits
        for slot, body in [("TypeA", "Type A"), ("TypeB", "Type B")]:
            port_dir = os.path.join(base_path, "Portraits", body)
            if os.path.isdir(port_dir):
                png = _find_first_png(port_dir)
                if png:
                    dest_name = f"T_{hero_id}_{slot}"
                    _import_texture(png, game_portrait_base, dest_name)
                    unreal.log(f"[ImportHeros] Portrait {hero_id} {slot}: {png} -> {game_portrait_base}/{dest_name}")
                    imported += 1

        # Skins: Default, Beach. Body: Type A, Type B.
        for skin in ["Default", "Beach"]:
            skin_key = "Default" if skin == "Default" else "Beachgoer"
            for body_type, slot in [("Type A", "TypeA"), ("Type B", "TypeB")]:
                src_dir = _resolve_type_folder(base_path, skin, body_type)
                if not src_dir:
                    unreal.log_warning(f"[ImportHeros] No source for {hero_id} {skin} {slot}: {base_path}")
                    continue

                dest_sub = f"{skin}_{slot}"  # Default_TypeA, Default_TypeB, Beach_TypeA, Beach_TypeB
                game_dest = f"{game_hero_base}/{dest_sub}"
                _ensure_game_dir(game_dest)

                mesh_fbx = _find_fbx_in_dir(src_dir, "Character_output")
                walk_fbx = _find_fbx_in_dir(src_dir, "Walking")
                alert_fbx = _find_fbx_in_dir(src_dir, "Alert")
                run_fbx = _find_fbx_in_dir(src_dir, "Running")

                if mesh_fbx:
                    mesh_name = f"SK_{hero_id}_{dest_sub}"
                    _import_fbx_skeletal(mesh_fbx, game_dest, mesh_name, import_animations=False, import_mesh=True)
                    unreal.log(f"[ImportHeros] Mesh {hero_id} {dest_sub}: {mesh_fbx}")
                    imported += 1
                if walk_fbx:
                    anim_name = f"AM_{hero_id}_{dest_sub}_Walking"
                    _import_fbx_skeletal(walk_fbx, game_dest, anim_name, import_animations=True, import_mesh=False)
                    imported += 1
                if alert_fbx:
                    anim_name = f"AM_{hero_id}_{dest_sub}_Alert"
                    _import_fbx_skeletal(alert_fbx, game_dest, anim_name, import_animations=True, import_mesh=False)
                    imported += 1
                if run_fbx:
                    anim_name = f"AM_{hero_id}_{dest_sub}_Running"
                    _import_fbx_skeletal(run_fbx, game_dest, anim_name, import_animations=True, import_mesh=False)
                    imported += 1

    unreal.log(f"=== ImportHeros: DONE (imported {imported} assets) ===")


if __name__ == "__main__":
    main()
