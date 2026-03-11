import unreal

asset = unreal.EditorAssetLibrary.load_asset('/Game/UE5RFX/Materials/UE5RFX_MaterialParameterCollection')
for prop in ('scalar_parameters', 'vector_parameters'):
    values = asset.get_editor_property(prop)
    unreal.log(f'[RetroMPCStruct] {prop} count={len(values)}')
    if values:
        first = values[0]
        unreal.log(f'[RetroMPCStruct] {prop} type={type(first)}')
        for field in ('parameter_name', 'default_value', 'id'):
            try:
                unreal.log(f'[RetroMPCStruct] {prop}.{field}={first.get_editor_property(field)}')
            except Exception as exc:
                unreal.log(f'[RetroMPCStruct] {prop}.{field} failed: {exc}')
