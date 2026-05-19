#!/usr/bin/env python3
"""Manifest-driven Pixal3D -> ToonStyle production import wrapper.

This is the canonical production entrypoint for Pixal3D assets that enter T66
runtime content. It intentionally delegates generation, Blender processing, and
Unreal import to the existing reusable scripts, then verifies that the ToonStyle
foundation outputs are present.
"""

from __future__ import annotations

import argparse
import json
import os
import shutil
import subprocess
import sys
from dataclasses import dataclass
from datetime import datetime, timezone
from pathlib import Path
from typing import Any


REPO_ROOT = Path(__file__).resolve().parents[3]
DEFAULT_MANIFEST = REPO_ROOT / "Model Generation" / "Pixal3D" / "production_asset_replacement_manifest.json"
DEFAULT_RUN_ROOT = REPO_ROOT / "Model Generation" / "Runs" / "Pixal3D" / "ProductionReplacement"
DEFAULT_REMOTE_ROOT = "/workspace/T66/ModelGeneration/Runs/Pixal3D/ProductionReplacement"
DEFAULT_BLENDER = Path(r"C:\Program Files\Blender Foundation\Blender 5.1\blender.exe")
DEFAULT_UNREAL = Path(r"C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe")
PROJECT_FILE = REPO_ROOT / "T66.uproject"
PIXAL_BATCH = REPO_ROOT / "Model Generation" / "Pixal3D" / "Scripts" / "run_pixal3d_batch.py"
BLENDER_PIPELINE = REPO_ROOT / "ToonStyle" / "BlenderScripts" / "run_toon_pipeline.py"
UE_IMPORT = REPO_ROOT / "ToonStyle" / "Source" / "ImportPixal3DAsset_Phase1C.py"
DEFAULT_BLACK = "/Game/ToonStyle/Textures/T_InnerLines_DefaultBlack"

ASSET_CLASSES = {"humanoid", "creature", "prop", "accepted-limitation"}
CATEGORIES = {"interactable", "prop", "mob", "environment", "hero"}
STATUSES = {"planned", "generated", "processed", "imported", "verified", "blocked"}


@dataclass
class ManifestAsset:
    row: dict[str, Any]

    @property
    def asset_id(self) -> str:
        return str(self.row["asset_id"])

    @property
    def asset_class(self) -> str:
        return str(self.row["asset_class"])

    @property
    def accepted_limitation(self) -> bool:
        return self.asset_class == "accepted-limitation"

    @property
    def target_dir(self) -> str:
        return str(self.row["target_dir"]).rstrip("/")

    @property
    def target_height(self) -> float:
        return float(self.row.get("target_height", 180.0))

    @property
    def source_image(self) -> Path:
        return resolve_path(str(self.row["source_image"]))

    @property
    def is_humanoid(self) -> bool:
        return self.asset_class == "humanoid" or bool(self.row.get("is_humanoid", False))


def utc_now() -> str:
    return datetime.now(timezone.utc).isoformat()


def resolve_path(value: str) -> Path:
    path = Path(value)
    if path.is_absolute():
        return path
    return (REPO_ROOT / path).resolve()


def load_manifest(path: Path) -> dict[str, Any]:
    with path.open("r", encoding="utf-8") as handle:
        return json.load(handle)


def settings(manifest: dict[str, Any]) -> dict[str, Any]:
    defaults = {
        "seed": 1337,
        "resolution": 1536,
        "texture_size": 4096,
        "decimation": 200000,
        "fallback_decimation": 80000,
        "remesh": True,
        "export_fallback": True,
        "safe_fill_holes_fallback": True,
        "ss_steps": 25,
        "ss_guidance": 7.5,
        "shape_steps": 25,
        "shape_guidance": 7.5,
        "tex_steps": 25,
        "tex_guidance": 4.0,
    }
    merged = defaults.copy()
    merged.update(manifest.get("settings", {}))
    return merged


def iter_assets(manifest: dict[str, Any]) -> list[ManifestAsset]:
    return [ManifestAsset(row) for row in manifest.get("assets", [])]


def validate_manifest(manifest_path: Path, allow_template: bool = False) -> tuple[dict[str, Any], list[str]]:
    manifest = load_manifest(manifest_path)
    errors: list[str] = []

    if manifest.get("workflow") != "pixal3d_toonstyle_production_import":
        errors.append("manifest.workflow must be pixal3d_toonstyle_production_import")
    if manifest.get("production_cleared") is not True:
        errors.append("manifest.production_cleared must be true")

    cfg = settings(manifest)
    if int(cfg["decimation"]) != 200000:
        errors.append("settings.decimation must be 200000 for production Pixal3D replacement runs")
    if int(cfg["fallback_decimation"]) > int(cfg["decimation"]):
        errors.append("settings.fallback_decimation cannot exceed settings.decimation")
    if int(cfg["fallback_decimation"]) < 80000:
        errors.append("settings.fallback_decimation should be at least 80000 for production unless an asset row documents an exception")
    if not bool(cfg["remesh"]):
        errors.append("settings.remesh must default to true; no-remesh is an explicit per-asset fallback state only")
    if not bool(cfg["export_fallback"]):
        errors.append("settings.export_fallback must be true")
    if not bool(cfg["safe_fill_holes_fallback"]):
        errors.append("settings.safe_fill_holes_fallback must be true")

    assets = iter_assets(manifest)
    if not assets:
        errors.append("manifest.assets must contain at least one asset row")

    for asset in assets:
        row = asset.row
        required = [
            "asset_id",
            "display_name",
            "category",
            "asset_class",
            "source_image",
            "target_dir",
            "target_height",
            "collision_policy",
            "gameplay_owner",
            "production_status",
        ]
        for key in required:
            if key not in row or row[key] in ("", None):
                errors.append(f"{asset.asset_id if 'asset_id' in row else '<missing>'}: missing {key}")
        if row.get("asset_id") == "example_interactable_replace_me" and not allow_template:
            errors.append("remove the example_interactable_replace_me row before production runs")
        if row.get("category") not in CATEGORIES:
            errors.append(f"{asset.asset_id}: category must be one of {sorted(CATEGORIES)}")
        if row.get("asset_class") not in ASSET_CLASSES:
            errors.append(f"{asset.asset_id}: asset_class must be one of {sorted(ASSET_CLASSES)}")
        if row.get("production_status") not in STATUSES:
            errors.append(f"{asset.asset_id}: production_status must be one of {sorted(STATUSES)}")
        if not str(row.get("target_dir", "")).startswith("/Game/"):
            errors.append(f"{asset.asset_id}: target_dir must start with /Game/")
        try:
            height = float(row.get("target_height", 0.0))
            if height <= 0:
                errors.append(f"{asset.asset_id}: target_height must be > 0")
        except (TypeError, ValueError):
            errors.append(f"{asset.asset_id}: target_height must be numeric")
        color = row.get("outline_color")
        if not isinstance(color, list) or len(color) != 4:
            errors.append(f"{asset.asset_id}: outline_color must be [r,g,b,a]")
        if row.get("asset_class") == "accepted-limitation" and not row.get("accepted_limitation_reason"):
            errors.append(f"{asset.asset_id}: accepted-limitation assets require accepted_limitation_reason")
        source = resolve_path(str(row.get("source_image", "")))
        if not source.exists() and not allow_template:
            errors.append(f"{asset.asset_id}: source_image not found: {source}")

    return manifest, errors


def run_command(args: list[str], cwd: Path = REPO_ROOT, env: dict[str, str] | None = None) -> None:
    print("+ " + " ".join(str(a) for a in args))
    completed = subprocess.run(args, cwd=str(cwd), env=env)
    if completed.returncode != 0:
        raise SystemExit(f"Command failed ({completed.returncode}): {' '.join(str(a) for a in args)}")


def stage_sources(manifest: dict[str, Any], run_root: Path) -> Path:
    sources_dir = run_root / "Sources"
    sources_dir.mkdir(parents=True, exist_ok=True)
    staged = []
    for asset in iter_assets(manifest):
        dest = sources_dir / f"{asset.asset_id}{asset.source_image.suffix.lower() or '.png'}"
        shutil.copy2(asset.source_image, dest)
        staged.append({"asset_id": asset.asset_id, "source": str(asset.source_image), "staged": str(dest)})
    (run_root / "source_stage_manifest.json").write_text(json.dumps(staged, indent=2) + "\n", encoding="utf-8")
    return sources_dir


def generation_command(manifest: dict[str, Any], args: argparse.Namespace, run_root: Path) -> list[str]:
    cfg = settings(manifest)
    sources_dir = stage_sources(manifest, run_root)
    command = [
        sys.executable,
        str(PIXAL_BATCH),
        "run",
        "--pod-ip",
        args.pod_ip,
        "--pod-port",
        str(args.pod_port),
        "--local-run-root",
        str(run_root),
        "--remote-run-root",
        args.remote_run_root,
        "--sources-dir",
        str(sources_dir),
        "--decimation",
        str(cfg["decimation"]),
        "--fallback-decimation",
        str(cfg["fallback_decimation"]),
        "--resolution",
        str(cfg["resolution"]),
        "--image-resolution",
        str(cfg.get("image_resolution", cfg["resolution"])),
        "--texture-size",
        str(cfg["texture_size"]),
        "--seed",
        str(cfg["seed"]),
        "--ss-steps",
        str(cfg["ss_steps"]),
        "--ss-guidance",
        str(cfg["ss_guidance"]),
        "--shape-steps",
        str(cfg["shape_steps"]),
        "--shape-guidance",
        str(cfg["shape_guidance"]),
        "--tex-steps",
        str(cfg["tex_steps"]),
        "--tex-guidance",
        str(cfg["tex_guidance"]),
    ]
    if cfg.get("remesh", True):
        command.append("--remesh")
    else:
        command.append("--no-remesh")
    if cfg.get("export_fallback", True):
        command.append("--export-fallback")
    else:
        command.append("--no-export-fallback")
    if cfg.get("safe_fill_holes_fallback", True):
        command.append("--safe-fill-holes-fallback")
    else:
        command.append("--no-safe-fill-holes-fallback")
    return command


def process_assets(manifest: dict[str, Any], args: argparse.Namespace, run_root: Path) -> None:
    outputs_dir = run_root / "Outputs"
    for asset in iter_assets(manifest):
        glb = outputs_dir / f"{asset.asset_id}.glb"
        if not glb.exists():
            raise SystemExit(f"{asset.asset_id}: missing generated GLB {glb}")
        working_dir = resolve_path(str(asset.row.get("working_dir", f"SourceAssets/ToonStyle/Pixal3D/Production/{asset.asset_id}/Working")))
        command = [
            str(args.blender_exe),
            "--background",
            "--python",
            str(BLENDER_PIPELINE),
            "--",
            "--input",
            str(glb),
            "--working-dir",
            str(working_dir),
            "--asset-name",
            asset.asset_id,
            "--target-height",
            str(asset.target_height),
            "--asset-class",
            asset.asset_class,
            "--enable-foundation-tools",
        ]
        if asset.is_humanoid:
            command.append("--is-humanoid")
        if asset.accepted_limitation:
            command.append("--accepted-limitation")
        run_command(command)


def import_assets(manifest: dict[str, Any], args: argparse.Namespace) -> None:
    for asset in iter_assets(manifest):
        working_dir = resolve_path(str(asset.row.get("working_dir", f"SourceAssets/ToonStyle/Pixal3D/Production/{asset.asset_id}/Working")))
        log_dir = working_dir / "Logs"
        log_dir.mkdir(parents=True, exist_ok=True)
        env = os.environ.copy()
        env["T66_PIXAL3D_WORKING_DIR"] = str(working_dir)
        env["T66_PIXAL3D_ASSET_NAME"] = asset.asset_id
        env["T66_PIXAL3D_TARGET_DIR"] = asset.target_dir
        env["T66_PIXAL3D_EXPECTED_HEIGHT"] = str(asset.target_height)
        env["T66_PIXAL3D_QUIT_EDITOR"] = "1"
        command = [
            str(args.unreal_editor_exe),
            str(PROJECT_FILE),
            f"-ExecutePythonScript={UE_IMPORT}",
            "-unattended",
            "-nop4",
            "-nosplash",
            "-NullRHI",
            f"-abslog={log_dir / (asset.asset_id + '_ue_import.log')}",
            "-forcelogflush",
        ]
        print("+ " + " ".join(command))
        completed = subprocess.run(command, cwd=str(REPO_ROOT), env=env)
        verify = working_dir / f"{asset.asset_id}_ue_verify.json"
        if not verify.exists():
            raise SystemExit(f"{asset.asset_id}: UE import failed, missing {verify} (exit={completed.returncode})")
        if completed.returncode != 0:
            print(f"[WARN] {asset.asset_id}: Unreal exited {completed.returncode} after writing verify JSON")


def verify_assets(manifest: dict[str, Any], run_root: Path) -> dict[str, Any]:
    rows = []
    errors = []
    outputs_dir = run_root / "Outputs"

    for asset in iter_assets(manifest):
        working_dir = resolve_path(str(asset.row.get("working_dir", f"SourceAssets/ToonStyle/Pixal3D/Production/{asset.asset_id}/Working")))
        glb = outputs_dir / f"{asset.asset_id}.glb"
        manifest_path = working_dir / f"{asset.asset_id}_manifest.json"
        verify_path = working_dir / f"{asset.asset_id}_ue_verify.json"
        row: dict[str, Any] = {
            "asset_id": asset.asset_id,
            "asset_class": asset.asset_class,
            "accepted_limitation": asset.accepted_limitation,
            "glb": str(glb),
            "working_dir": str(working_dir),
            "target_dir": asset.target_dir,
            "ok": True,
            "errors": [],
        }

        def fail(message: str) -> None:
            row["ok"] = False
            row["errors"].append(message)
            errors.append(f"{asset.asset_id}: {message}")

        if not glb.exists() or glb.stat().st_size <= 0:
            fail(f"missing or empty GLB: {glb}")
        if not manifest_path.exists():
            fail(f"missing Blender manifest: {manifest_path}")
            rows.append(row)
            continue
        if not verify_path.exists():
            fail(f"missing UE verify JSON: {verify_path}")
            rows.append(row)
            continue

        blender_manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
        ue_verify = json.loads(verify_path.read_text(encoding="utf-8"))
        foundation = blender_manifest.get("foundation_pass", {})
        close_gap = foundation.get("close_the_gap", {})
        inner_lines = foundation.get("inner_lines", {})

        if foundation.get("enabled") is not True:
            fail("foundation_pass.enabled is not true")
        if ue_verify.get("target_dir") != asset.target_dir:
            fail(f"UE target_dir mismatch: {ue_verify.get('target_dir')} != {asset.target_dir}")
        if not ue_verify.get("tint_param"):
            fail("TintTexture parameter is not bound")

        if asset.accepted_limitation:
            expected = f"{DEFAULT_BLACK}.{DEFAULT_BLACK.rsplit('/', 1)[-1]}"
            if ue_verify.get("inner_line_param") != expected:
                fail(f"accepted-limitation asset must bind default black inner line texture: {ue_verify.get('inner_line_param')}")
        else:
            rb = close_gap.get("readback", {})
            bmax = float(rb.get("B_max", 0.0))
            nonzero = float(rb.get("B_nonzero_fraction", 0.0))
            floor = 0.01 if asset.asset_class == "prop" else 0.05
            if not (0.0 < bmax <= 1.0):
                fail(f"invalid close-the-gap B_max {bmax}")
            if nonzero <= floor:
                fail(f"B_nonzero_fraction {nonzero} <= threshold {floor}")

            bake = inner_lines.get("bake_report", {})
            coverage = float(bake.get("line_coverage_fraction", 0.0))
            max_intensity = float(bake.get("max_intensity", 0.0))
            stddev = float(bake.get("stddev", 0.0))
            unique = int(bake.get("unique_value_count", 0))
            if unique < 2:
                fail(f"inner-line unique_value_count {unique} < 2")
            if not (0.005 < coverage < 0.5):
                fail(f"inner-line coverage {coverage} outside (0.005, 0.5)")
            if max_intensity <= 0.5:
                fail(f"inner-line max_intensity {max_intensity} <= 0.5")
            if stddev <= 0.01:
                fail(f"inner-line stddev {stddev} <= 0.01")
            if not ue_verify.get("inner_line_param") or "InnerLines" not in ue_verify["inner_line_param"]:
                fail(f"InnerLineTexture parameter not bound to asset texture: {ue_verify.get('inner_line_param')}")

            row.update(
                {
                    "B_max": bmax,
                    "B_nonzero_fraction": nonzero,
                    "inner_line_coverage_fraction": coverage,
                    "inner_line_unique_value_count": unique,
                    "inner_line_stddev": stddev,
                }
            )

        rows.append(row)

    report = {
        "ok": not errors,
        "generated_at": utc_now(),
        "errors": errors,
        "assets": rows,
    }
    report_path = run_root / "Reports" / "Pixal3D_ToonStyle_Production_Import_Report.json"
    report_path.parent.mkdir(parents=True, exist_ok=True)
    report_path.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(f"Wrote {report_path}")
    if errors:
        raise SystemExit("Production verification failed:\n" + "\n".join(errors))
    return report


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Run the Pixal3D ToonStyle production import workflow.")
    parser.add_argument("--manifest", type=Path, default=DEFAULT_MANIFEST)
    parser.add_argument("--run-root", type=Path, default=DEFAULT_RUN_ROOT)
    parser.add_argument("--pod-ip", default=os.environ.get("T66_PIXAL3D_POD_IP", ""))
    parser.add_argument("--pod-port", default=os.environ.get("T66_PIXAL3D_POD_PORT", ""))
    parser.add_argument("--remote-run-root", default=DEFAULT_REMOTE_ROOT)
    parser.add_argument("--blender-exe", type=Path, default=DEFAULT_BLENDER)
    parser.add_argument("--unreal-editor-exe", type=Path, default=DEFAULT_UNREAL)
    parser.add_argument("--allow-template", action="store_true")
    sub = parser.add_subparsers(dest="command", required=True)
    for name in ["validate", "generate", "process", "import", "verify", "run"]:
        sub.add_parser(name)
    return parser


def main(argv: list[str] | None = None) -> int:
    args = build_parser().parse_args(argv)
    args.manifest = args.manifest.resolve()
    args.run_root = args.run_root.resolve()

    manifest, errors = validate_manifest(args.manifest, allow_template=args.allow_template)
    if errors:
        for error in errors:
            print(f"ERROR: {error}", file=sys.stderr)
        return 2

    if args.command == "validate":
        print(f"Manifest valid: {args.manifest}")
        return 0
    if args.command in {"generate", "run"}:
        if not args.pod_ip or not args.pod_port:
            raise SystemExit("--pod-ip and --pod-port are required for generation")
        run_command(generation_command(manifest, args, args.run_root))
    if args.command in {"process", "run"}:
        process_assets(manifest, args, args.run_root)
    if args.command in {"import", "run"}:
        import_assets(manifest, args)
    if args.command in {"verify", "run"}:
        verify_assets(manifest, args.run_root)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
