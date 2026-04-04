"""
Import procedural terrain textures and create material instances for runtime use.

Run in Unreal Editor:
  Tools -> Execute Python Script -> Scripts/SetupTerrainTextureMaterials.py

Or headless:
  UnrealEditor-Cmd.exe T66.uproject -run=pythonscript -script="C:/UE/T66/Scripts/SetupTerrainTextureMaterials.py"
"""

import os
import re
import unreal

LOG = "[TerrainSetup]"
MASTER_PATH = "/Game/Materials/M_Environment_Unlit"

TERRAIN_SETS = [
    {
        "source_prefix": "MegabonkBlock",
        "dest_dir": "/Game/World/Terrain/Megabonk",
        "texture_prefix": "T_MegabonkBlock",
        "material_prefix": "MI_MegabonkBlock",
    },
    {
        "source_prefix": "MegabonkSlope",
        "dest_dir": "/Game/World/Terrain/Megabonk",
        "texture_prefix": "T_MegabonkSlope",
        "material_prefix": "MI_MegabonkSlope",
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


def discover_terrain_imports(import_dir):
    entries = []
    filenames = os.listdir(import_dir) if os.path.isdir(import_dir) else []

    for terrain_set in TERRAIN_SETS:
        pattern = re.compile(rf"^{re.escape(terrain_set['source_prefix'])}(\d+)\.png$", re.IGNORECASE)
        matches = []
        for filename in filenames:
            match = pattern.match(filename)
            if match:
                matches.append((int(match.group(1)), filename))

        matches.sort(key=lambda item: item[0])
        for index, filename in matches:
            entries.append(
                {
                    "source": filename,
                    "dest_dir": terrain_set["dest_dir"],
                    "texture_name": f"{terrain_set['texture_prefix']}{index}",
                    "material_name": f"{terrain_set['material_prefix']}{index}",
                }
            )

    return entries


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
    terrain_imports = discover_terrain_imports(import_dir)

    master = unreal.EditorAssetLibrary.load_asset(MASTER_PATH)
    if not master:
        fail(f"Missing master material: {MASTER_PATH}")
        return

    log("START")
    log(f"Import root: {import_dir}")

    if not terrain_imports:
        warn("No GroundTile*.png or HillTile*.png files found in SourceAssets/Import.")
        return

    for entry in terrain_imports:
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
