"""Flatten Interchange's nested GLB import output to the paths the C++ loads, bind
pattern MI slot defaults, save, and clean the leftover folders. Self-quits.

Run: UnrealEditor.exe T66.uproject -ExecutePythonScript="C:/UE/T66/Scripts/FinishInflatableTrapsAndExit.py" -unattended -nop4 -nosplash
"""
from __future__ import annotations
import json, os
import unreal

OUT = r"C:/UE/T66/Saved/CombatTest/InflatableTraps01/finish_inflatable_traps.json"
DEST = "/Game/World/Traps/Inflatable"
MESHES = {
    "SM_Inflatable_SweeperArm": "MI_Inflatable_StripesDiag",
    "SM_Inflatable_Hub": "MI_Inflatable_BandsHoriz",
    "SM_Inflatable_Bumper": "MI_Inflatable_BandsHoriz",
    "SM_Inflatable_Pad": "MI_Inflatable_Chevrons",
    "SM_Inflatable_Mallet": "MI_Inflatable_Stars",
    "SM_Inflatable_Tube": "MI_Inflatable_BandsHoriz",
    "SM_Inflatable_SpikeBall": "MI_Inflatable_Dots",
}

out = {"meshes": [], "errors": []}
EAL = unreal.EditorAssetLibrary

try:
    for mesh_name, mi_name in MESHES.items():
        flat_path = f"{DEST}/{mesh_name}"
        nested_path = f"{DEST}/{mesh_name}/StaticMeshes/{mesh_name}"
        if not EAL.does_asset_exist(flat_path):
            if not EAL.does_asset_exist(nested_path):
                out["errors"].append(f"missing both flat and nested: {mesh_name}")
                continue
            if not EAL.rename_asset(nested_path, flat_path):
                out["errors"].append(f"rename failed: {nested_path}")
                continue

        mesh = EAL.load_asset(flat_path)
        if not mesh:
            out["errors"].append(f"load failed after flatten: {flat_path}")
            continue

        mi = EAL.load_asset(f"{DEST}/{mi_name}")
        if mi:
            mats = mesh.get_editor_property("static_materials")
            for m in mats:
                m.set_editor_property("material_interface", mi)
            mesh.set_editor_property("static_materials", mats)
        EAL.save_asset(flat_path, only_if_is_dirty=False)

        # remove the now-empty nested import folder (and its redirector)
        nested_dir = f"{DEST}/{mesh_name}"
        # the flat asset shares the name with the directory; only delete the SUBfolder
        sub_dir = f"{DEST}/{mesh_name}/StaticMeshes"
        if EAL.does_directory_exist(sub_dir):
            EAL.delete_directory(sub_dir)

        tris = 0
        try:
            tris = mesh.get_num_triangles(0)
        except Exception:
            pass
        out["meshes"].append({"mesh": flat_path, "mi": mi_name, "triangles": tris})
finally:
    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    with open(OUT, "w", encoding="utf-8") as f:
        json.dump(out, f, indent=2)
    unreal.log("[FinishInflatableTraps] " + json.dumps(out))
    try:
        unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")
    except Exception:
        pass
