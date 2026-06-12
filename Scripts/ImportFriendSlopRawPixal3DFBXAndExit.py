"""
Import FriendSlop Easy Pixal3D textured FBX bundles as raw StaticMeshes.

This is the fallback for Unreal's GLB Interchange zero-static-mesh behavior.
It preserves the raw Pixal3D mesh/material images from the GLB-derived textured
FBX bundles and intentionally does not create ToonStyle outlines, Tint textures,
inner-line textures, QuadRetro meshes, or processed GLBs.
"""

from __future__ import annotations

import json
import os
from pathlib import Path

import unreal


LOG_PREFIX = "[ImportFriendSlopRawPixal3DFBX]"
REPO_ROOT = Path(__file__).resolve().parents[1]
RUN_ROOT = (
    REPO_ROOT
    / "Model Generation"
    / "Runs"
    / "Pixal3D"
    / "FriendSlopEasyBatch_20260604_1532"
)
DEFAULT_MANIFEST = RUN_ROOT / "FriendSlopEasyBatch_20260604_1532_manifest.json"
DEFAULT_FBX_ROOT = RUN_ROOT / "RawTexturedFBX"
DEFAULT_REPORT_PATH = (
    REPO_ROOT
    / "Reports"
    / "AgentReviews"
    / "FriendSlopEasyPixal3D"
    / "raw_fbx_unreal_import_report.json"
)
PARENT_MATERIAL = "/Game/Materials/Generated/M_Unlit_DiffuseColorMap"


def load_json(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8-sig"))


def asset_name(asset_id: str) -> str:
    return f"SM_{asset_id}"


def object_path(target_dir: str, asset_id: str) -> str:
    name = asset_name(asset_id)
    return f"{target_dir.rstrip('/')}/{name}.{name}"


def package_path(object_ref: str) -> str:
    return object_ref.split(".", 1)[0]


def texture_source_path(fbx_root: Path, asset_id: str) -> Path:
    return fbx_root / asset_id / "Textures" / f"{asset_id}_00_Image_0.png"


def texture_asset_paths(target_dir: str, asset_id: str) -> tuple[str, str, str]:
    texture_dir = f"{target_dir.rstrip('/')}/Textures"
    texture_name = f"T_{asset_id}_BaseColor"
    return texture_dir, texture_name, f"{texture_dir}/{texture_name}.{texture_name}"


def material_asset_paths(target_dir: str, asset_id: str) -> tuple[str, str, str]:
    material_dir = f"{target_dir.rstrip('/')}/Materials"
    material_name = f"MI_{asset_name(asset_id)}"
    return material_dir, material_name, f"{material_dir}/{material_name}.{material_name}"


def ensure_dir(path: str) -> None:
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def scan_game_path(path: str) -> None:
    try:
        unreal.AssetRegistryHelpers.get_asset_registry().scan_paths_synchronous([path], True)
    except Exception:
        pass


def import_texture(source: Path, target_dir: str, asset_id: str) -> tuple[str, unreal.Texture2D | None, str]:
    texture_dir, texture_name, texture_ref = texture_asset_paths(target_dir, asset_id)
    ensure_dir(texture_dir)
    if not source.exists() or source.stat().st_size <= 0:
        return texture_ref, None, f"missing base color texture: {source}"

    task = unreal.AssetImportTask()
    task.automated = True
    task.save = True
    task.replace_existing = True
    task.replace_existing_settings = True
    task.filename = str(source)
    task.destination_path = texture_dir
    task.destination_name = texture_name

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    scan_game_path(texture_dir)

    imported_paths = list(task.imported_object_paths or [])
    texture_package = str(imported_paths[0]).split(".", 1)[0] if imported_paths else package_path(texture_ref)
    texture = unreal.EditorAssetLibrary.load_asset(texture_package)
    if not texture or not isinstance(texture, unreal.Texture2D):
        return texture_ref, None, f"texture import did not produce Texture2D: {texture_package}"

    try:
        texture.set_editor_property("srgb", True)
    except Exception:
        pass
    try:
        texture.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_World)
    except Exception:
        pass
    try:
        unreal.EditorAssetLibrary.save_loaded_asset(texture)
    except Exception:
        pass
    return texture.get_path_name(), texture, ""


def ensure_material(target_dir: str, asset_id: str, texture: unreal.Texture2D) -> tuple[str, object | None, str]:
    material_dir, material_name, material_ref = material_asset_paths(target_dir, asset_id)
    ensure_dir(material_dir)
    material_package = package_path(material_ref)
    material = unreal.EditorAssetLibrary.load_asset(material_package)
    if not material:
        material = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
            material_name,
            material_dir,
            unreal.MaterialInstanceConstant,
            unreal.MaterialInstanceConstantFactoryNew(),
        )
    if not material:
        return material_ref, None, f"failed to create material instance: {material_ref}"

    parent = unreal.EditorAssetLibrary.load_asset(PARENT_MATERIAL)
    if not parent:
        return material.get_path_name(), material, f"missing parent material: {PARENT_MATERIAL}"
    try:
        material.set_editor_property("parent", parent)
    except Exception as exc:
        return material.get_path_name(), material, f"failed to set material parent: {exc}"

    for param_name in ("BaseColorTexture", "DiffuseColorMap"):
        try:
            unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(
                material, param_name, texture
            )
        except Exception:
            pass
    try:
        unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(
            material, "Tint", unreal.LinearColor(1.0, 1.0, 1.0, 1.0)
        )
    except Exception:
        pass
    try:
        unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(
            material, "Brightness", 1.0
        )
    except Exception:
        pass
    try:
        unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(
            material, "Opacity", 1.0
        )
    except Exception:
        pass
    try:
        unreal.EditorAssetLibrary.save_loaded_asset(material)
    except Exception:
        pass
    return material.get_path_name(), material, ""


def texture_param_path(material: object, name: str) -> str:
    try:
        value = unreal.MaterialEditingLibrary.get_material_instance_texture_parameter_value(material, name)
    except Exception:
        return ""
    if isinstance(value, (tuple, list)):
        value = value[-1] if value else None
    return value.get_path_name() if value else ""


def assign_material(mesh: unreal.StaticMesh, material: object) -> int:
    slot_count = 1
    try:
        slot_count = max(1, len(mesh.get_editor_property("static_materials") or []))
    except Exception:
        pass
    for index in range(slot_count):
        mesh.set_material(index, material)
    try:
        mesh.post_edit_change()
    except Exception:
        pass
    try:
        unreal.EditorAssetLibrary.save_loaded_asset(mesh)
    except Exception:
        pass
    return slot_count


def make_fbx_import_options() -> unreal.FbxImportUI:
    options = unreal.FbxImportUI()
    options.set_editor_property("automated_import_should_detect_type", False)
    options.set_editor_property("mesh_type_to_import", unreal.FBXImportType.FBXIT_STATIC_MESH)
    options.set_editor_property("import_mesh", True)
    options.set_editor_property("import_as_skeletal", False)
    options.set_editor_property("import_materials", False)
    options.set_editor_property("import_textures", False)
    options.set_editor_property("import_animations", False)

    static_mesh_data = options.get_editor_property("static_mesh_import_data")
    if static_mesh_data:
        for prop, value in (
            ("combine_meshes", True),
            ("convert_scene", True),
            ("convert_scene_unit", True),
            ("import_uniform_scale", 1.0),
            ("force_front_x_axis", False),
            ("generate_lightmap_u_vs", False),
            ("auto_generate_collision", False),
            ("remove_degenerates", False),
            ("build_nanite", False),
        ):
            try:
                static_mesh_data.set_editor_property(prop, value)
            except Exception:
                pass
    return options


def mesh_bounds(mesh: unreal.StaticMesh) -> dict[str, list[float]]:
    bounds = mesh.get_bounds()
    return {
        "origin_cm": [float(bounds.origin.x), float(bounds.origin.y), float(bounds.origin.z)],
        "extent_cm": [
            float(bounds.box_extent.x),
            float(bounds.box_extent.y),
            float(bounds.box_extent.z),
        ],
        "size_cm": [
            float(bounds.box_extent.x) * 2.0,
            float(bounds.box_extent.y) * 2.0,
            float(bounds.box_extent.z) * 2.0,
        ],
    }


def material_slot_report(mesh: unreal.StaticMesh) -> list[dict[str, str]]:
    rows: list[dict[str, str]] = []
    try:
        slot_count = int(mesh.get_num_sections(0))
    except Exception:
        slot_count = 0
    try:
        static_materials = list(mesh.get_editor_property("static_materials") or [])
    except Exception:
        static_materials = []

    max_count = max(slot_count, len(static_materials))
    for index in range(max_count):
        material_path = ""
        slot_name = ""
        if index < len(static_materials):
            entry = static_materials[index]
            try:
                slot_name = str(entry.get_editor_property("material_slot_name"))
            except Exception:
                pass
            try:
                material = entry.get_editor_property("material_interface")
                material_path = material.get_path_name() if material else ""
            except Exception:
                pass
        rows.append({"index": str(index), "slot": slot_name, "material": material_path})
    return rows


def import_one(asset: dict, fbx_root: Path) -> dict[str, object]:
    asset_id = str(asset.get("asset_id", "")).strip()
    target_dir = str(asset.get("target_dir", "")).strip().rstrip("/")
    name = asset_name(asset_id)
    fbx = fbx_root / asset_id / f"{asset_id}_Textured.fbx"
    texture_source = texture_source_path(fbx_root, asset_id)
    expected = object_path(target_dir, asset_id)
    ensure_dir(target_dir)

    if not fbx.exists() or fbx.stat().st_size <= 0:
        return {
            "asset_id": asset_id,
            "fbx": str(fbx),
            "expected": expected,
            "ok": False,
            "error": "missing textured FBX",
        }

    texture_ref, texture, texture_error = import_texture(texture_source, target_dir, asset_id)
    if not texture:
        return {
            "asset_id": asset_id,
            "fbx": str(fbx),
            "texture_source": str(texture_source),
            "texture": texture_ref,
            "expected": expected,
            "ok": False,
            "error": texture_error,
        }

    material_ref, material, material_error = ensure_material(target_dir, asset_id, texture)
    if not material:
        return {
            "asset_id": asset_id,
            "fbx": str(fbx),
            "texture_source": str(texture_source),
            "texture": texture_ref,
            "material": material_ref,
            "expected": expected,
            "ok": False,
            "error": material_error,
        }

    existing_package = package_path(expected)
    if unreal.EditorAssetLibrary.does_asset_exist(existing_package):
        try:
            unreal.EditorAssetLibrary.delete_asset(existing_package)
        except Exception as exc:
            unreal.log_warning(f"{LOG_PREFIX} Could not delete existing {existing_package}: {exc}")

    task = unreal.AssetImportTask()
    task.automated = True
    task.save = True
    task.replace_existing = True
    task.replace_existing_settings = False
    task.filename = str(fbx)
    task.destination_path = target_dir
    task.destination_name = name
    task.options = make_fbx_import_options()

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    imported_paths = list(task.imported_object_paths or [])
    loaded = unreal.EditorAssetLibrary.load_asset(expected)
    ok = bool(loaded and isinstance(loaded, unreal.StaticMesh))
    row: dict[str, object] = {
        "asset_id": asset_id,
        "fbx": str(fbx),
        "texture_source": str(texture_source),
        "texture": texture_ref,
        "material": material_ref,
        "expected": expected,
        "imported_paths": imported_paths,
        "ok": ok,
        "class": loaded.get_class().get_name() if loaded else "",
    }
    if ok:
        row["assigned_material_slots"] = assign_material(loaded, material)
        row["bounds"] = mesh_bounds(loaded)
        row["materials"] = material_slot_report(loaded)
        row["material_base_color_texture"] = texture_param_path(material, "BaseColorTexture")
        row["material_diffuse_texture"] = texture_param_path(material, "DiffuseColorMap")
        if material_error:
            row["material_warning"] = material_error
        try:
            unreal.EditorAssetLibrary.save_loaded_asset(loaded)
        except Exception:
            pass
    else:
        row["error"] = "imported asset did not load as StaticMesh"
    return row


def main() -> None:
    unreal.log(f"{LOG_PREFIX} START")
    manifest_path = Path(os.environ.get("T66_PIXAL3D_MANIFEST", str(DEFAULT_MANIFEST)))
    fbx_root = Path(os.environ.get("T66_PIXAL3D_RAW_FBX_ROOT", str(DEFAULT_FBX_ROOT)))
    report_path = Path(os.environ.get("T66_PIXAL3D_RAW_IMPORT_REPORT", str(DEFAULT_REPORT_PATH)))
    manifest = load_json(manifest_path)
    rows = [import_one(asset, fbx_root) for asset in manifest.get("assets", [])]
    errors = [f"{row['asset_id']}: {row.get('error', 'failed')}" for row in rows if not row.get("ok")]
    report = {
        "ok": not errors,
        "manifest": str(manifest_path),
        "fbx_root": str(fbx_root),
        "asset_count": len(rows),
        "rows": rows,
        "errors": errors,
        "notes": [
            "FBX fallback used because Unreal GLB Interchange returned zero StaticMesh assets.",
            "FBX bundles are derived from raw Pixal3D GLBs and preserve raw material image payloads.",
            "No ToonStyle, QuadRetro, outline, Tint, inner-line, or processed GLB stage was run.",
        ],
    }
    report_path.parent.mkdir(parents=True, exist_ok=True)
    report_path.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    unreal.log(f"{LOG_PREFIX} Wrote {report_path}")
    if errors:
        for error in errors:
            unreal.log_error(f"{LOG_PREFIX} {error}")
    else:
        unreal.log(f"{LOG_PREFIX} Verified {len(rows)} raw StaticMesh import(s)")

    try:
        unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")
    except Exception as exc:
        unreal.log_warning(f"{LOG_PREFIX} Failed to request QUIT_EDITOR: {exc}")


main()
