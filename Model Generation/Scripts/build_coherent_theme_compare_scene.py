import argparse
import json
import math
import os

import bpy
from mathutils import Vector


REPO_ROOT = r"C:\UE\T66"
MODEL_ROOT = os.path.join(REPO_ROOT, "Model Generation")
TRELLIS_DIR = os.path.join(
    MODEL_ROOT, "Runs", "Environment", "CoherentThemeKit01", "Raw", "Trellis"
)
TRELLIS_FLOOR_FIX_DIR = os.path.join(
    MODEL_ROOT, "Runs", "Environment", "CoherentThemeKit01", "Raw", "TrellisFloorFix01"
)
TRELLIS_FLOOR_FIX02_DIR = os.path.join(
    MODEL_ROOT, "Runs", "Environment", "CoherentThemeKit01", "Raw", "TrellisFloorFix02"
)
TRELLIS_FLOOR_FIX03_DIR = os.path.join(
    MODEL_ROOT, "Runs", "Environment", "CoherentThemeKit01", "Raw", "TrellisFloorFix03"
)
ARTHUR_PATH = os.path.join(
    MODEL_ROOT,
    "Runs",
    "Arthur",
    "Exports",
    "Runtime",
    "ArthurAIdle_Hero_1_TypeA_RuntimeMesh.glb",
)
ARTHUR_GAME_RUNTIME_SCALE = 1.7
ARTHUR_REVIEW_SCALE = 1.0
ARTHUR_RUNTIME_YAW_DEGREES = -90.0
WALL_REVIEW_TARGET_HEIGHT_METERS = 3.5
WALL_REVIEW_TARGET_THICKNESS_METERS = 1.2
FLOOR_REVIEW_TARGET_FOOTPRINT_METERS = 3.5

THEMES = [
    (
        "Easy / Dungeon",
        "Dungeon",
        [
            "DungeonWall_TorchSconce_A",
            "DungeonWall_StoneBlocks_A",
            "DungeonWall_Chains_A",
            "DungeonWall_BonesNiche_A",
            "DungeonFloor_StoneSlabs_A",
            "DungeonFloor_Drain_A",
            "DungeonFloor_Cracked_A",
            "DungeonFloor_Bones_A",
        ],
    ),
    (
        "Medium / Forest",
        "Forest",
        [
            "ForestWall_VineTotem_A",
            "ForestWall_TrunkWeave_A",
            "ForestWall_RootBraid_A",
            "ForestWall_MushroomBark_A",
            "ForestFloor_RootMat_A",
            "ForestFloor_MossStone_A",
            "ForestFloor_LeafCrack_A",
            "ForestFloor_BrambleEdge_A",
        ],
    ),
    (
        "Hard / Ocean",
        "Ocean",
        [
            "OceanWall_CoralReef_A",
            "OceanWall_ShellLimestone_A",
            "OceanWall_KelpCoral_A",
            "OceanWall_ReefRuin_A",
            "OceanFloor_ReefStone_A",
            "OceanFloor_ShellSand_A",
            "OceanFloor_CoralCrack_A",
            "OceanFloor_TidePool_A",
        ],
    ),
    (
        "VeryHard / Martian",
        "Martian",
        [
            "MartianWall_RuinPanel_A",
            "MartianWall_RedRock_A",
            "MartianWall_MeteorScar_A",
            "MartianWall_CrystalVein_A",
            "MartianFloor_RuinTile_A",
            "MartianFloor_RegolithPlates_A",
            "MartianFloor_CrystalDust_A",
            "MartianFloor_CraterCracks_A",
        ],
    ),
    (
        "Impossible / Hell",
        "Hell",
        [
            "HellWall_SpikeBasalt_A",
            "HellWall_LavaCrack_A",
            "HellWall_ChainsSkulls_A",
            "HellWall_Brimstone_A",
            "HellFloor_RunePlate_A",
            "HellFloor_Obsidian_A",
            "HellFloor_EmberFissure_A",
            "HellFloor_BoneAsh_A",
        ],
    ),
]


def parse_args():
    argv = []
    if "--" in os.sys.argv:
        argv = os.sys.argv[os.sys.argv.index("--") + 1 :]

    parser = argparse.ArgumentParser()
    parser.add_argument("--output-blend", required=True)
    parser.add_argument("--report", required=True)
    parser.add_argument("--render")
    parser.add_argument("--resolution", type=int, default=2600)
    return parser.parse_args(argv)


def reset_scene():
    bpy.ops.wm.read_homefile(use_empty=True, use_factory_startup=True)
    scene = bpy.context.scene
    scene.render.engine = "BLENDER_EEVEE"
    scene.render.film_transparent = False
    scene.render.resolution_x = 2600
    scene.render.resolution_y = 1500
    scene.render.image_settings.file_format = "PNG"
    if hasattr(scene.eevee, "taa_render_samples"):
        scene.eevee.taa_render_samples = 32
    if hasattr(scene.eevee, "use_gtao"):
        scene.eevee.use_gtao = True
        scene.eevee.gtao_distance = 3.0
        scene.eevee.gtao_factor = 1.1

    scene.world = bpy.data.worlds.new("CoherentThemeKitCompareWorld")
    scene.world.use_nodes = True
    bg = scene.world.node_tree.nodes["Background"]
    bg.inputs[0].default_value = (0.045, 0.048, 0.052, 1.0)
    bg.inputs[1].default_value = 0.7
    return scene


def ensure_parent(path):
    os.makedirs(os.path.dirname(path), exist_ok=True)


def gltf_import_override():
    wm = bpy.context.window_manager
    for window in wm.windows:
        screen = window.screen
        for area in screen.areas:
            if area.type != "VIEW_3D":
                continue
            region = next((r for r in area.regions if r.type == "WINDOW"), None)
            if region is None:
                continue
            return {
                "window": window,
                "screen": screen,
                "area": area,
                "region": region,
                "scene": bpy.context.scene,
                "view_layer": bpy.context.view_layer,
            }
    return None


def import_glb(path):
    before = set(bpy.data.objects)
    override = gltf_import_override()
    if override:
        with bpy.context.temp_override(**override):
            bpy.ops.import_scene.gltf(filepath=path)
    else:
        bpy.ops.import_scene.gltf(filepath=path)
    imported = [obj for obj in bpy.data.objects if obj not in before]
    meshes = [obj for obj in imported if obj.type == "MESH"]
    if not meshes:
        raise RuntimeError(f"No mesh objects imported from {path}")
    return imported, meshes


def world_bbox(meshes):
    mins = Vector((math.inf, math.inf, math.inf))
    maxs = Vector((-math.inf, -math.inf, -math.inf))
    for obj in meshes:
        for corner in obj.bound_box:
            world = obj.matrix_world @ Vector(corner)
            mins.x = min(mins.x, world.x)
            mins.y = min(mins.y, world.y)
            mins.z = min(mins.z, world.z)
            maxs.x = max(maxs.x, world.x)
            maxs.y = max(maxs.y, world.y)
            maxs.z = max(maxs.z, world.z)
    return mins, maxs


def create_root(name, imported):
    root = bpy.data.objects.new(name, None)
    bpy.context.collection.objects.link(root)
    for obj in imported:
        if obj.parent not in imported:
            obj.parent = root
    return root


def arthur_body_meshes(meshes):
    body_meshes = [obj for obj in meshes if obj.material_slots]
    return body_meshes or meshes


def configure_arthur_visibility(root, visible_meshes):
    visible_mesh_set = set(visible_meshes)
    root.hide_viewport = False
    root.hide_render = True
    for obj in root.children_recursive:
        if obj.type == "MESH":
            show_mesh = obj in visible_mesh_set
            obj.hide_viewport = not show_mesh
            obj.hide_render = not show_mesh
        elif obj.type == "ARMATURE":
            obj.hide_viewport = True
            obj.hide_render = True
            obj.display_type = "WIRE"
            obj.show_in_front = False


def move_root_to(root, meshes, target_x, target_y):
    mins, maxs = world_bbox(meshes)
    center = (mins + maxs) * 0.5
    delta = Vector((target_x - center.x, target_y - center.y, -mins.z))
    root.location += delta
    bpy.context.view_layer.update()


def apply_arthur_runtime_transform(root):
    root.rotation_euler = (0.0, 0.0, math.radians(ARTHUR_RUNTIME_YAW_DEGREES))
    root.scale = (ARTHUR_REVIEW_SCALE, ARTHUR_REVIEW_SCALE, ARTHUR_REVIEW_SCALE)
    bpy.context.view_layer.update()


def apply_wall_review_transform(module_id, root, meshes):
    if "Wall" not in module_id:
        return None

    mins, maxs = world_bbox(meshes)
    source_height = maxs.z - mins.z
    if source_height <= 0.001:
        return None

    uniform_scale = WALL_REVIEW_TARGET_HEIGHT_METERS / source_height
    root.scale *= uniform_scale
    bpy.context.view_layer.update()

    mins, maxs = world_bbox(meshes)
    scaled_size = maxs - mins
    if scaled_size.x >= scaled_size.y:
        thickness_axis = "Y"
        source_thickness = scaled_size.y
        root.scale.y *= WALL_REVIEW_TARGET_THICKNESS_METERS / max(source_thickness, 0.001)
        applied_rotation_degrees = 0.0
    else:
        thickness_axis = "X"
        source_thickness = scaled_size.x
        root.scale.x *= WALL_REVIEW_TARGET_THICKNESS_METERS / max(source_thickness, 0.001)
        root.rotation_euler.z += math.radians(90.0)
        applied_rotation_degrees = 90.0
    bpy.context.view_layer.update()

    return {
        "target_height_meters": WALL_REVIEW_TARGET_HEIGHT_METERS,
        "target_thickness_meters": WALL_REVIEW_TARGET_THICKNESS_METERS,
        "source_height_meters": round(source_height, 4),
        "uniform_scale": round(uniform_scale, 4),
        "thickness_axis": thickness_axis,
        "source_scaled_thickness_meters": round(source_thickness, 4),
        "applied_rotation_degrees": applied_rotation_degrees,
    }


def apply_floor_review_transform(module_id, root, meshes):
    if "Floor" not in module_id:
        return None

    mins, maxs = world_bbox(meshes)
    source_size = maxs - mins
    source_footprint = max(source_size.x, source_size.y)
    if source_footprint <= 0.001:
        return None

    uniform_scale = FLOOR_REVIEW_TARGET_FOOTPRINT_METERS / source_footprint
    root.scale *= uniform_scale
    bpy.context.view_layer.update()
    return {
        "target_footprint_meters": FLOOR_REVIEW_TARGET_FOOTPRINT_METERS,
        "source_footprint_meters": round(source_footprint, 4),
        "uniform_scale": round(uniform_scale, 4),
    }


def object_stats(meshes):
    mins, maxs = world_bbox(meshes)
    size = maxs - mins
    tris = 0
    for obj in meshes:
        tris += sum(len(poly.vertices) - 2 for poly in obj.data.polygons)
    return {
        "min": [round(mins.x, 4), round(mins.y, 4), round(mins.z, 4)],
        "max": [round(maxs.x, 4), round(maxs.y, 4), round(maxs.z, 4)],
        "size": [round(size.x, 4), round(size.y, 4), round(size.z, 4)],
        "triangles": tris,
        "mesh_count": len(meshes),
    }


def add_material(name, color, roughness=0.95):
    mat = bpy.data.materials.new(name)
    mat.use_nodes = True
    bsdf = mat.node_tree.nodes.get("Principled BSDF")
    bsdf.inputs["Base Color"].default_value = color
    bsdf.inputs["Roughness"].default_value = roughness
    return mat


def add_label(text, location, size, mat):
    curve = bpy.data.curves.new(f"Label_{text}", type="FONT")
    curve.body = text
    curve.align_x = "CENTER"
    curve.align_y = "CENTER"
    curve.size = size
    obj = bpy.data.objects.new(f"Label_{text}", curve)
    obj.location = location
    obj.rotation_euler = (math.radians(65), 0.0, 0.0)
    obj.data.materials.append(mat)
    bpy.context.collection.objects.link(obj)
    return obj


def add_ground_row(center_x, center_y, width, depth, mat):
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(center_x, center_y, -0.02))
    obj = bpy.context.view_layer.objects.active
    obj.name = "Row_Ground_Plate"
    obj.dimensions = (width, depth, 0.04)
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    obj.data.materials.append(mat)
    return obj


def add_height_ruler(x, y, height, mat, label_mat):
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(x, y, height * 0.5))
    ruler = bpy.context.view_layer.objects.active
    ruler.name = "Arthur_Height_Ruler"
    ruler.dimensions = (0.035, 0.035, height)
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    ruler.data.materials.append(mat)

    add_label(
        f"Arthur height {height:.2f}",
        (x, y - 0.28, max(height + 0.18, 0.4)),
        max(height * 0.055, 0.08),
        label_mat,
    )


def duplicate_group(source_root, source_meshes, name, target_x, target_y):
    duplicates = []
    mesh_map = []
    source_mesh_set = set(source_meshes)
    for obj in [source_root] + list(source_root.children_recursive):
        copy = obj.copy()
        copy.name = obj.name.replace("Arthur_Master", name)
        if obj.data:
            copy.data = obj.data
        bpy.context.collection.objects.link(copy)
        duplicates.append((obj, copy))
        if copy.type == "MESH" and obj in source_mesh_set:
            mesh_map.append(copy)

    by_original = {original: copy for original, copy in duplicates}
    for original, copy in duplicates:
        if original.parent:
            copy.parent = by_original.get(original.parent)
        copy.matrix_parent_inverse = original.matrix_parent_inverse.copy()
        for modifier in copy.modifiers:
            if hasattr(modifier, "object") and modifier.object in by_original:
                modifier.object = by_original[modifier.object]

    root_copy = by_original[source_root]
    move_root_to(root_copy, mesh_map, target_x, target_y)
    return root_copy, mesh_map


def add_lights():
    sun_data = bpy.data.lights.new("CompareSun", "SUN")
    sun_data.energy = 2.4
    sun = bpy.data.objects.new("CompareSun", sun_data)
    sun.rotation_euler = (math.radians(48), 0.0, math.radians(32))
    bpy.context.collection.objects.link(sun)

    fill_data = bpy.data.lights.new("CompareFill", "AREA")
    fill_data.energy = 1500
    fill_data.size = 28.0
    fill = bpy.data.objects.new("CompareFill", fill_data)
    fill.location = (4.0, -16.0, 15.0)
    fill.rotation_euler = (math.radians(65), 0.0, math.radians(12))
    bpy.context.collection.objects.link(fill)


def add_camera(center_x, center_y, width, depth, aspect):
    cam_data = bpy.data.cameras.new("CompareCamera")
    cam_data.type = "ORTHO"
    cam_data.ortho_scale = max(depth * 1.18, width / aspect * 1.16, 8.0)
    cam = bpy.data.objects.new("CompareCamera", cam_data)
    cam.location = (center_x, center_y - depth * 0.78, max(depth * 0.42, 7.0))
    bpy.context.collection.objects.link(cam)
    bpy.context.scene.camera = cam

    target = bpy.data.objects.new("CompareCameraTarget", None)
    target.location = (center_x, center_y, 0.9)
    bpy.context.collection.objects.link(target)

    track = cam.constraints.new(type="TRACK_TO")
    track.target = target
    track.track_axis = "TRACK_NEGATIVE_Z"
    track.up_axis = "UP_Y"
    return cam


def visible_mesh_bounds():
    meshes = [
        obj
        for obj in bpy.data.objects
        if obj.type == "MESH" and not obj.hide_viewport and not obj.hide_render
    ]
    return world_bbox(meshes)


def asset_path(module_id):
    if "Floor" in module_id:
        for directory, suffix in [
            (TRELLIS_FLOOR_FIX03_DIR, "FloorFix03"),
            (TRELLIS_FLOOR_FIX02_DIR, "FloorFix02"),
            (TRELLIS_FLOOR_FIX_DIR, "FloorFix01"),
        ]:
            fixed_path = os.path.join(
                directory, f"{module_id}_{suffix}_S1337_D80000_Trellis2.glb"
            )
            if os.path.exists(fixed_path):
                return fixed_path
    return os.path.join(TRELLIS_DIR, f"{module_id}_S1337_D80000_Trellis2.glb")


def main():
    args = parse_args()
    scene = reset_scene()
    scene.render.resolution_x = args.resolution
    scene.render.resolution_y = int(args.resolution * 0.577)
    aspect = scene.render.resolution_x / scene.render.resolution_y

    label_mat = add_material("Readable_Label_Material", (0.92, 0.94, 0.92, 1.0))
    row_mat = add_material("Row_Ground_Material", (0.13, 0.14, 0.15, 1.0))
    ruler_mat = add_material("Arthur_Height_Ruler_Material", (0.95, 0.22, 0.18, 1.0))
    arthur_imported, arthur_meshes = import_glb(ARTHUR_PATH)
    arthur_root = create_root("Arthur_Master_Root", arthur_imported)
    for obj in arthur_imported:
        obj.name = f"Arthur_Master_{obj.name}"
    arthur_meshes = arthur_body_meshes(arthur_meshes)
    apply_arthur_runtime_transform(arthur_root)
    configure_arthur_visibility(arthur_root, arthur_meshes)
    move_root_to(arthur_root, arthur_meshes, 0.0, 0.0)
    arthur_stats = object_stats(arthur_meshes)
    arthur_height = arthur_stats["size"][2]

    max_size = max(arthur_stats["size"])
    rows = []
    assets = []
    for theme_label, theme_id, modules in THEMES:
        row = {"theme_label": theme_label, "theme_id": theme_id, "assets": []}
        for module_id in modules:
            path = asset_path(module_id)
            if not os.path.exists(path):
                raise FileNotFoundError(path)
            imported, meshes = import_glb(path)
            root = create_root(f"{module_id}_Root", imported)
            review_transform = apply_wall_review_transform(module_id, root, meshes)
            if review_transform is None:
                review_transform = apply_floor_review_transform(module_id, root, meshes)
            stats = object_stats(meshes)
            max_size = max(max_size, stats["size"][0], stats["size"][1], stats["size"][2])
            row["assets"].append(
                {
                    "module_id": module_id,
                    "path": path,
                    "root": root,
                    "meshes": meshes,
                    "stats": stats,
                    "review_transform": review_transform,
                }
            )
            assets.append((root, meshes))
        rows.append(row)

    cell_x = max(max_size * 1.45, 2.4)
    row_y = max(max_size * 1.8, 3.1)
    hero_x = cell_x * 1.45
    first_asset_x = cell_x * 3.05
    label_size = max(max_size * 0.055, 0.095)
    row_depth = max(max_size * 1.55, 2.4)
    row_width = first_asset_x + cell_x * 8.45

    report = {
        "scene": args.output_blend,
        "arthur_reference": {
            "path": ARTHUR_PATH,
            "runtime_visual_id": "Hero_1_TypeA",
            "runtime_skeletal_mesh": "/Game/Characters/Heroes/Hero_1/TypeA/Idle/ArthurAIdle.ArthurAIdle",
            "game_runtime_mesh_relative_scale": [ARTHUR_GAME_RUNTIME_SCALE] * 3,
            "blender_review_mesh_scale": [ARTHUR_REVIEW_SCALE] * 3,
            "runtime_mesh_relative_yaw_degrees": ARTHUR_RUNTIME_YAW_DEGREES,
            **arthur_stats,
        },
        "review_targets": {
            "arthur_height_meters": 2.0,
            "wall_height_meters": WALL_REVIEW_TARGET_HEIGHT_METERS,
            "wall_thickness_meters": WALL_REVIEW_TARGET_THICKNESS_METERS,
            "floor_footprint_meters": FLOOR_REVIEW_TARGET_FOOTPRINT_METERS,
        },
        "assets": [],
    }

    # Hide the master import after making linked row copies.
    arthur_root.hide_viewport = True
    arthur_root.hide_render = True
    for obj in arthur_root.children_recursive:
        obj.hide_viewport = True
        obj.hide_render = True

    for row_index, row in enumerate(rows):
        y = -row_index * row_y
        row_center_x = row_width * 0.5 - cell_x * 0.25
        add_ground_row(row_center_x, y, row_width, row_depth, row_mat)
        add_label(
            row["theme_label"],
            (-cell_x * 0.36, y - row_depth * 0.34, max(arthur_height * 1.08, 1.0)),
            label_size * 1.32,
            label_mat,
        )
        arthur_copy_root, arthur_copy_meshes = duplicate_group(
            arthur_root, arthur_meshes, f"Arthur_{row['theme_id']}", hero_x, y
        )
        configure_arthur_visibility(arthur_copy_root, arthur_copy_meshes)
        add_height_ruler(hero_x + max(max_size * 0.35, 0.45), y, arthur_height, ruler_mat, label_mat)
        add_label("Arthur 2m ref", (hero_x, y + row_depth * 0.36, 0.16), label_size, label_mat)

        for col_index, asset in enumerate(row["assets"]):
            x = first_asset_x + col_index * cell_x
            move_root_to(asset["root"], asset["meshes"], x, y)
            module_id = asset["module_id"]
            label = module_id.replace("_A", "").replace("_", " ")
            add_label(label, (x, y + row_depth * 0.36, 0.16), label_size, label_mat)
            asset_report = {
                "theme": row["theme_label"],
                "module_id": module_id,
                "path": asset["path"],
                "review_transform": asset["review_transform"],
                **object_stats(asset["meshes"]),
            }
            report["assets"].append(asset_report)

    add_lights()
    mins, maxs = visible_mesh_bounds()
    pad = max_size * 0.95
    center_x = (mins.x + maxs.x) * 0.5
    center_y = (mins.y + maxs.y) * 0.5
    view_width = (maxs.x - mins.x) + pad * 2.0
    view_depth = (maxs.y - mins.y) + pad * 2.0
    add_camera(center_x, center_y, view_width, view_depth, aspect)

    ensure_parent(args.output_blend)
    bpy.ops.wm.save_as_mainfile(filepath=args.output_blend)

    ensure_parent(args.report)
    with open(args.report, "w", encoding="utf-8") as handle:
        json.dump(report, handle, indent=2)

    if args.render:
        ensure_parent(args.render)
        scene.render.filepath = args.render
        bpy.ops.render.render(write_still=True)


if __name__ == "__main__":
    main()
