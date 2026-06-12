import csv
import json
import os
import sys

import unreal


SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
if SCRIPT_DIR not in sys.path:
    sys.path.append(SCRIPT_DIR)

import ImportStaticMeshes


RUN_ID = "AutoAttackProjectileBatch01"
DEST_DIR = "/Game/Weapons/Projectiles"
SOURCE_IMPORT_DIR = os.path.join("Weapons", "Projectiles", "UnrealReady")
SOURCE_TEXTURE_DIR = os.path.join("Weapons", "Projectiles", "Textures")
HEROES_CSV_RELATIVE = os.path.join("Content", "Data", "Heroes.csv")
HERO_DATA_TABLE = "/Game/Data/DT_Heroes.DT_Heroes"
PARENT_MATERIAL = "/Game/Materials/Generated/M_Unlit_DiffuseColorMap"

PROJECTILES = (
    ("Hero_1", "FoundingChad_Rapier"),
    ("Hero_2", "ChineseChad_Guandao"),
    ("Hero_3", "BoxerChad_Glove"),
    ("Hero_4", "BillyChad_Bullet"),
    ("Hero_5", "RoyalChad_Sword"),
    ("Hero_6", "RoboChad_GearBlade"),
    ("Hero_7", "RabbitChad_Carrot"),
    ("Hero_8", "CSChad_TacticalKnife"),
    ("Hero_9", "GoblinoChad_Cleaver"),
    ("Hero_10", "MonotoneChad_InkShard"),
    ("Hero_11", "BaldChad_Hatchet"),
    ("Hero_12", "RoachChad_RustyCrown"),
)


def _project_dir():
    return unreal.Paths.project_dir().replace("\\", "/").rstrip("/")


def _run_root(project_dir):
    return os.path.join(
        project_dir,
        "Model Generation",
        "Runs",
        "Weapons",
        RUN_ID,
    ).replace("\\", "/")


def _source_glb_path(run_root, projectile_id):
    return os.path.join(
        run_root,
        "Raw",
        "Trellis",
        f"{projectile_id}_S1337_D80000_Trellis2.glb",
    ).replace("\\", "/")


def _source_import_root(project_dir):
    return os.path.join(project_dir, "SourceAssets", "Import").replace("\\", "/")


def _source_fbx_path(project_dir, projectile_id):
    name = _asset_name(projectile_id)
    return os.path.join(
        _source_import_root(project_dir),
        SOURCE_IMPORT_DIR,
        f"{name}_UnrealReady.fbx",
    ).replace("\\", "/")


def _source_texture_path(project_dir, projectile_id):
    name = _asset_name(projectile_id)
    return os.path.join(
        _source_import_root(project_dir),
        SOURCE_TEXTURE_DIR,
        f"{name}_BaseColor_00.png",
    ).replace("\\", "/")


def _asset_name(projectile_id):
    return f"SM_{projectile_id}"


def _mesh_object_path(projectile_id):
    name = _asset_name(projectile_id)
    return f"{DEST_DIR}/{name}.{name}"


def _ensure_inputs(run_root):
    missing = []
    project_dir = _project_dir()
    for _hero_id, projectile_id in PROJECTILES:
        for source in (
            _source_glb_path(run_root, projectile_id),
            _source_fbx_path(project_dir, projectile_id),
            _source_texture_path(project_dir, projectile_id),
        ):
            if not os.path.isfile(source):
                missing.append(source)
    if missing:
        raise RuntimeError("Missing projectile import inputs: " + ", ".join(missing))


def _ensure_game_dir(game_path):
    if not unreal.EditorAssetLibrary.does_directory_exist(game_path):
        unreal.EditorAssetLibrary.make_directory(game_path)


def _scan_game_path(game_path):
    try:
        unreal.AssetRegistryHelpers.get_asset_registry().scan_paths_synchronous([game_path], True)
    except Exception:
        pass


def _import_texture(source_path, asset_name):
    texture_dir = f"{DEST_DIR}/Textures"
    texture_name = f"{asset_name}_BaseColor_00"
    _ensure_game_dir(texture_dir)

    task = unreal.AssetImportTask()
    task.automated = True
    task.save = True
    task.replace_existing = True
    task.replace_existing_settings = True
    task.filename = source_path
    task.destination_path = texture_dir
    task.destination_name = texture_name

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    _scan_game_path(texture_dir)

    imported_paths = list(task.imported_object_paths or [])
    texture_path = str(imported_paths[0]).split(".")[0] if imported_paths else f"{texture_dir}/{texture_name}"
    texture = unreal.EditorAssetLibrary.load_asset(texture_path)
    if not texture or not isinstance(texture, unreal.Texture2D):
        raise RuntimeError(f"Texture import failed: {texture_path}")

    try:
        texture.set_editor_property("srgb", True)
    except Exception:
        pass
    try:
        texture.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_World)
    except Exception:
        pass
    unreal.EditorAssetLibrary.save_asset(texture_path)
    return texture_path, texture


def _ensure_material(asset_name, texture):
    material_dir = f"{DEST_DIR}/Materials"
    material_name = f"MI_{asset_name}"
    material_path = f"{material_dir}/{material_name}"
    _ensure_game_dir(material_dir)

    material = None
    if unreal.EditorAssetLibrary.does_asset_exist(material_path):
        material = unreal.EditorAssetLibrary.load_asset(material_path)
    if not material:
        material = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
            material_name,
            material_dir,
            unreal.MaterialInstanceConstant,
            unreal.MaterialInstanceConstantFactoryNew(),
        )
    if not material:
        raise RuntimeError(f"Material create/load failed: {material_path}")

    parent = unreal.EditorAssetLibrary.load_asset(PARENT_MATERIAL)
    if parent:
        try:
            material.set_editor_property("parent", parent)
        except Exception as exc:
            unreal.log_warning(f"[WeaponProjectiles] Could not set parent for {material_name}: {exc}")

    if texture:
        for param_name in ("BaseColorTexture", "DiffuseColorMap"):
            try:
                unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(
                    material, param_name, texture)
            except Exception:
                pass
    try:
        unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(
            material, "Tint", unreal.LinearColor(1.0, 1.0, 1.0, 1.0))
    except Exception:
        pass
    try:
        unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(
            material, "Brightness", 1.0)
    except Exception:
        pass

    unreal.EditorAssetLibrary.save_asset(material_path)
    return material_path, material


def _assign_mesh_material(mesh_path, material):
    mesh = unreal.EditorAssetLibrary.load_asset(mesh_path)
    if not mesh or not isinstance(mesh, unreal.StaticMesh):
        raise RuntimeError(f"StaticMesh not found for material bind: {mesh_path}")

    slot_count = 1
    try:
        slot_count = max(1, len(mesh.get_editor_property("static_materials") or []))
    except Exception:
        pass

    for slot_index in range(slot_count):
        mesh.set_material(slot_index, material)
    try:
        mesh.post_edit_change()
    except Exception:
        pass
    unreal.EditorAssetLibrary.save_asset(mesh_path)
    return slot_count


def _import_projectile_meshes(project_dir, run_root):
    results = []
    ImportStaticMeshes._ensure_game_dir(DEST_DIR)

    for hero_id, projectile_id in PROJECTILES:
        raw_source = _source_glb_path(run_root, projectile_id)
        source = _source_fbx_path(project_dir, projectile_id)
        texture_source = _source_texture_path(project_dir, projectile_id)
        dest_name = _asset_name(projectile_id)

        unreal.log(f"[WeaponProjectiles] Import {projectile_id} -> {DEST_DIR}/{dest_name}")
        ImportStaticMeshes._cleanup_existing_import_artifacts(DEST_DIR, dest_name, {})
        imported_paths = ImportStaticMeshes.import_glb(source, DEST_DIR, dest_name)
        _scan_game_path(DEST_DIR)
        final_path = ImportStaticMeshes._flatten_interchange_asset(DEST_DIR, dest_name)
        if not final_path:
            raise RuntimeError(f"Could not locate imported StaticMesh for {projectile_id}")

        ImportStaticMeshes._apply_static_mesh_build_settings(final_path, {})

        texture_path, texture = _import_texture(texture_source, dest_name)
        material_path, material = _ensure_material(dest_name, texture)
        material_slots = _assign_mesh_material(final_path, material)

        asset = unreal.EditorAssetLibrary.load_asset(final_path)
        if not asset or not isinstance(asset, unreal.StaticMesh):
            raise RuntimeError(f"Imported asset is not a StaticMesh: {final_path}")

        unreal.EditorAssetLibrary.save_asset(final_path)
        results.append(
            {
                "hero_id": hero_id,
                "projectile_id": projectile_id,
                "raw_source": raw_source,
                "source": source,
                "texture_source": texture_source,
                "asset_path": final_path,
                "object_path": _mesh_object_path(projectile_id),
                "texture_path": texture_path,
                "material_path": material_path,
                "material_slots": material_slots,
                "imported_paths": list(imported_paths or []),
            }
        )

    return results


def _update_heroes_csv(project_dir):
    csv_path = os.path.join(project_dir, HEROES_CSV_RELATIVE).replace("\\", "/")
    if not os.path.isfile(csv_path):
        raise RuntimeError(f"Heroes.csv not found: {csv_path}")

    hero_to_mesh = {hero_id: _mesh_object_path(projectile_id) for hero_id, projectile_id in PROJECTILES}

    with open(csv_path, "r", encoding="utf-8-sig", newline="") as handle:
        reader = csv.DictReader(handle)
        fieldnames = list(reader.fieldnames or [])
        rows = list(reader)

    if "AutoAttackProjectileMesh" not in fieldnames:
        if "PlaceholderColor" in fieldnames:
            insert_at = fieldnames.index("PlaceholderColor") + 1
        else:
            insert_at = min(5, len(fieldnames))
        fieldnames.insert(insert_at, "AutoAttackProjectileMesh")

    portrait_insert_after = "PortraitStacyFull"
    for required_field in ("PortraitInvincible", "PortraitStacyInvincible"):
        if required_field in fieldnames:
            continue
        if portrait_insert_after in fieldnames:
            insert_at = fieldnames.index(portrait_insert_after) + 1
        else:
            insert_at = min(13, len(fieldnames))
        fieldnames.insert(insert_at, required_field)
        portrait_insert_after = required_field

    updated = 0
    for row in rows:
        hero_id = row.get("HeroID") or row.get("---")
        if hero_id not in hero_to_mesh:
            continue
        if row.get("AutoAttackProjectileMesh") != hero_to_mesh[hero_id]:
            row["AutoAttackProjectileMesh"] = hero_to_mesh[hero_id]
            updated += 1

    with open(csv_path, "w", encoding="utf-8", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=fieldnames, quoting=csv.QUOTE_ALL, lineterminator="\n")
        writer.writeheader()
        writer.writerows(rows)

    unreal.log(f"[WeaponProjectiles] Updated {updated} Heroes.csv projectile mesh references")
    return csv_path, updated


def _reload_hero_data_table(csv_path):
    data_table = unreal.EditorAssetLibrary.load_asset(HERO_DATA_TABLE)
    if not data_table:
        raise RuntimeError(f"Failed to load DataTable asset: {HERO_DATA_TABLE}")

    if not unreal.DataTableFunctionLibrary.fill_data_table_from_csv_file(data_table, csv_path):
        raise RuntimeError(f"Failed to import hero CSV into {HERO_DATA_TABLE}")

    if not unreal.EditorAssetLibrary.save_loaded_asset(data_table):
        raise RuntimeError(f"Failed to save DataTable asset: {HERO_DATA_TABLE}")

    unreal.log(f"[WeaponProjectiles] Reloaded {HERO_DATA_TABLE} from {csv_path}")


def _write_report(run_root, import_results, csv_path, csv_updates):
    report_path = os.path.join(
        run_root,
        "Notes",
        "unreal_import_report_20260507.json",
    ).replace("\\", "/")
    os.makedirs(os.path.dirname(report_path), exist_ok=True)

    report = {
        "run_id": RUN_ID,
        "destination": DEST_DIR,
        "heroes_csv": csv_path,
        "csv_updates": csv_updates,
        "data_table": HERO_DATA_TABLE,
        "imports": import_results,
    }

    with open(report_path, "w", encoding="utf-8") as handle:
        json.dump(report, handle, indent=2)

    unreal.log(f"[WeaponProjectiles] Wrote report: {report_path}")
    return report_path


def main():
    project_dir = _project_dir()
    run_root = _run_root(project_dir)

    unreal.log("=" * 60)
    unreal.log("[WeaponProjectiles] START")
    unreal.log(f"[WeaponProjectiles] Run root: {run_root}")
    unreal.log("=" * 60)

    _ensure_inputs(run_root)
    import_results = _import_projectile_meshes(project_dir, run_root)
    csv_path, csv_updates = _update_heroes_csv(project_dir)
    _reload_hero_data_table(csv_path)
    report_path = _write_report(run_root, import_results, csv_path, csv_updates)

    unreal.log("=" * 60)
    unreal.log(f"[WeaponProjectiles] DONE imports={len(import_results)} csv_updates={csv_updates}")
    unreal.log(f"[WeaponProjectiles] Report: {report_path}")
    unreal.log("=" * 60)


if __name__ == "__main__":
    main()
