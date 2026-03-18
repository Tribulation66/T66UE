"""
Build an overhead-only Quake sky canopy material from existing texture assets.

Run in Unreal commandlet mode:
  UnrealEditor-Cmd.exe T66.uproject -run=pythonscript -script="C:/UE/T66/Scripts/BuildQuakeSkyCanopy.py"
"""

import unreal

LOG = "[QuakeSkyCanopy]"
DEST_DIR = "/Game/World/Sky/QuakeCanopy2"
MASTER_NAME = "M_QuakeSky_Canopy2"
MASTER_PATH = f"{DEST_DIR}/{MASTER_NAME}"
INSTANCE_NAME = "MI_QuakeSky_Canopy2"
INSTANCE_PATH = f"{DEST_DIR}/{INSTANCE_NAME}"
BACKGROUND_TEXTURE_PATH = "/Game/World/Sky/Quake/T_QuakeSky_Background.T_QuakeSky_Background"
CLOUD_TEXTURE_PATH = "/Game/World/Sky/Quake/T_QuakeSky_Clouds.T_QuakeSky_Clouds"

mel = unreal.MaterialEditingLibrary
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()


def log(message):
    unreal.log(f"{LOG} {message}")


def safe_set(obj, prop, value):
    try:
        obj.set_editor_property(prop, value)
    except Exception as exc:
        unreal.log_warning(f"{LOG} failed to set {prop}: {exc}")


def connect(src, src_output, dst, dst_input):
    mel.connect_material_expressions(src, src_output, dst, dst_input)


def ensure_directory(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


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


def build_master_material():
    material = recreate_asset(MASTER_PATH, unreal.Material, unreal.MaterialFactoryNew())
    safe_set(material, "shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    safe_set(material, "blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)
    safe_set(material, "two_sided", True)
    safe_set(material, "is_sky", False)

    texcoord = create_expression(material, unreal.MaterialExpressionTextureCoordinate, -1400, 120)

    bg_scale = create_expression(material, unreal.MaterialExpressionScalarParameter, -1160, -100)
    safe_set(bg_scale, "parameter_name", "BackgroundTiling")
    safe_set(bg_scale, "default_value", 3.0)

    cloud_scale = create_expression(material, unreal.MaterialExpressionScalarParameter, -1160, 320)
    safe_set(cloud_scale, "parameter_name", "CloudTiling")
    safe_set(cloud_scale, "default_value", 5.0)

    bg_scaled_uv = create_expression(material, unreal.MaterialExpressionMultiply, -900, -100)
    connect(texcoord, "", bg_scaled_uv, "A")
    connect(bg_scale, "", bg_scaled_uv, "B")

    cloud_scaled_uv = create_expression(material, unreal.MaterialExpressionMultiply, -900, 320)
    connect(texcoord, "", cloud_scaled_uv, "A")
    connect(cloud_scale, "", cloud_scaled_uv, "B")

    pan_time = create_expression(material, unreal.MaterialExpressionScalarParameter, -1160, 560)
    safe_set(pan_time, "parameter_name", "SkyPanTime")
    safe_set(pan_time, "default_value", 0.0)

    bg_speed = create_expression(material, unreal.MaterialExpressionScalarParameter, -900, -300)
    safe_set(bg_speed, "parameter_name", "BackgroundPanSpeed")
    safe_set(bg_speed, "default_value", 0.010)

    cloud_speed = create_expression(material, unreal.MaterialExpressionScalarParameter, -900, 120)
    safe_set(cloud_speed, "parameter_name", "CloudPanSpeed")
    safe_set(cloud_speed, "default_value", 0.024)

    zero = create_expression(material, unreal.MaterialExpressionConstant, -900, 760)
    safe_set(zero, "r", 0.0)

    bg_pan_x = create_expression(material, unreal.MaterialExpressionMultiply, -620, -300)
    connect(pan_time, "", bg_pan_x, "A")
    connect(bg_speed, "", bg_pan_x, "B")

    bg_pan_offset = create_expression(material, unreal.MaterialExpressionAppendVector, -320, -260)
    connect(bg_pan_x, "", bg_pan_offset, "A")
    connect(zero, "", bg_pan_offset, "B")

    bg_uv = create_expression(material, unreal.MaterialExpressionAdd, -40, -120)
    connect(bg_scaled_uv, "", bg_uv, "A")
    connect(bg_pan_offset, "", bg_uv, "B")

    cloud_pan_x = create_expression(material, unreal.MaterialExpressionMultiply, -620, 120)
    connect(pan_time, "", cloud_pan_x, "A")
    connect(cloud_speed, "", cloud_pan_x, "B")

    cloud_pan_offset = create_expression(material, unreal.MaterialExpressionAppendVector, -320, 160)
    connect(cloud_pan_x, "", cloud_pan_offset, "A")
    connect(zero, "", cloud_pan_offset, "B")

    cloud_uv = create_expression(material, unreal.MaterialExpressionAdd, -40, 320)
    connect(cloud_scaled_uv, "", cloud_uv, "A")
    connect(cloud_pan_offset, "", cloud_uv, "B")

    bg_tex = create_expression(material, unreal.MaterialExpressionTextureSampleParameter2D, 260, -100)
    safe_set(bg_tex, "parameter_name", "BackgroundTexture")
    connect(bg_uv, "", bg_tex, "UVs")

    cloud_tex = create_expression(material, unreal.MaterialExpressionTextureSampleParameter2D, 260, 320)
    safe_set(cloud_tex, "parameter_name", "CloudTexture")
    connect(cloud_uv, "", cloud_tex, "UVs")

    layer_mix = create_expression(material, unreal.MaterialExpressionLinearInterpolate, 560, 100)
    connect(bg_tex, "RGB", layer_mix, "A")
    connect(cloud_tex, "RGB", layer_mix, "B")
    connect(cloud_tex, "A", layer_mix, "Alpha")

    center = create_expression(material, unreal.MaterialExpressionConstant2Vector, -1160, 620)
    safe_set(center, "r", 0.5)
    safe_set(center, "g", 0.5)

    edge_mask = create_expression(material, unreal.MaterialExpressionSphereMask, -900, 620)
    safe_set(edge_mask, "attenuation_radius", 0.48)
    safe_set(edge_mask, "hardness_percent", 12.0)
    connect(texcoord, "", edge_mask, "A")
    connect(center, "", edge_mask, "B")

    brightness = create_expression(material, unreal.MaterialExpressionScalarParameter, 840, -40)
    safe_set(brightness, "parameter_name", "Brightness")
    safe_set(brightness, "default_value", 1.8)

    opacity = create_expression(material, unreal.MaterialExpressionScalarParameter, 840, 220)
    safe_set(opacity, "parameter_name", "Opacity")
    safe_set(opacity, "default_value", 1.0)

    emissive = create_expression(material, unreal.MaterialExpressionMultiply, 1140, 40)
    connect(layer_mix, "", emissive, "A")
    connect(brightness, "", emissive, "B")

    masked_emissive = create_expression(material, unreal.MaterialExpressionMultiply, 1440, 40)
    connect(emissive, "", masked_emissive, "A")
    connect(edge_mask, "", masked_emissive, "B")

    final_opacity = create_expression(material, unreal.MaterialExpressionMultiply, 1140, 240)
    connect(edge_mask, "", final_opacity, "A")
    connect(opacity, "", final_opacity, "B")

    mel.connect_material_property(masked_emissive, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    mel.connect_material_property(final_opacity, "", unreal.MaterialProperty.MP_OPACITY)

    mel.layout_material_expressions(material)
    mel.recompile_material(material)
    unreal.EditorAssetLibrary.save_asset(MASTER_PATH)
    log(f"Built {MASTER_PATH}")
    return material


def build_material_instance(background_texture, cloud_texture):
    instance = recreate_asset(
        INSTANCE_PATH,
        unreal.MaterialInstanceConstant,
        unreal.MaterialInstanceConstantFactoryNew(),
    )

    master = unreal.EditorAssetLibrary.load_asset(MASTER_PATH)
    if not master:
        raise RuntimeError(f"Missing master material {MASTER_PATH}")

    safe_set(instance, "parent", master)
    mel.set_material_instance_texture_parameter_value(instance, "BackgroundTexture", background_texture)
    mel.set_material_instance_texture_parameter_value(instance, "CloudTexture", cloud_texture)
    mel.set_material_instance_scalar_parameter_value(instance, "BackgroundTiling", 3.0)
    mel.set_material_instance_scalar_parameter_value(instance, "CloudTiling", 5.0)
    mel.set_material_instance_scalar_parameter_value(instance, "SkyPanTime", 0.0)
    mel.set_material_instance_scalar_parameter_value(instance, "BackgroundPanSpeed", 0.010)
    mel.set_material_instance_scalar_parameter_value(instance, "CloudPanSpeed", 0.024)
    mel.set_material_instance_scalar_parameter_value(instance, "Brightness", 1.8)
    mel.set_material_instance_scalar_parameter_value(instance, "Opacity", 1.0)
    mel.update_material_instance(instance)
    unreal.EditorAssetLibrary.save_asset(INSTANCE_PATH)
    log(f"Built {INSTANCE_PATH}")
    return instance


def main():
    ensure_directory(DEST_DIR)

    background_texture = unreal.EditorAssetLibrary.load_asset(BACKGROUND_TEXTURE_PATH)
    cloud_texture = unreal.EditorAssetLibrary.load_asset(CLOUD_TEXTURE_PATH)
    if not background_texture:
        raise RuntimeError(f"Missing texture asset {BACKGROUND_TEXTURE_PATH}")
    if not cloud_texture:
        raise RuntimeError(f"Missing texture asset {CLOUD_TEXTURE_PATH}")

    build_master_material()
    build_material_instance(background_texture, cloud_texture)
    log("DONE")


if __name__ == "__main__":
    main()
