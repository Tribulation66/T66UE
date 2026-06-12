"""PHASE D pass 2: clear the blockers, heal dangling parents, retry master deletion."""
from __future__ import annotations
import json, os
import unreal

OUT = r"C:/UE/T66/Saved/CombatTest/FallGuys/phase_d_pass2.json"
MASTER = "/Game/Materials/M_FriendSlop_FallGuys"
EAL = unreal.EditorAssetLibrary
MEL = unreal.MaterialEditingLibrary
out = {"deleted": [], "reparented": [], "healed_null_parent": [], "blocked": [], "errors": []}

RETRY_DELETES = [
    "/Game/Materials/MI_FriendSlop_Rubber_Unlit_Hero1_Chad",  # after lookdev folder removal
    "/Game/Materials/M_GLB_Unlit",
    "/Game/Materials/M_Character_Unlit",
    "/Game/Materials/M_FBX_Unlit",
    "/Game/Materials/M_Environment_Lit",
    "/Game/Materials/M_Environment_Unlit",
    "/Game/ToonStyle/Materials/M_Toon_Character_Outline",
    "/Game/World/Lighting/M_TorchFlicker_LightFn",
]
REPARENT_TO_MASTER = [
    "/Game/Materials/MI_GLB_Unlit_Character_Shared",
    "/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/Skeletal/Materials/MI_SK_Hero_1_Chad_Male_FriendSlop",
]

def main():
    master = EAL.load_asset(MASTER)
    if not master:
        out["errors"].append("master not loadable"); return

    # 1) Rubber-era lookdev folder: superseded consumer of the rubber MIs.
    try:
        if EAL.does_directory_exist("/Game/LookDev/FriendSlopRubber"):
            for p in EAL.list_assets("/Game/LookDev/FriendSlopRubber", recursive=True, include_folder=False):
                q = p.split(".")[0]
                if EAL.delete_asset(q):
                    out["deleted"].append(q)
                else:
                    out["blocked"].append(q)
    except Exception as e:
        out["errors"].append("lookdev: " + str(e))

    # 2) Reparent kept-but-referenced stragglers onto the master.
    for path in REPARENT_TO_MASTER:
        mi = EAL.load_asset(path)
        if isinstance(mi, unreal.MaterialInstanceConstant):
            try:
                MEL.set_material_instance_parent(mi, master)
                EAL.save_asset(path, only_if_is_dirty=False)
                out["reparented"].append(path)
            except Exception as e:
                out["errors"].append(f"reparent {path}: {e}")

    # 3) Heal ANY material instance in /Game left with a null parent (parent was deleted in pass 1).
    ar = unreal.AssetRegistryHelpers.get_asset_registry()
    f = unreal.ARFilter(class_paths=[unreal.TopLevelAssetPath("/Script/Engine", "MaterialInstanceConstant")],
                        package_paths=["/Game"], recursive_paths=True)
    for ad in ar.get_assets(f):
        pkg = str(ad.package_name)
        if pkg.startswith(("/Game/UI", "/Game/Stylized_VFX", "/Game/Characters/MobsVAT", "/Game/Audio")):
            continue
        mi = ad.get_asset()
        if not mi:
            continue
        try:
            if mi.get_editor_property("parent") is None:
                MEL.set_material_instance_parent(mi, master)
                EAL.save_asset(pkg, only_if_is_dirty=False)
                out["healed_null_parent"].append(pkg)
        except Exception as e:
            out["errors"].append(f"heal {pkg}: {e}")

    # 4) Retry the master deletions (children all reparented now).
    for path in RETRY_DELETES:
        if not EAL.does_asset_exist(path):
            continue
        try:
            refs = [str(r) for r in EAL.find_package_referencers_for_asset(path, load_assets_to_confirm=False)
                    if str(r).startswith("/Game")]
            if refs:
                out["blocked"].append({"asset": path, "referencers": refs[:6]})
                continue
            if EAL.delete_asset(path):
                out["deleted"].append(path)
            else:
                out["blocked"].append({"asset": path, "referencers": ["delete_asset returned false"]})
        except Exception as e:
            out["errors"].append(f"delete {path}: {e}")

    out["counts"] = {k: len(v) for k, v in out.items() if isinstance(v, list)}

try:
    main()
finally:
    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    with open(OUT, "w", encoding="utf-8") as fjson:
        json.dump(out, fjson, indent=2)
    unreal.log("[PhaseD2] " + json.dumps(out.get("counts", {})))
    try: unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")
    except Exception: pass
