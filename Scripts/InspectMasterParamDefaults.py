import unreal

mel = unreal.MaterialEditingLibrary
ASSETS = [
    '/Game/Materials/M_Character_Unlit',
    '/Game/Materials/M_Environment_Unlit',
    '/Game/Materials/M_FBX_Unlit',
    '/Game/Materials/M_GLB_Unlit',
    '/Game/Characters/Heroes/Hero_1/TypeA/Idle/Material_0',
    '/Game/Characters/Enemies/Enemy1/Material_001',
]

for asset_path in ASSETS:
    asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    unreal.log('[RetroParamDefaults] ============================================================')
    if not asset:
        unreal.log('[RetroParamDefaults] Missing ' + asset_path)
        continue
    unreal.log('[RetroParamDefaults] Asset=' + asset.get_path_name())
    unreal.log('[RetroParamDefaults] Class=' + asset.get_class().get_name())
    scalar_names = list(mel.get_scalar_parameter_names(asset))
    vector_names = list(mel.get_vector_parameter_names(asset))
    texture_names = list(mel.get_texture_parameter_names(asset))
    unreal.log('[RetroParamDefaults] Scalars=' + str(scalar_names))
    for name in scalar_names:
        try:
            if isinstance(asset, unreal.MaterialInstanceConstant):
                value = mel.get_material_instance_scalar_parameter_value(asset, name)
            else:
                value = mel.get_material_default_scalar_parameter_value(asset, name)
            unreal.log(f'[RetroParamDefaults]   {name}={value}')
        except Exception as exc:
            unreal.log(f'[RetroParamDefaults]   {name} failed: {exc}')
    unreal.log('[RetroParamDefaults] Vectors=' + str(vector_names))
    for name in vector_names:
        try:
            if isinstance(asset, unreal.MaterialInstanceConstant):
                value = mel.get_material_instance_vector_parameter_value(asset, name)
            else:
                value = mel.get_material_default_vector_parameter_value(asset, name)
            unreal.log(f'[RetroParamDefaults]   {name}={value}')
        except Exception as exc:
            unreal.log(f'[RetroParamDefaults]   {name} failed: {exc}')
    unreal.log('[RetroParamDefaults] Textures=' + str(texture_names))
    for name in texture_names:
        try:
            if isinstance(asset, unreal.MaterialInstanceConstant):
                value = mel.get_material_instance_texture_parameter_value(asset, name)
            else:
                value = mel.get_material_default_texture_parameter_value(asset, name)
            unreal.log(f"[RetroParamDefaults]   {name}={(value.get_path_name() if value else 'None')}")
        except Exception as exc:
            unreal.log(f'[RetroParamDefaults]   {name} failed: {exc}')
