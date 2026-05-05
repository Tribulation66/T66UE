"""
Import the CoherentThemeKit01 generated tower environment modules, then request
editor shutdown.
"""

import os
import sys
import unreal


SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
if SCRIPT_DIR not in sys.path:
    sys.path.append(SCRIPT_DIR)

import ImportStaticMeshes


DEST_DIR = ImportStaticMeshes.COHERENT_THEME_KIT_DEST
SOURCE_PREFIX = "WorldKit/CoherentThemeKit01/"
KIT_MESHES = tuple(f"{module_id}_UnrealReady" for module_id in ImportStaticMeshes.COHERENT_THEME_KIT_MODULES)
SOURCE_TEXTURE_DIR = "WorldKit/CoherentThemeKit01/Textures"
PARENT_MATERIAL = "/Game/Materials/M_Environment_Unlit"
KIT_TEXTURES = {
    asset_name: f"{asset_name}_BaseColor_00.png"
    for asset_name in KIT_MESHES
}


def _project_import_root():
    return os.path.join(
        unreal.SystemLibrary.get_project_directory(),
        "SourceAssets",
        "Import",
    ).replace("\\", "/")


def _import_texture(source_path, texture_name):
    task = unreal.AssetImportTask()
    task.automated = True
    task.save = False
    task.replace_existing = True
    task.replace_existing_settings = True
    task.filename = source_path
    task.destination_path = DEST_DIR
    task.destination_name = texture_name

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

    texture_path = f"{DEST_DIR}/{texture_name}"
    texture = unreal.EditorAssetLibrary.load_asset(texture_path)
    if not texture or not isinstance(texture, unreal.Texture2D):
        unreal.log_warning(f"[RunImportCoherentThemeKit01AndExit] Texture import failed: {texture_path}")
        return None

    try:
        texture.set_editor_property("srgb", True)
    except Exception:
        pass
    try:
        texture.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_World)
    except Exception:
        pass
    unreal.EditorAssetLibrary.save_asset(texture_path)
    return texture


def _ensure_material(asset_name, texture):
    material_name = f"{asset_name}_Mat_00"
    material_path = f"{DEST_DIR}/{material_name}"
    material = unreal.EditorAssetLibrary.load_asset(material_path)
    if not material:
        material = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
            material_name,
            DEST_DIR,
            unreal.MaterialInstanceConstant,
            unreal.MaterialInstanceConstantFactoryNew(),
        )
    if not material:
        unreal.log_warning(f"[RunImportCoherentThemeKit01AndExit] Material create/load failed: {material_path}")
        return None

    parent = unreal.EditorAssetLibrary.load_asset(PARENT_MATERIAL)
    if parent:
        try:
            material.set_editor_property("parent", parent)
        except Exception as exc:
            unreal.log_warning(
                f"[RunImportCoherentThemeKit01AndExit] Could not set parent for {material_name}: {exc}")

    if texture:
        for param_name in ("DiffuseColorMap", "BaseColorTexture"):
            try:
                unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(
                    material, param_name, texture)
            except Exception:
                pass
    try:
        unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(
            material, "Tint", unreal.LinearColor(1.0, 1.0, 1.0, 1.0))
    except Exception:
        pass
    try:
        unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(
            material, "Brightness", 1.0)
    except Exception:
        pass
    unreal.EditorAssetLibrary.save_asset(material_path)
    return material


def _assign_mesh_material(asset_name, material):
    if not material:
        return False

    mesh_path = f"{DEST_DIR}/{asset_name}"
    mesh = unreal.EditorAssetLibrary.load_asset(mesh_path)
    if not mesh or not isinstance(mesh, unreal.StaticMesh):
        unreal.log_warning(f"[RunImportCoherentThemeKit01AndExit] StaticMesh not found for material bind: {mesh_path}")
        return False

    slot_count = 1
    try:
        slots = mesh.get_editor_property("static_materials") or []
        slot_count = max(1, len(slots))
    except Exception:
        pass

    for slot_index in range(slot_count):
        mesh.set_material(slot_index, material)
    try:
        mesh.post_edit_change()
    except Exception:
        pass
    unreal.EditorAssetLibrary.save_asset(mesh_path)
    return True


def _apply_kit_textures():
    import_root = _project_import_root()
    applied = 0
    for asset_name, texture_file in KIT_TEXTURES.items():
        texture_source = os.path.join(import_root, SOURCE_TEXTURE_DIR, texture_file).replace("\\", "/")
        if not os.path.isfile(texture_source):
            unreal.log_warning(f"[RunImportCoherentThemeKit01AndExit] Missing texture source: {texture_source}")
            continue

        texture_name = os.path.splitext(texture_file)[0]
        texture = _import_texture(texture_source, texture_name)
        material = _ensure_material(asset_name, texture)
        if _assign_mesh_material(asset_name, material):
            applied += 1

    unreal.log(
        f"[RunImportCoherentThemeKit01AndExit] Applied textured materials: {applied}/{len(KIT_TEXTURES)}")


def _default_collision_trace_flag():
    try:
        return unreal.CollisionTraceFlag.CTF_USE_DEFAULT
    except Exception:
        return None


def _reset_kit_collision_trace_flags():
    collision_flag = _default_collision_trace_flag()
    applied = 0
    for asset_name in KIT_MESHES:
        mesh_path = f"{DEST_DIR}/{asset_name}"
        mesh = unreal.EditorAssetLibrary.load_asset(mesh_path)
        if not mesh or not isinstance(mesh, unreal.StaticMesh):
            unreal.log_warning(f"[RunImportCoherentThemeKit01AndExit] StaticMesh not found: {mesh_path}")
            continue

        body_setup = None
        try:
            body_setup = mesh.get_editor_property("body_setup")
        except Exception as exc:
            unreal.log_warning(
                f"[RunImportCoherentThemeKit01AndExit] Could not read body_setup for {asset_name}: {exc}")

        if body_setup and collision_flag is not None:
            try:
                body_setup.set_editor_property("collision_trace_flag", collision_flag)
            except Exception as exc:
                unreal.log_warning(
                    f"[RunImportCoherentThemeKit01AndExit] Could not reset collision trace flag on {asset_name}: {exc}")
                continue

        try:
            mesh.post_edit_change()
        except Exception:
            pass
        unreal.EditorAssetLibrary.save_asset(mesh_path)
        applied += 1

    unreal.log(
        f"[RunImportCoherentThemeKit01AndExit] Reset collision trace flags to default: {applied}/{len(KIT_MESHES)}")


def main():
    kit_imports = []
    for entry in ImportStaticMeshes.IMPORTS:
        relative_path, *_ = ImportStaticMeshes._normalize_import_entry(entry)
        if str(relative_path).startswith(SOURCE_PREFIX):
            kit_imports.append(entry)

    unreal.log(
        f"[RunImportCoherentThemeKit01AndExit] Running ImportStaticMeshes for {len(kit_imports)} CoherentThemeKit01 entries")

    original_imports = list(ImportStaticMeshes.IMPORTS)
    try:
        if unreal.EditorAssetLibrary.does_directory_exist(DEST_DIR):
            unreal.EditorAssetLibrary.delete_directory(DEST_DIR)
        unreal.EditorAssetLibrary.make_directory(DEST_DIR)
        ImportStaticMeshes.IMPORTS = kit_imports
        ImportStaticMeshes.main()
        _apply_kit_textures()
        _reset_kit_collision_trace_flags()
    finally:
        ImportStaticMeshes.IMPORTS = original_imports

    world = None
    try:
        subsystem = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
        if subsystem:
            world = subsystem.get_editor_world()
    except Exception:
        pass

    try:
        unreal.SystemLibrary.execute_console_command(world, "QUIT_EDITOR")
        unreal.log("[RunImportCoherentThemeKit01AndExit] QUIT_EDITOR requested")
    except Exception as exc:
        unreal.log_warning(f"[RunImportCoherentThemeKit01AndExit] Failed to request QUIT_EDITOR: {exc}")


if __name__ == "__main__":
    main()
