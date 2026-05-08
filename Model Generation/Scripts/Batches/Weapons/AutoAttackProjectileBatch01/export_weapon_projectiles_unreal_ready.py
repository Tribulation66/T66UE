import argparse
import json
import math
import os
from pathlib import Path

import bpy
from mathutils import Vector


REPO_ROOT = Path(r"C:\UE\T66")
RUN_ROOT = REPO_ROOT / "Model Generation" / "Runs" / "Weapons" / "AutoAttackProjectileBatch01"
MANIFEST_PATH = RUN_ROOT / "batch_manifest.json"
OUTPUT_ROOT = REPO_ROOT / "SourceAssets" / "Import" / "Weapons" / "Projectiles" / "UnrealReady"
TEXTURE_ROOT = REPO_ROOT / "SourceAssets" / "Import" / "Weapons" / "Projectiles" / "Textures"
NOTES_ROOT = RUN_ROOT / "Notes" / "unreal_ready"

TARGET_LONG_AXIS_M = 2.0


def parse_args():
    argv = []
    if "--" in os.sys.argv:
        argv = os.sys.argv[os.sys.argv.index("--") + 1 :]

    parser = argparse.ArgumentParser()
    parser.add_argument("--run-root", default=str(RUN_ROOT))
    parser.add_argument("--output-root", default=str(OUTPUT_ROOT))
    parser.add_argument("--texture-root", default=str(TEXTURE_ROOT))
    parser.add_argument("--notes-root", default=str(NOTES_ROOT))
    parser.add_argument("--target-long-axis-m", type=float, default=TARGET_LONG_AXIS_M)
    return parser.parse_args(argv)


def reset_scene():
    bpy.ops.wm.read_factory_settings(use_empty=True)


def import_glb(path):
    before = set(bpy.data.objects)
    bpy.ops.import_scene.gltf(filepath=str(path))
    imported = [obj for obj in bpy.data.objects if obj not in before]
    meshes = [obj for obj in imported if obj.type == "MESH"]
    if not meshes:
        raise RuntimeError(f"No mesh objects imported from {path}")
    return meshes


def detach_keep_world(meshes):
    for obj in meshes:
        world_matrix = obj.matrix_world.copy()
        obj.parent = None
        obj.matrix_world = world_matrix


def apply_object_transforms(meshes):
    bpy.ops.object.select_all(action="DESELECT")
    for obj in meshes:
        obj.select_set(True)
        bpy.context.view_layer.objects.active = obj
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)


def join_meshes(meshes, name):
    bpy.ops.object.select_all(action="DESELECT")
    for obj in meshes:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = meshes[0]
    bpy.ops.object.join()
    obj = bpy.context.view_layer.objects.active
    obj.name = name
    obj.data.name = f"{name}_Mesh"
    obj.location = (0.0, 0.0, 0.0)
    obj.rotation_euler = (0.0, 0.0, 0.0)
    obj.scale = (1.0, 1.0, 1.0)
    return obj


def local_bbox(obj):
    mins = Vector((math.inf, math.inf, math.inf))
    maxs = Vector((-math.inf, -math.inf, -math.inf))
    for vertex in obj.data.vertices:
        co = vertex.co
        mins.x = min(mins.x, co.x)
        mins.y = min(mins.y, co.y)
        mins.z = min(mins.z, co.z)
        maxs.x = max(maxs.x, co.x)
        maxs.y = max(maxs.y, co.y)
        maxs.z = max(maxs.z, co.z)
    return mins, maxs


def coord_axis(co, axis):
    if axis == 0:
        return co.x
    if axis == 1:
        return co.y
    return co.z


def normalize_projectile_forward_x(obj, target_long_axis_m):
    source_min, source_max = local_bbox(obj)
    source_size = source_max - source_min
    sizes = [source_size.x, source_size.y, source_size.z]
    if max(sizes) <= 0.001:
        raise RuntimeError(f"Invalid mesh bounds for {obj.name}: {source_size}")

    long_axis = max(range(3), key=lambda index: sizes[index])
    side_axes = [axis for axis in (0, 1, 2) if axis != long_axis]
    scale = target_long_axis_m / sizes[long_axis]

    centers = [
        (source_min.x + source_max.x) * 0.5,
        (source_min.y + source_max.y) * 0.5,
        (source_min.z + source_max.z) * 0.5,
    ]

    for vertex in obj.data.vertices:
        co = vertex.co
        forward = (coord_axis(co, long_axis) - centers[long_axis]) * scale
        side_y = (coord_axis(co, side_axes[0]) - centers[side_axes[0]]) * scale
        side_z = (coord_axis(co, side_axes[1]) - centers[side_axes[1]]) * scale
        vertex.co = Vector((forward, side_y, side_z))

    obj.data.update()
    final_min, final_max = local_bbox(obj)
    final_size = final_max - final_min
    return {
        "source_size_m": [source_size.x, source_size.y, source_size.z],
        "source_long_axis": ("X", "Y", "Z")[long_axis],
        "target_long_axis_m": target_long_axis_m,
        "scale": scale,
        "final_size_m": [final_size.x, final_size.y, final_size.z],
        "runtime_axes": "X=projectile forward, Y/Z=cross-section",
        "pivot": "center",
    }


def texture_node_from_base_color(material):
    if not material or not material.use_nodes:
        return None
    for node in material.node_tree.nodes:
        if node.type != "BSDF_PRINCIPLED":
            continue
        base_color = node.inputs.get("Base Color")
        if not base_color:
            continue
        for link in base_color.links:
            from_node = link.from_node
            if from_node and from_node.type == "TEX_IMAGE" and from_node.image:
                return from_node
    return None


def first_image_texture(obj):
    fallback = None
    for slot in obj.material_slots:
        material = slot.material
        node = texture_node_from_base_color(material)
        if node and node.image:
            return node.image
        if not material or not material.use_nodes:
            continue
        for node in material.node_tree.nodes:
            if node.type == "TEX_IMAGE" and node.image:
                fallback = fallback or node.image
    return fallback


def save_base_color_texture(obj, path):
    image = first_image_texture(obj)
    if not image:
        return None

    path.parent.mkdir(parents=True, exist_ok=True)
    if image.channels == 4:
        image = image.copy()
        pixels = list(image.pixels)
        for index in range(0, len(pixels), 4):
            alpha = pixels[index + 3]
            inv_alpha = 1.0 - alpha
            pixels[index] = (pixels[index] * alpha) + inv_alpha
            pixels[index + 1] = (pixels[index + 1] * alpha) + inv_alpha
            pixels[index + 2] = (pixels[index + 2] * alpha) + inv_alpha
            pixels[index + 3] = 1.0
        image.pixels[:] = pixels

    image.filepath_raw = str(path)
    image.file_format = "PNG"
    try:
        image.save()
    except RuntimeError:
        image.save_render(str(path))
    return str(path)


def export_selected_fbx(obj, path):
    path.parent.mkdir(parents=True, exist_ok=True)
    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    bpy.ops.export_scene.fbx(
        filepath=str(path),
        use_selection=True,
        object_types={"MESH"},
        apply_unit_scale=True,
        bake_space_transform=False,
        add_leaf_bones=False,
        mesh_smooth_type="FACE",
        path_mode="AUTO",
    )


def triangle_count(obj):
    obj.data.calc_loop_triangles()
    return len(obj.data.loop_triangles)


def load_manifest(run_root):
    manifest_path = Path(run_root) / "batch_manifest.json"
    with open(manifest_path, "r", encoding="utf-8") as handle:
        return json.load(handle)


def process_entry(entry, run_root, output_root, texture_root, target_long_axis_m):
    reset_scene()
    projectile_id = entry["projectile_id"]
    asset_name = f"SM_{projectile_id}"
    source = Path(run_root) / entry["trellis_output"]
    if not source.exists():
        raise FileNotFoundError(source)

    meshes = import_glb(source)
    detach_keep_world(meshes)
    apply_object_transforms(meshes)
    obj = join_meshes(meshes, asset_name)
    transform_report = normalize_projectile_forward_x(obj, target_long_axis_m)

    fbx_path = Path(output_root) / f"{asset_name}_UnrealReady.fbx"
    texture_path = Path(texture_root) / f"{asset_name}_BaseColor_00.png"
    export_selected_fbx(obj, fbx_path)
    saved_texture_path = save_base_color_texture(obj, texture_path)

    return {
        "hero_id": entry["hero_id"],
        "projectile_id": projectile_id,
        "asset_name": asset_name,
        "source": str(source),
        "fbx_export": str(fbx_path),
        "base_color_texture": saved_texture_path,
        "triangles": triangle_count(obj),
        **transform_report,
    }


def main():
    args = parse_args()
    run_root = Path(args.run_root)
    output_root = Path(args.output_root)
    texture_root = Path(args.texture_root)
    notes_root = Path(args.notes_root)
    notes_root.mkdir(parents=True, exist_ok=True)

    manifest = load_manifest(run_root)
    report = {
        "run_id": manifest.get("run_id"),
        "run_root": str(run_root),
        "output_root": str(output_root),
        "texture_root": str(texture_root),
        "target_long_axis_m": args.target_long_axis_m,
        "assets": [],
    }

    for entry in manifest.get("entries", []):
        result = process_entry(entry, run_root, output_root, texture_root, args.target_long_axis_m)
        report["assets"].append(result)
        print(f"[OK] {result['asset_name']} -> {result['fbx_export']}", flush=True)

    report_path = notes_root / "WeaponProjectiles_UnrealReadyManifest.json"
    with open(report_path, "w", encoding="ascii") as handle:
        json.dump(report, handle, indent=2)
    print(f"[OK] wrote {report_path}", flush=True)


if __name__ == "__main__":
    main()
