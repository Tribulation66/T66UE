"""
Generate a Blender-native OpenVDB fire explosion using a smoke-driven gas sim.

Why this exists:
- In this Blender 5.0 build, Mantaflow's scripted fire/BOTH bake path writes
  empty flame/density/temperature VDB grids for our explosion setup.
- Smoke-only gas sims do export valid density/temperature/velocity grids.

This script uses the working smoke path, then shades the resulting gas as fire
through density + temperature instead of relying on Mantaflow flame grids.

Outputs:
- FireExplosion_VDBSmokeDriven_Source.blend
- vdb_smoke_driven_cache
- vdb_smoke_driven_previews
"""

import math
import os
import shutil
from mathutils import Vector

import bpy


PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
OUT_DIR = os.path.join(PROJECT_ROOT, "SourceAssets", "Import", "VFX", "Idol_Fire_Explosion")
CACHE_DIR = os.path.join(OUT_DIR, "vdb_smoke_driven_cache")
PREVIEW_DIR = os.path.join(OUT_DIR, "vdb_smoke_driven_previews")
BLEND_PATH = os.path.join(OUT_DIR, "FireExplosion_VDBSmokeDriven_Source.blend")

FRAME_START = 1
FRAME_END = 24
RENDER_SIZE = 768
PREVIEW_FRAMES = (4, 7, 10, 13, 16, 20)


def reset_dir(path):
    if os.path.isdir(path):
        shutil.rmtree(path)
    os.makedirs(path, exist_ok=True)


def ensure_dirs():
    os.makedirs(OUT_DIR, exist_ok=True)
    reset_dir(CACHE_DIR)
    reset_dir(PREVIEW_DIR)


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


def set_camera_look_at(obj, target):
    direction = target - obj.location
    obj.rotation_euler = direction.to_track_quat("-Z", "Y").to_euler()


def configure_scene():
    scene = bpy.context.scene
    scene.frame_start = FRAME_START
    scene.frame_end = FRAME_END
    scene.render.engine = "CYCLES"
    scene.cycles.samples = 96
    scene.cycles.preview_samples = 24
    scene.cycles.use_adaptive_sampling = True
    scene.cycles.max_bounces = 6
    scene.cycles.transparent_max_bounces = 6
    scene.cycles.volume_bounces = 3
    scene.render.resolution_x = RENDER_SIZE
    scene.render.resolution_y = RENDER_SIZE
    scene.render.resolution_percentage = 100
    scene.render.image_settings.file_format = "PNG"
    scene.render.image_settings.color_mode = "RGBA"
    scene.render.fps = 24
    scene.render.film_transparent = False
    scene.view_settings.look = "AgX - High Contrast"
    scene.view_settings.exposure = 0.25
    scene.view_settings.gamma = 0.95

    world = bpy.data.worlds.new("FireExplosionSmokeDrivenWorld")
    world.use_nodes = True
    bg = world.node_tree.nodes["Background"]
    bg.inputs[0].default_value = (0.008, 0.010, 0.018, 1.0)
    bg.inputs[1].default_value = 0.28
    scene.world = world
    return scene


def add_ground():
    bpy.ops.mesh.primitive_plane_add(size=16, location=(0.0, 0.0, -0.02))
    ground = bpy.context.object
    ground.name = "Ground"

    material = bpy.data.materials.new("M_FireExplosionGround")
    material.use_nodes = True
    principled = material.node_tree.nodes["Principled BSDF"]
    principled.inputs["Base Color"].default_value = (0.018, 0.02, 0.024, 1.0)
    principled.inputs["Roughness"].default_value = 0.72
    ground.data.materials.append(material)
    return ground


def add_cameras(scene):
    target = Vector((0.0, 0.0, 1.15))

    hero_data = bpy.data.cameras.new("CameraHero")
    hero_data.lens = 55
    hero = bpy.data.objects.new("CameraHero", hero_data)
    bpy.context.collection.objects.link(hero)
    hero.location = (0.0, -8.1, 3.5)
    set_camera_look_at(hero, target)

    side_data = bpy.data.cameras.new("CameraSide")
    side_data.lens = 55
    side = bpy.data.objects.new("CameraSide", side_data)
    bpy.context.collection.objects.link(side)
    side.location = (7.6, 0.0, 2.55)
    set_camera_look_at(side, target)

    scene.camera = hero
    return hero, side


def add_lights():
    key_data = bpy.data.lights.new("ExplosionKey", "AREA")
    key_data.energy = 2200
    key_data.shape = "RECTANGLE"
    key_data.size = 8.0
    key_data.size_y = 5.0
    key = bpy.data.objects.new("ExplosionKey", key_data)
    bpy.context.collection.objects.link(key)
    key.location = (0.0, -4.0, 5.2)
    set_camera_look_at(key, Vector((0.0, 0.0, 1.0)))

    rim_data = bpy.data.lights.new("ExplosionRim", "AREA")
    rim_data.energy = 700
    rim_data.color = (0.35, 0.44, 1.0)
    rim_data.shape = "RECTANGLE"
    rim_data.size = 4.0
    rim_data.size_y = 4.0
    rim = bpy.data.objects.new("ExplosionRim", rim_data)
    bpy.context.collection.objects.link(rim)
    rim.location = (2.8, 3.6, 3.2)
    set_camera_look_at(rim, Vector((0.0, 0.0, 1.3)))


def add_flow(name, location, scale):
    bpy.ops.mesh.primitive_uv_sphere_add(location=location, segments=48, ring_count=24)
    obj = bpy.context.object
    obj.name = name
    obj.scale = scale
    return obj


def initialize_smoke_setup(flow_objects):
    bpy.ops.object.select_all(action="DESELECT")
    for obj in flow_objects:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = flow_objects[0]
    bpy.ops.object.quick_smoke(style="SMOKE")


def configure_flow(obj, density_keys, temperature_keys, velocity_coord, velocity_normal, velocity_random):
    flow_mod = next(mod for mod in obj.modifiers if mod.type == "FLUID")
    fs = flow_mod.flow_settings
    fs.flow_type = "SMOKE"
    fs.flow_behavior = "GEOMETRY"
    fs.surface_distance = 0.0
    fs.use_initial_velocity = True
    fs.velocity_coord = velocity_coord
    fs.velocity_normal = velocity_normal
    fs.velocity_random = velocity_random
    fs.subframes = 1

    for frame, value in density_keys:
        fs.density = value
        fs.keyframe_insert(data_path="density", frame=frame)
    for frame, value in temperature_keys:
        fs.temperature = value
        fs.keyframe_insert(data_path="temperature", frame=frame)


def add_force_fields():
    bpy.ops.object.effector_add(type="TURBULENCE", location=(0.0, 0.0, 1.4))
    turbulence = bpy.context.object
    turbulence.name = "ExplosionTurbulence"
    turbulence.field.strength = 3.0
    turbulence.field.size = 1.8
    turbulence.field.flow = 1.1
    turbulence.field.noise = 0.8

    bpy.ops.object.effector_add(type="VORTEX", location=(0.0, 0.0, 1.25))
    vortex = bpy.context.object
    vortex.name = "ExplosionVortex"
    vortex.field.strength = 0.18
    vortex.field.flow = 0.35

    bpy.ops.object.effector_add(type="WIND", location=(0.0, 0.0, 0.8))
    wind = bpy.context.object
    wind.name = "ExplosionUpdraft"
    wind.rotation_euler = (math.radians(90.0), 0.0, 0.0)
    wind.field.strength = 0.72
    wind.field.flow = 0.22


def configure_domain():
    domain = bpy.data.objects["Smoke Domain"]
    domain.name = "FireExplosionSmokeDrivenDomain"
    domain.location = (0.0, 0.0, 1.55)
    domain.scale = (2.95, 2.95, 3.2)

    mod = next(mod for mod in domain.modifiers if mod.type == "FLUID")
    ds = mod.domain_settings
    ds.cache_type = "ALL"
    ds.cache_data_format = "OPENVDB"
    ds.cache_directory = CACHE_DIR
    ds.cache_frame_start = FRAME_START
    ds.cache_frame_end = FRAME_END
    ds.resolution_max = 96
    ds.use_noise = True
    ds.beta = 0.95
    ds.vorticity = 0.22
    ds.use_adaptive_domain = True
    ds.adapt_margin = 3
    ds.adapt_threshold = 0.01
    ds.timesteps_max = 2
    ds.timesteps_min = 1
    ds.cfl_condition = 4.0
    if hasattr(ds, "use_dissolve_smoke"):
        ds.use_dissolve_smoke = True
    if hasattr(ds, "dissolve_speed"):
        ds.dissolve_speed = 10

    material = bpy.data.materials.new("M_FireExplosionSmokeDrivenVolume")
    material.use_nodes = True
    nodes = material.node_tree.nodes
    links = material.node_tree.links
    nodes.clear()

    output = nodes.new("ShaderNodeOutputMaterial")
    output.location = (520, 120)

    volume_info = nodes.new("ShaderNodeVolumeInfo")
    volume_info.location = (-980, 120)

    density_mul = nodes.new("ShaderNodeMath")
    density_mul.location = (-720, 240)
    density_mul.operation = "MULTIPLY"
    density_mul.inputs[1].default_value = 2.8

    temp_mul = nodes.new("ShaderNodeMath")
    temp_mul.location = (-720, 10)
    temp_mul.operation = "MULTIPLY"
    temp_mul.inputs[1].default_value = 1.8

    flame_ramp = nodes.new("ShaderNodeValToRGB")
    flame_ramp.location = (-470, 0)
    flame_ramp.color_ramp.elements[0].position = 0.0
    flame_ramp.color_ramp.elements[0].color = (0.05, 0.01, 0.00, 1.0)
    flame_ramp.color_ramp.elements[1].position = 1.0
    flame_ramp.color_ramp.elements[1].color = (1.0, 0.90, 0.42, 1.0)
    for position, color in (
        (0.20, (0.28, 0.03, 0.00, 1.0)),
        (0.46, (0.92, 0.14, 0.02, 1.0)),
        (0.72, (1.0, 0.42, 0.05, 1.0)),
    ):
        elem = flame_ramp.color_ramp.elements.new(position)
        elem.color = color

    principled_volume = nodes.new("ShaderNodeVolumePrincipled")
    principled_volume.location = (-60, 120)
    principled_volume.inputs["Color"].default_value = (0.09, 0.06, 0.05, 1.0)
    principled_volume.inputs["Absorption Color"].default_value = (0.18, 0.10, 0.05, 1.0)
    principled_volume.inputs["Blackbody Intensity"].default_value = 1.4
    principled_volume.inputs["Temperature"].default_value = 1200.0
    principled_volume.inputs["Anisotropy"].default_value = 0.08

    links.new(volume_info.outputs["Density"], density_mul.inputs[0])
    links.new(volume_info.outputs["Temperature"], temp_mul.inputs[0])
    links.new(temp_mul.outputs["Value"], flame_ramp.inputs["Fac"])
    links.new(density_mul.outputs["Value"], principled_volume.inputs["Density"])
    links.new(flame_ramp.outputs["Color"], principled_volume.inputs["Emission Color"])
    links.new(temp_mul.outputs["Value"], principled_volume.inputs["Emission Strength"])
    links.new(volume_info.outputs["Temperature"], principled_volume.inputs["Temperature"])
    links.new(principled_volume.outputs["Volume"], output.inputs["Volume"])

    domain.data.materials.clear()
    domain.data.materials.append(material)
    return domain


def render_previews(scene, hero_camera, side_camera, flow_objects):
    for obj in flow_objects:
        obj.hide_render = True
        obj.hide_viewport = True

    renders = []
    for label, camera in (("hero", hero_camera), ("side", side_camera)):
        scene.camera = camera
        for frame in PREVIEW_FRAMES:
            scene.frame_set(frame)
            filepath = os.path.join(PREVIEW_DIR, f"{label}_f{frame:02d}.png")
            scene.render.filepath = filepath
            bpy.ops.render.render(write_still=True)
            renders.append(filepath)
    return renders


def build_sim():
    ensure_dirs()
    clean_scene()
    scene = configure_scene()
    add_ground()
    hero_camera, side_camera = add_cameras(scene)
    add_lights()

    core = add_flow("CoreBurst", location=(0.0, 0.0, 0.82), scale=(1.00, 1.00, 0.70))

    initialize_smoke_setup((core,))

    configure_flow(
        core,
        density_keys=((1, 0.0), (2, 1.6), (3, 1.4), (4, 0.8), (5, 0.25), (6, 0.0)),
        temperature_keys=((1, 0.0), (2, 1.9), (3, 1.7), (4, 0.95), (5, 0.35), (6, 0.0)),
        velocity_coord=(0.0, 0.0, 1.8),
        velocity_normal=1.3,
        velocity_random=0.35,
    )

    add_force_fields()
    configure_domain()
    bpy.ops.wm.save_as_mainfile(filepath=BLEND_PATH)

    domain = bpy.data.objects["FireExplosionSmokeDrivenDomain"]
    bpy.context.view_layer.objects.active = domain
    scene.frame_set(FRAME_START)
    bpy.ops.fluid.bake_all()

    bpy.ops.wm.save_as_mainfile(filepath=BLEND_PATH)
    render_previews(scene, hero_camera, side_camera, (core,))
    bpy.ops.wm.save_as_mainfile(filepath=BLEND_PATH)


if __name__ == "__main__":
    build_sim()
