#!/usr/bin/env python3
"""
Create a Blender side-by-side comparison scene from a list of GLBs.

This is intentionally generic for Pixal3D/TRELLIS experiment review surfaces:
normalize all imported models to the same height, align bases to Z=0, label
them, set a camera that sees the row, and optionally write viewport screenshots.
"""

from __future__ import annotations

import argparse
import math
import sys
from pathlib import Path

import bpy
from mathutils import Vector


def parse_args() -> argparse.Namespace:
    argv = sys.argv
    if "--" in argv:
        argv = argv[argv.index("--") + 1 :]
    else:
        argv = []

    parser = argparse.ArgumentParser(description="Build a side-by-side GLB comparison scene.")
    parser.add_argument("--inputs", nargs="+", type=Path, required=True)
    parser.add_argument("--labels", nargs="+", required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--height", type=float, default=3.0)
    parser.add_argument("--spacing", type=float, default=3.2)
    parser.add_argument("--screenshot-dir", type=Path)
    parser.add_argument("--screenshot-size", type=int, default=1800)
    return parser.parse_args(argv)


def all_mesh_objects() -> list[bpy.types.Object]:
    return [obj for obj in bpy.context.scene.objects if obj.type == "MESH"]


def bounds_for(objects: list[bpy.types.Object]) -> tuple[Vector, Vector]:
    points: list[Vector] = []
    for obj in objects:
        for corner in obj.bound_box:
            points.append(obj.matrix_world @ Vector(corner))

    if not points:
        return Vector((0.0, 0.0, 0.0)), Vector((0.0, 0.0, 0.0))

    min_v = Vector((min(p.x for p in points), min(p.y for p in points), min(p.z for p in points)))
    max_v = Vector((max(p.x for p in points), max(p.y for p in points), max(p.z for p in points)))
    return min_v, max_v


def delete_default_cube() -> None:
    for obj in list(bpy.context.scene.objects):
        if obj.name == "Cube":
            bpy.data.objects.remove(obj, do_unlink=True)


def import_variant(glb_path: Path, label: str, x_location: float, target_height: float) -> dict[str, object]:
    before = set(bpy.context.scene.objects)
    bpy.ops.import_scene.gltf(filepath=str(glb_path))
    imported = [obj for obj in bpy.context.scene.objects if obj not in before]
    meshes = [obj for obj in imported if obj.type == "MESH"]

    if not meshes:
        raise RuntimeError(f"No mesh objects imported from {glb_path}")

    min_v, max_v = bounds_for(meshes)
    height = max(max_v.z - min_v.z, 0.0001)
    scale_factor = target_height / height

    for obj in meshes:
        obj.scale = obj.scale * scale_factor
    bpy.context.view_layer.update()

    min_v, max_v = bounds_for(meshes)
    center = (min_v + max_v) * 0.5
    translation = Vector((x_location - center.x, 0.0 - center.y, 0.0 - min_v.z))
    for obj in meshes:
        obj.location = obj.location + translation
    bpy.context.view_layer.update()

    min_v, max_v = bounds_for(meshes)
    add_label(label, x_location, max_v.z + 0.35)

    return {
        "path": str(glb_path),
        "label": label,
        "mesh_count": len(meshes),
        "scale_factor": scale_factor,
        "bounds_min": [round(min_v.x, 4), round(min_v.y, 4), round(min_v.z, 4)],
        "bounds_max": [round(max_v.x, 4), round(max_v.y, 4), round(max_v.z, 4)],
    }


def add_label(text: str, x_location: float, z_location: float) -> None:
    bpy.ops.object.text_add(location=(x_location, -0.08, z_location), rotation=(math.radians(-90.0), 0.0, 0.0))
    obj = bpy.context.object
    obj.name = f"Label_{text.replace(' ', '_')}"
    obj.data.body = text
    obj.data.align_x = "CENTER"
    obj.data.align_y = "CENTER"
    obj.data.size = 0.22
    obj.scale.x = -1.0
    obj.data.materials.append(label_material())


def label_material() -> bpy.types.Material:
    mat = bpy.data.materials.get("Label_Black")
    if mat:
        return mat
    mat = bpy.data.materials.new("Label_Black")
    mat.diffuse_color = (0.0, 0.0, 0.0, 1.0)
    return mat


def look_at(obj: bpy.types.Object, target: Vector) -> None:
    direction = target - obj.location
    obj.rotation_euler = direction.to_track_quat("-Z", "Y").to_euler()


def setup_camera(x_min: float, x_max: float, target_height: float) -> None:
    center_x = (x_min + x_max) * 0.5
    row_width = max(x_max - x_min, 1.0)
    aspect = 1800 / round(1800 * 0.56)
    camera = bpy.data.objects.get("Camera")
    if camera is None:
        bpy.ops.object.camera_add()
        camera = bpy.context.object
    camera.location = (center_x, max(10.0, row_width * 0.9), target_height * 1.35)
    camera.data.type = "ORTHO"
    camera.data.ortho_scale = max(target_height * 1.8, row_width * 1.75 / aspect)
    look_at(camera, Vector((center_x, 0.0, target_height * 0.55)))
    bpy.context.scene.camera = camera

    light = bpy.data.objects.get("Light")
    if light is not None:
        light.location = (center_x, 3.5, 6.0)
        light.data.energy = 600


def save_screenshots(args: argparse.Namespace, x_min: float, x_max: float) -> None:
    if args.screenshot_dir is None:
        return

    args.screenshot_dir.mkdir(parents=True, exist_ok=True)
    scene = bpy.context.scene
    for engine in ("BLENDER_EEVEE_NEXT", "BLENDER_EEVEE", "BLENDER_WORKBENCH"):
        try:
            scene.render.engine = engine
            break
        except TypeError:
            continue
    scene.render.resolution_x = args.screenshot_size
    scene.render.resolution_y = round(args.screenshot_size * 0.56)
    if hasattr(scene, "eevee"):
        scene.eevee.taa_render_samples = 16

    center_x = (x_min + x_max) * 0.5
    row_width = max(x_max - x_min, 1.0)
    camera = scene.camera
    shots = [
        ("comparison_front.png", Vector((center_x, max(8.0, row_width * 0.95), args.height * 1.35))),
        ("comparison_3q_left.png", Vector((center_x - row_width * 0.25, max(8.0, row_width * 0.88), args.height * 1.45))),
        ("comparison_3q_right.png", Vector((center_x + row_width * 0.25, max(8.0, row_width * 0.88), args.height * 1.45))),
    ]

    for name, location in shots:
        camera.location = location
        look_at(camera, Vector((center_x, 0.0, args.height * 0.55)))
        scene.render.filepath = str(args.screenshot_dir / name)
        if bpy.app.background:
            bpy.ops.render.render(write_still=True)
        else:
            try:
                bpy.ops.render.opengl(write_still=True, view_context=False)
            except RuntimeError:
                bpy.ops.render.render(write_still=True)


def main() -> int:
    args = parse_args()
    if len(args.inputs) != len(args.labels):
        raise SystemExit("--inputs and --labels must have the same count")

    bpy.ops.wm.read_factory_settings(use_empty=False)
    delete_default_cube()

    count = len(args.inputs)
    start_x = ((count - 1) * args.spacing) * 0.5
    metadata = []
    for index, (path, label) in enumerate(zip(args.inputs, args.labels)):
        if not path.exists():
            raise FileNotFoundError(path)
        metadata.append(import_variant(path, label, start_x - index * args.spacing, args.height))

    x_min = -abs(start_x) - args.spacing * 0.5
    x_max = abs(start_x) + args.spacing * 0.5
    setup_camera(x_min, x_max, args.height)

    args.output.parent.mkdir(parents=True, exist_ok=True)
    bpy.ops.wm.save_as_mainfile(filepath=str(args.output))
    save_screenshots(args, x_min, x_max)

    print("Saved", args.output)
    for row in metadata:
        print(row)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
