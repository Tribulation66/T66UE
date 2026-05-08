"""
Import WorldNpcInteractablesRetroBatch01 Quad Retro Unreal-ready FBXs.

The direct GLB/Interchange commandlet path is not used for this batch because
it can assert through ContentBrowser/Slate in headless runs. The Blender export
step writes FBXs plus pixelated textures under SourceAssets/Import, then this
script imports the FBXs and binds each mesh to the project unlit material flow.
"""

import json
import os
import sys
from datetime import datetime, timezone
from pathlib import PurePosixPath

import unreal


SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
if SCRIPT_DIR not in sys.path:
    sys.path.insert(0, SCRIPT_DIR)

import ImportStaticMeshes


PROJECT_DIR = unreal.SystemLibrary.get_project_directory().replace("\\", "/").rstrip("/")
PROJECT_ROOT = PROJECT_DIR
RUN_ROOT = os.path.join(
    PROJECT_ROOT,
    "Model Generation",
    "Runs",
    "Interactables",
    "WorldNpcInteractablesRetroBatch01",
).replace("\\", "/")
STAGE02_PATH = os.path.join(RUN_ROOT, "Reports", "Stage02_QuadRetroManifest.json").replace("\\", "/")
UNREAL_READY_PATH = os.path.join(
    RUN_ROOT,
    "Reports",
    "WorldNpcInteractablesRetroBatch01_UnrealReadyFbxManifest.json",
).replace("\\", "/")
IMPORT_REPORT_PATH = os.path.join(RUN_ROOT, "Reports", "UnrealImportManifest.json").replace("\\", "/")
IMPORT_ROOT = os.path.join(PROJECT_DIR, "SourceAssets", "Import").replace("\\", "/")
PARENT_MATERIAL = "/Game/Materials/M_Environment_Unlit"


def _load_json(path):
    with open(path, "r", encoding="utf-8") as handle:
        return json.load(handle)


def _write_json(path, payload):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", encoding="utf-8") as handle:
        json.dump(payload, handle, indent=2)
        handle.write("\n")


def _abs_project_path(relative_or_abs):
    if not relative_or_abs:
        return ""
    if os.path.isabs(relative_or_abs):
        return relative_or_abs.replace("\\", "/")
    return os.path.join(PROJECT_ROOT, relative_or_abs).replace("\\", "/")


def _object_path(asset_path):
    leaf = asset_path.rsplit("/", 1)[-1]
    return f"{asset_path}.{leaf}"


def _material_overrides(row_id):
    # Loot-bag Trellis bakes are intentionally dark; brighten/tint imported
    # material instances so rarity still reads at gameplay camera distance.
    if row_id == "LootBag_Black":
        return {"brightness": 1.10, "tint": (0.35, 0.35, 0.38, 1.0)}
    if row_id == "LootBag_Red":
        return {"brightness": 1.65, "tint": (1.0, 0.25, 0.20, 1.0)}
    if row_id == "LootBag_Yellow":
        return {"brightness": 1.35, "tint": (1.0, 0.84, 0.22, 1.0)}
    if row_id == "LootBag_White":
        return {"brightness": 1.85, "tint": (1.0, 1.0, 0.92, 1.0)}
    return {"brightness": 1.0, "tint": (1.0, 1.0, 1.0, 1.0)}


def _prepare_imports(unreal_ready):
    entries = []
    report_entries = []

    for entry in unreal_ready.get("entries", []):
        row_id = entry.get("row_id")
        category = entry.get("category")
        if entry.get("status") != "ready":
            report_entries.append({
                "row_id": row_id,
                "category": category,
                "status": "skipped_not_ready",
                "unreal_ready_status": entry.get("status"),
            })
            continue

        source = os.path.join(IMPORT_ROOT, entry.get("import_fbx_rel") or "").replace("\\", "/")
        if not os.path.isfile(source):
            report_entries.append({
                "row_id": row_id,
                "category": category,
                "status": "missing_source_fbx",
                "source_fbx": source,
            })
            continue

        texture_source = os.path.join(IMPORT_ROOT, entry.get("import_texture_rel") or "").replace("\\", "/")
        if not os.path.isfile(texture_source):
            report_entries.append({
                "row_id": row_id,
                "category": category,
                "status": "missing_source_texture",
                "source_fbx": source,
                "source_texture": texture_source,
            })
            continue

        dest = entry["destination_path"]
        name = entry["asset_name"]

        entries.append({
            "source": entry["import_fbx_rel"],
            "dest": dest,
            "name": name,
            "cleanup": {"mode": "asset_subtree"},
        })

        asset_path = f"{dest}/{name}"
        texture_name = PurePosixPath(entry["import_texture_rel"]).stem
        report_entries.append({
            "row_id": row_id,
            "category": category,
            "status": "prepared",
            "source_fbx": source,
            "source_texture": texture_source,
            "import_source": entry["import_fbx_rel"],
            "destination_path": dest,
            "asset_name": name,
            "texture_name": texture_name,
            "target_height_m": entry.get("target_height_m"),
            "triangle_count": entry.get("triangles"),
            "unreal_asset_path": _object_path(asset_path),
        })

    return entries, report_entries


def _ensure_game_dir(game_path):
    if not unreal.EditorAssetLibrary.does_directory_exist(game_path):
        unreal.EditorAssetLibrary.make_directory(game_path)


def _path_name(asset):
    return asset.get_path_name() if asset else ""


def _import_texture(source_path, dest_dir, texture_name):
    texture_dir = f"{dest_dir}/Textures"
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

    imported_paths = list(task.imported_object_paths or [])
    texture_path = str(imported_paths[0]).split(".")[0] if imported_paths else f"{texture_dir}/{texture_name}"
    try:
        unreal.AssetRegistryHelpers.get_asset_registry().scan_paths_synchronous([texture_dir], True)
    except Exception:
        pass

    texture = unreal.EditorAssetLibrary.load_asset(texture_path)
    if not texture or not isinstance(texture, unreal.Texture2D):
        unreal.log_warning(f"[WorldNpcRetroBatch01] Texture import failed: {texture_path}")
        return None, texture_path

    for prop_name, prop_value in (
        ("srgb", True),
        ("lod_group", getattr(unreal.TextureGroup, "TEXTUREGROUP_World", None)),
        ("filter", getattr(getattr(unreal, "TextureFilter", object), "TF_NEAREST", None)),
        ("mip_gen_settings", getattr(getattr(unreal, "TextureMipGenSettings", object), "TMGS_NO_MIPMAPS", None)),
    ):
        if prop_value is None:
            continue
        try:
            texture.set_editor_property(prop_name, prop_value)
        except Exception:
            pass

    try:
        texture.post_edit_change()
    except Exception:
        pass
    unreal.EditorAssetLibrary.save_asset(texture_path)
    return texture, texture_path


def _ensure_material(asset_name, dest_dir, texture, row_id):
    material_dir = f"{dest_dir}/Materials"
    material_name = f"M_{asset_name}"
    material_path = f"{material_dir}/{material_name}"
    _ensure_game_dir(material_dir)

    material = unreal.EditorAssetLibrary.load_asset(material_path)
    if not material:
        material = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
            material_name,
            material_dir,
            unreal.MaterialInstanceConstant,
            unreal.MaterialInstanceConstantFactoryNew(),
        )
    if not material:
        unreal.log_warning(f"[WorldNpcRetroBatch01] Material create/load failed: {material_path}")
        return None, material_path

    parent = unreal.EditorAssetLibrary.load_asset(PARENT_MATERIAL)
    if parent:
        try:
            material.set_editor_property("parent", parent)
        except Exception as exc:
            unreal.log_warning(f"[WorldNpcRetroBatch01] Could not set parent for {material_name}: {exc}")

    if texture:
        for param_name in ("DiffuseColorMap", "BaseColorTexture"):
            try:
                unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(
                    material, param_name, texture)
            except Exception:
                pass

    overrides = _material_overrides(row_id)
    tint_value = overrides.get("tint") or (1.0, 1.0, 1.0, 1.0)
    try:
        unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(
            material,
            "Tint",
            unreal.LinearColor(
                float(tint_value[0]),
                float(tint_value[1]),
                float(tint_value[2]),
                float(tint_value[3]),
            ),
        )
    except Exception:
        pass
    try:
        unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(
            material, "Brightness", float(overrides.get("brightness", 1.0)))
    except Exception:
        pass

    unreal.EditorAssetLibrary.save_asset(material_path)
    return material, material_path


def _assign_mesh_material(asset_name, dest_dir, material):
    if not material:
        return False

    mesh_path = f"{dest_dir}/{asset_name}"
    mesh = unreal.EditorAssetLibrary.load_asset(mesh_path)
    if not mesh or not isinstance(mesh, unreal.StaticMesh):
        unreal.log_warning(f"[WorldNpcRetroBatch01] StaticMesh not found: {mesh_path}")
        return False

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
    return True


def _apply_generated_textures(report_entries):
    applied = 0
    for entry in report_entries:
        if entry.get("status") != "prepared":
            continue

        texture, texture_asset_path = _import_texture(
            entry["source_texture"],
            entry["destination_path"],
            entry["texture_name"],
        )
        material, material_asset_path = _ensure_material(
            entry["asset_name"],
            entry["destination_path"],
            texture,
            entry["row_id"],
        )
        assigned = _assign_mesh_material(entry["asset_name"], entry["destination_path"], material)
        entry["texture_asset_path"] = _object_path(texture_asset_path) if texture_asset_path else ""
        entry["material_asset_path"] = _object_path(material_asset_path) if material_asset_path else ""
        entry["material_assigned"] = bool(assigned)
        if assigned:
            applied += 1

    unreal.log(f"[WorldNpcRetroBatch01] Applied generated materials: {applied}/{len(report_entries)}")


def _validate_report_entries(report_entries):
    for entry in report_entries:
        if entry.get("status") != "prepared":
            continue
        asset_object_path = entry["unreal_asset_path"]
        asset_path = asset_object_path.split(".", 1)[0]
        asset = unreal.EditorAssetLibrary.load_asset(asset_path)
        entry["asset_exists_after_import"] = bool(asset and isinstance(asset, unreal.StaticMesh))
        material = unreal.EditorAssetLibrary.load_asset((entry.get("material_asset_path") or "").split(".", 1)[0])
        texture = unreal.EditorAssetLibrary.load_asset((entry.get("texture_asset_path") or "").split(".", 1)[0])
        entry["material_exists_after_import"] = bool(material and isinstance(material, unreal.MaterialInstanceConstant))
        entry["texture_exists_after_import"] = bool(texture and isinstance(texture, unreal.Texture2D))
        entry["status"] = (
            "imported"
            if entry["asset_exists_after_import"]
            and entry["material_exists_after_import"]
            and entry["texture_exists_after_import"]
            and entry.get("material_assigned")
            else "missing_after_import"
        )
    return report_entries


def main():
    unreal.log("=== ImportWorldNpcInteractablesRetroBatch01AndExit ===")
    if not os.path.isfile(STAGE02_PATH):
        raise RuntimeError(f"Missing Stage02 manifest: {STAGE02_PATH}")
    if not os.path.isfile(UNREAL_READY_PATH):
        raise RuntimeError(f"Missing Unreal-ready manifest: {UNREAL_READY_PATH}")

    unreal_ready = _load_json(UNREAL_READY_PATH)
    imports, report_entries = _prepare_imports(unreal_ready)
    ImportStaticMeshes.IMPORTS = imports
    unreal.log(f"[WorldNpcRetroBatch01] Prepared {len(imports)} FBX imports")

    ImportStaticMeshes.main()
    _apply_generated_textures(report_entries)

    try:
        unreal.AssetRegistryHelpers.get_asset_registry().scan_paths_synchronous(["/Game/World", "/Game/Characters/NPCs"], True)
    except Exception:
        pass

    report_entries = _validate_report_entries(report_entries)
    imported = sum(1 for entry in report_entries if entry.get("status") == "imported")
    failed = [entry for entry in report_entries if entry.get("status") != "imported"]

    report = {
        "stage": "unreal_import",
        "output_root": RUN_ROOT,
        "summary": {
            "total_entries": len(report_entries),
            "prepared_imports": len(imports),
            "imported": imported,
            "failed_or_skipped": len(failed),
            "failed_or_skipped_rows": [entry.get("row_id") for entry in failed],
        },
        "entries": report_entries,
        "updated_utc": datetime.now(timezone.utc).isoformat(),
    }
    _write_json(IMPORT_REPORT_PATH, report)

    if failed:
        raise RuntimeError(f"Import validation failed for rows: {report['summary']['failed_or_skipped_rows']}")

    unreal.log("[WorldNpcRetroBatch01] Import OK")
    try:
        unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")
    except Exception as exc:
        unreal.log_warning(f"Failed to request QUIT_EDITOR: {exc}")


main()
