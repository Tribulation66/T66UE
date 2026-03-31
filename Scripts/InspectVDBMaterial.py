import bpy
bpy.ops.wm.open_mainfile(filepath=r'C:\UE\T66\SourceAssets\Import\VFX\Idol_Fire_Explosion\FireExplosion_VDB_Source.blend')
for obj in [bpy.data.objects['FireExplosionDomain']]:
    print(obj.name, obj.type, obj.active_material.name if obj.active_material else None, obj.hide_render)
    print('modifiers', [(m.type, getattr(m,'fluid_type',None)) for m in obj.modifiers])
    if obj.active_material:
        for node in obj.active_material.node_tree.nodes:
            print('node', node.bl_idname, node.name)
