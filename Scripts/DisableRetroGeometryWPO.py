import unreal

LOG = '[RetroWPOOff]'
MATERIALS = [
    '/Game/Materials/M_Character_Unlit',
    '/Game/Materials/M_Environment_Unlit',
    '/Game/Materials/M_FBX_Unlit',
    '/Game/Materials/M_GLB_Unlit',
]
mel = unreal.MaterialEditingLibrary


def log(text):
    unreal.log(f'{LOG} {text}')


def main():
    for material_path in MATERIALS:
        material = unreal.EditorAssetLibrary.load_asset(material_path)
        if not material:
            raise RuntimeError(f'Missing material: {material_path}')

        zero = mel.create_material_expression(material, unreal.MaterialExpressionConstant3Vector, 5200, 320)
        zero.set_editor_property('constant', unreal.LinearColor(0.0, 0.0, 0.0, 0.0))
        mel.connect_material_property(zero, '', unreal.MaterialProperty.MP_WORLD_POSITION_OFFSET)
        mel.recompile_material(material)
        unreal.EditorAssetLibrary.save_loaded_asset(material)
        log(f'Connected zero WPO for {material_path}')

    log('Retro geometry WPO rollback complete')


if __name__ == '__main__':
    main()
