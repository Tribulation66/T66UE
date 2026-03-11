import unreal

LOG = '[RetroGeometryFix]'
MATERIALS = [
    '/Game/Materials/M_Character_Unlit',
    '/Game/Materials/M_Environment_Unlit',
    '/Game/Materials/M_FBX_Unlit',
    '/Game/Materials/M_GLB_Unlit',
]
DEFAULT_TEXTURE_CANDIDATES = [
    '/Engine/EngineResources/WhiteSquareTexture.WhiteSquareTexture',
    '/Engine/EngineResources/DefaultTexture.DefaultTexture',
]
mel = unreal.MaterialEditingLibrary


def log(text):
    unreal.log(f'{LOG} {text}')


def load_asset(path):
    asset = unreal.EditorAssetLibrary.load_asset(path)
    if asset:
        return asset
    return unreal.load_asset(path)


def load_default_texture():
    for path in DEFAULT_TEXTURE_CANDIDATES:
        texture = load_asset(path)
        if texture:
            log(f'Using default texture {path}')
            return texture
    raise RuntimeError('Failed to load a default fallback texture from engine content.')


def patch_material(material_path, default_texture):
    material = load_asset(material_path)
    if not material:
        raise RuntimeError(f'Missing material: {material_path}')

    changed = False
    for expr in material.get_editor_property('expressions') or []:
        if expr.get_class().get_name() != 'MaterialExpressionTextureSampleParameter2D':
            continue

        texture = expr.get_editor_property('texture')
        if texture:
            continue

        expr.set_editor_property('texture', default_texture)
        param_name = expr.get_editor_property('parameter_name')
        log(f'Set fallback texture on {material_path} param {param_name}')
        changed = True

    if changed:
        mel.recompile_material(material)
        unreal.EditorAssetLibrary.save_loaded_asset(material)
    else:
        log(f'No missing texture defaults found on {material_path}')


def main():
    default_texture = load_default_texture()
    for material_path in MATERIALS:
        patch_material(material_path, default_texture)
    log('Retro geometry fallback texture patch complete')


if __name__ == '__main__':
    main()
