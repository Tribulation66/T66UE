"""
Build a Blender source file and preview frames for a reference-driven fire
explosion flipbook using AI-generated explosion stages as source plates.

This script intentionally uses a reference-driven 2.5D workflow:
- stage images generated via the OpenAI Image API
- alpha keyed in Blender from the stage colors
- timed crossfades + scale/rotation animation
- ember particles + a short ignition flash

The result is still rendered from Blender and is meant for user review before
any Unreal import.
"""

import math
import os
from mathutils import Euler, Vector

import bpy


PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
OUT_DIR = os.path.join(PROJECT_ROOT, "SourceAssets", "Import", "VFX", "Idol_Fire_Explosion")
FRAMES_DIR = os.path.join(OUT_DIR, "frames")
BLEND_PATH = os.path.join(OUT_DIR, "FireExplosion_Flipbook_Source.blend")

STAGE_IMAGES = [
    os.path.join(PROJECT_ROOT, "output", "imagegen", "fire_stages", "stage1.png"),
    os.path.join(PROJECT_ROOT, "output", "imagegen", "fire_stages", "stage2.png"),
    os.path.join(PROJECT_ROOT, "output", "imagegen", "fire_stages", "stage3.png"),
]

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
        bpy.data.materials,
        bpy.data.cameras,
        bpy.data.lights,
        bpy.data.worlds,
        bpy.data.images,
    ):
        for datablock in list(datablock_collection):
            if datablock.users == 0:
                datablock_collection.remove(datablock)


def look_at(obj, target):
    direction = target - obj.location
    obj.rotation_euler = direction.to_track_quat("-Z", "Y").to_euler()


def keyframe_vector(target, data_path, frame, value):
    setattr(target, data_path, value)
    target.keyframe_insert(data_path=data_path, frame=frame)


def configure_scene():
    scene = bpy.context.scene
    scene.frame_start = FRAME_START
    scene.frame_end = FRAME_END
    scene.render.engine = "BLENDER_EEVEE"
    scene.eevee.taa_render_samples = 64
    scene.render.resolution_x = RENDER_SIZE
    scene.render.resolution_y = RENDER_SIZE
    scene.render.resolution_percentage = 100
    scene.render.image_settings.file_format = "PNG"
    scene.render.image_settings.color_mode = "RGBA"
    scene.render.film_transparent = True
    scene.render.use_motion_blur = False
    scene.render.fps = 16
    scene.view_settings.look = "AgX - High Contrast"
    scene.view_settings.exposure = 0.05
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
    cam.location = (0.0, -7.2, 4.8)
    cam_data.lens = 46
    cam_data.sensor_width = 36
    look_at(cam, Vector((0.0, 0.0, 1.15)))
    scene.camera = cam

    fill_data = bpy.data.lights.new("FireExplosionFill", "AREA")
    fill_data.energy = 1200
    fill_data.shape = "RECTANGLE"
    fill_data.size = 7.0
    fill_data.size_y = 7.0
    fill = bpy.data.objects.new("FireExplosionFill", fill_data)
    bpy.context.collection.objects.link(fill)
    fill.location = (0.0, -2.8, 5.2)
    look_at(fill, Vector((0.0, 0.0, 1.2)))


def build_stage_material(name, image_path):
    image = bpy.data.images.load(image_path, check_existing=True)

    material = bpy.data.materials.new(name)
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

    image_tex = nodes.new("ShaderNodeTexImage")
    image_tex.location = (-820, 100)
    image_tex.image = image
    image_tex.interpolation = "Smart"
    image_tex.extension = "CLIP"

    tex_coord = nodes.new("ShaderNodeTexCoord")
    tex_coord.location = (-1040, 100)

    saturation = nodes.new("ShaderNodeSeparateColor")
    saturation.location = (-560, 100)
    saturation.mode = "HSV"

    alpha_ramp = nodes.new("ShaderNodeValToRGB")
    alpha_ramp.location = (-300, 10)
    alpha_ramp.color_ramp.elements[0].position = 0.06
    alpha_ramp.color_ramp.elements[0].color = (0.0, 0.0, 0.0, 1.0)
    alpha_ramp.color_ramp.elements[1].position = 0.28
    alpha_ramp.color_ramp.elements[1].color = (1.0, 1.0, 1.0, 1.0)

    alpha_color = nodes.new("ShaderNodeSeparateColor")
    alpha_color.location = (-90, 10)
    alpha_scale = nodes.new("ShaderNodeMath")
    alpha_scale.location = (120, 10)
    alpha_scale.operation = "MULTIPLY"
    alpha_scale.use_clamp = True

    alpha_driver = nodes.new("ShaderNodeValue")
    alpha_driver.name = f"{name}_Alpha"
    alpha_driver.location = (-60, -170)
    alpha_driver.outputs[0].default_value = 0.0

    strength_driver = nodes.new("ShaderNodeValue")
    strength_driver.name = f"{name}_Strength"
    strength_driver.location = (120, 300)
    strength_driver.outputs[0].default_value = 0.0

    links.new(tex_coord.outputs["UV"], image_tex.inputs["Vector"])
    links.new(image_tex.outputs["Color"], saturation.inputs["Color"])
    links.new(saturation.outputs["Green"], alpha_ramp.inputs["Fac"])
    links.new(alpha_ramp.outputs["Color"], alpha_color.inputs["Color"])
    links.new(alpha_color.outputs["Red"], alpha_scale.inputs[0])
    links.new(alpha_driver.outputs[0], alpha_scale.inputs[1])
    links.new(alpha_scale.outputs["Value"], mix.inputs["Fac"])
    links.new(image_tex.outputs["Color"], emission.inputs["Color"])
    links.new(strength_driver.outputs[0], emission.inputs["Strength"])
    links.new(transparent.outputs["BSDF"], mix.inputs[1])
    links.new(emission.outputs["Emission"], mix.inputs[2])
    links.new(mix.outputs["Shader"], output.inputs["Surface"])

    return material, alpha_driver.outputs[0], strength_driver.outputs[0]


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
    emission.inputs["Color"].default_value = (1.0, 0.90, 0.62, 1.0)
    transparent = nodes.new("ShaderNodeBsdfTransparent")
    transparent.location = (260, -90)
    mix = nodes.new("ShaderNodeMixShader")
    mix.location = (390, 0)
    strength = nodes.new("ShaderNodeValue")
    strength.location = (40, 120)
    strength.outputs[0].default_value = 14.0

    links.new(strength.outputs[0], emission.inputs["Strength"])
    links.new(transparent.outputs["BSDF"], mix.inputs[1])
    links.new(emission.outputs["Emission"], mix.inputs[2])
    links.new(mix.outputs["Shader"], output.inputs["Surface"])

    mix.inputs["Fac"].default_value = 0.92
    strength.outputs[0].keyframe_insert(data_path="default_value", frame=1)
    strength.outputs[0].default_value = 4.5
    strength.outputs[0].keyframe_insert(data_path="default_value", frame=2)
    strength.outputs[0].default_value = 0.0
    strength.outputs[0].keyframe_insert(data_path="default_value", frame=4)
    return material


def build_ember_material():
    material = bpy.data.materials.new("M_FireExplosionEmbers")
    material.use_nodes = True
    nodes = material.node_tree.nodes
    links = material.node_tree.links
    nodes.clear()

    output = nodes.new("ShaderNodeOutputMaterial")
    output.location = (360, 0)
    emission = nodes.new("ShaderNodeEmission")
    emission.location = (160, 0)
    emission.inputs["Color"].default_value = (1.0, 0.72, 0.16, 1.0)
    emission.inputs["Strength"].default_value = 8.0
    links.new(emission.outputs["Emission"], output.inputs["Surface"])
    return material


def create_stage_card(name, material, size, location, spin_degrees):
    bpy.ops.mesh.primitive_circle_add(vertices=48, radius=size * 0.5, fill_type="NGON", location=location)
    card = bpy.context.object
    card.name = name
    card.data.materials.append(material)
    card.rotation_euler = Euler(bpy.context.scene.camera.rotation_euler, "XYZ")
    card.rotation_euler.rotate_axis("Z", math.radians(spin_degrees))
    return card


def animate_scalar(socket, keys):
    for frame, value in keys:
        socket.default_value = value
        socket.keyframe_insert(data_path="default_value", frame=frame)


def add_stage_cards():
    stage_specs = [
        {
            "name": "FireStage01",
            "image": STAGE_IMAGES[0],
            "size": 2.1,
            "spin": 6.0,
            "frames": [(1, (0.22, 0.22, 1.0), (0.0, 0.0, 1.02)), (4, (0.58, 0.58, 1.0), (0.0, 0.0, 1.10))],
            "alpha": [(1, 1.0), (3, 1.0), (5, 0.0)],
            "strength": [(1, 3.2), (3, 3.6), (5, 0.0)],
            "spin_end": 20.0,
        },
        {
            "name": "FireStage02",
            "image": STAGE_IMAGES[1],
            "size": 3.0,
            "spin": -14.0,
            "frames": [(2, (0.42, 0.42, 1.0), (0.0, 0.0, 1.08)), (7, (0.90, 0.90, 1.0), (0.0, 0.0, 1.18)), (10, (1.02, 1.02, 1.0), (0.0, 0.0, 1.28))],
            "alpha": [(2, 0.0), (4, 1.0), (8, 1.0), (11, 0.0)],
            "strength": [(2, 0.0), (4, 3.0), (8, 3.3), (11, 0.0)],
            "spin_end": 8.0,
        },
        {
            "name": "FireStage03",
            "image": STAGE_IMAGES[2],
            "size": 3.4,
            "spin": 11.0,
            "frames": [(5, (0.62, 0.62, 1.0), (0.0, 0.0, 1.10)), (9, (1.08, 1.08, 1.0), (0.0, 0.0, 1.20)), (16, (1.34, 1.34, 1.0), (0.0, 0.0, 1.36))],
            "alpha": [(5, 0.0), (8, 0.86), (11, 0.90), (16, 0.0)],
            "strength": [(5, 0.0), (8, 2.8), (11, 2.0), (16, 0.0)],
            "spin_end": 38.0,
        },
    ]

    camera_rotation = Euler(bpy.context.scene.camera.rotation_euler, "XYZ")
    for spec in stage_specs:
        material, alpha_socket, strength_socket = build_stage_material(f"M_{spec['name']}", spec["image"])
        card = create_stage_card(spec["name"], material, spec["size"], spec["frames"][0][2], spec["spin"])
        for frame, scale, location in spec["frames"]:
            keyframe_vector(card, "scale", frame, Vector(scale))
            keyframe_vector(card, "location", frame, Vector(location))
        card.rotation_euler = Euler(camera_rotation, "XYZ")
        card.rotation_euler.rotate_axis("Z", math.radians(spec["spin"]))
        card.keyframe_insert(data_path="rotation_euler", frame=1)
        card.rotation_euler = Euler(camera_rotation, "XYZ")
        card.rotation_euler.rotate_axis("Z", math.radians(spec["spin_end"]))
        card.keyframe_insert(data_path="rotation_euler", frame=16)
        animate_scalar(alpha_socket, spec["alpha"])
        animate_scalar(strength_socket, spec["strength"])


def add_flash():
    flash_material = build_flash_material()
    bpy.ops.mesh.primitive_uv_sphere_add(radius=0.34, location=(0.0, 0.0, 1.10))
    flash = bpy.context.object
    flash.name = "FireExplosionFlash"
    flash.data.materials.append(flash_material)
    keyframe_vector(flash, "scale", 1, Vector((0.18, 0.18, 0.18)))
    keyframe_vector(flash, "scale", 2, Vector((0.74, 0.74, 0.74)))
    keyframe_vector(flash, "scale", 4, Vector((0.04, 0.04, 0.04)))


def add_embers():
    ember_material = build_ember_material()
    ember_specs = [
        ((0.00, 0.00, 1.10), (0.86, 0.20, 1.72), 7),
        ((0.00, 0.00, 1.10), (-0.78, -0.16, 1.64), 8),
        ((0.00, 0.00, 1.12), (0.34, 0.92, 1.98), 9),
        ((0.00, 0.00, 1.08), (-0.24, -0.96, 1.88), 8),
        ((0.00, 0.00, 1.10), (1.02, -0.48, 1.76), 7),
        ((0.00, 0.00, 1.10), (-1.04, 0.54, 1.84), 8),
    ]

    for index, (start, end, end_frame) in enumerate(ember_specs):
        bpy.ops.mesh.primitive_ico_sphere_add(subdivisions=1, radius=0.05 - (index * 0.003), location=start)
        ember = bpy.context.object
        ember.name = f"FireExplosionEmber_{index:02d}"
        ember.data.materials.append(ember_material)
        mid_frame = max(3, end_frame - 2)
        keyframe_vector(ember, "location", 2, Vector(start))
        keyframe_vector(
            ember,
            "location",
            mid_frame,
            Vector(((start[0] + end[0]) * 0.55, (start[1] + end[1]) * 0.55, (start[2] + end[2]) * 0.62)),
        )
        keyframe_vector(ember, "location", end_frame, Vector(end))
        keyframe_vector(ember, "scale", 2, Vector((1.0, 1.0, 1.0)))
        keyframe_vector(ember, "scale", mid_frame, Vector((0.72, 0.72, 0.72)))
        keyframe_vector(ember, "scale", end_frame, Vector((0.05, 0.05, 0.05)))


def build_effect():
    add_stage_cards()
    add_flash()
    add_embers()


def render_frames(scene):
    for frame in range(FRAME_START, FRAME_END + 1):
        scene.frame_set(frame)
        scene.render.filepath = os.path.join(FRAMES_DIR, f"fire_explosion_{frame:02d}.png")
        bpy.ops.render.render(write_still=True)
        print(f"[FireExplosionAI] Rendered frame {frame}/{FRAME_END}")


def main():
    for path in STAGE_IMAGES:
        if not os.path.exists(path):
            raise RuntimeError(f"Missing AI source image: {path}")

    ensure_dirs()
    clean_scene()
    scene = configure_scene()
    add_camera_and_lights(scene)
    build_effect()
    bpy.ops.wm.save_as_mainfile(filepath=BLEND_PATH)
    render_frames(scene)
    bpy.ops.wm.save_as_mainfile(filepath=BLEND_PATH)
    print(f"[FireExplosionAI] Saved blend source to {BLEND_PATH}")


if __name__ == "__main__":
    main()
