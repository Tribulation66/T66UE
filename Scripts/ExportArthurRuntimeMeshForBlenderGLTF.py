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
OUTPUT_GLB = os.path.join(OUTPUT_DIR, "ArthurAIdle_Hero_1_Chad_RuntimeMesh.glb")
REPORT_PATH = os.path.join(OUTPUT_DIR, "ArthurAIdle_Hero_1_Chad_RuntimeMesh_GLBReport.json")


def asset_path(obj):
    if not obj:
        return None
    try:
        return obj.get_path_name()
    except Exception:
        return str(obj)


def vec3(value):
    return [float(value.x), float(value.y), float(value.z)]


def bounds_info(asset):
    info = {}
    try:
        bounds = asset.get_bounds()
        info["bounds_origin_cm"] = vec3(bounds.origin)
        info["bounds_extent_cm"] = vec3(bounds.box_extent)
        info["bounds_size_cm"] = [
            float(bounds.box_extent.x) * 2.0,
            float(bounds.box_extent.y) * 2.0,
            float(bounds.box_extent.z) * 2.0,
        ]
    except Exception as exc:
        info["bounds_error"] = str(exc)
    return info


def main():
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    report = {
        "asset_path": ASSET_PATH,
        "output_glb": OUTPUT_GLB,
        "runtime_visual_id": "Hero_1_Chad",
        "runtime_mesh_relative_rotation": "(Pitch=0,Yaw=-90,Roll=0)",
        "runtime_mesh_relative_scale": [1.0, 1.0, 1.0],
        "runtime_auto_ground_to_actor_origin": True,
        "gltf_export_uniform_scale": 0.01,
    }

    try:
        asset = unreal.EditorAssetLibrary.load_asset(ASSET_PATH)
        report["loaded"] = asset is not None
        report["asset_class"] = asset.get_class().get_name() if asset else None
        report["asset_name"] = asset.get_name() if asset else None
        report["asset_path_loaded"] = asset_path(asset)
        if not asset:
            raise RuntimeError(f"Could not load asset: {ASSET_PATH}")

        report.update(bounds_info(asset))

        options = unreal.GLTFExportOptions()
        options.set_editor_property("export_uniform_scale", 0.01)
        options.set_editor_property("export_vertex_skin_weights", True)
        options.set_editor_property("export_morph_targets", False)
        options.set_editor_property("export_preview_mesh", True)
        options.set_editor_property("export_unlit_materials", True)

        success = unreal.GLTFExporter.export_to_gltf(asset, OUTPUT_GLB, options, set())
        report["export_success"] = bool(success)
        report["output_exists"] = os.path.exists(OUTPUT_GLB)
        report["output_bytes"] = os.path.getsize(OUTPUT_GLB) if os.path.exists(OUTPUT_GLB) else 0
    except Exception as exc:
        report["error"] = str(exc)
        report["traceback"] = traceback.format_exc()
        unreal.log_error(f"[ExportArthurRuntimeMeshForBlenderGLTF] Failed: {exc}")

    with open(REPORT_PATH, "w", encoding="utf-8") as handle:
        json.dump(report, handle, indent=2)

    unreal.log(f"[ExportArthurRuntimeMeshForBlenderGLTF] Wrote {REPORT_PATH}")
    unreal.log("[ExportArthurRuntimeMeshForBlenderGLTF] DONE")


if __name__ == "__main__":
    main()
