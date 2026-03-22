import unreal
for path in [
    '/Game/World/Terrain/Megabonk/SM_MegabonkBlock.SM_MegabonkBlock',
    '/Game/World/Terrain/Megabonk/SM_MegabonkSlope.SM_MegabonkSlope'
]:
    mesh = unreal.load_asset(path)
    if not mesh:
        unreal.log_warning(f'MISSING {path}')
        continue
    bounds = mesh.get_bounds()
    box = mesh.get_bounding_box()
    unreal.log_warning(f'ASSET {path}')
    unreal.log_warning(f'ORIGIN {bounds.origin}')
    unreal.log_warning(f'BOX_EXTENT {bounds.box_extent}')
    unreal.log_warning(f'MIN {box.min}')
    unreal.log_warning(f'MAX {box.max}')
