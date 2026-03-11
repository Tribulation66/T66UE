import unreal

MATERIALS = [
    '/Game/Materials/M_Character_Unlit',
    '/Game/Materials/M_Environment_Unlit',
    '/Game/Materials/M_FBX_Unlit',
    '/Game/Materials/M_GLB_Unlit',
]

mel = unreal.MaterialEditingLibrary
props = [
    unreal.MaterialProperty.MP_BASE_COLOR,
    unreal.MaterialProperty.MP_EMISSIVE_COLOR,
    unreal.MaterialProperty.MP_OPACITY_MASK,
    unreal.MaterialProperty.MP_WORLD_POSITION_OFFSET,
    unreal.MaterialProperty.MP_NORMAL,
]

for path in MATERIALS:
    mat = unreal.EditorAssetLibrary.load_asset(path)
    unreal.log('[RetroMasterProps] ============================================================')
    unreal.log('[RetroMasterProps] Material=' + path)
    if not mat:
        unreal.log('[RetroMasterProps] Missing')
        continue
    unreal.log('[RetroMasterProps] NumExpr=' + str(mel.get_num_material_expressions(mat)))
    try:
        unreal.log('[RetroMasterProps] ScalarParams=' + str(list(mel.get_scalar_parameter_names(mat))))
    except Exception as exc:
        unreal.log('[RetroMasterProps] ScalarParams failed: ' + str(exc))
    try:
        unreal.log('[RetroMasterProps] VectorParams=' + str(list(mel.get_vector_parameter_names(mat))))
    except Exception as exc:
        unreal.log('[RetroMasterProps] VectorParams failed: ' + str(exc))
    try:
        unreal.log('[RetroMasterProps] TextureParams=' + str(list(mel.get_texture_parameter_names(mat))))
    except Exception as exc:
        unreal.log('[RetroMasterProps] TextureParams failed: ' + str(exc))

    for prop in props:
        try:
            node = mel.get_material_property_input_node(mat, prop)
            name = node.get_name() if node else 'None'
            out = mel.get_material_property_input_node_output_name(mat, prop)
            unreal.log(f'[RetroMasterProps] {prop}: node={name} output={out}')
        except Exception as exc:
            unreal.log(f'[RetroMasterProps] {prop} failed: {exc}')
