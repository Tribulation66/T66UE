import unreal

LOG = '[IntegratedRetroMasters]'
DEST_DIR = '/Game/Materials'
BACKUP_DIR = '/Game/Materials/Retro/Backups'
MPC_PATH = '/Game/Materials/Retro/MPC_T66_RetroGeometry'
VERTEX_SNAP_FN = '/Game/UE5RFX/Materials/MaterialFunctions/UE5RFX_WPO_VertexSnapping'
VERTEX_NOISE_FN = '/Game/UE5RFX/Materials/MaterialFunctions/UE5RFX_WPO_VertexNoise'
AFFINE_FN = '/Game/UE5RFX/Materials/MaterialFunctions/UE5RFX_UV_AffineMapping'

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
mel = unreal.MaterialEditingLibrary


def log(text):
    unreal.log(f'{LOG} {text}')


def safe_set(obj, prop, value):
    try:
        obj.set_editor_property(prop, value)
    except Exception as exc:
        unreal.log_warning(f'{LOG} set_editor_property({prop}) failed: {exc}')


def ensure_directory(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


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
    safe_set(param, 'parameter_name', name)
    safe_set(param, 'default_value', float(default_value))
    return param


def make_vector_collection_param(name, default_value):
    param = unreal.CollectionVectorParameter()
    safe_set(param, 'parameter_name', name)
    safe_set(param, 'default_value', unreal.LinearColor(*default_value))
    return param


def ensure_geometry_collection():
    existing = unreal.EditorAssetLibrary.load_asset(MPC_PATH)
    collection = existing if existing else create_asset_if_missing(
        MPC_PATH,
        unreal.MaterialParameterCollection,
        unreal.MaterialParameterCollectionFactoryNew(),
    )
    safe_set(collection, 'scalar_parameters', [
        make_scalar_collection_param('WorldGeometryEnable', 0.0),
        make_scalar_collection_param('WorldVertexSnapStrength', 0.0),
        make_scalar_collection_param('WorldVertexSnapTargetResolution', 600.0),
        make_scalar_collection_param('WorldAffineBlend', 0.0),
        make_scalar_collection_param('WorldAffineDistance1', 400.0),
        make_scalar_collection_param('WorldAffineDistance2', 1200.0),
        make_scalar_collection_param('WorldAffineDistance3', 1800.0),
        make_scalar_collection_param('CharacterGeometryEnable', 0.0),
        make_scalar_collection_param('CharacterVertexSnapStrength', 0.0),
        make_scalar_collection_param('CharacterVertexSnapTargetResolution', 600.0),
        make_scalar_collection_param('CharacterAffineBlend', 0.0),
        make_scalar_collection_param('CharacterAffineDistance1', 400.0),
        make_scalar_collection_param('CharacterAffineDistance2', 1200.0),
        make_scalar_collection_param('CharacterAffineDistance3', 1800.0),
    ])
    safe_set(collection, 'vector_parameters', [
        make_vector_collection_param('WorldVertexNoiseOffset', (0.0, 0.0, 0.0, 0.0)),
        make_vector_collection_param('CharacterVertexNoiseOffset', (0.0, 0.0, 0.0, 0.0)),
    ])
    unreal.EditorAssetLibrary.save_loaded_asset(collection)
    log(f'Prepared geometry collection {collection.get_path_name()}')
    return collection


def backup_asset(asset_path, backup_name):
    if not unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        return
    ensure_directory(BACKUP_DIR)
    backup_path = f'{BACKUP_DIR}/{backup_name}'
    if unreal.EditorAssetLibrary.does_asset_exist(backup_path):
        unreal.EditorAssetLibrary.delete_asset(backup_path)
    if unreal.EditorAssetLibrary.duplicate_asset(asset_path, backup_path):
        log(f'Backed up {asset_path} -> {backup_path}')
    else:
        unreal.log_warning(f'{LOG} Failed to back up {asset_path}')


def recreate_material(asset_name, used_with_skeletal_mesh=False, used_with_instanced_static_meshes=False, used_with_nanite=False):
    asset_path = f'{DEST_DIR}/{asset_name}'
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        unreal.EditorAssetLibrary.delete_asset(asset_path)

    material = asset_tools.create_asset(asset_name, DEST_DIR, unreal.Material, unreal.MaterialFactoryNew())
    if not material:
        raise RuntimeError(f'Failed to create {asset_path}')

    safe_set(material, 'shading_model', unreal.MaterialShadingModel.MSM_UNLIT)
    safe_set(material, 'blend_mode', unreal.BlendMode.BLEND_OPAQUE)
    safe_set(material, 'two_sided', True)
    safe_set(material, 'use_material_attributes', False)
    safe_set(material, 'used_with_skeletal_mesh', used_with_skeletal_mesh)
    safe_set(material, 'used_with_instanced_static_meshes', used_with_instanced_static_meshes)
    safe_set(material, 'used_with_nanite', used_with_nanite)
    return material, asset_path


def save_material(material, asset_path):
    mel.layout_material_expressions(material)
    mel.recompile_material(material)
    unreal.EditorAssetLibrary.save_asset(asset_path)
    log(f'Saved {asset_path}')


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
    safe_set(expr, 'material_function', unreal.EditorAssetLibrary.load_asset(function_path))
    return expr


def create_texcoord(material, x, y):
    return mel.create_material_expression(material, unreal.MaterialExpressionTextureCoordinate, x, y)


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
    enable = create_collection_parameter(material, collection, f'{group_prefix}GeometryEnable', -2100, y + 360)

    affine = create_material_function_call(material, AFFINE_FN, -1500, y)
    connect(texcoord, '', affine, 'TextureCoordinate')
    connect(dist1, '', affine, 'Distance 1')
    connect(dist2, '', affine, 'Distance 2')
    connect(dist3, '', affine, 'Distance 3')

    gated_blend = create_multiply(material, -1250, y + 220)
    connect(blend, '', gated_blend, 'A')
    connect(enable, '', gated_blend, 'B')

    lerp = create_lerp(material, -950, y)
    connect(texcoord, '', lerp, 'A')
    connect(affine, '', lerp, 'B')
    connect(gated_blend, '', lerp, 'Alpha')
    return lerp


def build_geometry_wpo(material, collection, group_prefix):
    zero = create_constant3(material, (0.0, 0.0, 0.0), -2100, 320)
    enable = create_collection_parameter(material, collection, f'{group_prefix}GeometryEnable', -2100, 0)
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

    gated_wpo = create_multiply(material, -500, 320)
    connect(vertex_noise, '', gated_wpo, 'A')
    connect(enable, '', gated_wpo, 'B')

    mel.connect_material_property(gated_wpo, '', unreal.MaterialProperty.MP_WORLD_POSITION_OFFSET)


def build_character_material(collection):
    backup_asset('/Game/Materials/M_Character_Unlit', 'M_Character_Unlit_IntegratedRetroGeometryBackup')
    material, asset_path = recreate_material('M_Character_Unlit', used_with_skeletal_mesh=True)
    uv = build_affine_uv(material, collection, 'Character', -120)
    texture = create_texture_parameter(material, 'DiffuseColorMap', -350, -120)
    brightness = create_scalar_parameter(material, 'Brightness', 1.0, -350, 100)
    multiply = create_multiply(material, -20, -80)
    connect(uv, '', texture, 'UVs')
    connect(texture, 'RGB', multiply, 'A')
    connect(brightness, '', multiply, 'B')
    mel.connect_material_property(multiply, '', unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    build_geometry_wpo(material, collection, 'Character')
    save_material(material, asset_path)


def build_environment_material(collection):
    backup_asset('/Game/Materials/M_Environment_Unlit', 'M_Environment_Unlit_IntegratedRetroGeometryBackup')
    material, asset_path = recreate_material('M_Environment_Unlit', used_with_instanced_static_meshes=True)
    uv = build_affine_uv(material, collection, 'World', -140)
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
    build_geometry_wpo(material, collection, 'World')
    save_material(material, asset_path)


def build_fbx_material(collection):
    backup_asset('/Game/Materials/M_FBX_Unlit', 'M_FBX_Unlit_IntegratedRetroGeometryBackup')
    material, asset_path = recreate_material('M_FBX_Unlit', used_with_skeletal_mesh=True)
    uv = build_affine_uv(material, collection, 'Character', -220)
    diffuse = create_texture_parameter(material, 'DiffuseColorMap', -350, -220)
    normal = create_texture_parameter(material, 'NormalMap', -350, -80)
    specular = create_texture_parameter(material, 'SpecularColorMap', -350, 60)
    emissive = create_texture_parameter(material, 'EmissiveColorMap', -350, 200)
    emissive_weight = create_scalar_parameter(material, 'EmissiveColorMapWeight', 0.0, -350, 340)
    connect(uv, '', diffuse, 'UVs')
    connect(uv, '', normal, 'UVs')
    connect(uv, '', specular, 'UVs')
    connect(uv, '', emissive, 'UVs')
    mel.connect_material_property(diffuse, 'RGB', unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    _ = emissive_weight
    build_geometry_wpo(material, collection, 'Character')
    save_material(material, asset_path)


def build_glb_material(collection):
    backup_asset('/Game/Materials/M_GLB_Unlit', 'M_GLB_Unlit_IntegratedRetroGeometryBackup')
    material, asset_path = recreate_material('M_GLB_Unlit', used_with_instanced_static_meshes=True, used_with_nanite=True)
    uv = build_affine_uv(material, collection, 'World', -120)
    texture = create_texture_parameter(material, 'BaseColorTexture', -350, -120)
    connect(uv, '', texture, 'UVs')
    mel.connect_material_property(texture, 'RGB', unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    build_geometry_wpo(material, collection, 'World')
    save_material(material, asset_path)


def main():
    ensure_directory(DEST_DIR)
    ensure_directory(BACKUP_DIR)
    collection = ensure_geometry_collection()
    build_character_material(collection)
    build_environment_material(collection)
    build_fbx_material(collection)
    build_glb_material(collection)
    log('Integrated retro geometry master rebuild complete')


if __name__ == '__main__':
    main()
