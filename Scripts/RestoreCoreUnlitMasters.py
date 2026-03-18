import unreal

LOG = '[RestoreUnlitMasters]'
DEST_DIR = '/Game/Materials'
BACKUP_DIR = '/Game/Materials/Retro/Backups'

mel = unreal.MaterialEditingLibrary
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()


def safe_set(obj, prop, value):
    try:
        obj.set_editor_property(prop, value)
    except Exception as exc:
        unreal.log_warning(f'{LOG} set_editor_property({prop}) failed: {exc}')


def ensure_dir(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def backup_asset(asset_path, backup_name):
    if not unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        return
    ensure_dir(BACKUP_DIR)
    backup_path = f'{BACKUP_DIR}/{backup_name}'
    if unreal.EditorAssetLibrary.does_asset_exist(backup_path):
        unreal.EditorAssetLibrary.delete_asset(backup_path)
    if unreal.EditorAssetLibrary.duplicate_asset(asset_path, backup_path):
        unreal.log(f'{LOG} Backed up {asset_path} -> {backup_path}')
    else:
        unreal.log_warning(f'{LOG} Backup failed for {asset_path}')


def recreate_material(asset_name):
    path = f'{DEST_DIR}/{asset_name}'
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        unreal.EditorAssetLibrary.delete_asset(path)
    factory = unreal.MaterialFactoryNew()
    mat = asset_tools.create_asset(asset_name, DEST_DIR, unreal.Material, factory)
    if not mat:
        raise RuntimeError(f'Failed to create {path}')
    safe_set(mat, 'shading_model', unreal.MaterialShadingModel.MSM_UNLIT)
    safe_set(mat, 'two_sided', True)
    safe_set(mat, 'used_with_nanite', asset_name == 'M_GLB_Unlit')
    return mat, path


def save_material(mat, path):
    mel.recompile_material(mat)
    unreal.EditorAssetLibrary.save_asset(path)
    unreal.log(f'{LOG} Saved {path}')


def build_character():
    backup_asset('/Game/Materials/M_Character_Unlit', 'M_Character_Unlit_RetroGeometryBackup')
    mat, path = recreate_material('M_Character_Unlit')
    tex = mel.create_material_expression(mat, unreal.MaterialExpressionTextureSampleParameter2D, -400, 0)
    safe_set(tex, 'parameter_name', 'DiffuseColorMap')
    brightness = mel.create_material_expression(mat, unreal.MaterialExpressionScalarParameter, -400, 300)
    safe_set(brightness, 'parameter_name', 'Brightness')
    safe_set(brightness, 'default_value', 1.0)
    mul = mel.create_material_expression(mat, unreal.MaterialExpressionMultiply, -100, 0)
    mel.connect_material_expressions(tex, 'RGB', mul, 'A')
    mel.connect_material_expressions(brightness, '', mul, 'B')
    mel.connect_material_property(mul, '', unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    save_material(mat, path)


def build_environment():
    backup_asset('/Game/Materials/M_Environment_Unlit', 'M_Environment_Unlit_RetroGeometryBackup')
    mat, path = recreate_material('M_Environment_Unlit')
    tex = mel.create_material_expression(mat, unreal.MaterialExpressionTextureSampleParameter2D, -500, 0)
    safe_set(tex, 'parameter_name', 'DiffuseColorMap')
    brightness = mel.create_material_expression(mat, unreal.MaterialExpressionScalarParameter, -500, 250)
    safe_set(brightness, 'parameter_name', 'Brightness')
    safe_set(brightness, 'default_value', 1.0)
    tint = mel.create_material_expression(mat, unreal.MaterialExpressionVectorParameter, -500, 450)
    safe_set(tint, 'parameter_name', 'Tint')
    safe_set(tint, 'default_value', unreal.LinearColor(1.0, 1.0, 1.0, 1.0))
    mul_bright = mel.create_material_expression(mat, unreal.MaterialExpressionMultiply, -250, 100)
    mel.connect_material_expressions(tex, 'RGB', mul_bright, 'A')
    mel.connect_material_expressions(brightness, '', mul_bright, 'B')
    mul_tint = mel.create_material_expression(mat, unreal.MaterialExpressionMultiply, 0, 100)
    mel.connect_material_expressions(mul_bright, '', mul_tint, 'A')
    mel.connect_material_expressions(tint, '', mul_tint, 'B')
    mel.connect_material_property(mul_tint, '', unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    save_material(mat, path)


def build_fbx():
    backup_asset('/Game/Materials/M_FBX_Unlit', 'M_FBX_Unlit_RetroGeometryBackup')
    mat, path = recreate_material('M_FBX_Unlit')
    tex = mel.create_material_expression(mat, unreal.MaterialExpressionTextureSampleParameter2D, -400, 0)
    safe_set(tex, 'parameter_name', 'DiffuseColorMap')
    normal = mel.create_material_expression(mat, unreal.MaterialExpressionTextureSampleParameter2D, -400, 300)
    safe_set(normal, 'parameter_name', 'NormalMap')
    specular = mel.create_material_expression(mat, unreal.MaterialExpressionTextureSampleParameter2D, -400, 600)
    safe_set(specular, 'parameter_name', 'SpecularColorMap')
    emissive = mel.create_material_expression(mat, unreal.MaterialExpressionTextureSampleParameter2D, -400, 900)
    safe_set(emissive, 'parameter_name', 'EmissiveColorMap')
    emissive_weight = mel.create_material_expression(mat, unreal.MaterialExpressionScalarParameter, -400, 1200)
    safe_set(emissive_weight, 'parameter_name', 'EmissiveColorMapWeight')
    safe_set(emissive_weight, 'default_value', 0.0)
    mel.connect_material_property(tex, 'RGB', unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    save_material(mat, path)


def build_glb():
    backup_asset('/Game/Materials/M_GLB_Unlit', 'M_GLB_Unlit_RetroGeometryBackup')
    mat, path = recreate_material('M_GLB_Unlit')
    tex = mel.create_material_expression(mat, unreal.MaterialExpressionTextureSampleParameter2D, -300, 0)
    safe_set(tex, 'parameter_name', 'BaseColorTexture')
    mel.connect_material_property(tex, 'RGB', unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    save_material(mat, path)


ensure_dir(DEST_DIR)
build_character()
build_environment()
build_fbx()
build_glb()
unreal.log(f'{LOG} Restore complete')
