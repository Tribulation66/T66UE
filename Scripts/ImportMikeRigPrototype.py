"""
Import and wire the Mike Chad Pass02 rig prototype for in-game testing.

This is intentionally Mike-only. It does not rerun or alter the completed
Batch01 roster import, except for replacing the logical Hero_3_Chad visual row
with this prototype so Mike can be tested in the game.
"""

import json
import os
import sys
from pathlib import Path

import unreal


SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
if SCRIPT_DIR not in sys.path:
    sys.path.append(SCRIPT_DIR)

import MakeCharacterMaterialsUnlit
import RepairMikeRigPrototypeMaterials


PROJECT_DIR = Path(unreal.SystemLibrary.get_project_directory())
RIG_DIR = (
    PROJECT_DIR
    / "Model Generation"
    / "Runs"
    / "Heroes"
    / "Chad_Pass02_ProcessBuild"
    / "Rigging"
    / "Mike_Chad_RigPrototype_A03_LiftedNeckBridge"
)
RIG_REPORT = RIG_DIR / "rig_report.json"
DEST = "/Game/Characters/Heroes/Hero_3/Chad/RigPrototype"
ROW_ID = "Hero_3_Chad"
MESH_NAME = "SK_Hero_3_Mike_Chad_RigPrototype"
IDLE_NAME = "AM_Hero_3_Mike_Chad_RigPrototype_Idle"
WALK_NAME = "AM_Hero_3_Mike_Chad_RigPrototype_WalkLegsOnly"
CSV_PATH = PROJECT_DIR / "Content" / "Data" / "CharacterVisuals.csv"
DT_PATH = "/Game/Data/DT_CharacterVisuals"
OUTPUT_REPORT = PROJECT_DIR / "Saved" / "MikeRigPrototypeImportReport.json"


def _set_property(obj, name, value):
    try:
        obj.set_editor_property(name, value)
        return True
    except Exception:
        return False


def _game_path(object_path):
    return (object_path or "").split(".", 1)[0]


def _object_path(clean_path):
    name = clean_path.rsplit("/", 1)[-1]
    return f"{clean_path}.{name}"


def _load_asset(object_path):
    return unreal.EditorAssetLibrary.load_asset(_game_path(object_path))


def _asset_class(asset):
    return asset.get_class().get_name() if asset else ""


def _delete_if_exists(path):
    clean = _game_path(path)
    if unreal.EditorAssetLibrary.does_asset_exist(clean):
        unreal.EditorAssetLibrary.delete_asset(clean)


def _make_mesh_import_options():
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
        _set_property(data, "import_uniform_scale", 1.0)
    except Exception as exc:
        unreal.log_warning(f"[ImportMikeRigPrototype] Mesh import data setup warning: {exc}")
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
        unreal.log_warning(f"[ImportMikeRigPrototype] Anim import data setup warning: {exc}")
    return options


def _import_task(filename, dest, name, options):
    task = unreal.AssetImportTask()
    task.automated = True
    task.save = True
    task.replace_existing = True
    task.replace_existing_settings = False
    task.filename = str(filename)
    task.destination_path = dest
    task.destination_name = name
    if options is not None:
        task.options = options
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    return list(task.imported_object_paths or [])


def _find_anim_sequence(paths):
    for path in paths:
        asset = unreal.EditorAssetLibrary.load_asset(path)
        if asset and _asset_class(asset) == "AnimSequence":
            return path, asset
    return None, None


def _normalize_anim_path(imported_paths, dest, desired_name):
    desired_clean = f"{dest}/{desired_name}"
    desired_object = _object_path(desired_clean)
    if unreal.EditorAssetLibrary.does_asset_exist(desired_clean):
        asset = unreal.EditorAssetLibrary.load_asset(desired_clean)
        if asset and _asset_class(asset) == "AnimSequence":
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


def _bounds_report(mesh):
    bounds = mesh.get_bounds()
    origin_z = float(bounds.origin.z)
    extent_z = float(bounds.box_extent.z)
    return {
        "origin_z_cm": origin_z,
        "extent_z_cm": extent_z,
        "height_cm": extent_z * 2.0,
        "bottom_z_cm": origin_z - extent_z,
        "top_z_cm": origin_z + extent_z,
    }


def _import_assets(exports):
    fbx = exports["fbx"]
    idle_fbx = exports["idle_fbx"]
    walk_fbx = exports["walk_fbx"]
    for source in (fbx, idle_fbx, walk_fbx):
        if not os.path.isfile(source):
            raise RuntimeError(f"Missing source FBX: {source}")

    if not unreal.EditorAssetLibrary.does_directory_exist(DEST):
        unreal.EditorAssetLibrary.make_directory(DEST)

    for name in (MESH_NAME, IDLE_NAME, f"{IDLE_NAME}_Anim", WALK_NAME, f"{WALK_NAME}_Anim"):
        _delete_if_exists(f"{DEST}/{name}")

    mesh_paths = _import_task(fbx, DEST, MESH_NAME, _make_mesh_import_options())
    mesh_object = _object_path(f"{DEST}/{MESH_NAME}")
    mesh = _load_asset(mesh_object)
    if _asset_class(mesh) != "SkeletalMesh":
        raise RuntimeError(f"SkeletalMesh import failed: {mesh_object}; imported={mesh_paths}")

    skeleton = mesh.get_editor_property("skeleton")
    if not skeleton:
        raise RuntimeError(f"Imported mesh has no skeleton: {mesh_object}")

    idle_paths = _import_task(idle_fbx, DEST, IDLE_NAME, _make_anim_import_options(skeleton))
    idle_object = _normalize_anim_path(idle_paths, DEST, IDLE_NAME)
    walk_paths = _import_task(walk_fbx, DEST, WALK_NAME, _make_anim_import_options(skeleton))
    walk_object = _normalize_anim_path(walk_paths, DEST, WALK_NAME)

    unlit_results = MakeCharacterMaterialsUnlit.convert_character_materials_unlit([DEST])
    material_repair_results = RepairMikeRigPrototypeMaterials.repair()

    unreal.EditorAssetLibrary.save_asset(_game_path(mesh_object))
    unreal.EditorAssetLibrary.save_asset(_game_path(idle_object))
    unreal.EditorAssetLibrary.save_asset(_game_path(walk_object))
    unreal.EditorAssetLibrary.save_directory(DEST, only_if_is_dirty=True, recursive=True)

    return {
        "mesh": mesh_object,
        "idle": idle_object,
        "walk": walk_object,
        "skeleton": skeleton.get_path_name(),
        "mesh_imported_paths": mesh_paths,
        "idle_imported_paths": idle_paths,
        "walk_imported_paths": walk_paths,
        "bounds": _bounds_report(mesh),
        "unlit": unlit_results,
        "material_repair": material_repair_results,
    }


def _replace_character_visuals_row(mesh_object, idle_object, walk_object):
    new_line = ",".join(
        [
            f'"{ROW_ID}"',
            f'"{mesh_object}"',
            f'"{walk_object}"',
            f'"{idle_object}"',
            f'"{walk_object}"',
            '"(X=0,Y=0,Z=0)"',
            '"(Pitch=0,Yaw=-90,Roll=0)"',
            '"(X=1,Y=1,Z=1)"',
            '"true"',
            '"true"',
        ]
    )

    text = CSV_PATH.read_text(encoding="utf-8-sig")
    newline = "\r\n" if "\r\n" in text else "\n"
    lines = text.splitlines()
    previous = None
    changed = False
    for index, line in enumerate(lines):
        if line.startswith(f'"{ROW_ID}",'):
            previous = line
            lines[index] = new_line
            changed = True
            break
    if not changed:
        raise RuntimeError(f"Could not find {ROW_ID} in {CSV_PATH}")

    CSV_PATH.write_text(newline.join(lines) + newline, encoding="utf-8")
    return {"previous": previous, "current": new_line}


def _reload_character_visuals_dt():
    dt = unreal.EditorAssetLibrary.load_asset(DT_PATH)
    if not dt:
        raise RuntimeError(f"Missing DataTable: {DT_PATH}")
    if not unreal.DataTableFunctionLibrary.fill_data_table_from_csv_file(dt, str(CSV_PATH)):
        raise RuntimeError(f"Failed to reload {DT_PATH} from {CSV_PATH}")
    unreal.EditorAssetLibrary.save_asset(DT_PATH)


def _verify_wired_assets(mesh_object, idle_object, walk_object):
    errors = []
    mesh = _load_asset(mesh_object)
    idle = _load_asset(idle_object)
    walk = _load_asset(walk_object)
    if _asset_class(mesh) != "SkeletalMesh":
        errors.append(f"Missing SkeletalMesh: {mesh_object}")
    if _asset_class(idle) != "AnimSequence":
        errors.append(f"Missing idle AnimSequence: {idle_object}")
    if _asset_class(walk) != "AnimSequence":
        errors.append(f"Missing walk AnimSequence: {walk_object}")

    skeleton = mesh.get_editor_property("skeleton") if mesh else None
    for label, anim in (("idle", idle), ("walk", walk)):
        anim_skeleton = anim.get_editor_property("skeleton") if anim else None
        if skeleton and anim_skeleton and anim_skeleton != skeleton:
            errors.append(f"{label} animation skeleton mismatch")

    bounds = _bounds_report(mesh) if mesh else {}
    if bounds:
        if not 195.0 <= bounds["height_cm"] <= 205.0:
            errors.append(f"height outside expected range: {bounds['height_cm']:.2f} cm")
        if not -2.0 <= bounds["bottom_z_cm"] <= 2.0:
            errors.append(f"bottom Z not near ground: {bounds['bottom_z_cm']:.2f} cm")

    dt = unreal.EditorAssetLibrary.load_asset(DT_PATH)
    row_ok = False
    if dt:
        row_names = {str(name) for name in unreal.DataTableFunctionLibrary.get_data_table_row_names(dt)}
        row_ok = ROW_ID in row_names
    if not row_ok:
        errors.append(f"DataTable row missing after reload: {ROW_ID}")

    return {
        "ok": not errors,
        "errors": errors,
        "mesh": mesh_object,
        "idle": idle_object,
        "walk": walk_object,
        "bounds": bounds,
    }


def main():
    unreal.log("[ImportMikeRigPrototype] Starting")
    if not RIG_REPORT.exists():
        raise RuntimeError(f"Missing rig report: {RIG_REPORT}")
    with RIG_REPORT.open("r", encoding="utf-8") as handle:
        rig_report = json.load(handle)
    exports = rig_report.get("exports") or {}
    imported = _import_assets(exports)
    csv_update = _replace_character_visuals_row(imported["mesh"], imported["idle"], imported["walk"])
    _reload_character_visuals_dt()
    verification = _verify_wired_assets(imported["mesh"], imported["idle"], imported["walk"])

    report = {
        "ok": verification["ok"],
        "row_id": ROW_ID,
        "destination": DEST,
        "rig_report": str(RIG_REPORT),
        "imports": imported,
        "csv_update": csv_update,
        "verification": verification,
    }
    OUTPUT_REPORT.parent.mkdir(parents=True, exist_ok=True)
    OUTPUT_REPORT.write_text(json.dumps(report, indent=2), encoding="utf-8")
    unreal.log(f"[ImportMikeRigPrototype] Wrote {OUTPUT_REPORT}")

    unreal.EditorAssetLibrary.save_directory("/Game/Data", only_if_is_dirty=True, recursive=True)
    unreal.EditorAssetLibrary.save_directory(DEST, only_if_is_dirty=True, recursive=True)

    if not verification["ok"]:
        for error in verification["errors"]:
            unreal.log_error(f"[ImportMikeRigPrototype] {error}")
        raise RuntimeError(f"Mike rig prototype import verification failed: {len(verification['errors'])} error(s)")
    unreal.log("[ImportMikeRigPrototype] Import and wiring verified")


if __name__ == "__main__":
    main()
