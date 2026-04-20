import argparse
import json
import math
import os
from mathutils import Vector

import bpy


def parse_args():
    argv = []
    if "--" in os.sys.argv:
        argv = os.sys.argv[os.sys.argv.index("--") + 1 :]

    parser = argparse.ArgumentParser()
    parser.add_argument("--input", required=True)
    parser.add_argument("--render", required=True)
    parser.add_argument("--metadata")
    parser.add_argument("--export")
    parser.add_argument("--target-tris", type=int)
    parser.add_argument("--yaw", type=float, default=0.0)
    parser.add_argument("--pitch", type=float, default=5.0)
    parser.add_argument("--resolution", type=int, default=1024)
    return parser.parse_args(argv)


def reset_scene():
    bpy.ops.wm.read_factory_settings(use_empty=True)
    scene = bpy.context.scene
    scene.render.engine = "BLENDER_EEVEE"
    scene.render.film_transparent = False
    scene.render.resolution_x = 1024
    scene.render.resolution_y = 1024
    scene.render.image_settings.file_format = "PNG"
    if hasattr(scene.eevee, "taa_render_samples"):
        scene.eevee.taa_render_samples = 32
    if hasattr(scene.eevee, "use_gtao"):
        scene.eevee.use_gtao = True
    if hasattr(scene.eevee, "use_bloom"):
        scene.eevee.use_bloom = False
    scene.world = bpy.data.worlds.new("QAWorld")
    scene.world.use_nodes = True
    bg = scene.world.node_tree.nodes["Background"]
    bg.inputs[0].default_value = (0.92, 0.93, 0.95, 1.0)
    bg.inputs[1].default_value = 0.8
    return scene


def import_glb(path):
    before = set(bpy.data.objects)
    bpy.ops.import_scene.gltf(filepath=path)
    imported = [obj for obj in bpy.data.objects if obj not in before]
    meshes = [obj for obj in imported if obj.type == "MESH"]
    if not meshes:
        raise RuntimeError(f"No mesh objects imported from {path}")
    return imported, meshes


def world_bbox(meshes):
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


def total_triangles(meshes):
    total = 0
    for obj in meshes:
        mesh = obj.data
        mesh.calc_loop_triangles()
        total += len(mesh.loop_triangles)
    return total


def add_lights():
    sun_data = bpy.data.lights.new(name="QASun", type="SUN")
    sun_data.energy = 2.2
    sun = bpy.data.objects.new("QASun", sun_data)
    sun.rotation_euler = (math.radians(40), 0.0, math.radians(25))
    bpy.context.scene.collection.objects.link(sun)

    fill_data = bpy.data.lights.new(name="QAFill", type="AREA")
    fill_data.energy = 2500
    fill_data.shape = "RECTANGLE"
    fill_data.size = 4.0
    fill_data.size_y = 4.0
    fill = bpy.data.objects.new("QAFill", fill_data)
    fill.location = (3.5, -4.0, 4.5)
    fill.rotation_euler = (math.radians(55), 0.0, math.radians(35))
    bpy.context.scene.collection.objects.link(fill)


def add_camera(center, size, yaw_deg, pitch_deg):
    cam_data = bpy.data.cameras.new(name="QACamera")
    cam_data.type = "ORTHO"
    cam = bpy.data.objects.new("QACamera", cam_data)
    bpy.context.scene.collection.objects.link(cam)
    bpy.context.scene.camera = cam

    yaw = math.radians(yaw_deg)
    pitch = math.radians(pitch_deg)
    direction = Vector(
        (
            math.sin(yaw) * math.cos(pitch),
            -math.cos(yaw) * math.cos(pitch),
            math.sin(pitch),
        )
    )

    distance = max(size.x, size.y, size.z) * 4.0 + 2.0
    cam.location = center + direction * distance

    track = cam.constraints.new(type="TRACK_TO")
    target = bpy.data.objects.new("QATarget", None)
    target.location = center
    bpy.context.scene.collection.objects.link(target)
    track.target = target
    track.track_axis = "TRACK_NEGATIVE_Z"
    track.up_axis = "UP_Y"

    cam_data.ortho_scale = max(size.x, size.z) * 1.6 + 0.5
    return cam


def export_glb(path):
    bpy.ops.object.select_all(action="DESELECT")
    for obj in bpy.data.objects:
        if obj.type in {"MESH", "ARMATURE", "EMPTY"}:
            obj.select_set(True)
    bpy.ops.export_scene.gltf(
        filepath=path,
        export_format="GLB",
        use_selection=True,
        export_apply=True,
    )


def decimate_meshes(meshes, target_tris):
    current = total_triangles(meshes)
    if target_tris is None or target_tris <= 0 or current <= target_tris:
        return current, current

    ratio = max(0.01, min(1.0, target_tris / current))
    for obj in meshes:
        mod = obj.modifiers.new(name="QADecimate", type="DECIMATE")
        mod.decimate_type = "COLLAPSE"
        mod.ratio = ratio
        bpy.context.view_layer.objects.active = obj
        bpy.ops.object.modifier_apply(modifier=mod.name)

    return current, total_triangles(meshes)


def ensure_parent(path):
    os.makedirs(os.path.dirname(path), exist_ok=True)


def main():
    args = parse_args()
    scene = reset_scene()
    scene.render.resolution_x = args.resolution
    scene.render.resolution_y = args.resolution

    _, meshes = import_glb(args.input)
    raw_tris = total_triangles(meshes)

    before_tris, after_tris = decimate_meshes(meshes, args.target_tris)
    mins, maxs = world_bbox(meshes)
    center = (mins + maxs) * 0.5
    size = maxs - mins

    add_lights()
    add_camera(center, size, args.yaw, args.pitch)

    ensure_parent(args.render)
    scene.render.filepath = args.render
    bpy.ops.render.render(write_still=True)

    if args.export:
        ensure_parent(args.export)
        export_glb(args.export)

    if args.metadata:
        ensure_parent(args.metadata)
        payload = {
            "input": args.input,
            "render": args.render,
            "export": args.export,
            "raw_triangles": raw_tris,
            "triangles_before_decimate": before_tris,
            "triangles_after_decimate": after_tris,
            "bounds": {
                "min": [mins.x, mins.y, mins.z],
                "max": [maxs.x, maxs.y, maxs.z],
                "size": [size.x, size.y, size.z],
            },
        }
        with open(args.metadata, "w", encoding="ascii") as handle:
            json.dump(payload, handle, indent=2)


if __name__ == "__main__":
    main()
