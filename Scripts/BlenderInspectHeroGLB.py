import json
import os
import sys

import bpy
from mathutils import Vector


def _arg_value(name, default=None):
    if "--" not in sys.argv:
        return default
    args = sys.argv[sys.argv.index("--") + 1:]
    for index, arg in enumerate(args):
        if arg == name and index + 1 < len(args):
            return args[index + 1]
        if arg.startswith(name + "="):
            return arg.split("=", 1)[1]
    return default


def _clear_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete()


def _world_bounds(objects):
    points = []
    for obj in objects:
        if obj.type != "MESH":
            continue
        for corner in obj.bound_box:
            points.append(obj.matrix_world @ Vector(corner))
    if not points:
        return None
    mins = Vector((min(p.x for p in points), min(p.y for p in points), min(p.z for p in points)))
    maxs = Vector((max(p.x for p in points), max(p.y for p in points), max(p.z for p in points)))
    return {
        "min": [mins.x, mins.y, mins.z],
        "max": [maxs.x, maxs.y, maxs.z],
        "size": [maxs.x - mins.x, maxs.y - mins.y, maxs.z - mins.z],
        "center": [(mins.x + maxs.x) * 0.5, (mins.y + maxs.y) * 0.5, (mins.z + maxs.z) * 0.5],
    }


def _object_bounds(obj):
    if obj.type != "MESH":
        return None

    points = [obj.matrix_world @ Vector(corner) for corner in obj.bound_box]
    if not points:
        return None

    mins = Vector((min(p.x for p in points), min(p.y for p in points), min(p.z for p in points)))
    maxs = Vector((max(p.x for p in points), max(p.y for p in points), max(p.z for p in points)))
    return {
        "min": [mins.x, mins.y, mins.z],
        "max": [maxs.x, maxs.y, maxs.z],
        "size": [maxs.x - mins.x, maxs.y - mins.y, maxs.z - mins.z],
        "center": [(mins.x + maxs.x) * 0.5, (mins.y + maxs.y) * 0.5, (mins.z + maxs.z) * 0.5],
    }


def main():
    input_path = _arg_value("--input")
    output_report = _arg_value("--report")
    if not input_path:
        raise RuntimeError("--input is required")

    _clear_scene()
    if input_path.lower().endswith(".fbx"):
        bpy.ops.import_scene.fbx(filepath=input_path)
    else:
        bpy.ops.import_scene.gltf(filepath=input_path)

    objects = list(bpy.context.scene.objects)
    report = {
        "input": input_path,
        "objects": [
            {
                "name": obj.name,
                "type": obj.type,
                "location": list(obj.location),
                "rotation_euler": list(obj.rotation_euler),
                "scale": list(obj.scale),
                "bounds_m": _object_bounds(obj),
            }
            for obj in objects
        ],
        "bones": [
            {
                "armature": obj.name,
                "name": bone.name,
                "head_local": [bone.head_local.x, bone.head_local.y, bone.head_local.z],
                "tail_local": [bone.tail_local.x, bone.tail_local.y, bone.tail_local.z],
            }
            for obj in objects
            if obj.type == "ARMATURE"
            for bone in obj.data.bones
        ],
        "bounds_m": _world_bounds(objects),
    }

    text = json.dumps(report, indent=2)
    if output_report:
        os.makedirs(os.path.dirname(output_report), exist_ok=True)
        with open(output_report, "w", encoding="utf-8") as handle:
            handle.write(text)
    print(text)


if __name__ == "__main__":
    main()
