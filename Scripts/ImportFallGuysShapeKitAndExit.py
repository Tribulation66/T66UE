"""Import FallGuysShapeKit01 (hex/tri platform prisms) into /Game/World/Terrain/FallGuysKit,
flatten the Interchange nesting, generate 1-hull convex collision (the prisms are convex,
so the hull IS the mesh — exact-collision contract), and verify the 100x100x100 AABB.
Self-quits. Modeled on ImportInflatableTrapsAndExit.py.

Run: UnrealEditor.exe T66.uproject -ExecutePythonScript="C:/UE/T66/Scripts/ImportFallGuysShapeKitAndExit.py" -unattended -nop4 -nosplash
"""
from __future__ import annotations
import json, os
import unreal

OUT = r"C:/UE/T66/Saved/CombatTest/FallGuysKit/import_fallguys_shape_kit.json"
DEST = "/Game/World/Terrain/FallGuysKit"
SRC = r"C:/UE/T66/SourceAssets/Import/WorldKit/FallGuysShapeKit01/Meshes"
MESHES = ["SM_FGShape_Hex", "SM_FGShape_Tri"]

out = {"meshes": [], "errors": []}
EAL = unreal.EditorAssetLibrary
AT = unreal.AssetToolsHelpers.get_asset_tools()
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

        # Interchange nests GLB imports under Name/StaticMeshes/Name — flatten.
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

        # 1-hull convex decomposition: convex prism -> hull == mesh (exact).
        SMS.set_convex_decomposition_collisions(mesh, 1, 16, 100000)
        EAL.save_asset(final_path, only_if_is_dirty=False)

        bounds = mesh.get_bounds().box_extent
        agg = mesh.get_editor_property("body_setup").get_editor_property("agg_geom")
        convex_count = len(agg.get_editor_property("convex_elems"))
        out["meshes"].append({
            "path": final_path,
            "extent": [bounds.x, bounds.y, bounds.z],
            "convex_elems": convex_count,
        })
finally:
    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    with open(OUT, "w", encoding="utf-8") as f:
        json.dump(out, f, indent=2)
    unreal.log("[FGShapeKit] " + json.dumps(out))
    try:
        unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")
    except Exception:
        pass
