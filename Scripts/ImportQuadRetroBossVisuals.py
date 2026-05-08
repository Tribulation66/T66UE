import csv
import json
import os
import sys

import unreal


SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
if SCRIPT_DIR not in sys.path:
    sys.path.append(SCRIPT_DIR)

import ImportStaticMeshes
import MakeGLBImportsUnlit


MANIFEST_RELATIVE = os.path.join(
    "Model Generation",
    "Runs",
    "EnemyBosses",
    "EnemyBossBatch01",
    "Reports",
    "Stage02_Bosses_QuadRetroManifest.json",
)
CHARACTER_VISUALS_CSV_RELATIVE = os.path.join("Content", "Data", "CharacterVisuals.csv")
CHARACTER_VISUALS_DT = "/Game/Data/DT_CharacterVisuals"
REPORT_RELATIVE = os.path.join("Saved", "QuadRetroBossVisualImportReport.json")
DEST_ROOT = "/Game/Characters/Enemies/Bosses"
TARGET_BOSS_MAX_DIMENSION_CM = 520.0


def _project_dir():
    return unreal.SystemLibrary.get_project_directory().replace("\\", "/").rstrip("/")


def _manifest_path(project_dir):
    return os.path.join(project_dir, MANIFEST_RELATIVE).replace("\\", "/")


def _dest_dir(boss_id):
    return f"{DEST_ROOT}/{boss_id}/QuadRetro"


def _asset_name(boss_id):
    return f"SM_{boss_id}_QuadRetro"


def _object_path(dest_dir, asset_name):
    return f"{dest_dir}/{asset_name}.{asset_name}"


def _find_imported_pixelated_texture(dest_dir, source_glb):
    import_folder = os.path.splitext(os.path.basename(source_glb))[0]
    texture_root = f"{dest_dir}/{import_folder}/Textures"
    if not unreal.EditorAssetLibrary.does_directory_exist(texture_root):
        return None

    textures = []
    for asset_path in unreal.EditorAssetLibrary.list_assets(texture_root, recursive=True, include_folder=False) or []:
        asset = unreal.EditorAssetLibrary.load_asset(asset_path)
        if asset and isinstance(asset, unreal.Texture2D):
            textures.append(asset)

    if not textures:
        return None

    textures.sort(key=lambda tex: (0 if "Pixelated_512" in tex.get_name() else 1, tex.get_name()))
    return textures[0]


def _bind_imported_texture_to_materials(dest_dir, dest_name, source_glb):
    texture = _find_imported_pixelated_texture(dest_dir, source_glb)
    if not texture:
        unreal.log_warning(f"[QuadRetroBosses] No imported pixelated texture found for {dest_name}")
        return False

    materials = ImportStaticMeshes._find_candidate_materials(dest_dir, dest_name)
    if not materials:
        unreal.log_warning(f"[QuadRetroBosses] No materials found for {dest_name}")
        return False

    for material in materials:
        for parameter_name in ("BaseColorTexture", "DiffuseColorMap"):
            try:
                unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(
                    material,
                    parameter_name,
                    texture,
                )
            except Exception:
                pass
        unreal.EditorAssetLibrary.save_loaded_asset(material)

    unreal.log(f"[QuadRetroBosses] Bound {dest_name} materials to {texture.get_path_name()}")
    return True


def _asset_size_cm(asset):
    bounds = asset.get_bounds()
    return [
        float(bounds.box_extent.x) * 2.0,
        float(bounds.box_extent.y) * 2.0,
        float(bounds.box_extent.z) * 2.0,
    ]


def _format_vector(x, y=None, z=None):
    if y is None:
        y = x
    if z is None:
        z = x
    return f"(X={x:.6f},Y={y:.6f},Z={z:.6f})"


def _format_rotation(yaw):
    return f"(Pitch=0,Yaw={yaw:.6f},Roll=0)"


def _visual_scale(size_cm):
    max_dimension = max([abs(float(v)) for v in size_cm] + [1.0])
    return TARGET_BOSS_MAX_DIMENSION_CM / max_dimension


def _load_manifest(path):
    with open(path, "r", encoding="utf-8-sig") as handle:
        payload = json.load(handle)
    rows = list(payload.get("rows") or [])
    if len(rows) != 23:
        raise RuntimeError(f"Expected 23 boss rows in manifest, found {len(rows)} at {path}")
    return rows


def _import_one(row):
    boss_id = row["row_id"]
    source_glb = str(row.get("quad_retro_glb") or "").replace("\\", "/")
    if not os.path.isfile(source_glb):
        raise RuntimeError(f"missing source glb: {source_glb}")

    dest_dir = _dest_dir(boss_id)
    asset_name = _asset_name(boss_id)

    ImportStaticMeshes._cleanup_existing_import_artifacts(dest_dir, asset_name, {})
    imported_paths = ImportStaticMeshes.import_glb(source_glb, dest_dir, asset_name)
    final_path = ImportStaticMeshes._flatten_interchange_asset(dest_dir, asset_name)
    if not final_path:
        raise RuntimeError(f"Could not locate flattened StaticMesh for {asset_name}")

    scan_roots = ImportStaticMeshes._existing_scan_roots(dest_dir, asset_name)
    unlit_results = MakeGLBImportsUnlit.convert_glb_imports_unlit(scan_roots)
    _bind_imported_texture_to_materials(dest_dir, asset_name, source_glb)
    ImportStaticMeshes._apply_static_mesh_build_settings(final_path, {})
    ImportStaticMeshes._bind_materials_to_flattened_mesh(final_path, dest_dir, asset_name)
    ImportStaticMeshes._apply_material_overrides(dest_dir, asset_name, {})

    asset = unreal.EditorAssetLibrary.load_asset(final_path)
    if not asset or not isinstance(asset, unreal.StaticMesh):
        raise RuntimeError(f"Imported asset is not a StaticMesh: {final_path}")

    size_cm = _asset_size_cm(asset)
    visual_scale = _visual_scale(size_cm)

    return {
        "boss_id": boss_id,
        "display_name": row.get("display_name", ""),
        "source_glb": source_glb,
        "destination": _object_path(dest_dir, asset_name),
        "imported_paths": list(imported_paths or []),
        "final_path": final_path,
        "size_cm": size_cm,
        "visual_scale": visual_scale,
        "stage2_status": row.get("status", ""),
        "stage2_notes": row.get("notes", ""),
        "unlit": unlit_results,
    }


def _read_visual_rows(csv_path):
    with open(csv_path, "r", encoding="utf-8-sig", newline="") as handle:
        reader = csv.DictReader(handle)
        fieldnames = list(reader.fieldnames or [])
        rows = [dict(row) for row in reader]
    if "---" not in fieldnames:
        raise RuntimeError(f"CharacterVisuals.csv missing row-name column: {csv_path}")
    if "StaticMesh" not in fieldnames:
        insert_at = fieldnames.index("SkeletalMesh") + 1 if "SkeletalMesh" in fieldnames else 1
        fieldnames.insert(insert_at, "StaticMesh")
    return fieldnames, rows


def _write_visual_rows(csv_path, fieldnames, rows):
    with open(csv_path, "w", encoding="utf-8", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=fieldnames, quoting=csv.QUOTE_ALL, lineterminator="\n")
        writer.writeheader()
        for row in rows:
            writer.writerow({field: row.get(field, "") for field in fieldnames})


def _upsert_visual_rows(csv_path, import_reports):
    fieldnames, rows = _read_visual_rows(csv_path)
    by_name = {row.get("---", ""): row for row in rows}

    for report in import_reports:
        visual_id = report["boss_id"]
        row = by_name.get(visual_id)
        if row is None:
            row = {field: "" for field in fieldnames}
            row["---"] = visual_id
            rows.append(row)
            by_name[visual_id] = row

        for field in fieldnames:
            row.setdefault(field, "")

        scale = float(report["visual_scale"])
        row["SkeletalMesh"] = ""
        row["StaticMesh"] = report["destination"]
        row["LoopingAnimation"] = ""
        row["AlertAnimation"] = ""
        row["RunAnimation"] = ""
        row["MeshRelativeLocation"] = "(X=0,Y=0,Z=0)"
        row["MeshRelativeRotation"] = _format_rotation(90.0)
        row["MeshRelativeScale"] = _format_vector(scale)
        row["bLoopAnimation"] = "false"
        row["bAutoGroundToActorOrigin"] = "true"

    _write_visual_rows(csv_path, fieldnames, rows)


def _reload_character_visuals_data_table(project_dir):
    dt = unreal.EditorAssetLibrary.load_asset(CHARACTER_VISUALS_DT)
    if not dt:
        raise RuntimeError(f"DT_CharacterVisuals not found at {CHARACTER_VISUALS_DT}")

    csv_path = os.path.join(project_dir, CHARACTER_VISUALS_CSV_RELATIVE).replace("\\", "/")
    if not unreal.DataTableFunctionLibrary.fill_data_table_from_csv_file(dt, csv_path):
        raise RuntimeError(f"Failed to fill {CHARACTER_VISUALS_DT} from {csv_path}")
    unreal.EditorAssetLibrary.save_asset(CHARACTER_VISUALS_DT)


def main():
    project_dir = _project_dir()
    manifest_path = _manifest_path(project_dir)
    csv_path = os.path.join(project_dir, CHARACTER_VISUALS_CSV_RELATIVE).replace("\\", "/")
    report_path = os.path.join(project_dir, REPORT_RELATIVE).replace("\\", "/")

    manifest_rows = _load_manifest(manifest_path)
    import_reports = []
    failures = []

    for row in manifest_rows:
        boss_id = row["row_id"]
        try:
            report = _import_one(row)
            import_reports.append(report)
            unreal.log(f"[QuadRetroBosses] Imported {boss_id}")
        except Exception as exc:
            failures.append({"boss_id": boss_id, "error": str(exc)})
            unreal.log_error(f"[QuadRetroBosses] Failed {boss_id}: {exc}")

    payload = {
        "success": len(failures) == 0,
        "count": len(import_reports),
        "expected_count": 23,
        "target_boss_max_dimension_cm": TARGET_BOSS_MAX_DIMENSION_CM,
        "manifest": manifest_path,
        "character_visuals_csv": csv_path,
        "character_visuals_data_table": CHARACTER_VISUALS_DT,
        "imported": import_reports,
        "failures": failures,
    }

    if failures:
        os.makedirs(os.path.dirname(report_path), exist_ok=True)
        with open(report_path, "w", encoding="utf-8") as handle:
            json.dump(payload, handle, indent=2)
        raise RuntimeError(f"Quad Retro boss import failed for {len(failures)} asset(s); see {report_path}")

    _upsert_visual_rows(csv_path, import_reports)
    _reload_character_visuals_data_table(project_dir)

    os.makedirs(os.path.dirname(report_path), exist_ok=True)
    with open(report_path, "w", encoding="utf-8") as handle:
        json.dump(payload, handle, indent=2)

    unreal.log(f"[QuadRetroBosses] Imported and registered {len(import_reports)} boss visuals")
    unreal.log(f"[QuadRetroBosses] Wrote {report_path}")


if __name__ == "__main__":
    main()
