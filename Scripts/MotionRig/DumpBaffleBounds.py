import unreal

mesh = unreal.EditorAssetLibrary.load_asset("/Game/World/Terrain/TowerDungeon/Baffles/SM_BaffleTube")
if mesh:
    bounds = mesh.get_bounds()
    origin = bounds.origin
    extent = bounds.box_extent
    print(f"BAFFLE_BOUNDS origin=({origin.x:.1f},{origin.y:.1f},{origin.z:.1f}) extent=({extent.x:.1f},{extent.y:.1f},{extent.z:.1f})")
else:
    print("BAFFLE_BOUNDS asset_missing")
