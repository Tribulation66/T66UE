import os
import sys
import unreal

LOG = '[RetroGeometryAppend]'
MPC_PATH = '/Game/Materials/Retro/MPC_T66_RetroGeometry'
VERTEX_SNAP_FN = '/Game/UE5RFX/Materials/MaterialFunctions/UE5RFX_WPO_VertexSnapping'
VERTEX_NOISE_FN = '/Game/UE5RFX/Materials/MaterialFunctions/UE5RFX_WPO_VertexNoise'
AFFINE_FN = '/Game/UE5RFX/Materials/MaterialFunctions/UE5RFX_UV_AffineMapping'

SPECS = {
    'environment': {'path': '/Game/Materials/M_Environment_Unlit', 'group': 'World', 'builder': 'environment'},
    'fbx': {'path': '/Game/Materials/M_FBX_Unlit', 'group': 'Character', 'builder': 'fbx'},
    'glb': {'path': '/Game/Materials/M_GLB_Unlit', 'group': 'World', 'builder': 'glb'},
}

mel = unreal.MaterialEditingLibrary


def log(text):
    unreal.log(f'{LOG} {text}')


def load_asset(path):
    asset = unreal.EditorAssetLibrary.load_asset(path)
    if not asset:
        raise RuntimeError(f'Missing asset: {path}')
    return asset


def set_material_flags(material):
    material.set_editor_property('shading_model', unreal.MaterialShadingModel.MSM_UNLIT)
    material.set_editor_property('blend_mode', unreal.BlendMode.BLEND_OPAQUE)
    material.set_editor_property('two_sided', True)
    material.set_editor_property('use_material_attributes', False)


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


def create_texcoord(material, x, y):
    return mel.create_material_expression(material, unreal.MaterialExpressionTextureCoordinate, x, y)


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
    zero = create_constant3(material, (0.0, 0.0, 0.0), 2200, 300)
    snap_strength = create_collection_parameter(material, collection, f'{group_prefix}VertexSnapStrength', 2200, 120)
    snap_target_res = create_collection_parameter(material, collection, f'{group_prefix}VertexSnapTargetResolution', 2200, 220)
    noise_offset = create_collection_parameter(material, collection, f'{group_prefix}VertexNoiseOffset', 2200, 420)

    vertex_snap = create_material_function_call(material, load_asset(VERTEX_SNAP_FN), 2700, 260)
    connect(zero, '', vertex_snap, 'World Position Offset')
    connect(snap_strength, '', vertex_snap, 'Strength Multiplier')
    connect(snap_target_res, '', vertex_snap, 'TargetResolution')

    vertex_noise = create_material_function_call(material, load_asset(VERTEX_NOISE_FN), 3200, 300)
    connect(vertex_snap, '', vertex_noise, 'World Position Offset')
    connect(noise_offset, '', vertex_noise, 'Noise Offset')

    mel.connect_material_property(vertex_noise, '', unreal.MaterialProperty.MP_WORLD_POSITION_OFFSET)


def build_affine_uv(material, collection, group_prefix, start_y):
    texcoord = create_texcoord(material, 2200, start_y)
    affine_blend = create_collection_parameter(material, collection, f'{group_prefix}AffineBlend', 2200, start_y + 220)
    dist1 = create_collection_parameter(material, collection, f'{group_prefix}AffineDistance1', 2200, start_y - 120)
    dist2 = create_collection_parameter(material, collection, f'{group_prefix}AffineDistance2', 2200, start_y - 20)
    dist3 = create_collection_parameter(material, collection, f'{group_prefix}AffineDistance3', 2200, start_y + 80)

    affine = create_material_function_call(material, load_asset(AFFINE_FN), 2800, start_y)
    connect(texcoord, '', affine, 'TextureCoordinate')
    connect(dist1, '', affine, 'Distance 1')
    connect(dist2, '', affine, 'Distance 2')
    connect(dist3, '', affine, 'Distance 3')

    lerp = create_lerp(material, 3350, start_y)
    connect(texcoord, '', lerp, 'A')
    connect(affine, '', lerp, 'B')
    connect(affine_blend, '', lerp, 'Alpha')
    return lerp


def patch_environment(material, collection, group_prefix):
    uv = build_affine_uv(material, collection, group_prefix, -180)
    texture = create_texture_parameter(material, 'DiffuseColorMap', 3800, -180)
    tint = create_vector_parameter(material, 'Tint', (1.0, 1.0, 1.0, 1.0), 3800, -20)
    brightness = create_scalar_parameter(material, 'Brightness', 1.0, 3800, 140)
    tint_mul = create_multiply(material, 4150, -120)
    bright_mul = create_multiply(material, 4450, -80)
    connect(uv, '', texture, 'UVs')
    connect(texture, 'RGB', tint_mul, 'A')
    connect(tint, '', tint_mul, 'B')
    connect(tint_mul, '', bright_mul, 'A')
    connect(brightness, '', bright_mul, 'B')
    mel.connect_material_property(bright_mul, '', unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    build_geometry_nodes(material, collection, group_prefix)


def patch_fbx(material, collection, group_prefix):
    uv = build_affine_uv(material, collection, group_prefix, -220)
    diffuse = create_texture_parameter(material, 'DiffuseColorMap', 3800, -220)
    normal = create_texture_parameter(material, 'NormalMap', 3800, -80)
    specular = create_texture_parameter(material, 'SpecularColorMap', 3800, 60)
    emissive = create_texture_parameter(material, 'EmissiveColorMap', 3800, 200)
    emissive_weight = create_scalar_parameter(material, 'EmissiveColorMapWeight', 0.0, 3800, 340)
    connect(uv, '', diffuse, 'UVs')
    connect(uv, '', normal, 'UVs')
    connect(uv, '', specular, 'UVs')
    connect(uv, '', emissive, 'UVs')
    mel.connect_material_property(diffuse, 'RGB', unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    build_geometry_nodes(material, collection, group_prefix)


def patch_glb(material, collection, group_prefix):
    uv = build_affine_uv(material, collection, group_prefix, -120)
    texture = create_texture_parameter(material, 'BaseColorTexture', 3800, -120)
    connect(uv, '', texture, 'UVs')
    mel.connect_material_property(texture, 'RGB', unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    build_geometry_nodes(material, collection, group_prefix)


def main():
    key = os.environ.get('T66_RETRO_TARGET')
    if key not in SPECS:
        for arg in sys.argv[1:]:
            if arg in SPECS:
                key = arg
                break
    if key not in SPECS:
        raise RuntimeError('Expected one of: ' + ', '.join(sorted(SPECS.keys())))

    spec = SPECS[key]
    material = load_asset(spec['path'])
    collection = load_asset(MPC_PATH)

    log(f'Appending retro nodes to {spec["path"]}')
    set_material_flags(material)

    if spec['builder'] == 'environment':
        patch_environment(material, collection, spec['group'])
    elif spec['builder'] == 'fbx':
        patch_fbx(material, collection, spec['group'])
    elif spec['builder'] == 'glb':
        patch_glb(material, collection, spec['group'])

    mel.layout_material_expressions(material)
    mel.recompile_material(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material)
    log('Done')


if __name__ == '__main__':
    main()
