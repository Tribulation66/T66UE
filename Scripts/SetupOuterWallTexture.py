"""
Import SourceAssets/OuterWallTexture.png into /Game/World/Cliffs and create
MI_OuterWallTexture so the procedural outer boundary can use a persistent asset.

Run in Unreal Editor:
  Tools -> Execute Python Script -> Scripts/SetupOuterWallTexture.py
"""

import os
import unreal

LOG = "[OuterWallSetup]"
MASTER_PATH = "/Game/Materials/M_Environment_Unlit"
SOURCE_RELATIVE = ("SourceAssets", "OuterWallTexture.png")
DEST_DIR = "/Game/World/Cliffs"
TEXTURE_NAME = "T_OuterWallTexture"
MATERIAL_NAME = "MI_OuterWallTexture"


def log(msg):
    unreal.log(f"{LOG} {msg}")


def fail(msg):
    unreal.log_error(f"{LOG} {msg}")


def ensure_directory(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def import_texture(source_path):
    ensure_directory(DEST_DIR)

    task = unreal.AssetImportTask()
    task.automated = True
    task.save = False
    task.replace_existing = True
    task.replace_existing_settings = True
    task.filename = source_path
    task.destination_path = DEST_DIR
    task.destination_name = TEXTURE_NAME

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    asset_path = f"{DEST_DIR}/{TEXTURE_NAME}"
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


def ensure_material_instance(parent, texture):
    asset_path = f"{DEST_DIR}/{MATERIAL_NAME}"
    asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    if not asset:
        asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
            MATERIAL_NAME,
            DEST_DIR,
            unreal.MaterialInstanceConstant,
            unreal.MaterialInstanceConstantFactoryNew(),
        )
    if not asset:
        raise RuntimeError(f"Failed to create asset: {asset_path}")

    asset.set_editor_property("parent", parent)
    unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(asset, "DiffuseColorMap", texture)
    unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(asset, "BaseColorTexture", texture)
    unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(
        asset,
        "Tint",
        unreal.LinearColor(1.0, 1.0, 1.0, 1.0),
    )
    unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(asset, "Brightness", 1.0)
    unreal.EditorAssetLibrary.save_asset(asset_path)
    return asset


def main():
    project_dir = unreal.SystemLibrary.get_project_directory().replace("\\", "/").rstrip("/")
    source_path = os.path.join(project_dir, *SOURCE_RELATIVE).replace("\\", "/")
    if not os.path.isfile(source_path):
        fail(f"Missing source file: {source_path}")
        return

    parent = unreal.EditorAssetLibrary.load_asset(MASTER_PATH)
    if not parent:
        fail(f"Missing master material: {MASTER_PATH}")
        return

    log("START")
    texture = import_texture(source_path)
    material = ensure_material_instance(parent, texture)
    log(f"Ready texture: {texture.get_path_name()}")
    log(f"Ready material: {material.get_path_name()}")
    log("DONE")


if __name__ == "__main__":
    main()
