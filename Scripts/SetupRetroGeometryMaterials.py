import unreal

LOG = '[RetroGeometrySetup]'
RETRO_DIR = '/Game/Materials/Retro'
MPC_PATH = RETRO_DIR + '/MPC_T66_RetroGeometry'

VERTEX_SNAP_FN = '/Game/UE5RFX/Materials/MaterialFunctions/UE5RFX_WPO_VertexSnapping'
VERTEX_NOISE_FN = '/Game/UE5RFX/Materials/MaterialFunctions/UE5RFX_WPO_VertexNoise'
AFFINE_FN = '/Game/UE5RFX/Materials/MaterialFunctions/UE5RFX_UV_AffineMapping'

MATERIAL_SPECS = [
    {
        'path': '/Game/Materials/M_Character_Unlit',
        'group': 'Character',
        'builder': 'character',
    },
    {
        'path': '/Game/Materials/M_Environment_Unlit',
        'group': 'World',
        'builder': 'environment',
    },
    {
        'path': '/Game/Materials/M_FBX_Unlit',
        'group': 'Character',
        'builder': 'fbx',
    },
    {
        'path': '/Game/Materials/M_GLB_Unlit',
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
    collection = create_asset_if_missing(MPC_PATH, unreal.MaterialParameterCollection, unreal.MaterialParameterCollectionFactoryNew())

    scalar_params = [
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
    ]
    vector_params = [
        make_vector_collection_param('WorldVertexNoiseOffset', (0.0, 0.0, 0.0, 0.0)),
        make_vector_collection_param('CharacterVertexNoiseOffset', (0.0, 0.0, 0.0, 0.0)),
    ]

    collection.set_editor_property('scalar_parameters', scalar_params)
    collection.set_editor_property('vector_parameters', vector_params)
    unreal.EditorAssetLibrary.save_loaded_asset(collection)
    log(f'Prepared collection {collection.get_path_name()}')
    return collection


def set_material_flags(material):
    material.set_editor_property('shading_model', unreal.MaterialShadingModel.MSM_UNLIT)
    material.set_editor_property('blend_mode', unreal.BlendMode.BLEND_OPAQUE)
    material.set_editor_property('two_sided', True)
    material.set_editor_property('use_material_attributes', False)


def create_texcoord(material, x, y):
    return mel.create_material_expression(material, unreal.MaterialExpressionTextureCoordinate, x, y)


def create_collection_parameter(material, collection, parameter_name, x, y):
    expr = mel.create_material_expression(material, unreal.MaterialExpressionCollectionParameter, x, y)
    expr.set_editor_property('collection', collection)
    expr.set_editor_property('parameter_name', parameter_name)
    return expr


def create_scalar_parameter(material, parameter_name, default_value, x, y):
    expr = mel.create_material_expression(material, unreal.MaterialExpressionScalarParameter, x, y)
    expr.set_editor_property('parameter_name', parameter_name)
    expr.set_editor_property('default_value', float(default_value))
    return expr


def create_vector_parameter(material, parameter_name, default_value, x, y):
    expr = mel.create_material_expression(material, unreal.MaterialExpressionVectorParameter, x, y)
    expr.set_editor_property('parameter_name', parameter_name)
    expr.set_editor_property('default_value', unreal.LinearColor(*default_value))
    return expr


def create_texture_parameter(material, parameter_name, x, y):
    expr = mel.create_material_expression(material, unreal.MaterialExpressionTextureSampleParameter2D, x, y)
    expr.set_editor_property('parameter_name', parameter_name)
    return expr


def create_material_function_call(material, function_asset, x, y):
    expr = mel.create_material_expression(material, unreal.MaterialExpressionMaterialFunctionCall, x, y)
    expr.set_editor_property('material_function', function_asset)
    return expr


def create_multiply(material, x, y):
    return mel.create_material_expression(material, unreal.MaterialExpressionMultiply, x, y)


def create_lerp(material, x, y):
    return mel.create_material_expression(material, unreal.MaterialExpressionLinearInterpolate, x, y)


def create_constant3(material, value, x, y):
    expr = mel.create_material_expression(material, unreal.MaterialExpressionConstant3Vector, x, y)
    expr.set_editor_property('constant', unreal.LinearColor(value[0], value[1], value[2], 0.0))
    return expr


def connect(source, source_output, target, target_input):
    mel.connect_material_expressions(source, source_output, target, target_input)


def build_geometry_nodes(material, collection, group_prefix):
    zero = create_constant3(material, (0.0, 0.0, 0.0), -2200, 300)
    snap_strength = create_collection_parameter(material, collection, f'{group_prefix}VertexSnapStrength', -2200, 120)
    snap_target_res = create_collection_parameter(material, collection, f'{group_prefix}VertexSnapTargetResolution', -2200, 220)
    noise_offset = create_collection_parameter(material, collection, f'{group_prefix}VertexNoiseOffset', -2200, 420)

    vertex_snap = create_material_function_call(material, load_asset(VERTEX_SNAP_FN), -1700, 260)
    connect(zero, '', vertex_snap, 'World Position Offset')
    connect(snap_strength, '', vertex_snap, 'Strength Multiplier')
    connect(snap_target_res, '', vertex_snap, 'TargetResolution')

    vertex_noise = create_material_function_call(material, load_asset(VERTEX_NOISE_FN), -1200, 300)
    connect(vertex_snap, '', vertex_noise, 'World Position Offset')
    connect(noise_offset, '', vertex_noise, 'Noise Offset')

    mel.connect_material_property(vertex_noise, '', unreal.MaterialProperty.MP_WORLD_POSITION_OFFSET)


def build_affine_uv(material, collection, group_prefix, start_y):
    texcoord = create_texcoord(material, -2200, start_y)
    affine_blend = create_collection_parameter(material, collection, f'{group_prefix}AffineBlend', -2200, start_y + 220)
    dist1 = create_collection_parameter(material, collection, f'{group_prefix}AffineDistance1', -2200, start_y - 120)
    dist2 = create_collection_parameter(material, collection, f'{group_prefix}AffineDistance2', -2200, start_y - 20)
    dist3 = create_collection_parameter(material, collection, f'{group_prefix}AffineDistance3', -2200, start_y + 80)

    affine = create_material_function_call(material, load_asset(AFFINE_FN), -1600, start_y)
    connect(texcoord, '', affine, 'TextureCoordinate')
    connect(dist1, '', affine, 'Distance 1')
    connect(dist2, '', affine, 'Distance 2')
    connect(dist3, '', affine, 'Distance 3')

    lerp = create_lerp(material, -1150, start_y)
    connect(texcoord, '', lerp, 'A')
    connect(affine, '', lerp, 'B')
    connect(affine_blend, '', lerp, 'Alpha')
    return lerp


def rebuild_character_material(material, collection, group_prefix):
    uv = build_affine_uv(material, collection, group_prefix, -160)
    texture = create_texture_parameter(material, 'DiffuseColorMap', -700, -160)
    brightness = create_scalar_parameter(material, 'Brightness', 1.0, -700, 10)
    multiply = create_multiply(material, -320, -120)
    connect(uv, '', texture, 'UVs')
    connect(texture, 'RGB', multiply, 'A')
    connect(brightness, '', multiply, 'B')
    mel.connect_material_property(multiply, '', unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    build_geometry_nodes(material, collection, group_prefix)


def rebuild_environment_material(material, collection, group_prefix):
    uv = build_affine_uv(material, collection, group_prefix, -180)
    texture = create_texture_parameter(material, 'DiffuseColorMap', -700, -180)
    tint = create_vector_parameter(material, 'Tint', (1.0, 1.0, 1.0, 1.0), -700, -20)
    brightness = create_scalar_parameter(material, 'Brightness', 1.0, -700, 140)
    tint_mul = create_multiply(material, -360, -120)
    bright_mul = create_multiply(material, -80, -80)
    connect(uv, '', texture, 'UVs')
    connect(texture, 'RGB', tint_mul, 'A')
    connect(tint, '', tint_mul, 'B')
    connect(tint_mul, '', bright_mul, 'A')
    connect(brightness, '', bright_mul, 'B')
    mel.connect_material_property(bright_mul, '', unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    build_geometry_nodes(material, collection, group_prefix)


def rebuild_fbx_material(material, collection, group_prefix):
    uv = build_affine_uv(material, collection, group_prefix, -220)
    diffuse = create_texture_parameter(material, 'DiffuseColorMap', -700, -220)
    normal = create_texture_parameter(material, 'NormalMap', -700, -80)
    specular = create_texture_parameter(material, 'SpecularColorMap', -700, 60)
    emissive = create_texture_parameter(material, 'EmissiveColorMap', -700, 200)
    emissive_weight = create_scalar_parameter(material, 'EmissiveColorMapWeight', 0.0, -700, 340)

    connect(uv, '', diffuse, 'UVs')
    connect(uv, '', normal, 'UVs')
    connect(uv, '', specular, 'UVs')
    connect(uv, '', emissive, 'UVs')
    mel.connect_material_property(diffuse, 'RGB', unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    build_geometry_nodes(material, collection, group_prefix)


def rebuild_glb_material(material, collection, group_prefix):
    uv = build_affine_uv(material, collection, group_prefix, -120)
    texture = create_texture_parameter(material, 'BaseColorTexture', -700, -120)
    connect(uv, '', texture, 'UVs')
    mel.connect_material_property(texture, 'RGB', unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    build_geometry_nodes(material, collection, group_prefix)


def rebuild_material(spec, collection):
    material = load_asset(spec['path'])
    log(f'Rebuilding {spec["path"]}')
    mel.delete_all_material_expressions(material)
    set_material_flags(material)

    if spec['builder'] == 'character':
        rebuild_character_material(material, collection, spec['group'])
    elif spec['builder'] == 'environment':
        rebuild_environment_material(material, collection, spec['group'])
    elif spec['builder'] == 'fbx':
        rebuild_fbx_material(material, collection, spec['group'])
    elif spec['builder'] == 'glb':
        rebuild_glb_material(material, collection, spec['group'])
    else:
        raise RuntimeError(f'Unknown builder {spec["builder"]}')

    mel.layout_material_expressions(material)
    mel.recompile_material(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material)


def main():
    ensure_directory(RETRO_DIR)
    collection = ensure_geometry_collection()
    for spec in MATERIAL_SPECS:
        rebuild_material(spec, collection)
    log('Geometry retro material setup complete')


if __name__ == '__main__':
    main()
