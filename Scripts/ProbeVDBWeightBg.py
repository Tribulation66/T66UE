import os, shutil, bpy
from mathutils import Vector
out_dir = r'C:\UE\T66\SourceAssets\Import\VFX\Idol_Fire_Explosion\vdb_weight_bg_probe'
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
scene.frame_end = 12
scene.view_settings.look = 'AgX - High Contrast'
scene.view_settings.exposure = 0.2

world = bpy.data.worlds.new('W')
world.use_nodes = True
world.node_tree.nodes['Background'].inputs[0].default_value = (0.008, 0.010, 0.016, 1.0)
world.node_tree.nodes['Background'].inputs[1].default_value = 0.2
scene.world = world

bpy.ops.mesh.primitive_plane_add(size=16, location=(0,0,-0.02))
cam = bpy.data.cameras.new('Cam')
cam_obj = bpy.data.objects.new('Cam', cam)
bpy.context.collection.objects.link(cam_obj)
cam_obj.location = (0,-8,3)
direction = Vector((0,0,1.2)) - cam_obj.location
cam_obj.rotation_euler = direction.to_track_quat('-Z','Y').to_euler()
scene.camera = cam_obj

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

mat = domain.active_material
pv = next(node for node in mat.node_tree.nodes if node.bl_idname == 'ShaderNodeVolumePrincipled')
pv.inputs['Weight'].default_value = 1.0
pv.inputs['Blackbody Intensity'].default_value = 2.5
pv.inputs['Emission Strength'].default_value = 0.5

fs.flow_type = 'BOTH'
fs.flow_behavior = 'INFLOW'
fs.use_initial_velocity = True
fs.velocity_coord = (0.0, 0.0, 1.4)
fs.velocity_normal = 1.1
fs.velocity_random = 0.4
fs.surface_distance = 0.0
for frame, val in ((1,0.0),(2,1.4),(3,1.2),(4,0.4),(5,0.0)):
    fs.fuel_amount = val
    fs.keyframe_insert(data_path='fuel_amount', frame=frame)
for frame, val in ((1,0.0),(2,1.6),(3,1.4),(4,0.5),(5,0.0)):
    fs.temperature = val
    fs.keyframe_insert(data_path='temperature', frame=frame)
for frame, val in ((1,0.0),(2,0.9),(3,0.7),(4,0.2),(5,0.0)):
    fs.density = val
    fs.keyframe_insert(data_path='density', frame=frame)

ds.cache_type = 'ALL'
ds.cache_data_format = 'OPENVDB'
ds.cache_directory = out_dir
ds.cache_frame_start = 1
ds.cache_frame_end = 12
ds.resolution_max = 96
ds.use_noise = True
ds.vorticity = 0.4
ds.flame_vorticity = 1.25
ds.burning_rate = 1.1
ds.flame_smoke = 0.55
ds.flame_ignition = 1.1
ds.flame_max_temp = 3.0
ds.use_adaptive_domain = True
ds.adapt_margin = 3
ds.adapt_threshold = 0.01

bpy.context.view_layer.objects.active = domain
print('BAKE_START')
print(bpy.ops.fluid.bake_all())
flow.hide_render = True
flow.hide_viewport = True
scene.frame_set(6)
scene.render.filepath = os.path.join(out_dir, 'weight_bg_probe_render.png')
print('RENDER_START')
print(bpy.ops.render.render(write_still=True))
print('DONE')
