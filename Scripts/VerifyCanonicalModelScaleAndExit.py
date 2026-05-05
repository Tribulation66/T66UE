"""
Verify that character and generated-kit assets carry their canonical scale.

The rule this script enforces is deliberately simple: playable hero visuals
should use identity data scale and their imported skeletal mesh bounds should
already be 200 cm tall. Generated kit wall/floor meshes should already match
their authored module sizes before gameplay or preview placement code touches
them.
"""

import csv
import json
import math
import os
import sys

import unreal


SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
if SCRIPT_DIR not in sys.path:
    sys.path.append(SCRIPT_DIR)

import ImportStaticMeshes


PROJECT_DIR = unreal.SystemLibrary.get_project_directory()
CHARACTER_VISUALS_CSV = os.path.join(PROJECT_DIR, "Content", "Data", "CharacterVisuals.csv")
REPORT_PATH = os.path.join(PROJECT_DIR, "Saved", "CanonicalModelScaleReport.json")

HERO_TARGET_HEIGHT_CM = 200.0
HERO_HEIGHT_TOLERANCE_CM = 2.0
KIT_TOLERANCE_CM = 2.0

WALL_TARGET_DEPTH_CM = 120.0
WALL_TARGET_HEIGHT_CM = 600.0
FLOOR_TARGET_TILE_CM = 1300.0
FLOOR_TARGET_THICKNESS_CM = 24.0


def _soft_object_path(text):
    return (text or "").strip().strip('"')


def _vector_size(bounds):
    extent = bounds.box_extent
    return [float(extent.x) * 2.0, float(extent.y) * 2.0, float(extent.z) * 2.0]


def _parse_scale(text):
    scale = {}
    for part in (text or "").strip().strip("()").split(","):
        if "=" not in part:
            continue
        key, value = part.split("=", 1)
        try:
            scale[key.strip()] = float(value)
        except ValueError:
            pass
    return [scale.get("X", 0.0), scale.get("Y", 0.0), scale.get("Z", 0.0)]


def _nearly(value, target, tolerance):
    return math.isclose(float(value), float(target), abs_tol=float(tolerance))


def _load_asset(path):
    if not path:
        return None
    return unreal.EditorAssetLibrary.load_asset(path)


def _verify_heroes():
    results = []
    failures = []

    if not os.path.exists(CHARACTER_VISUALS_CSV):
        return [], [f"missing csv: {CHARACTER_VISUALS_CSV}"]

    with open(CHARACTER_VISUALS_CSV, newline="", encoding="utf-8-sig") as handle:
        reader = csv.DictReader(handle)
        for row in reader:
            visual_id = (row.get("---") or "").strip()
            if not visual_id.startswith("Hero_"):
                continue

            scale = _parse_scale(row.get("MeshRelativeScale"))
            scale_identity = all(_nearly(axis, 1.0, 0.001) for axis in scale)
            mesh_path = _soft_object_path(row.get("SkeletalMesh"))
            mesh = _load_asset(mesh_path)

            entry = {
                "visual_id": visual_id,
                "mesh": mesh_path,
                "mesh_relative_scale": scale,
                "scale_identity": scale_identity,
                "loaded": bool(mesh),
            }

            if not scale_identity:
                failures.append(f"{visual_id}: MeshRelativeScale is {scale}, expected [1, 1, 1]")

            if not mesh:
                failures.append(f"{visual_id}: failed to load {mesh_path}")
                results.append(entry)
                continue

            size = _vector_size(mesh.get_bounds())
            height = size[2]
            entry["bounds_size_cm"] = size
            entry["bounds_height_cm_z"] = height
            entry["height_is_200cm"] = _nearly(height, HERO_TARGET_HEIGHT_CM, HERO_HEIGHT_TOLERANCE_CM)
            if not entry["height_is_200cm"]:
                failures.append(f"{visual_id}: mesh Z height {height:.1f} cm, expected {HERO_TARGET_HEIGHT_CM:.1f} cm")

            results.append(entry)

    return results, failures


def _verify_generated_kit():
    results = []
    failures = []
    dest_dir = ImportStaticMeshes.COHERENT_THEME_KIT_DEST

    for module_id in ImportStaticMeshes.COHERENT_THEME_KIT_MODULES:
        asset_name = f"{module_id}_UnrealReady"
        asset_path = f"{dest_dir}/{asset_name}"
        mesh = _load_asset(asset_path)
        entry = {
            "module_id": module_id,
            "mesh": asset_path,
            "loaded": bool(mesh),
        }
        if not mesh or not isinstance(mesh, unreal.StaticMesh):
            failures.append(f"{module_id}: failed to load static mesh {asset_path}")
            results.append(entry)
            continue

        size = _vector_size(mesh.get_bounds())
        entry["bounds_size_cm"] = size
        if "Wall" in module_id:
            entry["depth_is_120cm"] = _nearly(size[0], WALL_TARGET_DEPTH_CM, KIT_TOLERANCE_CM)
            entry["height_is_600cm"] = _nearly(size[2], WALL_TARGET_HEIGHT_CM, KIT_TOLERANCE_CM)
            if not entry["depth_is_120cm"]:
                failures.append(f"{module_id}: wall X depth {size[0]:.1f} cm, expected {WALL_TARGET_DEPTH_CM:.1f} cm")
            if not entry["height_is_600cm"]:
                failures.append(f"{module_id}: wall Z height {size[2]:.1f} cm, expected {WALL_TARGET_HEIGHT_CM:.1f} cm")
        else:
            tile_size = max(size[0], size[1])
            entry["tile_is_1300cm"] = _nearly(tile_size, FLOOR_TARGET_TILE_CM, KIT_TOLERANCE_CM)
            entry["thickness_is_24cm"] = _nearly(size[2], FLOOR_TARGET_THICKNESS_CM, KIT_TOLERANCE_CM)
            if not entry["tile_is_1300cm"]:
                failures.append(f"{module_id}: floor tile max XY {tile_size:.1f} cm, expected {FLOOR_TARGET_TILE_CM:.1f} cm")
            if not entry["thickness_is_24cm"]:
                failures.append(f"{module_id}: floor Z thickness {size[2]:.1f} cm, expected {FLOOR_TARGET_THICKNESS_CM:.1f} cm")

        results.append(entry)

    return results, failures


def main():
    os.makedirs(os.path.dirname(REPORT_PATH), exist_ok=True)

    heroes, hero_failures = _verify_heroes()
    kit, kit_failures = _verify_generated_kit()
    failures = hero_failures + kit_failures

    report = {
        "ok": not failures,
        "hero_target_height_cm": HERO_TARGET_HEIGHT_CM,
        "hero_height_tolerance_cm": HERO_HEIGHT_TOLERANCE_CM,
        "heroes": heroes,
        "generated_kit": kit,
        "failures": failures,
    }

    with open(REPORT_PATH, "w", encoding="utf-8") as handle:
        json.dump(report, handle, indent=2)

    unreal.log(
        f"[VerifyCanonicalModelScale] heroes={len(heroes)} kit_modules={len(kit)} "
        f"failures={len(failures)} report={REPORT_PATH}"
    )
    if failures:
        for failure in failures[:50]:
            unreal.log_error(f"[VerifyCanonicalModelScale] {failure}")
        if len(failures) > 50:
            unreal.log_error(f"[VerifyCanonicalModelScale] ... {len(failures) - 50} more failures in report")
    else:
        unreal.log("[VerifyCanonicalModelScale] ok=true")

    world = None
    try:
        subsystem = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
        if subsystem:
            world = subsystem.get_editor_world()
    except Exception:
        pass

    try:
        unreal.SystemLibrary.execute_console_command(world, "QUIT_EDITOR")
        unreal.log("[VerifyCanonicalModelScale] QUIT_EDITOR requested")
    except Exception as exc:
        unreal.log_warning(f"[VerifyCanonicalModelScale] Failed to request QUIT_EDITOR: {exc}")


if __name__ == "__main__":
    main()
