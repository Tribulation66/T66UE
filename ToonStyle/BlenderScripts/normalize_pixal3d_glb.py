#!/usr/bin/env python3
"""Normalize a Pixal3D GLB without retopology, rebaking, or material edits."""

from __future__ import annotations

import argparse
import json
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

    parser = argparse.ArgumentParser(description="Normalize a Pixal3D GLB spatial transform.")
    parser.add_argument("--input", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--target-height", default=180.0, type=float)
    parser.add_argument("--report", type=Path)
    return parser.parse_args(argv)


def clear_scene() -> None:
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete()


def mesh_objects() -> list[bpy.types.Object]:
    return [obj for obj in bpy.context.scene.objects if obj.type == "MESH"]


def world_bounds(obj: bpy.types.Object) -> tuple[Vector, Vector]:
    corners = [obj.matrix_world @ Vector(corner) for corner in obj.bound_box]
    mins = Vector((min(c.x for c in corners), min(c.y for c in corners), min(c.z for c in corners)))
    maxs = Vector((max(c.x for c in corners), max(c.y for c in corners), max(c.z for c in corners)))
    return mins, maxs


def dimensions(obj: bpy.types.Object) -> Vector:
    mins, maxs = world_bounds(obj)
    return maxs - mins


def join_meshes(objects: list[bpy.types.Object]) -> bpy.types.Object:
    bpy.ops.object.select_all(action="DESELECT")
    for obj in objects:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = objects[0]
    if len(objects) > 1:
        bpy.ops.object.join()
    return bpy.context.view_layer.objects.active


def maybe_correct_y_up(obj: bpy.types.Object) -> bool:
    dims = dimensions(obj)
    if dims.z <= 0.0:
        return False

    # Blender's glTF importer usually handles axes. This only corrects obvious
    # sideways imports where Y is the dominant vertical-looking extent.
    if dims.y > dims.z * 1.5 and dims.y > dims.x * 1.15:
        obj.rotation_euler.rotate_axis("X", math.radians(90.0))
        bpy.context.view_layer.objects.active = obj
        obj.select_set(True)
        bpy.ops.object.transform_apply(location=False, rotation=True, scale=False)
        return True
    return False


def apply_scale_and_floor_origin(obj: bpy.types.Object, target_height: float) -> dict[str, object]:
    dims_before = dimensions(obj)
    if dims_before.z <= 0.0:
        raise RuntimeError("Imported mesh has zero Z height; cannot normalize scale.")

    scale = target_height / dims_before.z
    obj.scale = (obj.scale.x * scale, obj.scale.y * scale, obj.scale.z * scale)
    bpy.context.view_layer.objects.active = obj
    obj.select_set(True)
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)

    mins, maxs = world_bounds(obj)
    center_x = (mins.x + maxs.x) * 0.5
    center_y = (mins.y + maxs.y) * 0.5
    obj.location -= Vector((center_x, center_y, mins.z))
    bpy.ops.object.transform_apply(location=True, rotation=False, scale=False)

    final_mins, final_maxs = world_bounds(obj)
    return {
        "scale_factor": scale,
        "bounds_min": [final_mins.x, final_mins.y, final_mins.z],
        "bounds_max": [final_maxs.x, final_maxs.y, final_maxs.z],
        "height": final_maxs.z - final_mins.z,
    }


def export_glb(obj: bpy.types.Object, output_path: Path) -> None:
    output_path.parent.mkdir(parents=True, exist_ok=True)
    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj

    kwargs = {
        "filepath": str(output_path),
        "export_format": "GLB",
        "use_selection": True,
    }
    bpy.ops.export_scene.gltf(**kwargs)


def main() -> int:
    args = parse_args()
    clear_scene()

    bpy.ops.import_scene.gltf(filepath=str(args.input))
    imported_meshes = mesh_objects()
    if not imported_meshes:
        raise RuntimeError(f"No mesh objects imported from {args.input}")

    mesh_count_before = len(imported_meshes)
    obj = join_meshes(imported_meshes)
    corrected_y_up = maybe_correct_y_up(obj)
    report = apply_scale_and_floor_origin(obj, args.target_height)

    material_count = len(obj.data.materials) if obj and obj.data else 0
    export_glb(obj, args.output)

    report.update(
        {
            "input": str(args.input),
            "output": str(args.output),
            "mesh_count_before_join": mesh_count_before,
            "mesh_count_after_join": 1,
            "material_count": material_count,
            "corrected_y_up": corrected_y_up,
            "target_height": args.target_height,
        }
    )

    if args.report:
        args.report.parent.mkdir(parents=True, exist_ok=True)
        args.report.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    print(json.dumps(report, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
