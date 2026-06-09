"""Switch M_FriendSlop_FallGuys from MSM_SUBSURFACE to DEFAULT LIT and disconnect the
subsurface/opacity/emissive inputs (no emissive, no subsurface). Keeps BaseColor/Specular/Roughness.
Reversible (restore MSM_SUBSURFACE + reconnect). Writes JSON, self-quits."""
from __future__ import annotations
import json, os
import unreal

OUT = r"C:/UE/T66/Saved/CombatTest/FallGuys/set_default_lit.json"
MASTER = "/Game/Materials/M_FriendSlop_FallGuys"
MEL = unreal.MaterialEditingLibrary
MP = unreal.MaterialProperty

def main():
    out = {"disconnects": {}}
    m = unreal.EditorAssetLibrary.load_asset(MASTER)
    if not m:
        out["error"] = "master not loadable"; _w(out); return
    out["shading_model_before"] = str(m.get_editor_property("shading_model"))
    m.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_DEFAULT_LIT)
    out["shading_model_after"] = str(m.get_editor_property("shading_model"))
    out["used_with_skeletal_mesh"] = m.get_editor_property("used_with_skeletal_mesh")
    # Kill the subsurface + emissive wiring so nothing scatters or self-glows under Default Lit.
    for name, prop in (("Opacity", MP.MP_OPACITY),
                       ("SubsurfaceColor", MP.MP_SUBSURFACE_COLOR),
                       ("EmissiveColor", MP.MP_EMISSIVE_COLOR)):
        try:
            MEL.disconnect_material_property(m, prop)
            out["disconnects"][name] = True
        except Exception as e:
            out["disconnects"][name] = "ERR: " + str(e)
    try:
        MEL.recompile_material(m)
        out["recompiled"] = True
    except Exception as e:
        out["recompile_err"] = str(e)
    out["saved"] = unreal.EditorAssetLibrary.save_asset(MASTER, only_if_is_dirty=False)
    _w(out)

def _w(out):
    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    with open(OUT, "w", encoding="utf-8") as f:
        json.dump(out, f, indent=2)
    unreal.log("[SetDefaultLit] RESULT " + json.dumps(out))

try:
    main()
finally:
    try: unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")
    except Exception: pass
