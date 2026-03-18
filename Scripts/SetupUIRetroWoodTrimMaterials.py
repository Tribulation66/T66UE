import unreal

LOG = "[UIRetroWoodTrim]"
ASSET_DIR = "/Game/UI/Materials"
BASE_NAME = "M_UI_RetroWoodTrimV2"
BASE_PATH = f"{ASSET_DIR}/{BASE_NAME}"
INSTANCE_SPECS = {
    "MI_UI_RetroWoodTrimV2_N": {
        "Color1": unreal.LinearColor(0.95, 0.82, 0.24, 1.0),
        "Color2": unreal.LinearColor(0.32, 0.58, 0.98, 1.0),
        "Color3": unreal.LinearColor(0.07, 0.17, 0.46, 1.0),
        "BandScale": 24.0,
        "BandSmoothness": 0.10,
        "WarpScale": 7.8,
        "WarpStrength": 0.62,
        "SpotAmount": 0.28,
        "SpotSize": 13.0,
        "RimStrength": 0.82,
        "RimWidth": 0.16,
        "Brightness": 1.0,
    },
    "MI_UI_RetroWoodTrimV2_H": {
        "Color1": unreal.LinearColor(1.00, 0.88, 0.36, 1.0),
        "Color2": unreal.LinearColor(0.46, 0.70, 1.00, 1.0),
        "Color3": unreal.LinearColor(0.10, 0.22, 0.54, 1.0),
        "BandScale": 25.0,
        "BandSmoothness": 0.09,
        "WarpScale": 8.2,
        "WarpStrength": 0.68,
        "SpotAmount": 0.30,
        "SpotSize": 13.0,
        "RimStrength": 0.76,
        "RimWidth": 0.15,
        "Brightness": 1.12,
    },
    "MI_UI_RetroWoodTrimV2_P": {
        "Color1": unreal.LinearColor(0.82, 0.68, 0.18, 1.0),
        "Color2": unreal.LinearColor(0.22, 0.44, 0.82, 1.0),
        "Color3": unreal.LinearColor(0.05, 0.12, 0.34, 1.0),
        "BandScale": 22.5,
        "BandSmoothness": 0.11,
        "WarpScale": 7.0,
        "WarpStrength": 0.54,
        "SpotAmount": 0.24,
        "SpotSize": 12.0,
        "RimStrength": 0.92,
        "RimWidth": 0.17,
        "Brightness": 0.92,
    },
}

mel = unreal.MaterialEditingLibrary


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


def ensure_directory():
    if not unreal.EditorAssetLibrary.does_directory_exist(ASSET_DIR):
        unreal.EditorAssetLibrary.make_directory(ASSET_DIR)


def delete_if_exists(asset_path):
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        if not unreal.EditorAssetLibrary.delete_asset(asset_path):
            raise RuntimeError(f"Failed to delete existing asset {asset_path}")


def create_material():
    delete_if_exists(BASE_PATH)
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    material = asset_tools.create_asset(BASE_NAME, ASSET_DIR, unreal.Material, unreal.MaterialFactoryNew())
    if not material:
        raise RuntimeError(f"Failed to create material {BASE_PATH}")
    return material


def create_expr(material, cls, x, y):
    return mel.create_material_expression(material, cls, x, y)


def scalar_param(material, name, default_value, x, y):
    expr = create_expr(material, unreal.MaterialExpressionScalarParameter, x, y)
    safe_set(expr, "parameter_name", name)
    safe_set(expr, "default_value", default_value)
    return expr


def vector_param(material, name, default_value, x, y):
    expr = create_expr(material, unreal.MaterialExpressionVectorParameter, x, y)
    safe_set(expr, "parameter_name", name)
    safe_set(expr, "default_value", default_value)
    return expr


def const(material, value, x, y):
    expr = create_expr(material, unreal.MaterialExpressionConstant, x, y)
    safe_set(expr, "r", value)
    return expr


def component_mask(material, x, y, *, r=False, g=False, b=False, a=False):
    expr = create_expr(material, unreal.MaterialExpressionComponentMask, x, y)
    safe_set(expr, "r", r)
    safe_set(expr, "g", g)
    safe_set(expr, "b", b)
    safe_set(expr, "a", a)
    return expr


def build_soft_step(material, value, edge_center, softness, x, y):
    edge_min = create_expr(material, unreal.MaterialExpressionSubtract, x, y - 120)
    edge_max = create_expr(material, unreal.MaterialExpressionAdd, x, y + 20)
    span = create_expr(material, unreal.MaterialExpressionSubtract, x + 200, y + 20)
    shifted = create_expr(material, unreal.MaterialExpressionSubtract, x + 200, y - 120)
    divide = create_expr(material, unreal.MaterialExpressionDivide, x + 390, y - 50)
    saturate = create_expr(material, unreal.MaterialExpressionSaturate, x + 560, y - 50)

    connect(edge_center, "", edge_min, "A")
    connect(softness, "", edge_min, "B")
    connect(edge_center, "", edge_max, "A")
    connect(softness, "", edge_max, "B")
    connect(edge_max, "", span, "A")
    connect(edge_min, "", span, "B")
    connect(value, "", shifted, "A")
    connect(edge_min, "", shifted, "B")
    connect(shifted, "", divide, "A")
    connect(span, "", divide, "B")
    connect(divide, "", saturate, "Input")
    return saturate


def build_material(material):
    safe_set(material, "material_domain", unreal.MaterialDomain.MD_UI)
    safe_set(material, "blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)
    safe_set(material, "two_sided", True)

    uv = create_expr(material, unreal.MaterialExpressionTextureCoordinate, -1760, 0)
    uv_r = component_mask(material, -1560, -120, r=True)
    uv_g = component_mask(material, -1560, 120, g=True)
    connect(uv, "", uv_r, "Input")
    connect(uv, "", uv_g, "Input")

    band_scale = scalar_param(material, "BandScale", 24.0, -1560, 420)
    warp_scale = scalar_param(material, "WarpScale", 7.8, -1560, 580)
    warp_strength = scalar_param(material, "WarpStrength", 0.62, -1560, 740)
    band_smoothness = scalar_param(material, "BandSmoothness", 0.10, -1560, 900)
    spot_size = scalar_param(material, "SpotSize", 13.0, -1560, 1060)
    spot_amount = scalar_param(material, "SpotAmount", 0.28, -1560, 1220)
    brightness = scalar_param(material, "Brightness", 1.0, -1560, 1380)

    axis_major = create_expr(material, unreal.MaterialExpressionAdd, -1320, 20)
    axis_minor = create_expr(material, unreal.MaterialExpressionMultiply, -1320, 180)
    minor_weight = const(material, 0.65, -1500, 260)
    connect(uv_r, "", axis_major, "A")
    connect(uv_g, "", axis_minor, "A")
    connect(minor_weight, "", axis_minor, "B")
    connect(axis_minor, "", axis_major, "B")

    warp_u = create_expr(material, unreal.MaterialExpressionMultiply, -1320, 580)
    connect(uv_r, "", warp_u, "A")
    connect(warp_scale, "", warp_u, "B")

    warp_u_sin = create_expr(material, unreal.MaterialExpressionSine, -1120, 580)
    connect(warp_u, "", warp_u_sin, "Input")

    warp_v = create_expr(material, unreal.MaterialExpressionMultiply, -1320, 760)
    connect(uv_g, "", warp_v, "A")
    connect(warp_scale, "", warp_v, "B")

    warp_v_scale = const(material, 1.6, -1500, 840)
    warp_v_weighted = create_expr(material, unreal.MaterialExpressionMultiply, -1120, 760)
    connect(warp_v, "", warp_v_weighted, "A")
    connect(warp_v_scale, "", warp_v_weighted, "B")

    warp_v_sin = create_expr(material, unreal.MaterialExpressionSine, -920, 760)
    connect(warp_v_weighted, "", warp_v_sin, "Input")

    warp_sum = create_expr(material, unreal.MaterialExpressionAdd, -720, 650)
    connect(warp_u_sin, "", warp_sum, "A")
    connect(warp_v_sin, "", warp_sum, "B")

    warp_term = create_expr(material, unreal.MaterialExpressionMultiply, -520, 650)
    connect(warp_sum, "", warp_term, "A")
    connect(warp_strength, "", warp_term, "B")

    band_coord = create_expr(material, unreal.MaterialExpressionMultiply, -720, 120)
    connect(axis_major, "", band_coord, "A")
    connect(band_scale, "", band_coord, "B")

    band_input = create_expr(material, unreal.MaterialExpressionAdd, -320, 260)
    connect(band_coord, "", band_input, "A")
    connect(warp_term, "", band_input, "B")

    band_sin = create_expr(material, unreal.MaterialExpressionSine, -120, 260)
    connect(band_input, "", band_sin, "Input")

    half = const(material, 0.5, -300, 520)
    band_scaled = create_expr(material, unreal.MaterialExpressionMultiply, 80, 200)
    band_norm = create_expr(material, unreal.MaterialExpressionAdd, 280, 200)
    connect(band_sin, "", band_scaled, "A")
    connect(half, "", band_scaled, "B")
    connect(band_scaled, "", band_norm, "A")
    connect(half, "", band_norm, "B")

    threshold_a = const(material, 0.32, 80, 540)
    threshold_b = const(material, 0.68, 80, 700)
    step_a = build_soft_step(material, band_norm, threshold_a, band_smoothness, 300, 620)
    step_b = build_soft_step(material, band_norm, threshold_b, band_smoothness, 300, 900)

    color1 = vector_param(material, "Color1", unreal.LinearColor(0.95, 0.82, 0.24, 1.0), 980, 420)
    color2 = vector_param(material, "Color2", unreal.LinearColor(0.32, 0.58, 0.98, 1.0), 980, 600)
    color3 = vector_param(material, "Color3", unreal.LinearColor(0.07, 0.17, 0.46, 1.0), 980, 780)

    mix_mid = create_expr(material, unreal.MaterialExpressionLinearInterpolate, 1210, 640)
    connect(color3, "", mix_mid, "A")
    connect(color2, "", mix_mid, "B")
    connect(step_a, "", mix_mid, "Alpha")

    base_color = create_expr(material, unreal.MaterialExpressionLinearInterpolate, 1430, 540)
    connect(mix_mid, "", base_color, "A")
    connect(color1, "", base_color, "B")
    connect(step_b, "", base_color, "Alpha")

    spot_major = create_expr(material, unreal.MaterialExpressionMultiply, 980, 1020)
    connect(axis_major, "", spot_major, "A")
    connect(spot_size, "", spot_major, "B")

    spot_minor = create_expr(material, unreal.MaterialExpressionMultiply, 980, 1180)
    connect(uv_g, "", spot_minor, "A")
    connect(spot_size, "", spot_minor, "B")

    spot_minor_weight = const(material, 1.9, 1210, 1180)
    spot_minor_scaled = create_expr(material, unreal.MaterialExpressionMultiply, 1430, 1180)
    connect(spot_minor, "", spot_minor_scaled, "A")
    connect(spot_minor_weight, "", spot_minor_scaled, "B")

    spot_a_input = create_expr(material, unreal.MaterialExpressionAdd, 1650, 1080)
    connect(spot_major, "", spot_a_input, "A")
    connect(spot_minor_scaled, "", spot_a_input, "B")

    spot_a = create_expr(material, unreal.MaterialExpressionSine, 1860, 1080)
    connect(spot_a_input, "", spot_a, "Input")

    spot_b_weight = const(material, 2.6, 1210, 1360)
    spot_major_scaled = create_expr(material, unreal.MaterialExpressionMultiply, 1430, 1360)
    connect(spot_major, "", spot_major_scaled, "A")
    connect(spot_b_weight, "", spot_major_scaled, "B")

    spot_b_input = create_expr(material, unreal.MaterialExpressionSubtract, 1650, 1260)
    connect(spot_minor, "", spot_b_input, "A")
    connect(spot_major_scaled, "", spot_b_input, "B")

    spot_b = create_expr(material, unreal.MaterialExpressionSine, 1860, 1260)
    connect(spot_b_input, "", spot_b, "Input")

    spot_sum = create_expr(material, unreal.MaterialExpressionAdd, 2060, 1170)
    connect(spot_a, "", spot_sum, "A")
    connect(spot_b, "", spot_sum, "B")

    quarter = const(material, 0.25, 2060, 1340)
    spot_scaled = create_expr(material, unreal.MaterialExpressionMultiply, 2260, 1170)
    connect(spot_sum, "", spot_scaled, "A")
    connect(quarter, "", spot_scaled, "B")

    spot_norm = create_expr(material, unreal.MaterialExpressionAdd, 2460, 1170)
    connect(spot_scaled, "", spot_norm, "A")
    connect(half, "", spot_norm, "B")

    spot_threshold = const(material, 0.72, 2260, 1420)
    spot_soft = const(material, 0.07, 2260, 1560)
    spot_mask = build_soft_step(material, spot_norm, spot_threshold, spot_soft, 2480, 1490)

    spot_mix = create_expr(material, unreal.MaterialExpressionMultiply, 3060, 1340)
    connect(spot_mask, "", spot_mix, "A")
    connect(spot_amount, "", spot_mix, "B")

    accent_color = create_expr(material, unreal.MaterialExpressionLinearInterpolate, 2860, 1080)
    accent_half = const(material, 0.5, 2640, 980)
    connect(color2, "", accent_color, "A")
    connect(color1, "", accent_color, "B")
    connect(accent_half, "", accent_color, "Alpha")

    color_with_spots = create_expr(material, unreal.MaterialExpressionLinearInterpolate, 3260, 1170)
    connect(base_color, "", color_with_spots, "A")
    connect(accent_color, "", color_with_spots, "B")
    connect(spot_mix, "", color_with_spots, "Alpha")

    one_minus_u = create_expr(material, unreal.MaterialExpressionOneMinus, 2860, 420)
    one_minus_v = create_expr(material, unreal.MaterialExpressionOneMinus, 2860, 560)
    connect(uv_r, "", one_minus_u, "Input")
    connect(uv_g, "", one_minus_v, "Input")

    edge_u = create_expr(material, unreal.MaterialExpressionMin, 3060, 420)
    edge_v = create_expr(material, unreal.MaterialExpressionMin, 3060, 560)
    connect(uv_r, "", edge_u, "A")
    connect(one_minus_u, "", edge_u, "B")
    connect(uv_g, "", edge_v, "A")
    connect(one_minus_v, "", edge_v, "B")

    edge_dist = create_expr(material, unreal.MaterialExpressionMin, 3260, 490)
    connect(edge_u, "", edge_dist, "A")
    connect(edge_v, "", edge_dist, "B")

    rim_width = scalar_param(material, "RimWidth", 0.16, 3460, 420)
    rim_strength = scalar_param(material, "RimStrength", 0.82, 3460, 560)

    rim_divide = create_expr(material, unreal.MaterialExpressionDivide, 3680, 450)
    connect(edge_dist, "", rim_divide, "A")
    connect(rim_width, "", rim_divide, "B")

    rim_saturate = create_expr(material, unreal.MaterialExpressionSaturate, 3880, 450)
    connect(rim_divide, "", rim_saturate, "Input")

    rim_mask = create_expr(material, unreal.MaterialExpressionOneMinus, 4080, 450)
    connect(rim_saturate, "", rim_mask, "Input")

    rim_term = create_expr(material, unreal.MaterialExpressionMultiply, 4280, 450)
    connect(rim_mask, "", rim_term, "A")
    connect(rim_strength, "", rim_term, "B")

    rim_darkness = const(material, 0.55, 4280, 640)
    rim_drop = create_expr(material, unreal.MaterialExpressionMultiply, 4480, 520)
    connect(rim_term, "", rim_drop, "A")
    connect(rim_darkness, "", rim_drop, "B")

    one = const(material, 1.0, 4480, 700)
    rim_multiplier = create_expr(material, unreal.MaterialExpressionSubtract, 4680, 610)
    connect(one, "", rim_multiplier, "A")
    connect(rim_drop, "", rim_multiplier, "B")

    darkened = create_expr(material, unreal.MaterialExpressionMultiply, 4880, 980)
    connect(color_with_spots, "", darkened, "A")
    connect(rim_multiplier, "", darkened, "B")

    final_color = create_expr(material, unreal.MaterialExpressionMultiply, 5100, 980)
    connect(darkened, "", final_color, "A")
    connect(brightness, "", final_color, "B")

    opacity = const(material, 1.0, 5100, 1180)

    mel.connect_material_property(final_color, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    mel.connect_material_property(opacity, "", unreal.MaterialProperty.MP_OPACITY)

    mel.layout_material_expressions(material)
    mel.recompile_material(material)
    unreal.EditorAssetLibrary.save_asset(BASE_PATH)


def create_material_instance(material, asset_name, values):
    asset_path = f"{ASSET_DIR}/{asset_name}"
    delete_if_exists(asset_path)

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    instance = asset_tools.create_asset(asset_name, ASSET_DIR, unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
    if not instance:
        raise RuntimeError(f"Failed to create material instance {asset_path}")

    safe_set(instance, "parent", material)

    for param_name, value in values.items():
        if isinstance(value, unreal.LinearColor):
            mel.set_material_instance_vector_parameter_value(instance, param_name, value)
        else:
            mel.set_material_instance_scalar_parameter_value(instance, param_name, float(value))

    unreal.EditorAssetLibrary.save_asset(asset_path)


def main():
    ensure_directory()
    material = create_material()
    build_material(material)

    for asset_name, values in INSTANCE_SPECS.items():
        create_material_instance(material, asset_name, values)

    log(f"Created {BASE_PATH} and {len(INSTANCE_SPECS)} material instances")


if __name__ == "__main__":
    main()
