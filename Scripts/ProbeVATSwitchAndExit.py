"""Read back bUseVAT static switch on the VAT MIs; if unset, set+verify with explicit re-check."""
from __future__ import annotations
import json, os
import unreal

OUT = r"C:/UE/T66/Saved/CombatTest/FallGuys/probe_vat_switch.json"
MEL = unreal.MaterialEditingLibrary
EAL = unreal.EditorAssetLibrary
out = {"mis": {}, "errors": []}
try:
    ar = unreal.AssetRegistryHelpers.get_asset_registry()
    f = unreal.ARFilter(class_paths=[unreal.TopLevelAssetPath("/Script/Engine", "MaterialInstanceConstant")],
                        package_paths=["/Game/Characters/MobsVAT"], recursive_paths=True)
    has_get = hasattr(MEL, "get_material_instance_static_switch_parameter_value")
    out["has_get"] = has_get
    for ad in ar.get_assets(f):
        pkg = str(ad.package_name)
        mi = ad.get_asset()
        if not mi:
            continue
        entry = {}
        try:
            if has_get:
                entry["before"] = bool(MEL.get_material_instance_static_switch_parameter_value(mi, "bUseVAT"))
            if not entry.get("before", False):
                r = MEL.set_material_instance_static_switch_parameter_value(mi, "bUseVAT", True)
                entry["set_ret"] = str(r)
                MEL.update_material_instance(mi)
                if has_get:
                    entry["after"] = bool(MEL.get_material_instance_static_switch_parameter_value(mi, "bUseVAT"))
                EAL.save_asset(pkg, only_if_is_dirty=False)
            out["mis"][pkg.split("/")[-1]] = entry
        except Exception as e:
            out["errors"].append(f"{pkg}: {e}")
finally:
    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    with open(OUT, "w", encoding="utf-8") as fjson:
        json.dump(out, fjson, indent=2)
    unreal.log("[ProbeVATSwitch] " + json.dumps(out)[:500])
    try: unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")
    except Exception: pass
