import csv
import json
import os
import sys

import unreal


SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
if SCRIPT_DIR not in sys.path:
    sys.path.append(SCRIPT_DIR)

import QuadRetroCharacterPipelineDefaults as CharacterDefaults


CHARACTER_VISUALS_DT = "/Game/Data/DT_CharacterVisuals"
ENEMIES_DT = "/Game/Data/DT_Enemies"
CHARACTER_VISUALS_CSV_RELATIVE = os.path.join("Content", "Data", "CharacterVisuals.csv")
ENEMIES_CSV_RELATIVE = os.path.join("Content", "Data", "Enemies.csv")
REPORT_RELATIVE = os.path.join("Saved", "EnemyQuadRetroUnrealValidationReport.json")
EXPECTED_COUNT = 50
DEST_ROOT = "/Game/Characters/Mobs"


def _project_dir():
    return unreal.SystemLibrary.get_project_directory().replace("\\", "/").rstrip("/")


def _read_csv(path):
    with open(path, "r", encoding="utf-8-sig", newline="") as handle:
        return list(csv.DictReader(handle))


def _load_asset(path):
    asset = unreal.EditorAssetLibrary.load_asset(path)
    if not asset and "." in path:
        asset = unreal.EditorAssetLibrary.load_asset(path.split(".", 1)[0])
    return asset


def _path_for(package_path):
    asset_name = package_path.rsplit("/", 1)[-1]
    return f"{package_path}.{asset_name}"


def _lod_count(static_mesh):
    return CharacterDefaults.lod_count(static_mesh)


def _mesh_material_paths(static_mesh):
    paths = []
    for slot in list(static_mesh.get_editor_property("static_materials") or []):
        material = None
        try:
            material = slot.get_editor_property("material_interface")
        except Exception:
            pass
        paths.append(material.get_path_name() if material else "")
    return paths


def _is_shared_material_path(path):
    return path == CharacterDefaults.SHARED_MI_PATH or path.startswith(f"{CharacterDefaults.SHARED_MI_PATH}.")


def main():
    project_dir = _project_dir()
    enemies_path = os.path.join(project_dir, ENEMIES_CSV_RELATIVE)
    visuals_path = os.path.join(project_dir, CHARACTER_VISUALS_CSV_RELATIVE)
    report_path = os.path.join(project_dir, REPORT_RELATIVE)

    enemies = _read_csv(enemies_path)
    visual_rows = _read_csv(visuals_path)
    visuals_by_id = {row["---"]: row for row in visual_rows}

    visuals_dt = unreal.EditorAssetLibrary.load_asset(CHARACTER_VISUALS_DT)
    enemies_dt = unreal.EditorAssetLibrary.load_asset(ENEMIES_DT)
    if not visuals_dt:
        raise RuntimeError(f"Missing DataTable: {CHARACTER_VISUALS_DT}")
    if not enemies_dt:
        raise RuntimeError(f"Missing DataTable: {ENEMIES_DT}")

    visual_dt_row_names = {str(name) for name in unreal.DataTableFunctionLibrary.get_data_table_row_names(visuals_dt)}
    enemy_dt_row_names = {str(name) for name in unreal.DataTableFunctionLibrary.get_data_table_row_names(enemies_dt)}
    failures = []
    checked = []

    if len(enemies) != EXPECTED_COUNT:
        failures.append({"error": f"expected {EXPECTED_COUNT} enemy rows, found {len(enemies)}"})

    for enemy in enemies:
        enemy_id = enemy["EnemyID"]
        row = visuals_by_id.get(enemy_id)
        expected_mesh = _path_for(f"{DEST_ROOT}/{enemy_id}/SM_{enemy_id}")
        expected_texture = _path_for(f"{DEST_ROOT}/{enemy_id}/T_{enemy_id}")

        if enemy_id not in enemy_dt_row_names:
            failures.append({"enemy_id": enemy_id, "error": "missing DT_Enemies row"})
        if enemy.get("ModelStatus") != "MeshReady":
            failures.append({"enemy_id": enemy_id, "error": f"ModelStatus is {enemy.get('ModelStatus', '')!r}, expected MeshReady"})
        if not row:
            failures.append({"enemy_id": enemy_id, "error": "missing CharacterVisuals.csv row"})
            continue
        if enemy_id not in visual_dt_row_names:
            failures.append({"enemy_id": enemy_id, "error": "missing DT_CharacterVisuals row"})

        static_mesh_path = row.get("StaticMesh", "")
        texture_path = row.get("PixelatedTextureAssetPath", "")
        if static_mesh_path != expected_mesh:
            failures.append({"enemy_id": enemy_id, "error": f"StaticMesh path mismatch: {static_mesh_path} expected {expected_mesh}"})
        if texture_path != expected_texture:
            failures.append({"enemy_id": enemy_id, "error": f"PixelatedTextureAssetPath mismatch: {texture_path} expected {expected_texture}"})

        mesh = _load_asset(static_mesh_path)
        if not mesh or not isinstance(mesh, unreal.StaticMesh):
            failures.append({"enemy_id": enemy_id, "error": f"failed to load StaticMesh {static_mesh_path}"})
            continue

        texture = _load_asset(texture_path)
        if not texture or not isinstance(texture, unreal.Texture2D):
            failures.append({"enemy_id": enemy_id, "error": f"failed to load Texture2D {texture_path}"})

        material_paths = _mesh_material_paths(mesh)
        if not material_paths:
            failures.append({"enemy_id": enemy_id, "error": "StaticMesh has no material slots"})
        elif any(not _is_shared_material_path(path) for path in material_paths):
            failures.append({"enemy_id": enemy_id, "error": f"StaticMesh material slots are not all shared MI: {material_paths}"})

        lod_count = _lod_count(mesh)
        if lod_count < 4:
            failures.append({"enemy_id": enemy_id, "error": f"StaticMesh has {lod_count} LODs, expected at least 4"})

        bounds = mesh.get_bounds()
        checked.append(
            {
                "enemy_id": enemy_id,
                "static_mesh": static_mesh_path,
                "texture": texture_path,
                "asset_name": mesh.get_name(),
                "lod_count": lod_count,
                "bounds_extent": [float(bounds.box_extent.x), float(bounds.box_extent.y), float(bounds.box_extent.z)],
                "mesh_relative_scale": row.get("MeshRelativeScale", ""),
                "material_paths": material_paths,
            }
        )

    payload = {
        "success": len(failures) == 0,
        "expected_count": EXPECTED_COUNT,
        "checked_count": len(checked),
        "failures": failures,
        "character_visuals_data_table": CHARACTER_VISUALS_DT,
        "enemies_data_table": ENEMIES_DT,
        "checked": checked,
    }
    os.makedirs(os.path.dirname(report_path), exist_ok=True)
    with open(report_path, "w", encoding="utf-8") as handle:
        json.dump(payload, handle, indent=2)

    if failures:
        raise RuntimeError(f"Enemy visual Unreal validation failed for {len(failures)} row issue(s); see {report_path}")

    unreal.log(f"[ValidateImportedEnemyVisuals] Validated {len(checked)} production enemy StaticMesh rows")
    unreal.log(f"[ValidateImportedEnemyVisuals] Wrote {report_path}")

    world = None
    try:
        subsystem = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
        if subsystem:
            world = subsystem.get_editor_world()
    except Exception:
        pass
    unreal.SystemLibrary.execute_console_command(world, "QUIT_EDITOR")


if __name__ == "__main__":
    main()
