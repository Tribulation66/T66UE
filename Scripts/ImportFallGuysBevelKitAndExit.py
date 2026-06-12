"""Import the FallGuysShapeKit01 v2 BEVELED prisms (BevelCube/BevelPuck new,
Hex/Tri re-authored in place), generate 1-hull convex collision, verify the 100^3
AABBs, and create MI_FallGuys_Deck (coral, Tier 2 decks) + MI_FallGuys_Trim
(white, deck edge bands) as FriendSlop master instances. Self-quits.

Run: UnrealEditor.exe T66.uproject -ExecutePythonScript="C:/UE/T66/Scripts/ImportFallGuysBevelKitAndExit.py" -unattended -nop4 -nosplash
"""
from __future__ import annotations
import json, os
import unreal

OUT = r"C:/UE/T66/Saved/CombatTest/FallGuysKit/import_fallguys_bevel_kit.json"
DEST = "/Game/World/Terrain/FallGuysKit"
SRC = r"C:/UE/T66/SourceAssets/Import/WorldKit/FallGuysShapeKit01/Meshes"
MESHES = ["SM_FGShape_BevelCube", "SM_FGShape_BevelPuck", "SM_FGShape_Hex", "SM_FGShape_Tri"]

MASTER = "/Game/Materials/M_FriendSlop_FallGuys"
WHITE = "/Engine/EngineResources/WhiteSquareTexture"
NEW_MIS = {
    "MI_FallGuys_Deck": (0.96, 0.45, 0.20, 1.0),  # coral-orange: Tier 2 floating decks
    "MI_FallGuys_Trim": (0.96, 0.96, 0.94, 1.0),  # warm white: deck edge bands
}

out = {"meshes": [], "materials": [], "errors": []}
EAL = unreal.EditorAssetLibrary
AT = unreal.AssetToolsHelpers.get_asset_tools()
MEL = unreal.MaterialEditingLibrary
SMS = unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)


def import_file(filename, destination_path):
    task = unreal.AssetImportTask()
    task.set_editor_property("automated", True)
    task.set_editor_property("destination_path", destination_path)
    task.set_editor_property("filename", filename)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("save", True)
    AT.import_asset_tasks([task])


try:
    for name in MESHES:
        src = f"{SRC}/{name}.glb"
        if not os.path.isfile(src):
            out["errors"].append(f"missing source: {src}")
            continue

        final_path = f"{DEST}/{name}"
        if EAL.does_asset_exist(final_path):
            EAL.delete_asset(final_path)
        import_file(src, DEST)

        nested = f"{DEST}/{name}/StaticMeshes/{name}"
        if EAL.does_asset_exist(nested):
            EAL.rename_asset(nested, final_path)
            EAL.delete_directory(f"{DEST}/{name}")

        if not EAL.does_asset_exist(final_path):
            out["errors"].append(f"import produced no asset: {name}")
            continue

        mesh = EAL.load_asset(final_path)
        if not mesh:
            out["errors"].append(f"load failed: {name}")
            continue

        SMS.set_convex_decomposition_collisions(mesh, 1, 24, 100000)
        EAL.save_asset(final_path, only_if_is_dirty=False)

        bounds = mesh.get_bounds().box_extent
        agg = mesh.get_editor_property("body_setup").get_editor_property("agg_geom")
        convex_count = len(agg.get_editor_property("convex_elems"))
        out["meshes"].append({
            "path": final_path,
            "extent": [round(bounds.x, 2), round(bounds.y, 2), round(bounds.z, 2)],
            "convex_elems": convex_count,
        })

    master = EAL.load_asset(MASTER)
    white = EAL.load_asset(WHITE)
    if not master:
        out["errors"].append(f"master missing: {MASTER}")
    else:
        for name, rgba in NEW_MIS.items():
            mi_path = f"{DEST}/{name}"
            if EAL.does_asset_exist(mi_path):
                EAL.delete_asset(mi_path)
            mi = AT.create_asset(name, DEST, unreal.MaterialInstanceConstant,
                                 unreal.MaterialInstanceConstantFactoryNew())
            if not mi:
                out["errors"].append(f"create failed: {name}")
                continue
            MEL.set_material_instance_parent(mi, master)
            if white:
                MEL.set_material_instance_texture_parameter_value(mi, "BaseColorTexture", white)
            MEL.set_material_instance_vector_parameter_value(
                mi, "Tint", unreal.LinearColor(rgba[0], rgba[1], rgba[2], rgba[3]))
            EAL.save_asset(mi_path, only_if_is_dirty=False)
            out["materials"].append(mi_path)
finally:
    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    with open(OUT, "w", encoding="utf-8") as f:
        json.dump(out, f, indent=2)
    unreal.log("[FGBevelKit] " + json.dumps(out))
    try:
        unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")
    except Exception:
        pass
