"""
Generate a true 3D fire explosion source asset in Blender using a baked gas
simulation with OpenVDB cache output.

Outputs:
- FireExplosion_VDB_Source.blend
- VDB cache sequence
- hero/side preview stills for review

This script stops at the Blender/source-asset stage. It does not import
anything into Unreal.
"""

import math
import os
import shutil
from mathutils import Vector

import bpy


PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
OUT_DIR = os.path.join(PROJECT_ROOT, "SourceAssets", "Import", "VFX", "Idol_Fire_Explosion")
CACHE_DIR = os.path.join(OUT_DIR, "vdb_cache")
PREVIEW_DIR = os.path.join(OUT_DIR, "vdb_previews")
BLEND_PATH = os.path.join(OUT_DIR, "FireExplosion_VDB_Source.blend")

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


def keyframe_value(target, data_path, frame, value):
    setattr(target, data_path, value)
    target.keyframe_insert(data_path=data_path, frame=frame)


def configure_scene():
    scene = bpy.context.scene
    scene.frame_start = FRAME_START
    scene.frame_end = FRAME_END
    scene.render.engine = "CYCLES"
    scene.cycles.samples = 64
    scene.cycles.preview_samples = 32
    scene.cycles.use_adaptive_sampling = True
    scene.cycles.max_bounces = 6
    scene.cycles.transparent_max_bounces = 6
    scene.cycles.volume_bounces = 2
    scene.render.resolution_x = RENDER_SIZE
    scene.render.resolution_y = RENDER_SIZE
    scene.render.resolution_percentage = 100
    scene.render.image_settings.file_format = "PNG"
    scene.render.image_settings.color_mode = "RGBA"
    scene.render.fps = 24
    scene.render.film_transparent = False
    scene.view_settings.look = "AgX - High Contrast"
    scene.view_settings.exposure = 0.45
    scene.view_settings.gamma = 0.95

    world = bpy.data.worlds.new("FireExplosionWorld")
    world.use_nodes = True
    bg = world.node_tree.nodes["Background"]
    bg.inputs[0].default_value = (0.008, 0.010, 0.018, 1.0)
    bg.inputs[1].default_value = 0.35
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
    principled.inputs["Roughness"].default_value = 0.68
    principled.inputs["Specular IOR Level"].default_value = 0.22
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
    rim_data.energy = 950
    rim_data.color = (0.35, 0.44, 1.0)
    rim_data.shape = "RECTANGLE"
    rim_data.size = 4.0
    rim_data.size_y = 4.0
    rim = bpy.data.objects.new("ExplosionRim", rim_data)
    bpy.context.collection.objects.link(rim)
    rim.location = (2.8, 3.6, 3.2)
    set_camera_look_at(rim, Vector((0.0, 0.0, 1.3)))


def add_fire_flow(name, location, scale):
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
    bpy.ops.object.quick_smoke()


def configure_flow(obj, density_keys, fuel_keys, temp_keys, velocity_coord, velocity_normal, velocity_random):
    flow_mod = next(mod for mod in obj.modifiers if mod.type == "FLUID")
    fs = flow_mod.flow_settings
    fs.flow_type = "BOTH"
    fs.flow_behavior = "INFLOW"
    fs.surface_distance = 0.0
    fs.use_initial_velocity = True
    fs.velocity_coord = velocity_coord
    fs.velocity_normal = velocity_normal
    fs.velocity_random = velocity_random
    fs.subframes = 1

    for frame, value in density_keys:
        fs.density = value
        fs.keyframe_insert(data_path="density", frame=frame)
    for frame, value in fuel_keys:
        fs.fuel_amount = value
        fs.keyframe_insert(data_path="fuel_amount", frame=frame)
    for frame, value in temp_keys:
        fs.temperature = value
        fs.keyframe_insert(data_path="temperature", frame=frame)


def add_force_fields():
    bpy.ops.object.effector_add(type="TURBULENCE", location=(0.0, 0.0, 1.4))
    turbulence = bpy.context.object
    turbulence.name = "ExplosionTurbulence"
    turbulence.field.strength = 4.0
    turbulence.field.size = 1.9
    turbulence.field.flow = 1.2
    turbulence.field.noise = 0.9

    bpy.ops.object.effector_add(type="VORTEX", location=(0.0, 0.0, 1.25))
    vortex = bpy.context.object
    vortex.name = "ExplosionVortex"
    vortex.field.strength = 0.28
    vortex.field.flow = 0.4

    bpy.ops.object.effector_add(type="WIND", location=(0.0, 0.0, 0.8))
    wind = bpy.context.object
    wind.name = "ExplosionUpdraft"
    wind.rotation_euler = (math.radians(90.0), 0.0, 0.0)
    wind.field.strength = 0.65
    wind.field.flow = 0.25


def configure_domain(scene):
    domain = bpy.data.objects["Smoke Domain"]
    domain.name = "FireExplosionDomain"
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
    ds.vorticity = 0.28
    ds.flame_vorticity = 1.35
    ds.burning_rate = 1.05
    ds.flame_smoke = 0.62
    ds.flame_ignition = 1.15
    ds.flame_max_temp = 3.2
    ds.use_adaptive_domain = True
    ds.adapt_margin = 3
    ds.adapt_threshold = 0.01
    ds.timesteps_max = 2
    ds.timesteps_min = 1
    ds.cfl_condition = 4.0
    if hasattr(ds, "use_dissolve_smoke"):
        ds.use_dissolve_smoke = True
    if hasattr(ds, "dissolve_speed"):
        ds.dissolve_speed = 12
    if hasattr(ds, "use_dissolve_smoke_log"):
        ds.use_dissolve_smoke_log = False

    material = bpy.data.materials.new("M_FireExplosionVolume")
    material.use_nodes = True
    nodes = material.node_tree.nodes
    links = material.node_tree.links
    nodes.clear()

    output = nodes.new("ShaderNodeOutputMaterial")
    output.location = (920, 120)

    volume_info = nodes.new("ShaderNodeVolumeInfo")
    volume_info.location = (-1280, 100)

    density_mul = nodes.new("ShaderNodeMath")
    density_mul.location = (-1020, 240)
    density_mul.operation = "MULTIPLY"
    density_mul.inputs[1].default_value = 6.0

    flame_mul = nodes.new("ShaderNodeMath")
    flame_mul.location = (-1020, 40)
    flame_mul.operation = "MULTIPLY"
    flame_mul.inputs[1].default_value = 2.1

    flame_ramp = nodes.new("ShaderNodeValToRGB")
    flame_ramp.location = (-780, 20)
    flame_ramp.color_ramp.elements[0].position = 0.00
    flame_ramp.color_ramp.elements[0].color = (0.04, 0.008, 0.003, 1.0)
    flame_ramp.color_ramp.elements[1].position = 1.0
    flame_ramp.color_ramp.elements[1].color = (1.0, 0.97, 0.72, 1.0)
    for position, color in (
        (0.14, (0.18, 0.015, 0.006, 1.0)),
        (0.34, (0.62, 0.05, 0.012, 1.0)),
        (0.58, (1.0, 0.22, 0.03, 1.0)),
        (0.78, (1.0, 0.56, 0.08, 1.0)),
        (0.90, (1.0, 0.83, 0.27, 1.0)),
    ):
        elem = flame_ramp.color_ramp.elements.new(position)
        elem.color = color

    smoke_ramp = nodes.new("ShaderNodeValToRGB")
    smoke_ramp.location = (-780, 260)
    smoke_ramp.color_ramp.elements[0].position = 0.00
    smoke_ramp.color_ramp.elements[0].color = (0.025, 0.02, 0.018, 1.0)
    smoke_ramp.color_ramp.elements[1].position = 1.0
    smoke_ramp.color_ramp.elements[1].color = (0.24, 0.19, 0.15, 1.0)

    blackbody_mul = nodes.new("ShaderNodeMath")
    blackbody_mul.location = (-780, -180)
    blackbody_mul.operation = "MULTIPLY"
    blackbody_mul.inputs[1].default_value = 1.9

    principled_volume = nodes.new("ShaderNodeVolumePrincipled")
    principled_volume.location = (-300, 120)
    principled_volume.inputs["Anisotropy"].default_value = 0.1
    principled_volume.inputs["Absorption Color"].default_value = (0.16, 0.08, 0.04, 1.0)
    principled_volume.inputs["Blackbody Tint"].default_value = (1.0, 0.93, 0.85, 1.0)

    links.new(volume_info.outputs["Density"], density_mul.inputs[0])
    links.new(density_mul.outputs["Value"], smoke_ramp.inputs["Fac"])
    links.new(volume_info.outputs["Flame"], flame_mul.inputs[0])
    links.new(flame_mul.outputs["Value"], flame_ramp.inputs["Fac"])
    links.new(volume_info.outputs["Temperature"], blackbody_mul.inputs[0])

    links.new(smoke_ramp.outputs["Color"], principled_volume.inputs["Color"])
    links.new(density_mul.outputs["Value"], principled_volume.inputs["Density"])
    links.new(flame_ramp.outputs["Color"], principled_volume.inputs["Emission Color"])
    links.new(flame_mul.outputs["Value"], principled_volume.inputs["Emission Strength"])
    links.new(blackbody_mul.outputs["Value"], principled_volume.inputs["Blackbody Intensity"])
    links.new(volume_info.outputs["Temperature"], principled_volume.inputs["Temperature"])
    links.new(principled_volume.outputs["Volume"], output.inputs["Volume"])

    domain.data.materials.clear()
    domain.data.materials.append(material)
    return domain


def render_previews(scene, hero_camera, side_camera):
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

    core = add_fire_flow(
        "CoreBurst",
        location=(0.0, 0.0, 0.82),
        scale=(0.92, 0.92, 0.62),
    )

    shell = add_fire_flow(
        "ShellBurst",
        location=(0.0, 0.0, 0.58),
        scale=(1.55, 1.55, 0.42),
    )

    lobe = add_fire_flow(
        "AsymLobe",
        location=(0.34, -0.22, 0.96),
        scale=(0.62, 0.50, 0.72),
    )

    initialize_smoke_setup((core, shell, lobe))

    configure_flow(
        core,
        density_keys=((1, 0.0), (2, 1.2), (3, 1.0), (4, 0.45), (5, 0.0)),
        fuel_keys=((1, 0.0), (2, 1.8), (3, 1.6), (4, 0.55), (5, 0.0)),
        temp_keys=((1, 0.0), (2, 2.0), (3, 1.8), (4, 0.7), (5, 0.0)),
        velocity_coord=(0.0, 0.0, 2.1),
        velocity_normal=1.35,
        velocity_random=0.55,
    )
    configure_flow(
        shell,
        density_keys=((1, 0.0), (2, 0.95), (3, 0.82), (4, 0.18), (5, 0.0)),
        fuel_keys=((1, 0.0), (2, 1.15), (3, 0.95), (4, 0.18), (5, 0.0)),
        temp_keys=((1, 0.0), (2, 1.5), (3, 1.2), (4, 0.3), (5, 0.0)),
        velocity_coord=(0.0, 0.0, 1.05),
        velocity_normal=1.55,
        velocity_random=0.48,
    )
    configure_flow(
        lobe,
        density_keys=((1, 0.0), (3, 0.55), (4, 0.72), (6, 0.12), (7, 0.0)),
        fuel_keys=((1, 0.0), (3, 0.85), (4, 1.0), (6, 0.18), (7, 0.0)),
        temp_keys=((1, 0.0), (3, 1.3), (4, 1.55), (6, 0.28), (7, 0.0)),
        velocity_coord=(0.45, -0.15, 1.55),
        velocity_normal=0.82,
        velocity_random=0.62,
    )

    for obj, keys in (
        (core, ((1, (0.82, 0.82, 0.56)), (3, (1.16, 1.16, 0.76)), (5, (1.32, 1.32, 0.92)))),
        (shell, ((1, (1.40, 1.40, 0.34)), (3, (1.95, 1.95, 0.44)), (5, (2.20, 2.20, 0.55)))),
        (lobe, ((2, (0.56, 0.45, 0.62)), (4, (0.76, 0.62, 0.96)), (7, (0.96, 0.82, 1.18)))),
    ):
        for frame, scale in keys:
            obj.scale = scale
            obj.keyframe_insert(data_path="scale", frame=frame)

    add_force_fields()
    configure_domain(scene)
    bpy.ops.wm.save_as_mainfile(filepath=BLEND_PATH)

    domain = bpy.data.objects["FireExplosionDomain"]
    bpy.context.view_layer.objects.active = domain
    scene.frame_set(FRAME_START)
    bpy.ops.fluid.bake_all()

    bpy.ops.wm.save_as_mainfile(filepath=BLEND_PATH)
    render_previews(scene, hero_camera, side_camera)
    bpy.ops.wm.save_as_mainfile(filepath=BLEND_PATH)


if __name__ == "__main__":
    build_sim()
