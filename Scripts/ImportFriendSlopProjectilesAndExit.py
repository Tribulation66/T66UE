"""Import FriendSlop projectile FBXs + raw albedos; create master instances. Self-quits."""
from __future__ import annotations
import json, os
import unreal

OUT = r"C:/UE/T66/Saved/CombatTest/FallGuys/import_projectiles.json"
MASTER = "/Game/Materials/M_FriendSlop_FallGuys"
DEST = "/Game/Weapons/Projectiles/FriendSlop"
ITEMS = [
    {"name": "SM_WeaponProjectile_Black",
     "fbx": r"C:/UE/T66/SourceAssets/Import/Weapons/Projectiles/FriendSlop/UnrealReady/SM_WeaponProjectile_Black_UnrealReady.fbx",
     "tex": r"C:/UE/T66/SourceAssets/Import/Weapons/Projectiles/FriendSlop/Textures/SM_WeaponProjectile_Black_BaseColor_00.png",
     "mi": "MI_WeaponProjectile_Black"},
    {"name": "SM_IdolProjectile_FireBlack",
     "fbx": r"C:/UE/T66/SourceAssets/Import/Weapons/Projectiles/FriendSlop/UnrealReady/SM_IdolProjectile_FireBlack_UnrealReady.fbx",
     "tex": r"C:/UE/T66/SourceAssets/Import/Weapons/Projectiles/FriendSlop/Textures/SM_IdolProjectile_FireBlack_BaseColor_00.png",
     "mi": "MI_IdolProjectile_FireBlack"},
]
out = {"imported": [], "errors": []}
MEL = unreal.MaterialEditingLibrary
EAL = unreal.EditorAssetLibrary

def import_one(item):
    at = unreal.AssetToolsHelpers.get_asset_tools()
    # mesh — NO legacy FbxImportUI options: UE5.7 routes FBX through Interchange in commandlets
    # and passing the legacy options object hard-crashed the import. Interchange defaults are fine.
    task = unreal.AssetImportTask()
    task.set_editor_property("automated", True)
    task.set_editor_property("destination_path", DEST)
    task.set_editor_property("destination_name", item["name"])
    task.set_editor_property("filename", item["fbx"])
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("save", True)
    # texture
    ttask = unreal.AssetImportTask()
    ttask.set_editor_property("automated", True)
    ttask.set_editor_property("destination_path", DEST + "/Textures")
    ttask.set_editor_property("filename", item["tex"])
    ttask.set_editor_property("replace_existing", True)
    ttask.set_editor_property("save", True)
    at.import_asset_tasks([task, ttask])

    mesh_path = f"{DEST}/{item['name']}"
    tex_path = f"{DEST}/Textures/{os.path.splitext(os.path.basename(item['tex']))[0]}"
    mesh = EAL.load_asset(mesh_path)
    tex = EAL.load_asset(tex_path)
    if not mesh or not tex:
        out["errors"].append(f"import failed: mesh={bool(mesh)} tex={bool(tex)} for {item['name']}")
        return
    master = EAL.load_asset(MASTER)
    mi_path = f"{DEST}/{item['mi']}"
    if EAL.does_asset_exist(mi_path):
        EAL.delete_asset(mi_path)
    mi = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        item["mi"], DEST, unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
    MEL.set_material_instance_parent(mi, master)
    MEL.set_material_instance_texture_parameter_value(mi, "BaseColorTexture", tex)
    EAL.save_asset(mi_path, only_if_is_dirty=False)
    # assign MI as the mesh's slot default so the mesh alone is ready
    sm = mesh
    mats = sm.get_editor_property("static_materials")
    for m in mats:
        m.set_editor_property("material_interface", mi)
    sm.set_editor_property("static_materials", mats)
    EAL.save_asset(mesh_path, only_if_is_dirty=False)
    out["imported"].append({"mesh": mesh_path, "mi": mi_path, "tex": tex_path})

try:
    for item in ITEMS:
        import_one(item)
finally:
    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    with open(OUT, "w", encoding="utf-8") as f:
        json.dump(out, f, indent=2)
    unreal.log("[ImportProjectiles] " + json.dumps(out))
    try: unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")
    except Exception: pass
