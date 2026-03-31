import os, shutil, bpy
out_dir = r'C:\UE\T66\SourceAssets\Import\VFX\Idol_Fire_Explosion\default_probe'
if os.path.isdir(out_dir):
    shutil.rmtree(out_dir)
os.makedirs(out_dir, exist_ok=True)
bpy.ops.object.select_all(action='SELECT')
bpy.ops.object.delete(use_global=False)
scene = bpy.context.scene
scene.render.engine = 'CYCLES'
scene.cycles.samples = 64
scene.render.resolution_x = 768
scene.render.resolution_y = 768
scene.render.image_settings.file_format = 'PNG'
scene.render.image_settings.color_mode = 'RGBA'
scene.frame_start = 1
scene.frame_end = 12
bpy.ops.mesh.primitive_plane_add(size=16, location=(0,0,-0.02))
plane = bpy.context.object
bpy.ops.mesh.primitive_uv_sphere_add(location=(0,0,0.7))
flow = bpy.context.object
flow.scale = (0.8,0.8,0.8)
bpy.ops.object.select_all(action='DESELECT')
flow.select_set(True)
bpy.context.view_layer.objects.active = flow
bpy.ops.object.quick_smoke()
domain = bpy.data.objects['Smoke Domain']
domain.location.z = 1.2
domain.scale = (2.2,2.2,2.6)
flow_mod = next(mod for mod in flow.modifiers if mod.type == 'FLUID')
domain_mod = next(mod for mod in domain.modifiers if mod.type == 'FLUID')
fs = flow_mod.flow_settings
ds = domain_mod.domain_settings
fs.flow_type = 'BOTH'
fs.flow_behavior = 'GEOMETRY'
fs.use_initial_velocity = True
fs.velocity_coord = (0.0,0.0,1.2)
fs.surface_distance = 0.0
fs.subframes = 1
fs.fuel_amount = 1.4
fs.temperature = 1.6
fs.density = 0.8
ds.cache_type = 'ALL'
ds.cache_data_format = 'OPENVDB'
ds.cache_directory = out_dir
ds.resolution_max = 48
ds.use_noise = True
ds.vorticity = 0.35
ds.flame_vorticity = 1.0
ds.burning_rate = 1.2
ds.flame_smoke = 0.8
ds.flame_ignition = 1.2
ds.flame_max_temp = 3.5
ds.use_adaptive_domain = True
ds.adapt_margin = 3
ds.adapt_threshold = 0.01
cam = bpy.data.cameras.new('Cam')
cam_obj = bpy.data.objects.new('Cam', cam)
bpy.context.collection.objects.link(cam_obj)
cam_obj.location = (0,-8,3)
cam_obj.rotation_euler = (1.15, 0.0, 0.0)
scene.camera = cam_obj
bpy.context.view_layer.objects.active = domain
print('BAKE')
print(bpy.ops.fluid.bake_all())
flow.hide_render = True
flow.hide_viewport = True
scene.frame_set(7)
for obj in [domain]:
    print('DOMAIN_MAT', obj.active_material.name if obj.active_material else None)
    if obj.active_material:
        for node in obj.active_material.node_tree.nodes:
            print('node', node.bl_idname)
scene.render.filepath = os.path.join(out_dir, 'default_render.png')
print('RENDER')
print(bpy.ops.render.render(write_still=True))
