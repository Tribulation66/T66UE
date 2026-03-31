import os, shutil, bpy
from mathutils import Vector
out_dir = r'C:\UE\T66\SourceAssets\Import\VFX\Idol_Fire_Explosion\vdb_replay_probe'
if os.path.isdir(out_dir):
    shutil.rmtree(out_dir)
os.makedirs(out_dir, exist_ok=True)

bpy.ops.object.select_all(action='SELECT')
bpy.ops.object.delete(use_global=False)
scene = bpy.context.scene
scene.render.engine = 'CYCLES'
scene.cycles.samples = 24
scene.cycles.use_adaptive_sampling = True
scene.cycles.volume_bounces = 2
scene.render.resolution_x = 640
scene.render.resolution_y = 640
scene.render.image_settings.file_format = 'PNG'
scene.frame_start = 1
scene.frame_end = 16
scene.view_settings.look = 'AgX - High Contrast'
scene.view_settings.exposure = 0.2

world = bpy.data.worlds.new('W')
world.use_nodes = True
world.node_tree.nodes['Background'].inputs[0].default_value = (0.008, 0.010, 0.016, 1.0)
world.node_tree.nodes['Background'].inputs[1].default_value = 0.2
scene.world = world

bpy.ops.mesh.primitive_plane_add(size=16, location=(0,0,-0.02))
plane = bpy.context.object
matg = bpy.data.materials.new('Ground')
matg.use_nodes = True
plane.data.materials.append(matg)
matg.node_tree.nodes['Principled BSDF'].inputs['Base Color'].default_value = (0.02,0.02,0.025,1)

cam = bpy.data.cameras.new('Cam')
cam_obj = bpy.data.objects.new('Cam', cam)
bpy.context.collection.objects.link(cam_obj)
cam_obj.location = (0,-8,3)
direction = Vector((0,0,1.2)) - cam_obj.location
cam_obj.rotation_euler = direction.to_track_quat('-Z','Y').to_euler()
scene.camera = cam_obj

light = bpy.data.lights.new('Key','AREA')
light.energy = 1800
light.shape = 'RECTANGLE'
light.size = 6
light_obj = bpy.data.objects.new('Key', light)
bpy.context.collection.objects.link(light_obj)
light_obj.location = (0,-4,4.5)
light_obj.rotation_euler = (1.0,0,0)

bpy.ops.mesh.primitive_uv_sphere_add(location=(0,0,0.7), segments=48, ring_count=24)
flow = bpy.context.object
flow.scale = (0.8,0.8,0.7)

bpy.ops.object.select_all(action='DESELECT')
flow.select_set(True)
bpy.context.view_layer.objects.active = flow
bpy.ops.object.quick_smoke(style='FIRE')

domain = bpy.data.objects['Smoke Domain']
flow_mod = next(mod for mod in flow.modifiers if mod.type == 'FLUID')
domain_mod = next(mod for mod in domain.modifiers if mod.type == 'FLUID')
fs = flow_mod.flow_settings
ds = domain_mod.domain_settings

pv = next(node for node in domain.active_material.node_tree.nodes if node.bl_idname == 'ShaderNodeVolumePrincipled')
pv.inputs['Blackbody Intensity'].default_value = 2.5
pv.inputs['Emission Strength'].default_value = 0.5
if len(pv.inputs) >= 13:
    pv.inputs[12].default_value = 1.0

fs.flow_type = 'BOTH'
fs.flow_behavior = 'INFLOW'
fs.use_initial_velocity = True
fs.velocity_coord = (0.0, 0.0, 1.4)
fs.velocity_normal = 1.1
fs.velocity_random = 0.4
fs.surface_distance = 1.0

# keep replay mode and advance frames to force simulation evaluation
print('CACHE_TYPE', ds.cache_type)
for frame, fuel, temp, density in ((1,0.0,0.0,0.0),(2,1.4,1.6,0.9),(3,1.2,1.4,0.7),(4,0.4,0.5,0.2),(5,0.0,0.0,0.0)):
    fs.fuel_amount = fuel
    fs.temperature = temp
    fs.density = density
    fs.keyframe_insert(data_path='fuel_amount', frame=frame)
    fs.keyframe_insert(data_path='temperature', frame=frame)
    fs.keyframe_insert(data_path='density', frame=frame)

for frame in range(1, 8):
    scene.frame_set(frame)
    print('FRAME', frame, 'density_grid_len', len(ds.density_grid), 'flame_grid_len', len(ds.flame_grid), 'temp_grid_len', len(ds.temperature_grid))

flow.hide_render = True
flow.hide_viewport = True
scene.frame_set(6)
scene.render.filepath = os.path.join(out_dir, 'replay_probe_render.png')
print(bpy.ops.render.render(write_still=True))
