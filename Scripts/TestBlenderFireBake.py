import os
import shutil

import bpy

PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
OUT_DIR = os.path.join(PROJECT_ROOT, "SourceAssets", "Import", "VFX", "Idol_Fire_Explosion", "test_bake")

if os.path.isdir(OUT_DIR):
    shutil.rmtree(OUT_DIR)
os.makedirs(OUT_DIR, exist_ok=True)

bpy.ops.object.select_all(action="SELECT")
bpy.ops.object.delete(use_global=False)

scene = bpy.context.scene
scene.frame_start = 1
scene.frame_end = 12

bpy.ops.mesh.primitive_uv_sphere_add(location=(0, 0, 0.7))
flow = bpy.context.object
flow.name = "FireFlow"
flow.scale = (0.8, 0.8, 0.8)

bpy.ops.object.select_all(action="DESELECT")
flow.select_set(True)
bpy.context.view_layer.objects.active = flow
bpy.ops.object.quick_smoke()

domain = bpy.data.objects["Smoke Domain"]
domain.location.z = 1.2
domain.scale = (2.2, 2.2, 2.6)

flow_mod = next(mod for mod in flow.modifiers if mod.type == "FLUID")
domain_mod = next(mod for mod in domain.modifiers if mod.type == "FLUID")
fs = flow_mod.flow_settings
ds = domain_mod.domain_settings

fs.flow_type = "BOTH"
fs.flow_behavior = "GEOMETRY"
fs.use_initial_velocity = True
fs.velocity_coord = (0.0, 0.0, 1.2)
fs.surface_distance = 0.0
fs.subframes = 1
fs.fuel_amount = 1.4
fs.temperature = 1.6
fs.density = 0.8

ds.cache_type = "ALL"
ds.cache_data_format = "OPENVDB"
ds.cache_directory = OUT_DIR
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
ds.timesteps_max = 2
ds.timesteps_min = 1
ds.cfl_condition = 4.0

print("bake_dir", ds.cache_directory)
print("bake_start")
bpy.context.view_layer.objects.active = domain
result = bpy.ops.fluid.bake_all()
print("bake_result", result)

for root, _, files in os.walk(OUT_DIR):
    for name in files:
        print("FILE", os.path.join(root, name))
