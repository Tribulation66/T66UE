import unreal

MATERIALS = [
    '/Game/Characters/Heroes/Hero_1/Chad/Idle/Material_0',
    '/Game/Characters/Heroes/Hero_1/Chad/Idle/Material_0_003',
    '/Game/Characters/Heroes/Hero_1/Chad/Idle/Material_0_004',
    '/Game/Characters/Heroes/Hero_1/Chad/Walk/Material_0',
]
PARAMS = ['DiffuseColorMap', 'BaseColorTexture']

for path in MATERIALS:
    asset = unreal.EditorAssetLibrary.load_asset(path)
    unreal.log('[RetroMIC] ==========================================')
    unreal.log('[RetroMIC] Asset=' + path)
    if not asset:
        unreal.log('[RetroMIC] Missing')
        continue
    try:
        parent = asset.get_editor_property('parent')
        unreal.log('[RetroMIC] Parent=' + (parent.get_path_name() if parent else 'None'))
    except Exception as exc:
        unreal.log('[RetroMIC] Parent failed: ' + str(exc))

    for param in PARAMS:
        try:
            value = unreal.MaterialEditingLibrary.get_material_instance_texture_parameter_value(asset, param)
            if isinstance(value, (tuple, list)):
                tex = value[-1] if value else None
            else:
                tex = value
            unreal.log(f'[RetroMIC] {param}=' + (tex.get_path_name() if tex else 'None'))
        except Exception as exc:
            unreal.log(f'[RetroMIC] {param} failed: {exc}')
