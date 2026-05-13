import argparse
import json
import math
import os
from pathlib import Path

import bpy
from mathutils import Vector


VARIANTS = ("A", "B", "C")
ANGLES = {
    "front": 180.0,
    "3qleft": 135.0,
    "side": 90.0,
    "3qright": 225.0,
    "back": 0.0,
}
ORTHO_SCALE = 11.0


def parse_args():
    argv = []
    if "--" in os.sys.argv:
        argv = os.sys.argv[os.sys.argv.index("--") + 1 :]
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", required=True)
    parser.add_argument("--resolution", type=int, default=1024)
    return parser.parse_args(argv)


def reset_scene(resolution):
    bpy.ops.wm.read_factory_settings(use_empty=True)
    scene = bpy.context.scene
    scene.render.engine = "BLENDER_WORKBENCH"
    scene.render.resolution_x = resolution
    scene.render.resolution_y = resolution
    scene.render.film_transparent = False
    scene.render.image_settings.file_format = "PNG"
    scene.render.image_settings.color_mode = "RGBA"
    scene.display.shading.light = "FLAT"
    scene.display.shading.color_type = "TEXTURE"
    scene.display.shading.show_shadows = False
    scene.display.shading.show_cavity = False
    scene.display.shading.background_type = "VIEWPORT"
    scene.display.shading.background_color = (1.0, 1.0, 1.0)
    return scene


def import_glb(path):
    before = set(bpy.data.objects)
    bpy.ops.import_scene.gltf(filepath=str(path))
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


def normalize_scene(imported, meshes, target_height=2.2):
    mins, maxs = world_bbox(meshes)
    center = (mins + maxs) * 0.5
    height = max(maxs.z - mins.z, 1e-5)
    scale = target_height / height
    for obj in imported:
        obj.location = (obj.location - center) * scale
        obj.scale = obj.scale * scale
    bpy.context.view_layer.update()
    mins2, maxs2 = world_bbox(meshes)
    return mins2, maxs2, scale


def total_triangles(meshes):
    total = 0
    for obj in meshes:
        obj.data.calc_loop_triangles()
        total += len(obj.data.loop_triangles)
    return total


def find_largest_image():
    candidates = []
    for image in bpy.data.images:
        if image.name in {"Render Result", "Viewer Node"}:
            continue
        width, height = image.size[:]
        if width <= 0 or height <= 0:
            continue
        candidates.append((width * height, width, height, image))
    if not candidates:
        return None
    candidates.sort(key=lambda row: row[0], reverse=True)
    return candidates[0][3]


def save_image_png(image, path):
    path.parent.mkdir(parents=True, exist_ok=True)
    old_path = image.filepath_raw
    old_format = image.file_format
    try:
        image.filepath_raw = str(path)
        image.file_format = "PNG"
        image.save()
    except Exception:
        image.save_render(str(path))
    finally:
        image.filepath_raw = old_path
        image.file_format = old_format


def collect_uv_triangles(meshes):
    triangles = []
    for obj in meshes:
        mesh = obj.data
        uv_layer = mesh.uv_layers.active
        if uv_layer is None:
            continue
        mesh.calc_loop_triangles()
        for tri in mesh.loop_triangles:
            points = []
            for loop_index in tri.loops:
                uv = uv_layer.data[loop_index].uv
                points.append([float(uv.x % 1.0), float(uv.y % 1.0)])
            triangles.append(points)
    return triangles


def add_camera(center, yaw_deg):
    cam_data = bpy.data.cameras.new(name="ExperimentCamera")
    cam_data.type = "ORTHO"
    cam_data.ortho_scale = ORTHO_SCALE
    cam = bpy.data.objects.new("ExperimentCamera", cam_data)
    bpy.context.scene.collection.objects.link(cam)
    bpy.context.scene.camera = cam

    yaw = math.radians(yaw_deg)
    direction = Vector((math.sin(yaw), -math.cos(yaw), 0.03)).normalized()
    cam.location = center + direction * 5.0

    target = bpy.data.objects.new("ExperimentTarget", None)
    target.location = center
    bpy.context.scene.collection.objects.link(target)
    track = cam.constraints.new(type="TRACK_TO")
    track.target = target
    track.track_axis = "TRACK_NEGATIVE_Z"
    track.up_axis = "UP_Y"
    return cam, target


def render_angles(root, variant, center):
    render_dir = root / "Renders"
    render_dir.mkdir(parents=True, exist_ok=True)
    for angle_name, yaw in ANGLES.items():
        cam, target = add_camera(center, yaw)
        bpy.context.scene.render.filepath = str(render_dir / f"Variant_{variant}_{angle_name}.png")
        bpy.ops.render.render(write_still=True)
        bpy.data.objects.remove(cam, do_unlink=True)
        bpy.data.objects.remove(target, do_unlink=True)


def process_variant(root, variant, resolution):
    scene = reset_scene(resolution)
    glb = root / "Outputs" / f"Variant_{variant}.glb"
    imported, meshes = import_glb(glb)
    mins, maxs, scale = normalize_scene(imported, meshes)
    center = (mins + maxs) * 0.5

    image = find_largest_image()
    texture_path = root / "Textures" / f"Variant_{variant}_albedo.png"
    image_meta = None
    if image is not None:
        save_image_png(image, texture_path)
        image_meta = {
            "name": image.name,
            "width": int(image.size[0]),
            "height": int(image.size[1]),
            "filepath": image.filepath,
        }

    uv_triangles = collect_uv_triangles(meshes)
    analysis_dir = root / "Analysis"
    analysis_dir.mkdir(parents=True, exist_ok=True)
    metadata = {
        "variant": variant,
        "input_glb": str(glb),
        "output_texture": str(texture_path),
        "selected_texture": image_meta,
        "mesh_count": len(meshes),
        "triangle_count": total_triangles(meshes),
        "bounds_after_normalize": {
            "min": [mins.x, mins.y, mins.z],
            "max": [maxs.x, maxs.y, maxs.z],
            "size": [maxs.x - mins.x, maxs.y - mins.y, maxs.z - mins.z],
        },
        "normalization_scale": scale,
        "uv_triangle_count": len(uv_triangles),
        "uv_triangles": uv_triangles,
        "render_engine": "BLENDER_WORKBENCH",
        "render_light": "FLAT",
        "render_color_type": "TEXTURE",
        "camera_type": "ORTHO",
        "camera_ortho_scale": ORTHO_SCALE,
        "render_resolution": resolution,
        "angles_degrees": ANGLES,
    }
    (analysis_dir / f"Variant_{variant}_blender_metadata.json").write_text(
        json.dumps(metadata, indent=2), encoding="ascii"
    )
    render_angles(root, variant, center)
    return metadata


def main():
    args = parse_args()
    root = Path(args.root)
    all_metadata = {}
    for variant in VARIANTS:
        all_metadata[variant] = process_variant(root, variant, args.resolution)
    (root / "Analysis" / "blender_batch_metadata.json").write_text(
        json.dumps(all_metadata, indent=2), encoding="ascii"
    )


if __name__ == "__main__":
    main()
