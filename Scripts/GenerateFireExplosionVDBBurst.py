"""
Generate a working Blender-native OpenVDB fire-explosion source asset.

This uses the proven-safe path for this Blender 5.0 build:
- smoke-only flow
- geometry emitter
- constant flow density/temperature
- keyframed emitter scale for the burst silhouette

Why:
- Fire/BOTH flow bakes are writing empty density/flame/temperature grids.
- Keyframed flow density/temperature also collapse to zero during bake.

Outputs:
- FireExplosion_VDBBurst_Source.blend
- vdb_burst_cache
- vdb_burst_stats.txt
"""

import math
import os
import shutil
import sys
from mathutils import Vector

import bpy


PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
OUT_DIR = os.path.join(PROJECT_ROOT, "SourceAssets", "Import", "VFX", "Idol_Fire_Explosion")
BLEND_PATH = os.path.join(OUT_DIR, "FireExplosion_VDBBurst_Source.blend")
CACHE_DIR = os.path.join(OUT_DIR, "vdb_burst_cache")
STATS_PATH = os.path.join(OUT_DIR, "vdb_burst_stats.txt")

FRAME_START = 1
FRAME_END = 16


def reset_dir(path):
    if os.path.isdir(path):
        shutil.rmtree(path)
    os.makedirs(path, exist_ok=True)


def set_camera_look_at(obj, target):
    direction = target - obj.location
    obj.rotation_euler = direction.to_track_quat("-Z", "Y").to_euler()


def clean_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)

    for datablock_collection in (
        bpy.data.meshes,
        bpy.data.materials,
        bpy.data.cameras,
        bpy.data.lights,
        bpy.data.worlds,
        bpy.data.images,
        bpy.data.textures,
    ):
        for datablock in list(datablock_collection):
            if datablock.users == 0:
                datablock_collection.remove(datablock)


def configure_scene():
    scene = bpy.context.scene
    scene.frame_start = FRAME_START
    scene.frame_end = FRAME_END
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
    scene.view_settings.exposure = 0.05

    world = bpy.data.worlds.new("FireExplosionBurstWorld")
    world.use_nodes = True
    bg = world.node_tree.nodes["Background"]
    bg.inputs[0].default_value = (0.008, 0.010, 0.018, 1.0)
    bg.inputs[1].default_value = 0.18
    scene.world = world
    return scene


def add_ground():
    bpy.ops.mesh.primitive_plane_add(size=18, location=(0.0, 0.0, -0.02))
    ground = bpy.context.object
    ground.name = "Ground"

    material = bpy.data.materials.new("M_Ground")
    material.use_nodes = True
    principled = material.node_tree.nodes["Principled BSDF"]
    principled.inputs["Base Color"].default_value = (0.018, 0.02, 0.024, 1.0)
    principled.inputs["Roughness"].default_value = 0.72
    ground.data.materials.append(material)


def add_cameras(scene):
    target = Vector((0.0, 0.0, 1.2))

    hero_data = bpy.data.cameras.new("CameraHero")
    hero = bpy.data.objects.new("CameraHero", hero_data)
    bpy.context.collection.objects.link(hero)
    hero.location = (0.0, -5.9, 2.8)
    set_camera_look_at(hero, target)

    side_data = bpy.data.cameras.new("CameraSide")
    side = bpy.data.objects.new("CameraSide", side_data)
    bpy.context.collection.objects.link(side)
    side.location = (5.5, 0.0, 2.2)
    set_camera_look_at(side, target)

    scene.camera = hero
    return hero, side


def add_lights():
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


def add_force_fields():
    bpy.ops.object.effector_add(type="TURBULENCE", location=(0.0, 0.0, 1.2))
    turbulence = bpy.context.object
    turbulence.name = "ExplosionTurbulence"
    turbulence.field.strength = 2.8
    turbulence.field.size = 1.8
    turbulence.field.flow = 1.0
    turbulence.field.noise = 0.8

    bpy.ops.object.effector_add(type="VORTEX", location=(0.0, 0.0, 1.1))
    vortex = bpy.context.object
    vortex.name = "ExplosionVortex"
    vortex.field.strength = 0.12
    vortex.field.flow = 0.22

    bpy.ops.object.effector_add(type="WIND", location=(0.0, 0.0, 0.8))
    wind = bpy.context.object
    wind.name = "ExplosionUpdraft"
    wind.rotation_euler = (math.radians(90.0), 0.0, 0.0)
    wind.field.strength = 0.85
    wind.field.flow = 0.18


def add_emitter():
    bpy.ops.mesh.primitive_uv_sphere_add(location=(0.0, 0.0, 0.72), segments=48, ring_count=24)
    emitter = bpy.context.object
    emitter.name = "ExplosionEmitter"

    for frame, scale in (
        (1, (0.10, 0.10, 0.10)),
        (2, (0.85, 0.85, 0.60)),
        (3, (1.35, 1.35, 0.92)),
        (4, (1.65, 1.65, 1.10)),
        (5, (1.30, 1.30, 0.92)),
        (6, (0.75, 0.75, 0.56)),
        (7, (0.28, 0.28, 0.22)),
        (8, (0.04, 0.04, 0.04)),
        (9, (0.01, 0.01, 0.01)),
    ):
        emitter.scale = scale
        emitter.keyframe_insert(data_path="scale", frame=frame)
    return emitter


def initialize_smoke_setup(emitter):
    bpy.ops.object.select_all(action="DESELECT")
    emitter.select_set(True)
    bpy.context.view_layer.objects.active = emitter
    bpy.ops.object.quick_smoke(style="SMOKE")


def configure_flow(emitter):
    flow_mod = next(mod for mod in emitter.modifiers if mod.type == "FLUID")
    fs = flow_mod.flow_settings
    fs.flow_type = "SMOKE"
    fs.flow_behavior = "GEOMETRY"
    fs.surface_distance = 0.0
    fs.use_initial_velocity = True
    fs.velocity_coord = (0.0, 0.0, 2.4)
    fs.velocity_normal = 1.65
    fs.velocity_random = 0.42
    fs.subframes = 1
    fs.density = 2.2
    fs.temperature = 2.1


def configure_domain():
    domain = bpy.data.objects["Smoke Domain"]
    domain.name = "FireExplosionBurstDomain"
    domain.location = (0.0, 0.0, 1.2)
    domain.scale = (2.35, 2.35, 2.8)

    mod = next(mod for mod in domain.modifiers if mod.type == "FLUID")
    ds = mod.domain_settings
    ds.cache_type = "ALL"
    ds.cache_data_format = "OPENVDB"
    ds.cache_directory = CACHE_DIR
    ds.cache_frame_start = FRAME_START
    ds.cache_frame_end = FRAME_END
    ds.resolution_max = 72
    ds.use_noise = True
    ds.use_adaptive_domain = True
    ds.adapt_margin = 3
    ds.adapt_threshold = 0.01
    ds.vorticity = 0.38
    ds.timesteps_max = 2
    ds.timesteps_min = 1
    ds.cfl_condition = 4.0
    if hasattr(ds, "use_dissolve_smoke"):
        ds.use_dissolve_smoke = True
    if hasattr(ds, "dissolve_speed"):
        ds.dissolve_speed = 9

    material = domain.active_material
    nodes = material.node_tree.nodes
    for node in nodes:
        if node.bl_idname == "ShaderNodeVolumePrincipled":
            node.inputs["Density"].default_value = 4.5
            node.inputs["Density Attribute"].default_value = "density"
            node.inputs["Color"].default_value = (0.09, 0.05, 0.03, 1.0)
            node.inputs["Absorption Color"].default_value = (0.16, 0.08, 0.03, 1.0)
            node.inputs["Blackbody Intensity"].default_value = 2.2
            node.inputs["Temperature"].default_value = 1400.0
            node.inputs["Temperature Attribute"].default_value = "temperature"
            node.inputs["Anisotropy"].default_value = 0.12
            emission_input = node.inputs.get("Emission Strength")
            if emission_input is not None:
                emission_input.default_value = 1.1
            emission_color = node.inputs.get("Emission Color")
            if emission_color is not None:
                emission_color.default_value = (1.0, 0.42, 0.10, 1.0)
            break
    return domain


def write_stats():
    sys.path.insert(0, r"C:\Program Files\Blender Foundation\Blender 5.0\5.0\python\lib\site-packages")
    import openvdb

    with open(STATS_PATH, "w", encoding="utf-8") as handle:
        for frame in range(FRAME_START, FRAME_END + 1):
            path = os.path.join(CACHE_DIR, "data", f"fluid_data_{frame:04d}.vdb")
            grids, _ = openvdb.readAll(path)
            density = next(grid for grid in grids if grid.name == "density")
            temp = next(grid for grid in grids if grid.name == "temperature")
            vel = next(grid for grid in grids if grid.name == "velocity")
            handle.write(
                f"FRAME {frame:02d} "
                f"density_active={density.activeVoxelCount()} density_minmax={density.evalMinMax()} "
                f"temp_active={temp.activeVoxelCount()} temp_minmax={temp.evalMinMax()} "
                f"vel_active={vel.activeVoxelCount()} vel_minmax={vel.evalMinMax()}\n"
            )


def build():
    os.makedirs(OUT_DIR, exist_ok=True)
    reset_dir(CACHE_DIR)
    clean_scene()
    scene = configure_scene()
    add_ground()
    add_cameras(scene)
    add_lights()
    add_force_fields()
    emitter = add_emitter()
    initialize_smoke_setup(emitter)
    configure_flow(emitter)
    domain = configure_domain()

    bpy.ops.wm.save_as_mainfile(filepath=BLEND_PATH)
    bpy.context.view_layer.objects.active = domain
    scene.frame_set(FRAME_START)
    bpy.ops.fluid.bake_all()

    emitter.hide_render = True
    emitter.hide_viewport = True
    write_stats()
    bpy.ops.wm.save_as_mainfile(filepath=BLEND_PATH)


if __name__ == "__main__":
    build()
