#!/usr/bin/env python3
"""Export a Pixal3D GLB as textured FBX/OBJ for AccuRig handoff.

The ToonStyle production FBX intentionally strips texture references because
Unreal receives ToonStyle material instances separately. AccuRig needs a
standard textured mesh, so this script preserves the GLB's material texture
bindings and writes a handoff bundle.
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path
from typing import Any

import bpy

SCRIPT_DIR = Path(__file__).resolve().parent
if str(SCRIPT_DIR) not in sys.path:
    sys.path.insert(0, str(SCRIPT_DIR))

from pixal3d_blender_base import clear_scene, export_image_as_png, import_glb, join_meshes, normalize_spatial, world_bounds


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Export a textured AccuRig FBX/OBJ bundle from a Pixal3D GLB.")
    parser.add_argument("--input", required=True, type=Path)
    parser.add_argument("--output-dir", required=True, type=Path)
    parser.add_argument("--asset-name", required=True)
    parser.add_argument("--target-height", type=float, default=180.0)
    return parser.parse_args(argv)


def safe_name(value: str) -> str:
    cleaned = re.sub(r"[^A-Za-z0-9_.-]+", "_", value).strip("._-")
    return cleaned or "texture"


def find_image_nodes(material: bpy.types.Material) -> list[bpy.types.ShaderNodeTexImage]:
    if not material or not material.use_nodes or material.node_tree is None:
        return []
    return [node for node in material.node_tree.nodes if node.bl_idname == "ShaderNodeTexImage" and node.image is not None]


def texture_usage(obj: bpy.types.Object) -> list[dict[str, Any]]:
    usage: list[dict[str, Any]] = []
    seen: set[str] = set()
    for material in obj.data.materials:
        if material is None:
            continue
        for node in find_image_nodes(material):
            image = node.image
            key = f"{material.name}|{node.name}|{image.name}"
            if key in seen:
                continue
            seen.add(key)
            usage.append(
                {
                    "material": material.name,
                    "node": node.name,
                    "image": image.name,
                    "image_size": [int(image.size[0]), int(image.size[1])],
                }
            )
    return usage


def save_and_repath_images(texture_dir: Path, asset_name: str) -> list[dict[str, Any]]:
    texture_dir.mkdir(parents=True, exist_ok=True)
    exports: list[dict[str, Any]] = []
    seen_names: set[str] = set()
    for index, image in enumerate(
        image
        for image in bpy.data.images
        if image.name not in {"Render Result", "Viewer Node"} and not image.name.startswith(".")
    ):
        base = safe_name(image.name)
        file_name = f"{asset_name}_{index:02d}_{base}.png"
        while file_name.lower() in seen_names:
            file_name = f"{asset_name}_{index:02d}_{base}_{len(seen_names)}.png"
        seen_names.add(file_name.lower())
        output_path = texture_dir / file_name
        export_image_as_png(image, output_path)
        image.filepath = str(output_path)
        image.filepath_raw = str(output_path)
        image.source = "FILE"
        exports.append(
            {
                "image": image.name,
                "path": str(output_path),
                "size": [int(image.size[0]), int(image.size[1])],
            }
        )
    return exports


def select_only(obj: bpy.types.Object) -> None:
    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj


def export_fbx_embedded(obj: bpy.types.Object, output_path: Path) -> dict[str, Any]:
    output_path.parent.mkdir(parents=True, exist_ok=True)
    select_only(obj)
    options = {
        "filepath": str(output_path),
        "use_selection": True,
        "object_types": {"MESH"},
        "global_scale": 0.01,
        "apply_unit_scale": True,
        "apply_scale_options": "FBX_SCALE_ALL",
        "bake_space_transform": False,
        "path_mode": "COPY",
        "embed_textures": True,
        "use_mesh_modifiers": True,
        "mesh_smooth_type": "FACE",
        "add_leaf_bones": False,
    }
    bpy.ops.export_scene.fbx(**options)
    return {key: sorted(value) if isinstance(value, set) else value for key, value in options.items() if key != "filepath"}


def export_obj_external(obj: bpy.types.Object, output_path: Path) -> dict[str, Any]:
    output_path.parent.mkdir(parents=True, exist_ok=True)
    select_only(obj)
    options = {
        "filepath": str(output_path),
        "export_selected_objects": True,
        "export_uv": True,
        "export_normals": True,
        "export_materials": True,
        "export_pbr_extensions": False,
        "path_mode": "RELATIVE",
        "global_scale": 1.0,
        "apply_modifiers": True,
        "apply_transform": False,
    }
    if hasattr(bpy.ops.wm, "obj_export"):
        bpy.ops.wm.obj_export(**options)
    else:
        legacy = {
            "filepath": str(output_path),
            "use_selection": True,
            "use_uvs": True,
            "use_normals": True,
            "use_materials": True,
            "path_mode": "RELATIVE",
            "global_scale": 1.0,
        }
        bpy.ops.export_scene.obj(**legacy)
        options = legacy
    return {key: value for key, value in options.items() if key != "filepath"}


def bounds_report(obj: bpy.types.Object) -> dict[str, Any]:
    mins, maxs = world_bounds(obj)
    dims = maxs - mins
    return {
        "min": [mins.x, mins.y, mins.z],
        "max": [maxs.x, maxs.y, maxs.z],
        "dimensions": [dims.x, dims.y, dims.z],
    }


def blender_script_args(argv: list[str] | None = None) -> list[str]:
    raw = list(sys.argv if argv is None else argv)
    if "--" in raw:
        return raw[raw.index("--") + 1 :]
    return raw[1:]


def main(argv: list[str] | None = None) -> int:
    args = parse_args(blender_script_args(argv))
    args.output_dir.mkdir(parents=True, exist_ok=True)
    texture_dir = args.output_dir / "Textures"

    clear_scene()
    imported = import_glb(args.input)
    joined = join_meshes(imported)
    joined.name = args.asset_name
    joined.data.name = f"{args.asset_name}_mesh"
    normalization = normalize_spatial(joined, args.target_height)

    texture_exports = save_and_repath_images(texture_dir, args.asset_name)
    if not texture_exports:
        raise RuntimeError(f"{args.asset_name}: no images found in imported GLB; cannot create textured AccuRig bundle")

    usage = texture_usage(joined)
    if not usage:
        raise RuntimeError(f"{args.asset_name}: no material image texture nodes found after GLB import")

    fbx_path = args.output_dir / f"{args.asset_name}_Textured.fbx"
    obj_path = args.output_dir / f"{args.asset_name}_Textured.obj"
    fbx_options = export_fbx_embedded(joined, fbx_path)
    obj_options = export_obj_external(joined, obj_path)

    mtl_path = obj_path.with_suffix(".mtl")
    report = {
        "asset_name": args.asset_name,
        "input_glb": str(args.input),
        "output_dir": str(args.output_dir),
        "fbx_path": str(fbx_path),
        "fbx_bytes": fbx_path.stat().st_size if fbx_path.exists() else 0,
        "obj_path": str(obj_path),
        "obj_bytes": obj_path.stat().st_size if obj_path.exists() else 0,
        "mtl_path": str(mtl_path),
        "mtl_bytes": mtl_path.stat().st_size if mtl_path.exists() else 0,
        "textures": texture_exports,
        "texture_usage": usage,
        "normalization": normalization,
        "bounds": bounds_report(joined),
        "vertex_count": len(joined.data.vertices),
        "material_count": len(joined.data.materials),
        "fbx_export_options": fbx_options,
        "obj_export_options": obj_options,
    }
    report_path = args.output_dir / f"{args.asset_name}_accurig_textured_export.json"
    report_path.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(json.dumps(report, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
