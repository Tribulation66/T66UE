"""
Build a top-only Quake sky material from already-imported texture assets.

Run in Unreal commandlet mode:
  UnrealEditor-Cmd.exe T66.uproject -run=pythonscript -script="C:/UE/T66/Scripts/BuildQuakeSkyOverhead2.py"
"""

import unreal

LOG = "[QuakeSky]"
DEST_DIR = "/Game/World/Sky/QuakeOverhead2"
MASTER_NAME = "M_QuakeSky_Overhead2"
MASTER_PATH = f"{DEST_DIR}/{MASTER_NAME}"
INSTANCE_NAME = "MI_QuakeSky_Overhead2"
INSTANCE_PATH = f"{DEST_DIR}/{INSTANCE_NAME}"
BACKGROUND_TEXTURE_PATH = "/Game/World/Sky/QuakeOverhead/T_QuakeSky_Overhead_Background.T_QuakeSky_Overhead_Background"
CLOUD_TEXTURE_PATH = "/Game/World/Sky/QuakeOverhead/T_QuakeSky_Overhead_Clouds.T_QuakeSky_Overhead_Clouds"

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
    safe_set(material, "two_sided", True)
    blend_mode = getattr(unreal.BlendMode, "BLEND_MASKED", None)
    if blend_mode is not None:
        safe_set(material, "blend_mode", blend_mode)
    safe_set(material, "is_sky", False)
    safe_set(material, "opacity_mask_clip_value", 0.05)

    world_position = create_expression(material, unreal.MaterialExpressionWorldPosition, -1800, -120)
    object_position = create_expression(material, unreal.MaterialExpressionObjectPositionWS, -1800, -360)

    dome_direction = create_expression(material, unreal.MaterialExpressionSubtract, -1500, -220)
    connect(world_position, "", dome_direction, "A")
    connect(object_position, "", dome_direction, "B")

    normalized_dome_direction = create_expression(material, unreal.MaterialExpressionNormalize, -1220, -220)
    connect(dome_direction, "", normalized_dome_direction, "")

    direction_scale = create_expression(material, unreal.MaterialExpressionVectorParameter, -1800, 240)
    safe_set(direction_scale, "parameter_name", "DirectionScale")
    safe_set(direction_scale, "default_value", unreal.LinearColor(-0.2, -0.2, 1.0, 0.0))

    scale_dir = create_expression(material, unreal.MaterialExpressionMultiply, -1500, 80)
    connect(normalized_dome_direction, "", scale_dir, "A")
    connect(direction_scale, "", scale_dir, "B")

    normalize_dir = create_expression(material, unreal.MaterialExpressionNormalize, -1220, 80)
    connect(scale_dir, "", normalize_dir, "")

    dir_r = create_expression(material, unreal.MaterialExpressionComponentMask, -960, -60)
    safe_set(dir_r, "r", True)
    connect(normalize_dir, "", dir_r, "")

    dir_g = create_expression(material, unreal.MaterialExpressionComponentMask, -960, 180)
    safe_set(dir_g, "g", True)
    connect(normalize_dir, "", dir_g, "")

    dome_up = create_expression(material, unreal.MaterialExpressionComponentMask, -960, -360)
    safe_set(dome_up, "b", True)
    connect(normalized_dome_direction, "", dome_up, "")

    base_uv = create_expression(material, unreal.MaterialExpressionAppendVector, -700, 60)
    connect(dir_g, "", base_uv, "A")
    connect(dir_r, "", base_uv, "B")

    horizon_fade_start = create_expression(material, unreal.MaterialExpressionScalarParameter, -700, -520)
    safe_set(horizon_fade_start, "parameter_name", "HorizonFadeStart")
    safe_set(horizon_fade_start, "default_value", 0.03)

    horizon_fade_end = create_expression(material, unreal.MaterialExpressionScalarParameter, -700, -680)
    safe_set(horizon_fade_end, "parameter_name", "HorizonFadeEnd")
    safe_set(horizon_fade_end, "default_value", 0.20)

    fade_numerator = create_expression(material, unreal.MaterialExpressionSubtract, -430, -420)
    connect(dome_up, "", fade_numerator, "A")
    connect(horizon_fade_start, "", fade_numerator, "B")

    fade_range = create_expression(material, unreal.MaterialExpressionSubtract, -430, -620)
    connect(horizon_fade_end, "", fade_range, "A")
    connect(horizon_fade_start, "", fade_range, "B")

    fade_ratio = create_expression(material, unreal.MaterialExpressionDivide, -160, -500)
    connect(fade_numerator, "", fade_ratio, "A")
    connect(fade_range, "", fade_ratio, "B")

    sky_visibility = create_expression(material, unreal.MaterialExpressionSaturate, 160, -500)
    connect(fade_ratio, "", sky_visibility, "")

    bg_scale = create_expression(material, unreal.MaterialExpressionScalarParameter, -700, -220)
    safe_set(bg_scale, "parameter_name", "BackgroundTiling")
    safe_set(bg_scale, "default_value", 10.0)

    cloud_scale = create_expression(material, unreal.MaterialExpressionScalarParameter, -700, 320)
    safe_set(cloud_scale, "parameter_name", "CloudTiling")
    safe_set(cloud_scale, "default_value", 5.0)

    bg_scaled_uv = create_expression(material, unreal.MaterialExpressionMultiply, -430, -120)
    connect(base_uv, "", bg_scaled_uv, "A")
    connect(bg_scale, "", bg_scaled_uv, "B")

    cloud_scaled_uv = create_expression(material, unreal.MaterialExpressionMultiply, -430, 360)
    connect(base_uv, "", cloud_scaled_uv, "A")
    connect(cloud_scale, "", cloud_scaled_uv, "B")

    bg_panner = create_expression(material, unreal.MaterialExpressionPanner, -160, -120)
    safe_set(bg_panner, "speed_x", 5.0)
    connect(bg_scaled_uv, "", bg_panner, "Coordinate")

    cloud_panner = create_expression(material, unreal.MaterialExpressionPanner, -160, 360)
    safe_set(cloud_panner, "speed_x", 8.0)
    connect(cloud_scaled_uv, "", cloud_panner, "Coordinate")

    bg_tex = create_expression(material, unreal.MaterialExpressionTextureSampleParameter2D, 160, -120)
    safe_set(bg_tex, "parameter_name", "BackgroundTexture")

    cloud_tex = create_expression(material, unreal.MaterialExpressionTextureSampleParameter2D, 160, 360)
    safe_set(cloud_tex, "parameter_name", "CloudTexture")

    connect(bg_panner, "", bg_tex, "UVs")
    connect(cloud_panner, "", cloud_tex, "UVs")

    layer_mix = create_expression(material, unreal.MaterialExpressionLinearInterpolate, 460, 100)
    connect(bg_tex, "RGB", layer_mix, "A")
    connect(cloud_tex, "RGB", layer_mix, "B")
    connect(cloud_tex, "A", layer_mix, "Alpha")

    brightness = create_expression(material, unreal.MaterialExpressionScalarParameter, 460, 320)
    safe_set(brightness, "parameter_name", "Brightness")
    safe_set(brightness, "default_value", 1.0)

    emissive = create_expression(material, unreal.MaterialExpressionMultiply, 760, 120)
    connect(layer_mix, "", emissive, "A")
    connect(brightness, "", emissive, "B")

    visible_emissive = create_expression(material, unreal.MaterialExpressionMultiply, 1040, 40)
    connect(emissive, "", visible_emissive, "A")
    connect(sky_visibility, "", visible_emissive, "B")

    mel.connect_material_property(visible_emissive, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    mel.connect_material_property(sky_visibility, "", unreal.MaterialProperty.MP_OPACITY_MASK)
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
    mel.set_material_instance_scalar_parameter_value(instance, "BackgroundTiling", 10.0)
    mel.set_material_instance_scalar_parameter_value(instance, "CloudTiling", 5.0)
    mel.set_material_instance_scalar_parameter_value(instance, "Brightness", 1.0)
    mel.set_material_instance_scalar_parameter_value(instance, "HorizonFadeStart", 0.03)
    mel.set_material_instance_scalar_parameter_value(instance, "HorizonFadeEnd", 0.20)
    mel.set_material_instance_vector_parameter_value(
        instance,
        "DirectionScale",
        unreal.LinearColor(-0.2, -0.2, 1.0, 0.0),
    )
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
