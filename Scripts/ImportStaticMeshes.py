"""
Import static mesh GLBs into the project and immediately convert imported GLB
materials to the project's Unlit pipeline.

Update the IMPORTS list below before each run, then execute in Unreal Editor:
  Tools -> Execute Python Script -> Scripts/ImportStaticMeshes.py
  or from Output Log:
  py "C:/UE/T66/Scripts/ImportStaticMeshes.py"
"""

import os
import sys
import unreal

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
if SCRIPT_DIR not in sys.path:
    sys.path.append(SCRIPT_DIR)

import MakeGLBImportsUnlit

# ======================================================================
# IMPORT MANIFEST — edit this before each import run.
#
# Each entry: (source_glb_relative_to_SourceAssets/Import, destination_game_path, asset_name)
#
# source:      Relative path from SourceAssets/Import/ to the .glb file
# destination: /Game/... content path where the asset should live
# name:        Desired asset name in the content browser
# ======================================================================

IMPORTS = [
    # --- Generated dungeon kit ---
    (
        "WorldKit/DungeonKit01/DungeonWall_Straight_A_UnrealReady.fbx",
        "/Game/World/Terrain/TowerDungeon/GeneratedKit/DungeonKit01",
        "DungeonWall_Straight_A_UnrealReady",
    ),
    (
        "WorldKit/DungeonKit01/DungeonWall_Straight_Chains_UnrealReady.fbx",
        "/Game/World/Terrain/TowerDungeon/GeneratedKit/DungeonKit01",
        "DungeonWall_Straight_Chains_UnrealReady",
    ),
    (
        "WorldKit/DungeonKit01/DungeonWall_Straight_BonesNiche_UnrealReady.fbx",
        "/Game/World/Terrain/TowerDungeon/GeneratedKit/DungeonKit01",
        "DungeonWall_Straight_BonesNiche_UnrealReady",
    ),
    (
        "WorldKit/DungeonKit01/DungeonWall_Doorway_Arch_UnrealReady.fbx",
        "/Game/World/Terrain/TowerDungeon/GeneratedKit/DungeonKit01",
        "DungeonWall_Doorway_Arch_UnrealReady",
    ),
    (
        "WorldKit/DungeonKit01/DungeonFloor_BonesDrain_A_UnrealReady.fbx",
        "/Game/World/Terrain/TowerDungeon/GeneratedKit/DungeonKit01",
        "DungeonFloor_BonesDrain_A_UnrealReady",
    ),

    # --- Props ---
    ("Props/Barn.glb",     "/Game/World/Props",              "Barn"),
    ("Props/Boulder.glb",  "/Game/World/Props",              "Boulder"),
    ("Props/Branch.glb",   "/Game/World/Props",              "Branch"),
    ("Props/Bush.glb",     "/Game/World/Props",              "Bush"),
    ("Props/Fence.glb",    "/Game/World/Props",              "Fence"),
    ("Props/Fence2.glb",   "/Game/World/Props",              "Fence2"),
    ("Props/Fence3.glb",   "/Game/World/Props",              "Fence3"),
    ("Props/Haybell.glb",  "/Game/World/Props",              "Haybell"),
    ("Props/House.glb",    "/Game/World/Props",              "House"),
    ("Props/Log.glb",      "/Game/World/Props",              "Log"),
    ("Props/Mud.glb",      "/Game/World/Props",              "Mud"),
    ("Props/Rocks.glb",    "/Game/World/Props",              "Rocks"),
    ("Props/Scarecrow.glb","/Game/World/Props",              "Scarecrow"),
    ("Props/Silo.glb",     "/Game/World/Props",              "Silo"),
    ("Props/Stump.glb",    "/Game/World/Props",              "Stump"),
    ("Props/Tractor.glb",  "/Game/World/Props",              "Tractor"),
    ("Props/Tree.glb",     "/Game/World/Props",              "Tree"),
    (
        "Props/Tree2.glb",
        "/Game/World/Props",
        "Tree2",
        {},
        {
            "BaseColorTexture": {
                "mip_gen_settings": unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS,
                "never_stream": True,
            },
        },
    ),
    ("Props/Tree3.glb",    "/Game/World/Props",              "Tree3"),
    ("Props/Troth.glb",    "/Game/World/Props",              "Troth"),
    ("Props/Windmill.glb", "/Game/World/Props",              "Windmill"),
    ("Cow.glb",            "/Game/World/Props/StartAreaDecor", "Cow"),
    ("FullBody.glb",       "/Game/World/Props/StartAreaDecor", "FullBody"),
    ("RoboCow.glb",        "/Game/World/Props/StartAreaDecor", "RoboCow"),
    ("Vending.glb",        "/Game/World/Interactables/Vending", "Vending"),
    ("Casino.glb",         "/Game/World/Interactables/Casino",  "Casino"),
    ("TeleportPad.glb",    "/Game/World/Interactables/TeleportPad", "TeleportPad"),
]

DEFAULT_STATIC_MESH_BUILD_SETTINGS = {
    "use_full_precision_u_vs": True,
    "use_backwards_compatible_f16_trunc_u_vs": False,
    "generate_lightmap_u_vs": False,
    "recompute_normals": False,
    "recompute_tangents": False,
}

DEFAULT_GLB_MATERIAL_OVERRIDES = {
    "brightness": 1.0,
    "tint": (1.0, 1.0, 1.0, 1.0),
}

DEFAULT_IMPORT_CLEANUP = {
    "mode": "asset_subtree",
}


def _get_project_dir():
    try:
        return unreal.SystemLibrary.get_project_directory().replace("\\", "/").rstrip("/")
    except Exception:
        return os.path.abspath(os.path.join(os.getcwd(), "..")).replace("\\", "/")


def _ensure_game_dir(game_path):
    if not unreal.EditorAssetLibrary.does_directory_exist(game_path):
        unreal.EditorAssetLibrary.make_directory(game_path)


def _delete_asset_if_exists(asset_path):
    if not unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        return False
    return bool(unreal.EditorAssetLibrary.delete_asset(asset_path))


def _delete_directory_if_exists(directory_path):
    if not unreal.EditorAssetLibrary.does_directory_exist(directory_path):
        return False
    return bool(unreal.EditorAssetLibrary.delete_directory(directory_path))


def _cleanup_existing_import_artifacts(dest_dir, dest_name, cleanup_overrides):
    cleanup = dict(DEFAULT_IMPORT_CLEANUP)
    cleanup.update(dict(cleanup_overrides or {}))

    mode = str(cleanup.get("mode") or "asset_subtree")
    expected_path = f"{dest_dir}/{dest_name}"

    if mode == "none":
        unreal.log(f"    [CLEAN] Skipped cleanup for {dest_name}")
        return True

    if mode == "dest_dir":
        deleted_dir = _delete_directory_if_exists(dest_dir)
        _ensure_game_dir(dest_dir)
        unreal.log(
            f"    [CLEAN] {dest_name}: mode=dest_dir deleted_dir={deleted_dir}")
        return True

    deleted_asset = _delete_asset_if_exists(expected_path)
    deleted_dir = _delete_directory_if_exists(expected_path)
    unreal.log(
        f"    [CLEAN] {dest_name}: mode=asset_subtree "
        f"deleted_asset={deleted_asset} deleted_dir={deleted_dir}")
    return True


def _normalize_import_entry(entry):
    if isinstance(entry, dict):
        return (
            entry["source"],
            entry["dest"],
            entry["name"],
            dict(entry.get("material_overrides") or {}),
            dict(entry.get("texture_parameter_overrides") or {}),
            dict(entry.get("mesh_build_settings_overrides") or {}),
            dict(entry.get("cleanup") or {}),
        )

    if len(entry) >= 7:
        (
            source,
            dest_dir,
            dest_name,
            material_overrides,
            texture_parameter_overrides,
            mesh_build_settings_overrides,
            cleanup_overrides,
        ) = entry[:7]
        return (
            source,
            dest_dir,
            dest_name,
            dict(material_overrides or {}),
            dict(texture_parameter_overrides or {}),
            dict(mesh_build_settings_overrides or {}),
            dict(cleanup_overrides or {}),
        )

    if len(entry) >= 6:
        (
            source,
            dest_dir,
            dest_name,
            material_overrides,
            texture_parameter_overrides,
            mesh_build_settings_overrides,
        ) = entry[:6]
        return (
            source,
            dest_dir,
            dest_name,
            dict(material_overrides or {}),
            dict(texture_parameter_overrides or {}),
            dict(mesh_build_settings_overrides or {}),
            {},
        )

    if len(entry) == 5:
        source, dest_dir, dest_name, material_overrides, texture_parameter_overrides = entry[:5]
        return (
            source,
            dest_dir,
            dest_name,
            dict(material_overrides or {}),
            dict(texture_parameter_overrides or {}),
            {},
            {},
        )

    if len(entry) == 4:
        source, dest_dir, dest_name, material_overrides = entry
        return source, dest_dir, dest_name, dict(material_overrides or {}), {}, {}, {}

    source, dest_dir, dest_name = entry
    return source, dest_dir, dest_name, {}, {}, {}, {}


def import_glb(source_path, dest_dir, dest_name):
    """Static mesh import via Interchange/FBX pipelines."""
    _ensure_game_dir(dest_dir)

    task = unreal.AssetImportTask()
    task.automated = True
    task.save = True
    task.replace_existing = True
    task.replace_existing_settings = True
    task.filename = source_path
    task.destination_path = dest_dir
    task.destination_name = dest_name

    if os.path.splitext(source_path)[1].lower() == ".fbx":
        options = unreal.FbxImportUI()
        options.set_editor_property("automated_import_should_detect_type", False)
        options.set_editor_property("mesh_type_to_import", unreal.FBXImportType.FBXIT_STATIC_MESH)
        options.set_editor_property("import_mesh", True)
        options.set_editor_property("import_as_skeletal", False)
        options.set_editor_property("import_materials", False)
        options.set_editor_property("import_textures", False)
        options.set_editor_property("import_animations", False)

        static_mesh_data = options.get_editor_property("static_mesh_import_data")
        if static_mesh_data:
            try:
                static_mesh_data.set_editor_property("combine_meshes", True)
            except Exception:
                pass
            try:
                static_mesh_data.set_editor_property("auto_generate_collision", False)
            except Exception:
                pass
            try:
                static_mesh_data.set_editor_property("generate_lightmap_u_vs", False)
            except Exception:
                pass

        task.options = options

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

    imported = list(task.imported_object_paths or [])
    return imported


def _find_static_mesh_in_dir(game_dir):
    """Search a directory (recursively) for the first StaticMesh asset."""
    try:
        assets = unreal.EditorAssetLibrary.list_assets(game_dir, recursive=True, include_folder=False)
    except Exception:
        return None
    for asset_path in assets:
        asset = unreal.EditorAssetLibrary.load_asset(asset_path)
        if asset and isinstance(asset, unreal.StaticMesh):
            return asset_path
    return None


def _flatten_interchange_asset(dest_dir, dest_name):
    """
    Interchange creates: dest_dir/dest_name/StaticMeshes/dest_name
    We want:             dest_dir/dest_name

    Find the StaticMesh wherever Interchange put it and move it to the expected flat path.
    """
    expected_path = f"{dest_dir}/{dest_name}"

    existing = unreal.EditorAssetLibrary.load_asset(expected_path)
    if existing and isinstance(existing, unreal.StaticMesh):
        return expected_path

    interchange_dir = f"{dest_dir}/{dest_name}"
    mesh_path = _find_static_mesh_in_dir(interchange_dir)

    if not mesh_path:
        interchange_dir2 = dest_dir
        mesh_path = _find_static_mesh_in_dir(interchange_dir2)

    if not mesh_path:
        return None

    clean_path = str(mesh_path).split(".")[0]
    if clean_path == expected_path:
        return expected_path

    if unreal.EditorAssetLibrary.does_asset_exist(expected_path):
        unreal.EditorAssetLibrary.delete_asset(expected_path)

    success = unreal.EditorAssetLibrary.rename_asset(clean_path, expected_path)
    if success:
        unreal.EditorAssetLibrary.save_asset(expected_path)
        unreal.log(f"    [RENAME] {clean_path} -> {expected_path}")

        _cleanup_empty_interchange_dirs(dest_dir, dest_name)
        return expected_path
    else:
        unreal.log_warning(f"    [RENAME FAILED] {clean_path} -> {expected_path}")
        return clean_path


def _cleanup_empty_interchange_dirs(dest_dir, dest_name):
    """Remove the empty Interchange subdirectories after flattening."""
    for sub in ["StaticMeshes", "Materials", "Textures", "AnimationSequences"]:
        sub_dir = f"{dest_dir}/{dest_name}/{sub}"
        try:
            if unreal.EditorAssetLibrary.does_directory_exist(sub_dir):
                assets = unreal.EditorAssetLibrary.list_assets(sub_dir, recursive=True, include_folder=False)
                if not assets or len(assets) == 0:
                    unreal.EditorAssetLibrary.delete_directory(sub_dir)
        except Exception:
            pass


def _existing_scan_roots(dest_dir, dest_name):
    roots = []
    materials_root = f"{dest_dir}/{dest_name}/Materials"
    if unreal.EditorAssetLibrary.does_directory_exist(materials_root):
        roots.append(materials_root)
    if unreal.EditorAssetLibrary.does_directory_exist(dest_dir):
        roots.append(dest_dir)
    return roots


def _find_candidate_materials(dest_dir, dest_name):
    candidates = []
    seen = set()

    materials_root = f"{dest_dir}/{dest_name}/Materials"
    roots = []
    if unreal.EditorAssetLibrary.does_directory_exist(materials_root):
        roots.append((materials_root, True))
    if unreal.EditorAssetLibrary.does_directory_exist(dest_dir):
        roots.append((dest_dir, True))

    for root, recursive in roots:
        try:
            asset_paths = unreal.EditorAssetLibrary.list_assets(
                root, recursive=recursive, include_folder=False)
        except Exception:
            continue

        for asset_path in asset_paths:
            asset = unreal.EditorAssetLibrary.load_asset(asset_path)
            if not asset or not isinstance(asset, unreal.MaterialInterface):
                continue
            if not hasattr(asset, "get_editor_property"):
                continue
            try:
                asset.get_editor_property("parent")
            except Exception:
                continue
            asset_key = asset.get_path_name()
            if asset_key in seen:
                continue
            seen.add(asset_key)
            candidates.append(asset)

        if candidates:
            break

    candidates.sort(key=lambda asset: asset.get_name())
    return candidates


def _bind_materials_to_flattened_mesh(mesh_path, dest_dir, dest_name):
    mesh = unreal.EditorAssetLibrary.load_asset(mesh_path)
    if not mesh or not isinstance(mesh, unreal.StaticMesh):
        unreal.log_warning(f"    [BIND] StaticMesh not found at {mesh_path}")
        return False

    materials = _find_candidate_materials(dest_dir, dest_name)
    if not materials:
        unreal.log_warning(f"    [BIND] No candidate material instances found for {dest_name}")
        return False

    slots = list(mesh.get_editor_property("static_materials") or [])
    if not slots:
        unreal.log_warning(f"    [BIND] No static material slots found on {mesh_path}")
        return False

    mesh.modify()

    for index, slot in enumerate(slots):
        material = materials[min(index, len(materials) - 1)]
        try:
            slot.set_editor_property("material_interface", material)
        except Exception:
            try:
                mesh.set_material(index, material)
            except Exception as exc:
                unreal.log_warning(f"    [BIND] Failed slot {index} on {mesh_path}: {exc}")
                return False

    try:
        mesh.set_editor_property("static_materials", slots)
    except Exception:
        pass

    try:
        mesh.post_edit_change()
    except Exception:
        pass

    try:
        unreal.EditorAssetLibrary.save_asset(mesh_path)
    except Exception:
        pass

    unreal.log(
        f"    [BIND] Assigned {len(materials)} material candidate(s) to {mesh_path}")
    return True


def _apply_material_overrides(dest_dir, dest_name, material_overrides):
    materials = _find_candidate_materials(dest_dir, dest_name)
    if not materials:
        unreal.log_warning(f"    [OVERRIDE] No candidate materials found for {dest_name}")
        return False

    resolved_overrides = dict(DEFAULT_GLB_MATERIAL_OVERRIDES)
    resolved_overrides.update(dict(material_overrides or {}))

    tint_value = resolved_overrides.get("tint")
    tint = None
    if isinstance(tint_value, (tuple, list)) and len(tint_value) >= 4:
        tint = unreal.LinearColor(
            float(tint_value[0]),
            float(tint_value[1]),
            float(tint_value[2]),
            float(tint_value[3]),
        )

    for material in materials:
        try:
            if "brightness" in resolved_overrides:
                unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(
                    material, "Brightness", float(resolved_overrides["brightness"]))
            if tint is not None:
                unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(
                    material, "Tint", tint)
            unreal.EditorAssetLibrary.save_loaded_asset(material)
        except Exception as exc:
            unreal.log_warning(f"    [OVERRIDE] Failed on {material.get_name()}: {exc}")
            return False

    unreal.log(f"    [OVERRIDE] Applied material overrides to {dest_name}: {resolved_overrides}")
    return True


def _apply_texture_parameter_overrides(dest_dir, dest_name, texture_parameter_overrides):
    if not texture_parameter_overrides:
        return False

    materials = _find_candidate_materials(dest_dir, dest_name)
    if not materials:
        unreal.log_warning(f"    [TEX OVERRIDE] No candidate materials found for {dest_name}")
        return False

    touched_paths = set()
    applied = False

    for material in materials:
        for parameter_name, overrides in texture_parameter_overrides.items():
            try:
                texture = unreal.MaterialEditingLibrary.get_material_instance_texture_parameter_value(
                    material, parameter_name)
                if isinstance(texture, (tuple, list)):
                    texture = texture[-1] if texture else None
            except Exception:
                texture = None

            if not texture:
                continue

            texture_path = texture.get_path_name()
            if texture_path in touched_paths:
                continue

            changed = False
            for prop_name, value in dict(overrides or {}).items():
                try:
                    texture.set_editor_property(prop_name, value)
                    changed = True
                except Exception as exc:
                    unreal.log_warning(
                        f"    [TEX OVERRIDE] Failed {texture.get_name()}.{prop_name} on {dest_name}: {exc}")

            if not changed:
                continue

            touched_paths.add(texture_path)
            applied = True

            try:
                texture.post_edit_change()
            except Exception:
                pass

            try:
                unreal.EditorAssetLibrary.save_asset(texture_path)
            except Exception:
                pass

    if applied:
        unreal.log(
            f"    [TEX OVERRIDE] Applied texture parameter overrides to {dest_name}: "
            f"{list(texture_parameter_overrides.keys())}")
    else:
        unreal.log_warning(f"    [TEX OVERRIDE] No matching textures found for {dest_name}")
    return applied


def _get_lod_build_settings_accessors():
    try:
        subsystem = unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)
    except Exception:
        subsystem = None

    if subsystem:
        return subsystem.get_lod_build_settings, subsystem.set_lod_build_settings

    return None, None


def _apply_static_mesh_build_settings(mesh_path, mesh_build_settings_overrides):
    overrides = dict(DEFAULT_STATIC_MESH_BUILD_SETTINGS)
    overrides.update(dict(mesh_build_settings_overrides or {}))
    if not overrides:
        return False

    mesh = unreal.EditorAssetLibrary.load_asset(mesh_path)
    if not mesh or not isinstance(mesh, unreal.StaticMesh):
        unreal.log_warning(f"    [BUILD] StaticMesh not found at {mesh_path}")
        return False

    try:
        lod_count = int(mesh.get_num_lods())
    except Exception:
        try:
            lod_count = int(unreal.EditorStaticMeshLibrary.get_lod_count(mesh))
        except Exception:
            lod_count = 1

    if lod_count <= 0:
        lod_count = 1

    get_build_settings, set_build_settings = _get_lod_build_settings_accessors()
    if not get_build_settings or not set_build_settings:
        unreal.log_warning(
            f"    [BUILD] StaticMeshEditorSubsystem unavailable; run this script in UnrealEditor.exe, "
            f"not UnrealEditor-Cmd.exe, to apply mesh build settings on {mesh_path}")
        return False
    applied = False

    for lod_index in range(lod_count):
        try:
            build_settings = get_build_settings(mesh, lod_index)
        except Exception as exc:
            unreal.log_warning(f"    [BUILD] Failed to read LOD {lod_index} settings on {mesh_path}: {exc}")
            return False

        changed = False
        for prop_name, value in overrides.items():
            try:
                current = build_settings.get_editor_property(prop_name)
                if current != value:
                    build_settings.set_editor_property(prop_name, value)
                    changed = True
            except Exception as exc:
                unreal.log_warning(
                    f"    [BUILD] Failed to set {prop_name} on {mesh_path} LOD {lod_index}: {exc}")
                return False

        if not changed:
            continue

        try:
            set_build_settings(mesh, lod_index, build_settings)
            applied = True
        except Exception as exc:
            unreal.log_warning(f"    [BUILD] Failed to write LOD {lod_index} settings on {mesh_path}: {exc}")
            return False

    if applied:
        try:
            mesh.modify()
        except Exception:
            pass
        try:
            unreal.EditorAssetLibrary.save_loaded_asset(mesh)
        except Exception:
            pass
        unreal.log(f"    [BUILD] Applied mesh build settings to {mesh_path}: {overrides}")
    else:
        unreal.log(f"    [BUILD] Mesh build settings already correct on {mesh_path}")

    return True


def main():
    proj = _get_project_dir()
    import_root = os.path.join(proj, "SourceAssets", "Import").replace("\\", "/")

    unreal.log("=" * 60)
    unreal.log("[ImportStaticMeshes] START")
    unreal.log(f"[ImportStaticMeshes] Source root: {import_root}")
    unreal.log(f"[ImportStaticMeshes] {len(IMPORTS)} entries in manifest")
    unreal.log("=" * 60)

    success_count = 0
    fail_count = 0
    skip_count = 0

    for entry in IMPORTS:
        (
            relative_path,
            dest_dir,
            dest_name,
            material_overrides,
            texture_parameter_overrides,
            mesh_build_settings_overrides,
            cleanup_overrides,
        ) = _normalize_import_entry(entry)
        source = os.path.join(import_root, relative_path).replace("\\", "/")
        source_ext = os.path.splitext(source)[1].lower()

        if not os.path.isfile(source):
            unreal.log_warning(f"  [SKIP] {relative_path} — file not found at {source}")
            skip_count += 1
            continue

        try:
            _cleanup_existing_import_artifacts(dest_dir, dest_name, cleanup_overrides)
            imported = import_glb(source, dest_dir, dest_name)
            asset_count = len(imported)
            unreal.log(f"  [OK] {dest_name} -> {dest_dir} ({asset_count} assets)")
            for p in imported:
                unreal.log(f"        {p}")

            final_path = _flatten_interchange_asset(dest_dir, dest_name)
            if final_path:
                unreal.log(f"    [FINAL] {final_path}")
            else:
                unreal.log_warning(f"    [WARN] Could not locate StaticMesh for {dest_name}")

            if source_ext == ".fbx":
                unreal.log("    [UNLIT] skipped for FBX import; preserving imported material slots")
            else:
                scan_roots = _existing_scan_roots(dest_dir, dest_name)
                unlit_results = MakeGLBImportsUnlit.convert_glb_imports_unlit(scan_roots)
                unreal.log(
                    "    [UNLIT] converted={converted} already_ok={already_ok} "
                    "skipped={skipped} no_texture={no_texture} errors={errors}".format(**unlit_results)
                )

            if final_path:
                _apply_static_mesh_build_settings(final_path, mesh_build_settings_overrides)
                if source_ext != ".fbx":
                    _bind_materials_to_flattened_mesh(final_path, dest_dir, dest_name)
                    _apply_material_overrides(dest_dir, dest_name, material_overrides)
                    _apply_texture_parameter_overrides(
                        dest_dir, dest_name, texture_parameter_overrides)

            success_count += 1
        except Exception as e:
            unreal.log_error(f"  [FAIL] {dest_name} — {e}")
            fail_count += 1

    unreal.log("")
    unreal.log("=" * 60)
    unreal.log(f"[ImportStaticMeshes] DONE — success={success_count}, skipped={skip_count}, failed={fail_count}")
    unreal.log("=" * 60)


if __name__ == "__main__":
    main()
