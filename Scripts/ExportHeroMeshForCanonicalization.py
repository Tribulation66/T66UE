import json
import os
import csv

import unreal


PROJECT_DIR = unreal.SystemLibrary.get_project_directory()
ASSET_PATH = os.environ.get(
    "T66_EXPORT_HERO_ASSET",
    "/Game/Characters/Heroes/Hero_3/Chad/SK_Hero_3_Chad.SK_Hero_3_Chad",
)
OUTPUT_DIR = os.environ.get(
    "T66_EXPORT_HERO_OUTPUT_DIR",
    os.path.join(PROJECT_DIR, "SourceAssets", "Import", "Heros", "_Canonicalized", "Exports"),
)


def _asset_name_from_path(path):
    return path.rsplit("/", 1)[-1].split(".", 1)[0]


def _normalize_asset_path(path):
    path = (path or "").strip().strip('"')
    return path


def _playable_hero_asset_paths():
    csv_path = os.path.join(PROJECT_DIR, "Content", "Data", "CharacterVisuals.csv")
    paths = []
    seen = set()
    with open(csv_path, newline="", encoding="utf-8-sig") as handle:
        for row in csv.DictReader(handle):
            visual_id = row.get("---", "")
            if not visual_id.startswith("Hero_"):
                continue
            mesh_path = _normalize_asset_path(row.get("SkeletalMesh", ""))
            if mesh_path and mesh_path not in seen:
                seen.add(mesh_path)
                paths.append(mesh_path)
    return paths


def _bounds(asset):
    b = asset.get_bounds()
    return {
        "origin_cm": [float(b.origin.x), float(b.origin.y), float(b.origin.z)],
        "extent_cm": [float(b.box_extent.x), float(b.box_extent.y), float(b.box_extent.z)],
        "size_cm": [float(b.box_extent.x) * 2.0, float(b.box_extent.y) * 2.0, float(b.box_extent.z) * 2.0],
    }


def _export_fbx(asset, output_fbx):
    task = unreal.AssetExportTask()
    task.object = asset
    task.filename = output_fbx
    task.automated = True
    task.prompt = False
    task.replace_identical = True
    task.exporter = unreal.SkeletalMeshExporterFBX()
    try:
        task.options = unreal.FbxExportOption()
    except Exception:
        pass
    return bool(unreal.Exporter.run_asset_export_task(task))


def _export_glb(asset, output_glb):
    options = unreal.GLTFExportOptions()
    options.set_editor_property("export_uniform_scale", 0.01)
    options.set_editor_property("export_vertex_skin_weights", True)
    options.set_editor_property("export_morph_targets", False)
    options.set_editor_property("export_preview_mesh", True)
    options.set_editor_property("export_unlit_materials", True)
    return bool(unreal.GLTFExporter.export_to_gltf(asset, output_glb, options, set()))


def main():
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    asset_paths = [ASSET_PATH]
    if os.environ.get("T66_EXPORT_ALL_PLAYABLE_HEROS", "").strip() in ("1", "true", "TRUE", "yes"):
        asset_paths = _playable_hero_asset_paths()

    reports = []
    for asset_path in asset_paths:
        asset = unreal.EditorAssetLibrary.load_asset(asset_path)
        name = _asset_name_from_path(asset_path)
        output_fbx = os.path.join(OUTPUT_DIR, f"{name}.fbx")
        output_glb = os.path.join(OUTPUT_DIR, f"{name}.glb")

        report = {
            "asset": asset_path,
            "loaded": bool(asset),
            "output_fbx": output_fbx,
            "output_glb": output_glb,
        }
        if asset:
            report["class"] = asset.get_class().get_name()
            report["bounds"] = _bounds(asset)
            if os.environ.get("T66_EXPORT_HERO_SKIP_FBX", "1").strip() not in ("1", "true", "TRUE", "yes"):
                try:
                    report["fbx_success"] = _export_fbx(asset, output_fbx)
                except Exception as exc:
                    report["fbx_error"] = str(exc)
            else:
                report["fbx_skipped"] = True
            try:
                report["glb_success"] = _export_glb(asset, output_glb)
            except Exception as exc:
                report["glb_error"] = str(exc)
            report["fbx_exists"] = os.path.exists(output_fbx)
            report["glb_exists"] = os.path.exists(output_glb)
        reports.append(report)

        report_path = os.path.join(OUTPUT_DIR, f"{name}_export_report.json")
        with open(report_path, "w", encoding="utf-8") as handle:
            json.dump(report, handle, indent=2)
        unreal.log(f"[ExportHeroMeshForCanonicalization] wrote {report_path}")

    batch_report_path = os.path.join(OUTPUT_DIR, "hero_mesh_export_report.json")
    with open(batch_report_path, "w", encoding="utf-8") as handle:
        json.dump({"count": len(reports), "reports": reports}, handle, indent=2)
    unreal.log(f"[ExportHeroMeshForCanonicalization] wrote {batch_report_path}")


if __name__ == "__main__":
    main()
