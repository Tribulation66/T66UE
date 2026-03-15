"""
Import procedural terrain textures and create material instances for runtime use.

Run in Unreal Editor:
  Tools -> Execute Python Script -> Scripts/SetupTerrainTextureMaterials.py

Or headless:
  UnrealEditor-Cmd.exe T66.uproject -run=pythonscript -script="C:/UE/T66/Scripts/SetupTerrainTextureMaterials.py"
"""

import os
import unreal

LOG = "[TerrainSetup]"
MASTER_PATH = "/Game/Materials/M_Environment_Unlit"

TERRAIN_IMPORTS = [
    {
        "source": "Grass1.png",
        "dest_dir": "/Game/World/Ground",
        "texture_name": "T_Grass1",
        "material_name": "MI_Grass1",
    },
    {
        "source": "Grass2.png",
        "dest_dir": "/Game/World/Ground",
        "texture_name": "T_Grass2",
        "material_name": "MI_Grass2",
    },
    {
        "source": "Grass3.png",
        "dest_dir": "/Game/World/Ground",
        "texture_name": "T_Grass3",
        "material_name": "MI_Grass3",
    },
    {
        "source": "Grass4.png",
        "dest_dir": "/Game/World/Ground",
        "texture_name": "T_Grass4",
        "material_name": "MI_Grass4",
    },
    {
        "source": "Cliff2.png",
        "dest_dir": "/Game/World/Cliffs",
        "texture_name": "T_Cliff2",
        "material_name": "MI_Cliff2",
    },
    {
        "source": "Cliff3.png",
        "dest_dir": "/Game/World/Cliffs",
        "texture_name": "T_Cliff3",
        "material_name": "MI_Cliff3",
    },
]


def log(msg):
    unreal.log(f"{LOG} {msg}")


def warn(msg):
    unreal.log_warning(f"{LOG} {msg}")


def fail(msg):
    unreal.log_error(f"{LOG} {msg}")


def get_project_dir():
    return unreal.SystemLibrary.get_project_directory().replace("\\", "/").rstrip("/")


def ensure_directory(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def import_texture(source_path, dest_dir, dest_name):
    ensure_directory(dest_dir)

    task = unreal.AssetImportTask()
    task.automated = True
    task.save = False
    task.replace_existing = True
    task.replace_existing_settings = True
    task.filename = source_path
    task.destination_path = dest_dir
    task.destination_name = dest_name

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    asset_path = f"{dest_dir}/{dest_name}"
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


def create_asset_if_missing(asset_path, asset_class, factory):
    asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    if asset:
        return asset

    package_path, asset_name = asset_path.rsplit("/", 1)
    ensure_directory(package_path)
    asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        asset_name,
        package_path,
        asset_class,
        factory,
    )
    if not asset:
        raise RuntimeError(f"Failed to create asset: {asset_path}")
    return asset


def setup_material_instance(asset_path, parent, texture):
    mi = create_asset_if_missing(
        asset_path,
        unreal.MaterialInstanceConstant,
        unreal.MaterialInstanceConstantFactoryNew(),
    )
    mi.set_editor_property("parent", parent)
    unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(mi, "DiffuseColorMap", texture)
    unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(
        mi,
        "Tint",
        unreal.LinearColor(1.0, 1.0, 1.0, 1.0),
    )
    unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(mi, "Brightness", 1.0)
    unreal.EditorAssetLibrary.save_asset(asset_path)
    return mi


def main():
    project_dir = get_project_dir()
    import_dir = os.path.join(project_dir, "SourceAssets", "Import").replace("\\", "/")

    master = unreal.EditorAssetLibrary.load_asset(MASTER_PATH)
    if not master:
        fail(f"Missing master material: {MASTER_PATH}")
        return

    log("START")
    log(f"Import root: {import_dir}")

    for entry in TERRAIN_IMPORTS:
        source_path = os.path.join(import_dir, entry["source"]).replace("\\", "/")
        if not os.path.isfile(source_path):
            warn(f"Missing source file: {source_path}")
            continue

        texture = import_texture(source_path, entry["dest_dir"], entry["texture_name"])
        material_path = f"{entry['dest_dir']}/{entry['material_name']}"
        setup_material_instance(material_path, master, texture)
        log(f"Ready: {material_path}")

    log("DONE")


if __name__ == "__main__":
    main()
