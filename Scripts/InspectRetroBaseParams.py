import unreal

MATERIALS = [
    '/Game/UE5RFX/Materials/BaseMaterials/UE5RFX_PS1_BaseMaterial',
    '/Game/UE5RFX/Materials/BaseMaterials/UE5RFX_N64_BaseMaterial',
    '/Game/UE5RFX/Materials/BaseMaterials/UE5RFX_PS1_BaseMaterial_Inst',
]

mel = unreal.MaterialEditingLibrary

for path in MATERIALS:
    asset = unreal.EditorAssetLibrary.load_asset(path)
    unreal.log('[RetroBaseParams] ============================================================')
    unreal.log('[RetroBaseParams] Asset=' + path)
    if not asset:
        unreal.log('[RetroBaseParams] Missing')
        continue
    try:
        names = list(mel.get_scalar_parameter_names(asset))
        unreal.log('[RetroBaseParams] Scalars=' + str(names))
        for name in names:
            try:
                if isinstance(asset, unreal.MaterialInstanceConstant):
                    value = mel.get_material_instance_scalar_parameter_value(asset, name)
                else:
                    value = mel.get_material_default_scalar_parameter_value(asset, name)
                unreal.log(f'[RetroBaseParams]   {name}={value}')
            except Exception as exc:
                unreal.log(f'[RetroBaseParams]   {name} failed: {exc}')
    except Exception as exc:
        unreal.log('[RetroBaseParams] Scalar query failed: ' + str(exc))
