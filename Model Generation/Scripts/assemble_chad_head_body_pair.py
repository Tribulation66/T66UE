import argparse
import json
import math
import os
from pathlib import Path

import bpy
from mathutils import Matrix, Vector


DEFAULT_HEAD_HEIGHT_RATIO = 0.245
DEFAULT_NECK_OVERLAP_RATIO = 0.045
DEFAULT_HEAD_Y_OFFSET_RATIO = -0.006


def parse_args():
    argv = os.sys.argv[os.sys.argv.index("--") + 1 :] if "--" in os.sys.argv else []
    parser = argparse.ArgumentParser()
    parser.add_argument("--body", required=True)
    parser.add_argument("--head", required=True)
    parser.add_argument("--output", required=True)
    parser.add_argument("--report", required=True)
    parser.add_argument("--render-dir", required=True)
    parser.add_argument("--blend")
    parser.add_argument("--stem", required=True)
    parser.add_argument("--resolution", type=int, default=1400)
    parser.add_argument("--head-height-ratio", type=float, default=DEFAULT_HEAD_HEIGHT_RATIO)
    parser.add_argument("--neck-overlap-ratio", type=float, default=DEFAULT_NECK_OVERLAP_RATIO)
    parser.add_argument("--head-y-offset-ratio", type=float, default=DEFAULT_HEAD_Y_OFFSET_RATIO)
    parser.add_argument("--head-x-offset-ratio", type=float, default=0.0)
    return parser.parse_args(argv)


def scene_engine():
    engines = {item.identifier for item in bpy.types.RenderSettings.bl_rna.properties["engine"].enum_items}
    return "BLENDER_EEVEE_NEXT" if "BLENDER_EEVEE_NEXT" in engines else "BLENDER_EEVEE"


def reset_scene():
    bpy.ops.wm.read_factory_settings(use_empty=True)
    scene = bpy.context.scene
    scene.render.engine = scene_engine()
    scene.render.image_settings.file_format = "PNG"
    scene.render.film_transparent = False
    scene.view_settings.view_transform = "Standard"
    scene.world = bpy.data.worlds.new("ChadAssemblyWorld")
    scene.world.use_nodes = True
    bg = scene.world.node_tree.nodes["Background"]
    bg.inputs[0].default_value = (0.18, 0.18, 0.18, 1.0)
    bg.inputs[1].default_value = 0.8
    return scene


def import_glb(path, prefix):
    before = set(bpy.context.scene.objects)
    bpy.ops.import_scene.gltf(filepath=str(path))
    imported = [obj for obj in bpy.context.scene.objects if obj not in before]
    for obj in imported:
        obj.name = f"{prefix}_{obj.name}"
        if getattr(obj, "data", None):
            obj.data.name = f"{prefix}_{obj.data.name}"
    meshes = [obj for obj in imported if obj.type == "MESH"]
    if not meshes:
        raise RuntimeError(f"No mesh objects imported from {path}")
    return imported, meshes


def object_bounds(objects):
    min_v = Vector((math.inf, math.inf, math.inf))
    max_v = Vector((-math.inf, -math.inf, -math.inf))
    for obj in objects:
        if obj.type != "MESH":
            continue
        for corner in obj.bound_box:
            world = obj.matrix_world @ Vector(corner)
            min_v.x = min(min_v.x, world.x)
            min_v.y = min(min_v.y, world.y)
            min_v.z = min(min_v.z, world.z)
            max_v.x = max(max_v.x, world.x)
            max_v.y = max(max_v.y, world.y)
            max_v.z = max(max_v.z, world.z)
    if min_v.x == math.inf:
        raise RuntimeError("No mesh bounds available")
    return min_v, max_v, max_v - min_v


def imported_roots(imported):
    imported_set = set(imported)
    return [obj for obj in imported if obj.parent not in imported_set]


def apply_group_transform(imported, matrix):
    for root in imported_roots(imported):
        root.matrix_world = matrix @ root.matrix_world
    bpy.context.view_layer.update()


def total_triangles(meshes):
    total = 0
    for obj in meshes:
        obj.data.calc_loop_triangles()
        total += len(obj.data.loop_triangles)
    return total


def add_lights():
    bpy.ops.object.light_add(type="AREA", location=(0.0, -4.2, 4.5))
    key = bpy.context.object
    key.name = "Assembly_Key_Light"
    key.data.energy = 650
    key.data.size = 5.0

    bpy.ops.object.light_add(type="POINT", location=(-3.0, 2.5, 2.6))
    fill = bpy.context.object
    fill.name = "Assembly_Fill_Light"
    fill.data.energy = 110


def add_camera(center, size, yaw_deg, pitch_deg=4.0, ortho_scale=None):
    cam_data = bpy.data.cameras.new(name=f"QA_Camera_{yaw_deg:g}")
    cam_data.type = "ORTHO"
    cam = bpy.data.objects.new(cam_data.name, cam_data)
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

    target = bpy.data.objects.new(f"QA_Target_{yaw_deg:g}", None)
    target.location = center
    bpy.context.scene.collection.objects.link(target)

    track = cam.constraints.new(type="TRACK_TO")
    track.target = target
    track.track_axis = "TRACK_NEGATIVE_Z"
    track.up_axis = "UP_Y"
    cam_data.ortho_scale = ortho_scale if ortho_scale else max(size.x, size.z) * 1.28
    return cam


def render_views(stem, render_dir, resolution, neck_center=None, neck_size=None):
    render_dir.mkdir(parents=True, exist_ok=True)
    meshes = [obj for obj in bpy.context.scene.objects if obj.type == "MESH"]
    min_v, max_v, size = object_bounds(meshes)
    center = (min_v + max_v) * 0.5

    scene = bpy.context.scene
    scene.render.resolution_x = resolution
    scene.render.resolution_y = resolution

    outputs = {}
    for label, yaw in {"front": 0.0, "right": 90.0, "back": 180.0, "left": -90.0}.items():
        add_camera(center, size, yaw)
        out = render_dir / f"{stem}_{label}.png"
        scene.render.filepath = str(out)
        bpy.ops.render.render(write_still=True)
        outputs[label] = str(out)

    if neck_center and neck_size:
        add_camera(neck_center, neck_size, 0.0, pitch_deg=2.0, ortho_scale=max(neck_size.x, neck_size.z) * 2.2)
        out = render_dir / f"{stem}_neck_closeup.png"
        scene.render.filepath = str(out)
        bpy.ops.render.render(write_still=True)
        outputs["neck_closeup"] = str(out)

    return outputs


def export_glb(path):
    path.parent.mkdir(parents=True, exist_ok=True)
    bpy.ops.object.select_all(action="DESELECT")
    for obj in bpy.context.scene.objects:
        if obj.type in {"MESH", "ARMATURE", "EMPTY"} and not obj.name.startswith("QA_"):
            obj.select_set(True)
    bpy.ops.export_scene.gltf(
        filepath=str(path),
        export_format="GLB",
        use_selection=True,
        export_apply=True,
        export_materials="EXPORT",
    )


def main():
    args = parse_args()
    body_path = Path(args.body)
    head_path = Path(args.head)
    output_path = Path(args.output)
    report_path = Path(args.report)
    render_dir = Path(args.render_dir)

    reset_scene()
    body_imported, body_meshes = import_glb(body_path, "Body")
    head_imported, head_meshes = import_glb(head_path, "Head")

    body_min, body_max, body_size = object_bounds(body_meshes)
    head_min, head_max, head_size = object_bounds(head_meshes)
    body_center = (body_min + body_max) * 0.5
    head_anchor = Vector(((head_min.x + head_max.x) * 0.5, (head_min.y + head_max.y) * 0.5, head_min.z))
    head_scale = args.head_height_ratio * body_size.z / max(head_size.z, 0.001)
    target = Vector(
        (
            body_center.x + args.head_x_offset_ratio * body_size.z,
            body_center.y + args.head_y_offset_ratio * body_size.z,
            body_max.z - args.neck_overlap_ratio * body_size.z,
        )
    )
    apply_group_transform(head_imported, Matrix.Translation(target - head_anchor * head_scale) @ Matrix.Scale(head_scale, 4))

    add_lights()
    export_glb(output_path)

    combined_meshes = [obj for obj in bpy.context.scene.objects if obj.type == "MESH"]
    combined_min, combined_max, combined_size = object_bounds(combined_meshes)
    neck_center = Vector((body_center.x, body_center.y, body_max.z - 0.04 * body_size.z))
    neck_size = Vector((body_size.x * 0.45, body_size.y * 0.45, body_size.z * 0.22))
    renders = render_views(args.stem, render_dir, args.resolution, neck_center=neck_center, neck_size=neck_size)

    blend_path = None
    if args.blend:
        blend_path = Path(args.blend)
        blend_path.parent.mkdir(parents=True, exist_ok=True)
        bpy.ops.wm.save_as_mainfile(filepath=str(blend_path))

    payload = {
        "stem": args.stem,
        "body_glb": str(body_path),
        "head_glb": str(head_path),
        "output_glb": str(output_path),
        "blend": str(blend_path) if blend_path else None,
        "renders": renders,
        "placement": {
            "head_height_ratio": args.head_height_ratio,
            "neck_overlap_ratio": args.neck_overlap_ratio,
            "head_y_offset_ratio": args.head_y_offset_ratio,
            "head_x_offset_ratio": args.head_x_offset_ratio,
            "head_scale": head_scale,
        },
        "triangles": {
            "body": total_triangles(body_meshes),
            "head": total_triangles(head_meshes),
            "combined": total_triangles(combined_meshes),
        },
        "bounds": {
            "body_size": [body_size.x, body_size.y, body_size.z],
            "head_size_before": [head_size.x, head_size.y, head_size.z],
            "combined_size": [combined_size.x, combined_size.y, combined_size.z],
        },
    }
    report_path.parent.mkdir(parents=True, exist_ok=True)
    report_path.write_text(json.dumps(payload, indent=2), encoding="utf-8")
    print(f"WROTE {report_path}")


if __name__ == "__main__":
    main()
