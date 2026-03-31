"""
Generate a stylized fire explosion source asset in Blender and render it to
transparent frames for Unreal flipbook import.

This avoids Mantaflow because the installed Blender 5.0 build here is unstable
for scripted fire bakes. The effect is authored procedurally instead:
merged fire lobes, animated internal breakup, flash, sparks, smoke shell, and
an expanding shock ring.

Run from the repo root or via Blender:
  "C:/Program Files/Blender Foundation/Blender 5.0/blender.exe" ^
    --background --python "C:/UE/T66/Scripts/GenerateFireExplosionFlipbook.py"
"""

import math
import os
from mathutils import Euler, Vector

import bpy


PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
OUT_DIR = os.path.join(PROJECT_ROOT, "SourceAssets", "Import", "VFX", "Idol_Fire_Explosion")
FRAMES_DIR = os.path.join(OUT_DIR, "frames")
BLEND_PATH = os.path.join(OUT_DIR, "FireExplosion_Flipbook_Source.blend")

FRAME_START = 1
FRAME_END = 16
RENDER_SIZE = 512


def ensure_dirs():
    os.makedirs(OUT_DIR, exist_ok=True)
    os.makedirs(FRAMES_DIR, exist_ok=True)


def clean_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)

    for datablock_collection in (
        bpy.data.meshes,
        bpy.data.metaballs,
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


def set_active_only(obj):
    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj


def look_at(obj, target):
    direction = target - obj.location
    obj.rotation_euler = direction.to_track_quat("-Z", "Y").to_euler()


def keyframe_value(target, data_path, frame, value):
    setattr(target, data_path, value)
    target.keyframe_insert(data_path=data_path, frame=frame)


def keyframe_vector(target, data_path, frame, value):
    setattr(target, data_path, value)
    target.keyframe_insert(data_path=data_path, frame=frame)


def keyframe_node_input(node, input_name, frame, value):
    socket = node.inputs[input_name]
    if isinstance(value, (tuple, list, Vector, Euler)):
        for index, component in enumerate(value):
            socket.default_value[index] = component
    else:
        socket.default_value = value
    socket.keyframe_insert(data_path="default_value", frame=frame)


def configure_scene():
    scene = bpy.context.scene
    scene.frame_start = FRAME_START
    scene.frame_end = FRAME_END
    scene.render.engine = "BLENDER_EEVEE"
    scene.eevee.taa_render_samples = 64
    scene.eevee.use_taa_reprojection = True
    scene.render.resolution_x = RENDER_SIZE
    scene.render.resolution_y = RENDER_SIZE
    scene.render.resolution_percentage = 100
    scene.render.image_settings.file_format = "PNG"
    scene.render.image_settings.color_mode = "RGBA"
    scene.render.film_transparent = True
    scene.render.use_motion_blur = False
    scene.render.fps = 16

    scene.view_settings.look = "AgX - High Contrast"
    scene.view_settings.exposure = 0.12
    scene.view_settings.gamma = 1.0

    world = bpy.data.worlds.new("FireExplosionWorld")
    world.use_nodes = True
    bg = world.node_tree.nodes.get("Background")
    if bg:
        bg.inputs[0].default_value = (0.0, 0.0, 0.0, 1.0)
        bg.inputs[1].default_value = 0.0
    scene.world = world

    return scene


def add_camera_and_lights(scene):
    cam_data = bpy.data.cameras.new("FireExplosionCamera")
    cam = bpy.data.objects.new("FireExplosionCamera", cam_data)
    bpy.context.collection.objects.link(cam)
    cam.location = (0.0, -7.3, 4.9)
    cam_data.lens = 46
    cam_data.sensor_width = 36
    look_at(cam, Vector((0.0, 0.0, 1.15)))
    scene.camera = cam

    fill_data = bpy.data.lights.new("FireExplosionFill", "AREA")
    fill_data.energy = 1800
    fill_data.shape = "RECTANGLE"
    fill_data.size = 7.0
    fill_data.size_y = 7.0
    fill = bpy.data.objects.new("FireExplosionFill", fill_data)
    bpy.context.collection.objects.link(fill)
    fill.location = (0.0, -3.0, 5.6)
    look_at(fill, Vector((0.0, 0.0, 1.2)))

    rim_data = bpy.data.lights.new("FireExplosionRim", "POINT")
    rim_data.energy = 900
    rim_data.color = (1.0, 0.42, 0.08)
    rim = bpy.data.objects.new("FireExplosionRim", rim_data)
    bpy.context.collection.objects.link(rim)
    rim.location = (1.3, 1.8, 2.0)


def create_driver_empty(name, location):
    empty = bpy.data.objects.new(name, None)
    empty.empty_display_type = "SPHERE"
    empty.location = location
    bpy.context.collection.objects.link(empty)
    return empty


def build_fire_material(controller):
    material = bpy.data.materials.new("M_FireExplosion")
    material.use_nodes = True
    material.blend_method = "BLEND"

    nodes = material.node_tree.nodes
    links = material.node_tree.links
    nodes.clear()

    output = nodes.new("ShaderNodeOutputMaterial")
    output.location = (900, 0)
    mix = nodes.new("ShaderNodeMixShader")
    mix.location = (700, 0)
    transparent = nodes.new("ShaderNodeBsdfTransparent")
    transparent.location = (500, -140)
    emission = nodes.new("ShaderNodeEmission")
    emission.location = (500, 120)
    tex_coord = nodes.new("ShaderNodeTexCoord")
    tex_coord.location = (-1200, 20)
    mapping = nodes.new("ShaderNodeMapping")
    mapping.name = "FireCardMapping"
    mapping.location = (-980, 20)
    mapping.inputs["Location"].default_value = (-0.5, -0.5, 0.0)
    mapping.inputs["Scale"].default_value = (1.05, 1.05, 1.0)

    radial = nodes.new("ShaderNodeTexGradient")
    radial.location = (-760, 320)
    radial.gradient_type = "SPHERICAL"
    radial_ramp = nodes.new("ShaderNodeValToRGB")
    radial_ramp.location = (-520, 320)
    radial_ramp.color_ramp.elements[0].position = 0.0
    radial_ramp.color_ramp.elements[0].color = (1.0, 1.0, 1.0, 1.0)
    radial_ramp.color_ramp.elements[1].position = 0.94
    radial_ramp.color_ramp.elements[1].color = (0.0, 0.0, 0.0, 1.0)

    w_driver = nodes.new("ShaderNodeValue")
    w_driver.name = "FireCardWDriver"
    w_driver.location = (-1180, -220)
    w_driver.outputs[0].default_value = 0.0

    object_info = nodes.new("ShaderNodeObjectInfo")
    object_info.location = (-1180, -420)
    random_scale_a = nodes.new("ShaderNodeMath")
    random_scale_a.location = (-980, -360)
    random_scale_a.operation = "MULTIPLY"
    random_scale_a.inputs[1].default_value = 3.2

    random_scale_b = nodes.new("ShaderNodeMath")
    random_scale_b.location = (-980, -500)
    random_scale_b.operation = "MULTIPLY"
    random_scale_b.inputs[1].default_value = 6.1

    w_add_a = nodes.new("ShaderNodeMath")
    w_add_a.location = (-760, -300)
    w_add_a.operation = "ADD"

    w_add_b = nodes.new("ShaderNodeMath")
    w_add_b.location = (-760, -460)
    w_add_b.operation = "ADD"

    primary_noise = nodes.new("ShaderNodeTexNoise")
    primary_noise.name = "FireCardPrimaryNoise"
    primary_noise.location = (-760, 150)
    primary_noise.noise_dimensions = "4D"
    primary_noise.inputs["Scale"].default_value = 2.8
    primary_noise.inputs["Detail"].default_value = 9.0
    primary_noise.inputs["Roughness"].default_value = 0.58
    primary_noise.inputs["Distortion"].default_value = 0.18

    detail_noise = nodes.new("ShaderNodeTexNoise")
    detail_noise.name = "FireCardDetailNoise"
    detail_noise.location = (-760, -40)
    detail_noise.noise_dimensions = "4D"
    detail_noise.inputs["Scale"].default_value = 9.0
    detail_noise.inputs["Detail"].default_value = 4.0
    detail_noise.inputs["Roughness"].default_value = 0.42

    detail_scale = nodes.new("ShaderNodeMath")
    detail_scale.location = (-520, -40)
    detail_scale.operation = "MULTIPLY"
    detail_scale.inputs[1].default_value = 0.18

    noise_add = nodes.new("ShaderNodeMath")
    noise_add.location = (-300, 90)
    noise_add.operation = "ADD"
    noise_add.use_clamp = True

    shape_multiply = nodes.new("ShaderNodeMath")
    shape_multiply.location = (-80, 160)
    shape_multiply.operation = "MULTIPLY"
    shape_multiply.use_clamp = True

    alpha_ramp = nodes.new("ShaderNodeValToRGB")
    alpha_ramp.location = (130, -10)
    alpha_ramp.color_ramp.elements[0].position = 0.18
    alpha_ramp.color_ramp.elements[0].color = (0.0, 0.0, 0.0, 1.0)
    alpha_ramp.color_ramp.elements[1].position = 0.80
    alpha_ramp.color_ramp.elements[1].color = (1.0, 1.0, 1.0, 1.0)

    fire_ramp = nodes.new("ShaderNodeValToRGB")
    fire_ramp.location = (130, 210)
    fire_ramp.color_ramp.elements[0].position = 0.0
    fire_ramp.color_ramp.elements[0].color = (0.02, 0.00, 0.00, 1.0)
    fire_ramp.color_ramp.elements[1].position = 1.0
    fire_ramp.color_ramp.elements[1].color = (1.0, 0.88, 0.44, 1.0)
    for position, color in (
        (0.15, (0.12, 0.00, 0.00, 1.0)),
        (0.36, (0.48, 0.02, 0.00, 1.0)),
        (0.58, (0.90, 0.10, 0.00, 1.0)),
        (0.80, (1.0, 0.36, 0.04, 1.0)),
        (0.93, (1.0, 0.72, 0.14, 1.0)),
    ):
        element = fire_ramp.color_ramp.elements.new(position)
        element.color = color

    alpha_color = nodes.new("ShaderNodeSeparateColor")
    alpha_color.location = (360, -10)
    alpha_scale = nodes.new("ShaderNodeMath")
    alpha_scale.location = (520, -10)
    alpha_scale.operation = "MULTIPLY"
    alpha_scale.use_clamp = True

    fire_strength = nodes.new("ShaderNodeValue")
    fire_strength.name = "FireCardStrength"
    fire_strength.location = (320, 420)
    fire_strength.outputs[0].default_value = 5.5

    fire_alpha = nodes.new("ShaderNodeValue")
    fire_alpha.name = "FireCardAlpha"
    fire_alpha.location = (330, -210)
    fire_alpha.outputs[0].default_value = 0.0

    links.new(tex_coord.outputs["UV"], mapping.inputs["Vector"])
    links.new(mapping.outputs["Vector"], radial.inputs["Vector"])
    links.new(mapping.outputs["Vector"], primary_noise.inputs["Vector"])
    links.new(mapping.outputs["Vector"], detail_noise.inputs["Vector"])
    links.new(radial.outputs["Fac"], radial_ramp.inputs["Fac"])
    links.new(object_info.outputs["Random"], random_scale_a.inputs[0])
    links.new(object_info.outputs["Random"], random_scale_b.inputs[0])
    links.new(w_driver.outputs[0], w_add_a.inputs[0])
    links.new(random_scale_a.outputs["Value"], w_add_a.inputs[1])
    links.new(w_driver.outputs[0], w_add_b.inputs[0])
    links.new(random_scale_b.outputs["Value"], w_add_b.inputs[1])
    links.new(w_add_a.outputs["Value"], primary_noise.inputs["W"])
    links.new(w_add_b.outputs["Value"], detail_noise.inputs["W"])
    links.new(detail_noise.outputs["Fac"], detail_scale.inputs[0])
    links.new(primary_noise.outputs["Fac"], noise_add.inputs[0])
    links.new(detail_scale.outputs["Value"], noise_add.inputs[1])
    links.new(noise_add.outputs["Value"], shape_multiply.inputs[0])
    links.new(radial_ramp.outputs["Color"], shape_multiply.inputs[1])
    links.new(shape_multiply.outputs["Value"], alpha_ramp.inputs["Fac"])
    links.new(shape_multiply.outputs["Value"], fire_ramp.inputs["Fac"])
    links.new(alpha_ramp.outputs["Color"], alpha_color.inputs["Color"])
    links.new(alpha_color.outputs["Red"], alpha_scale.inputs[0])
    links.new(fire_alpha.outputs[0], alpha_scale.inputs[1])
    links.new(alpha_scale.outputs["Value"], mix.inputs["Fac"])
    links.new(fire_ramp.outputs["Color"], emission.inputs["Color"])
    links.new(fire_strength.outputs[0], emission.inputs["Strength"])
    links.new(emission.outputs["Emission"], mix.inputs[2])
    links.new(transparent.outputs["BSDF"], mix.inputs[1])
    links.new(mix.outputs["Shader"], output.inputs["Surface"])

    w_driver.outputs[0].keyframe_insert(data_path="default_value", frame=1)
    w_driver.outputs[0].default_value = 0.85
    w_driver.outputs[0].keyframe_insert(data_path="default_value", frame=8)
    w_driver.outputs[0].default_value = 1.9
    w_driver.outputs[0].keyframe_insert(data_path="default_value", frame=16)

    keyframe_node_input(mapping, "Rotation", FRAME_START, (0.0, 0.0, 0.0))
    keyframe_node_input(mapping, "Rotation", FRAME_END, (0.0, 0.0, math.radians(12.0)))

    fire_strength.outputs[0].keyframe_insert(data_path="default_value", frame=1)
    fire_strength.outputs[0].default_value = 10.0
    fire_strength.outputs[0].keyframe_insert(data_path="default_value", frame=4)
    fire_strength.outputs[0].default_value = 7.2
    fire_strength.outputs[0].keyframe_insert(data_path="default_value", frame=8)
    fire_strength.outputs[0].default_value = 3.0
    fire_strength.outputs[0].keyframe_insert(data_path="default_value", frame=12)
    fire_strength.outputs[0].default_value = 0.35
    fire_strength.outputs[0].keyframe_insert(data_path="default_value", frame=16)

    fire_alpha.outputs[0].keyframe_insert(data_path="default_value", frame=1)
    fire_alpha.outputs[0].default_value = 0.95
    fire_alpha.outputs[0].keyframe_insert(data_path="default_value", frame=4)
    fire_alpha.outputs[0].default_value = 0.78
    fire_alpha.outputs[0].keyframe_insert(data_path="default_value", frame=8)
    fire_alpha.outputs[0].default_value = 0.42
    fire_alpha.outputs[0].keyframe_insert(data_path="default_value", frame=11)
    fire_alpha.outputs[0].default_value = 0.05
    fire_alpha.outputs[0].keyframe_insert(data_path="default_value", frame=14)
    fire_alpha.outputs[0].default_value = 0.0
    fire_alpha.outputs[0].keyframe_insert(data_path="default_value", frame=16)

    return material


def build_smoke_material(controller):
    material = bpy.data.materials.new("M_FireExplosionSmoke")
    material.use_nodes = True
    material.blend_method = "BLEND"

    nodes = material.node_tree.nodes
    links = material.node_tree.links
    nodes.clear()

    output = nodes.new("ShaderNodeOutputMaterial")
    output.location = (850, 0)
    mix = nodes.new("ShaderNodeMixShader")
    mix.location = (650, 0)
    transparent = nodes.new("ShaderNodeBsdfTransparent")
    transparent.location = (450, -120)
    emission = nodes.new("ShaderNodeEmission")
    emission.location = (450, 120)
    emission.inputs["Strength"].default_value = 0.8
    emission.inputs["Color"].default_value = (0.18, 0.10, 0.08, 1.0)

    tex_coord = nodes.new("ShaderNodeTexCoord")
    tex_coord.location = (-980, 0)
    mapping = nodes.new("ShaderNodeMapping")
    mapping.name = "SmokeCardMapping"
    mapping.location = (-780, 0)
    mapping.inputs["Location"].default_value = (-0.5, -0.5, 0.0)

    radial = nodes.new("ShaderNodeTexGradient")
    radial.location = (-560, 220)
    radial.gradient_type = "SPHERICAL"
    radial_ramp = nodes.new("ShaderNodeValToRGB")
    radial_ramp.location = (-340, 220)
    radial_ramp.color_ramp.elements[0].position = 0.0
    radial_ramp.color_ramp.elements[0].color = (1.0, 1.0, 1.0, 1.0)
    radial_ramp.color_ramp.elements[1].position = 0.96
    radial_ramp.color_ramp.elements[1].color = (0.0, 0.0, 0.0, 1.0)

    smoke_noise = nodes.new("ShaderNodeTexNoise")
    smoke_noise.name = "SmokeCardNoise"
    smoke_noise.location = (-560, 0)
    smoke_noise.noise_dimensions = "4D"
    smoke_noise.inputs["Scale"].default_value = 2.4
    smoke_noise.inputs["Detail"].default_value = 9.0
    smoke_noise.inputs["Roughness"].default_value = 0.68

    object_info = nodes.new("ShaderNodeObjectInfo")
    object_info.location = (-980, -240)
    random_scale = nodes.new("ShaderNodeMath")
    random_scale.location = (-780, -240)
    random_scale.operation = "MULTIPLY"
    random_scale.inputs[1].default_value = 3.8

    w_driver = nodes.new("ShaderNodeValue")
    w_driver.name = "SmokeCardWDriver"
    w_driver.location = (-980, -100)
    w_driver.outputs[0].default_value = 0.0

    w_add = nodes.new("ShaderNodeMath")
    w_add.location = (-560, -180)
    w_add.operation = "ADD"

    alpha_multiply = nodes.new("ShaderNodeMath")
    alpha_multiply.location = (-120, 90)
    alpha_multiply.operation = "MULTIPLY"
    alpha_multiply.use_clamp = True

    alpha_ramp = nodes.new("ShaderNodeValToRGB")
    alpha_ramp.location = (120, 0)
    alpha_ramp.color_ramp.elements[0].position = 0.20
    alpha_ramp.color_ramp.elements[0].color = (0.0, 0.0, 0.0, 1.0)
    alpha_ramp.color_ramp.elements[1].position = 0.72
    alpha_ramp.color_ramp.elements[1].color = (0.50, 0.50, 0.50, 1.0)

    smoke_alpha = nodes.new("ShaderNodeValue")
    smoke_alpha.name = "SmokeAlpha"
    smoke_alpha.location = (320, -140)
    smoke_alpha.outputs[0].default_value = 0.0

    alpha_color = nodes.new("ShaderNodeSeparateColor")
    alpha_color.location = (320, 0)
    alpha_scale = nodes.new("ShaderNodeMath")
    alpha_scale.location = (500, 0)
    alpha_scale.operation = "MULTIPLY"
    alpha_scale.use_clamp = True

    smoke_strength = nodes.new("ShaderNodeValue")
    smoke_strength.name = "SmokeStrength"
    smoke_strength.location = (310, 210)
    smoke_strength.outputs[0].default_value = 0.0

    links.new(tex_coord.outputs["UV"], mapping.inputs["Vector"])
    links.new(mapping.outputs["Vector"], radial.inputs["Vector"])
    links.new(mapping.outputs["Vector"], smoke_noise.inputs["Vector"])
    links.new(radial.outputs["Fac"], radial_ramp.inputs["Fac"])
    links.new(object_info.outputs["Random"], random_scale.inputs[0])
    links.new(w_driver.outputs[0], w_add.inputs[0])
    links.new(random_scale.outputs["Value"], w_add.inputs[1])
    links.new(w_add.outputs["Value"], smoke_noise.inputs["W"])
    links.new(smoke_noise.outputs["Fac"], alpha_multiply.inputs[0])
    links.new(radial_ramp.outputs["Color"], alpha_multiply.inputs[1])
    links.new(alpha_multiply.outputs["Value"], alpha_ramp.inputs["Fac"])
    links.new(alpha_ramp.outputs["Color"], alpha_color.inputs["Color"])
    links.new(alpha_color.outputs["Red"], alpha_scale.inputs[0])
    links.new(smoke_alpha.outputs[0], alpha_scale.inputs[1])
    links.new(alpha_scale.outputs["Value"], mix.inputs["Fac"])
    links.new(transparent.outputs["BSDF"], mix.inputs[1])
    links.new(emission.outputs["Emission"], mix.inputs[2])
    links.new(mix.outputs["Shader"], output.inputs["Surface"])
    links.new(smoke_strength.outputs[0], emission.inputs["Strength"])

    w_driver.outputs[0].keyframe_insert(data_path="default_value", frame=1)
    w_driver.outputs[0].default_value = 0.6
    w_driver.outputs[0].keyframe_insert(data_path="default_value", frame=8)
    w_driver.outputs[0].default_value = 1.4
    w_driver.outputs[0].keyframe_insert(data_path="default_value", frame=16)

    smoke_alpha.outputs[0].keyframe_insert(data_path="default_value", frame=1)
    smoke_alpha.outputs[0].default_value = 0.0
    smoke_alpha.outputs[0].keyframe_insert(data_path="default_value", frame=3)
    smoke_alpha.outputs[0].default_value = 0.32
    smoke_alpha.outputs[0].keyframe_insert(data_path="default_value", frame=8)
    smoke_alpha.outputs[0].default_value = 0.40
    smoke_alpha.outputs[0].keyframe_insert(data_path="default_value", frame=12)
    smoke_alpha.outputs[0].default_value = 0.18
    smoke_alpha.outputs[0].keyframe_insert(data_path="default_value", frame=16)

    smoke_strength.outputs[0].keyframe_insert(data_path="default_value", frame=1)
    smoke_strength.outputs[0].default_value = 0.0
    smoke_strength.outputs[0].keyframe_insert(data_path="default_value", frame=4)
    smoke_strength.outputs[0].default_value = 0.72
    smoke_strength.outputs[0].keyframe_insert(data_path="default_value", frame=10)
    smoke_strength.outputs[0].default_value = 0.46
    smoke_strength.outputs[0].keyframe_insert(data_path="default_value", frame=16)

    return material


def build_ring_material():
    material = bpy.data.materials.new("M_FireExplosionRing")
    material.use_nodes = True
    material.blend_method = "BLEND"

    nodes = material.node_tree.nodes
    links = material.node_tree.links
    nodes.clear()

    output = nodes.new("ShaderNodeOutputMaterial")
    output.location = (500, 0)
    emission = nodes.new("ShaderNodeEmission")
    emission.location = (220, 60)
    emission.inputs["Color"].default_value = (1.0, 0.28, 0.03, 1.0)
    transparent = nodes.new("ShaderNodeBsdfTransparent")
    transparent.location = (220, -100)
    mix = nodes.new("ShaderNodeMixShader")
    mix.location = (360, 0)
    strength = nodes.new("ShaderNodeValue")
    strength.name = "RingStrength"
    strength.location = (20, 120)
    strength.outputs[0].default_value = 1.2

    links.new(strength.outputs[0], emission.inputs["Strength"])
    links.new(transparent.outputs["BSDF"], mix.inputs[1])
    links.new(emission.outputs["Emission"], mix.inputs[2])
    links.new(mix.outputs["Shader"], output.inputs["Surface"])

    strength.outputs[0].keyframe_insert(data_path="default_value", frame=1)
    strength.outputs[0].default_value = 0.8
    strength.outputs[0].keyframe_insert(data_path="default_value", frame=4)
    strength.outputs[0].default_value = 0.0
    strength.outputs[0].keyframe_insert(data_path="default_value", frame=6)

    mix.inputs["Fac"].default_value = 0.45
    return material


def build_flash_material():
    material = bpy.data.materials.new("M_FireExplosionFlash")
    material.use_nodes = True
    material.blend_method = "BLEND"

    nodes = material.node_tree.nodes
    links = material.node_tree.links
    nodes.clear()

    output = nodes.new("ShaderNodeOutputMaterial")
    output.location = (540, 0)
    emission = nodes.new("ShaderNodeEmission")
    emission.location = (260, 70)
    emission.inputs["Color"].default_value = (1.0, 0.84, 0.55, 1.0)
    transparent = nodes.new("ShaderNodeBsdfTransparent")
    transparent.location = (260, -90)
    mix = nodes.new("ShaderNodeMixShader")
    mix.location = (390, 0)
    strength = nodes.new("ShaderNodeValue")
    strength.name = "FlashStrength"
    strength.location = (40, 120)
    strength.outputs[0].default_value = 12.0

    links.new(strength.outputs[0], emission.inputs["Strength"])
    links.new(transparent.outputs["BSDF"], mix.inputs[1])
    links.new(emission.outputs["Emission"], mix.inputs[2])
    links.new(mix.outputs["Shader"], output.inputs["Surface"])

    mix.inputs["Fac"].default_value = 0.9
    strength.outputs[0].keyframe_insert(data_path="default_value", frame=1)
    strength.outputs[0].default_value = 3.6
    strength.outputs[0].keyframe_insert(data_path="default_value", frame=2)
    strength.outputs[0].default_value = 0.0
    strength.outputs[0].keyframe_insert(data_path="default_value", frame=4)
    return material


def build_ember_material():
    material = bpy.data.materials.new("M_FireExplosionEmbers")
    material.use_nodes = True
    material.blend_method = "BLEND"

    nodes = material.node_tree.nodes
    links = material.node_tree.links
    nodes.clear()

    output = nodes.new("ShaderNodeOutputMaterial")
    output.location = (360, 0)
    emission = nodes.new("ShaderNodeEmission")
    emission.location = (160, 0)
    emission.inputs["Color"].default_value = (1.0, 0.68, 0.12, 1.0)
    emission.inputs["Strength"].default_value = 10.0
    links.new(emission.outputs["Emission"], output.inputs["Surface"])

    return material


def create_cloud_texture(name, scale, depth, basis="IMPROVED_PERLIN", nabla=0.025):
    texture = bpy.data.textures.new(name, type="CLOUDS")
    texture.noise_scale = scale
    texture.noise_depth = depth
    texture.noise_basis = basis
    texture.nabla = nabla
    return texture


def add_displace_modifier(obj, name, texture, strength):
    modifier = obj.modifiers.new(name, "DISPLACE")
    modifier.texture = texture
    modifier.texture_coords = "LOCAL"
    modifier.strength = strength
    modifier.mid_level = 0.5
    return modifier


def add_subsurf_modifier(obj, levels):
    modifier = obj.modifiers.new("Subsurf", "SUBSURF")
    modifier.levels = levels
    modifier.render_levels = levels
    return modifier


def create_card(name, material, size, location, spin_degrees):
    bpy.ops.mesh.primitive_circle_add(vertices=48, radius=size * 0.5, fill_type="NGON", location=location)
    card = bpy.context.object
    card.name = name
    card.data.materials.append(material)
    card.rotation_euler = Euler(bpy.context.scene.camera.rotation_euler, "XYZ")
    card.rotation_euler.rotate_axis("Z", math.radians(spin_degrees))
    return card


def build_fire_cluster(fire_material):
    card_specs = [
        {
            "name": "FireCardCore",
            "size": 2.8,
            "location": (0.0, 0.0, 1.20),
            "spin": 8.0,
            "frames": [
                (1, (0.16, 0.16, 1.0), (0.0, 0.0, 1.08)),
                (4, (0.92, 0.92, 1.0), (0.0, 0.0, 1.20)),
                (8, (1.24, 1.18, 1.0), (0.0, 0.0, 1.34)),
                (14, (0.78, 0.72, 1.0), (0.0, 0.0, 1.50)),
            ],
        },
        {
            "name": "FireCardRight",
            "size": 2.0,
            "location": (0.18, 0.06, 1.16),
            "spin": 32.0,
            "frames": [
                (1, (0.14, 0.14, 1.0), (0.04, 0.02, 1.10)),
                (4, (0.74, 0.62, 1.0), (0.34, 0.12, 1.20)),
                (8, (0.94, 0.76, 1.0), (0.58, 0.18, 1.32)),
                (14, (0.42, 0.30, 1.0), (0.76, 0.24, 1.46)),
            ],
        },
        {
            "name": "FireCardLeft",
            "size": 2.0,
            "location": (-0.18, -0.08, 1.14),
            "spin": -26.0,
            "frames": [
                (1, (0.14, 0.14, 1.0), (-0.04, -0.02, 1.10)),
                (4, (0.76, 0.64, 1.0), (-0.30, -0.14, 1.20)),
                (8, (0.98, 0.82, 1.0), (-0.54, -0.24, 1.30)),
                (14, (0.44, 0.32, 1.0), (-0.72, -0.34, 1.44)),
            ],
        },
        {
            "name": "FireCardUpper",
            "size": 1.9,
            "location": (0.04, 0.18, 1.18),
            "spin": 58.0,
            "frames": [
                (1, (0.10, 0.10, 1.0), (0.00, 0.06, 1.12)),
                (5, (0.62, 0.78, 1.0), (0.08, 0.36, 1.24)),
                (9, (0.84, 0.98, 1.0), (0.12, 0.54, 1.38)),
                (15, (0.34, 0.42, 1.0), (0.16, 0.72, 1.54)),
            ],
        },
        {
            "name": "FireCardLower",
            "size": 1.6,
            "location": (0.0, -0.18, 1.02),
            "spin": -52.0,
            "frames": [
                (1, (0.12, 0.12, 1.0), (0.00, -0.04, 1.00)),
                (4, (0.56, 0.46, 1.0), (0.00, -0.16, 1.06)),
                (7, (0.74, 0.58, 1.0), (0.00, -0.20, 1.14)),
                (12, (0.28, 0.22, 1.0), (0.00, -0.24, 1.20)),
            ],
        },
    ]

    cards = []
    camera_rotation = Euler(bpy.context.scene.camera.rotation_euler, "XYZ")
    for spec in card_specs:
        card = create_card(spec["name"], fire_material, spec["size"], spec["location"], spec["spin"])
        for frame, scale, location in spec["frames"]:
            keyframe_vector(card, "scale", frame, Vector(scale))
            keyframe_vector(card, "location", frame, Vector(location))
        card.rotation_euler = Euler(camera_rotation, "XYZ")
        card.rotation_euler.rotate_axis("Z", math.radians(spec["spin"]))
        card.keyframe_insert(data_path="rotation_euler", frame=1)
        card.rotation_euler = Euler(camera_rotation, "XYZ")
        card.rotation_euler.rotate_axis("Z", math.radians(spec["spin"] + 24.0))
        card.keyframe_insert(data_path="rotation_euler", frame=16)
        cards.append(card)
    return cards


def build_smoke_shell(smoke_material):
    smoke_specs = [
        {
            "name": "SmokeCardTop",
            "size": 3.1,
            "location": (0.0, 0.0, 1.34),
            "spin": 12.0,
            "frames": [
                (1, (0.10, 0.10, 1.0), (0.0, 0.0, 1.26)),
                (5, (0.86, 0.76, 1.0), (0.0, 0.0, 1.46)),
                (10, (1.16, 1.04, 1.0), (0.0, 0.0, 1.72)),
                (16, (1.24, 1.12, 1.0), (0.0, 0.0, 1.96)),
            ],
        },
        {
            "name": "SmokeCardRight",
            "size": 2.4,
            "location": (0.30, 0.12, 1.26),
            "spin": 38.0,
            "frames": [
                (1, (0.08, 0.08, 1.0), (0.06, 0.02, 1.22)),
                (6, (0.72, 0.58, 1.0), (0.34, 0.16, 1.38)),
                (10, (0.92, 0.74, 1.0), (0.56, 0.24, 1.56)),
                (16, (1.00, 0.82, 1.0), (0.76, 0.34, 1.74)),
            ],
        },
        {
            "name": "SmokeCardLeft",
            "size": 2.3,
            "location": (-0.28, -0.10, 1.22),
            "spin": -30.0,
            "frames": [
                (1, (0.08, 0.08, 1.0), (-0.06, -0.02, 1.18)),
                (6, (0.68, 0.56, 1.0), (-0.30, -0.10, 1.34)),
                (10, (0.88, 0.74, 1.0), (-0.50, -0.18, 1.50)),
                (16, (0.96, 0.82, 1.0), (-0.70, -0.26, 1.68)),
            ],
        },
    ]

    cards = []
    camera_rotation = Euler(bpy.context.scene.camera.rotation_euler, "XYZ")
    for spec in smoke_specs:
        plume = create_card(spec["name"], smoke_material, spec["size"], spec["location"], spec["spin"])
        for frame, scale, location in spec["frames"]:
            keyframe_vector(plume, "scale", frame, Vector(scale))
            keyframe_vector(plume, "location", frame, Vector(location))
        plume.rotation_euler = Euler(camera_rotation, "XYZ")
        plume.rotation_euler.rotate_axis("Z", math.radians(spec["spin"]))
        plume.keyframe_insert(data_path="rotation_euler", frame=1)
        plume.rotation_euler = Euler(camera_rotation, "XYZ")
        plume.rotation_euler.rotate_axis("Z", math.radians(spec["spin"] + 14.0))
        plume.keyframe_insert(data_path="rotation_euler", frame=16)
        cards.append(plume)
    return cards


def add_flash(flash_material):
    bpy.ops.mesh.primitive_uv_sphere_add(radius=0.38, location=(0.0, 0.0, 1.15))
    flash = bpy.context.object
    flash.name = "FireExplosionFlash"
    flash.data.materials.append(flash_material)
    set_active_only(flash)
    bpy.ops.object.shade_smooth()

    keyframe_vector(flash, "scale", 1, Vector((0.18, 0.18, 0.18)))
    keyframe_vector(flash, "scale", 2, Vector((0.82, 0.82, 0.82)))
    keyframe_vector(flash, "scale", 4, Vector((0.05, 0.05, 0.05)))
    return flash


def add_ring(ring_material):
    bpy.ops.mesh.primitive_torus_add(
        major_radius=0.46,
        minor_radius=0.024,
        location=(0.0, 0.0, 0.18),
        rotation=(math.radians(90.0), 0.0, 0.0),
    )
    ring = bpy.context.object
    ring.name = "FireExplosionRing"
    ring.data.materials.append(ring_material)
    set_active_only(ring)
    bpy.ops.object.shade_smooth()

    keyframe_vector(ring, "scale", 1, Vector((0.22, 0.22, 0.22)))
    keyframe_vector(ring, "scale", 3, Vector((0.92, 0.92, 0.92)))
    keyframe_vector(ring, "scale", 6, Vector((1.42, 1.42, 1.42)))
    return ring


def add_embers(ember_material):
    ember_specs = [
        ((0.00, 0.00, 1.18), (0.95, 0.20, 1.90), 8),
        ((0.03, -0.02, 1.10), (-0.88, -0.18, 1.74), 9),
        ((0.04, 0.02, 1.22), (0.42, 1.05, 2.08), 10),
        ((-0.03, 0.04, 1.12), (-0.28, -1.06, 1.96), 9),
        ((0.00, 0.00, 1.18), (1.20, -0.58, 1.78), 7),
        ((0.00, 0.00, 1.18), (-1.10, 0.64, 1.86), 8),
        ((0.01, 0.00, 1.22), (0.18, 1.36, 2.28), 10),
        ((-0.01, 0.00, 1.18), (-0.18, -1.22, 2.16), 10),
        ((0.00, 0.00, 1.16), (0.72, 0.82, 2.42), 11),
        ((0.00, 0.00, 1.16), (-0.76, -0.72, 2.28), 11),
    ]

    for index, (start, end, end_frame) in enumerate(ember_specs):
        bpy.ops.mesh.primitive_ico_sphere_add(subdivisions=1, radius=0.055 - (index * 0.002), location=start)
        ember = bpy.context.object
        ember.name = f"FireExplosionEmber_{index:02d}"
        ember.data.materials.append(ember_material)
        set_active_only(ember)
        bpy.ops.object.shade_smooth()

        start_frame = 1 + (index % 3)
        mid_frame = max(start_frame + 2, end_frame - 2)

        keyframe_vector(ember, "location", start_frame, Vector(start))
        keyframe_vector(ember, "location", mid_frame, Vector((
            (start[0] + end[0]) * 0.55,
            (start[1] + end[1]) * 0.55,
            (start[2] + end[2]) * 0.60,
        )))
        keyframe_vector(ember, "location", end_frame, Vector(end))

        keyframe_vector(ember, "scale", start_frame, Vector((1.0, 1.0, 1.0)))
        keyframe_vector(ember, "scale", mid_frame, Vector((0.72, 0.72, 0.72)))
        keyframe_vector(ember, "scale", end_frame, Vector((0.06, 0.06, 0.06)))


def build_effect():
    fire_controller = create_driver_empty("FireMaterialDriver", Vector((0.0, 0.0, 0.0)))
    smoke_controller = create_driver_empty("SmokeMaterialDriver", Vector((0.0, 0.0, 0.0)))

    fire_material = build_fire_material(fire_controller)
    smoke_material = build_smoke_material(smoke_controller)
    flash_material = build_flash_material()
    ember_material = build_ember_material()

    build_fire_cluster(fire_material)
    build_smoke_shell(smoke_material)
    add_flash(flash_material)
    add_embers(ember_material)


def render_frames(scene):
    for frame in range(FRAME_START, FRAME_END + 1):
        scene.frame_set(frame)
        scene.render.filepath = os.path.join(FRAMES_DIR, f"fire_explosion_{frame:02d}.png")
        bpy.ops.render.render(write_still=True)
        print(f"[FireExplosion] Rendered frame {frame}/{FRAME_END}")


def main():
    ensure_dirs()
    clean_scene()
    scene = configure_scene()
    add_camera_and_lights(scene)
    build_effect()
    bpy.ops.wm.save_as_mainfile(filepath=BLEND_PATH)
    render_frames(scene)
    bpy.ops.wm.save_as_mainfile(filepath=BLEND_PATH)
    print(f"[FireExplosion] Saved blend source to {BLEND_PATH}")


if __name__ == "__main__":
    main()
