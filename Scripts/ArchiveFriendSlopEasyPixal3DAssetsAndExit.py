"""
Archive existing FriendSlop Easy Pixal3D target assets before production import.

This duplicates currently existing Unreal assets into /Game/Archive so the
production importer can overwrite target packages without losing the prior
packages. It does not delete or move live assets.
"""

from __future__ import annotations

import json
import os
from pathlib import Path

import unreal


LOG_PREFIX = "[ArchiveFriendSlopEasyPixal3D]"
ARCHIVE_ROOT = "/Game/Archive/FriendSlopEasyPixal3D_20260604"
DEFAULT_MANIFEST = (
    Path(__file__).resolve().parents[1]
    / "Model Generation"
    / "Runs"
    / "Pixal3D"
    / "FriendSlopEasyBatch_20260604_1532"
    / "FriendSlopEasyBatch_20260604_1532_manifest.json"
)
REPORT_PATH = (
    Path(__file__).resolve().parents[1]
    / "Reports"
    / "AgentReviews"
    / "FriendSlopEasyPixal3D"
    / "unreal_archive_report.json"
)


def package_path(ref: object) -> str:
    text = str(ref or "").strip()
    if not text:
        return ""
    if "." in text:
        return text.split(".", 1)[0]
    return text


def ensure_dir(path: str) -> None:
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def unique_destination(dest_package: str) -> str:
    candidate = dest_package
    index = 1
    while unreal.EditorAssetLibrary.does_asset_exist(candidate):
        candidate = f"{dest_package}_{index}"
        index += 1
    return candidate


def archive_destination(source_package: str) -> str:
    if not source_package.startswith("/Game/"):
        raise ValueError(f"Only /Game assets can be archived, got {source_package}")
    return f"{ARCHIVE_ROOT}/{source_package[len('/Game/'):]}".rstrip("/")


def expected_asset_packages(asset: dict[str, object]) -> list[str]:
    asset_id = str(asset["asset_id"]).strip()
    target_dir = str(asset["target_dir"]).strip().rstrip("/")
    packages = [
        f"{target_dir}/SM_{asset_id}",
        f"{target_dir}/SM_{asset_id}_Outline",
        f"{target_dir}/Textures/T_{asset_id}",
        f"{target_dir}/Textures/T_{asset_id}_0",
        f"{target_dir}/Textures/T_{asset_id}_Tint",
        f"{target_dir}/Textures/T_{asset_id}_InnerLines",
        f"{target_dir}/Materials/MI_{asset_id}",
        f"{target_dir}/Materials/MI_{asset_id}_Outline",
    ]
    replaced = package_path(asset.get("replaces"))
    if replaced:
        packages.append(replaced)
    return list(dict.fromkeys(packages))


def main() -> None:
    manifest_path = Path(os.environ.get("T66_PIXAL3D_MANIFEST", str(DEFAULT_MANIFEST)))
    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    ensure_dir(ARCHIVE_ROOT)
    archived: list[dict[str, object]] = []
    skipped: list[dict[str, str]] = []
    errors: list[str] = []

    for asset in manifest.get("assets", []):
        asset_id = str(asset.get("asset_id", "")).strip()
        for source in expected_asset_packages(asset):
            if not source.startswith("/Game/"):
                skipped.append({"asset_id": asset_id, "source": source, "reason": "outside /Game"})
                continue
            if not unreal.EditorAssetLibrary.does_asset_exist(source):
                skipped.append({"asset_id": asset_id, "source": source, "reason": "missing"})
                continue
            dest = unique_destination(archive_destination(source))
            ensure_dir(dest.rsplit("/", 1)[0])
            duplicate = unreal.EditorAssetLibrary.duplicate_asset(source, dest)
            if duplicate:
                unreal.EditorAssetLibrary.save_asset(dest)
                archived.append({"asset_id": asset_id, "source": source, "archive": dest})
                unreal.log(f"{LOG_PREFIX} Archived {source} -> {dest}")
            else:
                message = f"Failed to archive {source} -> {dest}"
                errors.append(message)
                unreal.log_error(f"{LOG_PREFIX} {message}")

    report = {
        "ok": not errors,
        "manifest": str(manifest_path),
        "archive_root": ARCHIVE_ROOT,
        "archived": archived,
        "skipped": skipped,
        "errors": errors,
    }
    REPORT_PATH.parent.mkdir(parents=True, exist_ok=True)
    REPORT_PATH.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    unreal.log(f"{LOG_PREFIX} Wrote {REPORT_PATH}")
    unreal.log(f"{LOG_PREFIX} Archived {len(archived)} existing asset(s); skipped {len(skipped)} missing/non-game asset(s)")

    try:
        unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")
    except Exception as exc:
        unreal.log_warning(f"{LOG_PREFIX} Failed to request QUIT_EDITOR: {exc}")


main()
