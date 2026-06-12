"""Final sweep to ONE gameplay master: reparent remaining toon-master instances onto
M_FriendSlop_FallGuys, then ref-check delete the four unreferenced straggler masters.
Re-census afterwards."""
from __future__ import annotations
import json, os
import unreal

OUT = r"C:/UE/T66/Saved/CombatTest/FallGuys/final_sweep.json"
MASTER = "/Game/Materials/M_FriendSlop_FallGuys"
MEL = unreal.MaterialEditingLibrary
EAL = unreal.EditorAssetLibrary
SWEEP = [
    "/Game/ToonStyle/Materials/M_Toon_Character",
    "/Game/ToonStyle/Materials/M_Toon_Environment",
    "/Game/Materials/M_TorchFlicker_LightFn",
    "/Game/Materials/M_GLB_ViewSpaceLit_Character",
    "/Game/Materials/Generated/M_Unlit_DiffuseColorMap",
]
out = {"reparented": 0, "deleted": [], "blocked": [], "errors": [], "census_after": {}}

try:
    master = EAL.load_asset(MASTER)
    sweep_set = set(SWEEP)
    ar = unreal.AssetRegistryHelpers.get_asset_registry()
    f = unreal.ARFilter(class_paths=[unreal.TopLevelAssetPath("/Script/Engine", "MaterialInstanceConstant")],
                        package_paths=["/Game"], recursive_paths=True)
    for ad in ar.get_assets(f):
        pkg = str(ad.package_name)
        if pkg.startswith(("/Game/UI", "/Game/Stylized_VFX", "/Game/VFX")):
            continue
        mi = ad.get_asset()
        if not mi:
            continue
        par = mi.get_editor_property("parent")
        par_pkg = par.get_path_name().split(".")[0] if par else ""
        if par_pkg in sweep_set:
            try:
                MEL.set_material_instance_parent(mi, master)
                EAL.save_asset(pkg, only_if_is_dirty=False)
                out["reparented"] += 1
            except Exception as e:
                out["errors"].append(f"reparent {pkg}: {e}")

    for path in SWEEP:
        if not EAL.does_asset_exist(path):
            out["deleted"].append(path + " (already gone)")
            continue
        refs = [str(r) for r in EAL.find_package_referencers_for_asset(path, load_assets_to_confirm=False)
                if str(r).startswith("/Game")]
        if refs:
            out["blocked"].append({"asset": path, "referencers": refs[:6]})
        elif EAL.delete_asset(path):
            out["deleted"].append(path)
        else:
            out["blocked"].append({"asset": path, "referencers": ["delete_asset false"]})

    # re-census (in-scope masters = not UI / Stylized_VFX / VFX*)
    f2 = unreal.ARFilter(class_paths=[unreal.TopLevelAssetPath("/Script/Engine", "Material")],
                         package_paths=["/Game"], recursive_paths=True)
    in_scope = [str(ad.package_name) for ad in ar.get_assets(f2)
                if not str(ad.package_name).startswith(("/Game/UI", "/Game/Stylized_VFX", "/Game/VFX", "/Game/VFXLab"))]
    out["census_after"]["in_scope_masters"] = sorted(in_scope)
finally:
    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    with open(OUT, "w", encoding="utf-8") as fjson:
        json.dump(out, fjson, indent=2)
    unreal.log("[FinalSweep] " + json.dumps({k: (len(v) if isinstance(v, list) else v) for k, v in out.items() if k != "census_after"}))
    try: unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")
    except Exception: pass
