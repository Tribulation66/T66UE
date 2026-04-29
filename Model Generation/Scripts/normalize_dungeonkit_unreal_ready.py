import argparse
import json
import math
import os
from dataclasses import dataclass
from pathlib import Path

import bpy
from mathutils import Matrix, Vector


@dataclass(frozen=True)
class ModuleSpec:
    module_id: str
    retopo_source_name: str
    raw_source_name: str
    raw_source_stage: str
    kind: str
    yaw_deg: float
    output_name: str


MODULES = [
    ModuleSpec(
        "DungeonWall_Straight_A",
        "DungeonWall_Straight_A_S1337_Retopo10000.glb",
        "DungeonWall_Straight_A_S1337_D80000_Trellis2.glb",
        "trellis",
        "wall",
        90.0,
        "DungeonWall_Straight_A_UnrealReady.glb",
    ),
    ModuleSpec(
        "DungeonWall_Straight_Chains",
        "DungeonWall_Straight_Chains_S1337_Retopo14000.glb",
        "DungeonWall_Straight_Chains_S1337_D80000_Trellis2.glb",
        "trellis",
        "wall",
        90.0,
        "DungeonWall_Straight_Chains_UnrealReady.glb",
    ),
    ModuleSpec(
        "DungeonWall_Straight_BonesNiche",
        "DungeonWall_Straight_BonesNiche_S1337_Retopo14000.glb",
        "DungeonWall_Straight_BonesNiche_S1337_D80000_Trellis2.glb",
        "trellis",
        "wall",
        90.0,
        "DungeonWall_Straight_BonesNiche_UnrealReady.glb",
    ),
    ModuleSpec(
        "DungeonWall_Doorway_Arch",
        "DungeonWall_Doorway_Arch_Alpha_S1337_Retopo16000.glb",
        "DungeonWall_Doorway_Arch_Alpha_S1337_D80000_Trellis2.glb",
        "trellis_alpha",
        "wall",
        0.0,
        "DungeonWall_Doorway_Arch_UnrealReady.glb",
    ),
    ModuleSpec(
        "DungeonFloor_BonesDrain_A",
        "DungeonFloor_BonesDrain_A_S1337_Retopo6000.glb",
        "DungeonFloor_BonesDrain_A_S1337_D80000_Trellis2.glb",
        "trellis",
        "floor",
        0.0,
        "DungeonFloor_BonesDrain_A_UnrealReady.glb",
    ),
]


def parse_args():
    argv = []
    if "--" in os.sys.argv:
        argv = os.sys.argv[os.sys.argv.index("--") + 1 :]

    parser = argparse.ArgumentParser()
    parser.add_argument("--run-root", required=True)
    parser.add_argument("--cell-size-cm", type=float, default=1300.0)
    parser.add_argument("--wall-depth-cm", type=float, default=120.0)
    parser.add_argument("--wall-height-cm", type=float, default=1200.0)
    parser.add_argument("--floor-thickness-cm", type=float, default=24.0)
    parser.add_argument("--resolution", type=int, default=1200)
    parser.add_argument("--source-stage", choices=("raw", "retopo"), default="raw")
    parser.add_argument("--fbx-output-dir")
    return parser.parse_args(argv)


def cm_to_blender_units(value_cm):
    return value_cm / 100.0


def reset_scene():
    bpy.ops.wm.read_factory_settings(use_empty=True)
    scene = bpy.context.scene
    scene.render.engine = "BLENDER_EEVEE"
    scene.render.film_transparent = False
    scene.render.resolution_x = 1200
    scene.render.resolution_y = 1200
    scene.render.image_settings.file_format = "PNG"
    if hasattr(scene.eevee, "taa_render_samples"):
        scene.eevee.taa_render_samples = 32
    scene.world = bpy.data.worlds.new("UnrealReadyWorld")
    scene.world.use_nodes = True
    bg = scene.world.node_tree.nodes["Background"]
    bg.inputs[0].default_value = (0.82, 0.84, 0.86, 1.0)
    bg.inputs[1].default_value = 0.8
    return scene


def import_glb(path):
    before = set(bpy.data.objects)
    bpy.ops.import_scene.gltf(filepath=str(path))
    imported = [obj for obj in bpy.data.objects if obj not in before]
    meshes = [obj for obj in imported if obj.type == "MESH"]
    if not meshes:
        raise RuntimeError(f"No mesh objects imported from {path}")
    return meshes


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


def target_bounds(spec, args):
    cell = cm_to_blender_units(args.cell_size_cm)
    if spec.kind == "floor":
        thickness = cm_to_blender_units(args.floor_thickness_cm)
        return Vector((-cell * 0.5, -cell * 0.5, 0.0)), Vector((cell * 0.5, cell * 0.5, thickness))

    depth = cm_to_blender_units(args.wall_depth_cm)
    height = cm_to_blender_units(args.wall_height_cm)
    return Vector((0.0, -cell * 0.5, 0.0)), Vector((depth, cell * 0.5, height))


def remap_to_target_bounds(obj, source_min, source_max, target_min, target_max):
    source_size = source_max - source_min
    target_size = target_max - target_min
    if source_size.x <= 0.0 or source_size.y <= 0.0 or source_size.z <= 0.0:
        raise RuntimeError(f"Invalid source bounds for {obj.name}: {source_size}")

    for vertex in obj.data.vertices:
        normalized = Vector(
            (
                (vertex.co.x - source_min.x) / source_size.x,
                (vertex.co.y - source_min.y) / source_size.y,
                (vertex.co.z - source_min.z) / source_size.z,
            )
        )
        vertex.co = Vector(
            (
                target_min.x + (normalized.x * target_size.x),
                target_min.y + (normalized.y * target_size.y),
                target_min.z + (normalized.z * target_size.z),
            )
        )
    obj.data.update()


def triangle_count(obj):
    obj.data.calc_loop_triangles()
    return len(obj.data.loop_triangles)


def add_lights():
    sun_data = bpy.data.lights.new("PreviewSun", "SUN")
    sun_data.energy = 2.4
    sun = bpy.data.objects.new("PreviewSun", sun_data)
    sun.rotation_euler = (math.radians(45), 0.0, math.radians(35))
    bpy.context.collection.objects.link(sun)

    fill_data = bpy.data.lights.new("PreviewFill", "AREA")
    fill_data.energy = 750
    fill_data.size = 8.0
    fill = bpy.data.objects.new("PreviewFill", fill_data)
    fill.location = (10.0, -12.0, 10.0)
    fill.rotation_euler = (math.radians(60), 0.0, math.radians(35))
    bpy.context.collection.objects.link(fill)


def add_camera_for_object(obj, spec):
    mins, maxs = local_bbox(obj)
    center = (mins + maxs) * 0.5
    size = maxs - mins

    cam_data = bpy.data.cameras.new("PreviewCamera")
    cam_data.type = "ORTHO"
    cam = bpy.data.objects.new("PreviewCamera", cam_data)
    bpy.context.collection.objects.link(cam)
    bpy.context.scene.camera = cam

    if spec.kind == "floor":
        cam.location = center + Vector((6.0, -9.0, 12.0))
        cam_data.ortho_scale = max(size.x, size.y) * 1.25
    else:
        cam.location = center + Vector((18.0, -8.0, 8.0))
        cam_data.ortho_scale = max(size.y, size.z) * 1.25

    target = bpy.data.objects.new("PreviewCameraTarget", None)
    target.location = center
    bpy.context.collection.objects.link(target)

    track = cam.constraints.new(type="TRACK_TO")
    track.target = target
    track.track_axis = "TRACK_NEGATIVE_Z"
    track.up_axis = "UP_Y"


def export_selected(path):
    bpy.ops.object.select_all(action="DESELECT")
    for obj in bpy.data.objects:
        if obj.type == "MESH":
            obj.select_set(True)
    bpy.ops.export_scene.gltf(filepath=str(path), export_format="GLB", use_selection=True, export_apply=True)


def export_selected_fbx(path):
    bpy.ops.object.select_all(action="DESELECT")
    for obj in bpy.data.objects:
        if obj.type == "MESH":
            obj.select_set(True)
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


def source_path_for_spec(spec, args, paths):
    if args.source_stage == "retopo":
        return paths["retopo"] / spec.retopo_source_name
    if spec.raw_source_stage == "trellis_alpha":
        return paths["raw_trellis_alpha"] / spec.raw_source_name
    return paths["raw_trellis"] / spec.raw_source_name


def normalize_module(spec, args, paths):
    scene = reset_scene()
    scene.render.resolution_x = args.resolution
    scene.render.resolution_y = args.resolution

    source = source_path_for_spec(spec, args, paths)
    export = paths["unreal_import"] / spec.output_name
    render = paths["unreal_ready_renders"] / f"{Path(spec.output_name).stem}.png"
    metadata = paths["notes"] / f"{Path(spec.output_name).stem}_metadata.json"
    fbx_export = None
    if args.fbx_output_dir:
        fbx_export = Path(args.fbx_output_dir) / f"{Path(spec.output_name).stem}.fbx"

    meshes = import_glb(source)
    apply_object_transforms(meshes)
    obj = join_meshes(meshes, spec.module_id)

    if spec.yaw_deg:
        rotation = Matrix.Rotation(math.radians(spec.yaw_deg), 4, "Z")
        obj.data.transform(rotation)
        obj.data.update()

    source_min, source_max = local_bbox(obj)
    target_min, target_max = target_bounds(spec, args)
    remap_to_target_bounds(obj, source_min, source_max, target_min, target_max)
    final_min, final_max = local_bbox(obj)

    add_lights()
    add_camera_for_object(obj, spec)

    export.parent.mkdir(parents=True, exist_ok=True)
    render.parent.mkdir(parents=True, exist_ok=True)
    metadata.parent.mkdir(parents=True, exist_ok=True)

    export_selected(export)
    if fbx_export:
        fbx_export.parent.mkdir(parents=True, exist_ok=True)
        export_selected_fbx(fbx_export)
    scene.render.filepath = str(render)
    bpy.ops.render.render(write_still=True)

    payload = {
        "module_id": spec.module_id,
        "kind": spec.kind,
        "source_stage": args.source_stage,
        "source": str(source),
        "export": str(export),
        "fbx_export": str(fbx_export) if fbx_export else None,
        "render": str(render),
        "yaw_degrees_applied_before_remap": spec.yaw_deg,
        "target_cm": {
            "cell_size": args.cell_size_cm,
            "wall_depth": args.wall_depth_cm,
            "wall_height": args.wall_height_cm,
            "floor_thickness": args.floor_thickness_cm,
        },
        "source_bounds_before_remap_blender_units": {
            "min": [source_min.x, source_min.y, source_min.z],
            "max": [source_max.x, source_max.y, source_max.z],
            "size": [(source_max - source_min).x, (source_max - source_min).y, (source_max - source_min).z],
        },
        "final_bounds_blender_units": {
            "min": [final_min.x, final_min.y, final_min.z],
            "max": [final_max.x, final_max.y, final_max.z],
            "size": [(final_max - final_min).x, (final_max - final_min).y, (final_max - final_min).z],
        },
        "triangles": triangle_count(obj),
        "pivot_convention": "wall/door back-bottom-center at origin; floor center at origin with visible detail above Z=0",
    }
    with open(metadata, "w", encoding="ascii") as handle:
        json.dump(payload, handle, indent=2)

    print(f"NORMALIZED {spec.module_id} -> {export}")


def main():
    args = parse_args()
    run_root = Path(args.run_root)
    paths = {
        "retopo": run_root / "Retopo",
        "raw_trellis": run_root / "Raw" / "Trellis",
        "raw_trellis_alpha": run_root / "Raw" / "TrellisAlpha",
        "unreal_import": run_root / "UnrealImport",
        "unreal_ready_renders": run_root / "Renders" / "unreal_ready",
        "notes": run_root / "Notes",
    }

    for spec in MODULES:
        normalize_module(spec, args, paths)


if __name__ == "__main__":
    main()
