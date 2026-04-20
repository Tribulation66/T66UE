"""
Import tower dungeon textures and create runtime material instances for the Tower preset.

Run in Unreal Editor:
  Tools -> Execute Python Script -> Scripts/SetupTowerDungeonMaterials.py

Or headless:
  UnrealEditor-Cmd.exe T66.uproject -run=pythonscript -script="C:/UE/T66/Scripts/SetupTowerDungeonMaterials.py"
"""

import os
import unreal


LOG = "[TowerDungeonSetup]"
MASTER_PATH = "/Game/Materials/M_Environment_Unlit"
DEST_DIR = "/Game/World/Terrain/TowerDungeon"

IMPORTS = [
    {
        "source": ("output", "imagegen", "tower_dungeon_textures", "tower_dungeon_ground.png"),
        "texture_name": "T_TowerDungeonGround",
        "material_name": "MI_TowerDungeonGround",
    },
    {
        "source": ("output", "imagegen", "tower_dungeon_textures", "tower_dungeon_wall.png"),
        "texture_name": "T_TowerDungeonWall",
        "material_name": "MI_TowerDungeonWall",
    },
    {
        "source": ("output", "imagegen", "tower_dungeon_textures", "tower_dungeon_roof.png"),
        "texture_name": "T_TowerDungeonRoof",
        "material_name": "MI_TowerDungeonRoof",
    },
]


def log(message):
    unreal.log(f"{LOG} {message}")


def fail(message):
    unreal.log_error(f"{LOG} {message}")


def ensure_directory(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def get_project_dir():
    return unreal.SystemLibrary.get_project_directory().replace("\\", "/").rstrip("/")


def import_texture(source_path, texture_name):
    ensure_directory(DEST_DIR)

    task = unreal.AssetImportTask()
    task.automated = True
    task.save = False
    task.replace_existing = True
    task.replace_existing_settings = True
    task.filename = source_path
    task.destination_path = DEST_DIR
    task.destination_name = texture_name

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    asset_path = f"{DEST_DIR}/{texture_name}"
    texture = unreal.EditorAssetLibrary.load_asset(asset_path)
    if not texture or not isinstance(texture, unreal.Texture2D):
        raise RuntimeError(f"Texture import failed: {asset_path}")

    texture.set_editor_property("srgb", True)
    try:
        texture.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_World)
    except Exception:
        pass
    unreal.EditorAssetLibrary.save_asset(asset_path)
    return texture


def ensure_material_instance(parent, material_name, texture):
    asset_path = f"{DEST_DIR}/{material_name}"
    material = unreal.EditorAssetLibrary.load_asset(asset_path)
    if not material:
        material = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
            material_name,
            DEST_DIR,
            unreal.MaterialInstanceConstant,
            unreal.MaterialInstanceConstantFactoryNew(),
        )
    if not material:
        raise RuntimeError(f"Failed to create material instance: {asset_path}")

    material.set_editor_property("parent", parent)
    unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(material, "DiffuseColorMap", texture)
    unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(material, "BaseColorTexture", texture)
    unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(
        material,
        "Tint",
        unreal.LinearColor(1.0, 1.0, 1.0, 1.0),
    )
    unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(material, "Brightness", 1.0)
    unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(material, "Opacity", 1.0)
    unreal.EditorAssetLibrary.save_asset(asset_path)
    return material


def main():
    project_dir = get_project_dir()
    parent = unreal.EditorAssetLibrary.load_asset(MASTER_PATH)
    if not parent:
        fail(f"Missing master material: {MASTER_PATH}")
        return

    ensure_directory(DEST_DIR)
    log("START")
    for entry in IMPORTS:
        source_path = os.path.join(project_dir, *entry["source"]).replace("\\", "/")
        if not os.path.isfile(source_path):
            fail(f"Missing source file: {source_path}")
            return

        texture = import_texture(source_path, entry["texture_name"])
        material = ensure_material_instance(parent, entry["material_name"], texture)
        log(f"Ready texture: {texture.get_path_name()}")
        log(f"Ready material: {material.get_path_name()}")

    log("DONE")


if __name__ == "__main__":
    main()
