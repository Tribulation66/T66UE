#!/usr/bin/env python3
"""
Render all demo-scope Animated ToonStyle humanoid source meshes across yaw
candidates so front/back corrections are evidence-driven before export.

Run with Blender:

  blender --background --python "Model Generation/Rigging and Animation/Tools/render_animated_toonstyle_facing_diagnostic.py"

Outputs one template axis sheet and four all-28 candidate sheets under
Model Generation/Rigging and Animation/Runs/AnimatedToonStyleFacing_<timestamp>.
"""

from __future__ import annotations

import math
import os
import shutil
from datetime import datetime
from pathlib import Path

import bpy
from mathutils import Vector


PROJECT_ROOT = Path(os.environ.get("T66_PROJECT_ROOT", r"C:\UE\T66"))
TEMPLATE_BLEND = Path(
    os.environ.get(
        "T66_ANIMATED_TOONSTYLE_TEMPLATE_BLEND",
        PROJECT_ROOT
        / "Model Generation"
        / "Runs"
        / "Pixal3D"
        / "HeroDemoLineup_20260522_AccuRig"
        / "RigifyWalkProbe_20260522"
        / "Hero_1_Chad_Rigify_AnimatedToonStyle_Template.blend",
    )
)
DEFAULT_SOURCE_ROOT = Path(
    os.environ.get(
        "T66_ANIMATED_TOONSTYLE_DEFAULT_SOURCE_ROOT",
        PROJECT_ROOT / "Model Generation" / "Runs" / "Pixal3D" / "HeroDemoLineup_20260522_AccuRig" / "Outputs",
    )
)
HUMANOID_SOURCE_ROOT = Path(
    os.environ.get(
        "T66_ANIMATED_TOONSTYLE_HUMANOID_SOURCE_ROOT",
        PROJECT_ROOT / "Model Generation" / "Runs" / "Pixal3D" / "HumanoidGuidelineTest_20260522_100k" / "Outputs",
    )
)
OUTPUT_ROOT = Path(
    os.environ.get(
        "T66_ANIMATED_TOONSTYLE_FACING_OUTPUT_ROOT",
        PROJECT_ROOT
        / "Model Generation"
        / "Rigging and Animation"
        / "Runs"
        / f"AnimatedToonStyleFacing_{datetime.now().strftime('%Y%m%d_%H%M%S')}",
    )
)

TEMPLATE_MESH_NAME = "Hero_1_Chad_RigifyProbe_Mesh"
TARGET_HEIGHT = 1.8
YAW_CANDIDATES = [0, 90, 180, 270]


def default_records() -> list[dict[str, str]]:
    records: list[dict[str, str]] = []
    for index in range(1, 6):
        for body in ("Chad", "Stacy"):
            visual_id = f"Hero_{index}_{body}"
            records.append({"visual_id": visual_id, "source_id": visual_id, "source_root": str(DEFAULT_SOURCE_ROOT)})
    return records


def humanoid_records() -> list[dict[str, str]]:
    records: list[dict[str, str]] = []
    for index in range(1, 6):
        for body in ("Chad", "Stacy"):
            records.append(
                {
                    "visual_id": f"Hero_{index}_{body}_DemoSkin",
                    "source_id": f"Hero_{index}_{body}_Demo",
                    "source_root": str(HUMANOID_SOURCE_ROOT),
                }
            )
    records.extend(
        [
            {
                "visual_id": "Companion_01",
                "source_id": "Companion_RapVixenLightskinBlack_Regular",
                "source_root": str(HUMANOID_SOURCE_ROOT),
            },
            {
                "visual_id": "Companion_01_DemoSkin",
                "source_id": "Companion_RapVixenLightskinBlack_Demo",
                "source_root": str(HUMANOID_SOURCE_ROOT),
            },
            {
                "visual_id": "Companion_02",
                "source_id": "Companion_BarMaidenBlonde_Regular",
                "source_root": str(HUMANOID_SOURCE_ROOT),
            },
            {
                "visual_id": "Companion_02_DemoSkin",
                "source_id": "Companion_BarMaidenBlonde_Demo",
                "source_root": str(HUMANOID_SOURCE_ROOT),
            },
            {
                "visual_id": "Companion_03",
                "source_id": "Companion_CollegeBrunette_Regular",
                "source_root": str(HUMANOID_SOURCE_ROOT),
            },
            {
                "visual_id": "Companion_03_DemoSkin",
                "source_id": "Companion_CollegeBrunette_Demo",
                "source_root": str(HUMANOID_SOURCE_ROOT),
            },
            {
                "visual_id": "Companion_04",
                "source_id": "Companion_OfficeLadyBlackhair_Regular",
                "source_root": str(HUMANOID_SOURCE_ROOT),
            },
            {
                "visual_id": "Companion_04_DemoSkin",
                "source_id": "Companion_OfficeLadyBlackhair_Demo",
                "source_root": str(HUMANOID_SOURCE_ROOT),
            },
        ]
    )
    return records


def world_bbox(objects: list[bpy.types.Object]) -> tuple[Vector, Vector]:
    coords: list[Vector] = []
    for obj in objects:
        if obj.type == "MESH" and obj.data and obj.data.vertices:
            coords.extend(obj.matrix_world @ vertex.co for vertex in obj.data.vertices)
        elif hasattr(obj, "bound_box"):
            coords.extend(obj.matrix_world @ Vector(corner) for corner in obj.bound_box)
    if not coords:
        return Vector((-0.5, -0.5, 0.0)), Vector((0.5, 0.5, TARGET_HEIGHT))
    return (
        Vector((min(v.x for v in coords), min(v.y for v in coords), min(v.z for v in coords))),
        Vector((max(v.x for v in coords), max(v.y for v in coords), max(v.z for v in coords))),
    )


def select_only(objects: list[bpy.types.Object]) -> None:
    bpy.ops.object.select_all(action="DESELECT")
    for obj in objects:
        obj.select_set(True)
    if objects:
        bpy.context.view_layer.objects.active = objects[0]


def imported_meshes(before_names: set[str]) -> list[bpy.types.Object]:
    return [
        obj
        for obj in bpy.data.objects
        if obj.name not in before_names and obj.type == "MESH" and not obj.name.startswith("WGT-")
    ]


def import_mesh(source_glb: Path, visual_id: str) -> bpy.types.Object:
    before = {obj.name for obj in bpy.data.objects}
    bpy.ops.import_scene.gltf(filepath=str(source_glb))
    meshes = imported_meshes(before)
    if not meshes:
        raise RuntimeError(f"{visual_id}: no mesh imported from {source_glb}")
    if len(meshes) > 1:
        select_only(meshes)
        bpy.context.view_layer.objects.active = meshes[0]
        bpy.ops.object.join()
        mesh = bpy.context.view_layer.objects.active
    else:
        mesh = meshes[0]
    mesh.name = f"{visual_id}_FacingDiagnostic"
    return mesh


def look_at(obj: bpy.types.Object, target: Vector) -> None:
    direction = target - obj.location
    obj.rotation_euler = direction.to_track_quat("-Z", "Y").to_euler()


def setup_scene(clear_scene: bool = True) -> bpy.types.Object:
    if clear_scene:
        bpy.ops.object.select_all(action="SELECT")
        bpy.ops.object.delete()
    scene = bpy.context.scene
    scene.render.engine = "BLENDER_EEVEE"
    scene.render.resolution_x = 260
    scene.render.resolution_y = 340
    if hasattr(scene, "eevee"):
        scene.eevee.taa_render_samples = 16
    scene.view_settings.view_transform = "Standard"
    scene.view_settings.look = "Medium High Contrast"
    scene.world = bpy.data.worlds.new("T66FacingWorld") if not scene.world else scene.world
    scene.world.color = (0.45, 0.45, 0.45)

    camera_data = bpy.data.cameras.new("FacingDiagnosticCamera")
    camera = bpy.data.objects.new("FacingDiagnosticCamera", camera_data)
    bpy.context.collection.objects.link(camera)
    camera.data.type = "ORTHO"
    camera.data.ortho_scale = 2.25
    scene.camera = camera

    light_data = bpy.data.lights.new("FacingDiagnosticKey", "AREA")
    light = bpy.data.objects.new("FacingDiagnosticKey", light_data)
    bpy.context.collection.objects.link(light)
    light.location = (0.0, -3.5, 3.0)
    light.data.energy = 650.0
    light.data.size = 5.0
    return camera


def render_object_front(obj: bpy.types.Object, yaw_degrees: float, output_path: Path, camera: bpy.types.Object) -> None:
    obj.rotation_mode = "XYZ"
    obj.rotation_euler = (0.0, 0.0, math.radians(yaw_degrees))
    obj.scale = (1.0, 1.0, 1.0)
    obj.location = (0.0, 0.0, 0.0)
    bpy.context.view_layer.update()

    min_v, max_v = world_bbox([obj])
    height = max(max_v.z - min_v.z, 0.001)
    scale = TARGET_HEIGHT / height
    obj.scale = (scale, scale, scale)
    bpy.context.view_layer.update()

    min_v, max_v = world_bbox([obj])
    obj.location += Vector((-(min_v.x + max_v.x) * 0.5, -(min_v.y + max_v.y) * 0.5, -min_v.z))
    bpy.context.view_layer.update()

    min_v, max_v = world_bbox([obj])
    target = Vector((0.0, 0.0, (min_v.z + max_v.z) * 0.5))
    camera.location = (0.0, -4.0, target.z)
    look_at(camera, target)
    bpy.context.scene.render.filepath = str(output_path)
    bpy.ops.render.render(write_still=True)


def render_template_axis(template_mesh: bpy.types.Object, output_dir: Path) -> list[Path]:
    outputs: list[Path] = []
    camera = setup_scene(clear_scene=False)
    for obj in list(bpy.context.scene.objects):
        if obj.name not in {camera.name, "FacingDiagnosticKey"}:
            try:
                obj.hide_set(True)
            except RuntimeError:
                pass
            obj.hide_render = True

    template_copy = template_mesh.copy()
    template_copy.data = template_mesh.data.copy()
    template_copy.name = "Hero_1_Chad_TemplateAxisCopy"
    bpy.context.collection.objects.link(template_copy)
    template_copy.hide_set(False)
    template_copy.hide_render = False
    for yaw in YAW_CANDIDATES:
        path = output_dir / f"template_yaw_{yaw:03d}.png"
        render_object_front(template_copy, yaw, path, camera)
        outputs.append(path)
    bpy.data.objects.remove(template_copy, do_unlink=True)
    return outputs


def compose_sheet(image_paths: list[Path], labels: list[str], columns: int, output_path: Path) -> None:
    try:
        from PIL import Image, ImageDraw, ImageFont
    except Exception as exc:
        manifest_path = output_path.with_suffix(".txt")
        manifest_path.write_text(
            "\n".join(f"{label}|{path}" for label, path in zip(labels, image_paths)),
            encoding="utf-8",
        )
        print(f"[AnimatedToonStyleFacing] PIL unavailable in Blender ({exc}); wrote {manifest_path}")
        return

    if not image_paths:
        return
    thumbs = [Image.open(path).convert("RGB") for path in image_paths]
    cell_w, cell_h = thumbs[0].size
    label_h = 34
    rows = math.ceil(len(thumbs) / columns)
    sheet = Image.new("RGB", (columns * cell_w, rows * (cell_h + label_h)), (24, 24, 28))
    draw = ImageDraw.Draw(sheet)
    try:
        font = ImageFont.truetype("arial.ttf", 13)
    except Exception:
        font = ImageFont.load_default()
    for index, thumb in enumerate(thumbs):
        col = index % columns
        row = index // columns
        x = col * cell_w
        y = row * (cell_h + label_h)
        draw.rectangle((x, y, x + cell_w, y + label_h), fill=(12, 12, 16))
        draw.text((x + 6, y + 8), labels[index][:34], fill=(245, 245, 245), font=font)
        sheet.paste(thumb, (x, y + label_h))
    sheet.save(output_path)


def main() -> None:
    if not TEMPLATE_BLEND.exists():
        raise FileNotFoundError(TEMPLATE_BLEND)
    if not DEFAULT_SOURCE_ROOT.exists():
        raise FileNotFoundError(DEFAULT_SOURCE_ROOT)
    if not HUMANOID_SOURCE_ROOT.exists():
        raise FileNotFoundError(HUMANOID_SOURCE_ROOT)
    OUTPUT_ROOT.mkdir(parents=True, exist_ok=True)
    cell_dir = OUTPUT_ROOT / "cells"
    if cell_dir.exists():
        shutil.rmtree(cell_dir)
    cell_dir.mkdir(parents=True)

    bpy.ops.wm.open_mainfile(filepath=str(TEMPLATE_BLEND))
    template_mesh = bpy.data.objects.get(TEMPLATE_MESH_NAME)
    if not template_mesh:
        raise RuntimeError(f"Missing template mesh {TEMPLATE_MESH_NAME}")
    axis_paths = render_template_axis(template_mesh, cell_dir)
    compose_sheet(axis_paths, [f"Template yaw {yaw}" for yaw in YAW_CANDIDATES], 4, OUTPUT_ROOT / "template_axis_sheet.png")

    records = default_records() + humanoid_records()
    missing = [
        (record["visual_id"], str(Path(record["source_root"]) / f"{record['source_id']}.glb"))
        for record in records
        if not (Path(record["source_root"]) / f"{record['source_id']}.glb").exists()
    ]
    if missing:
        raise FileNotFoundError(f"Missing source GLBs: {missing}")

    camera = setup_scene()
    all_cell_paths: list[Path] = []
    all_labels: list[str] = []
    for record in records:
        visual_id = record["visual_id"]
        source_glb = Path(record["source_root"]) / f"{record['source_id']}.glb"
        mesh = import_mesh(source_glb, visual_id)
        for yaw in YAW_CANDIDATES:
            path = cell_dir / f"{visual_id}_yaw_{yaw:03d}.png"
            render_object_front(mesh, yaw, path, camera)
            all_cell_paths.append(path)
            all_labels.append(f"{visual_id} yaw {yaw}")
        bpy.data.objects.remove(mesh, do_unlink=True)

    page_size = 7 * len(YAW_CANDIDATES)
    for page_index, start in enumerate(range(0, len(all_cell_paths), page_size), start=1):
        end = min(start + page_size, len(all_cell_paths))
        compose_sheet(
            all_cell_paths[start:end],
            all_labels[start:end],
            len(YAW_CANDIDATES),
            OUTPUT_ROOT / f"all28_yaw_candidates_page_{page_index:02d}.png",
        )

    print(f"[AnimatedToonStyleFacing] wrote {OUTPUT_ROOT}")


if __name__ == "__main__":
    main()
