import bpy
bpy.ops.wm.open_mainfile(filepath=r'C:\UE\T66\SourceAssets\Import\VFX\Idol_Fire_Explosion\FireExplosion_VDB_Source.blend')
scene = bpy.context.scene
scene.frame_set(7)
domain = bpy.data.objects['FireExplosionDomain']
for attr in ['density', 'flame', 'temperature']:
    try:
        print('ATTR', attr, domain.data.attributes.get(attr))
    except Exception as e:
        print('ATTRERR', attr, e)
print('vol attrs', [a.name for a in domain.data.attributes])
