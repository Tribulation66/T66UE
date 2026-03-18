import unreal

LOG = "[UIRetroBorder]"
ASSET_DIR = "/Game/UI/Materials"
BASE_NAME = "M_UI_RetroSkyBorderOverlay"
BASE_PATH = f"{ASSET_DIR}/{BASE_NAME}"
INSTANCE_SPECS = {
    "MI_UI_RetroSkyBorderOverlay_N": {
        "BorderColorA": unreal.LinearColor(0.45, 0.42, 0.78, 1.0),
        "BorderColorB": unreal.LinearColor(0.90, 0.80, 0.55, 1.0),
        "BorderThickness": 0.12,
        "BorderFeather": 0.018,
        "PatternScale": 7.0,
        "ScrollSpeed": 0.055,
        "BorderBoost": 1.0,
    },
    "MI_UI_RetroSkyBorderOverlay_H": {
        "BorderColorA": unreal.LinearColor(0.56, 0.52, 0.90, 1.0),
        "BorderColorB": unreal.LinearColor(1.00, 0.88, 0.63, 1.0),
        "BorderThickness": 0.125,
        "BorderFeather": 0.018,
        "PatternScale": 7.0,
        "ScrollSpeed": 0.075,
        "BorderBoost": 1.12,
    },
    "MI_UI_RetroSkyBorderOverlay_P": {
        "BorderColorA": unreal.LinearColor(0.32, 0.30, 0.60, 1.0),
        "BorderColorB": unreal.LinearColor(0.76, 0.66, 0.42, 1.0),
        "BorderThickness": 0.115,
        "BorderFeather": 0.018,
        "PatternScale": 7.0,
        "ScrollSpeed": 0.04,
        "BorderBoost": 0.88,
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


def build_material(material):
    safe_set(material, "material_domain", unreal.MaterialDomain.MD_UI)
    safe_set(material, "blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)
    safe_set(material, "two_sided", True)

    uv = create_expr(material, unreal.MaterialExpressionTextureCoordinate, -1800, -120)
    uv_r = component_mask(material, -1620, -250, r=True)
    uv_g = component_mask(material, -1620, 10, g=True)
    connect(uv, "", uv_r, "Input")
    connect(uv, "", uv_g, "Input")

    one_minus_u = create_expr(material, unreal.MaterialExpressionOneMinus, -1450, -250)
    one_minus_v = create_expr(material, unreal.MaterialExpressionOneMinus, -1450, 10)
    connect(uv_r, "", one_minus_u, "Input")
    connect(uv_g, "", one_minus_v, "Input")

    u_dist = create_expr(material, unreal.MaterialExpressionMin, -1280, -250)
    v_dist = create_expr(material, unreal.MaterialExpressionMin, -1280, 10)
    connect(uv_r, "", u_dist, "A")
    connect(one_minus_u, "", u_dist, "B")
    connect(uv_g, "", v_dist, "A")
    connect(one_minus_v, "", v_dist, "B")

    edge_dist = create_expr(material, unreal.MaterialExpressionMin, -1110, -120)
    connect(u_dist, "", edge_dist, "A")
    connect(v_dist, "", edge_dist, "B")

    border_thickness = scalar_param(material, "BorderThickness", 0.12, -1120, 170)
    border_feather = scalar_param(material, "BorderFeather", 0.018, -1120, 320)

    thickness_plus_feather = create_expr(material, unreal.MaterialExpressionAdd, -910, 250)
    connect(border_thickness, "", thickness_plus_feather, "A")
    connect(border_feather, "", thickness_plus_feather, "B")

    fade_numerator = create_expr(material, unreal.MaterialExpressionSubtract, -720, 80)
    connect(thickness_plus_feather, "", fade_numerator, "A")
    connect(edge_dist, "", fade_numerator, "B")

    fade_divide = create_expr(material, unreal.MaterialExpressionDivide, -520, 80)
    connect(fade_numerator, "", fade_divide, "A")
    connect(border_feather, "", fade_divide, "B")

    border_mask = create_expr(material, unreal.MaterialExpressionSaturate, -330, 80)
    connect(fade_divide, "", border_mask, "Input")

    time = create_expr(material, unreal.MaterialExpressionTime, -1620, 520)
    pattern_scale = scalar_param(material, "PatternScale", 7.0, -1620, 690)
    scroll_speed = scalar_param(material, "ScrollSpeed", 0.055, -1620, 840)

    scaled_uv = create_expr(material, unreal.MaterialExpressionMultiply, -1450, 690)
    connect(uv, "", scaled_uv, "A")
    connect(pattern_scale, "", scaled_uv, "B")

    speed_x = create_expr(material, unreal.MaterialExpressionMultiply, -1450, 840)
    connect(time, "", speed_x, "A")
    connect(scroll_speed, "", speed_x, "B")

    slow_factor = const(material, 0.37, -1450, 980)
    speed_y = create_expr(material, unreal.MaterialExpressionMultiply, -1280, 910)
    connect(speed_x, "", speed_y, "A")
    connect(slow_factor, "", speed_y, "B")

    animated_offset = create_expr(material, unreal.MaterialExpressionAppendVector, -1100, 860)
    connect(speed_x, "", animated_offset, "A")
    connect(speed_y, "", animated_offset, "B")

    animated_uv = create_expr(material, unreal.MaterialExpressionAdd, -910, 730)
    connect(scaled_uv, "", animated_uv, "A")
    connect(animated_offset, "", animated_uv, "B")

    anim_u = component_mask(material, -720, 630, r=True)
    anim_v = component_mask(material, -720, 820, g=True)
    connect(animated_uv, "", anim_u, "Input")
    connect(animated_uv, "", anim_v, "Input")

    c6 = const(material, 6.0, -540, 540)
    c2 = const(material, 2.0, -540, 660)
    wave_a_u = create_expr(material, unreal.MaterialExpressionMultiply, -360, 540)
    wave_a_v = create_expr(material, unreal.MaterialExpressionMultiply, -360, 660)
    connect(anim_u, "", wave_a_u, "A")
    connect(c6, "", wave_a_u, "B")
    connect(anim_v, "", wave_a_v, "A")
    connect(c2, "", wave_a_v, "B")

    wave_a_input = create_expr(material, unreal.MaterialExpressionAdd, -180, 600)
    connect(wave_a_u, "", wave_a_input, "A")
    connect(wave_a_v, "", wave_a_input, "B")
    wave_a = create_expr(material, unreal.MaterialExpressionSine, 20, 600)
    connect(wave_a_input, "", wave_a, "Input")

    c11 = const(material, 11.0, -540, 900)
    c5 = const(material, 5.0, -540, 1020)
    wave_b_u = create_expr(material, unreal.MaterialExpressionMultiply, -360, 900)
    wave_b_v = create_expr(material, unreal.MaterialExpressionMultiply, -360, 1020)
    connect(anim_u, "", wave_b_u, "A")
    connect(c11, "", wave_b_u, "B")
    connect(anim_v, "", wave_b_v, "A")
    connect(c5, "", wave_b_v, "B")

    wave_b_input = create_expr(material, unreal.MaterialExpressionSubtract, -180, 960)
    connect(wave_b_u, "", wave_b_input, "A")
    connect(wave_b_v, "", wave_b_input, "B")
    wave_b = create_expr(material, unreal.MaterialExpressionSine, 20, 960)
    connect(wave_b_input, "", wave_b, "Input")

    c13 = const(material, 13.0, -540, 1220)
    c07 = const(material, 0.7, -540, 1340)
    wave_c_v = create_expr(material, unreal.MaterialExpressionMultiply, -360, 1220)
    wave_c_a = create_expr(material, unreal.MaterialExpressionMultiply, -360, 1340)
    connect(anim_v, "", wave_c_v, "A")
    connect(c13, "", wave_c_v, "B")
    connect(wave_a, "", wave_c_a, "A")
    connect(c07, "", wave_c_a, "B")

    wave_c_input = create_expr(material, unreal.MaterialExpressionAdd, -180, 1280)
    connect(wave_c_v, "", wave_c_input, "A")
    connect(wave_c_a, "", wave_c_input, "B")
    wave_c = create_expr(material, unreal.MaterialExpressionSine, 20, 1280)
    connect(wave_c_input, "", wave_c, "Input")

    wave_sum = create_expr(material, unreal.MaterialExpressionAdd, 210, 860)
    wave_sum_2 = create_expr(material, unreal.MaterialExpressionAdd, 390, 1040)
    connect(wave_a, "", wave_sum, "A")
    connect(wave_b, "", wave_sum, "B")
    connect(wave_sum, "", wave_sum_2, "A")
    connect(wave_c, "", wave_sum_2, "B")

    one_third = const(material, 0.33333334, 210, 1220)
    half = const(material, 0.5, 210, 1360)
    avg = create_expr(material, unreal.MaterialExpressionMultiply, 580, 1040)
    connect(wave_sum_2, "", avg, "A")
    connect(one_third, "", avg, "B")

    centered = create_expr(material, unreal.MaterialExpressionMultiply, 760, 1040)
    connect(avg, "", centered, "A")
    connect(half, "", centered, "B")

    lifted = create_expr(material, unreal.MaterialExpressionAdd, 950, 1040)
    connect(centered, "", lifted, "A")
    connect(half, "", lifted, "B")

    quantize_steps = const(material, 4.0, 950, 1220)
    quantized_scale = create_expr(material, unreal.MaterialExpressionMultiply, 1140, 1040)
    connect(lifted, "", quantized_scale, "A")
    connect(quantize_steps, "", quantized_scale, "B")

    quantized_floor = create_expr(material, unreal.MaterialExpressionFloor, 1320, 1040)
    connect(quantized_scale, "", quantized_floor, "Input")

    quantized = create_expr(material, unreal.MaterialExpressionMultiply, 1490, 1040)
    connect(quantized_floor, "", quantized, "A")
    connect(one_third, "", quantized, "B")

    color_a = vector_param(material, "BorderColorA", unreal.LinearColor(0.45, 0.42, 0.78, 1.0), 1140, 1340)
    color_b = vector_param(material, "BorderColorB", unreal.LinearColor(0.90, 0.80, 0.55, 1.0), 1140, 1490)
    border_boost = scalar_param(material, "BorderBoost", 1.0, 1320, 1790)

    pattern_color = create_expr(material, unreal.MaterialExpressionLinearInterpolate, 1690, 1210)
    connect(color_a, "", pattern_color, "A")
    connect(color_b, "", pattern_color, "B")
    connect(quantized, "", pattern_color, "Alpha")

    boosted_pattern = create_expr(material, unreal.MaterialExpressionMultiply, 1880, 1210)
    connect(pattern_color, "", boosted_pattern, "A")
    connect(border_boost, "", boosted_pattern, "B")

    final_color = create_expr(material, unreal.MaterialExpressionMultiply, 2070, 980)
    connect(boosted_pattern, "", final_color, "A")
    connect(border_mask, "", final_color, "B")

    opacity = border_mask

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
