import argparse
import json
import math
import os
from pathlib import Path

import bpy
from mathutils import Vector


PREFIX = "MikeNeckGraft_"


def parse_args():
    argv = os.sys.argv[os.sys.argv.index("--") + 1 :] if "--" in os.sys.argv else []
    parser = argparse.ArgumentParser()
    parser.add_argument("--source-glb", required=True)
    parser.add_argument("--output", required=True)
    parser.add_argument("--report", required=True)
    parser.add_argument("--render-dir", required=True)
    parser.add_argument("--blend", required=True)
    parser.add_argument("--stem", required=True)
    parser.add_argument("--variant", default="C20")
    parser.add_argument("--resolution", type=int, default=1400)
    return parser.parse_args(argv)


def scene_engine():
    engines = {item.identifier for item in bpy.types.RenderSettings.bl_rna.properties["engine"].enum_items}
    return "BLENDER_EEVEE_NEXT" if "BLENDER_EEVEE_NEXT" in engines else "BLENDER_EEVEE"


def ensure_render_setup():
    scene = bpy.context.scene
    scene.render.engine = scene_engine()
    scene.render.image_settings.file_format = "PNG"
    scene.render.film_transparent = False
    scene.view_settings.view_transform = "Standard"
    if not scene.world:
        scene.world = bpy.data.worlds.new("NeckGraftWorld")
    scene.world.use_nodes = True
    bg = scene.world.node_tree.nodes["Background"]
    bg.inputs[0].default_value = (0.18, 0.18, 0.18, 1.0)
    bg.inputs[1].default_value = 0.8
    if not bpy.data.objects.get("Assembly_Key_Light"):
        bpy.ops.object.light_add(type="AREA", location=(0.0, -4.2, 4.5))
        key = bpy.context.object
        key.name = "Assembly_Key_Light"
        key.data.energy = 650
        key.data.size = 5.0
    if not bpy.data.objects.get("Assembly_Fill_Light"):
        bpy.ops.object.light_add(type="POINT", location=(-3.0, 2.5, 2.6))
        fill = bpy.context.object
        fill.name = "Assembly_Fill_Light"
        fill.data.energy = 110


def clean_previous():
    for obj in list(bpy.context.scene.objects):
        if obj.name.startswith("QA_") or obj.name.startswith(PREFIX):
            bpy.data.objects.remove(obj, do_unlink=True)


def import_source_glb(path):
    before = set(bpy.data.objects)
    bpy.ops.import_scene.gltf(filepath=str(path))
    imported = [obj for obj in bpy.data.objects if obj not in before]
    meshes = [obj for obj in imported if obj.type == "MESH"]
    if not meshes:
        raise RuntimeError(f"No meshes imported from {path}")
    return imported, meshes


def mesh_bounds(objects):
    min_v = Vector((math.inf, math.inf, math.inf))
    max_v = Vector((-math.inf, -math.inf, -math.inf))
    for obj in objects:
        if obj.type != "MESH":
            continue
        for corner in obj.bound_box:
            w = obj.matrix_world @ Vector(corner)
            min_v.x = min(min_v.x, w.x)
            min_v.y = min(min_v.y, w.y)
            min_v.z = min(min_v.z, w.z)
            max_v.x = max(max_v.x, w.x)
            max_v.y = max(max_v.y, w.y)
            max_v.z = max(max_v.z, w.z)
    return min_v, max_v, max_v - min_v


def find_source_head(meshes):
    return max(meshes, key=lambda obj: (obj.matrix_world @ Vector(obj.bound_box[7])).z)


def thresholds(variant):
    if variant == "C21":
        return {"z_min": 0.338, "z_max": 0.488, "x_max": 0.090, "y_min": -0.130, "y_max": 0.095}
    if variant == "C22":
        return {"z_min": 0.360, "z_max": 0.508, "x_max": 0.078, "y_min": -0.115, "y_max": 0.080}
    if variant == "C23":
        return {"z_min": 0.342, "z_max": 0.420, "x_max": 0.082, "y_min": -0.120, "y_max": 0.090}
    if variant == "C24":
        return {"z_min": 0.342, "z_max": 0.445, "x_max": 0.082, "y_min": -0.110, "y_max": 0.088}
    if variant == "C25":
        return {
            "z_min": 0.336,
            "z_max": 0.472,
            "x_max": 0.082,
            "y_min": -0.006,
            "y_max": 0.102,
            "front_reject_z": 0.380,
            "front_reject_y": 0.004,
        }
    if variant == "C26":
        return {
            "z_min": 0.330,
            "z_max": 0.432,
            "x_max": 0.086,
            "y_min": -0.010,
            "y_max": 0.104,
            "front_reject_z": 0.370,
            "front_reject_y": 0.006,
        }
    if variant == "C27":
        return {
            "z_min": 0.350,
            "z_max": 0.500,
            "x_max": 0.072,
            "y_min": 0.006,
            "y_max": 0.112,
            "front_reject_z": 0.360,
            "front_reject_y": 0.012,
        }
    return {"z_min": 0.350, "z_max": 0.468, "x_max": 0.084, "y_min": -0.125, "y_max": 0.090}


def create_neck_graft(source_head, variant):
    limits = thresholds(variant)
    uv_layer = source_head.data.uv_layers.active
    verts = []
    faces = []
    uvs = []
    mat_indices = []
    selected = 0

    for poly in source_head.data.polygons:
        wc = source_head.matrix_world @ poly.center
        if not (limits["z_min"] <= wc.z <= limits["z_max"]):
            continue
        if abs(wc.x) > limits["x_max"] or wc.y < limits["y_min"] or wc.y > limits["y_max"]:
            continue
        # Leave the A03 jaw/face untouched. This rejects forward jaw planes
        # while keeping the generated neck volume and side/back neck texture.
        if args_variant_needs_strict_front_reject(variant, wc):
            continue
        root = len(verts)
        loop_indices = list(poly.loop_indices)
        for li in loop_indices:
            v = source_head.data.vertices[source_head.data.loops[li].vertex_index]
            verts.append(tuple(source_head.matrix_world @ v.co))
            if uv_layer:
                uv = uv_layer.data[li].uv
                uvs.append((uv.x, uv.y))
            else:
                uvs.append((0.0, 0.0))
        faces.append(tuple(range(root, root + len(loop_indices))))
        mat_indices.append(poly.material_index)
        selected += 1

    if not faces:
        raise RuntimeError("No source neck faces matched graft thresholds")

    mesh = bpy.data.meshes.new(f"{PREFIX}Mesh_{variant}")
    mesh.from_pydata(verts, [], faces)
    mesh.update()
    graft = bpy.data.objects.new(f"{PREFIX}{variant}", mesh)
    bpy.context.scene.collection.objects.link(graft)
    for slot in source_head.material_slots:
        if slot.material:
            graft.data.materials.append(slot.material)
    if uv_layer:
        new_uv = graft.data.uv_layers.new(name="UVMap")
        cursor = 0
        for poly in graft.data.polygons:
            for li in poly.loop_indices:
                new_uv.data[li].uv = uvs[cursor]
                cursor += 1
    for i, poly in enumerate(graft.data.polygons):
        poly.material_index = mat_indices[i] if mat_indices else 0
        poly.use_smooth = True
    return graft, selected, limits


def args_variant_needs_strict_front_reject(variant, wc):
    limits = thresholds(variant)
    if "front_reject_z" in limits and "front_reject_y" in limits:
        return wc.z > limits["front_reject_z"] and wc.y < limits["front_reject_y"]
    if variant == "C24":
        return wc.z > 0.398 and wc.y < -0.018
    if variant == "C23":
        return wc.z > 0.388 and wc.y < -0.010
    return wc.z > 0.445 and wc.y < -0.055


def add_camera(center, size, yaw_deg, pitch_deg=4.0, ortho_scale=None):
    cam_data = bpy.data.cameras.new(name=f"QA_Camera_{yaw_deg:g}")
    cam_data.type = "ORTHO"
    cam = bpy.data.objects.new(cam_data.name, cam_data)
    bpy.context.scene.collection.objects.link(cam)
    yaw = math.radians(yaw_deg)
    pitch = math.radians(pitch_deg)
    distance = max(size.x, size.y, size.z) * 3.2
    cam.location = (
        center.x + math.sin(yaw) * math.cos(pitch) * distance,
        center.y - math.cos(yaw) * math.cos(pitch) * distance,
        center.z + math.sin(pitch) * distance,
    )
    direction = center - cam.location
    cam.rotation_euler = direction.to_track_quat("-Z", "Y").to_euler()
    cam.data.ortho_scale = ortho_scale or max(size.x, size.z) * 1.42
    return cam


def render_views(stem, render_dir, resolution):
    render_dir.mkdir(parents=True, exist_ok=True)
    meshes = [obj for obj in bpy.context.scene.objects if obj.type == "MESH"]
    min_v, max_v, size = mesh_bounds(meshes)
    center = (min_v + max_v) * 0.5
    scene = bpy.context.scene
    scene.render.resolution_x = resolution
    scene.render.resolution_y = resolution
    outputs = {}
    for label, yaw in {"front": 0.0, "right": 90.0, "back": 180.0, "left": 270.0}.items():
        cam = add_camera(center, size, yaw)
        scene.camera = cam
        out = render_dir / f"{stem}_{label}.png"
        scene.render.filepath = str(out)
        bpy.ops.render.render(write_still=True)
        outputs[label] = str(out)
    neck_center = Vector((0.0, -0.02, 0.462))
    neck_size = Vector((0.34, 0.24, 0.22))
    cam = add_camera(neck_center, neck_size, 0.0, pitch_deg=2.0, ortho_scale=0.46)
    scene.camera = cam
    out = render_dir / f"{stem}_neck_closeup.png"
    scene.render.filepath = str(out)
    bpy.ops.render.render(write_still=True)
    outputs["neck_closeup"] = str(out)
    return outputs


def export_glb(path):
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    for obj in bpy.context.scene.objects:
        obj.select_set(obj.type in {"MESH", "ARMATURE", "EMPTY"})
    bpy.ops.export_scene.gltf(
        filepath=str(path),
        export_format="GLB",
        use_selection=True,
        export_apply=True,
        export_materials="EXPORT",
    )


def total_triangles(objects):
    total = 0
    for obj in objects:
        if obj and obj.type == "MESH":
            obj.data.calc_loop_triangles()
            total += len(obj.data.loop_triangles)
    return total


def main():
    args = parse_args()
    clean_previous()
    ensure_render_setup()

    imported, meshes = import_source_glb(Path(args.source_glb))
    source_head = find_source_head(meshes)
    graft, selected_faces, limits = create_neck_graft(source_head, args.variant)
    for obj in imported:
        bpy.data.objects.remove(obj, do_unlink=True)

    output = Path(args.output)
    report = Path(args.report)
    blend = Path(args.blend)
    render_dir = Path(args.render_dir)

    export_glb(output)
    renders = render_views(args.stem, render_dir, args.resolution)
    blend.parent.mkdir(parents=True, exist_ok=True)
    bpy.ops.wm.save_as_mainfile(filepath=str(blend))
    meshes = [obj for obj in bpy.context.scene.objects if obj.type == "MESH"]
    _min, _max, size = mesh_bounds(meshes)
    payload = {
        "stem": args.stem,
        "variant": args.variant,
        "source_glb": args.source_glb,
        "output_glb": str(output),
        "blend": str(blend),
        "renders": renders,
        "graft_object": graft.name,
        "selected_source_faces": selected_faces,
        "thresholds": limits,
        "triangles": {"graft": total_triangles([graft]), "scene_total": total_triangles(meshes)},
        "bounds": {"combined_size": [size.x, size.y, size.z]},
        "notes": [
            "A03_B02 remains the face/head base.",
            "Only lower neck faces are grafted from the A04 thick-neck assembly.",
            "This directly tests the cut-thick-neck-then-use-the-neck workflow.",
        ],
    }
    report.parent.mkdir(parents=True, exist_ok=True)
    report.write_text(json.dumps(payload, indent=2), encoding="utf-8")
    print(f"WROTE {report}")


if __name__ == "__main__":
    main()
