import unreal

TEMP_DIR = '/Game/__CodexRetroScratch'
TEMP_NAME = 'M_RetroScratch'
FUNCTIONS = [
    '/Game/UE5RFX/Materials/MaterialFunctions/UE5RFX_WPO_VertexSnapping',
    '/Game/UE5RFX/Materials/MaterialFunctions/UE5RFX_WPO_VertexNoise',
    '/Game/UE5RFX/Materials/MaterialFunctions/UE5RFX_UV_AffineMapping',
    '/Game/UE5RFX/Materials/MaterialFunctions/UE5RFX_WPO_Fake_Sprite_Snap',
]

mel = unreal.MaterialEditingLibrary
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

if not unreal.EditorAssetLibrary.does_directory_exist(TEMP_DIR):
    unreal.EditorAssetLibrary.make_directory(TEMP_DIR)

scratch = unreal.EditorAssetLibrary.load_asset(f'{TEMP_DIR}/{TEMP_NAME}')
if not scratch:
    scratch = asset_tools.create_asset(TEMP_NAME, TEMP_DIR, unreal.Material, unreal.MaterialFactoryNew())

unreal.log('[RetroFnInputs] Scratch=' + scratch.get_path_name())

for index, func_path in enumerate(FUNCTIONS):
    func = unreal.EditorAssetLibrary.load_asset(func_path)
    if not func:
        unreal.log('[RetroFnInputs] Missing function ' + func_path)
        continue

    expr = mel.create_material_expression(scratch, unreal.MaterialExpressionMaterialFunctionCall, -1200, index * 320)
    expr.set_editor_property('material_function', func)
    try:
        mel.update_material_function(func)
    except Exception as exc:
        unreal.log('[RetroFnInputs] update_material_function failed for ' + func_path + ': ' + str(exc))

    names = mel.get_material_expression_input_names(expr)
    types = mel.get_material_expression_input_types(expr)
    unreal.log('[RetroFnInputs] Function=' + func_path)
    unreal.log('[RetroFnInputs] Inputs=' + str(list(names)))
    unreal.log('[RetroFnInputs] Types=' + str(list(types)))
    try:
        inputs = mel.get_inputs_for_material_expression(scratch, expr)
        unreal.log('[RetroFnInputs] InputExprs=' + str(inputs))
    except Exception as exc:
        unreal.log('[RetroFnInputs] get_inputs_for_material_expression failed: ' + str(exc))
    try:
        output_name = mel.get_input_node_output_name_for_material_expression(expr)
        unreal.log('[RetroFnInputs] Output=' + str(output_name))
    except Exception as exc:
        unreal.log('[RetroFnInputs] Output failed: ' + str(exc))

unreal.EditorAssetLibrary.delete_asset(f'{TEMP_DIR}/{TEMP_NAME}')
unreal.EditorAssetLibrary.delete_directory(TEMP_DIR)
