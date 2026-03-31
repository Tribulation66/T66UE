"""
Import the approved fire explosion flipbook and build a dedicated fire idol material.

Run with:
  "C:/Program Files/Epic Games/UE_5.7/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" ^
    "C:/UE/T66/T66.uproject" -unattended -NullRHI -stdout -FullStdOutLogOutput ^
    -ExecutePythonScript="C:/UE/T66/Scripts/SetupFireIdolFlipbook.py"
"""

import os
import unreal


LOG = "[FireIdolFlipbook]"
DEST_DIR = "/Game/VFX/Idols/Fire"
TEXTURE_NAME = "T_IdolFire_Explosion_Flipbook"
TEXTURE_PATH = f"{DEST_DIR}/{TEXTURE_NAME}"
MASTER_NAME = "M_IdolFire_Explosion_Flipbook_Additive"
MASTER_PATH = f"{DEST_DIR}/{MASTER_NAME}"
INSTANCE_NAME = "MI_IdolFire_Explosion_Flipbook_Additive"
INSTANCE_PATH = f"{DEST_DIR}/{INSTANCE_NAME}"
SOURCE_TEXTURE = r"C:/UE/T66/SourceAssets/Import/VFX/Idol_Fire_Explosion/FireExplosion_Flipbook_4x4.png"
FLIPBOOK_FN_PATH = "/Engine/Functions/Engine_MaterialFunctions02/Texturing/FlipBook_UniformNonUniform"

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
mel = unreal.MaterialEditingLibrary


def log(message):
    unreal.log(f"{LOG} {message}")


def warn(message):
    unreal.log_warning(f"{LOG} {message}")


def safe_set(obj, prop, value):
    try:
        obj.set_editor_property(prop, value)
    except Exception as exc:
        warn(f"Failed to set {prop}: {exc}")


def ensure_directory(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def connect(src, src_output, dst, dst_input):
    mel.connect_material_expressions(src, src_output, dst, dst_input)


def recreate_asset(asset_path, asset_class, factory):
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        if not unreal.EditorAssetLibrary.delete_asset(asset_path):
            raise RuntimeError(f"Failed to delete existing asset {asset_path}")

    asset_name = asset_path.rsplit("/", 1)[-1]
    package_path = asset_path.rsplit("/", 1)[0]
    asset = asset_tools.create_asset(asset_name, package_path, asset_class, factory)
    if not asset:
        raise RuntimeError(f"Failed to create asset {asset_path}")
    return asset


def create_expression(material, expr_class, x, y):
    expr = mel.create_material_expression(material, expr_class, x, y)
    if not expr:
        raise RuntimeError(f"Failed to create expression {expr_class}")
    return expr


def import_texture():
    existing = unreal.EditorAssetLibrary.load_asset(f"{TEXTURE_PATH}.{TEXTURE_NAME}")
    if existing:
        log(f"Using existing texture {TEXTURE_PATH}")
        return existing

    if not os.path.exists(SOURCE_TEXTURE):
        raise RuntimeError(f"Missing source texture {SOURCE_TEXTURE}")

    task = unreal.AssetImportTask()
    task.set_editor_property("filename", SOURCE_TEXTURE)
    task.set_editor_property("destination_path", DEST_DIR)
    task.set_editor_property("destination_name", TEXTURE_NAME)
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("replace_existing_settings", True)
    task.set_editor_property("save", True)
    asset_tools.import_asset_tasks([task])

    texture = unreal.EditorAssetLibrary.load_asset(f"{TEXTURE_PATH}.{TEXTURE_NAME}")
    if not texture:
        raise RuntimeError(f"Failed to import {TEXTURE_PATH}")

    safe_set(texture, "mip_gen_settings", unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
    safe_set(texture, "filter", unreal.TextureFilter.TF_BILINEAR)
    safe_set(texture, "never_stream", True)
    safe_set(texture, "srgb", True)
    try:
        texture.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_DEFAULT)
    except Exception as exc:
        warn(f"Failed to set compression settings: {exc}")

    unreal.EditorAssetLibrary.save_loaded_asset(texture)
    log(f"Imported {TEXTURE_PATH}")
    return texture


def build_master_material():
    material = recreate_asset(MASTER_PATH, unreal.Material, unreal.MaterialFactoryNew())
    safe_set(material, "shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    safe_set(material, "blend_mode", unreal.BlendMode.BLEND_ADDITIVE)
    safe_set(material, "two_sided", True)

    flipbook_fn = unreal.EditorAssetLibrary.load_asset(FLIPBOOK_FN_PATH)
    if not flipbook_fn:
        raise RuntimeError(f"Missing material function {FLIPBOOK_FN_PATH}")

    age = create_expression(material, unreal.MaterialExpressionScalarParameter, -2200, -320)
    safe_set(age, "parameter_name", "Age01")
    safe_set(age, "default_value", 0.0)

    rows = create_expression(material, unreal.MaterialExpressionScalarParameter, -2200, -40)
    safe_set(rows, "parameter_name", "Rows")
    safe_set(rows, "default_value", 4.0)

    columns = create_expression(material, unreal.MaterialExpressionScalarParameter, -2200, 220)
    safe_set(columns, "parameter_name", "Columns")
    safe_set(columns, "default_value", 4.0)

    glow = create_expression(material, unreal.MaterialExpressionScalarParameter, -2200, 740)
    safe_set(glow, "parameter_name", "GlowStrength")
    safe_set(glow, "default_value", 1.9)

    texcoord = create_expression(material, unreal.MaterialExpressionTextureCoordinate, -2200, 480)

    mip_bias = create_expression(material, unreal.MaterialExpressionConstant, -2200, 1260)
    safe_set(mip_bias, "r", 0.0)

    clamp_anim = create_expression(material, unreal.MaterialExpressionStaticBool, -2200, 1500)
    safe_set(clamp_anim, "value", True)

    texture_object = create_expression(material, unreal.MaterialExpressionTextureObjectParameter, -2200, 1780)
    safe_set(texture_object, "parameter_name", "FlipbookTexture")

    flipbook = create_expression(material, unreal.MaterialExpressionMaterialFunctionCall, -1480, 760)
    safe_set(flipbook, "material_function", flipbook_fn)

    rgb_mask = create_expression(material, unreal.MaterialExpressionComponentMask, -980, 600)
    safe_set(rgb_mask, "r", True)
    safe_set(rgb_mask, "g", True)
    safe_set(rgb_mask, "b", True)
    safe_set(rgb_mask, "a", False)

    emissive_mul = create_expression(material, unreal.MaterialExpressionMultiply, -620, 640)

    connect(age, "", flipbook, "Animation  Phase (0-1)")
    connect(rows, "", flipbook, "Number of Rows")
    connect(columns, "", flipbook, "Number of Columns")
    connect(texture_object, "", flipbook, "Texture")
    connect(texcoord, "", flipbook, "UVs")
    connect(mip_bias, "", flipbook, "MipBias")
    connect(clamp_anim, "", flipbook, "Clamp Anim (see tooltip)")

    connect(flipbook, "", rgb_mask, "")
    connect(rgb_mask, "", emissive_mul, "A")
    connect(glow, "", emissive_mul, "B")

    mel.connect_material_property(emissive_mul, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)

    mel.layout_material_expressions(material)
    mel.recompile_material(material)
    unreal.EditorAssetLibrary.save_asset(MASTER_PATH)
    log(f"Built {MASTER_PATH}")
    return material


def build_instance(texture):
    parent = unreal.EditorAssetLibrary.load_asset(MASTER_PATH)
    if not parent:
        raise RuntimeError(f"Missing parent material {MASTER_PATH}")

    instance = recreate_asset(
        INSTANCE_PATH,
        unreal.MaterialInstanceConstant,
        unreal.MaterialInstanceConstantFactoryNew(),
    )
    safe_set(instance, "parent", parent)

    mel.set_material_instance_texture_parameter_value(instance, "FlipbookTexture", texture)
    mel.set_material_instance_scalar_parameter_value(instance, "Rows", 4.0)
    mel.set_material_instance_scalar_parameter_value(instance, "Columns", 4.0)
    mel.set_material_instance_scalar_parameter_value(instance, "GlowStrength", 1.9)
    mel.set_material_instance_scalar_parameter_value(instance, "Age01", 0.0)
    mel.update_material_instance(instance)
    unreal.EditorAssetLibrary.save_asset(INSTANCE_PATH)
    log(f"Built {INSTANCE_PATH}")


def main():
    ensure_directory(DEST_DIR)
    texture = import_texture()
    build_master_material()
    build_instance(texture)
    log("DONE")


if __name__ == "__main__":
    main()
