"""Fresh-session ref-check + delete of the two remaining toon masters."""
from __future__ import annotations
import json, os
import unreal

OUT = r"C:/UE/T66/Saved/CombatTest/FallGuys/delete_toon_masters.json"
EAL = unreal.EditorAssetLibrary
out = {"deleted": [], "blocked": []}
for path in ("/Game/ToonStyle/Materials/M_Toon_Character", "/Game/ToonStyle/Materials/M_Toon_Environment"):
    if not EAL.does_asset_exist(path):
        out["deleted"].append(path + " (already gone)"); continue
    refs = [str(r) for r in EAL.find_package_referencers_for_asset(path, load_assets_to_confirm=True)
            if str(r).startswith("/Game")]
    if refs:
        out["blocked"].append({"asset": path, "referencers": refs[:8]})
    elif EAL.delete_asset(path):
        out["deleted"].append(path)
    else:
        out["blocked"].append({"asset": path, "referencers": ["delete_asset false"]})
os.makedirs(os.path.dirname(OUT), exist_ok=True)
with open(OUT, "w", encoding="utf-8") as f:
    json.dump(out, f, indent=2)
unreal.log("[DeleteToon] " + json.dumps(out))
try: unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")
except Exception: pass
