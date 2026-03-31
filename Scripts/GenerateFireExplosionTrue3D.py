"""
Generate a true 3D procedural fire explosion source asset in Blender.

This avoids the headless Mantaflow bake path, which currently writes empty
volume caches in this Blender 5.0 build. The effect is built from displaced
closed meshes using animated volumetric materials, so it stays readable from
the side and has internal motion.

Outputs:
- FireExplosion_True3D_Source.blend
- hero/side preview stills for review
"""

import math
import os
import shutil
from mathutils import Vector

import bpy


PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
OUT_DIR = os.path.join(PROJECT_ROOT, "SourceAssets", "Import", "VFX", "Idol_Fire_Explosion")
PREVIEW_DIR = os.path.join(OUT_DIR, "true3d_previews")
BLEND_PATH = os.path.join(OUT_DIR, "FireExplosion_True3D_Source.blend")

FRAME_START = 1
FRAME_END = 24
RENDER_SIZE = 640
PREVIEW_FRAMES = (4, 8, 12, 18)


def reset_dir(path):
    if os.path.isdir(path):
        shutil.rmtree(path)
    os.makedirs(path, exist_ok=True)


def ensure_dirs():
    os.makedirs(OUT_DIR, exist_ok=True)
    reset_dir(PREVIEW_DIR)


def clean_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)


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
    scene.cycles.samples = 40
    scene.cycles.preview_samples = 32
    scene.cycles.use_adaptive_sampling = True
    scene.cycles.max_bounces = 8
    scene.cycles.transparent_max_bounces = 8
    scene.cycles.volume_bounces = 4
    scene.render.resolution_x = RENDER_SIZE
    scene.render.resolution_y = RENDER_SIZE
    scene.render.resolution_percentage = 100
    scene.render.image_settings.file_format = "PNG"
    scene.render.image_settings.color_mode = "RGBA"
    scene.render.fps = 24
    scene.render.film_transparent = False
    scene.view_settings.look = "AgX - High Contrast"
    scene.view_settings.exposure = 0.08
    scene.view_settings.gamma = 0.95

    world = bpy.data.worlds.new("FireExplosionWorldTrue3D")
    world.use_nodes = True
    bg = world.node_tree.nodes["Background"]
    bg.inputs[0].default_value = (0.006, 0.008, 0.014, 1.0)
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
    principled.inputs["Base Color"].default_value = (0.018, 0.02, 0.025, 1.0)
    principled.inputs["Roughness"].default_value = 0.72
    principled.inputs["Specular IOR Level"].default_value = 0.16
    ground.data.materials.append(material)
    return ground


def add_cameras(scene):
    target = Vector((0.0, 0.0, 1.2))

    hero_data = bpy.data.cameras.new("CameraHero")
    hero_data.lens = 58
    hero = bpy.data.objects.new("CameraHero", hero_data)
    bpy.context.collection.objects.link(hero)
    hero.location = (0.0, -7.8, 3.4)
    set_camera_look_at(hero, target)

    side_data = bpy.data.cameras.new("CameraSide")
    side_data.lens = 60
    side = bpy.data.objects.new("CameraSide", side_data)
    bpy.context.collection.objects.link(side)
    side.location = (7.2, 0.0, 2.55)
    set_camera_look_at(side, target)

    scene.camera = hero
    return hero, side


def add_lights():
    key_data = bpy.data.lights.new("ExplosionKey", "AREA")
    key_data.energy = 1600
    key_data.shape = "RECTANGLE"
    key_data.size = 7.5
    key_data.size_y = 5.0
    key = bpy.data.objects.new("ExplosionKey", key_data)
    bpy.context.collection.objects.link(key)
    key.location = (0.0, -4.0, 5.0)
    set_camera_look_at(key, Vector((0.0, 0.0, 1.2)))

    rim_data = bpy.data.lights.new("ExplosionRim", "AREA")
    rim_data.energy = 520
    rim_data.color = (0.38, 0.42, 1.0)
    rim_data.shape = "RECTANGLE"
    rim_data.size = 4.5
    rim_data.size_y = 4.5
    rim = bpy.data.objects.new("ExplosionRim", rim_data)
    bpy.context.collection.objects.link(rim)
    rim.location = (3.2, 3.5, 3.0)
    set_camera_look_at(rim, Vector((0.0, 0.0, 1.3)))


def build_fire_material():
    material = bpy.data.materials.new("M_FireExplosionTrue3D_Fire")
    material.use_nodes = True
    nodes = material.node_tree.nodes
    links = material.node_tree.links
    nodes.clear()

    output = nodes.new("ShaderNodeOutputMaterial")
    output.location = (980, 120)

    texcoord = nodes.new("ShaderNodeTexCoord")
    texcoord.location = (-1540, 80)

    mapping = nodes.new("ShaderNodeMapping")
    mapping.location = (-1320, 80)
    mapping.inputs["Scale"].default_value = (1.0, 1.0, 1.25)

    separate_xyz = nodes.new("ShaderNodeSeparateXYZ")
    separate_xyz.location = (-1320, -420)

    vertical_bias = nodes.new("ShaderNodeMapRange")
    vertical_bias.location = (-1080, -420)
    vertical_bias.inputs["From Min"].default_value = -1.10
    vertical_bias.inputs["From Max"].default_value = 1.10
    vertical_bias.inputs["To Min"].default_value = 1.18
    vertical_bias.inputs["To Max"].default_value = 0.52
    vertical_bias.clamp = True

    radial = nodes.new("ShaderNodeVectorMath")
    radial.location = (-1320, -220)
    radial.operation = "LENGTH"

    radial_map = nodes.new("ShaderNodeMapRange")
    radial_map.location = (-1080, -220)
    radial_map.inputs["From Min"].default_value = 0.15
    radial_map.inputs["From Max"].default_value = 1.7
    radial_map.inputs["To Min"].default_value = 1.0
    radial_map.inputs["To Max"].default_value = 0.0
    radial_map.clamp = True

    primary_noise = nodes.new("ShaderNodeTexNoise")
    primary_noise.name = "PrimaryNoise"
    primary_noise.location = (-1080, 80)
    primary_noise.noise_dimensions = "4D"
    primary_noise.inputs["Scale"].default_value = 2.8
    primary_noise.inputs["Detail"].default_value = 8.0
    primary_noise.inputs["Roughness"].default_value = 0.55
    primary_noise.inputs["Distortion"].default_value = 0.18

    secondary_noise = nodes.new("ShaderNodeTexNoise")
    secondary_noise.name = "SecondaryNoise"
    secondary_noise.location = (-1080, 320)
    secondary_noise.noise_dimensions = "4D"
    secondary_noise.inputs["Scale"].default_value = 7.0
    secondary_noise.inputs["Detail"].default_value = 12.0
    secondary_noise.inputs["Roughness"].default_value = 0.48
    secondary_noise.inputs["Distortion"].default_value = 0.05

    secondary_mul = nodes.new("ShaderNodeMath")
    secondary_mul.location = (-840, 320)
    secondary_mul.operation = "MULTIPLY"
    secondary_mul.inputs[1].default_value = 0.42

    noise_add = nodes.new("ShaderNodeMath")
    noise_add.location = (-840, 120)
    noise_add.operation = "ADD"

    density_ramp = nodes.new("ShaderNodeValToRGB")
    density_ramp.name = "DensityRamp"
    density_ramp.location = (-620, 120)
    density_ramp.color_ramp.elements[0].position = 0.48
    density_ramp.color_ramp.elements[1].position = 0.90

    density_bw = nodes.new("ShaderNodeRGBToBW")
    density_bw.location = (-390, 130)

    shape_mul = nodes.new("ShaderNodeMath")
    shape_mul.location = (-160, 20)
    shape_mul.operation = "MULTIPLY"

    density_mul = nodes.new("ShaderNodeMath")
    density_mul.location = (10, 20)
    density_mul.operation = "MULTIPLY"

    energy_value = nodes.new("ShaderNodeValue")
    energy_value.name = "EnergyValue"
    energy_value.location = (-620, -180)
    energy_value.outputs[0].default_value = 0.0

    density_strength = nodes.new("ShaderNodeMath")
    density_strength.name = "DensityStrength"
    density_strength.location = (60, 20)
    density_strength.operation = "MULTIPLY"
    density_strength.inputs[1].default_value = 1.10

    emission_strength = nodes.new("ShaderNodeMath")
    emission_strength.name = "EmissionStrength"
    emission_strength.location = (60, 240)
    emission_strength.operation = "MULTIPLY"
    emission_strength.inputs[1].default_value = 3.4

    fire_ramp = nodes.new("ShaderNodeValToRGB")
    fire_ramp.name = "FireColorRamp"
    fire_ramp.location = (-380, 260)
    fire_ramp.color_ramp.elements[0].position = 0.0
    fire_ramp.color_ramp.elements[0].color = (0.04, 0.01, 0.004, 1.0)
    fire_ramp.color_ramp.elements[1].position = 1.0
    fire_ramp.color_ramp.elements[1].color = (1.0, 0.74, 0.24, 1.0)
    for position, color in (
        (0.14, (0.20, 0.02, 0.006, 1.0)),
        (0.34, (0.70, 0.06, 0.014, 1.0)),
        (0.58, (1.0, 0.24, 0.04, 1.0)),
        (0.76, (1.0, 0.46, 0.08, 1.0)),
        (0.90, (1.0, 0.64, 0.18, 1.0)),
    ):
        elem = fire_ramp.color_ramp.elements.new(position)
        elem.color = color

    principled_volume = nodes.new("ShaderNodeVolumePrincipled")
    principled_volume.name = "FireVolume"
    principled_volume.location = (320, 120)
    principled_volume.inputs["Color"].default_value = (0.18, 0.10, 0.07, 1.0)
    principled_volume.inputs["Absorption Color"].default_value = (0.30, 0.12, 0.04, 1.0)
    principled_volume.inputs["Anisotropy"].default_value = 0.08
    principled_volume.inputs["Blackbody Intensity"].default_value = 0.0

    links.new(texcoord.outputs["Object"], mapping.inputs["Vector"])
    links.new(texcoord.outputs["Object"], separate_xyz.inputs["Vector"])
    links.new(mapping.outputs["Vector"], primary_noise.inputs["Vector"])
    links.new(mapping.outputs["Vector"], secondary_noise.inputs["Vector"])
    links.new(texcoord.outputs["Object"], radial.inputs[0])
    links.new(separate_xyz.outputs["Z"], vertical_bias.inputs["Value"])
    links.new(radial.outputs["Value"], radial_map.inputs["Value"])
    links.new(secondary_noise.outputs["Fac"], secondary_mul.inputs[0])
    links.new(primary_noise.outputs["Fac"], noise_add.inputs[0])
    links.new(secondary_mul.outputs["Value"], noise_add.inputs[1])
    links.new(noise_add.outputs["Value"], density_ramp.inputs["Fac"])
    links.new(density_ramp.outputs["Color"], density_bw.inputs["Color"])
    links.new(density_bw.outputs["Val"], shape_mul.inputs[0])
    links.new(radial_map.outputs["Result"], shape_mul.inputs[1])
    links.new(shape_mul.outputs["Value"], density_mul.inputs[0])
    links.new(vertical_bias.outputs["Result"], density_mul.inputs[1])
    links.new(density_mul.outputs["Value"], density_strength.inputs[0])
    links.new(energy_value.outputs["Value"], density_strength.inputs[1])
    links.new(density_mul.outputs["Value"], emission_strength.inputs[0])
    links.new(density_bw.outputs["Val"], fire_ramp.inputs["Fac"])
    links.new(fire_ramp.outputs["Color"], principled_volume.inputs["Emission Color"])
    links.new(density_strength.outputs["Value"], principled_volume.inputs["Density"])
    links.new(emission_strength.outputs["Value"], principled_volume.inputs["Emission Strength"])
    links.new(principled_volume.outputs["Volume"], output.inputs["Volume"])

    for frame, value in ((1, 0.0), (2, 0.55), (4, 0.82), (7, 0.66), (12, 0.42), (18, 0.16), (24, 0.04)):
        energy_value.outputs[0].default_value = value
        energy_value.outputs[0].keyframe_insert("default_value", frame=frame)

    for frame, value in ((1, 0.0), (5, 0.35), (10, 0.82), (16, 1.35), (24, 1.85)):
        primary_noise.inputs["W"].default_value = value
        primary_noise.inputs["W"].keyframe_insert("default_value", frame=frame)
        secondary_noise.inputs["W"].default_value = value * 1.45
        secondary_noise.inputs["W"].keyframe_insert("default_value", frame=frame)

    return material


def build_smoke_material():
    material = bpy.data.materials.new("M_FireExplosionTrue3D_Smoke")
    material.use_nodes = True
    nodes = material.node_tree.nodes
    links = material.node_tree.links
    nodes.clear()

    output = nodes.new("ShaderNodeOutputMaterial")
    output.location = (840, 40)

    texcoord = nodes.new("ShaderNodeTexCoord")
    texcoord.location = (-1380, 60)

    mapping = nodes.new("ShaderNodeMapping")
    mapping.location = (-1160, 60)
    mapping.inputs["Scale"].default_value = (0.9, 0.9, 1.3)

    separate_xyz = nodes.new("ShaderNodeSeparateXYZ")
    separate_xyz.location = (-1160, -420)

    vertical_bias = nodes.new("ShaderNodeMapRange")
    vertical_bias.location = (-920, -420)
    vertical_bias.inputs["From Min"].default_value = -1.00
    vertical_bias.inputs["From Max"].default_value = 1.20
    vertical_bias.inputs["To Min"].default_value = 0.42
    vertical_bias.inputs["To Max"].default_value = 1.18
    vertical_bias.clamp = True

    radial = nodes.new("ShaderNodeVectorMath")
    radial.location = (-1160, -220)
    radial.operation = "LENGTH"

    radial_map = nodes.new("ShaderNodeMapRange")
    radial_map.location = (-920, -220)
    radial_map.inputs["From Min"].default_value = 0.2
    radial_map.inputs["From Max"].default_value = 1.95
    radial_map.inputs["To Min"].default_value = 1.0
    radial_map.inputs["To Max"].default_value = 0.0
    radial_map.clamp = True

    noise = nodes.new("ShaderNodeTexNoise")
    noise.location = (-920, 60)
    noise.noise_dimensions = "4D"
    noise.inputs["Scale"].default_value = 4.2
    noise.inputs["Detail"].default_value = 10.0
    noise.inputs["Roughness"].default_value = 0.55
    noise.inputs["Distortion"].default_value = 0.10

    ramp = nodes.new("ShaderNodeValToRGB")
    ramp.name = "SmokeDensityRamp"
    ramp.location = (-700, 60)
    ramp.color_ramp.elements[0].position = 0.50
    ramp.color_ramp.elements[1].position = 0.93

    density_bw = nodes.new("ShaderNodeRGBToBW")
    density_bw.location = (-470, 120)

    shape_mul = nodes.new("ShaderNodeMath")
    shape_mul.location = (-250, 20)
    shape_mul.operation = "MULTIPLY"

    density_mul = nodes.new("ShaderNodeMath")
    density_mul.location = (-90, 20)
    density_mul.operation = "MULTIPLY"

    smoke_value = nodes.new("ShaderNodeValue")
    smoke_value.name = "SmokeValue"
    smoke_value.location = (-700, -170)
    smoke_value.outputs[0].default_value = 0.0

    density_strength = nodes.new("ShaderNodeMath")
    density_strength.name = "SmokeDensityStrength"
    density_strength.location = (-30, 20)
    density_strength.operation = "MULTIPLY"
    density_strength.inputs[1].default_value = 1.05

    principled_volume = nodes.new("ShaderNodeVolumePrincipled")
    principled_volume.name = "SmokeVolume"
    principled_volume.location = (200, 40)
    principled_volume.inputs["Color"].default_value = (0.055, 0.045, 0.04, 1.0)
    principled_volume.inputs["Absorption Color"].default_value = (0.11, 0.085, 0.07, 1.0)
    principled_volume.inputs["Anisotropy"].default_value = 0.12
    principled_volume.inputs["Emission Strength"].default_value = 0.0

    links.new(texcoord.outputs["Object"], mapping.inputs["Vector"])
    links.new(texcoord.outputs["Object"], separate_xyz.inputs["Vector"])
    links.new(mapping.outputs["Vector"], noise.inputs["Vector"])
    links.new(texcoord.outputs["Object"], radial.inputs[0])
    links.new(separate_xyz.outputs["Z"], vertical_bias.inputs["Value"])
    links.new(radial.outputs["Value"], radial_map.inputs["Value"])
    links.new(noise.outputs["Fac"], ramp.inputs["Fac"])
    links.new(ramp.outputs["Color"], density_bw.inputs["Color"])
    links.new(density_bw.outputs["Val"], shape_mul.inputs[0])
    links.new(radial_map.outputs["Result"], shape_mul.inputs[1])
    links.new(shape_mul.outputs["Value"], density_mul.inputs[0])
    links.new(vertical_bias.outputs["Result"], density_mul.inputs[1])
    links.new(density_mul.outputs["Value"], density_strength.inputs[0])
    links.new(smoke_value.outputs["Value"], density_strength.inputs[1])
    links.new(density_strength.outputs["Value"], principled_volume.inputs["Density"])
    links.new(principled_volume.outputs["Volume"], output.inputs["Volume"])

    for frame, value in ((1, 0.0), (4, 0.0), (6, 0.32), (10, 0.72), (16, 0.92), (24, 0.78)):
        smoke_value.outputs[0].default_value = value
        smoke_value.outputs[0].keyframe_insert("default_value", frame=frame)

    for frame, value in ((1, 0.0), (6, 0.15), (12, 0.55), (18, 0.95), (24, 1.25)):
        noise.inputs["W"].default_value = value
        noise.inputs["W"].keyframe_insert("default_value", frame=frame)

    return material


def make_fire_variant(source_material, name, density_multiplier, emission_multiplier, base_color, absorption_color):
    material = source_material.copy()
    material.name = name
    nodes = material.node_tree.nodes
    nodes["DensityStrength"].inputs[1].default_value = density_multiplier
    nodes["EmissionStrength"].inputs[1].default_value = emission_multiplier
    nodes["FireVolume"].inputs["Color"].default_value = base_color
    nodes["FireVolume"].inputs["Absorption Color"].default_value = absorption_color
    return material


def create_displaced_volume(name, location, base_scale, material, displace_strength, texture_scale, texture_depth, texture_distortion=0.0):
    bpy.ops.mesh.primitive_ico_sphere_add(subdivisions=5, radius=1.0, location=location)
    obj = bpy.context.object
    obj.name = name
    obj.scale = base_scale
    bpy.ops.object.shade_smooth()

    subsurf = obj.modifiers.new(name="Subsurf", type="SUBSURF")
    subsurf.levels = 1
    subsurf.render_levels = 2

    control = bpy.data.objects.new(f"{name}_NoiseCtrl", None)
    bpy.context.collection.objects.link(control)
    control.location = location

    texture = bpy.data.textures.new(f"T_{name}_Displace", type="CLOUDS")
    texture.noise_scale = texture_scale
    texture.noise_depth = texture_depth
    texture.intensity = 1.0
    if hasattr(texture, "nabla"):
        texture.nabla = 0.01
    if hasattr(texture, "noise_basis"):
        texture.noise_basis = "BLENDER_ORIGINAL"
    if hasattr(texture, "noise_type"):
        texture.noise_type = "SOFT_NOISE"

    displace = obj.modifiers.new(name="Displace", type="DISPLACE")
    displace.texture = texture
    displace.texture_coords = "OBJECT"
    displace.texture_coords_object = control
    displace.strength = displace_strength
    displace.mid_level = 0.5

    obj.data.materials.clear()
    obj.data.materials.append(material)
    return obj, control


def animate_noise_control(control, start_frame, end_frame, offset, rotation_z):
    control.location = offset
    control.rotation_euler = (0.0, 0.0, 0.0)
    control.keyframe_insert(data_path="location", frame=start_frame)
    control.keyframe_insert(data_path="rotation_euler", frame=start_frame)

    control.location = (offset[0] + 0.28, offset[1] - 0.12, offset[2] + 0.95)
    control.rotation_euler = (0.0, 0.0, rotation_z)
    control.keyframe_insert(data_path="location", frame=end_frame)
    control.keyframe_insert(data_path="rotation_euler", frame=end_frame)


def animate_object(obj, scale_keys, location_keys):
    for frame, scale in scale_keys:
        obj.scale = scale
        obj.keyframe_insert(data_path="scale", frame=frame)
    for frame, location in location_keys:
        obj.location = location
        obj.keyframe_insert(data_path="location", frame=frame)


def add_fire_sparks():
    material = bpy.data.materials.new("M_FireExplosionSpark")
    material.use_nodes = True
    nodes = material.node_tree.nodes
    links = material.node_tree.links
    nodes.clear()
    output = nodes.new("ShaderNodeOutputMaterial")
    emission = nodes.new("ShaderNodeEmission")
    emission.location = (0, 0)
    emission.inputs["Color"].default_value = (1.0, 0.52, 0.10, 1.0)
    emission.inputs["Strength"].default_value = 4.8
    links.new(emission.outputs["Emission"], output.inputs["Surface"])

    sparks = []
    spark_specs = (
        ((0.02, -0.05, 0.78), (0.55, -0.30, 2.10), 0.03, 2, 9),
        ((-0.08, 0.02, 0.82), (-0.46, 0.20, 1.90), 0.028, 2, 8),
        ((0.10, 0.01, 0.85), (0.28, 0.34, 2.25), 0.032, 3, 10),
        ((-0.02, -0.02, 0.80), (-0.18, -0.44, 2.05), 0.026, 3, 9),
        ((0.00, 0.06, 0.76), (0.10, 0.48, 1.80), 0.024, 4, 10),
        ((0.04, -0.01, 0.83), (0.42, -0.08, 2.00), 0.03, 2, 8),
    )
    for index, (start, end, radius, start_frame, end_frame) in enumerate(spark_specs):
        bpy.ops.mesh.primitive_uv_sphere_add(radius=radius, location=start, segments=16, ring_count=8)
        spark = bpy.context.object
        spark.name = f"FireSpark_{index:02d}"
        spark.data.materials.append(material)
        bpy.ops.object.shade_smooth()

        spark.location = start
        spark.scale = (1.0, 1.0, 1.0)
        spark.keyframe_insert(data_path="location", frame=start_frame)
        spark.keyframe_insert(data_path="scale", frame=start_frame)

        spark.location = end
        spark.scale = (0.7, 0.7, 0.7)
        spark.keyframe_insert(data_path="location", frame=end_frame)
        spark.keyframe_insert(data_path="scale", frame=end_frame)

        spark.scale = (0.0, 0.0, 0.0)
        spark.keyframe_insert(data_path="scale", frame=end_frame + 3)
        sparks.append(spark)

    return sparks


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


def build_effect():
    ensure_dirs()
    clean_scene()
    scene = configure_scene()
    add_ground()
    hero_camera, side_camera = add_cameras(scene)
    add_lights()

    fire_material = build_fire_material()
    fire_lobe_material = make_fire_variant(
        fire_material,
        "M_FireExplosionTrue3D_FireWarm",
        density_multiplier=0.70,
        emission_multiplier=1.85,
        base_color=(0.20, 0.11, 0.07, 1.0),
        absorption_color=(0.24, 0.10, 0.04, 1.0),
    )
    fire_shell_material = make_fire_variant(
        fire_material,
        "M_FireExplosionTrue3D_FireShell",
        density_multiplier=0.38,
        emission_multiplier=0.34,
        base_color=(0.15, 0.09, 0.06, 1.0),
        absorption_color=(0.16, 0.07, 0.03, 1.0),
    )
    smoke_material = build_smoke_material()

    fire_core, fire_core_ctrl = create_displaced_volume(
        "FireCore",
        location=(0.0, 0.0, 0.88),
        base_scale=(0.22, 0.22, 0.18),
        material=fire_shell_material,
        displace_strength=0.26,
        texture_scale=0.58,
        texture_depth=3,
    )
    fire_shell, fire_shell_ctrl = create_displaced_volume(
        "FireShell",
        location=(0.0, 0.0, 0.55),
        base_scale=(0.18, 0.18, 0.08),
        material=fire_lobe_material,
        displace_strength=0.18,
        texture_scale=0.78,
        texture_depth=2,
    )
    fire_lobe_a, fire_lobe_a_ctrl = create_displaced_volume(
        "FireLobeA",
        location=(0.24, -0.18, 0.98),
        base_scale=(0.12, 0.10, 0.14),
        material=fire_lobe_material,
        displace_strength=0.22,
        texture_scale=0.52,
        texture_depth=3,
    )
    fire_lobe_b, fire_lobe_b_ctrl = create_displaced_volume(
        "FireLobeB",
        location=(-0.28, 0.20, 0.90),
        base_scale=(0.10, 0.12, 0.12),
        material=fire_material,
        displace_strength=0.20,
        texture_scale=0.50,
        texture_depth=2,
    )
    smoke_main, smoke_main_ctrl = create_displaced_volume(
        "SmokeMain",
        location=(0.0, 0.0, 0.94),
        base_scale=(0.16, 0.16, 0.12),
        material=smoke_material,
        displace_strength=0.16,
        texture_scale=0.90,
        texture_depth=2,
    )
    smoke_cap, smoke_cap_ctrl = create_displaced_volume(
        "SmokeCap",
        location=(0.18, 0.10, 1.12),
        base_scale=(0.08, 0.07, 0.08),
        material=smoke_material,
        displace_strength=0.14,
        texture_scale=0.74,
        texture_depth=2,
    )

    animate_noise_control(fire_core_ctrl, FRAME_START, FRAME_END, (0.0, 0.0, 0.88), math.radians(54.0))
    animate_noise_control(fire_shell_ctrl, FRAME_START, FRAME_END, (0.0, 0.0, 0.55), math.radians(-22.0))
    animate_noise_control(fire_lobe_a_ctrl, FRAME_START, FRAME_END, (0.24, -0.18, 0.98), math.radians(41.0))
    animate_noise_control(fire_lobe_b_ctrl, FRAME_START, FRAME_END, (-0.28, 0.20, 0.90), math.radians(-33.0))
    animate_noise_control(smoke_main_ctrl, FRAME_START, FRAME_END, (0.0, 0.0, 0.94), math.radians(18.0))
    animate_noise_control(smoke_cap_ctrl, FRAME_START, FRAME_END, (0.18, 0.10, 1.12), math.radians(-18.0))

    animate_object(
        fire_core,
        scale_keys=(
            (1, (0.20, 0.20, 0.16)),
            (2, (0.50, 0.50, 0.36)),
            (4, (0.96, 0.96, 0.72)),
            (7, (1.24, 1.20, 0.94)),
            (12, (1.46, 1.42, 1.08)),
            (18, (1.58, 1.54, 1.14)),
            (24, (1.46, 1.40, 1.02)),
        ),
        location_keys=(
            (1, (0.0, 0.0, 0.88)),
            (6, (0.02, 0.0, 1.06)),
            (12, (0.02, -0.01, 1.28)),
            (24, (0.00, -0.02, 1.46)),
        ),
    )
    animate_object(
        fire_shell,
        scale_keys=(
            (1, (0.10, 0.10, 0.05)),
            (3, (0.56, 0.56, 0.16)),
            (5, (0.96, 0.96, 0.22)),
            (8, (1.28, 1.28, 0.28)),
            (14, (1.42, 1.42, 0.32)),
            (24, (1.30, 1.30, 0.28)),
        ),
        location_keys=(
            (1, (0.0, 0.0, 0.55)),
            (10, (0.0, 0.0, 0.60)),
            (24, (0.0, 0.0, 0.66)),
        ),
    )
    animate_object(
        fire_lobe_a,
        scale_keys=(
            (1, (0.08, 0.07, 0.10)),
            (3, (0.28, 0.24, 0.34)),
            (6, (0.64, 0.52, 0.76)),
            (10, (0.86, 0.70, 1.00)),
            (18, (0.94, 0.78, 1.06)),
            (24, (0.78, 0.64, 0.88)),
        ),
        location_keys=(
            (1, (0.24, -0.18, 0.98)),
            (8, (0.42, -0.32, 1.24)),
            (16, (0.54, -0.40, 1.46)),
            (24, (0.58, -0.42, 1.60)),
        ),
    )
    animate_object(
        fire_lobe_b,
        scale_keys=(
            (1, (0.08, 0.09, 0.09)),
            (3, (0.24, 0.28, 0.28)),
            (6, (0.52, 0.60, 0.64)),
            (10, (0.72, 0.82, 0.82)),
            (18, (0.78, 0.88, 0.88)),
            (24, (0.68, 0.74, 0.74)),
        ),
        location_keys=(
            (1, (-0.28, 0.20, 0.90)),
            (8, (-0.42, 0.34, 1.16)),
            (16, (-0.52, 0.44, 1.32)),
            (24, (-0.56, 0.48, 1.46)),
        ),
    )
    animate_object(
        smoke_main,
        scale_keys=(
            (1, (0.10, 0.10, 0.08)),
            (5, (0.18, 0.18, 0.12)),
            (8, (0.88, 0.88, 0.64)),
            (12, (1.26, 1.22, 0.92)),
            (18, (1.54, 1.50, 1.16)),
            (24, (1.72, 1.66, 1.24)),
        ),
        location_keys=(
            (1, (0.0, 0.0, 0.94)),
            (10, (0.0, 0.0, 1.18)),
            (18, (0.02, 0.0, 1.42)),
            (24, (0.03, -0.01, 1.62)),
        ),
    )
    animate_object(
        smoke_cap,
        scale_keys=(
            (1, (0.05, 0.05, 0.05)),
            (7, (0.12, 0.10, 0.12)),
            (10, (0.46, 0.40, 0.48)),
            (14, (0.72, 0.60, 0.76)),
            (20, (0.86, 0.74, 0.88)),
            (24, (0.92, 0.80, 0.96)),
        ),
        location_keys=(
            (1, (0.18, 0.10, 1.12)),
            (10, (0.28, 0.18, 1.44)),
            (20, (0.36, 0.24, 1.78)),
            (24, (0.38, 0.24, 1.92)),
        ),
    )

    bpy.ops.wm.save_as_mainfile(filepath=BLEND_PATH)
    render_previews(scene, hero_camera, side_camera)
    bpy.ops.wm.save_as_mainfile(filepath=BLEND_PATH)


if __name__ == "__main__":
    build_effect()
