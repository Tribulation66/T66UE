import csv
import json
import os

import unreal


PROJECT_ROOT = r"C:\UE\T66"
OUT_PATH = os.path.join(PROJECT_ROOT, "Saved", "Codex", "Performance", "B13_MobHISM", "vat_material_audit.json")
CSV_PATH = os.path.join(PROJECT_ROOT, "Content", "Data", "MobVertexAnimations.csv")


def asset_path_from_object_path(value):
    if not value:
        return ""
    text = str(value)
    if "." in text:
        return text.split(".", 1)[0]
    return text


def object_path(value):
    if not value:
        return ""
    try:
        return value.get_path_name()
    except Exception:
        return str(value)


def material_expressions(material):
    if not material:
        return []
    expressions = []
    for attr in ("expressions", "Expressions"):
        try:
            value = material.get_editor_property(attr)
            if value:
                expressions = list(value)
                break
        except Exception:
            pass
    result = []
    for expr in expressions:
        if not expr:
            continue
        entry = {
            "class": expr.get_class().get_name(),
            "name": expr.get_name(),
        }
        for prop in ("parameter_name", "desc", "const_coordinate", "const_period", "custom_primitive_data_index"):
            try:
                entry[prop] = str(expr.get_editor_property(prop))
            except Exception:
                pass
        result.append(entry)
    return result


def material_instance_parameters(mi):
    if not mi:
        return {}
    out = {}
    for label, prop in (
        ("scalar", "scalar_parameter_values"),
        ("vector", "vector_parameter_values"),
        ("texture", "texture_parameter_values"),
        ("static_switch", "static_parameters"),
    ):
        try:
            raw_values = mi.get_editor_property(prop)
        except Exception:
            continue
        values = []
        if label == "static_switch":
            try:
                raw_values = raw_values.static_switch_parameters
            except Exception:
                raw_values = []
        for item in raw_values:
            try:
                info = item.parameter_info
                name = str(info.name)
            except Exception:
                name = ""
            value = None
            for value_prop in ("parameter_value", "value"):
                try:
                    value = item.get_editor_property(value_prop)
                    break
                except Exception:
                    pass
            values.append({"name": name, "value": object_path(value)})
        out[label] = values
    return out


rows = []
with open(CSV_PATH, newline="", encoding="utf-8-sig") as handle:
    reader = csv.DictReader(handle)
    for row in reader:
        rows.append(row)

asset_tools = unreal.AssetRegistryHelpers.get_asset_registry()
audit = {
    "base_materials": {},
    "mob_vertex_rows": [],
    "material_instance_parents": {},
}

for path in ["/Game/Materials/M_Character_Unlit", "/Game/Materials/Retro/M_Character_Unlit_RetroGeometry"]:
    asset = unreal.EditorAssetLibrary.load_asset(path)
    audit["base_materials"][path] = {
        "exists": bool(asset),
        "class": asset.get_class().get_name() if asset else "",
        "path": object_path(asset),
        "expressions": material_expressions(asset) if asset else [],
    }

seen_mis = set()
for row in rows:
    material_path = asset_path_from_object_path(row.get("Material", ""))
    static_mesh_path = asset_path_from_object_path(row.get("StaticMesh", ""))
    row_entry = {
        "EnemyID": row.get("EnemyID", ""),
        "StaticMesh": row.get("StaticMesh", ""),
        "Material": row.get("Material", ""),
        "PositionTexture": row.get("PositionTexture", ""),
        "NormalTexture": row.get("NormalTexture", ""),
        "NumFrames": row.get("NumFrames", ""),
        "RowsPerFrame": row.get("RowsPerFrame", ""),
    }
    audit["mob_vertex_rows"].append(row_entry)

    if material_path and material_path not in seen_mis:
        seen_mis.add(material_path)
        mi = unreal.EditorAssetLibrary.load_asset(material_path)
        parent = None
        try:
            parent = mi.get_editor_property("parent") if mi else None
        except Exception:
            parent = None
        audit["material_instance_parents"][material_path] = {
            "exists": bool(mi),
            "class": mi.get_class().get_name() if mi else "",
            "parent": object_path(parent),
            "parameters": material_instance_parameters(mi) if mi else {},
        }

os.makedirs(os.path.dirname(OUT_PATH), exist_ok=True)
with open(OUT_PATH, "w", encoding="utf-8") as handle:
    json.dump(audit, handle, indent=2)

unreal.log("B13 VAT material audit wrote {}".format(OUT_PATH))
