import bpy
bpy.ops.wm.open_mainfile(filepath=r'C:\UE\T66\SourceAssets\Import\VFX\Idol_Fire_Explosion\FireExplosion_VDB_Source.blend')
for name in ['CoreBurst','ShellBurst','AsymLobe']:
    obj = bpy.data.objects[name]
    obj.hide_render = True
    obj.hide_viewport = True
domain = bpy.data.objects['FireExplosionDomain']
mat = bpy.data.materials.new('ProbeDirect')
mat.use_nodes = True
nodes = mat.node_tree.nodes
links = mat.node_tree.links
nodes.clear()
out = nodes.new('ShaderNodeOutputMaterial')
out.location = (200,0)
pv = nodes.new('ShaderNodeVolumePrincipled')
pv.location = (-100,0)
pv.inputs['Color'].default_value = (0.12, 0.11, 0.10, 1.0)
pv.inputs['Density'].default_value = 7.0
pv.inputs['Density Attribute'].default_value = 'density'
pv.inputs['Absorption Color'].default_value = (0.22, 0.12, 0.05, 1.0)
pv.inputs['Emission Strength'].default_value = 0.3
pv.inputs['Emission Color'].default_value = (1.0, 0.48, 0.14, 1.0)
pv.inputs['Blackbody Intensity'].default_value = 5.0
pv.inputs['Blackbody Tint'].default_value = (1.0, 0.92, 0.86, 1.0)
pv.inputs['Temperature'].default_value = 1200.0
pv.inputs['Temperature Attribute'].default_value = 'temperature'
links.new(pv.outputs['Volume'], out.inputs['Volume'])
domain.data.materials.clear()
domain.data.materials.append(mat)
scene = bpy.context.scene
scene.camera = bpy.data.objects['CameraHero']
scene.frame_set(7)
scene.render.filepath = r'C:\UE\T66\SourceAssets\Import\VFX\Idol_Fire_Explosion\vdb_previews\hero_f07_directprobe.png'
bpy.ops.render.render(write_still=True)
print('rendered')
