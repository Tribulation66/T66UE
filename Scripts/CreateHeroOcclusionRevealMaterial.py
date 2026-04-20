import unreal

LOG = '[HeroOcclusionReveal]'
ASSET_DIR = '/Game/Materials/PostProcess'
ASSET_NAME = 'M_HeroOcclusionReveal'
ASSET_PATH = f'{ASSET_DIR}/{ASSET_NAME}'
HERO_STENCIL_VALUE = 17

mel = unreal.MaterialEditingLibrary
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()


def log(message):
    unreal.log(f'{LOG} {message}')


def safe_set(obj, prop, value):
    try:
        obj.set_editor_property(prop, value)
    except Exception as exc:
        unreal.log_warning(f'{LOG} set_editor_property({prop}) failed: {exc}')


def ensure_dir(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def create_asset():
    if unreal.EditorAssetLibrary.does_asset_exist(ASSET_PATH):
        unreal.EditorAssetLibrary.delete_asset(ASSET_PATH)

    asset = asset_tools.create_asset(
        ASSET_NAME,
        ASSET_DIR,
        unreal.Material,
        unreal.MaterialFactoryNew(),
    )
    if not asset:
        raise RuntimeError(f'Failed to create {ASSET_PATH}')
    return asset


def add_expr(material, expr_class, x, y):
    return mel.create_material_expression(material, expr_class, x, y)


def build_material(material):
    safe_set(material, 'material_domain', unreal.MaterialDomain.MD_POST_PROCESS)

    scene_color = add_expr(material, unreal.MaterialExpressionSceneTexture, -1200, -220)
    safe_set(scene_color, 'scene_texture_id', unreal.SceneTextureId.PPI_POST_PROCESS_INPUT0)

    scene_color_rgb = add_expr(material, unreal.MaterialExpressionComponentMask, -980, -220)
    safe_set(scene_color_rgb, 'r', True)
    safe_set(scene_color_rgb, 'g', True)
    safe_set(scene_color_rgb, 'b', True)
    mel.connect_material_expressions(scene_color, '', scene_color_rgb, '')

    scene_depth = add_expr(material, unreal.MaterialExpressionSceneTexture, -1200, 20)
    safe_set(scene_depth, 'scene_texture_id', unreal.SceneTextureId.PPI_SCENE_DEPTH)

    custom_depth = add_expr(material, unreal.MaterialExpressionSceneTexture, -1200, 260)
    safe_set(custom_depth, 'scene_texture_id', unreal.SceneTextureId.PPI_CUSTOM_DEPTH)

    custom_stencil = add_expr(material, unreal.MaterialExpressionSceneTexture, -1200, 520)
    safe_set(custom_stencil, 'scene_texture_id', unreal.SceneTextureId.PPI_CUSTOM_STENCIL)

    scene_depth_r = add_expr(material, unreal.MaterialExpressionComponentMask, -980, 20)
    safe_set(scene_depth_r, 'r', True)
    mel.connect_material_expressions(scene_depth, 'Color', scene_depth_r, '')

    custom_depth_r = add_expr(material, unreal.MaterialExpressionComponentMask, -980, 260)
    safe_set(custom_depth_r, 'r', True)
    mel.connect_material_expressions(custom_depth, 'Color', custom_depth_r, '')

    custom_stencil_r = add_expr(material, unreal.MaterialExpressionComponentMask, -980, 520)
    safe_set(custom_stencil_r, 'r', True)
    mel.connect_material_expressions(custom_stencil, 'Color', custom_stencil_r, '')

    depth_delta = add_expr(material, unreal.MaterialExpressionSubtract, -740, 140)
    mel.connect_material_expressions(custom_depth_r, '', depth_delta, 'A')
    mel.connect_material_expressions(scene_depth_r, '', depth_delta, 'B')

    depth_bias = add_expr(material, unreal.MaterialExpressionScalarParameter, -980, 500)
    safe_set(depth_bias, 'parameter_name', 'DepthBias')
    safe_set(depth_bias, 'default_value', 2.0)

    bias_adjusted = add_expr(material, unreal.MaterialExpressionSubtract, -500, 180)
    mel.connect_material_expressions(depth_delta, '', bias_adjusted, 'A')
    mel.connect_material_expressions(depth_bias, '', bias_adjusted, 'B')

    depth_scale = add_expr(material, unreal.MaterialExpressionScalarParameter, -980, 700)
    safe_set(depth_scale, 'parameter_name', 'DepthScale')
    safe_set(depth_scale, 'default_value', 0.08)

    occlusion_strength = add_expr(material, unreal.MaterialExpressionMultiply, -260, 220)
    mel.connect_material_expressions(bias_adjusted, '', occlusion_strength, 'A')
    mel.connect_material_expressions(depth_scale, '', occlusion_strength, 'B')

    zero_value = add_expr(material, unreal.MaterialExpressionConstant, -20, 80)
    safe_set(zero_value, 'r', 0.0)

    one_value = add_expr(material, unreal.MaterialExpressionConstant, 220, 80)
    safe_set(one_value, 'r', 1.0)

    occlusion_clamped_min = add_expr(material, unreal.MaterialExpressionMax, -20, 220)
    mel.connect_material_expressions(occlusion_strength, '', occlusion_clamped_min, 'A')
    mel.connect_material_expressions(zero_value, '', occlusion_clamped_min, 'B')

    occlusion_mask = add_expr(material, unreal.MaterialExpressionMin, 220, 220)
    mel.connect_material_expressions(occlusion_clamped_min, '', occlusion_mask, 'A')
    mel.connect_material_expressions(one_value, '', occlusion_mask, 'B')

    stencil_value = add_expr(material, unreal.MaterialExpressionScalarParameter, -980, 980)
    safe_set(stencil_value, 'parameter_name', 'HeroStencilValue')
    safe_set(stencil_value, 'default_value', float(HERO_STENCIL_VALUE))

    stencil_delta = add_expr(material, unreal.MaterialExpressionSubtract, -740, 620)
    mel.connect_material_expressions(custom_stencil_r, '', stencil_delta, 'A')
    mel.connect_material_expressions(stencil_value, '', stencil_delta, 'B')

    negative_one = add_expr(material, unreal.MaterialExpressionConstant, -740, 760)
    safe_set(negative_one, 'r', -1.0)

    stencil_delta_neg = add_expr(material, unreal.MaterialExpressionMultiply, -500, 760)
    mel.connect_material_expressions(stencil_delta, '', stencil_delta_neg, 'A')
    mel.connect_material_expressions(negative_one, '', stencil_delta_neg, 'B')

    stencil_delta_abs = add_expr(material, unreal.MaterialExpressionMax, -260, 680)
    mel.connect_material_expressions(stencil_delta, '', stencil_delta_abs, 'A')
    mel.connect_material_expressions(stencil_delta_neg, '', stencil_delta_abs, 'B')

    stencil_compare_scale = add_expr(material, unreal.MaterialExpressionScalarParameter, -980, 1180)
    safe_set(stencil_compare_scale, 'parameter_name', 'StencilCompareScale')
    safe_set(stencil_compare_scale, 'default_value', 1000.0)

    stencil_delta_scaled = add_expr(material, unreal.MaterialExpressionMultiply, -20, 620)
    mel.connect_material_expressions(stencil_delta_abs, '', stencil_delta_scaled, 'A')
    mel.connect_material_expressions(stencil_compare_scale, '', stencil_delta_scaled, 'B')

    stencil_clamped_min = add_expr(material, unreal.MaterialExpressionMax, -20, 620)
    mel.connect_material_expressions(stencil_delta_scaled, '', stencil_clamped_min, 'A')
    mel.connect_material_expressions(zero_value, '', stencil_clamped_min, 'B')

    stencil_delta_clamped = add_expr(material, unreal.MaterialExpressionMin, 220, 620)
    mel.connect_material_expressions(stencil_clamped_min, '', stencil_delta_clamped, 'A')
    mel.connect_material_expressions(one_value, '', stencil_delta_clamped, 'B')

    stencil_mask = add_expr(material, unreal.MaterialExpressionSubtract, 460, 520)
    mel.connect_material_expressions(one_value, '', stencil_mask, 'A')
    mel.connect_material_expressions(stencil_delta_clamped, '', stencil_mask, 'B')

    reveal_opacity = add_expr(material, unreal.MaterialExpressionScalarParameter, -980, 900)
    safe_set(reveal_opacity, 'parameter_name', 'RevealOpacity')
    safe_set(reveal_opacity, 'default_value', 0.92)

    masked_occlusion = add_expr(material, unreal.MaterialExpressionMultiply, 700, 320)
    mel.connect_material_expressions(occlusion_mask, '', masked_occlusion, 'A')
    mel.connect_material_expressions(stencil_mask, '', masked_occlusion, 'B')

    blend_alpha = add_expr(material, unreal.MaterialExpressionMultiply, 940, 220)
    mel.connect_material_expressions(masked_occlusion, '', blend_alpha, 'A')
    mel.connect_material_expressions(reveal_opacity, '', blend_alpha, 'B')

    reveal_color = add_expr(material, unreal.MaterialExpressionVectorParameter, -980, -240)
    safe_set(reveal_color, 'parameter_name', 'RevealColor')
    safe_set(reveal_color, 'default_value', unreal.LinearColor(0.97, 0.80, 0.24, 1.0))

    reveal_color_rgb = add_expr(material, unreal.MaterialExpressionComponentMask, -740, -240)
    safe_set(reveal_color_rgb, 'r', True)
    safe_set(reveal_color_rgb, 'g', True)
    safe_set(reveal_color_rgb, 'b', True)
    mel.connect_material_expressions(reveal_color, '', reveal_color_rgb, '')

    final_color = add_expr(material, unreal.MaterialExpressionLinearInterpolate, 1180, -80)
    mel.connect_material_expressions(scene_color_rgb, '', final_color, 'A')
    mel.connect_material_expressions(reveal_color_rgb, '', final_color, 'B')
    mel.connect_material_expressions(blend_alpha, '', final_color, 'Alpha')

    mel.connect_material_property(final_color, '', unreal.MaterialProperty.MP_EMISSIVE_COLOR)


ensure_dir(ASSET_DIR)
material = create_asset()
build_material(material)
mel.layout_material_expressions(material)
mel.recompile_material(material)
unreal.EditorAssetLibrary.save_asset(ASSET_PATH)
log(f'Saved {ASSET_PATH}')
