import bpy
bpy.ops.wm.open_mainfile(filepath=r'C:\UE\T66\SourceAssets\Import\VFX\Idol_Fire_Explosion\FireExplosion_VDB_Source.blend')
for obj in bpy.data.objects:
    mods = [(m.name, m.type, getattr(m, 'fluid_type', None)) for m in obj.modifiers]
    if mods:
        print(obj.name, mods)
        for m in obj.modifiers:
            if m.type == 'FLUID':
                if m.fluid_type == 'FLOW':
                    fs = m.flow_settings
                    print('  flow', fs.flow_type, fs.flow_behavior, tuple(fs.velocity_coord), fs.fuel_amount, fs.density, fs.temperature)
                elif m.fluid_type == 'DOMAIN':
                    ds = m.domain_settings
                    print('  domain', ds.domain_type, ds.cache_directory, ds.resolution_max)
