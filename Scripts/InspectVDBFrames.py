import bpy
bpy.ops.wm.open_mainfile(filepath=r'C:\UE\T66\SourceAssets\Import\VFX\Idol_Fire_Explosion\FireExplosion_VDB_Source.blend')
scene = bpy.context.scene
for frame in [4,7,10,13]:
    scene.frame_set(frame)
    domain = bpy.data.objects['FireExplosionDomain']
    print('FRAME', frame, 'bbox_z', [tuple(v) for v in domain.bound_box][:2], 'hide_render', domain.hide_render)
    # sample fluid cache presence from domain settings
    ds = next(mod for mod in domain.modifiers if mod.type == 'FLUID').domain_settings
    print('  cache dir', ds.cache_directory, 'res', ds.resolution_max)
