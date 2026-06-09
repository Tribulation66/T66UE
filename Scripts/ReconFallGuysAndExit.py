"""Read-only recon: inspect M_FriendSlop_Rubber family + SK_Hero_1_Chad_PhysicsFirst build settings
(recompute-normals feasibility). Writes JSON, self-quits. No mutations."""
from __future__ import annotations
import json, os
import unreal

OUT = r"C:/UE/T66/Saved/CombatTest/recon_fallguys.json"
MATS = [
    "/Game/Materials/M_FriendSlop_Rubber",
    "/Game/Materials/M_FriendSlop_Rubber_Unlit",
    "/Game/Materials/M_FriendSlop_Rubber_ClearCoat",
    "/Game/Materials/M_GLB_Unlit",
]
SK = "/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/PhysicsFirst/SK_Hero_1_Chad_PhysicsFirst"

def mat_info(path):
    m = unreal.EditorAssetLibrary.load_asset(path)
    if not m:
        return {"path": path, "exists": False}
    d = {"path": path, "exists": True, "class": m.get_class().get_name()}
    try: d["shading_model"] = str(m.get_editor_property("shading_model"))
    except Exception as e: d["shading_model_err"] = str(e)
    try: d["scalar_params"] = [str(n) for n in unreal.MaterialEditingLibrary.get_scalar_parameter_names(m)]
    except Exception as e: d["scalar_err"] = str(e)
    try: d["texture_params"] = [str(n) for n in unreal.MaterialEditingLibrary.get_texture_parameter_names(m)]
    except Exception as e: d["texture_err"] = str(e)
    try: d["vector_params"] = [str(n) for n in unreal.MaterialEditingLibrary.get_vector_parameter_names(m)]
    except Exception as e: d["vector_err"] = str(e)
    return d

def sk_info(path):
    sk = unreal.EditorAssetLibrary.load_asset(path)
    if not sk:
        return {"path": path, "exists": False}
    d = {"path": path, "exists": True, "class": sk.get_class().get_name()}
    # material slots
    try:
        mats = sk.get_editor_property("materials")
        d["material_slots"] = [(str(ms.get_editor_property("material_slot_name")),
                                ms.get_editor_property("material_interface").get_path_name()
                                if ms.get_editor_property("material_interface") else None) for ms in mats]
    except Exception as e:
        d["materials_err"] = str(e)
    # LOD build settings -> recompute_normals feasibility (READ ONLY)
    try:
        lod_info = sk.get_editor_property("lod_info")
        d["lod_count"] = len(lod_info)
        rows = []
        for i, li in enumerate(lod_info):
            row = {"lod": i}
            try:
                bs = li.get_editor_property("build_settings")
                row["build_settings_type"] = type(bs).__name__
                row["recompute_normals"] = bs.get_editor_property("recompute_normals")
                row["recompute_tangents"] = bs.get_editor_property("recompute_tangents")
                row["settable"] = True
            except Exception as e:
                row["build_settings_err"] = str(e); row["settable"] = False
            rows.append(row)
        d["lods"] = rows
    except Exception as e:
        d["lod_info_err"] = str(e)
    return d

def main():
    out = {"materials": [mat_info(p) for p in MATS], "skeletal_mesh": sk_info(SK)}
    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    with open(OUT, "w", encoding="utf-8") as f:
        json.dump(out, f, indent=2)
    unreal.log("[ReconFallGuys] RESULT " + json.dumps(out))

try:
    main()
finally:
    try: unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")
    except Exception: pass
