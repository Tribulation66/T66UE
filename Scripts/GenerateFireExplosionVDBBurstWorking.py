"""
Generate a working OpenVDB fire-burst source asset from the exact proven probe.

This script intentionally stays very close to ProbeSmokeOnlyVDBScaleKeyed.py
because that probe is the known-good bake path in this Blender 5.0 build.
Presentation objects (ground/cameras/lights) are added only after the bake.

Outputs:
- FireExplosion_VDBBurst_Working_Source.blend
- vdb_burst_working_cache
- vdb_burst_working_stats.txt
"""

import os
import shutil
import sys
from mathutils import Vector

import bpy


OUT_DIR = r"C:\UE\T66\SourceAssets\Import\VFX\Idol_Fire_Explosion"
CACHE_DIR = os.path.join(OUT_DIR, "vdb_burst_working_cache")
BLEND_PATH = os.path.join(OUT_DIR, "FireExplosion_VDBBurst_Working_Source.blend")
STATS_PATH = os.path.join(OUT_DIR, "vdb_burst_working_stats.txt")


def set_camera_look_at(obj, target):
    direction = target - obj.location
    obj.rotation_euler = direction.to_track_quat("-Z", "Y").to_euler()


def add_presentation_scene():
    scene = bpy.context.scene
    scene.render.engine = "CYCLES"
    scene.cycles.samples = 64
    scene.cycles.preview_samples = 24
    scene.cycles.use_adaptive_sampling = True
    scene.cycles.volume_bounces = 3
    scene.render.resolution_x = 768
    scene.render.resolution_y = 768
    scene.render.image_settings.file_format = "PNG"
    scene.render.image_settings.color_mode = "RGBA"
    scene.view_settings.look = "AgX - High Contrast"
    scene.view_settings.exposure = 0.10

    world = bpy.data.worlds.new("FireBurstWorld")
    world.use_nodes = True
    bg = world.node_tree.nodes["Background"]
    bg.inputs[0].default_value = (0.008, 0.010, 0.018, 1.0)
    bg.inputs[1].default_value = 0.18
    scene.world = world

    bpy.ops.mesh.primitive_plane_add(size=18, location=(0.0, 0.0, -0.02))
    ground = bpy.context.object
    ground.name = "Ground"
    ground_mat = bpy.data.materials.new("Ground")
    ground_mat.use_nodes = True
    ground_bsdf = ground_mat.node_tree.nodes["Principled BSDF"]
    ground_bsdf.inputs["Base Color"].default_value = (0.018, 0.02, 0.024, 1.0)
    ground_bsdf.inputs["Roughness"].default_value = 0.72
    ground.data.materials.append(ground_mat)

    hero_data = bpy.data.cameras.new("CameraHero")
    hero = bpy.data.objects.new("CameraHero", hero_data)
    bpy.context.collection.objects.link(hero)
    hero.location = (0.0, -5.9, 2.8)
    set_camera_look_at(hero, Vector((0.0, 0.0, 1.15)))
    scene.camera = hero

    side_data = bpy.data.cameras.new("CameraSide")
    side = bpy.data.objects.new("CameraSide", side_data)
    bpy.context.collection.objects.link(side)
    side.location = (5.5, 0.0, 2.2)
    set_camera_look_at(side, Vector((0.0, 0.0, 1.15)))

    key_data = bpy.data.lights.new("ExplosionKey", "AREA")
    key_data.energy = 1600
    key_data.shape = "RECTANGLE"
    key_data.size = 7.0
    key_data.size_y = 5.0
    key = bpy.data.objects.new("ExplosionKey", key_data)
    bpy.context.collection.objects.link(key)
    key.location = (0.0, -4.2, 4.8)
    set_camera_look_at(key, Vector((0.0, 0.0, 1.1)))

    rim_data = bpy.data.lights.new("ExplosionRim", "AREA")
    rim_data.energy = 700
    rim_data.color = (0.36, 0.45, 1.0)
    rim_data.shape = "RECTANGLE"
    rim_data.size = 4.0
    rim_data.size_y = 4.0
    rim = bpy.data.objects.new("ExplosionRim", rim_data)
    bpy.context.collection.objects.link(rim)
    rim.location = (2.2, 3.1, 2.8)
    set_camera_look_at(rim, Vector((0.0, 0.0, 1.0)))


if os.path.isdir(CACHE_DIR):
    shutil.rmtree(CACHE_DIR)
os.makedirs(CACHE_DIR, exist_ok=True)

bpy.ops.object.select_all(action="SELECT")
bpy.ops.object.delete(use_global=False)

scene = bpy.context.scene
scene.frame_start = 1
scene.frame_end = 12

bpy.ops.mesh.primitive_uv_sphere_add(location=(0.0, 0.0, 0.7), segments=48, ring_count=24)
flow = bpy.context.object
flow.name = "ExplosionEmitter"
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
domain.name = "FireExplosionDomain"
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
ds.cache_directory = CACHE_DIR
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

with open(STATS_PATH, "w", encoding="utf-8") as handle:
    for frame in range(1, 13):
        path = os.path.join(CACHE_DIR, "data", f"fluid_data_{frame:04d}.vdb")
        grids, _ = openvdb.readAll(path)
        density = next(grid for grid in grids if grid.name == "density")
        temp = next(grid for grid in grids if grid.name == "temperature")
        vel = next(grid for grid in grids if grid.name == "velocity")
        line = (
            f"FRAME {frame:02d} "
            f"density_active={density.activeVoxelCount()} density_minmax={density.evalMinMax()} "
            f"temp_active={temp.activeVoxelCount()} temp_minmax={temp.evalMinMax()} "
            f"vel_active={vel.activeVoxelCount()} vel_minmax={vel.evalMinMax()}"
        )
        print(line)
        handle.write(line + "\n")

flow.hide_render = True
flow.hide_viewport = True
add_presentation_scene()
bpy.ops.wm.save_as_mainfile(filepath=BLEND_PATH)
