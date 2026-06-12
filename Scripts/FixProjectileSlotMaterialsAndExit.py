"""Persist slot-0 defaults on the two projectile meshes via StaticMesh.set_material
(the static_materials array write from the import pass never persisted). Self-quits."""
from __future__ import annotations
import json, os
import unreal

OUT = r"C:/UE/T66/Saved/CombatTest/FallGuys/fix_slot_materials.json"
PAIRS = [
    ("/Game/Weapons/Projectiles/FriendSlop/SM_WeaponProjectile_Black",
     "/Game/Weapons/Projectiles/FriendSlop/MI_WeaponProjectile_Black"),
    ("/Game/Weapons/Projectiles/FriendSlop/SM_IdolProjectile_FireBlack",
     "/Game/Weapons/Projectiles/FriendSlop/MI_IdolProjectile_FireBlack"),
]
out = {"done": [], "errors": []}
try:
    for mesh_path, mi_path in PAIRS:
        mesh = unreal.EditorAssetLibrary.load_asset(mesh_path)
        mi = unreal.EditorAssetLibrary.load_asset(mi_path)
        if not mesh or not mi:
            out["errors"].append(f"load failed mesh={bool(mesh)} mi={bool(mi)} for {mesh_path}")
            continue
        mesh.set_material(0, mi)
        saved = unreal.EditorAssetLibrary.save_asset(mesh_path, only_if_is_dirty=False)
        # readback in the SAME session to confirm the write took
        mats = mesh.get_editor_property("static_materials")
        slot0 = mats[0].get_editor_property("material_interface") if mats else None
        out["done"].append({
            "mesh": mesh_path,
            "saved": bool(saved),
            "slot0_now": slot0.get_path_name() if slot0 else "None",
        })
    # junk material now unreferenced? delete if so
    junk = "/Game/Weapons/Projectiles/FriendSlop/Material_0"
    if unreal.EditorAssetLibrary.does_asset_exist(junk):
        refs = [str(r) for r in unreal.EditorAssetLibrary.find_package_referencers_for_asset(junk, load_assets_to_confirm=True) if str(r).startswith("/Game")]
        if not refs:
            unreal.EditorAssetLibrary.delete_asset(junk)
            out["junk_deleted"] = True
        else:
            out["junk_refs"] = refs[:3]
finally:
    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    with open(OUT, "w", encoding="utf-8") as f:
        json.dump(out, f, indent=2)
    unreal.log("[FixSlots] " + json.dumps(out))
    try: unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")
    except Exception: pass
