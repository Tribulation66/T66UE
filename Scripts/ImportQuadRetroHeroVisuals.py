import csv
import json
import os
import sys

import unreal


SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
if SCRIPT_DIR not in sys.path:
    sys.path.append(SCRIPT_DIR)

import ImportStaticMeshes
import QuadRetroCharacterPipelineDefaults as CharacterDefaults


RUN_ROOT_RELATIVE = os.path.join(
    "Model Generation",
    "Runs",
    "Heroes",
    "ChadStacySourceImageBatch01",
    "QuadRetroPipeline",
    "Medium",
)
CHARACTER_VISUALS_CSV_RELATIVE = os.path.join("Content", "Data", "CharacterVisuals.csv")
CHARACTER_VISUALS_DT = "/Game/Data/DT_CharacterVisuals"
REPORT_RELATIVE = os.path.join("Saved", "QuadRetroHeroVisualImportReport.json")
TARGET_HERO_HEIGHT_CM = 200.0


HERO_MAPPINGS = (
    ("Hero_1", "Founding"),
    ("Hero_2", "Chinese"),
    ("Hero_3", "Boxer"),
    ("Hero_4", "Billy"),
    ("Hero_5", "Royal"),
    ("Hero_6", "Robo"),
    ("Hero_7", "Rabbit"),
    ("Hero_8", "CS"),
    ("Hero_9", "Gamba"),
    ("Hero_10", "Monotone"),
    ("Hero_11", "Bald"),
    ("Hero_12", "Roach"),
)


BODY_SUFFIXES = ("Chad", "Stacy")


def _project_dir():
    return unreal.SystemLibrary.get_project_directory().replace("\\", "/").rstrip("/")


def _run_root(project_dir):
    return os.path.join(project_dir, RUN_ROOT_RELATIVE).replace("\\", "/")


def _source_glb(run_root, label):
    return os.path.join(run_root, label, "Models", f"{label}_QuadRetro.glb").replace("\\", "/")


def _dest_dir(hero_id, body_suffix):
    return f"/Game/Characters/Heroes/{hero_id}/{body_suffix}/QuadRetro"


def _asset_name(hero_id, body_suffix):
    return f"SM_{hero_id}_{body_suffix}_QuadRetro"


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
        unreal.log_warning(f"[QuadRetroHeroes] No imported pixelated texture found for {dest_name}")
        return ""

    texture_result = CharacterDefaults.apply_character_texture_defaults(texture)
    if texture_result.get("changed"):
        CharacterDefaults.safe_save(texture, texture.get_path_name().split(".", 1)[0])

    unreal.log(f"[QuadRetroHeroes] Registered {dest_name} texture {texture.get_path_name()}")
    return texture.get_path_name()


def _asset_size_cm(asset):
    bounds = asset.get_bounds()
    return [
        float(bounds.box_extent.x) * 2.0,
        float(bounds.box_extent.y) * 2.0,
        float(bounds.box_extent.z) * 2.0,
    ]


def _format_vector(value):
    return f"(X={value:.6f},Y={value:.6f},Z={value:.6f})"


def _format_rotation(yaw):
    return f"(Pitch=0,Yaw={yaw:.6f},Roll=0)"


def _row_name(hero_id, body_suffix):
    return f"{hero_id}_{body_suffix}"


def _import_one(source_glb, hero_id, body_suffix):
    dest_dir = _dest_dir(hero_id, body_suffix)
    asset_name = _asset_name(hero_id, body_suffix)

    ImportStaticMeshes._cleanup_existing_import_artifacts(dest_dir, asset_name, {})
    imported_paths = ImportStaticMeshes.import_glb(source_glb, dest_dir, asset_name)
    final_path = ImportStaticMeshes._flatten_interchange_asset(dest_dir, asset_name)
    if not final_path:
        raise RuntimeError(f"Could not locate flattened StaticMesh for {asset_name}")

    texture_path = _bind_imported_texture_to_materials(dest_dir, asset_name, source_glb)
    ImportStaticMeshes._apply_static_mesh_build_settings(final_path, {})
    ImportStaticMeshes._bind_materials_to_flattened_mesh(final_path, dest_dir, asset_name)
    lod_result = CharacterDefaults.apply_lod_ladder_to_mesh_path(final_path)
    material_result = CharacterDefaults.assign_shared_material_to_mesh_path(final_path)

    asset = unreal.EditorAssetLibrary.load_asset(final_path)
    if not asset or not isinstance(asset, unreal.StaticMesh):
        raise RuntimeError(f"Imported asset is not a StaticMesh: {final_path}")

    size_cm = _asset_size_cm(asset)
    height_cm = max(size_cm[2], 1.0)
    visual_scale = TARGET_HERO_HEIGHT_CM / height_cm

    return {
        "label": f"{hero_id}_{body_suffix}",
        "source_glb": source_glb,
        "destination": _object_path(dest_dir, asset_name),
        "imported_paths": list(imported_paths or []),
        "final_path": final_path,
        "pixelated_texture": texture_path,
        "size_cm": size_cm,
        "visual_scale": visual_scale,
        "lod": lod_result,
        "shared_material": material_result,
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
    if "PixelatedTextureAssetPath" not in fieldnames:
        insert_at = fieldnames.index("StaticMesh") + 1 if "StaticMesh" in fieldnames else len(fieldnames)
        fieldnames.insert(insert_at, "PixelatedTextureAssetPath")
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
        visual_id = report["label"]
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
        row["PixelatedTextureAssetPath"] = report.get("pixelated_texture", "")
        row["WalkAnimation"] = ""
        row["IdleAnimation"] = ""
        row["JumpAnimation"] = ""
        row["MeshRelativeLocation"] = "(X=0,Y=0,Z=0)"
        row["MeshRelativeRotation"] = _format_rotation(-90.0)
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
    run_root = _run_root(project_dir)
    csv_path = os.path.join(project_dir, CHARACTER_VISUALS_CSV_RELATIVE).replace("\\", "/")
    report_path = os.path.join(project_dir, REPORT_RELATIVE).replace("\\", "/")

    import_reports = []
    failures = []

    for hero_id, label_prefix in HERO_MAPPINGS:
        for body_suffix in BODY_SUFFIXES:
            label = f"{label_prefix}{body_suffix}"
            source_glb = _source_glb(run_root, label)
            if not os.path.isfile(source_glb):
                failures.append({"label": label, "error": f"missing source glb: {source_glb}"})
                continue
            try:
                import_reports.append(_import_one(source_glb, hero_id, body_suffix))
                unreal.log(f"[QuadRetroHeroes] Imported {label} as {_row_name(hero_id, body_suffix)}")
            except Exception as exc:
                failures.append({"label": label, "error": str(exc)})
                unreal.log_error(f"[QuadRetroHeroes] Failed {label}: {exc}")

    if failures:
        payload = {"success": False, "imported": import_reports, "failures": failures}
        os.makedirs(os.path.dirname(report_path), exist_ok=True)
        with open(report_path, "w", encoding="utf-8") as handle:
            json.dump(payload, handle, indent=2)
        raise RuntimeError(f"Quad Retro hero import failed for {len(failures)} asset(s); see {report_path}")

    _upsert_visual_rows(csv_path, import_reports)
    _reload_character_visuals_data_table(project_dir)

    payload = {
        "success": True,
        "count": len(import_reports),
        "target_height_cm": TARGET_HERO_HEIGHT_CM,
        "imported": import_reports,
        "character_visuals_csv": csv_path,
        "character_visuals_data_table": CHARACTER_VISUALS_DT,
    }
    os.makedirs(os.path.dirname(report_path), exist_ok=True)
    with open(report_path, "w", encoding="utf-8") as handle:
        json.dump(payload, handle, indent=2)

    unreal.log(f"[QuadRetroHeroes] Imported and registered {len(import_reports)} hero visuals")
    unreal.log(f"[QuadRetroHeroes] Wrote {report_path}")


if __name__ == "__main__":
    main()
