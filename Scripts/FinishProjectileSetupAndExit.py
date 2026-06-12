"""Finish projectile setup: import albedo PNGs, create master MIs, assign mesh slots, drop junk."""
from __future__ import annotations
import json, os
import unreal

OUT = r"C:/UE/T66/Saved/CombatTest/FallGuys/finish_projectiles.json"
MASTER = "/Game/Materials/M_FriendSlop_FallGuys"
DEST = "/Game/Weapons/Projectiles/FriendSlop"
ITEMS = [
    {"mesh": f"{DEST}/SM_WeaponProjectile_Black",
     "tex_file": r"C:/UE/T66/SourceAssets/Import/Weapons/Projectiles/FriendSlop/Textures/SM_WeaponProjectile_Black_BaseColor_00.png",
     "tex_name": "T_WeaponProjectile_Black_BaseColor", "mi": "MI_WeaponProjectile_Black"},
    {"mesh": f"{DEST}/SM_IdolProjectile_FireBlack",
     "tex_file": r"C:/UE/T66/SourceAssets/Import/Weapons/Projectiles/FriendSlop/Textures/SM_IdolProjectile_FireBlack_BaseColor_00.png",
     "tex_name": "T_IdolProjectile_FireBlack_BaseColor", "mi": "MI_IdolProjectile_FireBlack"},
]
out = {"done": [], "errors": []}
MEL = unreal.MaterialEditingLibrary
EAL = unreal.EditorAssetLibrary
try:
    at = unreal.AssetToolsHelpers.get_asset_tools()
    master = EAL.load_asset(MASTER)
    for item in ITEMS:
        ttask = unreal.AssetImportTask()
        ttask.set_editor_property("automated", True)
        ttask.set_editor_property("destination_path", DEST + "/Textures")
        ttask.set_editor_property("destination_name", item["tex_name"])
        ttask.set_editor_property("filename", item["tex_file"])
        ttask.set_editor_property("replace_existing", True)
        ttask.set_editor_property("save", True)
        at.import_asset_tasks([ttask])
        tex = EAL.load_asset(f"{DEST}/Textures/{item['tex_name']}")
        if not tex:
            out["errors"].append("texture import failed: " + item["tex_name"])
            continue
        mi_path = f"{DEST}/{item['mi']}"
        if EAL.does_asset_exist(mi_path):
            EAL.delete_asset(mi_path)
        mi = at.create_asset(item["mi"], DEST, unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
        MEL.set_material_instance_parent(mi, master)
        MEL.set_material_instance_texture_parameter_value(mi, "BaseColorTexture", tex)
        EAL.save_asset(mi_path, only_if_is_dirty=False)
        mesh = EAL.load_asset(item["mesh"])
        mats = mesh.get_editor_property("static_materials")
        for m in mats:
            m.set_editor_property("material_interface", mi)
        mesh.set_editor_property("static_materials", mats)
        EAL.save_asset(item["mesh"], only_if_is_dirty=False)
        out["done"].append({"mesh": item["mesh"], "mi": mi_path, "tex": f"{DEST}/Textures/{item['tex_name']}"})
    junk = f"{DEST}/Material_0"
    if EAL.does_asset_exist(junk):
        refs = [str(r) for r in EAL.find_package_referencers_for_asset(junk, load_assets_to_confirm=True) if str(r).startswith("/Game")]
        if not refs:
            EAL.delete_asset(junk)
            out["junk_material_deleted"] = True
        else:
            out["junk_material_refs"] = refs[:4]
finally:
    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    with open(OUT, "w", encoding="utf-8") as f:
        json.dump(out, f, indent=2)
    unreal.log("[FinishProjectiles] " + json.dumps(out))
    try: unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")
    except Exception: pass
