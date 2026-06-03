"""
Read-only load-integrity check for world cleanup.

Loads the live maps and all remaining /Game/World assets after a cleanup pass so
direct .uasset deletions cannot hide missing world packages behind a C++ compile.
"""

import json
from datetime import datetime, timezone
from pathlib import Path

import unreal


PROJECT_ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
OUTPUT_PATH = PROJECT_ROOT / "Reports" / "Hygiene" / "2026-05-27" / "world_asset_load_integrity.json"
SCAN_ROOTS = ["/Game/World", "/Game/Maps"]
MAPS_TO_LOAD = ["/Game/Maps/FrontendLevel", "/Game/Maps/GameplayLevel"]


def log(message):
    unreal.log(f"[VerifyWorldAssetLoadIntegrity] {message}")


def force_scan():
    registry = unreal.AssetRegistryHelpers.get_asset_registry()
    try:
        registry.scan_paths_synchronous(SCAN_ROOTS, True)
    except TypeError:
        registry.scan_paths_synchronous(SCAN_ROOTS)
    try:
        registry.wait_for_completion()
    except Exception:
        pass


def load_maps():
    rows = []
    for map_package in MAPS_TO_LOAD:
        row = {"map": map_package, "loaded": False, "error": None}
        try:
            unreal.EditorLoadingAndSavingUtils.load_map(map_package)
            row["loaded"] = True
        except Exception as exc:
            row["error"] = str(exc)
        rows.append(row)
    return rows


def load_world_assets():
    rows = []
    asset_paths = unreal.EditorAssetLibrary.list_assets("/Game/World", recursive=True, include_folder=False)
    for asset_path in sorted(asset_paths):
        row = {"asset": str(asset_path), "loaded": False, "class": None, "error": None}
        try:
            asset = unreal.EditorAssetLibrary.load_asset(asset_path)
            row["loaded"] = bool(asset)
            if asset:
                row["class"] = asset.get_class().get_name()
            else:
                row["error"] = "load_asset returned null"
        except Exception as exc:
            row["error"] = str(exc)
        rows.append(row)
    return rows


def main():
    force_scan()
    map_rows = load_maps()
    asset_rows = load_world_assets()
    failures = [row for row in map_rows if not row["loaded"]] + [row for row in asset_rows if not row["loaded"]]
    report = {
        "generated_utc": datetime.now(timezone.utc).isoformat(),
        "scan_roots": SCAN_ROOTS,
        "maps": map_rows,
        "world_assets": asset_rows,
        "failure_count": len(failures),
        "failures": failures,
    }
    OUTPUT_PATH.parent.mkdir(parents=True, exist_ok=True)
    OUTPUT_PATH.write_text(json.dumps(report, indent=2), encoding="utf-8")
    log(f"maps={len(map_rows)} world_assets={len(asset_rows)} failures={len(failures)} output={OUTPUT_PATH}")
    if failures:
        raise RuntimeError(f"World asset load integrity failed: {len(failures)} failures")
    try:
        unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")
        log("QUIT_EDITOR requested")
    except Exception as exc:
        unreal.log_warning(f"[VerifyWorldAssetLoadIntegrity] Failed to request QUIT_EDITOR: {exc}")


main()
