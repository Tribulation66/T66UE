import argparse
import json
import math
import os
import shutil
from datetime import datetime, timezone
from pathlib import Path

import bpy
from mathutils import Vector


REPO_ROOT = Path(r"C:\UE\T66")
RUN_ROOT = REPO_ROOT / "Model Generation" / "Runs" / "Interactables" / "WorldNpcInteractablesRetroBatch01"
STAGE02_MANIFEST = RUN_ROOT / "Reports" / "Stage02_QuadRetroManifest.json"
OUTPUT_ROOT = REPO_ROOT / "SourceAssets" / "Import" / "Interactables" / "WorldNpcInteractablesRetroBatch01" / "UnrealReady"
TEXTURE_ROOT = REPO_ROOT / "SourceAssets" / "Import" / "Interactables" / "WorldNpcInteractablesRetroBatch01" / "Textures"
REPORT_PATH = RUN_ROOT / "Reports" / "WorldNpcInteractablesRetroBatch01_UnrealReadyFbxManifest.json"


TARGET_HEIGHT_M = {
    "Vehicle": 1.35,
    "Arcade_Machine": 2.05,
    "Arcade_WhackAMole": 1.20,
    "Crate": 1.00,
    "Chest": 1.00,
    "LootBag_Black": 0.55,
    "LootBag_Red": 0.55,
    "LootBag_Yellow": 0.55,
    "LootBag_White": 0.55,
    "Fountain": 1.65,
    "DifficultyTotem": 1.00,
    "IdolAltar": 1.00,
    "QuickReviveVending": 2.20,
    "Shroom": 1.20,
    "ArcadeAmplifierPickup": 1.00,
    "ArcadeAmplifierPickup_Charged": 1.00,
    "Gambler": 2.35,
    "Saint": 2.15,
    "Ouroboros": 2.20,
}


def parse_args():
    argv = []
    if "--" in os.sys.argv:
        argv = os.sys.argv[os.sys.argv.index("--") + 1 :]

    parser = argparse.ArgumentParser()
    parser.add_argument("--stage02-manifest", default=str(STAGE02_MANIFEST))
    parser.add_argument("--output-root", default=str(OUTPUT_ROOT))
    parser.add_argument("--texture-root", default=str(TEXTURE_ROOT))
    parser.add_argument("--report", default=str(REPORT_PATH))
    return parser.parse_args(argv)


def reset_scene():
    bpy.ops.wm.read_factory_settings(use_empty=True)


def repo_path(relative_or_abs):
    if not relative_or_abs:
        return None
    path = Path(relative_or_abs)
    if path.is_absolute():
        return path
    return REPO_ROOT / path


def load_json(path):
    with Path(path).open("r", encoding="utf-8") as handle:
        return json.load(handle)


def write_json(path, payload):
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")


def target_for(row_id, category):
    if category == "NPCs":
        dest = f"/Game/Characters/NPCs/{row_id}/QuadRetro"
        name = f"SM_{row_id}_QuadRetro"
        return dest, name

    if row_id == "Vehicle" or row_id.startswith("Arcade_"):
        return f"/Game/World/Interactables/Arcade/{row_id}", f"{row_id}_QuadRetro"

    if row_id == "Chest":
        return "/Game/World/Interactables/Chests/ChestModel", "Chest_QuadRetro"

    if row_id.startswith("LootBag_"):
        color = row_id.split("_", 1)[1]
        return f"/Game/World/LootBags/{color}", f"SM_LootBag_{color}_QuadRetro"

    if row_id == "QuickReviveVending":
        return "/Game/World/Interactables/Vending", "QuickReviveVending_QuadRetro"

    if row_id in ("ArcadeAmplifierPickup", "ArcadeAmplifierPickup_Charged"):
        return "/Game/World/Interactables/ArcadeAmplifierPickup", f"{row_id}_QuadRetro"

    return f"/Game/World/Interactables/{row_id}", f"{row_id}_QuadRetro"


def object_path(asset_path):
    leaf = asset_path.rsplit("/", 1)[-1]
    return f"{asset_path}.{leaf}"


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
    bpy.context.view_layer.objects.active = meshes[0]
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


def normalize_bottom_center(obj, target_height_m):
    source_min, source_max = local_bbox(obj)
    source_size = source_max - source_min
    if source_size.z <= 0.001:
        raise RuntimeError(f"Invalid mesh height for {obj.name}: {source_size.z}")

    center_x = (source_min.x + source_max.x) * 0.5
    center_y = (source_min.y + source_max.y) * 0.5
    scale = float(target_height_m) / source_size.z

    for vertex in obj.data.vertices:
        co = vertex.co
        vertex.co = Vector(
            (
                (co.x - center_x) * scale,
                (co.y - center_y) * scale,
                (co.z - source_min.z) * scale,
            )
        )
    obj.data.update()

    final_min, final_max = local_bbox(obj)
    final_size = final_max - final_min
    return {
        "source_size_m": [source_size.x, source_size.y, source_size.z],
        "target_height_m": float(target_height_m),
        "scale": scale,
        "final_size_m": [final_size.x, final_size.y, final_size.z],
        "pivot": "bottom_center",
    }


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


def find_pixel_texture(entry):
    report_path = repo_path(entry.get("quad_retro_report"))
    if report_path and report_path.exists():
        report = load_json(report_path)
        texture = report.get("pixelated_texture") or report.get("baked_texture")
        texture_path = Path(texture) if texture else None
        if texture_path and texture_path.exists():
            return texture_path

    output_dir = repo_path(entry.get("quad_retro_output_dir"))
    if output_dir and output_dir.exists():
        matches = sorted((output_dir / "Textures").glob("*_Pixelated_*.png"))
        if matches:
            return matches[0]
    return None


def process_entry(entry, output_root, texture_root):
    row_id = entry.get("row_id")
    category = entry.get("category")
    if entry.get("quad_retro_status") != "ready_with_front_qa":
        return {
            "row_id": row_id,
            "category": category,
            "status": "skipped_not_ready",
            "quad_retro_status": entry.get("quad_retro_status"),
        }

    source_glb = repo_path(entry.get("quad_retro_glb"))
    if not source_glb or not source_glb.exists():
        return {
            "row_id": row_id,
            "category": category,
            "status": "missing_quad_retro_glb",
            "source_glb": str(source_glb) if source_glb else "",
        }

    dest, asset_name = target_for(row_id, category)
    target_height = TARGET_HEIGHT_M.get(row_id, 2.05 if category == "Interactables" else 2.10)
    fbx_path = output_root / category / row_id / f"{asset_name}_UnrealReady.fbx"
    texture_path = texture_root / category / row_id / f"T_{asset_name}_Pixelated_512.png"

    reset_scene()
    meshes = import_glb(source_glb)
    detach_keep_world(meshes)
    apply_object_transforms(meshes)
    obj = join_meshes(meshes, asset_name)
    transform_report = normalize_bottom_center(obj, target_height)
    export_selected_fbx(obj, fbx_path)

    source_texture = find_pixel_texture(entry)
    copied_texture = ""
    if source_texture and source_texture.exists():
        texture_path.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source_texture, texture_path)
        copied_texture = str(texture_path)

    fbx_rel = fbx_path.relative_to(REPO_ROOT / "SourceAssets" / "Import").as_posix()
    texture_rel = texture_path.relative_to(REPO_ROOT / "SourceAssets" / "Import").as_posix() if copied_texture else ""
    asset_path = f"{dest}/{asset_name}"

    return {
        "row_id": row_id,
        "category": category,
        "status": "ready",
        "source_glb": str(source_glb),
        "source_texture": str(source_texture) if source_texture else "",
        "fbx_export": str(fbx_path),
        "texture_export": copied_texture,
        "import_fbx_rel": fbx_rel,
        "import_texture_rel": texture_rel,
        "destination_path": dest,
        "asset_name": asset_name,
        "unreal_asset_path": object_path(asset_path),
        "triangles": triangle_count(obj),
        **transform_report,
    }


def main():
    args = parse_args()
    stage02_path = Path(args.stage02_manifest)
    output_root = Path(args.output_root)
    texture_root = Path(args.texture_root)
    report_path = Path(args.report)

    stage02 = load_json(stage02_path)
    entries = []
    for entry in stage02.get("entries", []):
        result = process_entry(entry, output_root, texture_root)
        entries.append(result)
        print(f"[{result.get('status')}] {result.get('row_id')} -> {result.get('fbx_export', '')}")

    failed = [entry for entry in entries if entry.get("status") != "ready"]
    missing_textures = [entry for entry in entries if entry.get("status") == "ready" and not entry.get("texture_export")]
    report = {
        "stage": "unreal_ready_fbx",
        "stage02_manifest": str(stage02_path),
        "output_root": str(output_root),
        "texture_root": str(texture_root),
        "summary": {
            "total_entries": len(entries),
            "ready": sum(1 for entry in entries if entry.get("status") == "ready"),
            "failed_or_skipped": len(failed),
            "failed_or_skipped_rows": [entry.get("row_id") for entry in failed],
            "missing_textures": len(missing_textures),
            "missing_texture_rows": [entry.get("row_id") for entry in missing_textures],
        },
        "entries": entries,
        "updated_utc": datetime.now(timezone.utc).isoformat(),
    }
    write_json(report_path, report)
    print(f"[OK] wrote {report_path}")

    if failed or missing_textures:
        raise RuntimeError(
            "Unreal-ready export incomplete: "
            f"failed={report['summary']['failed_or_skipped_rows']} "
            f"missing_textures={report['summary']['missing_texture_rows']}"
        )


if __name__ == "__main__":
    main()
