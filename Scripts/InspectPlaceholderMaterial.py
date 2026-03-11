import unreal
mel = unreal.MaterialEditingLibrary
path = '/Game/Materials/M_PlaceholderColor'
mat = unreal.EditorAssetLibrary.load_asset(path)
unreal.log('[PlaceholderMat] Material=' + path)
if mat:
    unreal.log('[PlaceholderMat] NumExpr=' + str(mel.get_num_material_expressions(mat)))
    unreal.log('[PlaceholderMat] Scalar=' + str(list(mel.get_scalar_parameter_names(mat))))
    unreal.log('[PlaceholderMat] Vector=' + str(list(mel.get_vector_parameter_names(mat))))
    unreal.log('[PlaceholderMat] Texture=' + str(list(mel.get_texture_parameter_names(mat))))
    for prop in ('shading_model','blend_mode','two_sided'):
        unreal.log(f'[PlaceholderMat] {prop}={mat.get_editor_property(prop)}')
    for prop in (unreal.MaterialProperty.MP_EMISSIVE_COLOR, unreal.MaterialProperty.MP_WORLD_POSITION_OFFSET):
        node = mel.get_material_property_input_node(mat, prop)
        unreal.log(f'[PlaceholderMat] {prop}: node={(node.get_name() if node else "None")}')
