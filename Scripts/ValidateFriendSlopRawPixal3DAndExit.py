"""
Validate FriendSlop Easy raw Pixal3D Unreal imports and runtime data references.

This verifier intentionally checks raw StaticMesh imports only. It does not
expect ToonStyle outlines, tint textures, inner-line textures, or processed GLBs.
"""

from __future__ import annotations

import csv
import json
import os
import re
from pathlib import Path

import unreal


LOG_PREFIX = "[ValidateFriendSlopRawPixal3D]"
REPO_ROOT = Path(__file__).resolve().parents[1]
RUN_ROOT = (
    REPO_ROOT
    / "Model Generation"
    / "Runs"
    / "Pixal3D"
    / "FriendSlopEasyBatch_20260604_1532"
)
DEFAULT_MANIFEST = RUN_ROOT / "FriendSlopEasyBatch_20260604_1532_manifest.json"
REPORT_PATH = (
    REPO_ROOT
    / "Reports"
    / "AgentReviews"
    / "FriendSlopEasyPixal3D"
    / "raw_runtime_reference_validation.json"
)
PARENT_MATERIAL = "/Game/Materials/M_GLB_Unlit.M_GLB_Unlit"
HERO1_RAW_STATIC_REF = (
    "/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/"
    "SM_Hero_1_Chad_Male.SM_Hero_1_Chad_Male"
)
HERO1_ROWS = ("Hero_1_Chad", "Hero_1_Chad_DemoSkin")
HERO1_EXPECTED_YAW = 90.0
MOB_VAT_DISABLED_ROWS = (
    "Slime",
    "CaveBat",
    "BoneWalker",
    "RatPack",
    "TombSpider",
    "HexSlinger",
    "StoneSentinel",
    "MimicLure",
    "BoneConjurer",
    "CryptWraith",
)


def load_json(path: Path) -> object:
    return json.loads(path.read_text(encoding="utf-8"))


def package_from_ref(ref: object) -> str:
    text = str(ref or "").strip().strip('"')
    if not text:
        return ""
    return text.split(".", 1)[0]


def object_ref(target_dir: str, asset_id: str) -> str:
    name = f"SM_{asset_id}"
    return f"{target_dir.rstrip('/')}/{name}.{name}"


def texture_ref(target_dir: str, asset_id: str) -> str:
    name = f"T_{asset_id}_BaseColor"
    return f"{target_dir.rstrip('/')}/Textures/{name}.{name}"


def material_ref(target_dir: str, asset_id: str) -> str:
    name = f"MI_SM_{asset_id}"
    return f"{target_dir.rstrip('/')}/Materials/{name}.{name}"


def load_static_mesh(ref: str) -> tuple[bool, str]:
    package = package_from_ref(ref)
    if not package:
        return False, ""
    loaded = unreal.EditorAssetLibrary.load_asset(package)
    cls = loaded.get_class().get_name() if loaded else ""
    return bool(loaded and isinstance(loaded, unreal.StaticMesh)), cls


def path_name(asset: object) -> str:
    return asset.get_path_name() if asset else ""


def load_asset(ref: str) -> object:
    package = package_from_ref(ref)
    return unreal.EditorAssetLibrary.load_asset(package) if package else None


def texture_param(material: object, name: str) -> str:
    try:
        value = unreal.MaterialEditingLibrary.get_material_instance_texture_parameter_value(material, name)
    except Exception:
        return ""
    if isinstance(value, (tuple, list)):
        value = value[-1] if value else None
    return path_name(value)


def material_slot_paths(mesh: unreal.StaticMesh) -> list[str]:
    paths: list[str] = []
    try:
        static_materials = list(mesh.get_editor_property("static_materials") or [])
    except Exception:
        static_materials = []
    for entry in static_materials:
        try:
            material = entry.get_editor_property("material_interface")
        except Exception:
            material = None
        paths.append(path_name(material))
    return paths


def read_csv_rows(path: Path) -> dict[str, dict[str, str]]:
    with path.open("r", encoding="utf-8-sig", newline="") as handle:
        reader = csv.DictReader(handle)
        return {str(row.get("---", "")).strip(): row for row in reader if row.get("---")}


def parse_yaw(rotation: str) -> float | None:
    match = re.search(r"Yaw\s*=\s*([-+]?\d+(?:\.\d+)?)", str(rotation or ""))
    return float(match.group(1)) if match else None


def validate_static_mesh_assets(manifest: dict) -> list[dict[str, object]]:
    rows: list[dict[str, object]] = []
    for asset in manifest.get("assets", []):
        asset_id = str(asset.get("asset_id", "")).strip()
        target_dir = str(asset.get("target_dir", "")).strip()
        ref = object_ref(target_dir, asset_id)
        loaded = load_asset(ref)
        ok = bool(loaded and isinstance(loaded, unreal.StaticMesh))
        cls = loaded.get_class().get_name() if loaded else ""
        expected_material = material_ref(target_dir, asset_id)
        expected_texture = texture_ref(target_dir, asset_id)
        material = load_asset(expected_material)
        texture = load_asset(expected_texture)
        material_parent = ""
        base_color = ""
        diffuse = ""
        slot_paths: list[str] = []
        if material:
            try:
                material_parent = path_name(material.get_editor_property("parent"))
            except Exception:
                pass
            base_color = texture_param(material, "BaseColorTexture")
            diffuse = texture_param(material, "DiffuseColorMap")
        if ok:
            slot_paths = material_slot_paths(loaded)
        rows.append(
            {
                "asset_id": asset_id,
                "ref": ref,
                "class": cls,
                "ok": ok,
                "expected_material": expected_material,
                "material": path_name(material),
                "material_parent": material_parent,
                "mesh_material_slots": slot_paths,
                "expected_texture": expected_texture,
                "texture": path_name(texture),
                "base_color_texture": base_color,
                "diffuse_texture": diffuse,
                "material_ok": path_name(material) == expected_material,
                "texture_ok": path_name(texture) == expected_texture,
                "base_color_ok": base_color == expected_texture,
                "diffuse_ok": diffuse == expected_texture,
                "mesh_slots_ok": bool(slot_paths) and all(path == expected_material for path in slot_paths),
            }
        )
    return rows


def validate_character_visuals(manifest: dict) -> list[dict[str, object]]:
    csv_rows = read_csv_rows(REPO_ROOT / "Content" / "Data" / "CharacterVisuals.csv")
    results: list[dict[str, object]] = []
    for asset in manifest.get("assets", []):
        asset_id = str(asset.get("asset_id", "")).strip()
        if asset_id not in csv_rows:
            continue
        expected = object_ref(str(asset.get("target_dir", "")).strip(), asset_id)
        actual = str(csv_rows[asset_id].get("StaticMesh", "")).strip()
        mesh_ok, cls = load_static_mesh(actual)
        results.append(
            {
                "row": asset_id,
                "expected": expected,
                "actual": actual,
                "path_ok": actual == expected,
                "loads_as_static_mesh": mesh_ok,
                "class": cls,
            }
        )
    return results


def validate_hero1_raw_rows() -> list[dict[str, object]]:
    csv_rows = read_csv_rows(REPO_ROOT / "Content" / "Data" / "CharacterVisuals.csv")
    results: list[dict[str, object]] = []
    for row_id in HERO1_ROWS:
        row = csv_rows.get(row_id, {})
        actual_static = str(row.get("StaticMesh", "")).strip()
        actual_skeletal = str(row.get("SkeletalMesh", "")).strip()
        actual_rotation = str(row.get("MeshRelativeRotation", "")).strip()
        actual_yaw = parse_yaw(actual_rotation)
        mesh_ok, cls = load_static_mesh(actual_static)
        animation_values = {
            key: str(row.get(key, "")).strip()
            for key in ("WalkAnimation", "IdleAnimation", "JumpAnimation", "RollAnimation")
        }
        results.append(
            {
                "row": row_id,
                "expected_static": HERO1_RAW_STATIC_REF,
                "actual_static": actual_static,
                "static_path_ok": actual_static == HERO1_RAW_STATIC_REF,
                "loads_as_static_mesh": mesh_ok,
                "class": cls,
                "skeletal_mesh_blank": not actual_skeletal,
                "expected_yaw": HERO1_EXPECTED_YAW,
                "actual_rotation": actual_rotation,
                "actual_yaw": actual_yaw,
                "yaw_ok": actual_yaw is not None and abs(actual_yaw - HERO1_EXPECTED_YAW) <= 0.01,
                "animation_values": animation_values,
                "animations_blank": all(not value for value in animation_values.values()),
            }
        )
    return results


def validate_mob_vat_disabled_rows() -> list[dict[str, object]]:
    csv_rows = read_csv_rows(REPO_ROOT / "Content" / "Data" / "MobVertexAnimations.csv")
    results: list[dict[str, object]] = []
    for row_id in MOB_VAT_DISABLED_ROWS:
        row = csv_rows.get(row_id, {})
        actual = str(row.get("bEnabled", "")).strip().lower()
        results.append(
            {
                "row": row_id,
                "actual_b_enabled": actual,
                "disabled": actual == "false",
            }
        )
    return results


def validate_pet_rows(manifest: dict) -> list[dict[str, object]]:
    csv_rows = read_csv_rows(REPO_ROOT / "Content" / "Data" / "Pets.csv")
    results: list[dict[str, object]] = []
    for asset in manifest.get("assets", []):
        asset_id = str(asset.get("asset_id", "")).strip()
        if asset_id not in csv_rows:
            continue
        expected = object_ref(str(asset.get("target_dir", "")).strip(), asset_id)
        actual = str(csv_rows[asset_id].get("CaptureVisualMesh", "")).strip()
        mesh_ok, cls = load_static_mesh(actual)
        results.append(
            {
                "row": asset_id,
                "expected": expected,
                "actual": actual,
                "path_ok": actual == expected,
                "loads_as_static_mesh": mesh_ok,
                "class": cls,
            }
        )
    return results


def row_payload(row: dict[str, object]) -> dict[str, object]:
    for key in ("VehicleData", "PropData"):
        nested = row.get(key)
        if isinstance(nested, dict):
            return nested
    return row


def validate_json_display_meshes(path: Path, expected_refs: set[str]) -> list[dict[str, object]]:
    data = load_json(path)
    results: list[dict[str, object]] = []
    rows = data if isinstance(data, list) else []
    for index, row in enumerate(rows):
        if not isinstance(row, dict):
            continue
        payload = row_payload(row)
        ref = payload.get("DisplayMesh", "")
        if not ref:
            continue
        if str(ref).strip() not in expected_refs:
            continue
        mesh_ok, cls = load_static_mesh(str(ref))
        results.append(
            {
                "file": str(path.relative_to(REPO_ROOT)).replace("\\", "/"),
                "index": index,
                "id": (
                    payload.get("RowName")
                    or payload.get("ID")
                    or payload.get("VehicleID")
                    or payload.get("PropID")
                    or row.get("Name")
                    or ""
                ),
                "ref": ref,
                "loads_as_static_mesh": mesh_ok,
                "class": cls,
            }
        )
    return results


def main() -> None:
    unreal.log(f"{LOG_PREFIX} START")
    manifest_path = Path(os.environ.get("T66_PIXAL3D_MANIFEST", str(DEFAULT_MANIFEST)))
    manifest = load_json(manifest_path)
    expected_refs = {
        object_ref(str(asset.get("target_dir", "")).strip(), str(asset.get("asset_id", "")).strip())
        for asset in manifest.get("assets", [])
    }

    imported_assets = validate_static_mesh_assets(manifest)
    character_visuals = validate_character_visuals(manifest)
    hero1_raw_rows = validate_hero1_raw_rows()
    mob_vat_disabled_rows = validate_mob_vat_disabled_rows()
    pets = validate_pet_rows(manifest)
    vehicle_refs = validate_json_display_meshes(REPO_ROOT / "Content" / "Data" / "VehicleInteractables.json", expected_refs)
    world_visual_props = validate_json_display_meshes(REPO_ROOT / "Content" / "Data" / "WorldVisualProps.json", expected_refs)

    errors: list[str] = []
    for row in imported_assets:
        if not row["ok"]:
            errors.append(f"Missing imported StaticMesh: {row['ref']}")
        if row["material_parent"] and row["material_parent"] != PARENT_MATERIAL:
            errors.append(f"{row['asset_id']}: material parent is {row['material_parent']}")
        for key, label in (
            ("material_ok", "material"),
            ("texture_ok", "texture"),
            ("base_color_ok", "BaseColorTexture"),
            ("diffuse_ok", "DiffuseColorMap"),
            ("mesh_slots_ok", "mesh material slots"),
        ):
            if not row[key]:
                errors.append(f"{row['asset_id']}: {label} not bound to raw Pixal3D texture/material")
    for row in character_visuals:
        if not row["path_ok"] or not row["loads_as_static_mesh"]:
            errors.append(f"CharacterVisuals {row['row']} mismatch or unloadable: {row['actual']}")
    for row in hero1_raw_rows:
        if not row["static_path_ok"] or not row["loads_as_static_mesh"]:
            errors.append(f"{row['row']} does not point to raw FriendSlop Hero 1 mesh: {row['actual_static']}")
        if not row["skeletal_mesh_blank"]:
            errors.append(f"{row['row']} still has SkeletalMesh set")
        if not row["yaw_ok"]:
            errors.append(f"{row['row']} raw FriendSlop hero yaw is {row['actual_yaw']}, expected {row['expected_yaw']}")
        if not row["animations_blank"]:
            errors.append(f"{row['row']} still has animation assets set")
    for row in mob_vat_disabled_rows:
        if not row["disabled"]:
            errors.append(f"MobVertexAnimations {row['row']} bEnabled is {row['actual_b_enabled']}, expected false")
    for row in pets:
        if not row["path_ok"] or not row["loads_as_static_mesh"]:
            errors.append(f"Pets {row['row']} mismatch or unloadable: {row['actual']}")
    for group_name, rows in (("VehicleInteractables", vehicle_refs), ("WorldVisualProps", world_visual_props)):
        for row in rows:
            if not row["loads_as_static_mesh"]:
                errors.append(f"{group_name} ref unloadable: {row['ref']}")

    report = {
        "ok": not errors,
        "manifest": str(manifest_path),
        "notes": [
            "Validation expects raw Pixal3D StaticMesh imports.",
            "ToonStyle outline/material/texture validators are intentionally not used for this FriendSlop raw import pass.",
        ],
        "imported_assets": imported_assets,
        "character_visuals": character_visuals,
        "hero1_raw_rows": hero1_raw_rows,
        "mob_vat_disabled_rows": mob_vat_disabled_rows,
        "pets": pets,
        "vehicle_refs": vehicle_refs,
        "world_visual_props": world_visual_props,
        "errors": errors,
    }
    REPORT_PATH.parent.mkdir(parents=True, exist_ok=True)
    REPORT_PATH.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    unreal.log(f"{LOG_PREFIX} Wrote {REPORT_PATH}")
    if errors:
        for error in errors:
            unreal.log_error(f"{LOG_PREFIX} {error}")
    else:
        unreal.log(f"{LOG_PREFIX} OK")

    try:
        unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")
    except Exception as exc:
        unreal.log_warning(f"{LOG_PREFIX} Failed to request QUIT_EDITOR: {exc}")


main()
