import unreal

mel = unreal.MaterialEditingLibrary
mat = unreal.EditorAssetLibrary.load_asset('/Game/Materials/M_Environment_Unlit')
root = mel.get_material_property_input_node(mat, unreal.MaterialProperty.MP_EMISSIVE_COLOR)
unreal.log('[ExprInspect] root=' + (root.get_name() if root else 'None'))
if root:
    inputs = mel.get_inputs_for_material_expression(mat, root)
    unreal.log('[ExprInspect] inputs type=' + str(type(inputs)) + ' len=' + str(len(inputs) if inputs is not None else 'None'))
    unreal.log('[ExprInspect] inputs repr=' + repr(inputs))
    for idx, item in enumerate(inputs or []):
        unreal.log(f'[ExprInspect] item {idx} type=' + str(type(item)) + ' repr=' + repr(item))
        try:
            unreal.log(f'[ExprInspect] item {idx} class=' + item.get_class().get_name())
        except Exception:
            pass
