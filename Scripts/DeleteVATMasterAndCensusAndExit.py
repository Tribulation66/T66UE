"""Delete M_EasyMobVAT_Unlit_UV2 (ref-checked) + MATERIAL CENSUS: every UMaterial master in /Game
with its instance count; final state should be ONE gameplay master (UI/Niagara excluded)."""
from __future__ import annotations
import json, os
import unreal

OUT = r"C:/UE/T66/Saved/CombatTest/FallGuys/vat_delete_census.json"
EAL = unreal.EditorAssetLibrary
out = {"deleted": [], "blocked": [], "census": {}, "errors": []}
VAT_MASTER = "/Game/Materials/M_EasyMobVAT_Unlit_UV2"

try:
    # 1) delete old VAT master if unreferenced
    if EAL.does_asset_exist(VAT_MASTER):
        refs = [str(r) for r in EAL.find_package_referencers_for_asset(VAT_MASTER, load_assets_to_confirm=False)
                if str(r).startswith("/Game")]
        if refs:
            out["blocked"].append({"asset": VAT_MASTER, "referencers": refs[:8]})
        elif EAL.delete_asset(VAT_MASTER):
            out["deleted"].append(VAT_MASTER)
        else:
            out["blocked"].append({"asset": VAT_MASTER, "referencers": ["delete_asset false"]})
    else:
        out["deleted"].append(VAT_MASTER + " (already gone)")

    # 2) census: all Material masters under /Game
    ar = unreal.AssetRegistryHelpers.get_asset_registry()
    f = unreal.ARFilter(class_paths=[unreal.TopLevelAssetPath("/Script/Engine", "Material")],
                        package_paths=["/Game"], recursive_paths=True)
    masters = sorted(str(ad.package_name) for ad in ar.get_assets(f))
    f2 = unreal.ARFilter(class_paths=[unreal.TopLevelAssetPath("/Script/Engine", "MaterialInstanceConstant")],
                         package_paths=["/Game"], recursive_paths=True)
    mics = [ad for ad in ar.get_assets(f2)]
    parent_counts = {}
    for ad in mics:
        mi = ad.get_asset()
        if not mi:
            continue
        par = mi.get_editor_property("parent")
        key = par.get_path_name().split(".")[0] if par else "NULL_PARENT"
        parent_counts[key] = parent_counts.get(key, 0) + 1
    out["census"] = {
        "masters": masters,
        "master_count": len(masters),
        "mic_count": len(mics),
        "instances_by_parent": dict(sorted(parent_counts.items(), key=lambda kv: -kv[1])),
    }
finally:
    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    with open(OUT, "w", encoding="utf-8") as fjson:
        json.dump(out, fjson, indent=2)
    unreal.log("[VATCensus] masters=%d" % len(out.get("census", {}).get("masters", [])))
    try: unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")
    except Exception: pass
