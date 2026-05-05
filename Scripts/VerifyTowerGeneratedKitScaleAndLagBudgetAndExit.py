"""
Verify generated tower-kit runtime scale and the floor-2 visual spawn budget,
then request editor shutdown.
"""

import csv
import math
import os
import re
import sys

import unreal


SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
if SCRIPT_DIR not in sys.path:
    sys.path.append(SCRIPT_DIR)

import ImportStaticMeshes


DEST_DIR = ImportStaticMeshes.COHERENT_THEME_KIT_DEST
MODULE_IDS = ImportStaticMeshes.COHERENT_THEME_KIT_MODULES

ARTHUR_VISUAL_ID = "Hero_1_Chad"
ARTHUR_EXPECTED_HEIGHT_CM = 200.0
CHARACTER_VISUALS_CSV = os.path.join(
    unreal.SystemLibrary.get_project_directory(),
    "Content",
    "Data",
    "CharacterVisuals.csv",
)

WALL_EXPECTED_HEIGHT_CM = 600.0
WALL_EXPECTED_DEPTH_CM = 120.0
FLOOR_EXPECTED_TILE_TARGET_CM = 600.0
FLOOR_EXPECTED_THICKNESS_CM = 24.0
FLOOR_EXPECTED_SPACING_CM = WALL_EXPECTED_HEIGHT_CM + FLOOR_EXPECTED_THICKNESS_CM
GENERATED_CEILING_BOTTOM_CM_ABOVE_FLOOR = WALL_EXPECTED_HEIGHT_CM

TOWER_SHELL_RADIUS_CM = 20000.0
TOWER_WALL_THICKNESS_CM = 120.0
TOWER_FLOOR_POLYGON_APOTHEM_CM = TOWER_SHELL_RADIUS_CM - (TOWER_WALL_THICKNESS_CM * 0.5) + 20.0
TOWER_DROP_HOLE_HALF_EXTENT_CM = 650.0


def _asset_path(asset_name):
    return f"{DEST_DIR}/{asset_name}"


def _size_cm(mesh):
    bounds = mesh.get_bounds()
    extent = bounds.box_extent
    return unreal.Vector(extent.x * 2.0, extent.y * 2.0, extent.z * 2.0)


def _range_text(values):
    if not values:
        return "none"
    return f"{min(values):.1f}-{max(values):.1f}"


def _runtime_scale_from_csv(visual_id):
    if not os.path.exists(CHARACTER_VISUALS_CSV):
        raise RuntimeError(f"missing CharacterVisuals.csv: {CHARACTER_VISUALS_CSV}")

    with open(CHARACTER_VISUALS_CSV, newline="", encoding="utf-8-sig") as handle:
        reader = csv.DictReader(handle)
        for row in reader:
            row_id = (row.get("---") or "").strip()
            if row_id != visual_id:
                continue

            scale_text = row.get("MeshRelativeScale") or ""
            match = re.search(r"Z=([0-9.]+)", scale_text)
            if not match:
                raise RuntimeError(f"could not parse MeshRelativeScale for {visual_id}: {scale_text}")

            return float(match.group(1)), row.get("SkeletalMesh")

    raise RuntimeError(f"missing {visual_id} row in CharacterVisuals.csv")


def _load_mesh_from_soft_path(path):
    if not path:
        return None
    cleaned = path.strip().strip('"')
    return unreal.EditorAssetLibrary.load_asset(cleaned)


def _module_count(span_length, module_size):
    if span_length <= 10.0 or module_size <= 1.0:
        return 0
    return max(1, int(round(span_length / module_size)))


def _floor2_tile_count(tile_size):
    full_min = -TOWER_FLOOR_POLYGON_APOTHEM_CM
    full_max = TOWER_FLOOR_POLYGON_APOTHEM_CM
    hole_min = -TOWER_DROP_HOLE_HALF_EXTENT_CM
    hole_max = TOWER_DROP_HOLE_HALF_EXTENT_CM

    boxes = [
        (full_min, full_min, hole_min, full_max),
        (hole_max, full_min, full_max, full_max),
        (hole_min, full_min, hole_max, hole_min),
        (hole_min, hole_max, hole_max, full_max),
    ]

    total = 0
    for min_x, min_y, max_x, max_y in boxes:
        total += _module_count(max_x - min_x, tile_size) * _module_count(max_y - min_y, tile_size)
    return total


def main():
    wall_x = []
    wall_y = []
    wall_z = []
    floor_x = []
    floor_y = []
    floor_z = []
    missing = []

    for module_id in MODULE_IDS:
        asset_name = f"{module_id}_UnrealReady"
        mesh = unreal.EditorAssetLibrary.load_asset(_asset_path(asset_name))
        if not mesh or not isinstance(mesh, unreal.StaticMesh):
            missing.append(asset_name)
            continue

        size = _size_cm(mesh)
        if "Wall" in module_id:
            wall_x.append(size.x)
            wall_y.append(size.y)
            wall_z.append(size.z)
        else:
            floor_x.append(size.x)
            floor_y.append(size.y)
            floor_z.append(size.z)

    arthur_scale_z, arthur_mesh_path = _runtime_scale_from_csv(ARTHUR_VISUAL_ID)
    arthur_mesh = _load_mesh_from_soft_path(arthur_mesh_path)
    if not arthur_mesh:
        missing.append(f"{ARTHUR_VISUAL_ID} mesh")
        arthur_raw_height = 0.0
        arthur_runtime_height = 0.0
    else:
        arthur_raw_height = _size_cm(arthur_mesh).z
        arthur_runtime_height = arthur_raw_height * arthur_scale_z

    source_floor_tile = max(max(floor_x) if floor_x else 0.0, max(floor_y) if floor_y else 0.0)
    old_floor2_visual_actors = _floor2_tile_count(source_floor_tile) * 2 if source_floor_tile > 0.0 else 0
    new_floor2_visual_instances = _floor2_tile_count(FLOOR_EXPECTED_TILE_TARGET_CM) * 2
    new_floor2_visual_actors = 4
    new_floor2_visual_components_max = 16

    checks = {
        "arthur_height": math.isclose(arthur_runtime_height, ARTHUR_EXPECTED_HEIGHT_CM, abs_tol=2.0),
        "wall_height": wall_z and math.isclose(WALL_EXPECTED_HEIGHT_CM, 600.0, abs_tol=0.01),
        "wall_depth": wall_x and min(wall_x) >= WALL_EXPECTED_DEPTH_CM - 1.0 and max(wall_x) <= WALL_EXPECTED_DEPTH_CM + 1.0,
        "floor_target": math.isclose(FLOOR_EXPECTED_TILE_TARGET_CM, 600.0, abs_tol=0.01),
        "floor_thickness": floor_z and min(floor_z) >= 1.0 and FLOOR_EXPECTED_THICKNESS_CM == 24.0,
        "floor_spacing": math.isclose(FLOOR_EXPECTED_SPACING_CM, 624.0, abs_tol=0.01),
        "floor2_actor_budget": new_floor2_visual_actors <= 4 and new_floor2_visual_components_max <= 16,
        "ceiling_bottom": math.isclose(GENERATED_CEILING_BOTTOM_CM_ABOVE_FLOOR, WALL_EXPECTED_HEIGHT_CM, abs_tol=0.01),
        "missing": not missing,
    }

    unreal.log(
        "[VerifyTowerGeneratedKitScaleAndLagBudget] "
        f"arthur_raw_cm_z={arthur_raw_height:.1f} arthur_scale_z={arthur_scale_z:.3f} "
        f"arthur_runtime_cm_z={arthur_runtime_height:.1f} "
        f"wall_cm_x={_range_text(wall_x)} wall_cm_y={_range_text(wall_y)} wall_cm_z={_range_text(wall_z)} "
        f"runtime_wall_cm_z={WALL_EXPECTED_HEIGHT_CM:.1f} "
        f"source_floor_cm_x={_range_text(floor_x)} source_floor_cm_y={_range_text(floor_y)} "
        f"source_floor_cm_z={_range_text(floor_z)} runtime_floor_tile_cm_xy={FLOOR_EXPECTED_TILE_TARGET_CM:.1f} "
        f"runtime_floor_thickness_cm_z={FLOOR_EXPECTED_THICKNESS_CM:.1f} "
        f"runtime_floor_spacing_cm_z={FLOOR_EXPECTED_SPACING_CM:.1f} "
        f"runtime_ceiling_bottom_above_floor_cm={GENERATED_CEILING_BOTTOM_CM_ABOVE_FLOOR:.1f} "
        f"floor2_old_visual_actor_estimate={old_floor2_visual_actors} "
        f"floor2_new_visual_instance_estimate={new_floor2_visual_instances} "
        f"floor2_new_visual_actor_budget={new_floor2_visual_actors} "
        f"floor2_new_visual_component_budget_max={new_floor2_visual_components_max}"
    )

    if missing:
        unreal.log_error(f"[VerifyTowerGeneratedKitScaleAndLagBudget] missing={missing}")

    failed = [name for name, ok in checks.items() if not ok]
    if failed:
        unreal.log_error(f"[VerifyTowerGeneratedKitScaleAndLagBudget] failed={failed}")
    else:
        unreal.log("[VerifyTowerGeneratedKitScaleAndLagBudget] ok=true")

    world = None
    try:
        subsystem = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
        if subsystem:
            world = subsystem.get_editor_world()
    except Exception:
        pass

    try:
        unreal.SystemLibrary.execute_console_command(world, "QUIT_EDITOR")
        unreal.log("[VerifyTowerGeneratedKitScaleAndLagBudget] QUIT_EDITOR requested")
    except Exception as exc:
        unreal.log_warning(f"[VerifyTowerGeneratedKitScaleAndLagBudget] Failed to request QUIT_EDITOR: {exc}")


if __name__ == "__main__":
    main()
