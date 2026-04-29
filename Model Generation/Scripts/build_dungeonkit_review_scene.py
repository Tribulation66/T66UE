import argparse
import math
import os

import bpy
from mathutils import Matrix, Vector


ASSETS = [
    ("DungeonWall_Straight_A_S1337_Retopo10000.glb", "Wall A", (-3.9, 0.0, 0.0), 0.0),
    ("DungeonWall_Straight_Chains_S1337_Retopo14000.glb", "Wall Chains", (-1.3, 0.0, 0.0), 0.0),
    ("DungeonWall_Straight_BonesNiche_S1337_Retopo14000.glb", "Wall Bones", (1.3, 0.0, 0.0), 0.0),
    ("DungeonWall_Doorway_Arch_Alpha_S1337_Retopo16000.glb", "Door Arch", (3.9, 0.0, 0.0), 90.0),
    ("DungeonFloor_BonesDrain_A_S1337_Retopo6000.glb", "Floor", (-1.3, -2.7, 0.0), 0.0),
    ("DungeonCeiling_ChainAnchor_A_S1337_Retopo8000.glb", "Ceiling", (1.3, -2.7, 0.0), 0.0),
]


def parse_args():
    argv = []
    if "--" in os.sys.argv:
        argv = os.sys.argv[os.sys.argv.index("--") + 1 :]

    parser = argparse.ArgumentParser()
    parser.add_argument("--retopo-dir", required=True)
    parser.add_argument("--output", required=True)
    parser.add_argument("--render")
    parser.add_argument("--resolution", type=int, default=1600)
    return parser.parse_args(argv)


def reset_scene():
    bpy.ops.wm.read_factory_settings(use_empty=True)
    scene = bpy.context.scene
    scene.render.engine = "BLENDER_EEVEE"
    scene.render.film_transparent = False
    scene.render.resolution_x = 1600
    scene.render.resolution_y = 1000
    scene.render.image_settings.file_format = "PNG"
    if hasattr(scene.eevee, "taa_render_samples"):
        scene.eevee.taa_render_samples = 32
    scene.world = bpy.data.worlds.new("DungeonKitReviewWorld")
    scene.world.use_nodes = True
    bg = scene.world.node_tree.nodes["Background"]
    bg.inputs[0].default_value = (0.04, 0.045, 0.05, 1.0)
    bg.inputs[1].default_value = 0.55
    return scene


def import_glb(path):
    before = set(bpy.data.objects)
    bpy.ops.import_scene.gltf(filepath=path)
    imported = [obj for obj in bpy.data.objects if obj not in before]
    meshes = [obj for obj in imported if obj.type == "MESH"]
    if not meshes:
        raise RuntimeError(f"No mesh objects imported from {path}")
    return imported, meshes


def bbox(meshes):
    mins = Vector((math.inf, math.inf, math.inf))
    maxs = Vector((-math.inf, -math.inf, -math.inf))
    for obj in meshes:
        for corner in obj.bound_box:
            world = obj.matrix_world @ Vector(corner)
            mins.x = min(mins.x, world.x)
            mins.y = min(mins.y, world.y)
            mins.z = min(mins.z, world.z)
            maxs.x = max(maxs.x, world.x)
            maxs.y = max(maxs.y, world.y)
            maxs.z = max(maxs.z, world.z)
    return mins, maxs


def add_label(text, loc):
    curve = bpy.data.curves.new(f"Label_{text}", type="FONT")
    curve.body = text
    curve.align_x = "CENTER"
    curve.align_y = "CENTER"
    curve.size = 0.105
    obj = bpy.data.objects.new(f"Label_{text}", curve)
    obj.location = (loc[0], loc[1] - 0.35, -0.85)
    obj.rotation_euler = (math.radians(75), 0.0, 0.0)
    bpy.context.collection.objects.link(obj)
    return obj


def add_lights():
    sun_data = bpy.data.lights.new("ReviewSun", "SUN")
    sun_data.energy = 2.5
    sun = bpy.data.objects.new("ReviewSun", sun_data)
    sun.rotation_euler = (math.radians(45), 0.0, math.radians(35))
    bpy.context.collection.objects.link(sun)

    fill_data = bpy.data.lights.new("ReviewFill", "AREA")
    fill_data.energy = 550
    fill_data.size = 6.0
    fill = bpy.data.objects.new("ReviewFill", fill_data)
    fill.location = (0.0, -5.0, 6.0)
    fill.rotation_euler = (math.radians(60), 0.0, 0.0)
    bpy.context.collection.objects.link(fill)


def add_camera():
    cam_data = bpy.data.cameras.new("ReviewCamera")
    cam_data.type = "ORTHO"
    cam_data.ortho_scale = 9.6
    cam = bpy.data.objects.new("ReviewCamera", cam_data)
    cam.location = (0.0, -9.5, 5.0)
    bpy.context.collection.objects.link(cam)
    bpy.context.scene.camera = cam

    target = bpy.data.objects.new("ReviewCameraTarget", None)
    target.location = (0.0, -1.15, 0.0)
    bpy.context.collection.objects.link(target)
    track = cam.constraints.new(type="TRACK_TO")
    track.target = target
    track.track_axis = "TRACK_NEGATIVE_Z"
    track.up_axis = "UP_Y"


def main():
    args = parse_args()
    scene = reset_scene()
    scene.render.resolution_x = args.resolution
    scene.render.resolution_y = int(args.resolution * 0.625)

    for filename, label, location, yaw_deg in ASSETS:
        path = os.path.join(args.retopo_dir, filename)
        imported, meshes = import_glb(path)
        mins, maxs = bbox(meshes)
        center = (mins + maxs) * 0.5
        size = maxs - mins
        target_size = 1.35
        scale = target_size / max(size.x, size.y, size.z)

        root = bpy.data.objects.new(f"ReviewRoot_{label}", None)
        bpy.context.collection.objects.link(root)
        for obj in imported:
            if obj.parent not in imported:
                obj.parent = root
        yaw = math.radians(yaw_deg)
        rotation = Matrix.Rotation(yaw, 4, "Z")
        root.rotation_euler[2] = yaw
        root.scale = (scale, scale, scale)
        root.location = Vector(location) - rotation @ (center * scale)
        add_label(label, location)

    add_lights()
    add_camera()

    os.makedirs(os.path.dirname(args.output), exist_ok=True)
    bpy.ops.wm.save_as_mainfile(filepath=args.output)

    if args.render:
        os.makedirs(os.path.dirname(args.render), exist_ok=True)
        scene.render.filepath = args.render
        bpy.ops.render.render(write_still=True)


if __name__ == "__main__":
    main()
