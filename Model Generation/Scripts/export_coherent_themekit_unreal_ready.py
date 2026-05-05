import argparse
import json
import math
import os
from pathlib import Path

import bpy
from mathutils import Vector


REPO_ROOT = Path(r"C:\UE\T66")
RUN_ROOT = REPO_ROOT / "Model Generation" / "Runs" / "Environment" / "CoherentThemeKit01"
OUTPUT_ROOT = REPO_ROOT / "SourceAssets" / "Import" / "WorldKit" / "CoherentThemeKit01"
TEXTURE_ROOT = OUTPUT_ROOT / "Textures"
NOTES_ROOT = RUN_ROOT / "Notes" / "unreal_ready"

TRELLIS_DIR = RUN_ROOT / "Raw" / "Trellis"
FLOOR_FIX_DIRS = [
    (RUN_ROOT / "Raw" / "TrellisFloorFix03", "FloorFix03"),
    (RUN_ROOT / "Raw" / "TrellisFloorFix02", "FloorFix02"),
    (RUN_ROOT / "Raw" / "TrellisFloorFix01", "FloorFix01"),
]

WALL_HEIGHT_M = 6.0
WALL_DEPTH_M = 1.2
WALL_FACE_WIDTH_M = 6.0
FLOOR_TILE_M = 6.0
FLOOR_THICKNESS_M = 0.24

THEMES = {
    "Dungeon": {
        "walls": [
            "DungeonWall_TorchSconce_A",
            "DungeonWall_StoneBlocks_A",
            "DungeonWall_Chains_A",
            "DungeonWall_BonesNiche_A",
        ],
        "floors": [
            "DungeonFloor_StoneSlabs_A",
            "DungeonFloor_Drain_A",
            "DungeonFloor_Cracked_A",
            "DungeonFloor_Bones_A",
        ],
    },
    "Forest": {
        "walls": [
            "ForestWall_VineTotem_A",
            "ForestWall_TrunkWeave_A",
            "ForestWall_RootBraid_A",
            "ForestWall_MushroomBark_A",
        ],
        "floors": [
            "ForestFloor_RootMat_A",
            "ForestFloor_MossStone_A",
            "ForestFloor_LeafCrack_A",
            "ForestFloor_BrambleEdge_A",
        ],
    },
    "Ocean": {
        "walls": [
            "OceanWall_CoralReef_A",
            "OceanWall_ShellLimestone_A",
            "OceanWall_KelpCoral_A",
            "OceanWall_ReefRuin_A",
        ],
        "floors": [
            "OceanFloor_ReefStone_A",
            "OceanFloor_ShellSand_A",
            "OceanFloor_CoralCrack_A",
            "OceanFloor_TidePool_A",
        ],
    },
    "Martian": {
        "walls": [
            "MartianWall_RuinPanel_A",
            "MartianWall_RedRock_A",
            "MartianWall_MeteorScar_A",
            "MartianWall_CrystalVein_A",
        ],
        "floors": [
            "MartianFloor_RuinTile_A",
            "MartianFloor_RegolithPlates_A",
            "MartianFloor_CrystalDust_A",
            "MartianFloor_CraterCracks_A",
        ],
    },
    "Hell": {
        "walls": [
            "HellWall_SpikeBasalt_A",
            "HellWall_LavaCrack_A",
            "HellWall_ChainsSkulls_A",
            "HellWall_Brimstone_A",
        ],
        "floors": [
            "HellFloor_RunePlate_A",
            "HellFloor_Obsidian_A",
            "HellFloor_EmberFissure_A",
            "HellFloor_BoneAsh_A",
        ],
    },
}


def parse_args():
    argv = []
    if "--" in os.sys.argv:
        argv = os.sys.argv[os.sys.argv.index("--") + 1 :]

    parser = argparse.ArgumentParser()
    parser.add_argument("--output-root", default=str(OUTPUT_ROOT))
    parser.add_argument("--texture-root", default=str(TEXTURE_ROOT))
    parser.add_argument("--notes-root", default=str(NOTES_ROOT))
    return parser.parse_args(argv)


def reset_scene():
    bpy.ops.wm.read_factory_settings(use_empty=True)


def source_path(module_id):
    if "Floor" in module_id:
        for directory, suffix in FLOOR_FIX_DIRS:
            candidate = directory / f"{module_id}_{suffix}_S1337_D80000_Trellis2.glb"
            if candidate.exists():
                return candidate
    return TRELLIS_DIR / f"{module_id}_S1337_D80000_Trellis2.glb"


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


def axis_value(co, axis):
    return co.x if axis == "x" else co.y


def normalize_wall(obj):
    source_min, source_max = local_bbox(obj)
    source_size = source_max - source_min
    if source_size.z <= 0.001 or source_size.x <= 0.001 or source_size.y <= 0.001:
        raise RuntimeError(f"Invalid wall bounds for {obj.name}: {source_size}")

    length_axis = "x" if source_size.x >= source_size.y else "y"
    depth_axis = "y" if length_axis == "x" else "x"
    length_min = axis_value(source_min, length_axis)
    length_max = axis_value(source_max, length_axis)
    depth_min = axis_value(source_min, depth_axis)
    depth_max = axis_value(source_max, depth_axis)
    height_size = source_size.z
    length_size = length_max - length_min
    depth_size = depth_max - depth_min

    for vertex in obj.data.vertices:
        co = vertex.co
        length_norm = (axis_value(co, length_axis) - length_min) / max(length_size, 0.001)
        depth_norm = (axis_value(co, depth_axis) - depth_min) / max(depth_size, 0.001)
        height_norm = (co.z - source_min.z) / max(height_size, 0.001)
        vertex.co = Vector(
            (
                (depth_norm - 0.5) * WALL_DEPTH_M,
                (length_norm - 0.5) * WALL_FACE_WIDTH_M,
                height_norm * WALL_HEIGHT_M,
            )
        )
    obj.data.update()

    return {
        "kind": "wall",
        "source_size_m": [source_size.x, source_size.y, source_size.z],
        "length_axis": length_axis.upper(),
        "depth_axis": depth_axis.upper(),
        "target_face_width_m": WALL_FACE_WIDTH_M,
        "target_depth_m": WALL_DEPTH_M,
        "target_height_m": WALL_HEIGHT_M,
        "source_face_aspect": length_size / max(height_size, 0.001),
        "runtime_axes": "X=wall thickness, Y=wall length, Z=wall height",
    }


def normalize_floor(obj):
    source_min, source_max = local_bbox(obj)
    source_size = source_max - source_min
    if source_size.z <= 0.001 or source_size.x <= 0.001 or source_size.y <= 0.001:
        raise RuntimeError(f"Invalid floor bounds for {obj.name}: {source_size}")

    for vertex in obj.data.vertices:
        co = vertex.co
        x_norm = (co.x - source_min.x) / max(source_size.x, 0.001)
        y_norm = (co.y - source_min.y) / max(source_size.y, 0.001)
        z_norm = (co.z - source_min.z) / max(source_size.z, 0.001)
        vertex.co = Vector(
            (
                (x_norm - 0.5) * FLOOR_TILE_M,
                (y_norm - 0.5) * FLOOR_TILE_M,
                z_norm * FLOOR_THICKNESS_M,
            )
        )
    obj.data.update()

    return {
        "kind": "floor",
        "source_size_m": [source_size.x, source_size.y, source_size.z],
        "target_footprint_m": FLOOR_TILE_M,
        "target_thickness_m": FLOOR_THICKNESS_M,
        "runtime_axes": "X/Y=tile footprint, Z=visual thickness",
    }


def triangle_count(obj):
    obj.data.calc_loop_triangles()
    return len(obj.data.loop_triangles)


def export_selected_glb(obj, path):
    path.parent.mkdir(parents=True, exist_ok=True)
    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    bpy.ops.export_scene.gltf(
        filepath=str(path),
        export_format="GLB",
        use_selection=True,
        export_apply=True,
        export_materials="EXPORT",
    )


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


def first_image_texture(obj):
    for slot in obj.material_slots:
        material = slot.material
        if not material or not material.use_nodes:
            continue
        for node in material.node_tree.nodes:
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


def normalize_asset(module_id, kind, output_root, texture_root):
    reset_scene()
    source = source_path(module_id)
    if not source.exists():
        raise FileNotFoundError(source)

    meshes = import_glb(source)
    detach_keep_world(meshes)
    apply_object_transforms(meshes)
    obj = join_meshes(meshes, f"{module_id}_UnrealReady")

    if kind == "wall":
        transform_report = normalize_wall(obj)
    else:
        transform_report = normalize_floor(obj)

    final_min, final_max = local_bbox(obj)
    final_size = final_max - final_min
    export_path = output_root / f"{module_id}_UnrealReady.glb"
    fbx_export_path = output_root / f"{module_id}_UnrealReady.fbx"
    texture_path = texture_root / f"{module_id}_UnrealReady_BaseColor_00.png"
    export_selected_glb(obj, export_path)
    export_selected_fbx(obj, fbx_export_path)
    saved_texture_path = save_base_color_texture(obj, texture_path)

    return {
        "module_id": module_id,
        "kind": kind,
        "source": str(source),
        "export": str(export_path),
        "fbx_export": str(fbx_export_path),
        "base_color_texture": saved_texture_path,
        "triangles": triangle_count(obj),
        "final_size_m": [final_size.x, final_size.y, final_size.z],
        **transform_report,
    }


def main():
    args = parse_args()
    output_root = Path(args.output_root)
    texture_root = Path(args.texture_root)
    notes_root = Path(args.notes_root)
    notes_root.mkdir(parents=True, exist_ok=True)

    manifest = {
        "output_root": str(output_root),
        "texture_root": str(texture_root),
        "wall_height_cm": WALL_HEIGHT_M * 100.0,
        "wall_depth_cm": WALL_DEPTH_M * 100.0,
        "wall_face_width_cm": WALL_FACE_WIDTH_M * 100.0,
        "floor_tile_cm": FLOOR_TILE_M * 100.0,
        "floor_thickness_cm": FLOOR_THICKNESS_M * 100.0,
        "assets": [],
    }

    for theme_name, theme in THEMES.items():
        for module_id in theme["walls"]:
            payload = normalize_asset(module_id, "wall", output_root, texture_root)
            payload["theme"] = theme_name
            manifest["assets"].append(payload)
            print(f"EXPORTED {module_id} -> {payload['export']}")
        for module_id in theme["floors"]:
            payload = normalize_asset(module_id, "floor", output_root, texture_root)
            payload["theme"] = theme_name
            manifest["assets"].append(payload)
            print(f"EXPORTED {module_id} -> {payload['export']}")

    manifest_path = notes_root / "CoherentThemeKit01_UnrealReadyManifest.json"
    with open(manifest_path, "w", encoding="ascii") as handle:
        json.dump(manifest, handle, indent=2)
    print(f"MANIFEST {manifest_path}")


if __name__ == "__main__":
    main()
