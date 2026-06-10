import unreal

for path in [
    "/Game/Characters/MotionRig/Hero_1/SK_MotionRig_Hero1",
    "/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/SM_Hero_1_Chad_Male",
    "/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/Skeletal/SK_Hero_1_Chad_Male_FriendSlop",
]:
    asset = unreal.EditorAssetLibrary.load_asset(path)
    if asset:
        b = asset.get_bounds()
        o, e = b.origin, b.box_extent
        print(f"HERO_BOUNDS {path.split('/')[-1]} origin_z={o.z:.1f} extent=({e.x:.1f},{e.y:.1f},{e.z:.1f}) height={2*e.z:.1f} top={o.z+e.z:.1f} bottom={o.z-e.z:.1f}")
    else:
        print(f"HERO_BOUNDS {path.split('/')[-1]} MISSING")
