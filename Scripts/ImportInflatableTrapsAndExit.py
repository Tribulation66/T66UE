"""Import the InflatableTraps01 kit (GLB meshes + pattern textures); create FriendSlop
master instances; bind slot defaults. Self-quits. Modeled on ImportFriendSlopProjectilesAndExit.

Run: UnrealEditor.exe T66.uproject -ExecutePythonScript="C:/UE/T66/Scripts/ImportInflatableTrapsAndExit.py" -unattended -nop4 -nosplash
"""
from __future__ import annotations
import json, os
import unreal

OUT = r"C:/UE/T66/Saved/CombatTest/InflatableTraps01/import_inflatable_traps.json"
MASTER = "/Game/Materials/M_FriendSlop_FallGuys"
DEST = "/Game/World/Traps/Inflatable"
SRC_MESH = r"C:/UE/T66/SourceAssets/Import/WorldKit/InflatableTraps01/Meshes"
SRC_TEX = r"C:/UE/T66/SourceAssets/Import/WorldKit/InflatableTraps01/Textures"

TEXTURES = [
    "T_Inflatable_StripesDiag",
    "T_Inflatable_BandsHoriz",
    "T_Inflatable_Chevrons",
    "T_Inflatable_Stars",
    "T_Inflatable_Dots",
]
# mesh -> default pattern MI (C++ also sets materials explicitly; defaults make meshes self-contained)
MESHES = {
    "SM_Inflatable_SweeperArm": "MI_Inflatable_StripesDiag",
    "SM_Inflatable_Hub": "MI_Inflatable_BandsHoriz",
    "SM_Inflatable_Bumper": "MI_Inflatable_BandsHoriz",
    "SM_Inflatable_Pad": "MI_Inflatable_Chevrons",
    "SM_Inflatable_Mallet": "MI_Inflatable_Stars",
    "SM_Inflatable_Tube": "MI_Inflatable_BandsHoriz",
    "SM_Inflatable_SpikeBall": "MI_Inflatable_Dots",
}

out = {"textures": [], "materials": [], "meshes": [], "errors": []}
MEL = unreal.MaterialEditingLibrary
EAL = unreal.EditorAssetLibrary
AT = unreal.AssetToolsHelpers.get_asset_tools()


def import_file(filename, destination_path, destination_name=None):
    task = unreal.AssetImportTask()
    task.set_editor_property("automated", True)
    task.set_editor_property("destination_path", destination_path)
    if destination_name:
        task.set_editor_property("destination_name", destination_name)
    task.set_editor_property("filename", filename)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("save", True)
    AT.import_asset_tasks([task])


try:
    # 1) textures
    for tex_name in TEXTURES:
        src = f"{SRC_TEX}/{tex_name}.png"
        if not os.path.isfile(src):
            out["errors"].append(f"missing texture source: {src}")
            continue
        import_file(src, DEST + "/Textures")
        tex_path = f"{DEST}/Textures/{tex_name}"
        if EAL.does_asset_exist(tex_path):
            out["textures"].append(tex_path)
        else:
            out["errors"].append(f"texture import failed: {tex_path}")

    # 2) pattern MIs of the FriendSlop master
    master = EAL.load_asset(MASTER)
    if not master:
        out["errors"].append(f"master material missing: {MASTER}")
    else:
        for tex_name in TEXTURES:
            mi_name = "MI_Inflatable_" + tex_name.split("T_Inflatable_")[-1]
            tex_path = f"{DEST}/Textures/{tex_name}"
            tex = EAL.load_asset(tex_path)
            if not tex:
                out["errors"].append(f"texture missing for MI: {tex_path}")
                continue
            mi_path = f"{DEST}/{mi_name}"
            if EAL.does_asset_exist(mi_path):
                EAL.delete_asset(mi_path)
            mi = AT.create_asset(mi_name, DEST, unreal.MaterialInstanceConstant,
                                 unreal.MaterialInstanceConstantFactoryNew())
            MEL.set_material_instance_parent(mi, master)
            MEL.set_material_instance_texture_parameter_value(mi, "BaseColorTexture", tex)
            EAL.save_asset(mi_path, only_if_is_dirty=False)
            out["materials"].append(mi_path)

    # 3) meshes + slot defaults
    for mesh_name, mi_name in MESHES.items():
        src = f"{SRC_MESH}/{mesh_name}.glb"
        if not os.path.isfile(src):
            out["errors"].append(f"missing mesh source: {src}")
            continue
        import_file(src, DEST, mesh_name)
        mesh_path = f"{DEST}/{mesh_name}"
        mesh = EAL.load_asset(mesh_path)
        if not mesh:
            out["errors"].append(f"mesh import failed: {mesh_path}")
            continue
        mi = EAL.load_asset(f"{DEST}/{mi_name}")
        if mi:
            mats = mesh.get_editor_property("static_materials")
            for m in mats:
                m.set_editor_property("material_interface", mi)
            mesh.set_editor_property("static_materials", mats)
        EAL.save_asset(mesh_path, only_if_is_dirty=False)
        tris = 0
        try:
            tris = mesh.get_num_triangles(0)
        except Exception:
            pass
        out["meshes"].append({"mesh": mesh_path, "mi": mi_name, "triangles": tris})
finally:
    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    with open(OUT, "w", encoding="utf-8") as f:
        json.dump(out, f, indent=2)
    unreal.log("[ImportInflatableTraps] " + json.dumps(out))
    try:
        unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")
    except Exception:
        pass
