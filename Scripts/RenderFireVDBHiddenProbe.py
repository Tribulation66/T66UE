import bpy
bpy.ops.wm.open_mainfile(filepath=r'C:\UE\T66\SourceAssets\Import\VFX\Idol_Fire_Explosion\FireExplosion_VDB_Source.blend')
for name in ['CoreBurst','ShellBurst','AsymLobe']:
    obj = bpy.data.objects[name]
    obj.hide_render = True
    obj.hide_viewport = True
scene = bpy.context.scene
scene.camera = bpy.data.objects['CameraHero']
scene.frame_set(7)
scene.render.filepath = r'C:\UE\T66\SourceAssets\Import\VFX\Idol_Fire_Explosion\vdb_previews\hero_f07_hidden.png'
bpy.ops.render.render(write_still=True)
print('rendered')
