"""FriendSlop projectile export (modeled on export_weapon_projectiles_unreal_ready.py).
WeaponProjectile + IdolProjectile GLBs -> UnrealReady FBX (+X forward, 2.0m long axis) with
RAW base-color PNGs. Lit-era additions: conservative merge-by-distance clean (UVs kept),
shade smooth + Weighted Normal modifier applied (master material is LIT).
Run: blender -b --python <this> -- [--target-long-axis-m 2.0]
"""
from __future__ import annotations
import json
import os
from pathlib import Path

import bpy
from mathutils import Vector

REPO_ROOT = Path(r"C:/UE/T66")
RUN_ROOT = REPO_ROOT / "Model Generation" / "Runs" / "Pixal3D" / "FriendSlopProjectiles_20260609_0659"
OUTPUT_ROOT = REPO_ROOT / "SourceAssets" / "Import" / "Weapons" / "Projectiles" / "FriendSlop" / "UnrealReady"
TEXTURE_ROOT = REPO_ROOT / "SourceAssets" / "Import" / "Weapons" / "Projectiles" / "FriendSlop" / "Textures"
NOTES_ROOT = RUN_ROOT / "Notes"
TARGET_LONG_AXIS_M = 2.0
ENTRIES = [
    {"projectile_id": "WeaponProjectile_Black", "glb": RUN_ROOT / "Outputs" / "WeaponProjectile.glb"},
    {"projectile_id": "IdolProjectile_FireBlack", "glb": RUN_ROOT / "Outputs" / "IdolProjectile.glb"},
]


def reset_scene():
    bpy.ops.wm.read_factory_settings(use_empty=True)


def import_glb(path):
    before = set(bpy.data.objects)
    bpy.ops.import_scene.gltf(filepath=str(path))
    return [o for o in set(bpy.data.objects) - before if o.type == "MESH"]


def detach_keep_world(meshes):
    for obj in meshes:
        world = obj.matrix_world.copy()
        obj.parent = None
        obj.matrix_world = world


def apply_object_transforms(meshes):
    bpy.ops.object.select_all(action="DESELECT")
    for obj in meshes:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = meshes[0]
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)


def join_meshes(meshes, name):
    bpy.ops.object.select_all(action="DESELECT")
    for obj in meshes:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = meshes[0]
    if len(meshes) > 1:
        bpy.ops.object.join()
    obj = bpy.context.view_layer.objects.active
    obj.name = name
    obj.data.name = name
    obj.scale = (1.0, 1.0, 1.0)
    return obj


def clean_mesh_keep_uvs(obj):
    """Conservative cleanup: merge-by-distance (tiny epsilon) + degenerate dissolve. UVs untouched."""
    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    bpy.ops.object.mode_set(mode="EDIT")
    bpy.ops.mesh.select_all(action="SELECT")
    bpy.ops.mesh.remove_doubles(threshold=0.0001)
    bpy.ops.mesh.dissolve_degenerate(threshold=0.0001)
    bpy.ops.mesh.normals_make_consistent(inside=False)
    bpy.ops.object.mode_set(mode="OBJECT")


def smooth_weighted_normals(obj):
    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    bpy.ops.object.shade_smooth()
    mod = obj.modifiers.new(name="WeightedNormal", type="WEIGHTED_NORMAL")
    mod.keep_sharp = True
    mod.weight = 50
    bpy.ops.object.modifier_apply(modifier=mod.name)


def local_bbox(obj):
    coords = [Vector(c) for c in obj.bound_box]
    mins = Vector((min(c.x for c in coords), min(c.y for c in coords), min(c.z for c in coords)))
    maxs = Vector((max(c.x for c in coords), max(c.y for c in coords), max(c.z for c in coords)))
    return mins, maxs


def coord_axis(co, axis):
    return (co.x, co.y, co.z)[axis]


def normalize_projectile_forward_x(obj, target_long_axis_m):
    mins, maxs = local_bbox(obj)
    sizes = [maxs[i] - mins[i] for i in range(3)]
    centers = [(maxs[i] + mins[i]) * 0.5 for i in range(3)]
    long_axis = max(range(3), key=lambda index: sizes[index])
    side_axes = [axis for axis in (0, 1, 2) if axis != long_axis]
    scale = target_long_axis_m / sizes[long_axis]
    for vertex in obj.data.vertices:
        co = vertex.co.copy()
        forward = (coord_axis(co, long_axis) - centers[long_axis]) * scale
        side_y = (coord_axis(co, side_axes[0]) - centers[side_axes[0]]) * scale
        side_z = (coord_axis(co, side_axes[1]) - centers[side_axes[1]]) * scale
        vertex.co = Vector((forward, side_y, side_z))
    obj.data.update()
    return {
        "source_long_axis": ("X", "Y", "Z")[long_axis],
        "target_long_axis_m": target_long_axis_m,
        "scale": scale,
        "runtime_axes": "X=projectile forward, Y/Z=cross-section",
    }


def first_image_texture(obj):
    for slot in obj.material_slots:
        mat = slot.material
        if not mat or not mat.use_nodes:
            continue
        for node in mat.node_tree.nodes:
            if node.type == "TEX_IMAGE" and node.image:
                return node.image
    return None


def save_base_color_texture(obj, path):
    image = first_image_texture(obj)
    if not image:
        return None
    path.parent.mkdir(parents=True, exist_ok=True)
    image.filepath_raw = str(path)
    image.file_format = "PNG"
    image.save()
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


def main():
    NOTES_ROOT.mkdir(parents=True, exist_ok=True)
    report = {"run_root": str(RUN_ROOT), "assets": []}
    for entry in ENTRIES:
        reset_scene()
        asset_name = f"SM_{entry['projectile_id']}"
        meshes = import_glb(entry["glb"])
        if not meshes:
            raise RuntimeError(f"no meshes in {entry['glb']}")
        detach_keep_world(meshes)
        apply_object_transforms(meshes)
        obj = join_meshes(meshes, asset_name)
        clean_mesh_keep_uvs(obj)
        smooth_weighted_normals(obj)
        transform_report = normalize_projectile_forward_x(obj, TARGET_LONG_AXIS_M)
        fbx_path = OUTPUT_ROOT / f"{asset_name}_UnrealReady.fbx"
        texture_path = TEXTURE_ROOT / f"{asset_name}_BaseColor_00.png"
        export_selected_fbx(obj, fbx_path)
        saved_texture = save_base_color_texture(obj, texture_path)
        report["assets"].append({
            "asset_name": asset_name,
            "source": str(entry["glb"]),
            "fbx_export": str(fbx_path),
            "base_color_texture": saved_texture,
            "triangles": triangle_count(obj),
            "uv_layers": len(obj.data.uv_layers),
            **transform_report,
        })
        print(f"[OK] {asset_name} -> {fbx_path}", flush=True)
    report_path = NOTES_ROOT / "FriendSlopProjectiles_UnrealReadyManifest.json"
    with open(report_path, "w", encoding="ascii") as handle:
        json.dump(report, handle, indent=2)
    print(f"[OK] wrote {report_path}", flush=True)


if __name__ == "__main__":
    main()
