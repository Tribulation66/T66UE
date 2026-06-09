"""Set bUsedWithSkeletalMesh on M_FriendSlop_FallGuys, recompile, save. Minimal + reversible. Self-quits."""
from __future__ import annotations
import json, os
import unreal

OUT = r"C:/UE/T66/Saved/CombatTest/FallGuys/set_skeletal_usage.json"
MASTER = "/Game/Materials/M_FriendSlop_FallGuys"

def main():
    out = {}
    m = unreal.EditorAssetLibrary.load_asset(MASTER)
    if not m:
        out["error"] = "master not loadable"; _w(out); return
    out["before"] = m.get_editor_property("used_with_skeletal_mesh")
    m.set_editor_property("used_with_skeletal_mesh", True)
    out["after"] = m.get_editor_property("used_with_skeletal_mesh")
    try:
        unreal.MaterialEditingLibrary.recompile_material(m)
        out["recompiled"] = True
    except Exception as e:
        out["recompile_err"] = str(e)
    out["saved"] = unreal.EditorAssetLibrary.save_asset(MASTER, only_if_is_dirty=False)
    _w(out)

def _w(out):
    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    with open(OUT, "w", encoding="utf-8") as f:
        json.dump(out, f, indent=2)
    unreal.log("[SetSkeletalUsage] RESULT " + json.dumps(out))

try:
    main()
finally:
    try: unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")
    except Exception: pass
