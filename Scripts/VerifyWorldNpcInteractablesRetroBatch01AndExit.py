"""
Validate WorldNpcInteractablesRetroBatch01 assets and live data references.
"""

import csv
import json
import os
from datetime import datetime, timezone
from pathlib import Path

import unreal


PROJECT_DIR = Path(unreal.SystemLibrary.get_project_directory())
RUN_ROOT = PROJECT_DIR / "Model Generation" / "Runs" / "Interactables" / "WorldNpcInteractablesRetroBatch01"
REPORTS_DIR = RUN_ROOT / "Reports"
STAGE01_PATH = REPORTS_DIR / "Stage01_SourceAndTrellisManifest.json"
STAGE02_PATH = REPORTS_DIR / "Stage02_QuadRetroManifest.json"
IMPORT_PATH = REPORTS_DIR / "UnrealImportManifest.json"
VALIDATION_PATH = REPORTS_DIR / "UnrealValidationReport.json"
ARCADE_JSON = PROJECT_DIR / "Content" / "Data" / "ArcadeInteractables.json"
CHARACTER_VISUALS_CSV = PROJECT_DIR / "Content" / "Data" / "CharacterVisuals.csv"
EXPECTED_PARENT = "/Game/Materials/M_Environment_Unlit"


STALE_IN_SCOPE_PATHS = [
    "/Game/World/Interactables/ArcadeMachine/ArcadeMachine.ArcadeMachine",
    "/Game/World/Interactables/Crate.Crate",
    "/Game/World/Interactables/Chests/ChestModel/Chest.Chest",
    "/Game/World/Interactables/Fountain/Fountain.Fountain",
    "/Game/World/Interactables/Totem.Totem",
    "/Game/World/Interactables/SM_IdolAltar.SM_IdolAltar",
    "/Game/World/Interactables/Vending/Vending.Vending",
    "/Game/World/Interactables/Shroom.Shroom",
    "/Game/World/LootBags/Black/SM_LootBag_Black.SM_LootBag_Black",
    "/Game/World/LootBags/Red/SM_LootBag_Red.SM_LootBag_Red",
    "/Game/World/LootBags/Yellow/SM_LootBag_Yellow.SM_LootBag_Yellow",
    "/Game/World/LootBags/White/SM_LootBag_White.SM_LootBag_White",
    "/Game/Characters/NPCs/Gambler/GamblerDemonStand/GamblerDemonStand.GamblerDemonStand",
    "/Game/Characters/NPCs/Saint/SaintNPC/",
    "/Game/Characters/NPCs/Ouroboros/Ouroboros/",
]

BOSS_EXCLUDED_PATHS = [
    "/Game/Characters/NPCs/Gambler/Gambler/Meshy_AI_Axe_Wielding_Patriot_0130203714_texture",
]


def load_json(path):
    with Path(path).open("r", encoding="utf-8") as handle:
        return json.load(handle)


def write_json(path, payload):
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")


def project_file(path_text):
    if not path_text:
        return None
    path = Path(path_text)
    return path if path.is_absolute() else PROJECT_DIR / path


def package_path(object_path):
    return (object_path or "").split(".", 1)[0]


def load_asset(object_path):
    package = package_path(object_path)
    return unreal.EditorAssetLibrary.load_asset(package) if package else None


def asset_path_name(asset):
    return asset.get_path_name() if asset else ""


def parent_path(material):
    try:
        parent = material.get_editor_property("parent")
        return package_path(parent.get_path_name()) if parent else ""
    except Exception:
        return ""


def texture_param(material, name):
    try:
        value = unreal.MaterialEditingLibrary.get_material_instance_texture_parameter_value(material, name)
    except Exception:
        return None
    if isinstance(value, (tuple, list)):
        return value[-1] if value else None
    return value


def validate_generated_files(stage01, stage02):
    entries = []
    errors = []
    stage01_by_row = {entry.get("row_id"): entry for entry in stage01.get("entries", [])}
    for entry in stage02.get("entries", []):
        row_id = entry.get("row_id")
        source_entry = stage01_by_row.get(row_id, {})
        checks = {
            "source_image": bool(project_file(entry.get("source_image")) and project_file(entry.get("source_image")).is_file()),
            "raw_trellis_glb": bool(project_file(entry.get("raw_trellis_glb")) and project_file(entry.get("raw_trellis_glb")).is_file()),
            "trellis_front_render": bool(project_file(entry.get("trellis_front_render")) and project_file(entry.get("trellis_front_render")).is_file()),
            "quad_retro_glb": bool(project_file(entry.get("quad_retro_glb")) and project_file(entry.get("quad_retro_glb")).is_file()),
            "quad_retro_front_render": bool(project_file(entry.get("quad_retro_front_render")) and project_file(entry.get("quad_retro_front_render")).is_file()),
        }
        if not all(checks.values()):
            errors.append(f"{row_id}: missing generated file checks {checks}")
        entries.append({
            "row_id": row_id,
            "category": entry.get("category"),
            "checks": checks,
            "stage01_status": source_entry.get("status") or source_entry.get("trellis_status"),
            "stage02_status": entry.get("quad_retro_status"),
        })
    return entries, errors


def validate_imports(import_manifest):
    entries = []
    errors = []
    for entry in import_manifest.get("entries", []):
        row_id = entry.get("row_id")
        mesh = load_asset(entry.get("unreal_asset_path"))
        material = load_asset(entry.get("material_asset_path"))
        texture = load_asset(entry.get("texture_asset_path"))
        diffuse = texture_param(material, "DiffuseColorMap") if material else None
        base_color = texture_param(material, "BaseColorTexture") if material else None
        material_parent = parent_path(material) if material else ""
        expected_texture = entry.get("texture_asset_path", "")
        result = {
            "row_id": row_id,
            "category": entry.get("category"),
            "mesh": asset_path_name(mesh),
            "material": asset_path_name(material),
            "texture": asset_path_name(texture),
            "material_parent": material_parent,
            "diffuse_texture": asset_path_name(diffuse),
            "base_color_texture": asset_path_name(base_color),
        }
        if not mesh or not isinstance(mesh, unreal.StaticMesh):
            errors.append(f"{row_id}: missing StaticMesh {entry.get('unreal_asset_path')}")
        if not material or not isinstance(material, unreal.MaterialInstanceConstant):
            errors.append(f"{row_id}: missing MaterialInstanceConstant {entry.get('material_asset_path')}")
        if not texture or not isinstance(texture, unreal.Texture2D):
            errors.append(f"{row_id}: missing Texture2D {entry.get('texture_asset_path')}")
        if material and material_parent != EXPECTED_PARENT:
            errors.append(f"{row_id}: material parent is {material_parent}")
        if texture and asset_path_name(diffuse) != expected_texture:
            errors.append(f"{row_id}: DiffuseColorMap is {asset_path_name(diffuse)}")
        if texture and asset_path_name(base_color) != expected_texture:
            errors.append(f"{row_id}: BaseColorTexture is {asset_path_name(base_color)}")
        entries.append(result)
    return entries, errors


def validate_arcade_data(import_manifest):
    arcade = load_json(ARCADE_JSON)
    expected = {
        entry["row_id"]: entry["unreal_asset_path"]
        for entry in import_manifest.get("entries", [])
        if entry.get("row_id") == "Vehicle" or str(entry.get("row_id")).startswith("Arcade_")
    }
    rows = []
    errors = []
    for row in arcade:
        row_id = row.get("Name")
        if row_id not in expected:
            continue
        data = row.get("ArcadeData") or {}
        mesh_path = data.get("DisplayMesh", "")
        exists = bool(load_asset(mesh_path))
        rows.append({
            "row_id": row_id,
            "display_mesh": mesh_path,
            "expected": expected[row_id],
            "asset_exists": exists,
        })
        if mesh_path != expected[row_id]:
            errors.append(f"{row_id}: DisplayMesh is {mesh_path}, expected {expected[row_id]}")
        if not exists:
            errors.append(f"{row_id}: DisplayMesh asset does not resolve")
    return rows, errors


def validate_character_visuals(import_manifest):
    expected = {
        entry["row_id"]: entry["unreal_asset_path"]
        for entry in import_manifest.get("entries", [])
        if entry.get("row_id") in {"CasinoNPC", "Saint", "Ouroboros"}
    }
    rows = []
    errors = []
    with CHARACTER_VISUALS_CSV.open("r", encoding="utf-8", newline="") as handle:
        reader = csv.DictReader(handle)
        for row in reader:
            row_id = row.get("---")
            if row_id not in expected:
                continue
            static_mesh = row.get("StaticMesh", "")
            skeletal_mesh = row.get("SkeletalMesh", "")
            exists = bool(load_asset(static_mesh))
            rows.append({
                "row_id": row_id,
                "skeletal_mesh": skeletal_mesh,
                "static_mesh": static_mesh,
                "expected": expected[row_id],
                "asset_exists": exists,
                "b_loop_animation": row.get("bLoopAnimation", ""),
                "b_auto_ground": row.get("bAutoGroundToActorOrigin", ""),
            })
            if skeletal_mesh:
                errors.append(f"{row_id}: SkeletalMesh should be empty for Quad Retro static mesh")
            if static_mesh != expected[row_id]:
                errors.append(f"{row_id}: StaticMesh is {static_mesh}, expected {expected[row_id]}")
            if not exists:
                errors.append(f"{row_id}: StaticMesh asset does not resolve")
    missing = sorted(set(expected) - {row["row_id"] for row in rows})
    for row_id in missing:
        errors.append(f"{row_id}: missing CharacterVisuals row")
    return rows, errors


def scan_stale_paths():
    scan_roots = [PROJECT_DIR / "Source", PROJECT_DIR / "Content" / "Data"]
    hits = []
    excluded = []
    for root in scan_roots:
        for path in root.rglob("*"):
            if not path.is_file() or path.suffix.lower() not in {".cpp", ".h", ".csv", ".json"}:
                continue
            text = path.read_text(encoding="utf-8", errors="ignore")
            lines = text.splitlines()
            for line_no, line in enumerate(lines, 1):
                normalized = str(path.relative_to(PROJECT_DIR)).replace("\\", "/")
                for pattern in STALE_IN_SCOPE_PATHS:
                    if pattern in line:
                        hits.append({"file": normalized, "line": line_no, "pattern": pattern, "text": line.strip()})
                for pattern in BOSS_EXCLUDED_PATHS:
                    if pattern in line:
                        excluded.append({
                            "file": normalized,
                            "line": line_no,
                            "pattern": pattern,
                            "text": line.strip(),
                            "reason": "boss row/reference excluded by task constraint",
                        })
    return hits, excluded


def main():
    stage01 = load_json(STAGE01_PATH)
    stage02 = load_json(STAGE02_PATH)
    import_manifest = load_json(IMPORT_PATH)

    generated_entries, generated_errors = validate_generated_files(stage01, stage02)
    import_entries, import_errors = validate_imports(import_manifest)
    arcade_rows, arcade_errors = validate_arcade_data(import_manifest)
    character_rows, character_errors = validate_character_visuals(import_manifest)
    stale_hits, excluded_hits = scan_stale_paths()

    errors = generated_errors + import_errors + arcade_errors + character_errors
    if stale_hits:
        errors.append(f"stale in-scope path references remain: {len(stale_hits)}")

    report = {
        "stage": "unreal_validation",
        "output_root": str(RUN_ROOT),
        "summary": {
            "generated_rows": len(generated_entries),
            "imported_rows": len(import_entries),
            "arcade_rows_checked": len(arcade_rows),
            "character_visual_rows_checked": len(character_rows),
            "stale_in_scope_references": len(stale_hits),
            "boss_excluded_legacy_references": len(excluded_hits),
            "errors": len(errors),
        },
        "generated_files": generated_entries,
        "imported_assets": import_entries,
        "arcade_data": arcade_rows,
        "character_visuals": character_rows,
        "stale_in_scope_references": stale_hits,
        "boss_excluded_legacy_references": excluded_hits,
        "errors": errors,
        "updated_utc": datetime.now(timezone.utc).isoformat(),
    }
    write_json(VALIDATION_PATH, report)

    if errors:
        raise RuntimeError("; ".join(errors))

    unreal.log("[VerifyWorldNpcInteractablesRetroBatch01] OK")
    try:
        unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")
    except Exception:
        pass


main()
