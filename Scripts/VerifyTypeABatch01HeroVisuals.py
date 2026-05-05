"""
Verify Chad Batch01 hero visual rows and imported assets.

Run with:
  UnrealEditor-Cmd.exe T66.uproject -run=pythonscript -script=Scripts/VerifyTypeABatch01HeroVisuals.py
"""

import csv
import json
import os
import sys

import unreal


ACTIVE_HERO_IDS = (1, 3, 4, 5, 7, 8, 9, 11, 12, 13, 14, 15)
CSV_RELATIVE = ("Content", "Data", "CharacterVisuals.csv")
DT_PATH = "/Game/Data/DT_CharacterVisuals"
CHARACTER_MASTER_MATERIAL = "/Game/Materials/M_Character_Unlit"
TEXTURE_PARAMS = ("DiffuseColorMap", "BaseColorTexture")
OUTPUT_REPORT = os.path.join(
    unreal.SystemLibrary.get_project_directory(),
    "Saved",
    "ChadBatch01HeroVisualVerification.json",
)


def _clean_path(object_path):
    return (object_path or "").split(".", 1)[0]


def _load_asset(object_path):
    return unreal.EditorAssetLibrary.load_asset(_clean_path(object_path))


def _bounds_height_cm(mesh):
    bounds = mesh.get_bounds()
    return float(bounds.box_extent.z) * 2.0


def _bounds_report(mesh):
    bounds = mesh.get_bounds()
    origin_z = float(bounds.origin.z)
    extent_z = float(bounds.box_extent.z)
    return {
        "origin_z_cm": origin_z,
        "extent_z_cm": extent_z,
        "height_cm": extent_z * 2.0,
        "bottom_z_cm": origin_z - extent_z,
        "top_z_cm": origin_z + extent_z,
    }


def _asset_class(asset):
    return asset.get_class().get_name() if asset else ""


def _object_path_matches(asset, object_path):
    return asset and asset.get_path_name() == object_path


def _texture_param(material, param_name):
    try:
        result = unreal.MaterialEditingLibrary.get_material_instance_texture_parameter_value(material, param_name)
        if isinstance(result, (tuple, list)):
            return result[-1] if result else None
        return result
    except Exception:
        return None


def _material_texture_ok(material):
    if not material:
        return False, ""
    for param in TEXTURE_PARAMS:
        texture = _texture_param(material, param)
        if texture and isinstance(texture, unreal.Texture):
            path = texture.get_path_name()
            if not path.startswith("/Engine/"):
                return True, path
    return False, material.get_path_name()


def _skeletal_materials(mesh):
    try:
        slots = mesh.get_editor_property("materials")
    except Exception:
        return []
    materials = []
    for slot in slots:
        try:
            materials.append(slot.get_editor_property("material_interface"))
        except Exception:
            materials.append(None)
    return materials


def _row_id(hero_id, beachgoer):
    suffix = "_Chad_Beachgoer" if beachgoer else "_Chad"
    return f"Hero_{hero_id}{suffix}"


def _read_csv_rows():
    project_dir = unreal.SystemLibrary.get_project_directory()
    csv_path = os.path.join(project_dir, *CSV_RELATIVE)
    with open(csv_path, "r", encoding="utf-8", newline="") as handle:
        return {row["---"]: row for row in csv.DictReader(handle)}


def _data_table_row_names():
    dt = unreal.EditorAssetLibrary.load_asset(DT_PATH)
    if not dt:
        raise RuntimeError(f"Missing DataTable: {DT_PATH}")
    return {str(name) for name in unreal.DataTableFunctionLibrary.get_data_table_row_names(dt)}


def _verify_master_material():
    errors = []
    material = unreal.EditorAssetLibrary.load_asset(CHARACTER_MASTER_MATERIAL)
    if not material or not isinstance(material, unreal.Material):
        return {"ok": False, "errors": [f"Missing character master material: {CHARACTER_MASTER_MATERIAL}"]}

    try:
        blend_mode = material.get_editor_property("blend_mode")
        if blend_mode != unreal.BlendMode.BLEND_MASKED:
            errors.append(f"M_Character_Unlit blend mode is not masked: {blend_mode}")
    except Exception as exc:
        errors.append(f"Could not read M_Character_Unlit blend mode: {exc}")

    try:
        opacity_node = unreal.MaterialEditingLibrary.get_material_property_input_node(
            material,
            unreal.MaterialProperty.MP_OPACITY_MASK,
        )
        opacity_output = unreal.MaterialEditingLibrary.get_material_property_input_node_output_name(
            material,
            unreal.MaterialProperty.MP_OPACITY_MASK,
        )
        if not opacity_node or str(opacity_output) != "A":
            errors.append("M_Character_Unlit opacity mask is not driven by texture alpha")
    except Exception as exc:
        errors.append(f"Could not inspect M_Character_Unlit opacity mask: {exc}")

    return {
        "ok": not errors,
        "material": material.get_path_name(),
        "errors": errors,
    }


def _verify_row(row_id, row):
    errors = []
    mesh = _load_asset(row.get("SkeletalMesh", ""))
    if _asset_class(mesh) != "SkeletalMesh":
        errors.append(f"SkeletalMesh missing or wrong class: {row.get('SkeletalMesh')}")
        return {"row_id": row_id, "ok": False, "errors": errors}

    bounds = _bounds_report(mesh)
    height_cm = bounds["height_cm"]
    if not 195.0 <= height_cm <= 205.0:
        errors.append(f"SkeletalMesh height outside Chad tolerance: {height_cm:.2f} cm")
    if not -2.0 <= bounds["bottom_z_cm"] <= 2.0:
        errors.append(f"SkeletalMesh feet/pivot bottom is not near Z=0: {bounds['bottom_z_cm']:.2f} cm")

    material_reports = []
    for index, material in enumerate(_skeletal_materials(mesh)):
        ok, detail = _material_texture_ok(material)
        material_reports.append(
            {
                "slot": index,
                "material": material.get_path_name() if material else "",
                "texture": detail,
                "ok": ok,
            }
        )
        if not ok:
            errors.append(f"Material slot {index} has no imported diffuse/base-color texture: {detail}")

    skeleton = None
    try:
        skeleton = mesh.get_editor_property("skeleton")
    except Exception as exc:
        errors.append(f"Could not read mesh skeleton: {exc}")

    anim_paths = [
        row.get("LoopingAnimation", ""),
        row.get("AlertAnimation", ""),
        row.get("RunAnimation", ""),
    ]
    for label, anim_path in zip(("LoopingAnimation", "AlertAnimation", "RunAnimation"), anim_paths):
        anim = _load_asset(anim_path)
        if _asset_class(anim) != "AnimSequence":
            errors.append(f"{label} missing or wrong class: {anim_path}")
            continue
        anim_skeleton = None
        try:
            anim_skeleton = anim.get_editor_property("skeleton")
        except Exception as exc:
            errors.append(f"Could not read {label} skeleton: {exc}")
        if skeleton and anim_skeleton and anim_skeleton != skeleton:
            errors.append(f"{label} skeleton does not match mesh skeleton: {anim_path}")

    return {
        "row_id": row_id,
        "ok": not errors,
        "height_cm": height_cm,
        "bounds": bounds,
        "materials": material_reports,
        "mesh": row.get("SkeletalMesh", ""),
        "animation": row.get("AlertAnimation", ""),
        "errors": errors,
    }


def main():
    unreal.log("[VerifyTypeABatch01HeroVisuals] Starting")
    rows = _read_csv_rows()
    dt_rows = _data_table_row_names()

    checks = []
    errors = []
    master_material = _verify_master_material()
    errors.extend(master_material["errors"])
    for hero_id in ACTIVE_HERO_IDS:
        for beachgoer in (False, True):
            row_id = _row_id(hero_id, beachgoer)
            if row_id not in rows:
                errors.append(f"Missing CSV row: {row_id}")
                continue
            if row_id not in dt_rows:
                errors.append(f"Missing DataTable row: {row_id}")
            result = _verify_row(row_id, rows[row_id])
            checks.append(result)
            errors.extend(f"{row_id}: {error}" for error in result["errors"])

    report = {
        "checked_rows": len(checks),
        "expected_rows": len(ACTIVE_HERO_IDS) * 2,
        "ok": not errors and len(checks) == len(ACTIVE_HERO_IDS) * 2,
        "errors": errors,
        "master_material": master_material,
        "checks": checks,
    }
    os.makedirs(os.path.dirname(OUTPUT_REPORT), exist_ok=True)
    with open(OUTPUT_REPORT, "w", encoding="utf-8") as handle:
        json.dump(report, handle, indent=2)

    unreal.log(f"[VerifyTypeABatch01HeroVisuals] Wrote {OUTPUT_REPORT}")
    if not report["ok"]:
        for error in errors:
            unreal.log_error(f"[VerifyTypeABatch01HeroVisuals] {error}")
        raise RuntimeError(f"Chad hero visual verification failed: {len(errors)} error(s)")
    unreal.log("[VerifyTypeABatch01HeroVisuals] All Chad hero visuals verified")


if __name__ == "__main__":
    main()
