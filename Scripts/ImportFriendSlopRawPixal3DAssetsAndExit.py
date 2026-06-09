"""
Import the FriendSlop Easy raw Pixal3D GLBs as static meshes, then exit.

This intentionally bypasses ToonStyle/QuadRetro processing. The source GLBs are
the raw Pixal3D outputs copied under SourceAssets/Import, and the existing
ImportStaticMeshes core handles Interchange import, flattening, GLB material
conversion, and mesh build settings.
"""

from __future__ import annotations

import json
import os
import sys
from pathlib import Path

import unreal


SCRIPT_DIR = Path(__file__).resolve().parent
REPO_ROOT = SCRIPT_DIR.parent
if str(SCRIPT_DIR) not in sys.path:
    sys.path.insert(0, str(SCRIPT_DIR))

import ImportStaticMeshes


LOG_PREFIX = "[ImportFriendSlopRawPixal3D]"
RUN_ROOT = (
    REPO_ROOT
    / "Model Generation"
    / "Runs"
    / "Pixal3D"
    / "FriendSlopEasyBatch_20260604_1532"
)
DEFAULT_MANIFEST = RUN_ROOT / "FriendSlopEasyBatch_20260604_1532_manifest.json"
RAW_MERGE_MANIFEST = RUN_ROOT / "raw_glb_merge_manifest.json"
REPORT_PATH = (
    REPO_ROOT
    / "Reports"
    / "AgentReviews"
    / "FriendSlopEasyPixal3D"
    / "raw_unreal_import_report.json"
)


def asset_name(asset_id: str) -> str:
    return f"SM_{asset_id}"


def load_json(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8"))


def build_import_entries(manifest: dict, merge_manifest: dict) -> list[dict]:
    source_by_id = {
        str(entry["asset_id"]): str(entry["source_import_relative"])
        for entry in merge_manifest.get("entries", [])
    }
    imports: list[dict] = []
    missing: list[str] = []

    for asset in manifest.get("assets", []):
        asset_id = str(asset.get("asset_id", "")).strip()
        target_dir = str(asset.get("target_dir", "")).strip().rstrip("/")
        source_rel = source_by_id.get(asset_id)
        if not asset_id or not target_dir or not source_rel:
            missing.append(asset_id or "<missing asset_id>")
            continue
        imports.append(
            {
                "source": source_rel,
                "dest": target_dir,
                "name": asset_name(asset_id),
                "material_overrides": {"brightness": 1.0},
                "cleanup": {"mode": "asset_subtree"},
            }
        )

    if missing:
        raise RuntimeError(f"Missing raw import entries for: {', '.join(missing)}")
    return imports


def verify_imported_assets(manifest: dict) -> tuple[list[dict], list[str]]:
    imported: list[dict] = []
    errors: list[str] = []
    for asset in manifest.get("assets", []):
        asset_id = str(asset.get("asset_id", "")).strip()
        target_dir = str(asset.get("target_dir", "")).strip().rstrip("/")
        package = f"{target_dir}/{asset_name(asset_id)}"
        object_path = f"{package}.{asset_name(asset_id)}"
        loaded = unreal.EditorAssetLibrary.load_asset(package)
        ok = bool(loaded and isinstance(loaded, unreal.StaticMesh))
        imported.append(
            {
                "asset_id": asset_id,
                "package": package,
                "object_path": object_path,
                "class": loaded.get_class().get_name() if loaded else "",
                "ok": ok,
            }
        )
        if not ok:
            errors.append(f"{asset_id}: missing StaticMesh {object_path}")
    return imported, errors


def main() -> None:
    unreal.log(f"{LOG_PREFIX} START raw Pixal3D import")
    manifest_path = Path(os.environ.get("T66_PIXAL3D_MANIFEST", str(DEFAULT_MANIFEST)))
    merge_path = Path(os.environ.get("T66_PIXAL3D_RAW_MERGE_MANIFEST", str(RAW_MERGE_MANIFEST)))
    manifest = load_json(manifest_path)
    merge_manifest = load_json(merge_path)

    imports = build_import_entries(manifest, merge_manifest)
    ImportStaticMeshes.IMPORTS = imports
    unreal.log(f"{LOG_PREFIX} Importing {len(imports)} raw GLB asset(s)")
    ImportStaticMeshes.main()

    imported, errors = verify_imported_assets(manifest)
    report = {
        "ok": not errors,
        "manifest": str(manifest_path),
        "merge_manifest": str(merge_path),
        "asset_count": len(imported),
        "imported": imported,
        "errors": errors,
        "notes": [
            "Raw Pixal3D GLBs imported directly.",
            "No ToonStyle, QuadRetro, outline, or processed GLB stage was run.",
        ],
    }
    REPORT_PATH.parent.mkdir(parents=True, exist_ok=True)
    REPORT_PATH.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    unreal.log(f"{LOG_PREFIX} Wrote {REPORT_PATH}")
    if errors:
        for error in errors:
            unreal.log_error(f"{LOG_PREFIX} {error}")
    else:
        unreal.log(f"{LOG_PREFIX} Verified {len(imported)} imported StaticMesh asset(s)")

    try:
        unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")
    except Exception as exc:
        unreal.log_warning(f"{LOG_PREFIX} Failed to request QUIT_EDITOR: {exc}")


main()
