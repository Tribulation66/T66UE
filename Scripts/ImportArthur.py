"""
Reimport Arthur (Hero_1) using the legacy FBX pipeline.

The previous import used the Interchange pipeline which created materials
with parameter names (Image_0_006, etc.) that don't match DiffuseColorMap.
This script reimports using FbxImportUI so that materials expose
DiffuseColorMap — compatible with M_FBX_Unlit reparenting.

Source files:
  SourceAssets/Import 1/Hero/ArthurIdle.fbx  (mesh + skeleton + idle anim)
  SourceAssets/Import 1/Hero/ArthurWalk.fbx  (walk anim only)

After running this, run ReparentToFBXUnlit.py to make the materials Unlit.

Run in-editor: Tools > Execute Python Script > Scripts/ImportArthur.py
"""

import os
import unreal

LOG_PREFIX = "[ImportArthur]"
DEST_DIR = "/Game/Characters/Heroes/Hero_1/TypeA"
MESH_NAME = "SK_Hero_1_TypeA"
IDLE_ANIM = "AM_Hero_1_TypeA_Idle"
WALK_ANIM = "AM_Hero_1_TypeA_Walk"


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
            unreal.log(f"{LOG_PREFIX} Deleted existing: {game_dir}")
    except Exception as e:
        unreal.log_warning(f"{LOG_PREFIX} Could not delete {game_dir}: {e}")


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
    unreal.log_warning(f"{LOG_PREFIX} Failed to rename: {old_path} -> {new_path}")
    return old_path


def _find_of_class(imported_paths, target_classes):
    for path in imported_paths or []:
        clean = str(path).split(".")[0]
        asset = _load_asset(clean)
        if asset and _asset_class_name(asset) in target_classes:
            return clean, asset
    return None, None


def _import_fbx(fbx_path, dest_dir, dest_name,
                import_animations=True, import_mesh=True, skeleton=None):
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
    if skeleton is not None:
        try:
            opts.set_editor_property("skeleton", skeleton)
        except Exception:
            pass
    if not import_mesh and import_animations:
        try:
            opts.set_editor_property("mesh_type_to_import",
                                     unreal.FBXImportType.FBXIT_ANIMATION)
            opts.set_editor_property("original_import_type",
                                     unreal.FBXImportType.FBXIT_ANIMATION)
        except Exception:
            pass

    task.options = opts
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    return list(task.imported_object_paths or [])


def main():
    proj = _proj_dir()
    idle_fbx = os.path.join(proj, "SourceAssets", "Import 1", "Hero", "ArthurIdle.fbx")
    walk_fbx = os.path.join(proj, "SourceAssets", "Import 1", "Hero", "ArthurWalk.fbx")

    if not os.path.isfile(idle_fbx):
        unreal.log_error(f"{LOG_PREFIX} ArthurIdle.fbx not found at: {idle_fbx}")
        return
    if not os.path.isfile(walk_fbx):
        unreal.log_error(f"{LOG_PREFIX} ArthurWalk.fbx not found at: {walk_fbx}")
        return

    unreal.log("=" * 60)
    unreal.log(f"{LOG_PREFIX} ImportArthur START (legacy FBX pipeline)")
    unreal.log("=" * 60)

    # Clean existing Hero_1 to remove broken Interchange-imported materials
    _delete_dir_if_exists(DEST_DIR)
    _ensure_game_dir(DEST_DIR)

    # Step 1: Import mesh + materials + textures from Idle FBX
    unreal.log(f"{LOG_PREFIX} Step 1: Importing mesh from ArthurIdle.fbx ...")
    mesh_paths = _import_fbx(idle_fbx, DEST_DIR, MESH_NAME,
                              import_animations=False, import_mesh=True)
    unreal.log(f"{LOG_PREFIX}   Imported {len(mesh_paths)} assets")
    for p in mesh_paths:
        unreal.log(f"{LOG_PREFIX}     -> {p}")

    skeleton = None
    mesh_asset = _load_asset(f"{DEST_DIR}/{MESH_NAME}")
    if mesh_asset:
        try:
            skeleton = mesh_asset.get_editor_property("skeleton")
        except Exception:
            pass

    if not skeleton:
        unreal.log_error(f"{LOG_PREFIX} FAILED: no Skeleton found after mesh import")
        return

    unreal.log(f"{LOG_PREFIX}   Skeleton: {skeleton.get_path_name()}")

    # Step 2: Import Idle animation (anim only, reuse skeleton)
    unreal.log(f"{LOG_PREFIX} Step 2: Importing Idle animation ...")
    idle_paths = _import_fbx(idle_fbx, DEST_DIR, IDLE_ANIM,
                              import_animations=True, import_mesh=False,
                              skeleton=skeleton)
    idle_anim, _ = _find_of_class(idle_paths, {"AnimSequence", "AnimationAsset"})
    if idle_anim:
        _rename_asset(idle_anim, f"{DEST_DIR}/{IDLE_ANIM}")
        unreal.log(f"{LOG_PREFIX}   Idle anim: OK")
    else:
        unreal.log_warning(f"{LOG_PREFIX}   Idle anim: FAILED (no AnimSequence found)")

    # Step 3: Import Walk animation (anim only, reuse skeleton)
    unreal.log(f"{LOG_PREFIX} Step 3: Importing Walk animation ...")
    walk_paths = _import_fbx(walk_fbx, DEST_DIR, WALK_ANIM,
                              import_animations=True, import_mesh=False,
                              skeleton=skeleton)
    walk_anim, _ = _find_of_class(walk_paths, {"AnimSequence", "AnimationAsset"})
    if walk_anim:
        _rename_asset(walk_anim, f"{DEST_DIR}/{WALK_ANIM}")
        unreal.log(f"{LOG_PREFIX}   Walk anim: OK")
    else:
        unreal.log_warning(f"{LOG_PREFIX}   Walk anim: FAILED (no AnimSequence found)")

    # Log the materials created so user can verify DiffuseColorMap is populated
    unreal.log(f"{LOG_PREFIX} Checking materials ...")
    try:
        assets = unreal.EditorAssetLibrary.list_assets(DEST_DIR, recursive=True, include_folder=False)
        for p in assets:
            asset = _load_asset(p)
            if asset and isinstance(asset, unreal.MaterialInstanceConstant):
                mic_name = asset.get_name()
                parent = None
                try:
                    parent = asset.get_editor_property("parent")
                except Exception:
                    pass
                parent_name = parent.get_name() if parent else "None"

                has_tex = False
                try:
                    result = unreal.MaterialEditingLibrary.get_material_instance_texture_parameter_value(
                        asset, "DiffuseColorMap"
                    )
                    tex = result[-1] if isinstance(result, (tuple, list)) and len(result) > 1 else result
                    has_tex = tex is not None and isinstance(tex, unreal.Texture)
                except Exception:
                    pass

                status = "HAS TEXTURE" if has_tex else "NO TEXTURE"
                unreal.log(f"{LOG_PREFIX}   MIC: {mic_name} (parent: {parent_name}) — {status}")
    except Exception as e:
        unreal.log_warning(f"{LOG_PREFIX}   Could not list materials: {e}")

    unreal.log("=" * 60)
    unreal.log(f"{LOG_PREFIX} DONE — Now run ReparentToFBXUnlit.py to make materials Unlit")
    unreal.log("=" * 60)


if __name__ == "__main__":
    main()
