import bpy, os, shutil
from mathutils import Vector
out_dir = r'C:\UE\T66\SourceAssets\Import\VFX\Idol_Fire_Explosion\vdb_import_probe'
if os.path.isdir(out_dir):
    shutil.rmtree(out_dir)
os.makedirs(out_dir, exist_ok=True)

bpy.ops.object.select_all(action='SELECT')
bpy.ops.object.delete(use_global=False)
scene = bpy.context.scene
scene.render.engine = 'CYCLES'
scene.cycles.samples = 32
scene.cycles.use_adaptive_sampling = True
scene.cycles.volume_bounces = 2
scene.render.resolution_x = 640
scene.render.resolution_y = 640
scene.render.image_settings.file_format = 'PNG'
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
cam_obj.location = (0,-7.5,3.0)
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

vdb_path = r'C:\UE\T66\SourceAssets\Import\VFX\Idol_Fire_Explosion\grid_probe\data\fluid_data_0006.vdb'
print('IMPORT', vdb_path, os.path.getsize(vdb_path))
bpy.ops.object.volume_import(filepath=vdb_path, directory=os.path.dirname(vdb_path), files=[{'name': os.path.basename(vdb_path)}])
vol = bpy.context.object
vol.location = (0,0,1.2)
vol.scale = (2.3,2.3,2.3)
print('grids', [g.name for g in vol.data.grids])

mat = bpy.data.materials.new('VDBMat')
mat.use_nodes = True
nodes = mat.node_tree.nodes
links = mat.node_tree.links
nodes.clear()
out = nodes.new('ShaderNodeOutputMaterial')
out.location = (400,0)
attr_density = nodes.new('ShaderNodeAttribute')
attr_density.location = (-900,180)
attr_density.attribute_name = 'density'
attr_flame = nodes.new('ShaderNodeAttribute')
attr_flame.location = (-900,-40)
attr_flame.attribute_name = 'flame'
attr_temp = nodes.new('ShaderNodeAttribute')
attr_temp.location = (-900,-260)
attr_temp.attribute_name = 'temperature'
dmul = nodes.new('ShaderNodeMath')
dmul.location = (-650,180)
dmul.operation = 'MULTIPLY'
dmul.inputs[1].default_value = 3.0
fmul = nodes.new('ShaderNodeMath')
fmul.location = (-650,-40)
fmul.operation = 'MULTIPLY'
fmul.inputs[1].default_value = 3.0
ramp = nodes.new('ShaderNodeValToRGB')
ramp.location = (-420,-40)
ramp.color_ramp.elements[0].color = (0.1,0.02,0.0,1)
ramp.color_ramp.elements[1].color = (1,0.75,0.2,1)
pv = nodes.new('ShaderNodeVolumePrincipled')
pv.location = (120,0)
pv.inputs['Color'].default_value = (0.08,0.06,0.05,1)
pv.inputs['Absorption Color'].default_value = (0.24,0.10,0.04,1)
pv.inputs['Blackbody Intensity'].default_value = 1.5
pvi = pv.inputs.get('Weight')
if pvi is not None:
    pvi.default_value = 1.0
links.new(attr_density.outputs['Fac'], dmul.inputs[0])
links.new(attr_flame.outputs['Fac'], fmul.inputs[0])
links.new(fmul.outputs['Value'], ramp.inputs['Fac'])
links.new(dmul.outputs['Value'], pv.inputs['Density'])
links.new(ramp.outputs['Color'], pv.inputs['Emission Color'])
links.new(fmul.outputs['Value'], pv.inputs['Emission Strength'])
out.inputs['Volume'].default_value
links.new(pv.outputs['Volume'], out.inputs['Volume'])
vol.data.materials.append(mat)

scene.render.filepath = os.path.join(out_dir, 'vdb_import_render.png')
print(bpy.ops.render.render(write_still=True))
