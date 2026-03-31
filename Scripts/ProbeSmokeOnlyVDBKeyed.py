import os
import shutil
import sys

import bpy


OUT_DIR = r"C:\UE\T66\SourceAssets\Import\VFX\Idol_Fire_Explosion\smoke_only_keyed_probe"

if os.path.isdir(OUT_DIR):
    shutil.rmtree(OUT_DIR)
os.makedirs(OUT_DIR, exist_ok=True)

bpy.ops.object.select_all(action="SELECT")
bpy.ops.object.delete(use_global=False)

scene = bpy.context.scene
scene.frame_start = 1
scene.frame_end = 12

bpy.ops.mesh.primitive_uv_sphere_add(location=(0.0, 0.0, 0.7), segments=48, ring_count=24)
flow = bpy.context.object
flow.scale = (0.8, 0.8, 0.8)

bpy.ops.object.select_all(action="DESELECT")
flow.select_set(True)
bpy.context.view_layer.objects.active = flow
bpy.ops.object.quick_smoke(style="SMOKE")

domain = bpy.data.objects["Smoke Domain"]
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
for frame, value in ((1, 0.0), (2, 1.6), (3, 1.2), (4, 0.5), (5, 0.0)):
    fs.density = value
    fs.keyframe_insert(data_path="density", frame=frame)
for frame, value in ((1, 0.0), (2, 1.8), (3, 1.5), (4, 0.6), (5, 0.0)):
    fs.temperature = value
    fs.keyframe_insert(data_path="temperature", frame=frame)

ds.cache_type = "ALL"
ds.cache_data_format = "OPENVDB"
ds.cache_directory = OUT_DIR
ds.cache_frame_start = 1
ds.cache_frame_end = 12
ds.resolution_max = 64
ds.use_noise = True

bpy.context.view_layer.objects.active = domain
print("BAKE", bpy.ops.fluid.bake_all())

sys.path.insert(0, r"C:\Program Files\Blender Foundation\Blender 5.0\5.0\python\lib\site-packages")
import openvdb

path = os.path.join(OUT_DIR, "data", "fluid_data_0006.vdb")
grids, _ = openvdb.readAll(path)
for grid in grids:
    print("GRID", grid.name, "active", grid.activeVoxelCount(), "minmax", grid.evalMinMax())
