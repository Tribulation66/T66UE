"""
Import the historical TypeA_Masculine_Batch01 rigged hero FBXs and wire their generated
idle animations to predictable asset paths.

Run through the full editor wrapper:
  UnrealEditor.exe T66.uproject -ExecutePythonScript="Scripts/RunImportTypeABatch01RiggedHeroesAndExit.py"
"""

import json
import os
import sys

import unreal


SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
if SCRIPT_DIR not in sys.path:
    sys.path.append(SCRIPT_DIR)

import MakeCharacterMaterialsUnlit


BATCH_REPORT = os.path.join(
    unreal.SystemLibrary.get_project_directory(),
    "Model Generation",
    "Runs",
    "Heroes",
    "TypeA_Masculine_Batch01",
    "Rigging",
    "batch_typea_rig_report.json",
)
OUTPUT_REPORT = os.path.join(
    unreal.SystemLibrary.get_project_directory(),
    "Saved",
    "ChadBatch01RiggedHeroImportReport.json",
)
SCALE_MAP = os.path.join(
    unreal.SystemLibrary.get_project_directory(),
    "Model Generation",
    "Runs",
    "Heroes",
    "TypeA_Masculine_Batch01",
    "Rigging",
    "unreal_import_scale_map.json",
)
TARGET_HERO_HEIGHT_CM = 200.0
DEFAULT_IMPORT_UNIFORM_SCALE = 100.0
BLENDER_AUTHORED_IMPORT_UNIFORM_SCALE = 1.0
PREVIOUS_SCALE_BY_KEY = {}
SCALE_BY_KEY = {}


def _set_property(obj, name, value):
    try:
        obj.set_editor_property(name, value)
        return True
    except Exception:
        return False


def _game_path(object_path):
    return object_path.split(".", 1)[0]


def _load(object_path):
    return unreal.EditorAssetLibrary.load_asset(_game_path(object_path))


def _load_clean(clean_path):
    return unreal.EditorAssetLibrary.load_asset(clean_path)


def _canonicalize_body_path(path):
    return (
        (path or "")
        .replace("/TypeA", "/Chad")
        .replace("_TypeA", "_Chad")
        .replace("/TypeB", "/Stacy")
        .replace("_TypeB", "_Stacy")
    )


def _delete_if_exists(path):
    clean = _game_path(path)
    if unreal.EditorAssetLibrary.does_asset_exist(clean):
        try:
            unreal.EditorAssetLibrary.delete_asset(clean)
            return True
        except Exception as exc:
            unreal.log_warning(f"[ImportTypeABatch01] Could not delete stale asset {clean}: {exc}")
            return False
    return True


def _clean_target_assets(dest, mesh_name, idle_name):
    # Mesh imports use replace_existing. Only pre-clean the fresh animation target;
    # older generated RigIdle assets can have invalid skeleton references and may
    # fail to load during deletion.
    for asset_name in (
        idle_name,
        f"{idle_name}_Anim",
    ):
        _delete_if_exists(f"{dest}/{asset_name}")


def _desired_idle_object(entry):
    dest = _canonicalize_body_path(entry["unreal"]["destination"])
    suffix = "_Beachgoer" if entry["skin"] != "Standard" else ""
    idle_name = f"AM_Hero_{entry['hero_id']}_Chad{suffix}_RigIdleV2"
    return f"{dest}/{idle_name}.{idle_name}"


def _report_key(entry):
    return f"{entry['hero_id']}|{entry['skin']}"


def _load_previous_scale_map():
    global PREVIOUS_SCALE_BY_KEY
    PREVIOUS_SCALE_BY_KEY = {}
    if not os.path.isfile(OUTPUT_REPORT):
        return
    try:
        with open(OUTPUT_REPORT, "r", encoding="utf-8") as handle:
            previous = json.load(handle)
    except Exception:
        return
    for report in previous.get("reports", []):
        size = ((report.get("bounds") or {}).get("size_cm") or [])
        if len(size) < 3:
            continue
        height = float(size[2])
        if height <= 1.0:
            continue
        PREVIOUS_SCALE_BY_KEY[_report_key(report)] = DEFAULT_IMPORT_UNIFORM_SCALE * TARGET_HERO_HEIGHT_CM / height


def _load_scale_map():
    global SCALE_BY_KEY
    SCALE_BY_KEY = {}
    if not os.path.isfile(SCALE_MAP):
        unreal.log_warning(f"[ImportTypeABatch01] Missing scale map, using fallback import scale: {SCALE_MAP}")
        return
    try:
        with open(SCALE_MAP, "r", encoding="utf-8") as handle:
            scale_map = json.load(handle)
    except Exception as exc:
        unreal.log_warning(f"[ImportTypeABatch01] Could not read scale map {SCALE_MAP}: {exc}")
        return
    for item in scale_map.get("entries", []):
        try:
            scale = float(item.get("import_uniform_scale", 0.0))
        except Exception:
            scale = 0.0
        if scale > 0.0:
            SCALE_BY_KEY[str(item.get("key", ""))] = scale
    unreal.log(f"[ImportTypeABatch01] Loaded {len(SCALE_BY_KEY)} import scales from {SCALE_MAP}")


def _import_scale_for_entry(entry):
    normalization = entry.get("normalization") or {}
    try:
        target_height_m = float(normalization.get("target_height_m", 0.0))
        normalized_size = normalization.get("normalized_size_m") or []
        normalized_height_m = float(normalized_size[2]) if len(normalized_size) >= 3 else 0.0
    except Exception:
        target_height_m = 0.0
        normalized_height_m = 0.0

    if abs(target_height_m - 2.0) <= 0.01 and 1.95 <= normalized_height_m <= 2.05:
        return BLENDER_AUTHORED_IMPORT_UNIFORM_SCALE

    scale = SCALE_BY_KEY.get(_report_key(entry), 0.0)
    if scale > 0.0:
        return max(0.1, min(10.0, scale))
    scale = PREVIOUS_SCALE_BY_KEY.get(_report_key(entry), 0.0)
    if scale > 0.0:
        return max(0.1, min(10.0, scale))
    return 2.0


def _make_mesh_import_options(import_uniform_scale):
    options = unreal.FbxImportUI()
    _set_property(options, "import_as_skeletal", True)
    _set_property(options, "import_mesh", True)
    _set_property(options, "import_animations", False)
    _set_property(options, "import_materials", True)
    _set_property(options, "import_textures", True)
    _set_property(options, "mesh_type_to_import", unreal.FBXImportType.FBXIT_SKELETAL_MESH)

    try:
        data = options.get_editor_property("skeletal_mesh_import_data")
        _set_property(data, "convert_scene", True)
        _set_property(data, "convert_scene_unit", True)
        _set_property(data, "force_front_x_axis", False)
        _set_property(data, "import_uniform_scale", import_uniform_scale)
    except Exception as exc:
        unreal.log_warning(f"[ImportTypeABatch01] Mesh import data setup warning: {exc}")
    return options


def _make_anim_import_options(skeleton):
    options = unreal.FbxImportUI()
    _set_property(options, "import_as_skeletal", True)
    _set_property(options, "import_mesh", False)
    _set_property(options, "import_animations", True)
    _set_property(options, "import_materials", False)
    _set_property(options, "import_textures", False)
    _set_property(options, "mesh_type_to_import", unreal.FBXImportType.FBXIT_ANIMATION)
    _set_property(options, "skeleton", skeleton)

    try:
        data = options.get_editor_property("anim_sequence_import_data")
        _set_property(data, "import_custom_attribute", False)
        _set_property(data, "delete_existing_custom_attribute_curves", True)
        _set_property(data, "convert_scene", True)
        _set_property(data, "convert_scene_unit", True)
    except Exception as exc:
        unreal.log_warning(f"[ImportTypeABatch01] Anim import data setup warning: {exc}")
    return options


def _import_task(filename, dest, name, options):
    task = unreal.AssetImportTask()
    task.automated = True
    task.save = True
    task.replace_existing = True
    task.replace_existing_settings = False
    task.filename = filename
    task.destination_path = dest
    task.destination_name = name
    if options is not None:
        task.options = options
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    return list(task.imported_object_paths or [])


def _import_texture(filename, dest, name):
    if not filename or not os.path.isfile(filename):
        raise RuntimeError(f"Missing texture source: {filename}")

    paths = _import_task(filename, dest, name, None)
    texture = _load_clean(f"{dest}/{name}")
    if not texture or not isinstance(texture, unreal.Texture2D):
        raise RuntimeError(f"Texture import failed: {filename}; imported={paths}")

    _set_property(texture, "srgb", True)
    try:
        _set_property(texture, "lod_group", unreal.TextureGroup.TEXTUREGROUP_CHARACTER)
    except Exception:
        pass
    unreal.EditorAssetLibrary.save_asset(f"{dest}/{name}")
    return texture.get_path_name()


def _import_textures_for_entry(entry, dest):
    imported = []
    for texture_entry in entry.get("textures", []):
        texture_name = _canonicalize_body_path(texture_entry.get("texture_asset_name"))
        texture_path = texture_entry.get("base_color_png")
        if not texture_name or not texture_path:
            continue
        imported.append(
            {
                "material_name": texture_entry.get("material_name", ""),
                "texture": _import_texture(texture_path, dest, texture_name),
            }
        )
    return imported


def _find_anim_sequence(paths):
    for path in paths:
        asset = unreal.EditorAssetLibrary.load_asset(path)
        if asset and asset.get_class().get_name() == "AnimSequence":
            return path, asset
    return None, None


def _normalize_anim_path(imported_paths, dest, idle_name):
    desired_clean = f"{dest}/{idle_name}"
    desired_object = f"{desired_clean}.{idle_name}"
    if unreal.EditorAssetLibrary.does_asset_exist(desired_clean):
        asset = unreal.EditorAssetLibrary.load_asset(desired_clean)
        if asset and asset.get_class().get_name() == "AnimSequence":
            unreal.EditorAssetLibrary.save_asset(desired_clean)
            return desired_object

    imported_path, imported_asset = _find_anim_sequence(imported_paths)
    if not imported_asset:
        raise RuntimeError(f"No AnimSequence imported for {desired_clean}; imported={imported_paths}")

    source_clean = _game_path(imported_path)
    if source_clean != desired_clean:
        _delete_if_exists(desired_clean)
        if not unreal.EditorAssetLibrary.rename_asset(source_clean, desired_clean):
            raise RuntimeError(f"Failed to rename {source_clean} -> {desired_clean}")
    unreal.EditorAssetLibrary.save_asset(desired_clean)
    return desired_object


def _mesh_bounds(asset):
    try:
        bounds = asset.get_bounds()
        return {
            "origin_cm": [float(bounds.origin.x), float(bounds.origin.y), float(bounds.origin.z)],
            "extent_cm": [float(bounds.box_extent.x), float(bounds.box_extent.y), float(bounds.box_extent.z)],
            "size_cm": [
                float(bounds.box_extent.x) * 2.0,
                float(bounds.box_extent.y) * 2.0,
                float(bounds.box_extent.z) * 2.0,
            ],
        }
    except Exception:
        return {}


def _import_one(entry):
    fbx = entry["exports"]["fbx"]
    dest = _canonicalize_body_path(entry["unreal"]["destination"])
    mesh_object = _canonicalize_body_path(entry["unreal"]["skeletal_mesh"])
    idle_object = _desired_idle_object(entry)
    mesh_name = _game_path(mesh_object).rsplit("/", 1)[-1]
    idle_name = _game_path(idle_object).rsplit("/", 1)[-1]
    import_uniform_scale = _import_scale_for_entry(entry)

    if not os.path.isfile(fbx):
        raise RuntimeError(f"Missing FBX: {fbx}")
    if not unreal.EditorAssetLibrary.does_directory_exist(dest):
        unreal.EditorAssetLibrary.make_directory(dest)

    _clean_target_assets(dest, mesh_name, idle_name)

    mesh_paths = _import_task(fbx, dest, mesh_name, _make_mesh_import_options(import_uniform_scale))
    mesh = _load(mesh_object)
    if not mesh or mesh.get_class().get_name() != "SkeletalMesh":
        raise RuntimeError(f"SkeletalMesh import failed: {mesh_object}; imported={mesh_paths}")

    skeleton = None
    try:
        skeleton = mesh.get_editor_property("skeleton")
    except Exception:
        pass
    if not skeleton:
        raise RuntimeError(f"Imported mesh has no skeleton: {mesh_object}")

    anim_paths = _import_task(fbx, dest, idle_name, _make_anim_import_options(skeleton))
    normalized_idle = _normalize_anim_path(anim_paths, dest, idle_name)

    imported_textures = _import_textures_for_entry(entry, dest)
    unlit_results = MakeCharacterMaterialsUnlit.convert_character_materials_unlit([dest])
    unreal.EditorAssetLibrary.save_asset(_game_path(mesh_object))
    unreal.EditorAssetLibrary.save_asset(_game_path(normalized_idle))

    return {
        "hero_id": entry["hero_id"],
        "hero_name": entry["hero_name"],
        "skin": entry["skin"],
        "fbx": fbx,
        "mesh": mesh_object,
        "idle": normalized_idle,
        "skeleton": skeleton.get_path_name() if skeleton else "",
        "import_uniform_scale": import_uniform_scale,
        "mesh_imported_paths": mesh_paths,
        "anim_imported_paths": anim_paths,
        "texture_imported_paths": imported_textures,
        "bounds": _mesh_bounds(mesh),
        "unlit": unlit_results,
    }


def main():
    _load_scale_map()
    _load_previous_scale_map()
    with open(BATCH_REPORT, "r", encoding="utf-8") as handle:
        batch = json.load(handle)

    if not batch.get("all_verified"):
        raise RuntimeError(f"Batch report is not verified: {BATCH_REPORT}")

    reports = []
    failures = []
    for entry in batch.get("reports", []):
        label = f"Hero_{entry['hero_id']}_{entry['skin']}"
        unreal.log(f"[ImportTypeABatch01] Importing {label}")
        try:
            reports.append(_import_one(entry))
        except Exception as exc:
            failures.append({"label": label, "error": repr(exc)})
            unreal.log_error(f"[ImportTypeABatch01] Failed {label}: {exc!r}")

    os.makedirs(os.path.dirname(OUTPUT_REPORT), exist_ok=True)
    with open(OUTPUT_REPORT, "w", encoding="utf-8") as handle:
        json.dump({"count": len(reports), "failures": failures, "reports": reports}, handle, indent=2)

    unreal.log(f"[ImportTypeABatch01] Wrote {OUTPUT_REPORT}")
    if failures:
        raise RuntimeError(f"Import failures: {len(failures)}")


if __name__ == "__main__":
    main()
