"""Set used_with_nanite (+ spline) on M_FriendSlop_FallGuys — kit/baffle meshes are Nanite and
fell back to Default Material gray. Recompile+save, JSON, self-quit."""
from __future__ import annotations
import json, os
import unreal

OUT = r"C:/UE/T66/Saved/CombatTest/FallGuys/set_nanite_flag.json"
MASTER = "/Game/Materials/M_FriendSlop_FallGuys"
out = {}
try:
    m = unreal.EditorAssetLibrary.load_asset(MASTER)
    for prop in ("used_with_nanite", "used_with_spline_meshes"):
        out[prop + "_before"] = m.get_editor_property(prop)
        m.set_editor_property(prop, True)
        out[prop + "_after"] = m.get_editor_property(prop)
    unreal.MaterialEditingLibrary.recompile_material(m)
    out["saved"] = unreal.EditorAssetLibrary.save_asset(MASTER, only_if_is_dirty=False)
    if not out["saved"]:
        pkg = m.get_outer() if hasattr(m, "get_outer") else None
        try:
            ok = unreal.EditorLoadingAndSavingUtils.save_packages([m.get_package()], only_dirty=False)
            out["saved_fallback"] = bool(ok)
        except Exception as e:
            out["saved_fallback_err"] = str(e)
finally:
    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    with open(OUT, "w", encoding="utf-8") as f:
        json.dump(out, f, indent=2)
    unreal.log("[NaniteFlag] " + json.dumps(out))
    try: unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")
    except Exception: pass
