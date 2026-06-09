"""
Clean non-durable Pixal3D generated run folders from a cleanup manifest.

This helper only deletes manifest entries whose recommended_action is
delete_after_summary, and only when their resolved path remains inside the
configured generated-runs root.
"""

from __future__ import annotations

import argparse
import json
import shutil
from datetime import datetime, timezone
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]


def read_json(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8"))


def resolve_inside(path_value: str, root: Path) -> Path:
    path = Path(path_value)
    if not path.is_absolute():
        path = REPO_ROOT / path
    resolved = path.resolve()
    root_resolved = root.resolve()
    if resolved != root_resolved and root_resolved not in resolved.parents:
        raise RuntimeError(f"Refusing to touch path outside generated-runs root: {resolved}")
    return resolved


def clean(manifest: dict, runs_root: Path, dry_run: bool) -> dict:
    deleted = []
    skipped = []
    for row in manifest.get("generated_run_candidates", []):
        action = row.get("recommended_action")
        path = resolve_inside(row.get("path", ""), runs_root)
        entry = {
            "name": row.get("name", path.name),
            "path": str(path),
            "recommended_action": action,
            "reason": row.get("reason", ""),
            "existed": path.exists(),
        }
        if action != "delete_after_summary":
            skipped.append({**entry, "skip_reason": "not_marked_delete_after_summary"})
            continue
        if not path.exists():
            skipped.append({**entry, "skip_reason": "missing"})
            continue
        if dry_run:
            skipped.append({**entry, "skip_reason": "dry_run"})
            continue
        shutil.rmtree(path)
        deleted.append(entry)
    return {
        "generated_utc": datetime.now(timezone.utc).isoformat(),
        "dry_run": dry_run,
        "runs_root": str(runs_root.resolve()),
        "deleted_count": len(deleted),
        "skipped_count": len(skipped),
        "deleted": deleted,
        "skipped": skipped,
    }


def main():
    parser = argparse.ArgumentParser(description="Clean generated Pixal3D run folders from a manifest.")
    parser.add_argument("--manifest", type=Path, required=True)
    parser.add_argument("--runs-root", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--dry-run", action="store_true")
    args = parser.parse_args()

    manifest_path = args.manifest if args.manifest.is_absolute() else REPO_ROOT / args.manifest
    manifest = read_json(manifest_path)
    runs_root_value = args.runs_root or Path(manifest.get("inputs", {}).get("runs_root", ""))
    if not runs_root_value:
        raise RuntimeError("Runs root was not provided and is missing from manifest inputs.")
    runs_root = runs_root_value if runs_root_value.is_absolute() else REPO_ROOT / runs_root_value

    report = clean(manifest, runs_root, args.dry_run)
    output = args.output if args.output.is_absolute() else REPO_ROOT / args.output
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(report, indent=2), encoding="utf-8")
    print(
        f"Wrote generated-run cleanup report: {output} "
        f"deleted={report['deleted_count']} skipped={report['skipped_count']} dry_run={report['dry_run']}"
    )


if __name__ == "__main__":
    main()
