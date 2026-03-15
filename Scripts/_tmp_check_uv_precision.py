import unreal
assets = [
    '/Game/World/Props/Tree2.Tree2',
    '/Game/World/Props/Rock.Rock',
    '/Game/World/LootBags/Black/SM_LootBag_Black.SM_LootBag_Black',
    '/Game/World/LootBags/Red/SM_LootBag_Red.SM_LootBag_Red',
    '/Game/World/LootBags/Yellow/SM_LootBag_Yellow.SM_LootBag_Yellow',
    '/Game/World/LootBags/White/SM_LootBag_White.SM_LootBag_White',
]
for path in assets:
    mesh = unreal.EditorAssetLibrary.load_asset(path)
    print(path)
    if not mesh:
        print('  missing')
        continue
    try:
        sms = mesh.get_editor_property('source_models')
        for i, sm in enumerate(sms):
            bs = sm.get_editor_property('build_settings')
            vals = {}
            for prop in ['use_full_precision_u_vs','use_backwards_compatible_f16_trunc_u_vs','generate_lightmap_u_vs','recompute_normals','recompute_tangents']:
                try:
                    vals[prop] = bs.get_editor_property(prop)
                except Exception as e:
                    vals[prop] = f'ERR:{e}'
            print('  lod', i, vals)
    except Exception as e:
        print('  source_models err', e)
