import json
import os
import traceback

import unreal


ASSET_PATH = "/Game/Characters/Heroes/Hero_1/Chad/Idle/ArthurAIdle.ArthurAIdle"
PROJECT_DIR = unreal.SystemLibrary.get_project_directory()
OUTPUT_DIR = os.path.join(
    PROJECT_DIR,
    "Model Generation",
    "Runs",
    "Arthur",
    "Exports",
    "Runtime",
)
OUTPUT_FBX = os.path.join(OUTPUT_DIR, "ArthurAIdle_Hero_1_Chad_RuntimeMesh.fbx")
REPORT_PATH = os.path.join(OUTPUT_DIR, "ArthurAIdle_Hero_1_Chad_RuntimeMesh_ExportReport.json")


def asset_path(obj):
    if not obj:
        return None
    try:
        return obj.get_path_name()
    except Exception:
        return str(obj)


def get_bounds_info(asset):
    info = {}
    try:
        bounds = asset.get_bounds()
        info["bounds_origin"] = [bounds.origin.x, bounds.origin.y, bounds.origin.z]
        info["bounds_extent"] = [bounds.box_extent.x, bounds.box_extent.y, bounds.box_extent.z]
        info["bounds_sphere_radius"] = bounds.sphere_radius
    except Exception as exc:
        info["bounds_error"] = str(exc)
    try:
        imported_bounds = asset.get_editor_property("imported_bounds")
        info["imported_bounds_origin"] = [
            imported_bounds.origin.x,
            imported_bounds.origin.y,
            imported_bounds.origin.z,
        ]
        info["imported_bounds_extent"] = [
            imported_bounds.box_extent.x,
            imported_bounds.box_extent.y,
            imported_bounds.box_extent.z,
        ]
    except Exception as exc:
        info["imported_bounds_error"] = str(exc)
    return info


def run_export(asset):
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    task = unreal.AssetExportTask()
    task.object = asset
    task.filename = OUTPUT_FBX
    task.automated = True
    task.prompt = False
    task.replace_identical = True

    try:
        options = unreal.FbxExportOption()
        task.options = options
    except Exception:
        pass

    return bool(unreal.Exporter.run_asset_export_task(task))


def main():
    report = {
        "asset_path": ASSET_PATH,
        "output_fbx": OUTPUT_FBX,
        "runtime_visual_id": "Hero_1_Chad",
        "runtime_mesh_relative_rotation": "(Pitch=0,Yaw=-90,Roll=0)",
        "runtime_mesh_relative_scale": "(X=1,Y=1,Z=1)",
        "runtime_auto_ground_to_actor_origin": True,
    }

    try:
        asset = unreal.EditorAssetLibrary.load_asset(ASSET_PATH)
        report["loaded"] = asset is not None
        report["asset_class"] = asset.get_class().get_name() if asset else None
        report["asset_name"] = asset.get_name() if asset else None
        report["asset_path_loaded"] = asset_path(asset)
        if not asset:
            raise RuntimeError(f"Could not load asset: {ASSET_PATH}")

        report.update(get_bounds_info(asset))
        report["export_success"] = run_export(asset)
        report["output_exists"] = os.path.exists(OUTPUT_FBX)
        report["output_bytes"] = os.path.getsize(OUTPUT_FBX) if os.path.exists(OUTPUT_FBX) else 0
    except Exception as exc:
        report["error"] = str(exc)
        report["traceback"] = traceback.format_exc()
        report["exporter_names"] = [
            name
            for name in dir(unreal)
            if "Export" in name or "FBX" in name or "Fbx" in name
        ]
        unreal.log_error(f"[ExportArthurRuntimeMeshForBlender] Failed: {exc}")

    os.makedirs(OUTPUT_DIR, exist_ok=True)
    with open(REPORT_PATH, "w", encoding="utf-8") as handle:
        json.dump(report, handle, indent=2)

    unreal.log(f"[ExportArthurRuntimeMeshForBlender] Wrote {REPORT_PATH}")
    unreal.log("[ExportArthurRuntimeMeshForBlender] DONE")


if __name__ == "__main__":
    main()
