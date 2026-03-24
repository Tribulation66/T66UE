"""
Import the paid pixel portal textures and create Hero 1's attack materials.

Run with:
  "C:/Program Files/Epic Games/UE_5.7/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" ^
    "C:/UE/T66/T66.uproject" -run=pythonscript -script="C:/UE/T66/Scripts/SetupHero1PixelAttackVFX.py"
"""

import os
import unreal


LOG = "[Hero1PixelAttack]"
DEST_DIR = "/Game/VFX/Hero1"
MASTER_NAME = "M_Hero1_PixelAttack"
MASTER_PATH = f"{DEST_DIR}/{MASTER_NAME}"
STREAK_MI_NAME = "MI_Hero1_Attack_Streak"
STREAK_MI_PATH = f"{DEST_DIR}/{STREAK_MI_NAME}"
IMPACT_MI_NAME = "MI_Hero1_Attack_Impact"
IMPACT_MI_PATH = f"{DEST_DIR}/{IMPACT_MI_NAME}"

SOURCE_ROOT = r"C:/UE/T66/SourceAssets/Example VFX Full/examples/extended/sprites"
TEXTURE_SPECS = {
    "T_Hero1_Attack_Runes": os.path.join(SOURCE_ROOT, "runes.png"),
    "T_Hero1_Attack_Seals": os.path.join(SOURCE_ROOT, "seals.png"),
    "T_Hero1_Attack_Wall": os.path.join(SOURCE_ROOT, "wall.png"),
}

mel = unreal.MaterialEditingLibrary
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()


def log(message):
    unreal.log(f"{LOG} {message}")


def warn(message):
    unreal.log_warning(f"{LOG} {message}")


def safe_set(obj, prop, value):
    try:
        obj.set_editor_property(prop, value)
    except Exception as exc:
        warn(f"failed to set {prop}: {exc}")


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


def import_texture(asset_name, source_path):
    if not os.path.exists(source_path):
        raise RuntimeError(f"Missing source texture {source_path}")

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
        raise RuntimeError(f"Failed to import {source_path} to {asset_path}")

    safe_set(texture, "mip_gen_settings", unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
    safe_set(texture, "filter", unreal.TextureFilter.TF_NEAREST)
    safe_set(texture, "never_stream", True)
    safe_set(texture, "srgb", True)
    try:
        texture.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_DEFAULT)
    except Exception:
        pass

    unreal.EditorAssetLibrary.save_loaded_asset(texture)
    log(f"Imported {asset_path}")
    return texture


def build_master_material():
    material = recreate_asset(MASTER_PATH, unreal.Material, unreal.MaterialFactoryNew())
    safe_set(material, "shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    safe_set(material, "blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)
    safe_set(material, "two_sided", True)

    texcoord = create_expression(material, unreal.MaterialExpressionTextureCoordinate, -2300, -80)
    time = create_expression(material, unreal.MaterialExpressionTime, -2300, 780)

    pixel_grid = create_expression(material, unreal.MaterialExpressionScalarParameter, -2060, -340)
    safe_set(pixel_grid, "parameter_name", "PixelGrid")
    safe_set(pixel_grid, "default_value", 32.0)

    detail_tiling = create_expression(material, unreal.MaterialExpressionScalarParameter, -2060, 150)
    safe_set(detail_tiling, "parameter_name", "DetailTiling")
    safe_set(detail_tiling, "default_value", 2.0)

    scroll_speed = create_expression(material, unreal.MaterialExpressionScalarParameter, -2060, 760)
    safe_set(scroll_speed, "parameter_name", "ScrollSpeed")
    safe_set(scroll_speed, "default_value", 0.4)

    age = create_expression(material, unreal.MaterialExpressionScalarParameter, -2060, 1080)
    safe_set(age, "parameter_name", "Age01")
    safe_set(age, "default_value", 0.0)

    tint_color = create_expression(material, unreal.MaterialExpressionVectorParameter, -2060, 1390)
    safe_set(tint_color, "parameter_name", "TintColor")
    safe_set(tint_color, "default_value", unreal.LinearColor(1.0, 1.0, 1.0, 1.0))

    inner_radius = create_expression(material, unreal.MaterialExpressionScalarParameter, -2060, -880)
    safe_set(inner_radius, "parameter_name", "InnerRadius")
    safe_set(inner_radius, "default_value", 0.38)

    outer_radius = create_expression(material, unreal.MaterialExpressionScalarParameter, -2060, -1110)
    safe_set(outer_radius, "parameter_name", "OuterRadius")
    safe_set(outer_radius, "default_value", 0.46)

    primary_color = create_expression(material, unreal.MaterialExpressionVectorParameter, -2060, 1710)
    safe_set(primary_color, "parameter_name", "PrimaryColor")
    safe_set(primary_color, "default_value", unreal.LinearColor(1.0, 0.94, 0.72, 1.0))

    secondary_color = create_expression(material, unreal.MaterialExpressionVectorParameter, -2060, 1970)
    safe_set(secondary_color, "parameter_name", "SecondaryColor")
    safe_set(secondary_color, "default_value", unreal.LinearColor(0.42, 0.24, 0.06, 1.0))

    outline_color = create_expression(material, unreal.MaterialExpressionVectorParameter, -2060, 2230)
    safe_set(outline_color, "parameter_name", "OutlineColor")
    safe_set(outline_color, "default_value", unreal.LinearColor(0.0, 0.0, 0.0, 1.0))

    glow = create_expression(material, unreal.MaterialExpressionScalarParameter, -2060, 2490)
    safe_set(glow, "parameter_name", "GlowStrength")
    safe_set(glow, "default_value", 4.0)

    opacity_boost = create_expression(material, unreal.MaterialExpressionScalarParameter, -2060, 2720)
    safe_set(opacity_boost, "parameter_name", "OpacityBoost")
    safe_set(opacity_boost, "default_value", 0.95)

    center = create_expression(material, unreal.MaterialExpressionConstant2Vector, -1780, -1080)
    safe_set(center, "r", 0.5)
    safe_set(center, "g", 0.5)

    mask_uv_mul = create_expression(material, unreal.MaterialExpressionMultiply, -1780, -120)
    connect(texcoord, "", mask_uv_mul, "A")
    connect(pixel_grid, "", mask_uv_mul, "B")

    mask_uv_floor = create_expression(material, unreal.MaterialExpressionFloor, -1540, -120)
    connect(mask_uv_mul, "", mask_uv_floor, "")

    mask_uv = create_expression(material, unreal.MaterialExpressionDivide, -1290, -120)
    connect(mask_uv_floor, "", mask_uv, "A")
    connect(pixel_grid, "", mask_uv, "B")

    detail_scaled_uv = create_expression(material, unreal.MaterialExpressionMultiply, -1780, 150)
    connect(texcoord, "", detail_scaled_uv, "A")
    connect(detail_tiling, "", detail_scaled_uv, "B")

    scroll_amount = create_expression(material, unreal.MaterialExpressionMultiply, -1780, 760)
    connect(time, "", scroll_amount, "A")
    connect(scroll_speed, "", scroll_amount, "B")

    zero = create_expression(material, unreal.MaterialExpressionConstant, -1780, 930)
    safe_set(zero, "r", 0.0)

    scroll_offset = create_expression(material, unreal.MaterialExpressionAppendVector, -1540, 850)
    connect(scroll_amount, "", scroll_offset, "A")
    connect(zero, "", scroll_offset, "B")

    detail_uv_add = create_expression(material, unreal.MaterialExpressionAdd, -1290, 430)
    connect(detail_scaled_uv, "", detail_uv_add, "A")
    connect(scroll_offset, "", detail_uv_add, "B")

    detail_uv_mul = create_expression(material, unreal.MaterialExpressionMultiply, -1050, 430)
    connect(detail_uv_add, "", detail_uv_mul, "A")
    connect(pixel_grid, "", detail_uv_mul, "B")

    detail_uv_floor = create_expression(material, unreal.MaterialExpressionFloor, -820, 430)
    connect(detail_uv_mul, "", detail_uv_floor, "")

    detail_uv = create_expression(material, unreal.MaterialExpressionDivide, -570, 430)
    connect(detail_uv_floor, "", detail_uv, "A")
    connect(pixel_grid, "", detail_uv, "B")

    detail_tex = create_expression(material, unreal.MaterialExpressionTextureSampleParameter2D, -320, 430)
    safe_set(detail_tex, "parameter_name", "DetailTexture")
    connect(detail_uv, "", detail_tex, "UVs")

    inner_mask = create_expression(material, unreal.MaterialExpressionSphereMask, -1020, -910)
    safe_set(inner_mask, "hardness_percent", 85.0)
    connect(mask_uv, "", inner_mask, "A")
    connect(center, "", inner_mask, "B")
    connect(inner_radius, "", inner_mask, "Radius")

    outer_mask = create_expression(material, unreal.MaterialExpressionSphereMask, -1020, -1200)
    safe_set(outer_mask, "hardness_percent", 82.0)
    connect(mask_uv, "", outer_mask, "A")
    connect(center, "", outer_mask, "B")
    connect(outer_radius, "", outer_mask, "Radius")

    outline_mask = create_expression(material, unreal.MaterialExpressionSubtract, -760, -1070)
    connect(outer_mask, "", outline_mask, "A")
    connect(inner_mask, "", outline_mask, "B")

    outline_sat = create_expression(material, unreal.MaterialExpressionSaturate, -520, -1070)
    connect(outline_mask, "", outline_sat, "")

    detail_plus = create_expression(material, unreal.MaterialExpressionAdd, 180, 260)
    connect(detail_tex, "R", detail_plus, "A")
    connect(detail_tex, "A", detail_plus, "B")

    detail_half = create_expression(material, unreal.MaterialExpressionConstant, 180, 470)
    safe_set(detail_half, "r", 0.5)

    detail_mask = create_expression(material, unreal.MaterialExpressionMultiply, 420, 320)
    connect(detail_plus, "", detail_mask, "A")
    connect(detail_half, "", detail_mask, "B")

    color_lerp = create_expression(material, unreal.MaterialExpressionLinearInterpolate, 700, 1820)
    connect(secondary_color, "", color_lerp, "A")
    connect(primary_color, "", color_lerp, "B")
    connect(detail_mask, "", color_lerp, "Alpha")

    tint_mul = create_expression(material, unreal.MaterialExpressionMultiply, 960, 1820)
    connect(color_lerp, "", tint_mul, "A")
    connect(tint_color, "", tint_mul, "B")

    detail_opacity_bias = create_expression(material, unreal.MaterialExpressionConstant, 700, 610)
    safe_set(detail_opacity_bias, "r", 0.45)

    detail_opacity_scale = create_expression(material, unreal.MaterialExpressionConstant, 700, 820)
    safe_set(detail_opacity_scale, "r", 0.55)

    detail_opacity_mul = create_expression(material, unreal.MaterialExpressionMultiply, 960, 730)
    connect(detail_mask, "", detail_opacity_mul, "A")
    connect(detail_opacity_scale, "", detail_opacity_mul, "B")

    detail_opacity = create_expression(material, unreal.MaterialExpressionAdd, 1200, 670)
    connect(detail_opacity_bias, "", detail_opacity, "A")
    connect(detail_opacity_mul, "", detail_opacity, "B")

    core_opacity = create_expression(material, unreal.MaterialExpressionMultiply, 1440, 520)
    connect(inner_mask, "", core_opacity, "A")
    connect(detail_opacity, "", core_opacity, "B")

    one_minus_age = create_expression(material, unreal.MaterialExpressionOneMinus, 420, 1100)
    connect(age, "", one_minus_age, "")

    opacity_pre = create_expression(material, unreal.MaterialExpressionAdd, 1700, 450)
    connect(core_opacity, "", opacity_pre, "A")
    connect(outline_sat, "", opacity_pre, "B")

    opacity_scaled = create_expression(material, unreal.MaterialExpressionMultiply, 1960, 450)
    connect(opacity_pre, "", opacity_scaled, "A")
    connect(opacity_boost, "", opacity_scaled, "B")

    final_opacity = create_expression(material, unreal.MaterialExpressionMultiply, 2220, 450)
    connect(opacity_scaled, "", final_opacity, "A")
    connect(one_minus_age, "", final_opacity, "B")

    glow_plus = create_expression(material, unreal.MaterialExpressionConstant, 960, 2130)
    safe_set(glow_plus, "r", 1.0)

    glow_scalar = create_expression(material, unreal.MaterialExpressionAdd, 1200, 2130)
    connect(glow_plus, "", glow_scalar, "A")
    connect(glow, "", glow_scalar, "B")

    emissive_base = create_expression(material, unreal.MaterialExpressionMultiply, 1460, 1910)
    connect(tint_mul, "", emissive_base, "A")
    connect(glow_scalar, "", emissive_base, "B")

    emissive_core = create_expression(material, unreal.MaterialExpressionMultiply, 1710, 1830)
    connect(emissive_base, "", emissive_core, "A")
    connect(core_opacity, "", emissive_core, "B")

    emissive_outline = create_expression(material, unreal.MaterialExpressionMultiply, 1460, 2230)
    connect(outline_color, "", emissive_outline, "A")
    connect(outline_sat, "", emissive_outline, "B")

    emissive_combined = create_expression(material, unreal.MaterialExpressionAdd, 1980, 1930)
    connect(emissive_core, "", emissive_combined, "A")
    connect(emissive_outline, "", emissive_combined, "B")

    final_emissive = create_expression(material, unreal.MaterialExpressionMultiply, 2240, 1930)
    connect(emissive_combined, "", final_emissive, "A")
    connect(one_minus_age, "", final_emissive, "B")

    mel.connect_material_property(final_emissive, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    mel.connect_material_property(final_opacity, "", unreal.MaterialProperty.MP_OPACITY)

    mel.layout_material_expressions(material)
    mel.recompile_material(material)
    unreal.EditorAssetLibrary.save_asset(MASTER_PATH)
    log(f"Built {MASTER_PATH}")
    return material


def build_material_instance(instance_path, detail_texture, vector_params, scalar_params):
    instance = recreate_asset(
        instance_path,
        unreal.MaterialInstanceConstant,
        unreal.MaterialInstanceConstantFactoryNew(),
    )

    master = unreal.EditorAssetLibrary.load_asset(MASTER_PATH)
    if not master:
        raise RuntimeError(f"Missing master material {MASTER_PATH}")

    safe_set(instance, "parent", master)
    mel.set_material_instance_texture_parameter_value(instance, "DetailTexture", detail_texture)

    for param_name, value in vector_params.items():
        mel.set_material_instance_vector_parameter_value(instance, param_name, value)

    for param_name, value in scalar_params.items():
        mel.set_material_instance_scalar_parameter_value(instance, param_name, float(value))

    mel.update_material_instance(instance)
    unreal.EditorAssetLibrary.save_asset(instance_path)
    log(f"Built {instance_path}")
    return instance


def main():
    ensure_directory(DEST_DIR)

    textures = {name: import_texture(name, source_path) for name, source_path in TEXTURE_SPECS.items()}

    build_master_material()

    gold_white = unreal.LinearColor(1.00, 0.95, 0.76, 1.0)
    warm_bronze = unreal.LinearColor(0.47, 0.28, 0.08, 1.0)
    ink_black = unreal.LinearColor(0.0, 0.0, 0.0, 1.0)
    pale_steel = unreal.LinearColor(0.90, 0.92, 1.00, 1.0)

    build_material_instance(
        STREAK_MI_PATH,
        textures["T_Hero1_Attack_Runes"],
        {
            "PrimaryColor": gold_white,
            "SecondaryColor": warm_bronze,
            "OutlineColor": ink_black,
            "TintColor": unreal.LinearColor(1.0, 1.0, 1.0, 1.0),
        },
        {
            "PixelGrid": 28.0,
            "DetailTiling": 4.0,
            "ScrollSpeed": 0.70,
            "InnerRadius": 0.40,
            "OuterRadius": 0.48,
            "GlowStrength": 4.5,
            "OpacityBoost": 0.90,
            "Age01": 0.0,
        },
    )

    build_material_instance(
        IMPACT_MI_PATH,
        textures["T_Hero1_Attack_Seals"],
        {
            "PrimaryColor": gold_white,
            "SecondaryColor": pale_steel,
            "OutlineColor": ink_black,
            "TintColor": unreal.LinearColor(1.0, 1.0, 1.0, 1.0),
        },
        {
            "PixelGrid": 24.0,
            "DetailTiling": 1.2,
            "ScrollSpeed": 0.10,
            "InnerRadius": 0.36,
            "OuterRadius": 0.47,
            "GlowStrength": 5.5,
            "OpacityBoost": 1.00,
            "Age01": 0.0,
        },
    )

    log("DONE")


if __name__ == "__main__":
    main()
