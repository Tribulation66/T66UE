import argparse
import json
import math
import os
from dataclasses import dataclass
from pathlib import Path

import bpy
from mathutils import Matrix, Vector


MODEL_GEN_ROOT = Path(__file__).resolve().parents[1]
BATCH_ROOT = MODEL_GEN_ROOT / "Runs" / "Heroes" / "TypeA_Masculine_Batch01"
ARTHUR_RAW = MODEL_GEN_ROOT / "Runs" / "Heroes" / "Hero_1_Arthur" / "Raw" / "Trellis"


@dataclass(frozen=True)
class PairSpec:
    hero_id: str
    name: str
    variant: str
    body_glb: Path
    head_glb: Path
    output_stem: str


# These are deliberately conservative. They are visual assembly parameters, not
# rig or socket data, and should be adjusted only after multi-angle Blender QA.
DEFAULT_HEAD_HEIGHT_RATIO = 0.245
DEFAULT_NECK_OVERLAP_RATIO = 0.045
DEFAULT_HEAD_Y_OFFSET_RATIO = -0.006

PLACEMENT_OVERRIDES = {
    # hero/name/variant: optional small visual corrections after screenshot QA.
    # "Hero_3_LuBu_standard": {"head_height_ratio": 0.252, "neck_overlap_ratio": 0.050},
}


def parse_args():
    argv = os.sys.argv[os.sys.argv.index("--") + 1 :] if "--" in os.sys.argv else []
    parser = argparse.ArgumentParser()
    parser.add_argument("--all", action="store_true", help="Assemble every available standard and beach body/head pair.")
    parser.add_argument("--clean-tests", action="store_true", help="Assemble the first clean review set: Lu Bu, Mike, George, Robo.")
    parser.add_argument("--hero", action="append", default=[], help="Hero name or Hero_ID to assemble. Can be passed multiple times.")
    parser.add_argument("--variant", choices=["standard", "beach_goer", "both"], default="both")
    parser.add_argument("--render", action="store_true", default=True)
    parser.add_argument("--export", action="store_true", default=True)
    parser.add_argument("--resolution", type=int, default=1400)
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
    scene.world = bpy.data.worlds.new("AssemblyWorld")
    scene.world.use_nodes = True
    bg = scene.world.node_tree.nodes["Background"]
    bg.inputs[0].default_value = (0.18, 0.18, 0.18, 1.0)
    bg.inputs[1].default_value = 0.8
    return scene


def import_glb(path: Path, prefix: str):
    before = set(bpy.context.scene.objects)
    bpy.ops.import_scene.gltf(filepath=str(path))
    imported = [obj for obj in bpy.context.scene.objects if obj not in before]
    if not imported:
        raise RuntimeError(f"No objects imported from {path}")
    for obj in imported:
        obj.name = f"{prefix}_{obj.name}"
        if obj.data:
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


def add_camera(center, size, yaw_deg, pitch_deg=4.0):
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
    cam_data.ortho_scale = max(size.x, size.z) * 1.28
    return cam


def render_views(stem: str, render_dir: Path, resolution: int):
    render_dir.mkdir(parents=True, exist_ok=True)
    meshes = [obj for obj in bpy.context.scene.objects if obj.type == "MESH"]
    min_v, max_v, size = object_bounds(meshes)
    center = (min_v + max_v) * 0.5

    scene = bpy.context.scene
    scene.render.resolution_x = resolution
    scene.render.resolution_y = resolution

    views = {
        "front": 0.0,
        "right": 90.0,
        "left_oblique": -35.0,
        "right_oblique": 35.0,
    }
    outputs = {}
    for label, yaw in views.items():
        add_camera(center, size, yaw)
        out = render_dir / f"{stem}_{label}.png"
        scene.render.filepath = str(out)
        bpy.ops.render.render(write_still=True)
        outputs[label] = str(out)
    return outputs


def export_glb(path: Path):
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


def placement_for(spec: PairSpec):
    key = f"{spec.hero_id}_{spec.name.replace(' ', '')}_{spec.variant}"
    override = PLACEMENT_OVERRIDES.get(key, {})
    return {
        "head_height_ratio": override.get("head_height_ratio", DEFAULT_HEAD_HEIGHT_RATIO),
        "neck_overlap_ratio": override.get("neck_overlap_ratio", DEFAULT_NECK_OVERLAP_RATIO),
        "head_y_offset_ratio": override.get("head_y_offset_ratio", DEFAULT_HEAD_Y_OFFSET_RATIO),
        "head_x_offset_ratio": override.get("head_x_offset_ratio", 0.0),
    }


def assemble_pair(spec: PairSpec, export_dir: Path, render_dir: Path, blend_dir: Path, resolution: int):
    scene = reset_scene()
    body_imported, body_meshes = import_glb(spec.body_glb, "Body")
    head_imported, head_meshes = import_glb(spec.head_glb, "Head")

    body_min, body_max, body_size = object_bounds(body_meshes)
    head_min, head_max, head_size = object_bounds(head_meshes)
    body_center = (body_min + body_max) * 0.5
    head_anchor = Vector(
        (
            (head_min.x + head_max.x) * 0.5,
            (head_min.y + head_max.y) * 0.5,
            head_min.z,
        )
    )

    placement = placement_for(spec)
    head_scale = placement["head_height_ratio"] * body_size.z / max(head_size.z, 0.001)
    target = Vector(
        (
            body_center.x + placement["head_x_offset_ratio"] * body_size.z,
            body_center.y + placement["head_y_offset_ratio"] * body_size.z,
            body_max.z - placement["neck_overlap_ratio"] * body_size.z,
        )
    )
    transform = Matrix.Translation(target - head_anchor * head_scale) @ Matrix.Scale(head_scale, 4)
    apply_group_transform(head_imported, transform)

    add_lights()
    export_path = export_dir / f"{spec.output_stem}.glb"
    export_glb(export_path)

    combined_meshes = [obj for obj in bpy.context.scene.objects if obj.type == "MESH"]
    combined_min, combined_max, combined_size = object_bounds(combined_meshes)
    render_outputs = render_views(spec.output_stem, render_dir, resolution)

    blend_dir.mkdir(parents=True, exist_ok=True)
    blend_path = blend_dir / f"{spec.output_stem}.blend"
    bpy.ops.wm.save_as_mainfile(filepath=str(blend_path))

    return {
        "output_stem": spec.output_stem,
        "hero_id": spec.hero_id,
        "name": spec.name,
        "variant": spec.variant,
        "body_glb": str(spec.body_glb),
        "head_glb": str(spec.head_glb),
        "export": str(export_path),
        "blend": str(blend_path),
        "renders": render_outputs,
        "placement": placement,
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


def manifest_entries():
    manifest = json.loads((BATCH_ROOT / "batch_manifest.json").read_text(encoding="utf-8"))
    return manifest["entries"]


def rel_to_batch(path_value):
    return BATCH_ROOT / Path(path_value)


def build_pair_specs():
    entries = manifest_entries()
    by_key = {}
    for entry in entries:
        by_key[(entry["hero_id"], entry["name"], entry["variant"], entry["part"])] = rel_to_batch(entry["trellis_output"])

    hero_order = []
    seen = set()
    for entry in entries:
        key = (entry["hero_id"], entry["name"])
        if key not in seen:
            seen.add(key)
            hero_order.append(key)

    specs = [
        PairSpec(
            hero_id="Hero_1",
            name="Arthur",
            variant="standard",
            body_glb=ARTHUR_RAW / "Hero_1_Arthur_05_BodyOnly_Green_S1337_D80000_Trellis2.glb",
            head_glb=ARTHUR_RAW / "Hero_1_Arthur_04_HeadOnly_Green_S1337_D120000_Trellis2.glb",
            output_stem="Hero_1_Arthur_TypeA_Standard_HeadBody_Assembled",
        )
    ]

    for hero_id, name in hero_order:
        if hero_id == "Hero_1":
            head = ARTHUR_RAW / "Hero_1_Arthur_04_HeadOnly_Green_S1337_D120000_Trellis2.glb"
            beach_body = by_key.get((hero_id, name, "beach_goer", "body"))
            if beach_body:
                specs.append(
                    PairSpec(
                        hero_id=hero_id,
                        name=name,
                        variant="beach_goer",
                        body_glb=beach_body,
                        head_glb=head,
                        output_stem="Hero_1_Arthur_TypeA_BeachGoer_HeadBody_Assembled",
                    )
                )
            continue

        standard_body = by_key.get((hero_id, name, "standard", "body"))
        standard_head = by_key.get((hero_id, name, "standard", "head"))
        beach_body = by_key.get((hero_id, name, "beach_goer", "body"))
        safe_name = name.replace(" ", "")
        if standard_body and standard_head:
            specs.append(
                PairSpec(
                    hero_id=hero_id,
                    name=name,
                    variant="standard",
                    body_glb=standard_body,
                    head_glb=standard_head,
                    output_stem=f"{hero_id}_{safe_name}_TypeA_Standard_HeadBody_Assembled",
                )
            )
        if beach_body and standard_head:
            specs.append(
                PairSpec(
                    hero_id=hero_id,
                    name=name,
                    variant="beach_goer",
                    body_glb=beach_body,
                    head_glb=standard_head,
                    output_stem=f"{hero_id}_{safe_name}_TypeA_BeachGoer_HeadBody_Assembled",
                )
            )
    return specs


def filter_specs(specs, args):
    if args.clean_tests:
        names = {"Lu Bu", "Mike", "George", "Robo"}
        specs = [spec for spec in specs if spec.name in names and spec.variant == "standard"]
    elif args.hero:
        wanted = {value.lower() for value in args.hero}
        specs = [
            spec
            for spec in specs
            if spec.name.lower() in wanted or spec.hero_id.lower() in wanted or spec.output_stem.lower() in wanted
        ]
    elif not args.all:
        specs = [spec for spec in specs if spec.name in {"Lu Bu", "Mike", "George", "Robo"} and spec.variant == "standard"]

    if args.variant != "both":
        specs = [spec for spec in specs if spec.variant == args.variant]
    return specs


def main():
    args = parse_args()
    specs = filter_specs(build_pair_specs(), args)
    if not specs:
        raise SystemExit("No head/body pairs matched the requested filter.")

    export_dir = BATCH_ROOT / "Assembly" / "HeadBody"
    render_dir = BATCH_ROOT / "Renders" / "HeadBodyAssembly"
    blend_dir = MODEL_GEN_ROOT / "Scenes" / "TypeA_Batch01_HeadBodyAssembly"
    report_path = BATCH_ROOT / "Assembly" / "head_body_assembly_report.json"
    export_dir.mkdir(parents=True, exist_ok=True)
    render_dir.mkdir(parents=True, exist_ok=True)
    blend_dir.mkdir(parents=True, exist_ok=True)
    report_path.parent.mkdir(parents=True, exist_ok=True)

    reports = []
    for spec in specs:
        print(f"ASSEMBLE {spec.hero_id} {spec.name} {spec.variant}")
        reports.append(assemble_pair(spec, export_dir, render_dir, blend_dir, args.resolution))

    report_path.write_text(json.dumps({"pairs": reports}, indent=2), encoding="utf-8")
    print(f"WROTE {report_path}")


if __name__ == "__main__":
    main()
