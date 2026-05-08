import csv
import json
import os

import unreal


CHARACTER_VISUALS_DT = "/Game/Data/DT_CharacterVisuals"
CHARACTER_VISUALS_CSV_RELATIVE = os.path.join("Content", "Data", "CharacterVisuals.csv")
BOSSES_CSV_RELATIVE = os.path.join("Content", "Data", "Bosses.csv")
REPORT_RELATIVE = os.path.join("Saved", "BossQuadRetroUnrealValidationReport.json")


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


def main():
    project_dir = _project_dir()
    bosses_path = os.path.join(project_dir, BOSSES_CSV_RELATIVE)
    visuals_path = os.path.join(project_dir, CHARACTER_VISUALS_CSV_RELATIVE)
    report_path = os.path.join(project_dir, REPORT_RELATIVE)

    bosses = _read_csv(bosses_path)
    visual_rows = _read_csv(visuals_path)
    boss_ids = [row["BossID"] for row in bosses]
    visuals_by_id = {row["---"]: row for row in visual_rows}

    dt = unreal.EditorAssetLibrary.load_asset(CHARACTER_VISUALS_DT)
    if not dt:
        raise RuntimeError(f"Missing DataTable: {CHARACTER_VISUALS_DT}")

    dt_row_names = {str(name) for name in dt.get_row_names()}
    failures = []
    checked = []

    for boss_id in boss_ids:
        row = visuals_by_id.get(boss_id)
        if not row:
            failures.append({"boss_id": boss_id, "error": "missing CharacterVisuals.csv row"})
            continue

        static_mesh_path = row.get("StaticMesh", "")
        if boss_id not in dt_row_names:
            failures.append({"boss_id": boss_id, "error": "missing DT_CharacterVisuals row"})
            continue
        if not static_mesh_path:
            failures.append({"boss_id": boss_id, "error": "empty StaticMesh path"})
            continue

        asset = _load_asset(static_mesh_path)
        if not asset or not isinstance(asset, unreal.StaticMesh):
            failures.append({"boss_id": boss_id, "error": f"failed to load StaticMesh {static_mesh_path}"})
            continue

        bounds = asset.get_bounds()
        checked.append({
            "boss_id": boss_id,
            "static_mesh": static_mesh_path,
            "asset_name": asset.get_name(),
            "bounds_extent": [float(bounds.box_extent.x), float(bounds.box_extent.y), float(bounds.box_extent.z)],
            "mesh_relative_scale": row.get("MeshRelativeScale", ""),
        })

    payload = {
        "success": len(failures) == 0,
        "expected_count": 23,
        "checked_count": len(checked),
        "failures": failures,
        "character_visuals_data_table": CHARACTER_VISUALS_DT,
        "checked": checked,
    }
    os.makedirs(os.path.dirname(report_path), exist_ok=True)
    with open(report_path, "w", encoding="utf-8") as handle:
        json.dump(payload, handle, indent=2)

    if failures:
        raise RuntimeError(f"Boss visual Unreal validation failed for {len(failures)} row(s); see {report_path}")

    unreal.log(f"[ValidateImportedBossVisuals] Validated {len(checked)} boss StaticMesh rows")
    unreal.log(f"[ValidateImportedBossVisuals] Wrote {report_path}")

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
