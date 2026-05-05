"""
Delete unused hero model/sprite folders identified by AuditCharacterModelDataAndExit.

This script is intentionally narrow: it only deletes hero folders reported as
having no hero row, no character-visual rows, no character-data references, and
no outside package referencers.
"""

import json
from pathlib import Path

import unreal


PROJECT_ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
AUDIT_PATH = PROJECT_ROOT / "Saved" / "Audits" / "CharacterModelDataAudit.json"
CLEANUP_REPORT_PATH = PROJECT_ROOT / "Saved" / "Audits" / "CharacterModelDataCleanupReport.json"
DELETE_ROOTS = (
    "/Game/Characters/Heroes",
    "/Game/UI/Sprites/Heroes",
)


def load_audit():
    if not AUDIT_PATH.exists():
        raise RuntimeError(f"Missing audit report: {AUDIT_PATH}")
    return json.loads(AUDIT_PATH.read_text(encoding="utf-8"))


def validate_candidate(hero_id, summary):
    if summary.get("has_hero_row"):
        return False
    if summary.get("has_visual_rows"):
        return False
    if int(summary.get("character_data_reference_count", 0)) != 0:
        return False
    if int(summary.get("outside_referencer_count", 0)) != 0:
        return False
    return True


def main():
    audit = load_audit()
    candidates = audit.get("unused_hero_folder_candidates", {})
    deleted = []
    skipped = []

    for hero_id, summary in sorted(candidates.items()):
        if not validate_candidate(hero_id, summary):
            skipped.append({"hero_id": hero_id, "reason": "candidate_failed_validation", "summary": summary})
            continue
        for root in DELETE_ROOTS:
            directory = f"{root}/{hero_id}"
            if not unreal.EditorAssetLibrary.does_directory_exist(directory):
                skipped.append({"hero_id": hero_id, "directory": directory, "reason": "missing_directory"})
                continue
            if not unreal.EditorAssetLibrary.delete_directory(directory):
                raise RuntimeError(f"Failed to delete directory: {directory}")
            deleted.append({"hero_id": hero_id, "directory": directory})
            unreal.log(f"[CleanupUnusedHeroCharacterAssets] Deleted {directory}")

    for root in DELETE_ROOTS:
        if unreal.EditorAssetLibrary.does_directory_exist(root):
            unreal.EditorAssetLibrary.save_directory(root, only_if_is_dirty=True, recursive=True)

    CLEANUP_REPORT_PATH.parent.mkdir(parents=True, exist_ok=True)
    CLEANUP_REPORT_PATH.write_text(
        json.dumps({"deleted": deleted, "skipped": skipped}, indent=2),
        encoding="utf-8",
    )
    unreal.log(
        "[CleanupUnusedHeroCharacterAssets] "
        f"deleted={len(deleted)} skipped={len(skipped)} output={CLEANUP_REPORT_PATH}"
    )

    try:
        unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")
    except Exception as exc:
        unreal.log_warning(f"[CleanupUnusedHeroCharacterAssets] Failed to request QUIT_EDITOR: {exc}")


main()
