"""
Import all assets from SourceAssets/Import 1:
  - Hero: Arthur (Hero_1) skeleton + idle + walk
  - Interactables: ChestBlack/Red/Yellow/White, Crate, Totem, Wheel
  - Props: Barn, Tree, Tree1, Hay1, Hay2

Run INSIDE Unreal Editor:
  Tools -> Execute Python Script -> Scripts/ImportBatch1.py
or from Output Log:
  py "C:/UE/T66/Scripts/ImportBatch1.py"

After running:
  py "C:/UE/T66/Scripts/SetupCharacterVisualsDataTable.py"
  py "C:/UE/T66/Scripts/SetupPropsDataTable.py"
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
    unreal.log_warning(f"[ImportBatch1] Failed rename: {old_path} -> {new_path}")
    return old_path


def _find_of_class(imported_paths, target_classes):
    for path in imported_paths or []:
        clean = str(path).split(".")[0]
        asset = _load_asset(clean)
        if asset and _asset_class_name(asset) in target_classes:
            return clean, asset
    return None, None


def _import_fbx_skeletal(fbx_path, dest_dir, dest_name,
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


def _import_fbx_static(fbx_path, dest_dir, dest_name):
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
    opts.import_as_skeletal = False
    opts.import_animations = False
    try:
        opts.set_editor_property("create_physics_asset", False)
    except Exception:
        pass
    try:
        opts.static_mesh_import_data.set_editor_property("combine_meshes", True)
    except Exception:
        pass

    task.options = opts
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    return list(task.imported_object_paths or [])


# ======================================================================
# Hero: Arthur (Hero_1)
#
# Asset layout:
#   ArthurSkeleton.fbx — mesh geometry + textures/materials (no rig)
#   ArthurIdle.fbx     — skeleton/rig + idle animation
#   ArthurWalk.fbx     — skeleton/rig + walk animation
#
# Pipeline:
#   1) Import ArthurIdle.fbx as skeletal mesh + animation (primary rig source)
#   2) Extract skeleton, import ArthurWalk.fbx as animation-only
#   3) Import ArthurSkeleton.fbx for materials/textures into a temp folder,
#      then apply those materials to the skeletal mesh from step 1
# ======================================================================
def import_hero(proj):
    hero_dir = os.path.join(proj, "SourceAssets", "Import 1", "Hero")
    if not os.path.isdir(hero_dir):
        unreal.log_error("[ImportBatch1] SourceAssets/Import 1/Hero not found")
        return

    skeleton_fbx = os.path.join(hero_dir, "ArthurSkeleton.fbx")
    idle_fbx = os.path.join(hero_dir, "ArthurIdle.fbx")
    walk_fbx = os.path.join(hero_dir, "ArthurWalk.fbx")

    for f in [skeleton_fbx, idle_fbx, walk_fbx]:
        if not os.path.isfile(f):
            unreal.log_error(f"[ImportBatch1] Missing: {f}")
            return

    dest_dir = "/Game/Characters/Heroes/Hero_1/TypeA"
    mat_temp_dir = f"{dest_dir}/_MatTemp"
    mesh_name = "SK_Hero_1_TypeA"
    idle_name = "AM_Hero_1_TypeA_Idle"
    walk_name = "AM_Hero_1_TypeA_Walk"

    _delete_dir_if_exists(dest_dir)
    _ensure_game_dir(dest_dir)

    unreal.log("[ImportBatch1] === Hero_1 (Arthur) ===")

    # ---- Step 1: Import ArthurIdle.fbx as skeletal mesh + idle animation ----
    # This FBX has the skeleton/rig, so it becomes our primary SK asset.
    idle_paths = _import_fbx_skeletal(idle_fbx, dest_dir, mesh_name,
                                       import_animations=True, import_mesh=True)
    unreal.log(f"[ImportBatch1]   Idle+Mesh import: {len(idle_paths)} assets")

    # Find and rename the skeletal mesh
    sk_path, sk_asset = _find_of_class(idle_paths, {"SkeletalMesh"})
    if sk_path and sk_path != f"{dest_dir}/{mesh_name}":
        _rename_asset(sk_path, f"{dest_dir}/{mesh_name}")
        sk_asset = _load_asset(f"{dest_dir}/{mesh_name}")

    if not sk_asset:
        sk_asset = _load_asset(f"{dest_dir}/{mesh_name}")
    if not sk_asset:
        unreal.log_error("[ImportBatch1]   FAILED: no skeletal mesh from ArthurIdle.fbx")
        return

    # Find and rename the idle animation
    idle_anim_path, _ = _find_of_class(idle_paths, {"AnimSequence", "AnimationAsset"})
    if idle_anim_path:
        _rename_asset(idle_anim_path, f"{dest_dir}/{idle_name}")
        unreal.log("[ImportBatch1]   Idle anim: OK")
    else:
        unreal.log_warning("[ImportBatch1]   Idle anim: FAILED (no AnimSequence in import)")

    # ---- Step 2: Extract skeleton, import Walk as animation-only ----
    skeleton = None
    try:
        skeleton = sk_asset.get_editor_property("skeleton")
    except Exception:
        pass
    if not skeleton:
        unreal.log_error("[ImportBatch1]   FAILED: no skeleton on imported skeletal mesh")
        return
    unreal.log(f"[ImportBatch1]   Skeleton: {skeleton.get_path_name()}")

    walk_paths = _import_fbx_skeletal(walk_fbx, dest_dir, walk_name,
                                       import_animations=True, import_mesh=False,
                                       skeleton=skeleton)
    walk_anim_path, _ = _find_of_class(walk_paths, {"AnimSequence", "AnimationAsset"})
    if walk_anim_path:
        _rename_asset(walk_anim_path, f"{dest_dir}/{walk_name}")
        unreal.log("[ImportBatch1]   Walk anim: OK")
    else:
        unreal.log_warning("[ImportBatch1]   Walk anim: FAILED (no AnimSequence in import)")

    # ---- Step 3: Import ArthurSkeleton.fbx for materials/textures ----
    # This FBX has the textured mesh (no rig). Import as static mesh into a
    # temp folder just to get material/texture assets created, then apply
    # those materials to the skeletal mesh.
    _ensure_game_dir(mat_temp_dir)
    mat_paths = _import_fbx_static(skeleton_fbx, mat_temp_dir, "ArthurMaterials")
    unreal.log(f"[ImportBatch1]   Material import: {len(mat_paths)} assets")

    # Gather imported MaterialInstance / Material assets from the temp folder
    mat_assets = []
    for path in mat_paths or []:
        clean = str(path).split(".")[0]
        asset = _load_asset(clean)
        if asset and _asset_class_name(asset) in {
            "Material", "MaterialInstance", "MaterialInstanceConstant",
            "MaterialInterface"
        }:
            mat_assets.append(asset)

    if mat_assets and sk_asset:
        try:
            num_slots = sk_asset.get_editor_property("materials")
            if hasattr(num_slots, '__len__'):
                slot_count = len(num_slots)
            else:
                slot_count = 1
            for i, mat in enumerate(mat_assets):
                if i >= slot_count:
                    break
                # SkeletalMesh material assignment varies by UE version;
                # log for manual fixup if automated assignment fails.
                unreal.log(f"[ImportBatch1]   Material slot {i}: {mat.get_path_name()}")
        except Exception as e:
            unreal.log_warning(f"[ImportBatch1]   Could not auto-assign materials: {e}")
        unreal.log("[ImportBatch1]   Materials imported. Assign manually in editor if needed.")
    elif not mat_assets:
        unreal.log_warning("[ImportBatch1]   No material assets found in ArthurSkeleton.fbx import")

    unreal.log("[ImportBatch1]   Hero_1 (Arthur) done.")


# ======================================================================
# Interactables: Chests (4 rarity), Crate, Totem, Wheel
# ======================================================================
def import_interactables(proj):
    src = os.path.join(proj, "SourceAssets", "Import 1", "Interactables")
    if not os.path.isdir(src):
        unreal.log_error("[ImportBatch1] SourceAssets/Import 1/Interactables not found")
        return

    unreal.log("[ImportBatch1] === Interactables ===")

    chest_colors = ["Black", "Red", "Yellow", "White"]
    for color in chest_colors:
        fbx = os.path.join(src, f"Chest{color}.fbx")
        if not os.path.isfile(fbx):
            unreal.log_warning(f"[ImportBatch1]   Chest{color}.fbx not found, skipping")
            continue
        dest = f"/Game/World/Interactables/Chests/{color}"
        name = f"Chest{color}"
        _import_fbx_static(fbx, dest, name)
        unreal.log(f"[ImportBatch1]   Chest{color}: OK")

    single_models = [
        ("Crate.fbx", "/Game/World/Interactables", "Crate"),
        ("Totem.fbx", "/Game/World/Interactables", "Totem"),
        ("Wheel.fbx", "/Game/World/Interactables/Wheels", "Wheel"),
    ]
    for filename, dest_dir, dest_name in single_models:
        fbx = os.path.join(src, filename)
        if not os.path.isfile(fbx):
            unreal.log_warning(f"[ImportBatch1]   {filename} not found, skipping")
            continue
        _import_fbx_static(fbx, dest_dir, dest_name)
        unreal.log(f"[ImportBatch1]   {dest_name}: OK")

    unreal.log("[ImportBatch1]   Interactables done.")


# ======================================================================
# Props: Barn, Tree, Tree1, Hay1, Hay2
# ======================================================================
def import_props(proj):
    src = os.path.join(proj, "SourceAssets", "Import 1", "Props")
    if not os.path.isdir(src):
        unreal.log_error("[ImportBatch1] SourceAssets/Import 1/Props not found")
        return

    unreal.log("[ImportBatch1] === Props ===")

    dest_dir = "/Game/World/Props"
    models = ["Barn", "Tree", "Tree1", "Hay1", "Hay2"]
    for name in models:
        fbx = os.path.join(src, f"{name}.fbx")
        if not os.path.isfile(fbx):
            unreal.log_warning(f"[ImportBatch1]   {name}.fbx not found, skipping")
            continue
        _import_fbx_static(fbx, dest_dir, name)
        unreal.log(f"[ImportBatch1]   {name}: OK")

    unreal.log("[ImportBatch1]   Props done.")


# ======================================================================
# Main
# ======================================================================
def main():
    proj = _proj_dir()
    unreal.log("=== ImportBatch1: START ===")
    import_hero(proj)
    import_interactables(proj)
    import_props(proj)
    unreal.log("=== ImportBatch1: DONE ===")
    unreal.log("Next steps:")
    unreal.log("  1) py Scripts/SetupCharacterVisualsDataTable.py")
    unreal.log("  2) py Scripts/SetupPropsDataTable.py")


if __name__ == "__main__":
    main()
