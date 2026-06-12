"""PHASE D (assets): migrate remaining references to the ONE master, then ref-checked deletion.
1. Probe VAT mob MI parents (must NOT be reparented/broken).
2. Reparent every MaterialInstanceConstant in /Game whose parent is a superseded master
   (M_GLB_Unlit, M_Character_Unlit, M_FBX_Unlit, M_Environment_Lit, M_Environment_Unlit,
   M_Toon_Character_Outline) onto M_FriendSlop_FallGuys — texture overrides carry by name.
   Exclusions: /Game/UI, /Game/Stylized_VFX, /Game/Characters/MobsVAT, /Game/UE5RFX,
   /Game/Materials/Retro, /Game/Archive (deleted below anyway).
3. Delete (each ref-checked; blocked => skip + record):
   explicit superseded assets, /Game/Materials/Retro/**, /Game/UE5RFX/**,
   /Game/Archive/FriendSlopEasyPixal3D_20260604 material assets, ToonStyle TestAssets
   material packs (Lineup/LuBu_Matrix/Validation), UI CRT pair, then the old masters.
Writes JSON report, self-quits."""
from __future__ import annotations
import json, os
import unreal

OUT = r"C:/UE/T66/Saved/CombatTest/FallGuys/phase_d_assets.json"
MASTER = "/Game/Materials/M_FriendSlop_FallGuys"
EAL = unreal.EditorAssetLibrary
MEL = unreal.MaterialEditingLibrary

OLD_MASTERS = [
    "/Game/Materials/M_GLB_Unlit",
    "/Game/Materials/M_Character_Unlit",
    "/Game/Materials/M_FBX_Unlit",
    "/Game/Materials/M_Environment_Lit",
    "/Game/Materials/M_Environment_Unlit",
    "/Game/ToonStyle/Materials/M_Toon_Character_Outline",
]
EXPLICIT_DELETES = [
    "/Game/Materials/M_FriendSlop_Rubber",
    "/Game/Materials/M_FriendSlop_Rubber_Unlit",
    "/Game/Materials/M_FriendSlop_Rubber_ClearCoat",
    "/Game/Materials/MI_FriendSlop_Rubber_Hero1_Chad",
    "/Game/Materials/MI_FriendSlop_Rubber_Unlit_Hero1_Chad",
    "/Game/Materials/MI_FriendSlop_Rubber_ClearCoat_Hero1_Chad",
    "/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/Skeletal/Materials/MI_SK_Hero_1_Chad_Male_FriendSlop",
    "/Game/Materials/MI_Hero_1_Chad_FallGuys",
    "/Game/Materials/MI_GLB_Unlit_Character_Shared",
    "/Game/UI/Materials/M_T66_UI_CRTPostProcess",
    "/Game/UI/Materials/M_UI_RetroRetainer",
    "/Game/World/Lighting/M_TorchFlicker_LightFn",
]
DELETE_FOLDERS = [
    "/Game/Materials/Retro",
    "/Game/UE5RFX",
    "/Game/ToonStyle/TestAssets/Lineup/Materials",
    "/Game/ToonStyle/TestAssets/LuBu_Matrix/Materials",
    "/Game/ToonStyle/TestAssets/Validation/Materials",
]
ARCHIVE_ROOT = "/Game/Archive/FriendSlopEasyPixal3D_20260604"
REPARENT_EXCLUDE_PREFIXES = (
    "/Game/UI", "/Game/Stylized_VFX", "/Game/Characters/MobsVAT",
    "/Game/UE5RFX", "/Game/Materials/Retro", "/Game/Archive",
)

out = {"vat_parents": {}, "reparented": [], "deleted": [], "blocked": [], "errors": []}

def norm(p):
    return p.split(".")[0]

def main():
    master = EAL.load_asset(MASTER)
    if not master:
        out["errors"].append("master not loadable"); return

    ar = unreal.AssetRegistryHelpers.get_asset_registry()

    # 1) VAT parents (informational + safety)
    f = unreal.ARFilter(class_paths=[unreal.TopLevelAssetPath("/Script/Engine", "MaterialInstanceConstant")],
                        package_paths=["/Game/Characters/MobsVAT"], recursive_paths=True)
    for ad in ar.get_assets(f):
        mi = ad.get_asset()
        if mi:
            par = mi.get_editor_property("parent")
            out["vat_parents"][str(ad.package_name)] = par.get_path_name() if par else "None"
    vat_parent_pkgs = {norm(v) for v in out["vat_parents"].values() if v != "None"}

    # 2) Reparent all MICs parented to old masters
    old_master_objs = {}
    for m in OLD_MASTERS:
        if m in vat_parent_pkgs:
            out["blocked"].append({"asset": m, "reason": "VAT mob MIs are parented to it (vertex-anim shader)"})
            continue
        old_master_objs[m] = EAL.load_asset(m)
    f2 = unreal.ARFilter(class_paths=[unreal.TopLevelAssetPath("/Script/Engine", "MaterialInstanceConstant")],
                         package_paths=["/Game"], recursive_paths=True)
    for ad in ar.get_assets(f2):
        pkg = str(ad.package_name)
        if pkg.startswith(REPARENT_EXCLUDE_PREFIXES):
            continue
        mi = ad.get_asset()
        if not mi:
            continue
        par = mi.get_editor_property("parent")
        par_pkg = norm(par.get_path_name()) if par else ""
        if par_pkg in old_master_objs:
            try:
                MEL.set_material_instance_parent(mi, master)
                if EAL.save_asset(pkg, only_if_is_dirty=False):
                    out["reparented"].append(pkg)
                else:
                    out["errors"].append("save fail (reparent): " + pkg)
            except Exception as e:
                out["errors"].append(f"reparent {pkg}: {e}")

    # 3) Build the delete list
    delete_list = list(EXPLICIT_DELETES)
    for folder in DELETE_FOLDERS:
        try:
            delete_list += [norm(p) for p in EAL.list_assets(folder, recursive=True, include_folder=False)]
        except Exception as e:
            out["errors"].append(f"list {folder}: {e}")
    # archive: material assets only
    try:
        for p in EAL.list_assets(ARCHIVE_ROOT, recursive=True, include_folder=False):
            short = p.split("/")[-1].split(".")[0]
            if short.startswith(("M_", "MI_")):
                delete_list.append(norm(p))
    except Exception as e:
        out["errors"].append(f"list archive: {e}")
    # old masters last (children reparented first)
    delete_list += [m for m in OLD_MASTERS if m in old_master_objs]

    delete_set = {norm(d) for d in delete_list}

    # 4) Ref-checked deletion (refs inside the delete set are fine)
    for path in delete_list:
        path = norm(path)
        if not EAL.does_asset_exist(path):
            continue
        try:
            refs = EAL.find_package_referencers_for_asset(path, load_assets_to_confirm=False)
            outside = [str(r) for r in refs
                       if str(r).startswith("/Game") and norm(str(r)) not in delete_set and norm(str(r)) != path]
            if outside:
                out["blocked"].append({"asset": path, "referencers": outside[:8]})
                continue
            if EAL.delete_asset(path):
                out["deleted"].append(path)
            else:
                out["blocked"].append({"asset": path, "referencers": ["delete_asset returned false"]})
        except Exception as e:
            out["errors"].append(f"delete {path}: {e}")

    out["counts"] = {"reparented": len(out["reparented"]), "deleted": len(out["deleted"]),
                     "blocked": len(out["blocked"]), "errors": len(out["errors"])}

try:
    main()
finally:
    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    with open(OUT, "w", encoding="utf-8") as fjson:
        json.dump(out, fjson, indent=2)
    unreal.log("[PhaseD] " + json.dumps(out.get("counts", {})))
    try: unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")
    except Exception: pass
