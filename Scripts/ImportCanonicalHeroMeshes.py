import json
import os

import unreal


PROJECT_DIR = unreal.SystemLibrary.get_project_directory()
CANONICAL_DIR = os.environ.get(
    "T66_CANONICAL_HERO_FBX_DIR",
    os.path.join(PROJECT_DIR, "SourceAssets", "Import", "Heros", "_Canonicalized", "CanonicalFBX"),
)
EXPORT_REPORT = os.environ.get(
    "T66_HERO_EXPORT_REPORT",
    os.path.join(PROJECT_DIR, "SourceAssets", "Import", "Heros", "_Canonicalized", "Exports", "hero_mesh_export_report.json"),
)
OUTPUT_REPORT = os.environ.get(
    "T66_CANONICAL_HERO_IMPORT_REPORT",
    os.path.join(PROJECT_DIR, "Saved", "CanonicalHeroImportReport.json"),
)
TEST_ONLY_ASSET = os.environ.get("T66_CANONICAL_HERO_TEST_ASSET", "").strip()
TEST_ALL = os.environ.get("T66_CANONICAL_HERO_TEST_ALL", "0").strip() in ("1", "true", "TRUE", "yes")
IMPORT_UNIFORM_SCALE = float(os.environ.get("T66_CANONICAL_HERO_IMPORT_SCALE", "100.0"))
USE_EXISTING_SKELETON = os.environ.get("T66_CANONICAL_HERO_USE_EXISTING_SKELETON", "1").strip() in ("1", "true", "TRUE", "yes")
USE_GLB = os.environ.get("T66_CANONICAL_HERO_USE_GLB", "0").strip() in ("1", "true", "TRUE", "yes")
CONVERT_SCENE_UNIT = os.environ.get("T66_CANONICAL_HERO_CONVERT_SCENE_UNIT", "1").strip() in ("1", "true", "TRUE", "yes")
IMPORT_TRANSLATION_Z = float(os.environ.get("T66_CANONICAL_HERO_IMPORT_TRANSLATION_Z", "0.0"))
IMPORT_TRANSLATION_MAP = os.environ.get("T66_CANONICAL_HERO_IMPORT_TRANSLATION_MAP", "").strip()


def _asset_leaf(asset_path):
    return asset_path.rsplit("/", 1)[-1].split(".", 1)[0]


def _dest_from_asset(asset_path):
    package_path = asset_path.split(".", 1)[0]
    dest, name = package_path.rsplit("/", 1)
    return dest, name


def _bounds(asset):
    b = asset.get_bounds()
    return {
        "origin_cm": [float(b.origin.x), float(b.origin.y), float(b.origin.z)],
        "extent_cm": [float(b.box_extent.x), float(b.box_extent.y), float(b.box_extent.z)],
        "size_cm": [float(b.box_extent.x) * 2.0, float(b.box_extent.y) * 2.0, float(b.box_extent.z) * 2.0],
    }


def _make_fbx_import_options(existing_asset, import_translation_z, import_uniform_scale):
    options = unreal.FbxImportUI()
    options.set_editor_property("import_as_skeletal", True)
    options.set_editor_property("import_mesh", True)
    options.set_editor_property("import_animations", False)
    options.set_editor_property("import_materials", True)
    options.set_editor_property("import_textures", True)
    options.set_editor_property("mesh_type_to_import", unreal.FBXImportType.FBXIT_SKELETAL_MESH)

    try:
        skeleton = existing_asset.get_editor_property("skeleton") if (existing_asset and USE_EXISTING_SKELETON) else None
        if skeleton:
            options.set_editor_property("skeleton", skeleton)
    except Exception:
        pass

    try:
        data = options.get_editor_property("skeletal_mesh_import_data")
        data.set_editor_property("import_uniform_scale", import_uniform_scale)
        data.set_editor_property("convert_scene", True)
        data.set_editor_property("force_front_x_axis", False)
        data.set_editor_property("convert_scene_unit", CONVERT_SCENE_UNIT)
        if abs(import_translation_z) > 0.0001:
            data.set_editor_property("import_translation", unreal.Vector(0.0, 0.0, import_translation_z))
    except Exception as exc:
        unreal.log_warning(f"[ImportCanonicalHeroMeshes] Could not set all FBX skeletal import data: {exc}")

    return options


def _import_one(asset_path, canonical_fbx, test_only=False, import_translation_z=0.0, import_uniform_scale=IMPORT_UNIFORM_SCALE):
    existing_asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    dest, name = _dest_from_asset(asset_path)
    if test_only:
        dest = f"/Game/Characters/Heroes/_CanonicalImportTest/{name}"
        name = f"{name}_Test"
        if unreal.EditorAssetLibrary.does_directory_exist(dest):
            unreal.EditorAssetLibrary.delete_directory(dest)
        unreal.EditorAssetLibrary.make_directory(dest)

    task = unreal.AssetImportTask()
    task.automated = True
    task.save = True
    task.replace_existing = not test_only
    task.replace_existing_settings = False
    task.filename = canonical_fbx
    task.destination_path = dest
    task.destination_name = name
    if canonical_fbx.lower().endswith(".fbx"):
        task.options = _make_fbx_import_options(existing_asset, import_translation_z, import_uniform_scale)

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

    imported_paths = list(task.imported_object_paths or [])
    loaded = unreal.EditorAssetLibrary.load_asset(f"{dest}/{name}.{name}")
    report = {
        "asset": asset_path,
        "canonical_fbx": canonical_fbx,
        "import_translation_z": import_translation_z,
        "import_uniform_scale": import_uniform_scale,
        "destination": f"{dest}/{name}.{name}",
        "imported_paths": imported_paths,
        "loaded": bool(loaded),
    }
    if loaded:
        report["class"] = loaded.get_class().get_name()
        report["bounds"] = _bounds(loaded)
        try:
            skeleton = loaded.get_editor_property("skeleton")
            report["skeleton"] = skeleton.get_path_name() if skeleton else None
        except Exception:
            pass
    return report


def main():
    with open(EXPORT_REPORT, "r", encoding="utf-8") as handle:
        export_report = json.load(handle)

    translation_map = {}
    if IMPORT_TRANSLATION_MAP:
        with open(IMPORT_TRANSLATION_MAP, "r", encoding="utf-8") as handle:
            translation_map = json.load(handle)

    reports = []
    for entry in export_report.get("reports", []):
        asset_path = entry.get("asset", "")
        size_cm = ((entry.get("bounds") or {}).get("size_cm") or [0.0, 0.0, 0.0])
        if TEST_ONLY_ASSET and asset_path != TEST_ONLY_ASSET:
            continue
        if not TEST_ONLY_ASSET and not TEST_ALL and abs(float(size_cm[2]) - 200.0) <= 0.5:
            continue

        leaf = _asset_leaf(asset_path)
        canonical_fbx = os.path.join(CANONICAL_DIR, f"{leaf}_Canonical2m.{'glb' if USE_GLB else 'fbx'}")
        if not os.path.isfile(canonical_fbx):
            reports.append({
                "asset": asset_path,
                "canonical_fbx": canonical_fbx,
                "error": "canonical fbx missing",
            })
            continue

        map_value = translation_map.get(asset_path, None)
        import_translation_z = IMPORT_TRANSLATION_Z
        import_uniform_scale = IMPORT_UNIFORM_SCALE
        if isinstance(map_value, dict):
            import_translation_z = float(map_value.get("translation_z", import_translation_z))
            import_uniform_scale = float(map_value.get("import_scale", import_uniform_scale))
        elif map_value is not None:
            import_translation_z = float(map_value)
        reports.append(_import_one(
            asset_path,
            canonical_fbx,
            test_only=bool(TEST_ONLY_ASSET) or TEST_ALL,
            import_translation_z=import_translation_z,
            import_uniform_scale=import_uniform_scale))

    os.makedirs(os.path.dirname(OUTPUT_REPORT), exist_ok=True)
    with open(OUTPUT_REPORT, "w", encoding="utf-8") as handle:
        json.dump({"count": len(reports), "reports": reports}, handle, indent=2)
    unreal.log(f"[ImportCanonicalHeroMeshes] wrote {OUTPUT_REPORT}")


if __name__ == "__main__":
    main()
