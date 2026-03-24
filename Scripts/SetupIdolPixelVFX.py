"""
Import shared pixel-portal textures for idol proc VFX and build a reusable base MI.

Run with:
  "C:/Program Files/Epic Games/UE_5.7/Engine/Binaries/Win64/UnrealEditor.exe" ^
    "C:/UE/T66/T66.uproject" -ExecutePythonScript="C:/UE/T66/Scripts/SetupIdolPixelVFX.py"
"""

import os
import unreal


LOG = "[IdolPixelVFX]"
DEST_DIR = "/Game/VFX/Idols/Shared"
BASE_MI_PATH = f"{DEST_DIR}/MI_IdolPixelPortal_Base"
PARENT_PATH = "/Game/VFX/Hero1/M_Hero1_PixelAttack.M_Hero1_PixelAttack"
SOURCE_ROOT = r"C:/UE/T66/SourceAssets/Example VFX Full/examples/extended/sprites"
TEXTURE_SPECS = {
    "T_IdolVFX_Runes": os.path.join(SOURCE_ROOT, "runes.png"),
    "T_IdolVFX_Seals": os.path.join(SOURCE_ROOT, "seals.png"),
    "T_IdolVFX_Wall": os.path.join(SOURCE_ROOT, "wall.png"),
}

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
mel = unreal.MaterialEditingLibrary


def log(message):
    unreal.log(f"{LOG} {message}")


def ensure_directory(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def import_texture(asset_name, source_path):
    if not os.path.exists(source_path):
        raise RuntimeError(f"Missing source texture: {source_path}")

    task = unreal.AssetImportTask()
    task.set_editor_property("filename", source_path)
    task.set_editor_property("destination_path", DEST_DIR)
    task.set_editor_property("destination_name", asset_name)
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("replace_existing_settings", True)
    task.set_editor_property("save", True)
    asset_tools.import_asset_tasks([task])

    asset_path = f"{DEST_DIR}/{asset_name}.{asset_name}"
    texture = unreal.EditorAssetLibrary.load_asset(asset_path)
    if not texture:
        raise RuntimeError(f"Failed to import {asset_path}")

    texture.set_editor_property("mip_gen_settings", unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
    texture.set_editor_property("filter", unreal.TextureFilter.TF_NEAREST)
    texture.set_editor_property("never_stream", True)
    texture.set_editor_property("srgb", True)
    unreal.EditorAssetLibrary.save_loaded_asset(texture)
    log(f"Imported {asset_path}")
    return texture


def recreate_asset(asset_path, asset_class, factory):
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        unreal.EditorAssetLibrary.delete_asset(asset_path)

    asset_name = asset_path.rsplit("/", 1)[-1]
    package_path = asset_path.rsplit("/", 1)[0]
    asset = asset_tools.create_asset(asset_name, package_path, asset_class, factory)
    if not asset:
        raise RuntimeError(f"Failed to create asset {asset_path}")
    return asset


def build_base_instance(default_texture):
    parent = unreal.EditorAssetLibrary.load_asset(PARENT_PATH)
    if not parent:
        raise RuntimeError(f"Missing parent material: {PARENT_PATH}")

    instance = recreate_asset(
        BASE_MI_PATH,
        unreal.MaterialInstanceConstant,
        unreal.MaterialInstanceConstantFactoryNew(),
    )
    instance.set_editor_property("parent", parent)

    mel.set_material_instance_texture_parameter_value(instance, "DetailTexture", default_texture)
    mel.set_material_instance_vector_parameter_value(instance, "PrimaryColor", unreal.LinearColor(1.0, 1.0, 1.0, 1.0))
    mel.set_material_instance_vector_parameter_value(instance, "SecondaryColor", unreal.LinearColor(0.32, 0.32, 0.32, 1.0))
    mel.set_material_instance_vector_parameter_value(instance, "OutlineColor", unreal.LinearColor(0.0, 0.0, 0.0, 1.0))
    mel.set_material_instance_vector_parameter_value(instance, "TintColor", unreal.LinearColor(1.0, 1.0, 1.0, 1.0))
    mel.set_material_instance_scalar_parameter_value(instance, "PixelGrid", 28.0)
    mel.set_material_instance_scalar_parameter_value(instance, "DetailTiling", 2.4)
    mel.set_material_instance_scalar_parameter_value(instance, "ScrollSpeed", 0.35)
    mel.set_material_instance_scalar_parameter_value(instance, "InnerRadius", 0.34)
    mel.set_material_instance_scalar_parameter_value(instance, "OuterRadius", 0.48)
    mel.set_material_instance_scalar_parameter_value(instance, "GlowStrength", 4.0)
    mel.set_material_instance_scalar_parameter_value(instance, "OpacityBoost", 0.92)
    mel.set_material_instance_scalar_parameter_value(instance, "Age01", 0.0)

    mel.update_material_instance(instance)
    unreal.EditorAssetLibrary.save_asset(BASE_MI_PATH)
    log(f"Built {BASE_MI_PATH}")


def main():
    ensure_directory(DEST_DIR)
    textures = {name: import_texture(name, path) for name, path in TEXTURE_SPECS.items()}
    build_base_instance(textures["T_IdolVFX_Seals"])
    log("DONE")


if __name__ == "__main__":
    main()
