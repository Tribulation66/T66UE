import argparse
import glob
import json
import math
import os

import bpy
from mathutils import Matrix, Vector


TARGET_HEIGHT_M = 2.0


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


def _visible_mesh_objects():
    return [
        obj
        for obj in bpy.context.scene.objects
        if obj.type == "MESH" and not obj.name.lower().startswith("icosphere")
    ]


def _delete_helper_meshes():
    for obj in list(bpy.context.scene.objects):
        if obj.type == "MESH" and obj.name.lower().startswith("icosphere"):
            bpy.data.objects.remove(obj, do_unlink=True)


def _transform_all(matrix):
    for obj in list(bpy.context.scene.objects):
        obj.matrix_world = matrix @ obj.matrix_world
    bpy.context.view_layer.update()


def _apply_all_transforms():
    bpy.ops.object.select_all(action="DESELECT")
    for obj in bpy.context.scene.objects:
        obj.select_set(True)
    if bpy.context.scene.objects:
        bpy.context.view_layer.objects.active = bpy.context.scene.objects[0]
        bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)


def _export_fbx(output_fbx, global_scale, apply_scale_options):
    os.makedirs(os.path.dirname(output_fbx), exist_ok=True)
    bpy.ops.export_scene.fbx(
        filepath=output_fbx,
        use_selection=False,
        object_types={"ARMATURE", "MESH"},
        add_leaf_bones=False,
        bake_anim=False,
        global_scale=global_scale,
        apply_scale_options=apply_scale_options,
        axis_forward="-Y",
        axis_up="Z",
        mesh_smooth_type="FACE",
        use_armature_deform_only=True,
    )


def _export_glb(output_glb):
    os.makedirs(os.path.dirname(output_glb), exist_ok=True)
    bpy.ops.export_scene.gltf(
        filepath=output_glb,
        export_format="GLB",
        export_skins=True,
        export_animations=False,
        export_yup=False,
    )


def _canonicalize_one(input_glb, output_dir, apply_transforms=True, fbx_global_scale=1.0, fbx_apply_scale_options="FBX_SCALE_NONE"):
    _clear_scene()
    bpy.ops.import_scene.gltf(filepath=input_glb)
    _delete_helper_meshes()

    mesh_objects = _visible_mesh_objects()
    before = _world_bounds(mesh_objects)
    if not before:
        raise RuntimeError(f"No visible mesh bounds found in {input_glb}")

    rotation_mode = "none"
    before_size = before["size"]
    if before_size[2] < 1.5 and before_size[1] >= 1.5:
        _transform_all(Matrix.Rotation(math.radians(90.0), 4, "X"))
        rotation_mode = "y_to_z"
    elif before_size[2] < 1.5 and before_size[0] >= 1.5:
        _transform_all(Matrix.Rotation(math.radians(-90.0), 4, "Y"))
        rotation_mode = "x_to_z"

    rotated = _world_bounds(_visible_mesh_objects())
    height = rotated["size"][2]
    if height <= 0.0001:
        raise RuntimeError(f"Invalid rotated height for {input_glb}: {height}")

    scale = TARGET_HEIGHT_M / height
    min_z = rotated["min"][2]
    _transform_all(Matrix.Translation(Vector((0.0, 0.0, -(min_z * scale)))) @ Matrix.Scale(scale, 4))

    if apply_transforms:
        _apply_all_transforms()

    after = _world_bounds(_visible_mesh_objects())
    base_name = os.path.splitext(os.path.basename(input_glb))[0]
    output_fbx = os.path.join(output_dir, f"{base_name}_Canonical2m.fbx")
    output_glb = os.path.join(output_dir, f"{base_name}_Canonical2m.glb")
    _export_fbx(output_fbx, fbx_global_scale, fbx_apply_scale_options)
    _export_glb(output_glb)

    return {
        "input_glb": input_glb,
        "output_fbx": output_fbx,
        "output_glb": output_glb,
        "rotation_mode": rotation_mode,
        "uniform_scale": scale,
        "fbx_global_scale": fbx_global_scale,
        "fbx_apply_scale_options": fbx_apply_scale_options,
        "before_bounds_m": before,
        "after_bounds_m": after,
        "objects": [
            {
                "name": obj.name,
                "type": obj.type,
            }
            for obj in bpy.context.scene.objects
        ],
    }


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", default="")
    parser.add_argument("--input-dir", default="")
    parser.add_argument("--output-dir", required=True)
    parser.add_argument("--report", required=True)
    parser.add_argument("--no-apply-transforms", action="store_true")
    parser.add_argument("--fbx-global-scale", type=float, default=1.0)
    parser.add_argument("--fbx-apply-scale-options", default="FBX_SCALE_NONE")
    args = parser.parse_args()

    inputs = []
    if args.input:
        inputs.append(args.input)
    if args.input_dir:
        inputs.extend(sorted(glob.glob(os.path.join(args.input_dir, "*.glb"))))
    inputs = list(dict.fromkeys(inputs))
    if not inputs:
        raise RuntimeError("--input or --input-dir is required")

    reports = [
        _canonicalize_one(
            path,
            args.output_dir,
            apply_transforms=not args.no_apply_transforms,
            fbx_global_scale=args.fbx_global_scale,
            fbx_apply_scale_options=args.fbx_apply_scale_options)
        for path in inputs
    ]

    os.makedirs(os.path.dirname(args.report), exist_ok=True)
    with open(args.report, "w", encoding="utf-8") as handle:
        json.dump({"count": len(reports), "reports": reports}, handle, indent=2)


if __name__ == "__main__":
    if "--" in os.sys.argv:
        os.sys.argv = [os.sys.argv[0]] + os.sys.argv[os.sys.argv.index("--") + 1:]
    main()
