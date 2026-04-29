import argparse
import math
import os

import bpy
from mathutils import Vector


ROOM_ASSETS = [
    ("DungeonFloor_BonesDrain_A_UnrealReady.glb", "Floor", (0.0, 0.0, 0.0), 0.0),
    ("DungeonWall_Straight_A_UnrealReady.glb", "Wall A", (-6.5, 0.0, 0.0), 0.0),
    ("DungeonWall_Straight_Chains_UnrealReady.glb", "Wall Chains", (0.0, 6.5, 0.0), -90.0),
    ("DungeonWall_Straight_BonesNiche_UnrealReady.glb", "Wall Bones", (6.5, 0.0, 0.0), 180.0),
    ("DungeonWall_Doorway_Arch_UnrealReady.glb", "Door Arch", (0.0, -6.5, 0.0), 90.0),
]


def parse_args():
    argv = []
    if "--" in os.sys.argv:
        argv = os.sys.argv[os.sys.argv.index("--") + 1 :]

    parser = argparse.ArgumentParser()
    parser.add_argument("--asset-dir", required=True)
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
    if hasattr(scene.eevee, "use_gtao"):
        scene.eevee.use_gtao = True
    scene.world = bpy.data.worlds.new("DungeonKitRoomPreviewWorld")
    scene.world.use_nodes = True
    bg = scene.world.node_tree.nodes["Background"]
    bg.inputs[0].default_value = (0.035, 0.038, 0.042, 1.0)
    bg.inputs[1].default_value = 0.55
    return scene


def import_glb(path):
    before = set(bpy.data.objects)
    bpy.ops.import_scene.gltf(filepath=path)
    imported = [obj for obj in bpy.data.objects if obj not in before]
    meshes = [obj for obj in imported if obj.type == "MESH"]
    if not meshes:
        raise RuntimeError(f"No mesh objects imported from {path}")
    return imported


def add_label(text, location):
    curve = bpy.data.curves.new(f"Label_{text}", type="FONT")
    curve.body = text
    curve.align_x = "CENTER"
    curve.align_y = "CENTER"
    curve.size = 0.35
    obj = bpy.data.objects.new(f"Label_{text}", curve)
    obj.location = (location[0], location[1], 0.05)
    obj.rotation_euler = (0.0, 0.0, math.radians(0.0))
    bpy.context.collection.objects.link(obj)
    return obj


def add_lights():
    sun_data = bpy.data.lights.new("RoomPreviewSun", "SUN")
    sun_data.energy = 2.1
    sun = bpy.data.objects.new("RoomPreviewSun", sun_data)
    sun.rotation_euler = (math.radians(50), 0.0, math.radians(35))
    bpy.context.collection.objects.link(sun)

    fill_data = bpy.data.lights.new("RoomPreviewFill", "AREA")
    fill_data.energy = 900
    fill_data.size = 10.0
    fill = bpy.data.objects.new("RoomPreviewFill", fill_data)
    fill.location = (1.0, -6.0, 10.0)
    fill.rotation_euler = (math.radians(65), 0.0, 0.0)
    bpy.context.collection.objects.link(fill)


def add_camera():
    cam_data = bpy.data.cameras.new("RoomPreviewCamera")
    cam_data.type = "ORTHO"
    cam_data.ortho_scale = 24.0
    cam = bpy.data.objects.new("RoomPreviewCamera", cam_data)
    cam.location = (18.0, -22.0, 18.0)
    bpy.context.collection.objects.link(cam)
    bpy.context.scene.camera = cam

    target = bpy.data.objects.new("RoomPreviewCameraTarget", None)
    target.location = (0.0, 0.0, 4.2)
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

    for filename, label, location, yaw_deg in ROOM_ASSETS:
        imported = import_glb(os.path.join(args.asset_dir, filename))
        root = bpy.data.objects.new(f"RoomRoot_{label}", None)
        bpy.context.collection.objects.link(root)
        for obj in imported:
            if obj.parent not in imported:
                obj.parent = root
        root.location = Vector(location)
        root.rotation_euler[2] = math.radians(yaw_deg)

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
