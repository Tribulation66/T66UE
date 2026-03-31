import os
import shutil
import sys

import bpy


OUT_DIR = r"C:\UE\T66\SourceAssets\Import\VFX\Idol_Fire_Explosion\smoke_only_probe"

if os.path.isdir(OUT_DIR):
    shutil.rmtree(OUT_DIR)
os.makedirs(OUT_DIR, exist_ok=True)

bpy.ops.object.select_all(action="SELECT")
bpy.ops.object.delete(use_global=False)

scene = bpy.context.scene
scene.frame_start = 1
scene.frame_end = 12
scene.render.engine = "CYCLES"
scene.cycles.samples = 48
scene.cycles.use_adaptive_sampling = True
scene.cycles.volume_bounces = 2
scene.render.resolution_x = 640
scene.render.resolution_y = 640
scene.render.image_settings.file_format = "PNG"
scene.render.image_settings.color_mode = "RGBA"
scene.view_settings.look = "AgX - High Contrast"
scene.view_settings.exposure = 0.25

world = bpy.data.worlds.new("SmokeOnlyWorld")
world.use_nodes = True
world.node_tree.nodes["Background"].inputs[0].default_value = (0.01, 0.01, 0.015, 1.0)
world.node_tree.nodes["Background"].inputs[1].default_value = 0.25
scene.world = world

bpy.ops.mesh.primitive_plane_add(size=16, location=(0.0, 0.0, -0.02))
ground = bpy.context.object
ground_mat = bpy.data.materials.new("SmokeOnlyGround")
ground_mat.use_nodes = True
ground_bsdf = ground_mat.node_tree.nodes["Principled BSDF"]
ground_bsdf.inputs["Base Color"].default_value = (0.02, 0.02, 0.025, 1.0)
ground_bsdf.inputs["Roughness"].default_value = 0.75
ground.data.materials.append(ground_mat)

bpy.ops.mesh.primitive_uv_sphere_add(location=(0.0, 0.0, 0.7), segments=48, ring_count=24)
flow = bpy.context.object
flow.scale = (0.8, 0.8, 0.8)

bpy.ops.object.select_all(action="DESELECT")
flow.select_set(True)
bpy.context.view_layer.objects.active = flow
bpy.ops.object.quick_smoke(style="SMOKE")

domain = bpy.data.objects["Smoke Domain"]
domain.location.z = 1.2
domain.scale = (2.2, 2.2, 2.6)
flow_mod = next(mod for mod in flow.modifiers if mod.type == "FLUID")
domain_mod = next(mod for mod in domain.modifiers if mod.type == "FLUID")
fs = flow_mod.flow_settings
ds = domain_mod.domain_settings

fs.flow_type = "SMOKE"
fs.flow_behavior = "GEOMETRY"
fs.use_initial_velocity = True
fs.velocity_coord = (0.0, 0.0, 1.2)
fs.velocity_normal = 1.0
fs.velocity_random = 0.2
fs.density = 1.0

ds.cache_type = "ALL"
ds.cache_data_format = "OPENVDB"
ds.cache_directory = OUT_DIR
ds.cache_frame_start = 1
ds.cache_frame_end = 12
ds.resolution_max = 64
ds.use_noise = True

cam = bpy.data.cameras.new("SmokeOnlyCam")
cam_obj = bpy.data.objects.new("SmokeOnlyCam", cam)
bpy.context.collection.objects.link(cam_obj)
cam_obj.location = (0.0, -8.0, 3.0)
cam_obj.rotation_euler = (1.15, 0.0, 0.0)
scene.camera = cam_obj

light = bpy.data.lights.new("SmokeOnlyKey", "AREA")
light.energy = 1800
light.shape = "RECTANGLE"
light.size = 6.0
light_obj = bpy.data.objects.new("SmokeOnlyKey", light)
bpy.context.collection.objects.link(light_obj)
light_obj.location = (0.0, -4.0, 4.5)
light_obj.rotation_euler = (0.95, 0.0, 0.0)

def build_volume_material(name: str):
    mat = bpy.data.materials.new(name)
    mat.use_nodes = True
    nodes = mat.node_tree.nodes
    links = mat.node_tree.links
    nodes.clear()

    output = nodes.new("ShaderNodeOutputMaterial")
    output.location = (380, 0)

    pv = nodes.new("ShaderNodeVolumePrincipled")
    pv.location = (40, 0)
    pv.inputs["Color"].default_value = (0.10, 0.07, 0.05, 1.0)
    pv.inputs["Density"].default_value = 3.0
    pv.inputs["Density Attribute"].default_value = "density"
    pv.inputs["Absorption Color"].default_value = (0.22, 0.11, 0.04, 1.0)
    pv.inputs["Blackbody Intensity"].default_value = 1.7
    pv.inputs["Temperature"].default_value = 1200.0
    pv.inputs["Temperature Attribute"].default_value = "temperature"
    pv.inputs["Emission Strength"].default_value = 1.2
    pv.inputs["Anisotropy"].default_value = 0.08
    links.new(pv.outputs["Volume"], output.inputs["Volume"])
    return mat

bpy.context.view_layer.objects.active = domain
print("BAKE", bpy.ops.fluid.bake_all())

for frame in (1, 2, 4, 6, 8, 12):
    scene.frame_set(frame)
    print("FRAME", frame, "density_len", len(ds.density_grid), "vel_len", len(ds.velocity_grid))

sys.path.insert(0, r"C:\Program Files\Blender Foundation\Blender 5.0\5.0\python\lib\site-packages")
import openvdb

path = os.path.join(OUT_DIR, "data", "fluid_data_0006.vdb")
grids, _ = openvdb.readAll(path)
for grid in grids:
    print("GRID", grid.name, "active", grid.activeVoxelCount(), "minmax", grid.evalMinMax())

scene.frame_set(6)
flow.hide_render = True
flow.hide_viewport = True
domain.hide_render = True
domain.hide_viewport = True

bpy.ops.object.volume_import(filepath=path)
volume_obj = bpy.context.object
volume_obj.location = domain.location
volume_obj.scale = domain.scale
volume_obj.data.grids.load()
volume_obj.data.materials.clear()
volume_obj.data.materials.append(build_volume_material("SmokeOnlyImportedVDB"))
print("IMPORTED_GRIDS", [grid.name for grid in volume_obj.data.grids])

scene.render.filepath = os.path.join(OUT_DIR, "render_f06.png")
print("RENDER", bpy.ops.render.render(write_still=True))
