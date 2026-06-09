"""
Delete model cleanup assets approved by EvaluateModelCleanupExactAudit.py.

This script runs in Unreal and only consumes an approved gate JSON. It never
performs candidate discovery and never deletes packages that were skipped by the
gate report.
"""

from __future__ import annotations

import json
import re
from datetime import datetime, timezone
from pathlib import Path

import unreal


LOG = "[DeleteModelCleanupAssets]"
PROJECT_ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
DEFAULT_REPORT = PROJECT_ROOT / "Reports" / "Hygiene" / "2026-06-05" / "model_cleanup_delete_report.json"

CLASS_DELETE_ORDER = {
    "AnimSequence": 10,
    "SkeletalMesh": 20,
    "Skeleton": 30,
    "StaticMesh": 40,
    "MaterialInstanceConstant": 50,
    "Material": 60,
    "Texture2D": 70,
}


def log(message: str):
    unreal.log(f"{LOG} {message}")


def warn(message: str):
    unreal.log_warning(f"{LOG} {message}")


def get_command_line() -> str:
    try:
        return unreal.SystemLibrary.get_command_line()
    except Exception:
        return ""


def get_param(name: str, default: str | None = None) -> str | None:
    match = re.search(rf"-{re.escape(name)}=(\"[^\"]*\"|\S+)", get_command_line())
    if not match:
        return default
    value = match.group(1)
    if value.startswith('"') and value.endswith('"'):
        return value[1:-1]
    return value


def package_to_asset_path(package: str) -> str:
    return f"{package}.{package.rsplit('/', 1)[-1]}"


def load_asset_row(package: str) -> dict:
    asset_path = package_to_asset_path(package)
    asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    class_name = asset.get_class().get_name() if asset else ""
    return {
        "package": package,
        "asset_path": asset_path,
        "class": class_name,
        "exists": bool(asset),
        "delete_order": CLASS_DELETE_ORDER.get(class_name, 100),
    }


def save_touched_roots(packages: list[str]):
    roots = sorted({"/".join(package.split("/")[:3]) for package in packages if package.startswith("/Game/")})
    for root in roots:
        if unreal.EditorAssetLibrary.does_directory_exist(root):
            unreal.EditorAssetLibrary.save_directory(root, only_if_is_dirty=True, recursive=True)


def main():
    gate_path_value = get_param("T66ModelCleanupGate")
    if not gate_path_value:
        raise RuntimeError("Missing -T66ModelCleanupGate=<gate json path>")
    gate_path = Path(gate_path_value)
    if not gate_path.is_absolute():
        gate_path = PROJECT_ROOT / gate_path
    if not gate_path.exists():
        raise RuntimeError(f"Missing gate report: {gate_path}")

    report_path = Path(get_param("T66ModelCleanupDeleteReport", str(DEFAULT_REPORT)))
    if not report_path.is_absolute():
        report_path = PROJECT_ROOT / report_path

    gate = json.loads(gate_path.read_text(encoding="utf-8"))
    packages = [str(package) for package in gate.get("approved_packages", [])]
    rows = [load_asset_row(package) for package in packages]
    rows.sort(key=lambda row: (row["delete_order"], row["package"]))

    deleted = []
    skipped = []
    for row in rows:
        if not row["exists"]:
            skipped.append({**row, "reason": "asset_missing"})
            continue
        if not unreal.EditorAssetLibrary.delete_asset(row["asset_path"]):
            skipped.append({**row, "reason": "delete_failed"})
            warn(f"Delete failed: {row['asset_path']}")
            continue
        deleted.append(row)
        log(f"Deleted {row['asset_path']}")

    save_touched_roots(packages)

    output = {
        "generated_utc": datetime.now(timezone.utc).isoformat(),
        "gate_path": str(gate_path),
        "group": gate.get("group"),
        "approved_count": len(packages),
        "deleted_count": len(deleted),
        "skipped_count": len(skipped),
        "deleted": deleted,
        "skipped": skipped,
    }
    report_path.parent.mkdir(parents=True, exist_ok=True)
    report_path.write_text(json.dumps(output, indent=2), encoding="utf-8")
    log(f"deleted={len(deleted)} skipped={len(skipped)} output={report_path}")

    try:
        unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")
        log("QUIT_EDITOR requested")
    except Exception as exc:
        warn(f"Failed to request QUIT_EDITOR: {exc}")


main()
