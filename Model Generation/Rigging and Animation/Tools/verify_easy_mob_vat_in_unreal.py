r"""
Verify Easy mob vertex-animation assets, data rows, and live fallback visual preservation.

Run with a forward-slash script path:
  UnrealEditor-Cmd.exe T66.uproject -run=pythonscript -script=C:/UE/T66/Model Generation/Rigging and Animation/Tools/verify_easy_mob_vat_in_unreal.py -unattended -nop4 -nosplash -NullRHI
"""

import csv
import json
from pathlib import Path

import unreal


PROJECT_DIR = Path(unreal.SystemLibrary.get_project_directory()).resolve()
CSV_PATH = PROJECT_DIR / "Content" / "Data" / "MobVertexAnimations.csv"
CHARACTER_VISUALS_CSV = PROJECT_DIR / "Content" / "Data" / "CharacterVisuals.csv"
REPORT_PATH = PROJECT_DIR / "Saved" / "EasyMobVATVerifyReport.json"
DATA_TABLE_PATH = "/Game/Data/DT_MobVertexAnimations.DT_MobVertexAnimations"

EXPECTED_EASY_MOBS = [
    "Slime",
    "BoneWalker",
    "RatPack",
    "CaveBat",
    "HexSlinger",
    "TombSpider",
    "StoneSentinel",
    "MimicLure",
    "BoneConjurer",
    "CryptWraith",
]

CLIPS = ("Idle", "Move", "AttackCue", "HitReact", "Death")
EXPECTED_VISUAL_ROOT = "/Game/Characters/Mobs/"


def log(message):
    unreal.log(f"[EasyMobVATVerify] {message}")


def warn(message):
    unreal.log_warning(f"[EasyMobVATVerify] {message}")


def load_asset(path):
    if not path:
        return None
    asset = unreal.EditorAssetLibrary.load_asset(path)
    if not asset:
        asset = unreal.load_asset(path)
    return asset


def read_csv(path):
    with path.open("r", encoding="utf-8", newline="") as handle:
        return list(csv.DictReader(handle))


def vec_to_tuple(value):
    if value is None:
        return (0.0, 0.0, 0.0)
    if isinstance(value, (list, tuple)) and len(value) >= 3:
        return (float(value[0]), float(value[1]), float(value[2]))
    to_tuple = getattr(value, "to_tuple", None)
    if callable(to_tuple):
        raw = to_tuple()
        if len(raw) >= 3:
            return (float(raw[0]), float(raw[1]), float(raw[2]))
    return (
        float(getattr(value, "x", getattr(value, "X", 0.0))),
        float(getattr(value, "y", getattr(value, "Y", 0.0))),
        float(getattr(value, "z", getattr(value, "Z", 0.0))),
    )


def get_bake_vector(data_asset, property_names):
    for property_name in property_names:
        try:
            return data_asset.get_editor_property(property_name)
        except Exception:
            continue
    raise RuntimeError(f"Could not read any bake vector property from {property_names}")


def csv_vec_to_tuple(value):
    if not value:
        return (0.0, 0.0, 0.0)
    normalized = value.strip().strip("()")
    parts = {}
    for item in normalized.split(","):
        if "=" not in item:
            continue
        key, raw = item.split("=", 1)
        try:
            parts[key.strip().lower()] = float(raw)
        except ValueError:
            parts[key.strip().lower()] = 0.0
    return (parts.get("x", 0.0), parts.get("y", 0.0), parts.get("z", 0.0))


def magnitude3(values):
    return abs(values[0]) + abs(values[1]) + abs(values[2])


def object_path(package_path):
    name = package_path.rsplit("/", 1)[-1]
    return f"{package_path}.{name}"


def get_texture_dimensions(texture):
    for pair in (("blueprint_get_size_x", "blueprint_get_size_y"), ("get_size_x", "get_size_y")):
        get_x = getattr(texture, pair[0], None)
        get_y = getattr(texture, pair[1], None)
        if callable(get_x) and callable(get_y):
            try:
                return int(get_x()), int(get_y())
            except Exception:
                pass
    return (0, 0)


def get_material_texture_path(material, parameter_name):
    try:
        value = unreal.MaterialEditingLibrary.get_material_instance_texture_parameter_value(material, parameter_name)
    except Exception:
        return ""
    return value.get_path_name() if value else ""


def get_material_scalar(material, parameter_name):
    try:
        return float(unreal.MaterialEditingLibrary.get_material_instance_scalar_parameter_value(material, parameter_name))
    except Exception:
        return None


def get_material_vector(material, parameter_name):
    try:
        value = unreal.MaterialEditingLibrary.get_material_instance_vector_parameter_value(material, parameter_name)
    except Exception:
        return None
    if value is None:
        return None
    return tuple(float(component) for component in (value.r, value.g, value.b, value.a))


def get_static_mesh_uv_channels(static_mesh):
    channels = {}
    try:
        lods = int(static_mesh.get_num_lods())
    except Exception:
        lods = 1
    for lod_index in range(max(1, lods)):
        count = 0
        while count < 8:
            try:
                static_mesh.get_uv_channel_data(lod_index, count)
                count += 1
            except Exception:
                break
        channels[str(lod_index)] = count
    return channels


def find_data_asset_for_enemy(enemy_id):
    return load_asset(object_path(f"/Game/Characters/MobsVAT/{enemy_id}/DA_EasyMobVAT_{enemy_id}"))


def verify_enemy(row, visual_row):
    enemy_id = row["EnemyID"]
    result = {"enemy_id": enemy_id, "errors": [], "warnings": []}

    static_mesh = load_asset(row["StaticMesh"])
    material = load_asset(row["Material"])
    position_texture = load_asset(row["PositionTexture"])
    normal_texture = load_asset(row["NormalTexture"])
    data_asset = find_data_asset_for_enemy(enemy_id)

    for label, asset in (
        ("static_mesh", static_mesh),
        ("material", material),
        ("position_texture", position_texture),
        ("normal_texture", normal_texture),
        ("data_asset", data_asset),
    ):
        result[label] = asset.get_path_name() if asset else ""
        if not asset:
            result["errors"].append(f"Missing {label}")

    if visual_row:
        result["fallback_static_mesh"] = visual_row.get("StaticMesh", "")
        result["fallback_texture"] = visual_row.get("PixelatedTextureAssetPath", "")
        result["fallback_scale"] = visual_row.get("MeshRelativeScale", "")
        if not result["fallback_static_mesh"].startswith(f"{EXPECTED_VISUAL_ROOT}{enemy_id}/"):
            result["errors"].append("CharacterVisuals fallback static mesh was not preserved")
        if row.get("PixelatedTextureAssetPath") != visual_row.get("PixelatedTextureAssetPath"):
            result["errors"].append("VAT row texture does not match live CharacterVisuals texture")
        if row.get("MeshRelativeScale") != visual_row.get("MeshRelativeScale"):
            result["errors"].append("VAT row scale does not match live CharacterVisuals scale")
    else:
        result["errors"].append("Missing CharacterVisuals row")

    result["b_enabled"] = row.get("bEnabled", "").lower() == "true"
    if not result["b_enabled"]:
        result["errors"].append("VAT row is disabled")

    csv_min = csv_vec_to_tuple(row.get("MinBBox", ""))
    csv_size = csv_vec_to_tuple(row.get("SizeBBox", ""))
    result["csv_min_bbox"] = csv_min
    result["csv_size_bbox"] = csv_size
    if magnitude3(csv_size) <= 0.0001:
        result["errors"].append("CSV SizeBBox is zero")

    if data_asset:
        try:
            data_num_frames = int(data_asset.get_editor_property("num_frames"))
            data_rows = int(data_asset.get_editor_property("vertex_rows_per_frame"))
            data_min = vec_to_tuple(get_bake_vector(data_asset, ("vertex_min_bbox", "vertex_min_b_box")))
            data_size = vec_to_tuple(get_bake_vector(data_asset, ("vertex_size_bbox", "vertex_size_b_box")))
        except Exception as exc:
            result["errors"].append(f"Could not read data asset bake fields: {exc}")
            data_num_frames = 0
            data_rows = 0
            data_min = (0.0, 0.0, 0.0)
            data_size = (0.0, 0.0, 0.0)
        result["data_num_frames"] = data_num_frames
        result["data_rows_per_frame"] = data_rows
        result["data_min_bbox"] = data_min
        result["data_size_bbox"] = data_size
        if data_num_frames <= 0:
            result["errors"].append("Data asset num_frames is zero")
        if data_rows <= 0:
            result["errors"].append("Data asset vertex_rows_per_frame is zero")
        if magnitude3(data_size) <= 0.0001:
            result["warnings"].append("Python data asset SizeBBox read is zero; using material parameters as bake bounds authority")
        if int(row.get("NumFrames", "0") or "0") != data_num_frames:
            result["errors"].append("CSV NumFrames does not match data asset")
        if int(row.get("RowsPerFrame", "0") or "0") != data_rows:
            result["errors"].append("CSV RowsPerFrame does not match data asset")
        if magnitude3(data_size) > 0.0001 and magnitude3(csv_size) > 0.0001 and any(abs(csv_size[i] - data_size[i]) > 0.01 for i in range(3)):
            result["warnings"].append("CSV SizeBBox differs from data asset")

    if position_texture:
        result["position_texture_dimensions"] = get_texture_dimensions(position_texture)
        if result["position_texture_dimensions"][0] <= 0 or result["position_texture_dimensions"][1] <= 0:
            result["errors"].append("Position texture has invalid dimensions")
    if normal_texture:
        result["normal_texture_dimensions"] = get_texture_dimensions(normal_texture)
        if result["normal_texture_dimensions"][0] <= 0 or result["normal_texture_dimensions"][1] <= 0:
            result["errors"].append("Normal texture has invalid dimensions")

    if material:
        result["material_position_texture"] = get_material_texture_path(material, "PositionTexture")
        result["material_base_texture"] = get_material_texture_path(material, "BaseColorTexture")
        result["material_frame"] = get_material_scalar(material, "Frame")
        result["material_rows_per_frame"] = get_material_scalar(material, "RowsPerFrame")
        result["material_min_bbox"] = get_material_vector(material, "MinBBox")
        result["material_size_bbox"] = get_material_vector(material, "SizeBBox")
        if position_texture and result["material_position_texture"] != position_texture.get_path_name():
            result["errors"].append("Material PositionTexture parameter does not match row")
        if result["material_rows_per_frame"] is not None and result.get("data_rows_per_frame", 0) > 0:
            if abs(result["material_rows_per_frame"] - result["data_rows_per_frame"]) > 0.01:
                result["errors"].append("Material RowsPerFrame does not match data asset")
        if result["material_size_bbox"] is not None and magnitude3(result["material_size_bbox"][:3]) <= 0.0001:
            result["errors"].append("Material SizeBBox parameter is zero")
        if result["material_min_bbox"] is not None and any(abs(csv_min[i] - result["material_min_bbox"][i]) > 0.01 for i in range(3)):
            result["errors"].append("CSV MinBBox does not match material parameter")
        if result["material_size_bbox"] is not None and any(abs(csv_size[i] - result["material_size_bbox"][i]) > 0.01 for i in range(3)):
            result["errors"].append("CSV SizeBBox does not match material parameter")

    if static_mesh:
        result["static_mesh_uv_channels"] = get_static_mesh_uv_channels(static_mesh)
        if int(result["static_mesh_uv_channels"].get("0", 0)) < 3:
            result["warnings"].append("Could not prove static mesh has UV channel 2 through Python")

    for clip in CLIPS:
        start = int(row.get(f"{clip}StartFrame", "-1") or "-1")
        end = int(row.get(f"{clip}EndFrame", "-1") or "-1")
        play_rate = float(row.get(f"{clip}PlayRate", "0") or "0")
        result[f"{clip.lower()}_range"] = [start, end, play_rate]
        if start < 0 or end < start:
            result["errors"].append(f"{clip} frame range is invalid")
        if result.get("data_num_frames") and end >= result["data_num_frames"]:
            result["errors"].append(f"{clip} end frame exceeds data asset num_frames")
        if play_rate <= 0:
            result["errors"].append(f"{clip} play rate is invalid")

    return result


def main():
    log("=== Easy mob VAT verification start ===")
    if not CSV_PATH.is_file():
        raise RuntimeError(f"Missing {CSV_PATH}")

    vat_rows = {row["EnemyID"]: row for row in read_csv(CSV_PATH)}
    visual_rows = {row["---"]: row for row in read_csv(CHARACTER_VISUALS_CSV)}
    data_table = load_asset(DATA_TABLE_PATH)
    data_table_rows = []
    if data_table:
        try:
            data_table_rows = [str(name) for name in unreal.DataTableFunctionLibrary.get_data_table_row_names(data_table)]
        except Exception as exc:
            data_table_rows = []
            warn(f"Could not inspect {DATA_TABLE_PATH}: {exc}")
    report = {
        "csv": str(CSV_PATH),
        "data_table": DATA_TABLE_PATH,
        "data_table_loaded": bool(data_table),
        "data_table_row_count": len(data_table_rows),
        "data_table_rows": data_table_rows,
        "expected_mobs": EXPECTED_EASY_MOBS,
        "row_count": len(vat_rows),
        "missing_rows": [enemy_id for enemy_id in EXPECTED_EASY_MOBS if enemy_id not in vat_rows],
        "extra_rows": [enemy_id for enemy_id in sorted(vat_rows) if enemy_id not in EXPECTED_EASY_MOBS],
        "mobs": [],
        "errors": [],
        "warnings": [],
    }
    if not data_table:
        report["errors"].append(f"Missing data table: {DATA_TABLE_PATH}")
    elif sorted(data_table_rows) != sorted(EXPECTED_EASY_MOBS):
        report["errors"].append(f"DT_MobVertexAnimations rows do not match expected Easy mobs: {data_table_rows}")
    if report["missing_rows"]:
        report["errors"].append(f"Missing VAT rows: {report['missing_rows']}")
    if report["extra_rows"]:
        report["warnings"].append(f"Extra VAT rows: {report['extra_rows']}")

    for enemy_id in EXPECTED_EASY_MOBS:
        row = vat_rows.get(enemy_id)
        if not row:
            continue
        mob_result = verify_enemy(row, visual_rows.get(enemy_id))
        report["mobs"].append(mob_result)
        for error in mob_result["errors"]:
            report["errors"].append(f"{enemy_id}: {error}")
        for warning in mob_result["warnings"]:
            report["warnings"].append(f"{enemy_id}: {warning}")

    report["passed"] = not report["errors"]
    REPORT_PATH.parent.mkdir(parents=True, exist_ok=True)
    REPORT_PATH.write_text(json.dumps(report, indent=2, sort_keys=True), encoding="utf-8")
    log(f"Wrote {REPORT_PATH}")
    if report["errors"]:
        for error in report["errors"]:
            warn(error)
        raise RuntimeError(f"Easy mob VAT verification failed with {len(report['errors'])} errors")
    for warning in report["warnings"]:
        warn(warning)
    log("=== Easy mob VAT verification passed ===")


if __name__ == "__main__":
    main()
