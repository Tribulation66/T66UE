"""
Import ArcadeReplacementBatch01 Unreal-ready FBXs into canonical asset paths and
bind their exported Trellis base-color textures to the project unlit material.
"""

import os
import sys

import unreal

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
if SCRIPT_DIR not in sys.path:
    sys.path.insert(0, SCRIPT_DIR)

import ImportStaticMeshes


ImportStaticMeshes.IMPORTS = [
    {
        "source": "Interactables/ArcadeReplacementBatch01/UnrealReady/GamblerDemonStand_UnrealReady.fbx",
        "dest": "/Game/Characters/NPCs/Gambler/GamblerDemonStand",
        "name": "GamblerDemonStand",
    },
    {
        "source": "Interactables/ArcadeReplacementBatch01/UnrealReady/ArcadeMachine_UnrealReady.fbx",
        "dest": "/Game/World/Interactables/ArcadeMachine",
        "name": "ArcadeMachine",
    },
    {
        "source": "Interactables/ArcadeReplacementBatch01/UnrealReady/ArcadeAmplifierPickup_UnrealReady.fbx",
        "dest": "/Game/World/Interactables/ArcadeAmplifierPickup",
        "name": "ArcadeAmplifierPickup",
    },
    {
        "source": "Interactables/ArcadeReplacementBatch01/UnrealReady/ArcadeAmplifierPickup_Charged_UnrealReady.fbx",
        "dest": "/Game/World/Interactables/ArcadeAmplifierPickup",
        "name": "ArcadeAmplifierPickup_Charged",
    },
    {
        "source": "Interactables/ArcadeReplacementBatch01/UnrealReady/Chest_UnrealReady.fbx",
        "dest": "/Game/World/Interactables/Chests/ChestModel",
        "name": "Chest",
    },
]

TEXTURE_DIR = "Interactables/ArcadeReplacementBatch01/Textures"
PARENT_MATERIAL = "/Game/Materials/M_Environment_Unlit"


TEXTURE_BINDINGS = {
    "GamblerDemonStand": {
        "dest": "/Game/Characters/NPCs/Gambler/GamblerDemonStand",
        "texture": "GamblerDemonStand_BaseColor_00.png",
    },
    "ArcadeMachine": {
        "dest": "/Game/World/Interactables/ArcadeMachine",
        "texture": "ArcadeMachine_BaseColor_00.png",
    },
    "ArcadeAmplifierPickup": {
        "dest": "/Game/World/Interactables/ArcadeAmplifierPickup",
        "texture": "ArcadeAmplifierPickup_BaseColor_00.png",
    },
    "ArcadeAmplifierPickup_Charged": {
        "dest": "/Game/World/Interactables/ArcadeAmplifierPickup",
        "texture": "ArcadeAmplifierPickup_Charged_BaseColor_00.png",
    },
    "Chest": {
        "dest": "/Game/World/Interactables/Chests/ChestModel",
        "texture": "Chest_BaseColor_00.png",
    },
}


def _project_import_root():
    return os.path.join(
        unreal.SystemLibrary.get_project_directory(),
        "SourceAssets",
        "Import",
    ).replace("\\", "/")


def _ensure_game_dir(game_path):
    if not unreal.EditorAssetLibrary.does_directory_exist(game_path):
        unreal.EditorAssetLibrary.make_directory(game_path)


def _import_texture(source_path, dest_dir, texture_name):
    texture_dir = f"{dest_dir}/Textures"
    _ensure_game_dir(texture_dir)

    task = unreal.AssetImportTask()
    task.automated = True
    task.save = True
    task.replace_existing = True
    task.replace_existing_settings = True
    task.filename = source_path
    task.destination_path = texture_dir
    task.destination_name = texture_name

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

    imported_paths = list(task.imported_object_paths or [])
    texture_path = str(imported_paths[0]).split(".")[0] if imported_paths else f"{texture_dir}/{texture_name}"
    try:
        unreal.AssetRegistryHelpers.get_asset_registry().scan_paths_synchronous([texture_dir], True)
    except Exception:
        pass

    texture = unreal.EditorAssetLibrary.load_asset(texture_path)
    if not texture or not isinstance(texture, unreal.Texture2D):
        unreal.log_warning(f"[ImportArcadeReplacementBatch01] Texture import failed: {texture_path}")
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


def _ensure_material(asset_name, dest_dir, texture):
    material_dir = f"{dest_dir}/Materials"
    material_name = f"M_{asset_name}"
    material_path = f"{material_dir}/{material_name}"
    _ensure_game_dir(material_dir)

    material = unreal.EditorAssetLibrary.load_asset(material_path)
    if not material:
        material = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
            material_name,
            material_dir,
            unreal.MaterialInstanceConstant,
            unreal.MaterialInstanceConstantFactoryNew(),
        )
    if not material:
        unreal.log_warning(f"[ImportArcadeReplacementBatch01] Material create/load failed: {material_path}")
        return None

    parent = unreal.EditorAssetLibrary.load_asset(PARENT_MATERIAL)
    if parent:
        try:
            material.set_editor_property("parent", parent)
        except Exception as exc:
            unreal.log_warning(
                f"[ImportArcadeReplacementBatch01] Could not set parent for {material_name}: {exc}")

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


def _assign_mesh_material(asset_name, dest_dir, material):
    if not material:
        return False

    mesh_path = f"{dest_dir}/{asset_name}"
    mesh = unreal.EditorAssetLibrary.load_asset(mesh_path)
    if not mesh or not isinstance(mesh, unreal.StaticMesh):
        unreal.log_warning(f"[ImportArcadeReplacementBatch01] StaticMesh not found: {mesh_path}")
        return False

    slot_count = 1
    try:
        slot_count = max(1, len(mesh.get_editor_property("static_materials") or []))
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


def _apply_generated_textures():
    import_root = _project_import_root()
    applied = 0

    for asset_name, binding in TEXTURE_BINDINGS.items():
        source_path = os.path.join(
            import_root,
            TEXTURE_DIR,
            binding["texture"],
        ).replace("\\", "/")
        if not os.path.isfile(source_path):
            unreal.log_warning(f"[ImportArcadeReplacementBatch01] Missing texture source: {source_path}")
            continue

        dest_dir = binding["dest"]
        texture_name = os.path.splitext(binding["texture"])[0]
        texture = _import_texture(source_path, dest_dir, texture_name)
        material = _ensure_material(asset_name, dest_dir, texture)
        if _assign_mesh_material(asset_name, dest_dir, material):
            applied += 1

    unreal.log(
        f"[ImportArcadeReplacementBatch01] Applied generated materials: {applied}/{len(TEXTURE_BINDINGS)}")


def main():
    ImportStaticMeshes.main()
    _apply_generated_textures()

    world = None
    try:
        subsystem = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
        if subsystem:
            world = subsystem.get_editor_world()
    except Exception:
        pass

    try:
        unreal.SystemLibrary.execute_console_command(world, "QUIT_EDITOR")
    except Exception as exc:
        unreal.log_warning(f"Failed to request QUIT_EDITOR: {exc}")


main()
