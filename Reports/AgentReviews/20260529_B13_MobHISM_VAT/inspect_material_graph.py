import json
import os

import unreal


PROJECT_ROOT = r"C:\UE\T66"
OUT_PATH = os.path.join(PROJECT_ROOT, "Saved", "Codex", "Performance", "B13_MobHISM", "material_graph_probe.json")


def safe_str(value):
    try:
        return str(value)
    except Exception:
        return "<unstringable>"


def expr_summary(expr):
    if not expr:
        return None
    out = {
        "name": expr.get_name(),
        "class": expr.get_class().get_name(),
    }
    for prop in (
        "parameter_name",
        "desc",
        "const_coordinate",
        "r",
        "g",
        "b",
        "a",
        "texture",
        "sampler_type",
        "custom_primitive_data_index",
        "data_index",
        "code",
        "output_type",
    ):
        try:
            out[prop] = safe_str(expr.get_editor_property(prop))
        except Exception:
            pass
    try:
        names = unreal.MaterialEditingLibrary.get_material_expression_input_names(expr)
        out["input_names"] = [safe_str(n) for n in names]
    except Exception as exc:
        out["input_names_error"] = safe_str(exc)
    try:
        types = unreal.MaterialEditingLibrary.get_material_expression_input_types(expr)
        out["input_types"] = [safe_str(t) for t in types]
    except Exception as exc:
        out["input_types_error"] = safe_str(exc)
    return out


def trace_expression(material, expr, depth=0, visited=None):
    if visited is None:
        visited = set()
    if not expr or depth > 8:
        return None
    key = expr.get_path_name()
    if key in visited:
        return {"cycle": expr_summary(expr)}
    visited.add(key)
    node = expr_summary(expr)
    try:
        inputs = unreal.MaterialEditingLibrary.get_inputs_for_material_expression(material, expr)
    except Exception as exc:
        node["inputs_error"] = safe_str(exc)
        return node
    child_nodes = []
    for item in inputs:
        child = {"raw": safe_str(item)}
        for prop in ("input_name", "expression", "output_index", "output_name"):
            try:
                child[prop] = safe_str(item.get_editor_property(prop))
            except Exception:
                pass
        next_expr = item if isinstance(item, unreal.MaterialExpression) else None
        if not next_expr:
            try:
                next_expr = item.get_editor_property("expression")
            except Exception:
                next_expr = None
        if next_expr:
            child["node"] = trace_expression(material, next_expr, depth + 1, visited)
        child_nodes.append(child)
    node["inputs"] = child_nodes
    return node


material = unreal.EditorAssetLibrary.load_asset("/Game/Materials/M_EasyMobVAT_Unlit_UV2")
properties = [
    "MP_BASE_COLOR",
    "MP_EMISSIVE_COLOR",
    "MP_WORLD_POSITION_OFFSET",
    "MP_OPACITY_MASK",
    "MP_NORMAL",
]

out = {"material": "/Game/Materials/M_EasyMobVAT_Unlit_UV2", "properties": {}}
for prop_name in properties:
    prop = getattr(unreal.MaterialProperty, prop_name, None)
    if prop is None:
        out["properties"][prop_name] = {"error": "missing enum"}
        continue
    try:
        node = unreal.MaterialEditingLibrary.get_material_property_input_node(material, prop)
        output_name = unreal.MaterialEditingLibrary.get_material_property_input_node_output_name(material, prop)
        out["properties"][prop_name] = {
            "output_name": safe_str(output_name),
            "node": trace_expression(material, node),
        }
    except Exception as exc:
        out["properties"][prop_name] = {"error": safe_str(exc)}

os.makedirs(os.path.dirname(OUT_PATH), exist_ok=True)
with open(OUT_PATH, "w", encoding="utf-8") as handle:
    json.dump(out, handle, indent=2)

unreal.log("B13 material graph probe wrote {}".format(OUT_PATH))
