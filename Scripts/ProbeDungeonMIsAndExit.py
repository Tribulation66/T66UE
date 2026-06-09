"""Probe reparented MI_Dungeon_* instances: parent + texture/scalar/vector parameter values."""
from __future__ import annotations
import json, os
import unreal

OUT = r"C:/UE/T66/Saved/CombatTest/FallGuys/probe_dungeon_mis.json"
MEL = unreal.MaterialEditingLibrary
EAL = unreal.EditorAssetLibrary
MIS = [
    "/Game/ToonStyle/Environment/Dungeon/Materials/MI_Dungeon_Floor",
    "/Game/ToonStyle/Environment/Dungeon/Materials/MI_Dungeon_Wall_XZ",
    "/Game/ToonStyle/Environment/Dungeon/Materials/MI_Dungeon_Ceiling",
    "/Game/World/Terrain/TowerForest/MI_TowerForestGround",
]
PARAM_NAMES = ["BaseColorTexture", "DiffuseColorMap", "Texture", "Albedo", "BaseColorMap"]
out = {}

def main():
    for path in MIS:
        info = {}
        mi = EAL.load_asset(path)
        if not mi:
            out[path] = "NOT LOADABLE"; continue
        try:
            info["parent"] = str(mi.get_editor_property("parent").get_path_name()) if mi.get_editor_property("parent") else "None"
        except Exception as e:
            info["parent_err"] = str(e)
        texvals = {}
        for p in PARAM_NAMES:
            try:
                t = MEL.get_material_instance_texture_parameter_value(mi, p)
                texvals[p] = t.get_path_name() if t else None
            except Exception as e:
                texvals[p] = "ERR " + str(e)
        info["tex_params"] = texvals
        try:
            # overridden parameters actually stored on the instance
            overrides = []
            for tp in mi.get_editor_property("texture_parameter_values"):
                overrides.append({
                    "name": str(tp.get_editor_property("parameter_info").get_editor_property("name")),
                    "value": tp.get_editor_property("parameter_value").get_path_name() if tp.get_editor_property("parameter_value") else "None",
                })
            info["stored_texture_overrides"] = overrides
        except Exception as e:
            info["stored_err"] = str(e)
        out[path] = info
    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    with open(OUT, "w", encoding="utf-8") as f:
        json.dump(out, f, indent=2)
    unreal.log("[ProbeDungeonMIs] " + json.dumps(out)[:800])

try:
    main()
finally:
    try: unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")
    except Exception: pass
