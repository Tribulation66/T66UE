"""
Import Megabonk terrain meshes and textures into /Game/World/Terrain/Megabonk.

Run inside Unreal Editor or via UnrealEditor-Cmd:
  py "C:/UE/T66/Scripts/ImportMegabonkTerrain.py"
"""

import os
import unreal


LOG = "[ImportMegabonkTerrain]"
DEST_DIR = "/Game/World/Terrain/Megabonk"
PARENT_MATERIAL = "/Game/Materials/M_Environment_Unlit"

MESH_IMPORTS = [
    ("Models/Block.fbx", "SM_MegabonkBlock", "MI_MegabonkBlock"),
    ("Models/Slope.fbx", "SM_MegabonkSlope", "MI_MegabonkSlope"),
]

TEXTURE_IMPORTS = [
    ("Materials/Textures/Texture_Block.png", "T_MegabonkBlock", "MI_MegabonkBlock"),
    ("Materials/Textures/Texture_Slope.png", "T_MegabonkSlope", "MI_MegabonkSlope"),
    ("Materials/Textures/Texture_Dirt.png", "T_MegabonkDirt", "MI_MegabonkDirt"),
]


def log(msg):
    unreal.log(f"{LOG} {msg}")


def fail(msg):
    unreal.log_error(f"{LOG} {msg}")


def get_project_dir():
    try:
        return unreal.SystemLibrary.get_project_directory().replace("\\", "/").rstrip("/")
    except Exception:
        return os.path.abspath(os.path.join(os.getcwd(), "..")).replace("\\", "/")


def ensure_directory(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def import_texture(source_path, dest_name):
    task = unreal.AssetImportTask()
    task.automated = True
    task.save = False
    task.replace_existing = True
    task.replace_existing_settings = True
    task.filename = source_path
    task.destination_path = DEST_DIR
    task.destination_name = dest_name

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    asset_path = f"{DEST_DIR}/{dest_name}"
    asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    if not asset or not isinstance(asset, unreal.Texture2D):
        raise RuntimeError(f"Texture import failed: {asset_path}")

    asset.set_editor_property("srgb", True)
    try:
        asset.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_World)
    except Exception:
        pass
    unreal.EditorAssetLibrary.save_asset(asset_path)
    return asset


def ensure_material_instance(parent, asset_name, texture):
    asset_path = f"{DEST_DIR}/{asset_name}"
    asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    if not asset:
        asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
            asset_name,
            DEST_DIR,
            unreal.MaterialInstanceConstant,
            unreal.MaterialInstanceConstantFactoryNew(),
        )
    if not asset:
        raise RuntimeError(f"Failed to create material instance: {asset_path}")

    asset.set_editor_property("parent", parent)
    unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(asset, "DiffuseColorMap", texture)
    unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(asset, "BaseColorTexture", texture)
    unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(
        asset, "Tint", unreal.LinearColor(1.0, 1.0, 1.0, 1.0)
    )
    unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(asset, "Brightness", 1.0)
    unreal.EditorAssetLibrary.save_asset(asset_path)
    return asset


def import_static_mesh(source_path, dest_name):
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
            static_mesh_data.set_editor_property("combine_meshes", False)
        except Exception:
            pass
        try:
            static_mesh_data.set_editor_property("generate_lightmap_u_vs", False)
        except Exception:
            pass
        try:
            static_mesh_data.set_editor_property("auto_generate_collision", True)
        except Exception:
            pass

    task = unreal.AssetImportTask()
    task.automated = True
    task.save = False
    task.replace_existing = True
    task.replace_existing_settings = True
    task.filename = source_path
    task.destination_path = DEST_DIR
    task.destination_name = dest_name
    task.options = options

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    asset_path = f"{DEST_DIR}/{dest_name}"
    mesh = unreal.EditorAssetLibrary.load_asset(asset_path)
    if not mesh or not isinstance(mesh, unreal.StaticMesh):
        raise RuntimeError(f"Static mesh import failed: {asset_path}")

    unreal.EditorAssetLibrary.save_asset(asset_path)
    return mesh


def assign_material(mesh, material):
    slots = list(mesh.get_editor_property("static_materials") or [])
    if not slots:
        mesh.set_material(0, material)
    else:
        for index, slot in enumerate(slots):
            try:
                slot.set_editor_property("material_interface", material)
            except Exception:
                mesh.set_material(index, material)
        try:
            mesh.set_editor_property("static_materials", slots)
        except Exception:
            pass

    try:
        mesh.post_edit_change()
    except Exception:
        pass
    unreal.EditorAssetLibrary.save_asset(mesh.get_path_name().split(".")[0])


def main():
    ensure_directory(DEST_DIR)

    parent = unreal.EditorAssetLibrary.load_asset(PARENT_MATERIAL)
    if not parent:
        fail(f"Missing master material: {PARENT_MATERIAL}")
        return

    project_dir = get_project_dir()
    source_root = os.path.join(
        project_dir,
        "SourceAssets",
        "MegabonkMapGeneration",
        "MegabonkTerrainGenerator_UnityProject",
        "Assets",
        "MegabonkTerrain",
    ).replace("\\", "/")

    log("START")
    materials = {}
    for texture_rel, texture_name, material_name in TEXTURE_IMPORTS:
        texture_source = os.path.join(source_root, texture_rel).replace("\\", "/")
        if not os.path.isfile(texture_source):
            raise RuntimeError(f"Missing texture source: {texture_source}")

        texture = import_texture(texture_source, texture_name)
        material = ensure_material_instance(parent, material_name, texture)
        materials[material_name] = material
        log(f"Ready material: {material.get_path_name()}")

    for mesh_rel, mesh_name, material_name in MESH_IMPORTS:
        mesh_source = os.path.join(source_root, mesh_rel).replace("\\", "/")
        if not os.path.isfile(mesh_source):
            raise RuntimeError(f"Missing mesh source: {mesh_source}")

        mesh = import_static_mesh(mesh_source, mesh_name)
        assign_material(mesh, materials[material_name])
        log(f"Ready mesh: {mesh.get_path_name()}")

    log("DONE")


if __name__ == "__main__":
    main()
