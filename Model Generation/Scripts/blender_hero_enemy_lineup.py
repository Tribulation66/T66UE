import argparse
import math
import os
from mathutils import Vector

import bpy


ASSETS = [
    (
        "Skeleton Warrior",
        r"C:\UE\T66\Model Generation\Runs\Enemies\Easy\SkeletonWarrior\Retopo\SkeletonWarrior_Front_Green_S1337_D200000_Retopo24k.glb",
        1.0,
        -4.2,
    ),
    (
        "Goblin Charger",
        r"C:\UE\T66\Model Generation\Runs\Enemies\Easy\GoblinCharger\Retopo\GoblinCharger_Front_Green_S1337_D200000_Retopo24k.glb",
        0.92,
        -1.9,
    ),
    (
        "Arthur Hero",
        r"C:\UE\T66\Model Generation\Runs\Arthur\Raw\Arthur_HeroReference_Full_White_S1337_D80000_Decimate40k_WithSword.glb",
        1.75,
        0.8,
    ),
    (
        "Slime Spitter",
        r"C:\UE\T66\Model Generation\Runs\Enemies\Easy\SlimeSpitter\Retopo\SlimeSpitter_Front_Green_S1337_D200000_Retopo10k.glb",
        0.9,
        4.0,
    ),
    (
        "Cave Bat",
        r"C:\UE\T66\Model Generation\Runs\Enemies\Easy\CaveBat\Retopo\CaveBat_Front_Green_S1337_D200000_Retopo14k.glb",
        1.05,
        6.9,
    ),
]


def parse_args():
    argv = []
    if "--" in os.sys.argv:
        argv = os.sys.argv[os.sys.argv.index("--") + 1 :]

    parser = argparse.ArgumentParser()
    parser.add_argument("--output-blend", required=True)
    parser.add_argument("--render")
    return parser.parse_args(argv)


def ensure_parent(path):
    os.makedirs(os.path.dirname(path), exist_ok=True)


def reset_scene():
    bpy.ops.wm.read_factory_settings(use_empty=True)
    scene = bpy.context.scene
    scene.render.engine = "BLENDER_EEVEE"
    scene.render.resolution_x = 1920
    scene.render.resolution_y = 1080
    scene.render.image_settings.file_format = "PNG"
    if hasattr(scene.eevee, "taa_render_samples"):
        scene.eevee.taa_render_samples = 32
    scene.world = bpy.data.worlds.new("LineupWorld")
    scene.world.use_nodes = True
    bg = scene.world.node_tree.nodes["Background"]
    bg.inputs[0].default_value = (0.96, 0.96, 0.98, 1.0)
    bg.inputs[1].default_value = 0.9
    return scene


def import_asset(path):
    before = set(bpy.data.objects)
    bpy.ops.import_scene.gltf(filepath=path)
    imported = [obj for obj in bpy.data.objects if obj not in before]
    meshes = [obj for obj in imported if obj.type == "MESH"]
    if not meshes:
        raise RuntimeError(f"No meshes imported from {path}")
    return imported, meshes


def world_bbox(objects):
    mins = Vector((math.inf, math.inf, math.inf))
    maxs = Vector((-math.inf, -math.inf, -math.inf))
    for obj in objects:
        if obj.type != "MESH":
            continue
        for corner in obj.bound_box:
            world = obj.matrix_world @ Vector(corner)
            mins.x = min(mins.x, world.x)
            mins.y = min(mins.y, world.y)
            mins.z = min(mins.z, world.z)
            maxs.x = max(maxs.x, world.x)
            maxs.y = max(maxs.y, world.y)
            maxs.z = max(maxs.z, world.z)
    return mins, maxs


def move_objects(objects, delta):
    for obj in objects:
        obj.location += delta


def add_floor(center_x, width):
    bpy.ops.mesh.primitive_plane_add(size=1.0, location=(center_x, 0.0, -0.015))
    plane = bpy.context.object
    plane.scale = (width * 0.5, 1.8, 1.0)
    mat = bpy.data.materials.new(name="Floor")
    mat.use_nodes = True
    bsdf = mat.node_tree.nodes.get("Principled BSDF")
    bsdf.inputs["Base Color"].default_value = (0.83, 0.84, 0.87, 1.0)
    bsdf.inputs["Roughness"].default_value = 0.98
    plane.data.materials.append(mat)


def add_lights():
    sun_data = bpy.data.lights.new(name="LineupSun", type="SUN")
    sun_data.energy = 2.0
    sun = bpy.data.objects.new("LineupSun", sun_data)
    sun.rotation_euler = (math.radians(45), 0.0, math.radians(30))
    bpy.context.scene.collection.objects.link(sun)

    fill_data = bpy.data.lights.new(name="LineupFill", type="AREA")
    fill_data.energy = 3200
    fill_data.shape = "RECTANGLE"
    fill_data.size = 7.0
    fill_data.size_y = 7.0
    fill = bpy.data.objects.new("LineupFill", fill_data)
    fill.location = (1.2, -6.0, 4.2)
    fill.rotation_euler = (math.radians(68), 0.0, math.radians(12))
    bpy.context.scene.collection.objects.link(fill)


def add_camera(center_x, width, max_height):
    cam_data = bpy.data.cameras.new(name="LineupCamera")
    cam_data.type = "ORTHO"
    cam = bpy.data.objects.new("LineupCamera", cam_data)
    bpy.context.scene.collection.objects.link(cam)
    bpy.context.scene.camera = cam
    cam.location = (center_x, -10.0, max_height * 0.75)
    cam.rotation_euler = (math.radians(90), 0.0, 0.0)
    cam_data.ortho_scale = max(width * 0.4, max_height * 2.25)
    return cam


def build_lineup():
    placements = []
    max_height = 0.0

    for name, path, scale_factor, target_x in ASSETS:
        imported, meshes = import_asset(path)
        for obj in imported:
            obj.scale *= scale_factor
        bpy.context.view_layer.update()
        mins, maxs = world_bbox(meshes)
        center_x = (mins.x + maxs.x) * 0.5
        delta = Vector((target_x - center_x, -((mins.y + maxs.y) * 0.5), -mins.z))
        move_objects(imported, delta)
        bpy.context.view_layer.update()
        mins, maxs = world_bbox(meshes)
        height = maxs.z - mins.z
        for obj in imported:
            obj.name = f"{name}_{obj.name}"
        placements.append((name, mins, maxs))
        max_height = max(max_height, height)

    scene_min_x = min(item[1].x for item in placements)
    scene_max_x = max(item[2].x for item in placements)
    total_width = scene_max_x - scene_min_x
    center_x = scene_min_x + total_width * 0.5
    add_floor(center_x, total_width + 2.0)
    add_lights()
    add_camera(center_x, total_width, max_height)


def main():
    args = parse_args()
    scene = reset_scene()
    build_lineup()

    ensure_parent(args.output_blend)
    bpy.ops.wm.save_as_mainfile(filepath=args.output_blend)

    if args.render:
        ensure_parent(args.render)
        scene.render.filepath = args.render
        bpy.ops.render.render(write_still=True)


if __name__ == "__main__":
    main()
