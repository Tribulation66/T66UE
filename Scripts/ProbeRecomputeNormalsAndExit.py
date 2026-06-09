"""Determine whether SK recompute-normals build setting is reachable + applies via Python (5.7).
Enumerates API, attempts set+rebuild, saves if it works. Writes JSON, self-quits."""
from __future__ import annotations
import json, os
import unreal

OUT = r"C:/UE/T66/Saved/CombatTest/probe_recompute_normals.json"
SK = "/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/PhysicsFirst/SK_Hero_1_Chad_PhysicsFirst"

def main():
    out = {"applied": False, "method": None, "errors": [], "api": {}}
    sk = unreal.EditorAssetLibrary.load_asset(SK)
    if not sk:
        out["errors"].append("SK not loadable"); _write(out); return
    out["api"]["sk_lod_build_members"] = [m for m in dir(sk) if ("lod" in m.lower() or "build" in m.lower())]
    out["api"]["skeletalmesh_static"] = [m for m in dir(unreal.SkeletalMesh) if ("lod" in m.lower() or "build" in m.lower())]

    # Approach 1: get_lod_info(index) method
    try:
        li = sk.get_lod_info(0)
        out["api"]["get_lod_info_ok"] = True
        bs = li.get_editor_property("build_settings")
        out["api"]["build_settings_type"] = type(bs).__name__
        out["api"]["recompute_normals_before"] = bs.get_editor_property("recompute_normals")
        bs.set_editor_property("recompute_normals", True)
        li.set_editor_property("build_settings", bs)
        sk.set_lod_info(0, li)
        out["method"] = "get_lod_info/set_lod_info"
        out["api"]["recompute_normals_after"] = bs.get_editor_property("recompute_normals")
        out["applied_struct"] = True
    except Exception as e:
        out["errors"].append("approach1 get_lod_info: " + str(e))

    # Approach 2: SkeletalMeshEditorSubsystem
    try:
        sub = unreal.get_editor_subsystem(unreal.SkeletalMeshEditorSubsystem)
        out["api"]["skel_subsystem_members"] = [m for m in dir(sub) if ("lod" in m.lower() or "build" in m.lower() or "normal" in m.lower())]
    except Exception as e:
        out["errors"].append("approach2 subsystem: " + str(e))

    # Rebuild + save if we managed to set the struct
    if out.get("applied_struct"):
        rebuilt = False
        for fn in ("build", "rebuild"):
            try:
                getattr(sk, fn)(); rebuilt = True; out["rebuild_method"] = fn; break
            except Exception as e:
                out["errors"].append(f"rebuild {fn}: {e}")
        try:
            unreal.EditorAssetLibrary.save_asset(SK, only_if_is_dirty=False)
            out["saved"] = True
        except Exception as e:
            out["errors"].append("save: " + str(e))
        out["applied"] = bool(out.get("saved"))
        out["rebuilt"] = rebuilt

    _write(out)

def _write(out):
    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    with open(OUT, "w", encoding="utf-8") as f: json.dump(out, f, indent=2)
    unreal.log("[ProbeRecomputeNormals] RESULT " + json.dumps(out))

try:
    main()
finally:
    try: unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")
    except Exception: pass
