import os, shutil, bpy
out_dir = r'C:\UE\T66\SourceAssets\Import\VFX\Idol_Fire_Explosion\grid_probe'
if os.path.isdir(out_dir):
    shutil.rmtree(out_dir)
os.makedirs(out_dir, exist_ok=True)
bpy.ops.object.select_all(action='SELECT')
bpy.ops.object.delete(use_global=False)
scene = bpy.context.scene
scene.frame_start = 1
scene.frame_end = 12
bpy.ops.mesh.primitive_uv_sphere_add(location=(0,0,0.7))
flow = bpy.context.object
flow.scale = (0.8,0.8,0.8)
bpy.ops.object.select_all(action='DESELECT')
flow.select_set(True)
bpy.context.view_layer.objects.active = flow
bpy.ops.object.quick_smoke(style='FIRE')
domain = bpy.data.objects['Smoke Domain']
flow_mod = next(mod for mod in flow.modifiers if mod.type == 'FLUID')
domain_mod = next(mod for mod in domain.modifiers if mod.type == 'FLUID')
fs = flow_mod.flow_settings
ds = domain_mod.domain_settings
print('flow_source?', getattr(fs, 'flow_source', '<na>'))
fs.flow_type = 'BOTH'
fs.flow_behavior = 'GEOMETRY'
fs.use_initial_velocity = True
fs.velocity_coord = (0.0,0.0,1.2)
fs.surface_distance = 0.0
fs.fuel_amount = 2.0
fs.temperature = 2.0
fs.density = 1.0
ds.cache_type = 'ALL'
ds.cache_data_format = 'OPENVDB'
ds.cache_directory = out_dir
ds.cache_frame_start = 1
ds.cache_frame_end = 12
ds.resolution_max = 64
ds.use_noise = True
ds.vorticity = 0.6
ds.flame_vorticity = 1.5
ds.burning_rate = 1.0
ds.flame_smoke = 0.5
ds.flame_ignition = 1.0
ds.flame_max_temp = 3.0
bpy.context.view_layer.objects.active = domain
print('BAKE', bpy.ops.fluid.bake_all())
for frame in [1,2,4,8,12]:
    scene.frame_set(frame)
    print('FRAME', frame, 'density_len', len(ds.density_grid), 'flame_len', len(ds.flame_grid), 'temp_len', len(ds.temperature_grid))
    if len(ds.density_grid):
        arr = list(ds.density_grid[:20])
        print(' density sample', arr)
        print(' max20', max(arr))
