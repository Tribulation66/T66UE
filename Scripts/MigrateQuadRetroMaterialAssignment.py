"""
Migrate QuadRetro character static meshes to the shared GLB unlit character MI.

This keeps per-character texture identity in CharacterVisuals.csv through the
PixelatedTextureAssetPath row field, while mesh assets use one shared material.
"""

import csv
import json
import os
import sys

import unreal


SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
if SCRIPT_DIR not in sys.path:
    sys.path.append(SCRIPT_DIR)

import QuadRetroCharacterPipelineDefaults as CharacterDefaults


LOG_PREFIX = "[MigrateQuadRetroMaterialAssignment]"
CHARACTER_ROOT = "/Game/Characters"
CHARACTER_VISUALS_CSV = os.path.join(
    unreal.SystemLibrary.get_project_directory(),
    "Content",
    "Data",
    "CharacterVisuals.csv",
)
CHARACTER_VISUALS_DT = "/Game/Data/DT_CharacterVisuals"
REPORT_PATH = os.path.join(
    unreal.SystemLibrary.get_project_directory(),
    "Saved",
    "Logs",
    "MigrateQuadRetroMaterialAssignment.json",
)
PIXELATED_TEXTURE_FIELD = "PixelatedTextureAssetPath"


def _quit_editor():
    world = None
    try:
        subsystem = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
        if subsystem:
            world = subsystem.get_editor_world()
    except Exception:
        pass
    try:
        unreal.SystemLibrary.execute_console_command(world, "QUIT_EDITOR")
    except Exception as exc:
        unreal.log_warning(f"{LOG_PREFIX} Failed to request QUIT_EDITOR: {exc}")


def _read_csv_rows(csv_path):
    with open(csv_path, "r", encoding="utf-8-sig", newline="") as handle:
        reader = csv.DictReader(handle)
        fieldnames = list(reader.fieldnames or [])
        rows = [dict(row) for row in reader]
    if "---" not in fieldnames:
        raise RuntimeError(f"CharacterVisuals.csv missing row-name column: {csv_path}")
    if PIXELATED_TEXTURE_FIELD not in fieldnames:
        insert_at = fieldnames.index("StaticMesh") + 1 if "StaticMesh" in fieldnames else len(fieldnames)
        fieldnames.insert(insert_at, PIXELATED_TEXTURE_FIELD)
    for row in rows:
        for field in fieldnames:
            row.setdefault(field, "")
    return fieldnames, rows


def _write_csv_rows(csv_path, fieldnames, rows):
    with open(csv_path, "w", encoding="utf-8", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=fieldnames, quoting=csv.QUOTE_ALL, lineterminator="\n")
        writer.writeheader()
        for row in rows:
            writer.writerow({field: row.get(field, "") for field in fieldnames})


def _load_data_table_from_csv():
    dt = unreal.EditorAssetLibrary.load_asset(CHARACTER_VISUALS_DT)
    if not dt:
        raise RuntimeError(f"DT_CharacterVisuals not found at {CHARACTER_VISUALS_DT}")
    if not unreal.DataTableFunctionLibrary.fill_data_table_from_csv_file(dt, CHARACTER_VISUALS_CSV):
        raise RuntimeError(f"Failed to fill {CHARACTER_VISUALS_DT} from {CHARACTER_VISUALS_CSV}")
    unreal.EditorAssetLibrary.save_asset(CHARACTER_VISUALS_DT)


def _iter_quadretro_mesh_assets():
    for asset_path in unreal.EditorAssetLibrary.list_assets(CHARACTER_ROOT, recursive=True, include_folder=False) or []:
        if not CharacterDefaults.is_quadretro_static_mesh_path(asset_path):
            continue
        asset = unreal.EditorAssetLibrary.load_asset(asset_path)
        if asset and isinstance(asset, unreal.StaticMesh):
            yield asset_path, asset


def _mesh_path_to_csv_value(mesh):
    return mesh.get_path_name()


def _inventory_material_instances(include_legacy=True):
    rows = []
    parent_counts = {}
    total_size = 0
    for asset_path in unreal.EditorAssetLibrary.list_assets(CHARACTER_ROOT, recursive=True, include_folder=False) or []:
        if not include_legacy and asset_path.startswith(CharacterDefaults.LEGACY_MI_DIR + "/"):
            continue
        asset = unreal.EditorAssetLibrary.load_asset(asset_path)
        if not asset or not isinstance(asset, unreal.MaterialInstanceConstant):
            continue
        parent = None
        try:
            parent = unreal.MaterialEditingLibrary.get_material_instance_parent(asset)
        except Exception:
            try:
                parent = asset.get_editor_property("parent")
            except Exception:
                parent = None
        parent_path = parent.get_path_name() if parent else ""
        size = 0
        disk_path = CharacterDefaults.game_path_to_uasset_path(asset_path)
        if disk_path and os.path.isfile(disk_path):
            size = os.path.getsize(disk_path)
        total_size += size
        parent_counts[parent_path] = parent_counts.get(parent_path, 0) + 1
        rows.append({"asset": asset_path, "parent": parent_path, "bytes": size})
    return {
        "count": len(rows),
        "bytes": total_size,
        "parent_counts": parent_counts,
        "assets": rows,
    }


def _legacy_target_path(asset_path):
    if not unreal.EditorAssetLibrary.does_directory_exist(CharacterDefaults.LEGACY_MI_DIR):
        unreal.EditorAssetLibrary.make_directory(CharacterDefaults.LEGACY_MI_DIR)

    base_name = asset_path.rsplit("/", 1)[-1]
    target = f"{CharacterDefaults.LEGACY_MI_DIR}/{base_name}"
    if not unreal.EditorAssetLibrary.does_asset_exist(target):
        return target
    index = 2
    while True:
        candidate = f"{CharacterDefaults.LEGACY_MI_DIR}/{base_name}_{index}"
        if not unreal.EditorAssetLibrary.does_asset_exist(candidate):
            return candidate
        index += 1


def _is_glb_unlit_mi(asset):
    parent = None
    try:
        parent = unreal.MaterialEditingLibrary.get_material_instance_parent(asset)
    except Exception:
        try:
            parent = asset.get_editor_property("parent")
        except Exception:
            parent = None
    if not parent:
        return False
    return parent.get_path_name().split(".", 1)[0] == CharacterDefaults.GLB_MASTER_PATH


def _move_legacy_material_instances():
    moved = []
    skipped = []
    errors = []
    for asset_path in unreal.EditorAssetLibrary.list_assets(CHARACTER_ROOT, recursive=True, include_folder=False) or []:
        if asset_path.startswith(CharacterDefaults.LEGACY_MI_DIR + "/"):
            continue
        asset = unreal.EditorAssetLibrary.load_asset(asset_path)
        if not asset or not isinstance(asset, unreal.MaterialInstanceConstant):
            continue
        if not _is_glb_unlit_mi(asset):
            skipped.append(asset_path)
            continue
        target_path = _legacy_target_path(asset_path)
        try:
            if unreal.EditorAssetLibrary.rename_asset(asset_path, target_path):
                moved.append({"from": asset_path, "to": target_path})
                unreal.log(f"{LOG_PREFIX} Moved legacy MI {asset_path} -> {target_path}")
            else:
                errors.append({"asset": asset_path, "error": "rename_asset returned false"})
        except Exception as exc:
            errors.append({"asset": asset_path, "error": str(exc)})
    return {"moved": moved, "skipped": skipped, "errors": errors}


def _write_partial_report(payload):
    os.makedirs(os.path.dirname(REPORT_PATH), exist_ok=True)
    with open(REPORT_PATH, "w", encoding="utf-8") as handle:
        json.dump(payload, handle, indent=2, default=str)


def main():
    fieldnames, rows = _read_csv_rows(CHARACTER_VISUALS_CSV)
    by_mesh_path = {}
    for row in rows:
        static_mesh = row.get("StaticMesh", "")
        if static_mesh:
            by_mesh_path[static_mesh] = row

    inventory_before = _inventory_material_instances(include_legacy=True)
    active_inventory_before = _inventory_material_instances(include_legacy=False)
    shared_material = CharacterDefaults.ensure_shared_quadretro_material()

    migrated = []
    mismatches = []
    csv_rows_verified = 0
    partial_payload = {
        "character_visuals_csv": CHARACTER_VISUALS_CSV,
        "character_visuals_data_table": CHARACTER_VISUALS_DT,
        "shared_material": shared_material.get_path_name(),
        "legacy_material_folder": CharacterDefaults.LEGACY_MI_DIR,
        "migrated_mesh_count": 0,
        "csv_rows_verified": 0,
        "mismatches_found": 0,
        "inventory_before": inventory_before,
        "active_inventory_before": active_inventory_before,
        "migrated": migrated,
        "mismatches": mismatches,
        "partial": True,
        "ok": False,
    }
    _write_partial_report(partial_payload)

    for asset_path, mesh in _iter_quadretro_mesh_assets():
        mesh_object_path = _mesh_path_to_csv_value(mesh)
        row = by_mesh_path.get(mesh_object_path)
        if not row:
            mismatches.append({"mesh": mesh_object_path, "error": "no CharacterVisuals.csv row references this mesh"})
            continue

        texture = CharacterDefaults.find_pixelated_texture_for_mesh(mesh)
        if not texture:
            existing = row.get(PIXELATED_TEXTURE_FIELD, "")
            if existing.startswith("/Engine/EngineResources/DefaultTexture."):
                existing = ""
            if existing:
                mismatches.append({"mesh": mesh_object_path, "warning": "texture not found from material; kept existing CSV value", "existing": existing})
            else:
                mismatches.append({"mesh": mesh_object_path, "error": "no texture found on current MI/material slots"})
                continue
        else:
            texture_result = CharacterDefaults.apply_character_texture_defaults(texture)
            if texture_result.get("changed"):
                CharacterDefaults.safe_save(texture, texture.get_path_name().split(".", 1)[0])
            row[PIXELATED_TEXTURE_FIELD] = texture.get_path_name()
            _write_csv_rows(CHARACTER_VISUALS_CSV, fieldnames, rows)

        assignment = CharacterDefaults.assign_shared_material_to_mesh(mesh, shared_material)
        if not assignment.get("ok"):
            mismatches.append({"mesh": mesh_object_path, "error": "shared material assignment failed", "assignment": assignment})
            continue
        saved = True
        if assignment.get("changed"):
            saved = CharacterDefaults.safe_save(mesh, asset_path)
        csv_rows_verified += 1
        migrated.append(
            {
                "row": row.get("---", ""),
                "mesh": mesh_object_path,
                "pixelated_texture": row.get(PIXELATED_TEXTURE_FIELD, ""),
                "assignment": assignment,
                "saved": saved,
            }
        )
        unreal.log(f"{LOG_PREFIX} Migrated {mesh_object_path}")
        partial_payload.update(
            {
                "migrated_mesh_count": len(migrated),
                "csv_rows_verified": csv_rows_verified,
                "mismatches_found": len(mismatches),
            }
        )
        _write_partial_report(partial_payload)

    _write_csv_rows(CHARACTER_VISUALS_CSV, fieldnames, rows)
    dt_reload = {"ok": False}
    try:
        _load_data_table_from_csv()
        dt_reload = {"ok": True, "data_table": CHARACTER_VISUALS_DT}
    except Exception as exc:
        dt_reload = {"ok": False, "error": str(exc), "data_table": CHARACTER_VISUALS_DT}
        unreal.log_error(f"{LOG_PREFIX} DataTable reload failed: {exc}")

    legacy_move = _move_legacy_material_instances()
    inventory_after = _inventory_material_instances(include_legacy=True)
    active_inventory_after = _inventory_material_instances(include_legacy=False)

    payload = {
        "character_visuals_csv": CHARACTER_VISUALS_CSV,
        "character_visuals_data_table": CHARACTER_VISUALS_DT,
        "shared_material": shared_material.get_path_name(),
        "legacy_material_folder": CharacterDefaults.LEGACY_MI_DIR,
        "migrated_mesh_count": len(migrated),
        "csv_rows_verified": csv_rows_verified,
        "mismatches_found": len(mismatches),
        "inventory_before": inventory_before,
        "inventory_after": inventory_after,
        "active_inventory_before": active_inventory_before,
        "active_inventory_after": active_inventory_after,
        "legacy_move": legacy_move,
        "migrated": migrated,
        "mismatches": mismatches,
        "data_table_reload": dt_reload,
        "ok": (
            len(migrated) > 0
            and len([item for item in mismatches if "error" in item]) == 0
            and dt_reload.get("ok")
            and len(legacy_move.get("errors") or []) == 0
        ),
    }

    os.makedirs(os.path.dirname(REPORT_PATH), exist_ok=True)
    with open(REPORT_PATH, "w", encoding="utf-8") as handle:
        json.dump(payload, handle, indent=2, default=str)

    line = (
        f"{LOG_PREFIX} migrated={len(migrated)} csv_rows_verified={csv_rows_verified} "
        f"mismatches={len(mismatches)} legacy_moved={len(legacy_move.get('moved') or [])} "
        f"report={REPORT_PATH} ok={str(payload['ok']).lower()}"
    )
    if payload["ok"]:
        unreal.log(line)
    else:
        unreal.log_error(line)

    _quit_editor()


if __name__ == "__main__":
    main()
