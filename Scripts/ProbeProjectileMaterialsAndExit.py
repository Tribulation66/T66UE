"""READ-ONLY: dump material slots of the two projectile meshes + MI parents/params."""
from __future__ import annotations
import json, os
import unreal

OUT = r"C:/UE/T66/Saved/CombatTest/FallGuys/projectile_materials_probe.json"
out = {"meshes": {}, "mis": {}}
try:
    for name in ("SM_WeaponProjectile_Black", "SM_IdolProjectile_FireBlack"):
        sm = unreal.EditorAssetLibrary.load_asset(f"/Game/Weapons/Projectiles/FriendSlop/{name}")
        slots = []
        if sm:
            for m in sm.get_editor_property("static_materials"):
                iface = m.get_editor_property("material_interface")
                slots.append({
                    "slot_name": str(m.get_editor_property("material_slot_name")),
                    "material": iface.get_path_name() if iface else "None",
                })
        out["meshes"][name] = slots
    for name in ("MI_WeaponProjectile_Black", "MI_IdolProjectile_FireBlack"):
        mi = unreal.EditorAssetLibrary.load_asset(f"/Game/Weapons/Projectiles/FriendSlop/{name}")
        if mi:
            parent = mi.get_editor_property("parent")
            tex = unreal.MaterialEditingLibrary.get_material_instance_texture_parameter_value(mi, "BaseColorTexture")
            out["mis"][name] = {
                "parent": parent.get_path_name() if parent else "None",
                "BaseColorTexture": tex.get_path_name() if tex else "None",
            }
finally:
    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    with open(OUT, "w", encoding="utf-8") as f:
        json.dump(out, f, indent=2)
    unreal.log("[MatProbe] " + json.dumps(out))
    try: unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")
    except Exception: pass
