#!/usr/bin/env python3
"""Batch export textured AccuRig FBX/OBJ bundles from Pixal3D GLBs."""

from __future__ import annotations

import argparse
import json
import subprocess
import sys
from pathlib import Path
from typing import Any

REPO_ROOT = Path(__file__).resolve().parents[3]
PROJECT_SCRIPT = REPO_ROOT / "ToonStyle" / "BlenderScripts" / "export_accurig_textured_mesh.py"
DEFAULT_BLENDER = Path(r"C:\Program Files\Blender Foundation\Blender 5.1\blender.exe")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Export textured AccuRig FBX/OBJ bundles from a Pixal3D run.")
    parser.add_argument("--manifest", required=True, type=Path)
    parser.add_argument("--run-root", required=True, type=Path)
    parser.add_argument("--output-root", type=Path)
    parser.add_argument("--blender-exe", type=Path, default=DEFAULT_BLENDER)
    parser.add_argument("--target-height", type=float, default=180.0)
    parser.add_argument("--only", nargs="*", default=[])
    parser.add_argument("--force", action="store_true")
    return parser.parse_args()


def resolve_repo_path(path_text: str) -> Path:
    path = Path(path_text)
    if path.is_absolute():
        return path
    return REPO_ROOT / path


def load_manifest(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8-sig"))


def asset_ids(manifest: dict[str, Any]) -> list[str]:
    return [str(row["asset_id"]) for row in manifest.get("assets", [])]


def run_blender(command: list[str], cwd: Path) -> None:
    print("+ " + " ".join(command))
    completed = subprocess.run(command, cwd=str(cwd))
    if completed.returncode != 0:
        raise SystemExit(f"Blender export failed ({completed.returncode})")


def validate_bundle(asset_id: str, output_dir: Path) -> dict[str, Any]:
    fbx = output_dir / f"{asset_id}_Textured.fbx"
    obj = output_dir / f"{asset_id}_Textured.obj"
    mtl = output_dir / f"{asset_id}_Textured.mtl"
    report = output_dir / f"{asset_id}_accurig_textured_export.json"
    textures = sorted((output_dir / "Textures").glob("*.png"))
    mtl_text = mtl.read_text(encoding="utf-8", errors="replace") if mtl.exists() else ""
    return {
        "asset_id": asset_id,
        "output_dir": str(output_dir),
        "fbx": str(fbx),
        "fbx_bytes": fbx.stat().st_size if fbx.exists() else 0,
        "obj": str(obj),
        "obj_bytes": obj.stat().st_size if obj.exists() else 0,
        "mtl": str(mtl),
        "mtl_bytes": mtl.stat().st_size if mtl.exists() else 0,
        "texture_count": len(textures),
        "texture_bytes": sum(path.stat().st_size for path in textures),
        "mtl_has_map_kd": "map_Kd" in mtl_text,
        "per_asset_report": str(report),
        "per_asset_report_exists": report.exists(),
    }


def main() -> int:
    args = parse_args()
    manifest = load_manifest(args.manifest)
    run_root = args.run_root.resolve()
    output_root = (args.output_root or (run_root / "AccuRig_Textured")).resolve()
    output_root.mkdir(parents=True, exist_ok=True)

    if not args.blender_exe.exists():
        raise SystemExit(f"Blender not found: {args.blender_exe}")
    if not PROJECT_SCRIPT.exists():
        raise SystemExit(f"AccuRig Blender script not found: {PROJECT_SCRIPT}")

    only = set(args.only)
    selected = [row for row in manifest.get("assets", []) if not only or str(row["asset_id"]) in only]
    if not selected:
        raise SystemExit("No manifest assets selected.")

    rows: list[dict[str, Any]] = []
    for row in selected:
        asset_id = str(row["asset_id"])
        glb = run_root / "Outputs" / f"{asset_id}.glb"
        if not glb.exists() or glb.stat().st_size <= 0:
            raise SystemExit(f"{asset_id}: missing nonzero GLB: {glb}")
        target_height = float(row.get("target_height", args.target_height) or args.target_height)
        output_dir = output_root / asset_id
        fbx = output_dir / f"{asset_id}_Textured.fbx"
        obj = output_dir / f"{asset_id}_Textured.obj"
        if (fbx.exists() or obj.exists()) and not args.force:
            print(f"[SKIP] {asset_id}: textured bundle exists; pass --force to overwrite.")
            rows.append(validate_bundle(asset_id, output_dir))
            continue

        command = [
            str(args.blender_exe),
            "--background",
            "--python-exit-code",
            "1",
            "--python",
            str(PROJECT_SCRIPT),
            "--",
            "--input",
            str(glb),
            "--output-dir",
            str(output_dir),
            "--asset-name",
            asset_id,
            "--target-height",
            str(target_height),
        ]
        run_blender(command, REPO_ROOT)
        rows.append(validate_bundle(asset_id, output_dir))

    errors = []
    for row in rows:
        if row["fbx_bytes"] <= 0:
            errors.append(f"{row['asset_id']}: missing textured FBX")
        if row["obj_bytes"] <= 0:
            errors.append(f"{row['asset_id']}: missing OBJ")
        if row["mtl_bytes"] <= 0:
            errors.append(f"{row['asset_id']}: missing MTL")
        if row["texture_count"] <= 0:
            errors.append(f"{row['asset_id']}: missing texture PNGs")
        if not row["mtl_has_map_kd"]:
            errors.append(f"{row['asset_id']}: MTL has no map_Kd texture reference")
        if not row["per_asset_report_exists"]:
            errors.append(f"{row['asset_id']}: missing per-asset report")

    report = {
        "schema_version": 1,
        "manifest": str(args.manifest.resolve()),
        "run_root": str(run_root),
        "output_root": str(output_root),
        "asset_count": len(rows),
        "fbx_count": sum(1 for row in rows if row["fbx_bytes"] > 0),
        "obj_count": sum(1 for row in rows if row["obj_bytes"] > 0),
        "texture_bundle_count": sum(1 for row in rows if row["texture_count"] > 0),
        "errors": errors,
        "rows": rows,
    }
    report_path = output_root / "AccuRig_Textured_Export_Report.json"
    report_path.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(json.dumps({"report": str(report_path), "errors": errors, "asset_count": len(rows)}, indent=2))
    if errors:
        raise SystemExit(1)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
