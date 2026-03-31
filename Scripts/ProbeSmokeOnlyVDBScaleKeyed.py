import os
import shutil
import sys

import bpy


OUT_DIR = r"C:\UE\T66\SourceAssets\Import\VFX\Idol_Fire_Explosion\smoke_only_scale_probe"

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

for frame, scale in (
    (1, (0.15, 0.15, 0.15)),
    (2, (0.55, 0.55, 0.45)),
    (3, (0.95, 0.95, 0.65)),
    (5, (1.10, 1.10, 0.80)),
    (7, (0.45, 0.45, 0.35)),
    (8, (0.05, 0.05, 0.05)),
    (9, (0.01, 0.01, 0.01)),
):
    flow.scale = scale
    flow.keyframe_insert(data_path="scale", frame=frame)

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
fs.velocity_coord = (0.0, 0.0, 1.5)
fs.velocity_normal = 1.2
fs.velocity_random = 0.3
fs.density = 1.2
fs.temperature = 1.7

ds.cache_type = "ALL"
ds.cache_data_format = "OPENVDB"
ds.cache_directory = OUT_DIR
ds.cache_frame_start = 1
ds.cache_frame_end = 12
ds.resolution_max = 64
ds.use_noise = True
ds.use_adaptive_domain = True
ds.adapt_margin = 3
ds.adapt_threshold = 0.01
if hasattr(ds, "use_dissolve_smoke"):
    ds.use_dissolve_smoke = True
if hasattr(ds, "dissolve_speed"):
    ds.dissolve_speed = 6

bpy.context.view_layer.objects.active = domain
print("BAKE", bpy.ops.fluid.bake_all())

sys.path.insert(0, r"C:\Program Files\Blender Foundation\Blender 5.0\5.0\python\lib\site-packages")
import openvdb

for frame in (1, 2, 3, 4, 5, 6, 7, 8, 10, 12):
    path = os.path.join(OUT_DIR, "data", f"fluid_data_{frame:04d}.vdb")
    grids, _ = openvdb.readAll(path)
    density = next(grid for grid in grids if grid.name == "density")
    temp = next(grid for grid in grids if grid.name == "temperature")
    print(
        "FRAME",
        frame,
        "density_active",
        density.activeVoxelCount(),
        "density_minmax",
        density.evalMinMax(),
        "temp_active",
        temp.activeVoxelCount(),
        "temp_minmax",
        temp.evalMinMax(),
    )
