import unreal

LOG = '[PromoteIntegratedVariants]'
PAIRS = [
    ('/Game/Materials/M_Character_Unlit', '/Game/Materials/Retro/M_Character_Unlit_RetroGeometry'),
    ('/Game/Materials/M_Environment_Unlit', '/Game/Materials/Retro/M_Environment_Unlit_RetroGeometry'),
    ('/Game/Materials/M_FBX_Unlit', '/Game/Materials/Retro/M_FBX_Unlit_RetroGeometry'),
    ('/Game/Materials/M_GLB_Unlit', '/Game/Materials/Retro/M_GLB_Unlit_RetroGeometry'),
]
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()


def log(text):
    unreal.log(f'{LOG} {text}')


def main():
    for source_path, target_path in PAIRS:
        if not unreal.EditorAssetLibrary.does_asset_exist(source_path):
            raise RuntimeError(f'Missing source asset: {source_path}')

        if unreal.EditorAssetLibrary.does_asset_exist(target_path):
            if not unreal.EditorAssetLibrary.delete_asset(target_path):
                raise RuntimeError(f'Failed to delete existing target asset: {target_path}')

        source_asset = unreal.EditorAssetLibrary.load_asset(source_path)
        if not source_asset:
            raise RuntimeError(f'Failed to load source asset: {source_path}')

        package_path, asset_name = target_path.rsplit('/', 1)
        duplicated = asset_tools.duplicate_asset(asset_name, package_path, source_asset)
        if not duplicated:
            raise RuntimeError(f'Failed to duplicate {source_path} -> {target_path}')

        if not unreal.EditorAssetLibrary.save_loaded_asset(duplicated):
            raise RuntimeError(f'Failed to save duplicated asset: {target_path}')

        log(f'Promoted {source_path} -> {target_path}')

    log('Integrated masters promoted to retro variants')


if __name__ == '__main__':
    main()
