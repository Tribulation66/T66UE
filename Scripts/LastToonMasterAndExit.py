"""Reparent MI_WallTorch_Easy_Pixal3D to the one master, then delete M_Toon_Character."""
from __future__ import annotations
import json, os
import unreal

OUT = r"C:/UE/T66/Saved/CombatTest/FallGuys/last_toon_master.json"
MEL = unreal.MaterialEditingLibrary
EAL = unreal.EditorAssetLibrary
out = {}
master = EAL.load_asset("/Game/Materials/M_FriendSlop_FallGuys")
mi = EAL.load_asset("/Game/World/VisualProps/Easy/Materials/MI_WallTorch_Easy_Pixal3D")
if isinstance(mi, unreal.MaterialInstanceConstant) and master:
    MEL.set_material_instance_parent(mi, master)
    out["reparented"] = EAL.save_asset("/Game/World/VisualProps/Easy/Materials/MI_WallTorch_Easy_Pixal3D", only_if_is_dirty=False)
path = "/Game/ToonStyle/Materials/M_Toon_Character"
refs = [str(r) for r in EAL.find_package_referencers_for_asset(path, load_assets_to_confirm=True) if str(r).startswith("/Game")]
out["remaining_refs"] = refs[:6]
out["deleted"] = (not refs) and EAL.delete_asset(path)
os.makedirs(os.path.dirname(OUT), exist_ok=True)
with open(OUT, "w", encoding="utf-8") as f:
    json.dump(out, f, indent=2)
unreal.log("[LastToon] " + json.dumps(out))
try: unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")
except Exception: pass
