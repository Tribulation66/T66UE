import unreal

LOG = '[RetroGeometryVariants]'
RETRO_DIR = '/Game/Materials/Retro'
MPC_PATH = RETRO_DIR + '/MPC_T66_RetroGeometry'

VERTEX_SNAP_FN = '/Game/UE5RFX/Materials/MaterialFunctions/UE5RFX_WPO_VertexSnapping'
VERTEX_NOISE_FN = '/Game/UE5RFX/Materials/MaterialFunctions/UE5RFX_WPO_VertexNoise'
AFFINE_FN = '/Game/UE5RFX/Materials/MaterialFunctions/UE5RFX_UV_AffineMapping'

MATERIAL_SPECS = [
    {
        'path': RETRO_DIR + '/M_Character_Unlit_RetroGeometry',
        'asset_name': 'M_Character_Unlit_RetroGeometry',
        'group': 'Character',
        'builder': 'character',
    },
    {
        'path': RETRO_DIR + '/M_Environment_Unlit_RetroGeometry',
        'asset_name': 'M_Environment_Unlit_RetroGeometry',
        'group': 'World',
        'builder': 'environment',
    },
    {
        'path': RETRO_DIR + '/M_FBX_Unlit_RetroGeometry',
        'asset_name': 'M_FBX_Unlit_RetroGeometry',
        'group': 'Character',
        'builder': 'fbx',
    },
    {
        'path': RETRO_DIR + '/M_GLB_Unlit_RetroGeometry',
        'asset_name': 'M_GLB_Unlit_RetroGeometry',
        'group': 'World',
        'builder': 'glb',
    },
]

mel = unreal.MaterialEditingLibrary
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()


def log(text):
    unreal.log(f'{LOG} {text}')


def ensure_directory(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def load_asset(path):
    asset = unreal.EditorAssetLibrary.load_asset(path)
    if not asset:
        raise RuntimeError(f'Missing asset: {path}')
    return asset


def safe_set(obj, prop, value):
    try:
        obj.set_editor_property(prop, value)
    except Exception as exc:
        unreal.log_warning(f'{LOG} set_editor_property({prop}) failed: {exc}')


def recreate_material(asset_name):
    asset_path = f'{RETRO_DIR}/{asset_name}'
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        unreal.EditorAssetLibrary.delete_asset(asset_path)
    factory = unreal.MaterialFactoryNew()
    material = asset_tools.create_asset(asset_name, RETRO_DIR, unreal.Material, factory)
    if not material:
        raise RuntimeError(f'Failed to create {asset_path}')
    return material, asset_path


def save_material(material, asset_path):
    mel.layout_material_expressions(material)
    mel.recompile_material(material)
    unreal.EditorAssetLibrary.save_asset(asset_path)
    log(f'Saved {asset_path}')


def create_asset_if_missing(asset_path, asset_class, factory):
    asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    if asset:
        return asset

    package_path, asset_name = asset_path.rsplit('/', 1)
    ensure_directory(package_path)
    asset = asset_tools.create_asset(asset_name, package_path, asset_class, factory)
    if not asset:
        raise RuntimeError(f'Failed to create asset: {asset_path}')
    return asset


def make_scalar_collection_param(name, default_value):
    param = unreal.CollectionScalarParameter()
    param.set_editor_property('parameter_name', name)
    param.set_editor_property('default_value', float(default_value))
    return param


def make_vector_collection_param(name, default_value):
    param = unreal.CollectionVectorParameter()
    param.set_editor_property('parameter_name', name)
    param.set_editor_property('default_value', unreal.LinearColor(*default_value))
    return param


def ensure_geometry_collection():
    existing = unreal.EditorAssetLibrary.load_asset(MPC_PATH)
    if existing:
        log(f'Using existing collection {existing.get_path_name()}')
        return existing

    collection = create_asset_if_missing(MPC_PATH, unreal.MaterialParameterCollection, unreal.MaterialParameterCollectionFactoryNew())

    collection.set_editor_property('scalar_parameters', [
        make_scalar_collection_param('WorldVertexSnapStrength', 0.0),
        make_scalar_collection_param('WorldVertexSnapTargetResolution', 600.0),
        make_scalar_collection_param('WorldAffineBlend', 0.0),
        make_scalar_collection_param('WorldAffineDistance1', 400.0),
        make_scalar_collection_param('WorldAffineDistance2', 1200.0),
        make_scalar_collection_param('WorldAffineDistance3', 1800.0),
        make_scalar_collection_param('CharacterVertexSnapStrength', 0.0),
        make_scalar_collection_param('CharacterVertexSnapTargetResolution', 600.0),
        make_scalar_collection_param('CharacterAffineBlend', 0.0),
        make_scalar_collection_param('CharacterAffineDistance1', 400.0),
        make_scalar_collection_param('CharacterAffineDistance2', 1200.0),
        make_scalar_collection_param('CharacterAffineDistance3', 1800.0),
    ])
    collection.set_editor_property('vector_parameters', [
        make_vector_collection_param('WorldVertexNoiseOffset', (0.0, 0.0, 0.0, 0.0)),
        make_vector_collection_param('CharacterVertexNoiseOffset', (0.0, 0.0, 0.0, 0.0)),
    ])
    unreal.EditorAssetLibrary.save_loaded_asset(collection)
    log(f'Prepared collection {collection.get_path_name()}')
    return collection


def create_texcoord(material, x, y):
    return mel.create_material_expression(material, unreal.MaterialExpressionTextureCoordinate, x, y)


def create_collection_parameter(material, collection, parameter_name, x, y):
    expr = mel.create_material_expression(material, unreal.MaterialExpressionCollectionParameter, x, y)
    safe_set(expr, 'collection', collection)
    safe_set(expr, 'parameter_name', parameter_name)
    return expr


def create_scalar_parameter(material, parameter_name, default_value, x, y):
    expr = mel.create_material_expression(material, unreal.MaterialExpressionScalarParameter, x, y)
    safe_set(expr, 'parameter_name', parameter_name)
    safe_set(expr, 'default_value', float(default_value))
    return expr


def create_vector_parameter(material, parameter_name, default_value, x, y):
    expr = mel.create_material_expression(material, unreal.MaterialExpressionVectorParameter, x, y)
    safe_set(expr, 'parameter_name', parameter_name)
    safe_set(expr, 'default_value', unreal.LinearColor(*default_value))
    return expr


def create_texture_parameter(material, parameter_name, x, y):
    expr = mel.create_material_expression(material, unreal.MaterialExpressionTextureSampleParameter2D, x, y)
    safe_set(expr, 'parameter_name', parameter_name)
    return expr


def create_material_function_call(material, function_path, x, y):
    expr = mel.create_material_expression(material, unreal.MaterialExpressionMaterialFunctionCall, x, y)
    safe_set(expr, 'material_function', load_asset(function_path))
    return expr


def create_multiply(material, x, y):
    return mel.create_material_expression(material, unreal.MaterialExpressionMultiply, x, y)


def create_lerp(material, x, y):
    return mel.create_material_expression(material, unreal.MaterialExpressionLinearInterpolate, x, y)


def create_constant3(material, value, x, y):
    expr = mel.create_material_expression(material, unreal.MaterialExpressionConstant3Vector, x, y)
    safe_set(expr, 'constant', unreal.LinearColor(value[0], value[1], value[2], 0.0))
    return expr


def connect(source, source_output, target, target_input):
    mel.connect_material_expressions(source, source_output, target, target_input)


def build_affine_uv(material, collection, group_prefix, y):
    texcoord = create_texcoord(material, -2100, y)
    dist1 = create_collection_parameter(material, collection, f'{group_prefix}AffineDistance1', -2100, y - 140)
    dist2 = create_collection_parameter(material, collection, f'{group_prefix}AffineDistance2', -2100, y - 20)
    dist3 = create_collection_parameter(material, collection, f'{group_prefix}AffineDistance3', -2100, y + 100)
    blend = create_collection_parameter(material, collection, f'{group_prefix}AffineBlend', -2100, y + 240)

    affine = create_material_function_call(material, AFFINE_FN, -1500, y)
    connect(texcoord, '', affine, 'TextureCoordinate')
    connect(dist1, '', affine, 'Distance 1')
    connect(dist2, '', affine, 'Distance 2')
    connect(dist3, '', affine, 'Distance 3')

    lerp = create_lerp(material, -950, y)
    connect(texcoord, '', lerp, 'A')
    connect(affine, '', lerp, 'B')
    connect(blend, '', lerp, 'Alpha')
    return lerp


def build_geometry_wpo(material, collection, group_prefix):
    zero = create_constant3(material, (0.0, 0.0, 0.0), -2100, 320)
    snap_strength = create_collection_parameter(material, collection, f'{group_prefix}VertexSnapStrength', -2100, 120)
    snap_target = create_collection_parameter(material, collection, f'{group_prefix}VertexSnapTargetResolution', -2100, 220)
    noise_offset = create_collection_parameter(material, collection, f'{group_prefix}VertexNoiseOffset', -2100, 460)

    vertex_snap = create_material_function_call(material, VERTEX_SNAP_FN, -1500, 240)
    connect(zero, '', vertex_snap, 'World Position Offset')
    connect(snap_strength, '', vertex_snap, 'Strength Multiplier')
    connect(snap_target, '', vertex_snap, 'TargetResolution')

    vertex_noise = create_material_function_call(material, VERTEX_NOISE_FN, -950, 320)
    connect(vertex_snap, '', vertex_noise, 'World Position Offset')
    connect(noise_offset, '', vertex_noise, 'Noise Offset')

    mel.connect_material_property(vertex_noise, '', unreal.MaterialProperty.MP_WORLD_POSITION_OFFSET)


def build_character_material(material, collection, group_prefix):
    uv = build_affine_uv(material, collection, group_prefix, -120)
    texture = create_texture_parameter(material, 'DiffuseColorMap', -350, -120)
    brightness = create_scalar_parameter(material, 'Brightness', 1.0, -350, 100)
    multiply = create_multiply(material, -20, -80)

    connect(uv, '', texture, 'UVs')
    connect(texture, 'RGB', multiply, 'A')
    connect(brightness, '', multiply, 'B')
    mel.connect_material_property(multiply, '', unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    build_geometry_wpo(material, collection, group_prefix)


def build_environment_material(material, collection, group_prefix):
    uv = build_affine_uv(material, collection, group_prefix, -140)
    texture = create_texture_parameter(material, 'DiffuseColorMap', -350, -140)
    brightness = create_scalar_parameter(material, 'Brightness', 1.0, -350, 100)
    tint = create_vector_parameter(material, 'Tint', (1.0, 1.0, 1.0, 1.0), -350, 300)
    bright_mul = create_multiply(material, -40, -100)
    tint_mul = create_multiply(material, 240, -100)

    connect(uv, '', texture, 'UVs')
    connect(texture, 'RGB', bright_mul, 'A')
    connect(brightness, '', bright_mul, 'B')
    connect(bright_mul, '', tint_mul, 'A')
    connect(tint, '', tint_mul, 'B')
    mel.connect_material_property(tint_mul, '', unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    build_geometry_wpo(material, collection, group_prefix)


def build_fbx_material(material, collection, group_prefix):
    uv = build_affine_uv(material, collection, group_prefix, -120)
    texture = create_texture_parameter(material, 'DiffuseColorMap', -350, -120)
    connect(uv, '', texture, 'UVs')
    mel.connect_material_property(texture, 'RGB', unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    build_geometry_wpo(material, collection, group_prefix)


def build_glb_material(material, collection, group_prefix):
    uv = build_affine_uv(material, collection, group_prefix, -120)
    texture = create_texture_parameter(material, 'BaseColorTexture', -350, -120)
    brightness = create_scalar_parameter(material, 'Brightness', 1.0, -350, 100)
    tint = create_vector_parameter(material, 'Tint', (1.0, 1.0, 1.0, 1.0), -350, 300)
    bright_mul = create_multiply(material, -40, -100)
    tint_mul = create_multiply(material, 240, -100)
    connect(uv, '', texture, 'UVs')
    connect(texture, 'RGB', bright_mul, 'A')
    connect(brightness, '', bright_mul, 'B')
    connect(bright_mul, '', tint_mul, 'A')
    connect(tint, '', tint_mul, 'B')
    mel.connect_material_property(tint_mul, '', unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    build_geometry_wpo(material, collection, group_prefix)


def rebuild_material(spec, collection):
    material, asset_path = recreate_material(spec['asset_name'])
    log(f'Rebuilding {asset_path}')
    safe_set(material, 'shading_model', unreal.MaterialShadingModel.MSM_UNLIT)
    safe_set(material, 'blend_mode', unreal.BlendMode.BLEND_OPAQUE)
    safe_set(material, 'two_sided', True)
    safe_set(material, 'use_material_attributes', False)
    safe_set(material, 'used_with_skeletal_mesh', spec['builder'] in ('character', 'fbx'))
    safe_set(material, 'used_with_instanced_static_meshes', spec['builder'] in ('environment', 'glb'))
    safe_set(material, 'used_with_nanite', spec['builder'] == 'glb')

    if spec['builder'] == 'character':
        build_character_material(material, collection, spec['group'])
    elif spec['builder'] == 'environment':
        build_environment_material(material, collection, spec['group'])
    elif spec['builder'] == 'fbx':
        build_fbx_material(material, collection, spec['group'])
    elif spec['builder'] == 'glb':
        build_glb_material(material, collection, spec['group'])
    else:
        raise RuntimeError(f'Unknown builder: {spec["builder"]}')

    save_material(material, asset_path)


def main():
    ensure_directory(RETRO_DIR)
    collection = ensure_geometry_collection()
    for spec in MATERIAL_SPECS:
        rebuild_material(spec, collection)
    log('Retro geometry variants are ready')


if __name__ == '__main__':
    main()
