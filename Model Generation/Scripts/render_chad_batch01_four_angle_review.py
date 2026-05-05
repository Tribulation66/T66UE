import json
import math
import re
import sys
from pathlib import Path

import bpy
from mathutils import Vector


MODEL_GEN_ROOT = Path(__file__).resolve().parents[1]
BATCH_ROOT = MODEL_GEN_ROOT / "Runs" / "Heroes" / "TypeA_Masculine_Batch01"
RIG_REPORT = BATCH_ROOT / "Rigging" / "batch_typea_rig_report.json"
OUTPUT_ROOT = BATCH_ROOT / "Review" / "ChadFourAngleRenders"

ACTIVE_HERO_IDS = (1, 3, 4, 5, 7, 8, 9, 11, 12, 13, 14, 15)
VIEW_YAWS = {
    "front": 0.0,
    "left": -90.0,
    "right": 90.0,
    "back": 180.0,
}


def scene_engine():
    engines = {item.identifier for item in bpy.types.RenderSettings.bl_rna.properties["engine"].enum_items}
    return "BLENDER_EEVEE_NEXT" if "BLENDER_EEVEE_NEXT" in engines else "BLENDER_EEVEE"


def safe_name(value):
    return re.sub(r"[^A-Za-z0-9_]+", "_", value).strip("_")


def mesh_objects():
    return [obj for obj in bpy.context.scene.objects if obj.type == "MESH"]


def mesh_bounds(objects):
    depsgraph = bpy.context.evaluated_depsgraph_get()
    min_v = Vector((float("inf"), float("inf"), float("inf")))
    max_v = Vector((float("-inf"), float("-inf"), float("-inf")))

    for obj in objects:
        eval_obj = obj.evaluated_get(depsgraph)
        for corner in eval_obj.bound_box:
            point = eval_obj.matrix_world @ Vector(corner)
            min_v.x = min(min_v.x, point.x)
            min_v.y = min(min_v.y, point.y)
            min_v.z = min(min_v.z, point.z)
            max_v.x = max(max_v.x, point.x)
            max_v.y = max(max_v.y, point.y)
            max_v.z = max(max_v.z, point.z)

    if min_v.x == float("inf"):
        return Vector((0.0, 0.0, 0.0)), Vector((0.0, 0.0, 0.0)), Vector((0.0, 0.0, 0.0))
    return min_v, max_v, max_v - min_v


def reset_armatures_to_rest():
    for obj in bpy.context.scene.objects:
        if obj.type != "ARMATURE":
            continue
        bpy.context.view_layer.objects.active = obj
        obj.select_set(True)
        bpy.ops.object.mode_set(mode="POSE")
        for pose_bone in obj.pose.bones:
            pose_bone.rotation_mode = "XYZ"
            pose_bone.location = (0.0, 0.0, 0.0)
            pose_bone.rotation_euler = (0.0, 0.0, 0.0)
            pose_bone.scale = (1.0, 1.0, 1.0)
        bpy.ops.object.mode_set(mode="OBJECT")
        obj.select_set(False)
    bpy.context.scene.frame_set(1)
    bpy.context.view_layer.update()


def clear_review_nodes():
    for obj in list(bpy.data.objects):
        if obj.name.startswith("ChadReview_"):
            bpy.data.objects.remove(obj, do_unlink=True)


def ensure_lighting(center):
    if not any(obj.type == "LIGHT" for obj in bpy.context.scene.objects):
        bpy.ops.object.light_add(type="AREA", location=(center.x - 1.5, center.y - 3.0, center.z + 3.0))
        light = bpy.context.object
        light.name = "ChadReview_Key_Light"
        light.data.energy = 600
        light.data.size = 4.0


def add_camera(center, size, yaw_deg):
    cam_data = bpy.data.cameras.new(name=f"ChadReview_Camera_{yaw_deg:g}")
    cam_data.type = "ORTHO"
    cam = bpy.data.objects.new(cam_data.name, cam_data)
    bpy.context.scene.collection.objects.link(cam)

    yaw = math.radians(yaw_deg)
    pitch = math.radians(4.0)
    direction = Vector((math.sin(yaw) * math.cos(pitch), -math.cos(yaw) * math.cos(pitch), math.sin(pitch)))
    cam.location = center + direction * 4.5

    target = bpy.data.objects.new(f"ChadReview_Target_{yaw_deg:g}", None)
    target.location = center
    bpy.context.scene.collection.objects.link(target)

    track = cam.constraints.new(type="TRACK_TO")
    track.target = target
    track.track_axis = "TRACK_NEGATIVE_Z"
    track.up_axis = "UP_Y"

    horizontal = max(size.x, size.y, 0.001)
    cam_data.ortho_scale = max(horizontal, size.z) * 1.18
    bpy.context.scene.camera = cam


def render_one(report, view, yaw):
    hero_id = int(report["hero_id"])
    hero_name = report["hero_name"]
    skin = report["skin"]
    blend_path = Path(report["blend"])
    if not blend_path.is_file():
        raise RuntimeError(f"Missing blend file: {blend_path}")

    bpy.ops.wm.open_mainfile(filepath=str(blend_path))
    reset_armatures_to_rest()
    clear_review_nodes()

    meshes = mesh_objects()
    if not meshes:
        raise RuntimeError(f"No mesh objects in {blend_path}")

    min_v, max_v, size = mesh_bounds(meshes)
    center = (min_v + max_v) * 0.5
    ensure_lighting(center)
    add_camera(center, size, yaw)

    scene = bpy.context.scene
    scene.render.engine = scene_engine()
    scene.render.resolution_x = 960
    scene.render.resolution_y = 960
    scene.view_settings.view_transform = "Standard"
    scene.view_settings.look = "Medium High Contrast"
    scene.world.color = (0.035, 0.035, 0.035)

    out_dir = OUTPUT_ROOT / f"Hero_{hero_id}_{safe_name(hero_name)}" / skin
    out_dir.mkdir(parents=True, exist_ok=True)
    out_path = out_dir / f"Hero_{hero_id}_{safe_name(hero_name)}_Chad_{skin}_{view}.png"
    scene.render.filepath = str(out_path)
    bpy.ops.render.render(write_still=True)
    return out_path


def load_reports():
    data = json.loads(RIG_REPORT.read_text(encoding="utf-8"))
    reports = data.get("reports", [])
    selected = [
        report
        for report in reports
        if int(report.get("hero_id", -1)) in ACTIVE_HERO_IDS
        and report.get("skin") in {"Standard", "Beachgoer"}
    ]
    selected.sort(key=lambda report: (int(report["hero_id"]), 0 if report["skin"] == "Standard" else 1))
    if len(selected) != 24:
        raise RuntimeError(f"Expected 24 Chad rig reports, found {len(selected)}")
    return selected


def write_indexes(manifest):
    OUTPUT_ROOT.mkdir(parents=True, exist_ok=True)
    json_path = OUTPUT_ROOT / "manifest.json"
    json_path.write_text(json.dumps(manifest, indent=2), encoding="utf-8")

    by_hero = {}
    for item in manifest["renders"]:
        key = (item["hero_id"], item["hero_name"])
        by_hero.setdefault(key, {}).setdefault(item["skin"], {})[item["view"]] = item["path"]

    lines = [
        "# Chad Batch01 Four-Angle Review",
        "",
        "Rest-pose Blender renders from the existing rigged Chad Standard and Beachgoer scenes.",
        "",
    ]
    for hero_id, hero_name in sorted(by_hero):
        lines.append(f"## Hero {hero_id}: {hero_name}")
        for skin in ("Standard", "Beachgoer"):
            views = by_hero[(hero_id, hero_name)].get(skin, {})
            links = []
            for view in ("front", "left", "right", "back"):
                path = views.get(view, "")
                label = view.title()
                links.append(f"[{label}]({Path(path).as_posix()})")
            lines.append(f"- {skin}: " + " | ".join(links))
        lines.append("")

    md_path = OUTPUT_ROOT / "INDEX.md"
    md_path.write_text("\n".join(lines), encoding="utf-8")
    return json_path, md_path


def main():
    reports = load_reports()
    renders = []
    failures = []

    for index, report in enumerate(reports, start=1):
        label = f"Hero_{report['hero_id']}_{report['hero_name']}_{report['skin']}"
        print(f"[{index:02d}/{len(reports)}] Rendering {label}")
        for view, yaw in VIEW_YAWS.items():
            try:
                out_path = render_one(report, view, yaw)
                renders.append(
                    {
                        "hero_id": int(report["hero_id"]),
                        "hero_name": report["hero_name"],
                        "skin": report["skin"],
                        "view": view,
                        "path": str(out_path),
                    }
                )
                print(f"  {view}: {out_path}")
            except Exception as exc:
                failures.append(
                    {
                        "hero_id": int(report["hero_id"]),
                        "hero_name": report["hero_name"],
                        "skin": report["skin"],
                        "view": view,
                        "error": repr(exc),
                    }
                )
                print(f"  [ERROR] {view}: {exc!r}")

    manifest = {
        "source_report": str(RIG_REPORT),
        "output_root": str(OUTPUT_ROOT),
        "render_count": len(renders),
        "failure_count": len(failures),
        "renders": renders,
        "failures": failures,
    }
    json_path, md_path = write_indexes(manifest)
    print(json.dumps({"manifest": str(json_path), "index": str(md_path), "render_count": len(renders), "failure_count": len(failures)}, indent=2))
    if failures:
        raise SystemExit(2)


if __name__ == "__main__":
    main()
