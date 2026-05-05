import json
import os

import unreal


PROJECT_DIR = unreal.SystemLibrary.get_project_directory()
REPORT_PATH = os.path.join(PROJECT_DIR, "Saved", "HeroMeshAssetDetails.json")

ASSETS = [
    "/Game/Characters/Heroes/Hero_1/Chad/Idle/ArthurAIdle.ArthurAIdle",
    "/Game/Characters/Heroes/Hero_1/Stacy/Idle/ArthurBIdle.ArthurBIdle",
    "/Game/Characters/Heroes/Hero_3/Chad/SK_Hero_3_Chad.SK_Hero_3_Chad",
]

if os.environ.get("T66_INSPECT_MESH_ASSETS", "").strip():
    ASSETS = [
        path.strip()
        for path in os.environ["T66_INSPECT_MESH_ASSETS"].split(";")
        if path.strip()
    ]


def _vec(value):
    return [float(value.x), float(value.y), float(value.z)]


def _bounds(asset):
    bounds = asset.get_bounds()
    return {
        "origin": _vec(bounds.origin),
        "extent": _vec(bounds.box_extent),
        "size": [
            float(bounds.box_extent.x) * 2.0,
            float(bounds.box_extent.y) * 2.0,
            float(bounds.box_extent.z) * 2.0,
        ],
    }


def _import_sources(asset):
    results = []
    try:
        import_data = asset.get_editor_property("asset_import_data")
    except Exception as exc:
        return [{"error": str(exc)}]

    if not import_data:
        return []

    try:
        source_files = import_data.get_editor_property("source_data").get_editor_property("source_files")
        for source_file in source_files:
            entry = {}
            for prop in ("relative_filename", "display_label", "timestamp", "file_hash"):
                try:
                    value = source_file.get_editor_property(prop)
                    entry[prop] = str(value)
                except Exception:
                    pass
            results.append(entry)
    except Exception as exc:
        results.append({"source_data_error": str(exc)})

    try:
        first = import_data.get_first_filename()
        if first:
            results.append({"first_filename": first})
    except Exception as exc:
        results.append({"first_filename_error": str(exc)})

    return results


def main():
    report = {
        "assets": [],
        "exporters": [
            name for name in dir(unreal)
            if "Export" in name or "FBX" in name or "Fbx" in name or "GLTF" in name
        ],
    }

    for path in ASSETS:
        asset = unreal.EditorAssetLibrary.load_asset(path)
        entry = {
            "path": path,
            "loaded": bool(asset),
            "class": asset.get_class().get_name() if asset else None,
        }
        if asset:
            entry["bounds"] = _bounds(asset)
            entry["import_sources"] = _import_sources(asset)
            try:
                entry["skeleton"] = asset.get_editor_property("skeleton").get_path_name()
            except Exception as exc:
                entry["skeleton_error"] = str(exc)
        report["assets"].append(entry)

    os.makedirs(os.path.dirname(REPORT_PATH), exist_ok=True)
    with open(REPORT_PATH, "w", encoding="utf-8") as handle:
        json.dump(report, handle, indent=2)
    unreal.log(f"[InspectHeroMeshAssetDetails] wrote {REPORT_PATH}")


if __name__ == "__main__":
    main()
