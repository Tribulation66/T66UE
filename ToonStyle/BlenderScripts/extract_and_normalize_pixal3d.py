#!/usr/bin/env python3
"""Compatibility wrapper for the Phase 1B Pixal3D Blender base module.

The durable implementation lives in ``pixal3d_blender_base.py`` so newer
pipeline scripts can import the same operations inside one Blender session.
This wrapper preserves the Phase 1A.2 command-line behavior for any manual
single-stage extraction runs.
"""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

from pixal3d_blender_base import (
    clear_scene,
    export_fbx,
    extract_textures,
    join_meshes,
    normalize_spatial,
    strip_material_texture_references,
    import_glb,
)


def parse_args() -> argparse.Namespace:
    argv = sys.argv
    if "--" in argv:
        argv = argv[argv.index("--") + 1 :]
    else:
        argv = []

    parser = argparse.ArgumentParser(description="Extract and normalize a Pixal3D GLB for UE import.")
    parser.add_argument("--input", required=True, type=Path)
    parser.add_argument("--working-dir", required=True, type=Path)
    parser.add_argument("--asset-name", required=True)
    parser.add_argument("--target-height", default=180.0, type=float)
    parser.add_argument(
        "--scale-policy",
        default="height",
        choices=("height",),
        help="Current ToonStyle evaluation policy: normalize Z height to target-height.",
    )
    return parser.parse_args(argv)


def main() -> int:
    args = parse_args()
    args.working_dir.mkdir(parents=True, exist_ok=True)

    clear_scene()
    imported = import_glb(args.input)
    textures = extract_textures(imported, args.working_dir, args.asset_name)
    joined = join_meshes(imported)
    normalization = normalize_spatial(joined, target_height=args.target_height)
    material_count_before_strip = len(joined.data.materials)
    strip_material_texture_references(joined)
    material_count_after_strip = len(joined.data.materials)

    fbx_path = args.working_dir / f"{args.asset_name}.fbx"
    export_options = export_fbx(joined, fbx_path)

    manifest = {
        "asset_name": args.asset_name,
        "input_glb": str(args.input),
        "working_dir": str(args.working_dir),
        "fbx_path": str(fbx_path),
        "textures": textures,
        "normalization": normalization,
        "vertex_count": len(joined.data.vertices),
        "material_count_before_strip": material_count_before_strip,
        "material_count_after_strip": material_count_after_strip,
        "mesh_count_before_join": len(imported),
        "mesh_count_after_join": 1,
        "target_height": args.target_height,
        "fbx_export_options": export_options,
    }

    manifest_path = args.working_dir / f"{args.asset_name}_manifest.json"
    manifest_path.write_text(json.dumps(manifest, indent=2), encoding="utf-8")
    print(json.dumps(manifest, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
