"""Import a ToonStyle Pixal3D working set into Unreal.

Inputs are supplied either as command-line arguments or environment variables:
  --working-dir / T66_PIXAL3D_WORKING_DIR
  --asset-name / T66_PIXAL3D_ASSET_NAME
  --target-dir / T66_PIXAL3D_TARGET_DIR
"""

from __future__ import annotations

import argparse
import json
import os
import sys
from pathlib import Path

import unreal


CHARACTER_PARENT_MATERIAL = "/Game/ToonStyle/Materials/M_Toon_Character"
OUTLINE_PARENT_MATERIAL = "/Game/ToonStyle/Materials/M_Toon_Character_Outline"
EXPECTED_HEIGHT = 180.0
HEIGHT_TOLERANCE = 0.10


def parse_args() -> argparse.Namespace:
    argv = list(sys.argv[1:])
    if "--" in argv:
        argv = argv[argv.index("--") + 1 :]

    parser = argparse.ArgumentParser(description="Import a Pixal3D FBX/PNG working set.")
    parser.add_argument("--working-dir", default=os.environ.get("T66_PIXAL3D_WORKING_DIR"))
    parser.add_argument("--asset-name", default=os.environ.get("T66_PIXAL3D_ASSET_NAME"))
    parser.add_argument("--target-dir", default=os.environ.get("T66_PIXAL3D_TARGET_DIR", "/Game/ToonStyle/TestAssets/Lineup"))
    parser.add_argument("--expected-height", type=float, default=float(os.environ.get("T66_PIXAL3D_EXPECTED_HEIGHT", EXPECTED_HEIGHT)))
    parser.add_argument("--height-tolerance", type=float, default=float(os.environ.get("T66_PIXAL3D_HEIGHT_TOLERANCE", HEIGHT_TOLERANCE)))
    args, _unknown = parser.parse_known_args(argv)

    missing = [name for name in ("working_dir", "asset_name", "target_dir") if not getattr(args, name)]
    if missing:
        raise RuntimeError(f"Missing required import argument(s): {', '.join(missing)}")
    return args


def ensure_dir(path: str) -> None:
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def delete_asset(path: str) -> None:
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        unreal.EditorAssetLibrary.delete_asset(path)


def import_fbx(fbx_path: Path, target_dir: str, asset_name: str) -> list[str]:
    ensure_dir(target_dir)
    task = unreal.AssetImportTask()
    task.automated = True
    task.save = False
    task.replace_existing = True
    task.replace_existing_settings = True
    task.filename = str(fbx_path)
    task.destination_path = target_dir
    task.destination_name = f"SM_{asset_name}"

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
        settings = {
            "combine_meshes": True,
            "auto_generate_collision": False,
            "generate_lightmap_u_vs": False,
            "normal_import_method": unreal.FBXNormalImportMethod.FBXNIM_IMPORT_NORMALS_AND_TANGENTS,
        }
        for prop_name, value in settings.items():
            try:
                static_mesh_data.set_editor_property(prop_name, value)
            except Exception as exc:
                unreal.log_warning(f"[ImportPixal3DAsset] Could not set FBX import {prop_name}: {exc}")
        try:
            static_mesh_data.set_editor_property("vertex_color_import_option", unreal.VertexColorImportOption.REPLACE)
        except Exception as exc:
            unreal.log_warning(f"[ImportPixal3DAsset] Could not set vertex_color_import_option: {exc}")

    task.options = options
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    return list(task.imported_object_paths or [])


def find_static_mesh(target_dir: str, asset_name: str) -> tuple[str, unreal.StaticMesh]:
    expected_path = f"{target_dir}/SM_{asset_name}"
    mesh = unreal.EditorAssetLibrary.load_asset(expected_path)
    if mesh and isinstance(mesh, unreal.StaticMesh):
        return expected_path, mesh

    assets = unreal.EditorAssetLibrary.list_assets(target_dir, recursive=True, include_folder=False)
    candidates = []
    for asset_path in assets:
        asset = unreal.EditorAssetLibrary.load_asset(asset_path)
        if asset and isinstance(asset, unreal.StaticMesh):
            candidates.append(asset_path.split(".", 1)[0])
    if not candidates:
        raise RuntimeError(f"FBX import returned no StaticMesh assets under {target_dir}")

    source_path = sorted(candidates)[0]
    if source_path != expected_path:
        if unreal.EditorAssetLibrary.does_asset_exist(expected_path):
            unreal.EditorAssetLibrary.delete_asset(expected_path)
        if not unreal.EditorAssetLibrary.rename_asset(source_path, expected_path):
            raise RuntimeError(f"Failed to rename imported StaticMesh {source_path} -> {expected_path}")
    mesh = unreal.EditorAssetLibrary.load_asset(expected_path)
    if not mesh or not isinstance(mesh, unreal.StaticMesh):
        raise RuntimeError(f"Final asset is not a StaticMesh: {expected_path}")
    return expected_path, mesh


def bounds_report(mesh: unreal.StaticMesh) -> dict[str, object]:
    bounds = mesh.get_bounds()
    extent = bounds.box_extent
    origin = bounds.origin
    size = [float(extent.x) * 2.0, float(extent.y) * 2.0, float(extent.z) * 2.0]
    return {
        "origin": [float(origin.x), float(origin.y), float(origin.z)],
        "extent": [float(extent.x), float(extent.y), float(extent.z)],
        "size": size,
        "height": size[2],
    }


def verify_height(mesh: unreal.StaticMesh, expected: float, tolerance: float) -> dict[str, object]:
    report = bounds_report(mesh)
    height = float(report["height"])
    lower = expected * (1.0 - tolerance)
    upper = expected * (1.0 + tolerance)
    if height < lower or height > upper:
        raise RuntimeError(f"Imported mesh height {height:.3f} is outside expected range {lower:.3f}-{upper:.3f}")
    return report


def import_texture(source_path: Path, texture_dir: str, texture_name: str, lod_group_name: str) -> tuple[str, unreal.Texture2D]:
    ensure_dir(texture_dir)
    texture_path = f"{texture_dir}/{texture_name}"
    delete_asset(texture_path)

    task = unreal.AssetImportTask()
    task.automated = True
    task.save = False
    task.replace_existing = True
    task.filename = str(source_path)
    task.destination_path = texture_dir
    task.destination_name = texture_name
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

    texture = unreal.EditorAssetLibrary.load_asset(texture_path)
    if not texture or not isinstance(texture, unreal.Texture2D):
        raise RuntimeError(f"Texture import failed: {texture_path}")

    lod_group = getattr(unreal.TextureGroup, lod_group_name, None)
    if lod_group is not None:
        texture.set_editor_property("lod_group", lod_group)
    try:
        texture.set_editor_property("srgb", True)
    except Exception:
        pass
    unreal.EditorAssetLibrary.save_loaded_asset(texture)
    return texture_path, texture


def create_material(
    material_dir: str,
    material_name: str,
    parent_path: str,
    texture: unreal.Texture2D | None = None,
) -> tuple[str, unreal.MaterialInstanceConstant]:
    ensure_dir(material_dir)
    material_path = f"{material_dir}/{material_name}"
    delete_asset(material_path)

    parent = unreal.EditorAssetLibrary.load_asset(parent_path)
    if not parent:
        raise RuntimeError(f"Missing parent material {parent_path}")

    factory = unreal.MaterialInstanceConstantFactoryNew()
    material = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        material_name,
        material_dir,
        unreal.MaterialInstanceConstant,
        factory,
    )
    if not material:
        raise RuntimeError(f"Material instance creation failed: {material_path}")

    material.set_editor_property("parent", parent)
    if texture is not None:
        unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(material, "DiffuseColorMap", texture)
        unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(material, "BaseColorTexture", texture)
        unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(material, "Brightness", 1.0)
        unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(
            material,
            "Tint",
            unreal.LinearColor(1.0, 1.0, 1.0, 1.0),
        )
    unreal.EditorAssetLibrary.save_loaded_asset(material)
    return material_path, material


def main() -> int:
    args = parse_args()
    working_dir = Path(args.working_dir)
    manifest_path = working_dir / f"{args.asset_name}_manifest.json"
    if not manifest_path.exists():
        raise RuntimeError(f"Missing manifest: {manifest_path}")
    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))

    target_dir = args.target_dir.rstrip("/")
    texture_dir = f"{target_dir}/Textures"
    material_dir = f"{target_dir}/Materials"
    mesh_path = f"{target_dir}/SM_{args.asset_name}"
    outline_mesh_path = f"{target_dir}/SM_{args.asset_name}_Outline"
    material_path = f"{material_dir}/MI_{args.asset_name}"
    outline_material_path = f"{material_dir}/MI_{args.asset_name}_Outline"

    delete_asset(mesh_path)
    delete_asset(outline_mesh_path)
    delete_asset(material_path)
    delete_asset(outline_material_path)

    imported_paths = import_fbx(Path(manifest["fbx_path"]), target_dir, args.asset_name)
    if not imported_paths:
        raise RuntimeError(f"FBX import returned zero object paths for {manifest['fbx_path']}")

    final_mesh_path, mesh = find_static_mesh(target_dir, args.asset_name)
    mesh_bounds = verify_height(mesh, args.expected_height, args.height_tolerance)

    outline_fbx_path = manifest.get("outline_fbx_path")
    if not outline_fbx_path:
        raise RuntimeError("Manifest missing outline_fbx_path")
    imported_outline_paths = import_fbx(Path(outline_fbx_path), target_dir, f"{args.asset_name}_Outline")
    if not imported_outline_paths:
        raise RuntimeError(f"Outline FBX import returned zero object paths for {outline_fbx_path}")
    final_outline_mesh_path, outline_mesh = find_static_mesh(target_dir, f"{args.asset_name}_Outline")
    outline_mesh_bounds = verify_height(outline_mesh, args.expected_height, args.height_tolerance)

    texture_entries = manifest.get("textures") or []
    if not texture_entries:
        raise RuntimeError("Manifest has no extracted textures")

    imported_textures = []
    for index, entry in enumerate(texture_entries):
        source_path = Path(entry["path"])
        texture_name = f"T_{args.asset_name}" if len(texture_entries) == 1 else f"T_{args.asset_name}_{index}"
        texture_path, texture = import_texture(source_path, texture_dir, texture_name, "TEXTUREGROUP_CHARACTER")
        imported_textures.append({"path": texture_path, "asset": texture})

    material_asset_path, material = create_material(
        material_dir,
        f"MI_{args.asset_name}",
        CHARACTER_PARENT_MATERIAL,
        imported_textures[0]["asset"],
    )
    outline_material_asset_path, outline_material = create_material(
        material_dir,
        f"MI_{args.asset_name}_Outline",
        OUTLINE_PARENT_MATERIAL,
    )
    mesh.set_material(0, material)
    unreal.EditorAssetLibrary.save_loaded_asset(mesh)
    outline_mesh.set_material(0, outline_material)
    unreal.EditorAssetLibrary.save_loaded_asset(outline_mesh)

    diffuse = unreal.MaterialEditingLibrary.get_material_instance_texture_parameter_value(material, "DiffuseColorMap")
    base = unreal.MaterialEditingLibrary.get_material_instance_texture_parameter_value(material, "BaseColorTexture")

    verify = {
        "asset_name": args.asset_name,
        "target_dir": target_dir,
        "static_mesh": final_mesh_path,
        "outline_static_mesh": final_outline_mesh_path,
        "static_mesh_bounds": mesh_bounds,
        "outline_static_mesh_bounds": outline_mesh_bounds,
        "imported_object_paths": imported_paths,
        "imported_outline_object_paths": imported_outline_paths,
        "textures": [entry["path"] for entry in imported_textures],
        "material_instance": material_asset_path,
        "outline_material_instance": outline_material_asset_path,
        "slot0_material": str(mesh.get_material(0).get_path_name()) if mesh.get_material(0) else None,
        "outline_slot0_material": str(outline_mesh.get_material(0).get_path_name()) if outline_mesh.get_material(0) else None,
        "diffuse_param": str(diffuse.get_path_name()) if diffuse else None,
        "base_color_param": str(base.get_path_name()) if base else None,
        "expected_height": args.expected_height,
        "height_tolerance": args.height_tolerance,
        "manifest_vertex_colors": manifest.get("vertex_colors"),
        "manifest_normal_transfer": manifest.get("normal_transfer"),
        "manifest_normal_delta_sample": manifest.get("normal_delta_sample"),
        "manifest_outline_winding": manifest.get("outline_winding"),
        "manifest_texture_flattening": manifest.get("texture_flattening"),
        "retained_from_phase1a": bool(manifest.get("retained_from_phase1a")),
        "flatten_k": manifest.get("flatten_k"),
    }

    verify_path = working_dir / f"{args.asset_name}_ue_verify.json"
    verify_path.write_text(json.dumps(verify, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    unreal.EditorAssetLibrary.save_directory(target_dir, only_if_is_dirty=False, recursive=True)
    unreal.log(f"[ImportPixal3DAsset] Imported {args.asset_name}: {json.dumps(verify, sort_keys=True)}")
    if os.environ.get("T66_PIXAL3D_QUIT_EDITOR") == "1":
        unreal.SystemLibrary.quit_editor()
    return 0


main()
