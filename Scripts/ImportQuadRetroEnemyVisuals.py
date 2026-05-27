import csv
import json
import os
import subprocess
import sys

import unreal


SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
if SCRIPT_DIR not in sys.path:
    sys.path.append(SCRIPT_DIR)

import ImportStaticMeshes
import QuadRetroCharacterPipelineDefaults as CharacterDefaults


PRODUCTION_MANIFESTS_RELATIVE = [
    os.path.join("Model Generation", "Production", "Roster_v1", "AgentA", "manifest.csv"),
    os.path.join("Model Generation", "Production", "Roster_v1", "AgentB", "manifest.csv"),
]
CHARACTER_VISUALS_CSV_RELATIVE = os.path.join("Content", "Data", "CharacterVisuals.csv")
ENEMIES_CSV_RELATIVE = os.path.join("Content", "Data", "Enemies.csv")
CHARACTER_VISUALS_DT = "/Game/Data/DT_CharacterVisuals"
ENEMIES_DT = "/Game/Data/DT_Enemies"
REPORT_RELATIVE = os.path.join("Saved", "QuadRetroEnemyVisualImportReport.json")
DEST_ROOT = "/Game/Characters/Mobs"
LEGACY_REGULAR_ENEMY_DIR = "/Game/Characters/Enemies/Regular"
TARGET_ENEMY_MAX_DIMENSION_CM = 180.0
EXPECTED_PRODUCTION_ROSTER_COUNT = 50
CONVERTED_ROOT_RELATIVE = os.path.join("Saved", "MobProductionImport", "Converted")
BLENDER_EXE = os.environ.get("T66_BLENDER_EXE", r"C:\Program Files\Blender Foundation\Blender 5.1\blender.exe")

BLENDER_CONVERT_SCRIPT = r'''
import json
import os
import sys

import bpy


def main():
    argv = sys.argv
    if "--" in argv:
        argv = argv[argv.index("--") + 1:]
    if len(argv) < 3:
        raise SystemExit("expected args: source_glb output_dir enemy_id")
    source_glb, output_dir, enemy_id = argv[:3]
    os.makedirs(output_dir, exist_ok=True)

    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete()

    bpy.ops.import_scene.gltf(filepath=source_glb)

    mesh_objects = [obj for obj in bpy.context.scene.objects if obj.type == "MESH"]
    if not mesh_objects:
        raise RuntimeError(f"no mesh objects imported from {source_glb}")

    for obj in bpy.context.scene.objects:
        obj.select_set(False)
    for obj in mesh_objects:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = mesh_objects[0]

    texture_candidates = []
    for image in bpy.data.images:
        if image.name in {"Render Result", "Viewer Node"}:
            continue
        width, height = image.size
        if width <= 0 or height <= 0:
            continue
        texture_candidates.append((int(width) * int(height), image.name, image))
    texture_candidates.sort(key=lambda item: (-item[0], item[1]))
    if not texture_candidates:
        raise RuntimeError(f"no Texture2D image data found in {source_glb}")

    texture_path = os.path.join(output_dir, f"{enemy_id}_Texture.png")
    image = texture_candidates[0][2]
    image.filepath_raw = texture_path
    image.file_format = "PNG"
    image.save()

    fbx_path = os.path.join(output_dir, f"{enemy_id}.fbx")
    bpy.ops.export_scene.fbx(
        filepath=fbx_path,
        use_selection=True,
        add_leaf_bones=False,
        path_mode="AUTO",
        bake_space_transform=False,
    )

    report = {
        "source_glb": source_glb,
        "fbx_path": fbx_path,
        "texture_path": texture_path,
        "mesh_count": len(mesh_objects),
        "texture_name": image.name,
        "texture_size": [int(image.size[0]), int(image.size[1])],
    }
    with open(os.path.join(output_dir, f"{enemy_id}_conversion.json"), "w", encoding="utf-8") as handle:
        json.dump(report, handle, indent=2)


if __name__ == "__main__":
    main()
'''


def _project_dir():
    return unreal.SystemLibrary.get_project_directory().replace("\\", "/").rstrip("/")


def _project_path(project_dir, relative_path):
    return os.path.join(project_dir, relative_path).replace("\\", "/")


def _dest_dir(enemy_id):
    return f"{DEST_ROOT}/{enemy_id}"


def _asset_name(enemy_id):
    return f"SM_{enemy_id}"


def _texture_name(enemy_id):
    return f"T_{enemy_id}"


def _object_path(package_path):
    asset_name = package_path.rsplit("/", 1)[-1]
    return f"{package_path}.{asset_name}"


def _read_csv_rows(csv_path):
    with open(csv_path, "r", encoding="utf-8-sig", newline="") as handle:
        reader = csv.DictReader(handle)
        return list(reader.fieldnames or []), [dict(row) for row in reader]


def _write_csv_rows(csv_path, fieldnames, rows, quoting=csv.QUOTE_MINIMAL):
    with open(csv_path, "w", encoding="utf-8", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=fieldnames, quoting=quoting, lineterminator="\n")
        writer.writeheader()
        for row in rows:
            writer.writerow({field: row.get(field, "") for field in fieldnames})


def _load_enemy_rows(csv_path):
    fieldnames, rows = _read_csv_rows(csv_path)
    if "---" not in fieldnames or "EnemyID" not in fieldnames:
        raise RuntimeError(f"Enemies.csv is missing expected columns: {csv_path}")
    if len(rows) != EXPECTED_PRODUCTION_ROSTER_COUNT:
        raise RuntimeError(
            f"Expected {EXPECTED_PRODUCTION_ROSTER_COUNT} enemy rows, found {len(rows)} at {csv_path}"
        )
    return fieldnames, rows


def _resolve_manifest_path(project_dir, raw_path):
    raw = (raw_path or "").strip()
    if not raw:
        return ""
    if os.path.isabs(raw):
        return raw.replace("\\", "/")
    return os.path.join(project_dir, raw).replace("\\", "/")


def _converted_dir(project_dir, enemy_id):
    return os.path.join(project_dir, CONVERTED_ROOT_RELATIVE, enemy_id).replace("\\", "/")


def _write_blender_convert_script(project_dir):
    script_path = os.path.join(project_dir, "Saved", "MobProductionImport", "convert_glb_to_fbx.py").replace("\\", "/")
    os.makedirs(os.path.dirname(script_path), exist_ok=True)
    with open(script_path, "w", encoding="utf-8", newline="\n") as handle:
        handle.write(BLENDER_CONVERT_SCRIPT)
    return script_path


def _convert_glb_for_unreal(project_dir, enemy_id, source_glb):
    if not os.path.isfile(BLENDER_EXE):
        raise RuntimeError(f"Blender executable not found: {BLENDER_EXE}")
    output_dir = _converted_dir(project_dir, enemy_id)
    os.makedirs(output_dir, exist_ok=True)
    script_path = _write_blender_convert_script(project_dir)
    command = [
        BLENDER_EXE,
        "--background",
        "--python",
        script_path,
        "--",
        source_glb,
        output_dir,
        enemy_id,
    ]
    completed = subprocess.run(command, capture_output=True, text=True)
    if completed.returncode != 0:
        raise RuntimeError(
            "Blender GLB conversion failed "
            f"(exit={completed.returncode}): stdout={completed.stdout[-2000:]} stderr={completed.stderr[-2000:]}"
        )
    fbx_path = os.path.join(output_dir, f"{enemy_id}.fbx").replace("\\", "/")
    texture_path = os.path.join(output_dir, f"{enemy_id}_Texture.png").replace("\\", "/")
    if not os.path.isfile(fbx_path):
        raise RuntimeError(f"Blender conversion did not create FBX: {fbx_path}")
    if not os.path.isfile(texture_path):
        raise RuntimeError(f"Blender conversion did not extract texture: {texture_path}")
    return {
        "fbx_path": fbx_path,
        "texture_path": texture_path,
        "conversion_report": os.path.join(output_dir, f"{enemy_id}_conversion.json").replace("\\", "/"),
    }


def _load_manifest_rows(project_dir):
    rows_by_name = {}
    manifest_reports = []
    for relative_manifest in PRODUCTION_MANIFESTS_RELATIVE:
        manifest_path = _project_path(project_dir, relative_manifest)
        if not os.path.isfile(manifest_path):
            raise RuntimeError(f"Missing production manifest: {manifest_path}")
        _fieldnames, rows = _read_csv_rows(manifest_path)
        success_count = 0
        failed_count = 0
        for row in rows:
            name = (row.get("Name") or "").strip()
            status = (row.get("Status") or "").strip()
            if not name:
                continue
            if status.lower() == "success":
                success_count += 1
            else:
                failed_count += 1
            rows_by_name[name] = {
                "name": name,
                "status": status,
                "glb_path": _resolve_manifest_path(project_dir, row.get("GLB_Path")),
                "image_path": _resolve_manifest_path(project_dir, row.get("Image_Path")),
                "manifest": manifest_path,
                "notes": row.get("Notes", ""),
            }
        manifest_reports.append(
            {
                "manifest": manifest_path,
                "row_count": len(rows),
                "success_count": success_count,
                "failed_count": failed_count,
            }
        )
    return rows_by_name, manifest_reports


def _find_imported_texture(dest_dir, target_texture_path):
    if not unreal.EditorAssetLibrary.does_directory_exist(dest_dir):
        return None

    candidates = []
    for asset_path in unreal.EditorAssetLibrary.list_assets(dest_dir, recursive=True, include_folder=False) or []:
        clean_path = str(asset_path).split(".", 1)[0]
        asset = unreal.EditorAssetLibrary.load_asset(asset_path)
        if not asset or not isinstance(asset, unreal.Texture2D):
            continue
        score = 0
        name = asset.get_name()
        if clean_path == target_texture_path:
            score -= 100
        if "BaseColor" in name or "Diffuse" in name or "Texture" in name or name.startswith("T_"):
            score -= 10
        if "Normal" in name or "ORM" in name or "Roughness" in name or "Metallic" in name:
            score += 20
        candidates.append((score, name, clean_path, asset))

    if not candidates:
        return None

    candidates.sort(key=lambda item: (item[0], item[1]))
    return candidates[0][3]


def _rename_texture_to_target(texture, dest_dir, enemy_id):
    target_package = f"{dest_dir}/{_texture_name(enemy_id)}"
    if not texture:
        return ""

    source_package = texture.get_path_name().split(".", 1)[0]
    if source_package != target_package:
        if unreal.EditorAssetLibrary.does_asset_exist(target_package):
            unreal.EditorAssetLibrary.delete_asset(target_package)
        if not unreal.EditorAssetLibrary.rename_asset(source_package, target_package):
            unreal.log_warning(f"[QuadRetroEnemies] Failed to rename texture {source_package} -> {target_package}")
            target_package = source_package
        texture = unreal.EditorAssetLibrary.load_asset(target_package) or texture

    texture_result = CharacterDefaults.apply_character_texture_defaults(texture)
    if not texture_result.get("ok"):
        unreal.log_warning(f"[QuadRetroEnemies] Texture defaults did not fully verify for {target_package}: {texture_result}")
    CharacterDefaults.safe_save(texture, target_package)
    return texture.get_path_name()


def _import_texture(texture_source_path, dest_dir, enemy_id):
    target_package = f"{dest_dir}/{_texture_name(enemy_id)}"
    if unreal.EditorAssetLibrary.does_asset_exist(target_package):
        unreal.EditorAssetLibrary.delete_asset(target_package)

    task = unreal.AssetImportTask()
    task.automated = True
    task.save = True
    task.replace_existing = True
    task.replace_existing_settings = True
    task.filename = texture_source_path
    task.destination_path = dest_dir
    task.destination_name = _texture_name(enemy_id)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

    texture = unreal.EditorAssetLibrary.load_asset(target_package)
    if not texture and task.imported_object_paths:
        texture = unreal.EditorAssetLibrary.load_asset(task.imported_object_paths[0])
    if not texture or not isinstance(texture, unreal.Texture2D):
        raise RuntimeError(f"Could not import Texture2D for {enemy_id} from {texture_source_path}")
    return _rename_texture_to_target(texture, dest_dir, enemy_id)


def _cleanup_imported_material_artifacts(dest_dir, asset_name):
    deleted = []
    for directory in (
        f"{dest_dir}/{asset_name}/Materials",
        f"{dest_dir}/{asset_name}/AnimationSequences",
        f"{dest_dir}/{asset_name}/StaticMeshes",
    ):
        try:
            if unreal.EditorAssetLibrary.does_directory_exist(directory):
                if unreal.EditorAssetLibrary.delete_directory(directory):
                    deleted.append(directory)
        except Exception as exc:
            unreal.log_warning(f"[QuadRetroEnemies] Could not delete imported artifact directory {directory}: {exc}")
    return deleted


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


def _visual_scale(size_cm):
    max_dimension = max([abs(float(v)) for v in size_cm] + [1.0])
    return TARGET_ENEMY_MAX_DIMENSION_CM / max_dimension


def _import_one(project_dir, enemy_id, display_name, source_glb):
    if not os.path.isfile(source_glb):
        raise RuntimeError(f"missing source glb: {source_glb}")

    dest_dir = _dest_dir(enemy_id)
    asset_name = _asset_name(enemy_id)
    mesh_package = f"{dest_dir}/{asset_name}"
    converted = _convert_glb_for_unreal(project_dir, enemy_id, source_glb)

    ImportStaticMeshes._cleanup_existing_import_artifacts(dest_dir, asset_name, {"mode": "dest_dir"})
    imported_paths = ImportStaticMeshes.import_glb(converted["fbx_path"], dest_dir, asset_name)
    final_path = ImportStaticMeshes._flatten_interchange_asset(dest_dir, asset_name)
    if not final_path:
        raise RuntimeError(f"Could not locate flattened StaticMesh for {asset_name}")
    if final_path != mesh_package:
        if unreal.EditorAssetLibrary.does_asset_exist(mesh_package):
            unreal.EditorAssetLibrary.delete_asset(mesh_package)
        if unreal.EditorAssetLibrary.rename_asset(final_path, mesh_package):
            final_path = mesh_package

    texture_path = _import_texture(converted["texture_path"], dest_dir, enemy_id)

    ImportStaticMeshes._apply_static_mesh_build_settings(final_path, {})
    ImportStaticMeshes._bind_materials_to_flattened_mesh(final_path, dest_dir, asset_name)
    lod_result = CharacterDefaults.apply_lod_ladder_to_mesh_path(final_path)
    material_result = CharacterDefaults.assign_shared_material_to_mesh_path(final_path)
    artifact_deletions = _cleanup_imported_material_artifacts(dest_dir, asset_name)

    asset = unreal.EditorAssetLibrary.load_asset(final_path)
    if not asset or not isinstance(asset, unreal.StaticMesh):
        raise RuntimeError(f"Imported asset is not a StaticMesh: {final_path}")

    size_cm = _asset_size_cm(asset)
    visual_scale = _visual_scale(size_cm)

    return {
        "enemy_id": enemy_id,
        "display_name": display_name,
        "source_glb": source_glb,
        "converted_fbx": converted["fbx_path"],
        "converted_texture": converted["texture_path"],
        "conversion_report": converted["conversion_report"],
        "destination": _object_path(final_path),
        "imported_paths": list(imported_paths or []),
        "final_path": final_path,
        "pixelated_texture": texture_path,
        "size_cm": size_cm,
        "visual_scale": visual_scale,
        "lod": lod_result,
        "shared_material": material_result,
        "artifact_deletions": artifact_deletions,
    }


def _read_visual_rows(csv_path):
    fieldnames, rows = _read_csv_rows(csv_path)
    if "---" not in fieldnames:
        raise RuntimeError(f"CharacterVisuals.csv missing row-name column: {csv_path}")
    if "StaticMesh" not in fieldnames:
        insert_at = fieldnames.index("SkeletalMesh") + 1 if "SkeletalMesh" in fieldnames else 1
        fieldnames.insert(insert_at, "StaticMesh")
    if "PixelatedTextureAssetPath" not in fieldnames:
        insert_at = fieldnames.index("StaticMesh") + 1 if "StaticMesh" in fieldnames else len(fieldnames)
        fieldnames.insert(insert_at, "PixelatedTextureAssetPath")
    return fieldnames, rows


def _upsert_visual_rows(csv_path, import_reports):
    fieldnames, rows = _read_visual_rows(csv_path)
    by_name = {row.get("---", ""): row for row in rows}

    for report in import_reports:
        visual_id = report["enemy_id"]
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
        row["MeshRelativeRotation"] = _format_rotation(90.0)
        row["MeshRelativeScale"] = _format_vector(scale)
        row["bLoopAnimation"] = "false"
        row["bAutoGroundToActorOrigin"] = "true"

    _write_csv_rows(csv_path, fieldnames, rows, quoting=csv.QUOTE_ALL)


def _mark_enemy_rows_mesh_ready(csv_path, import_reports):
    fieldnames, rows = _load_enemy_rows(csv_path)
    imported_ids = {report["enemy_id"] for report in import_reports}

    if "ModelStatus" not in fieldnames:
        fieldnames.append("ModelStatus")

    for row in rows:
        if row.get("EnemyID") in imported_ids:
            row["ModelStatus"] = "MeshReady"

    _write_csv_rows(csv_path, fieldnames, rows)


def _reload_data_table(dt_path, csv_path):
    dt = unreal.EditorAssetLibrary.load_asset(dt_path)
    if not dt:
        raise RuntimeError(f"DataTable not found at {dt_path}")

    if not unreal.DataTableFunctionLibrary.fill_data_table_from_csv_file(dt, csv_path):
        raise RuntimeError(f"Failed to fill {dt_path} from {csv_path}")
    unreal.EditorAssetLibrary.save_asset(dt_path)


def _delete_directory(directory_path):
    if not unreal.EditorAssetLibrary.does_directory_exist(directory_path):
        return {"path": directory_path, "deleted": False, "reason": "not present"}
    deleted = bool(unreal.EditorAssetLibrary.delete_directory(directory_path))
    return {"path": directory_path, "deleted": deleted, "reason": "" if deleted else "delete_directory returned false"}


def main():
    project_dir = _project_dir()
    enemies_csv = _project_path(project_dir, ENEMIES_CSV_RELATIVE)
    visuals_csv = _project_path(project_dir, CHARACTER_VISUALS_CSV_RELATIVE)
    report_path = _project_path(project_dir, REPORT_RELATIVE)

    _enemy_fieldnames, enemy_rows = _load_enemy_rows(enemies_csv)
    manifests_by_name, manifest_reports = _load_manifest_rows(project_dir)
    import_reports = []
    skipped = []
    failures = []

    for row in enemy_rows:
        enemy_id = row["EnemyID"]
        display_name = row.get("DisplayName", "")
        manifest_row = manifests_by_name.get(enemy_id)
        if not manifest_row:
            failures.append({"enemy_id": enemy_id, "error": "missing production manifest row"})
            unreal.log_error(f"[QuadRetroEnemies] Missing production manifest row for {enemy_id}")
            continue
        if manifest_row.get("status", "").lower() != "success":
            skipped.append({"enemy_id": enemy_id, "status": manifest_row.get("status", ""), "manifest": manifest_row.get("manifest", "")})
            unreal.log_warning(f"[QuadRetroEnemies] Skipping {enemy_id}; manifest status={manifest_row.get('status', '')}")
            continue
        try:
            report = _import_one(project_dir, enemy_id, display_name, manifest_row["glb_path"])
            report["manifest"] = manifest_row.get("manifest", "")
            report["concept_image"] = manifest_row.get("image_path", "")
            report["manifest_notes"] = manifest_row.get("notes", "")
            import_reports.append(report)
            unreal.log(f"[QuadRetroEnemies] Imported {enemy_id}")
        except Exception as exc:
            failures.append({"enemy_id": enemy_id, "source_glb": manifest_row.get("glb_path", ""), "error": str(exc)})
            unreal.log_error(f"[QuadRetroEnemies] Failed {enemy_id}: {exc}")

    deletion_reports = []
    payload = {
        "success": len(failures) == 0 and len(import_reports) + len(skipped) == len(enemy_rows),
        "count": len(import_reports),
        "skipped_count": len(skipped),
        "expected_count": EXPECTED_PRODUCTION_ROSTER_COUNT,
        "target_enemy_max_dimension_cm": TARGET_ENEMY_MAX_DIMENSION_CM,
        "destination_root": DEST_ROOT,
        "character_visuals_csv": visuals_csv,
        "enemies_csv": enemies_csv,
        "character_visuals_data_table": CHARACTER_VISUALS_DT,
        "enemies_data_table": ENEMIES_DT,
        "manifests": manifest_reports,
        "imported": import_reports,
        "skipped": skipped,
        "failures": failures,
        "deletions": deletion_reports,
    }

    os.makedirs(os.path.dirname(report_path), exist_ok=True)

    if failures:
        with open(report_path, "w", encoding="utf-8") as handle:
            json.dump(payload, handle, indent=2)
        raise RuntimeError(f"Production enemy import failed for {len(failures)} asset(s); see {report_path}")

    _upsert_visual_rows(visuals_csv, import_reports)
    _mark_enemy_rows_mesh_ready(enemies_csv, import_reports)
    _reload_data_table(CHARACTER_VISUALS_DT, visuals_csv)
    _reload_data_table(ENEMIES_DT, enemies_csv)

    deletion_reports.append(_delete_directory(LEGACY_REGULAR_ENEMY_DIR))
    deletion_reports.append(_delete_directory(CharacterDefaults.LEGACY_MI_DIR))
    payload["deletions"] = deletion_reports

    with open(report_path, "w", encoding="utf-8") as handle:
        json.dump(payload, handle, indent=2)

    unreal.log(
        f"[QuadRetroEnemies] Imported and registered {len(import_reports)} production enemy visuals; "
        f"skipped={len(skipped)} report={report_path}"
    )


if __name__ == "__main__":
    main()
