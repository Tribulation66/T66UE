import os

import bpy


VDB_PATH = r"C:\UE\T66\SourceAssets\Import\VFX\Idol_Fire_Explosion\smoke_only_scale_probe\data\fluid_data_0004.vdb"
OUT_PATH = r"C:\UE\T66\SourceAssets\Import\VFX\Idol_Fire_Explosion\smoke_only_scale_probe\import_render_f04.png"


bpy.ops.object.select_all(action="SELECT")
bpy.ops.object.delete(use_global=False)

scene = bpy.context.scene
scene.render.engine = "CYCLES"
scene.cycles.samples = 48
scene.cycles.use_adaptive_sampling = True
scene.cycles.volume_bounces = 3
scene.render.resolution_x = 768
scene.render.resolution_y = 768
scene.render.image_settings.file_format = "PNG"
scene.render.image_settings.color_mode = "RGBA"
scene.view_settings.look = "AgX - High Contrast"
scene.view_settings.exposure = 0.2

world = bpy.data.worlds.new("ImportProbeWorld")
world.use_nodes = True
world.node_tree.nodes["Background"].inputs[0].default_value = (0.008, 0.010, 0.018, 1.0)
world.node_tree.nodes["Background"].inputs[1].default_value = 0.25
scene.world = world

bpy.ops.mesh.primitive_plane_add(size=16, location=(0.0, 0.0, -0.02))
ground = bpy.context.object
ground_mat = bpy.data.materials.new("Ground")
ground_mat.use_nodes = True
ground_bsdf = ground_mat.node_tree.nodes["Principled BSDF"]
ground_bsdf.inputs["Base Color"].default_value = (0.018, 0.02, 0.024, 1.0)
ground_bsdf.inputs["Roughness"].default_value = 0.72
ground.data.materials.append(ground_mat)

cam = bpy.data.cameras.new("CameraHero")
cam_obj = bpy.data.objects.new("CameraHero", cam)
bpy.context.collection.objects.link(cam_obj)
cam_obj.location = (0.0, -8.1, 3.5)
cam_obj.rotation_euler = (1.18, 0.0, 0.0)
scene.camera = cam_obj

light = bpy.data.lights.new("ExplosionKey", "AREA")
light.energy = 2200
light.shape = "RECTANGLE"
light.size = 8.0
light.size_y = 5.0
light_obj = bpy.data.objects.new("ExplosionKey", light)
bpy.context.collection.objects.link(light_obj)
light_obj.location = (0.0, -4.0, 5.2)
light_obj.rotation_euler = (0.95, 0.0, 0.0)

bpy.ops.object.volume_import(filepath=VDB_PATH)
vol = bpy.context.object
vol.location = (0.0, 0.0, 1.2)
vol.scale = (2.2, 2.2, 2.6)
vol.data.grids.load()

mat = bpy.data.materials.new("ImportedVDBMat")
mat.use_nodes = True
nodes = mat.node_tree.nodes
links = mat.node_tree.links
nodes.clear()

output = nodes.new("ShaderNodeOutputMaterial")
output.location = (500, 100)

volume_info = nodes.new("ShaderNodeVolumeInfo")
volume_info.location = (-850, 120)

density_mul = nodes.new("ShaderNodeMath")
density_mul.location = (-620, 240)
density_mul.operation = "MULTIPLY"
density_mul.inputs[1].default_value = 5.5

temp_mul = nodes.new("ShaderNodeMath")
temp_mul.location = (-620, 20)
temp_mul.operation = "MULTIPLY"
temp_mul.inputs[1].default_value = 1.8

ramp = nodes.new("ShaderNodeValToRGB")
ramp.location = (-380, 0)
ramp.color_ramp.elements[0].position = 0.0
ramp.color_ramp.elements[0].color = (0.08, 0.01, 0.00, 1.0)
ramp.color_ramp.elements[1].position = 1.0
ramp.color_ramp.elements[1].color = (1.0, 0.86, 0.32, 1.0)
for pos, color in (
    (0.22, (0.35, 0.03, 0.00, 1.0)),
    (0.48, (0.95, 0.14, 0.02, 1.0)),
    (0.76, (1.0, 0.46, 0.06, 1.0)),
):
    elem = ramp.color_ramp.elements.new(pos)
    elem.color = color

pv = nodes.new("ShaderNodeVolumePrincipled")
pv.location = (-40, 120)
pv.inputs["Color"].default_value = (0.08, 0.05, 0.04, 1.0)
pv.inputs["Absorption Color"].default_value = (0.18, 0.09, 0.04, 1.0)
pv.inputs["Anisotropy"].default_value = 0.08
pv.inputs["Blackbody Intensity"].default_value = 1.0
pv.inputs["Temperature"].default_value = 1200.0

links.new(volume_info.outputs["Density"], density_mul.inputs[0])
links.new(volume_info.outputs["Temperature"], temp_mul.inputs[0])
links.new(temp_mul.outputs["Value"], ramp.inputs["Fac"])
links.new(density_mul.outputs["Value"], pv.inputs["Density"])
links.new(ramp.outputs["Color"], pv.inputs["Emission Color"])
links.new(temp_mul.outputs["Value"], pv.inputs["Emission Strength"])
links.new(volume_info.outputs["Temperature"], pv.inputs["Temperature"])
links.new(pv.outputs["Volume"], output.inputs["Volume"])

vol.data.materials.clear()
vol.data.materials.append(mat)

scene.render.filepath = OUT_PATH
print("GRIDS", [grid.name for grid in vol.data.grids])
print("RENDER", bpy.ops.render.render(write_still=True))
