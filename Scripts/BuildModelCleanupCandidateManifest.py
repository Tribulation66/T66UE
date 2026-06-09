"""
Build model cleanup candidate manifests from current Unreal audit JSON.

This is intentionally non-destructive. It turns broad audit output into named
package groups that still require exact reference proof before any runtime
Content asset can be deleted.
"""

from __future__ import annotations

import argparse
import json
from datetime import datetime, timezone
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
DEFAULT_CHARACTER_AUDIT = REPO_ROOT / "Saved" / "Audits" / "CharacterModelDataAudit.json"
DEFAULT_WORLD_AUDIT = REPO_ROOT / "Saved" / "Audits" / "WorldAssetAudit.json"
DEFAULT_OUTPUT = REPO_ROOT / "Reports" / "Hygiene" / "2026-06-05" / "model_cleanup_candidate_manifest.json"

WORLD_GROUP_ROOTS = {
    "Boosts": "world_boost_visual_candidates",
    "Gates": "world_gate_visual_candidates",
    "Interactables": "world_interactable_visual_candidates",
    "LootBags": "world_lootbag_visual_candidates",
    "VisualProps": "world_visualprop_candidates",
}

GENERATED_DELETE_NAME_FRAGMENTS = (
    "retry",
    "PipelineSmoke",
    "HumanoidGuidelineTest",
)


def read_json(path: Path):
    return json.loads(path.read_text(encoding="utf-8"))


def package_from_asset_path(asset_path: str) -> str:
    return (asset_path or "").split(".", 1)[0]


def is_zero_ref_character_asset(row: dict) -> bool:
    return (
        int(row.get("referencer_count") or 0) == 0
        and int(row.get("text_reference_count") or 0) == 0
        and not bool(row.get("referenced_by_character_data"))
        and row.get("class") != "ObjectRedirector"
    )


def is_zero_ref_world_asset(row: dict) -> bool:
    return (
        int(row.get("referencer_count") or 0) == 0
        and int(row.get("text_reference_count") or 0) == 0
        and row.get("class") in {"StaticMesh", "MaterialInstanceConstant", "Texture2D"}
    )


def world_group_for_package(package: str) -> str | None:
    prefix = "/Game/World/"
    if not package.startswith(prefix):
        return None
    first = package[len(prefix) :].split("/", 1)[0]
    return WORLD_GROUP_ROOTS.get(first)


def add_group(groups: dict, key: str, description: str, rows: list[dict], note: str):
    packages = []
    assets = []
    for row in rows:
        asset = row.get("asset") or ""
        package = row.get("package") or package_from_asset_path(asset)
        if not package or package in packages:
            continue
        packages.append(package)
        assets.append(
            {
                "package": package,
                "asset": asset,
                "class": row.get("class", ""),
                "name": row.get("name", ""),
            }
        )
    groups[key] = {
        "description": description,
        "status": "candidate_exact_audit_required",
        "note": note,
        "package_count": len(packages),
        "packages": packages,
        "assets": assets,
    }


def build_runtime_groups(character_audit: dict, world_audit: list[dict]) -> dict:
    groups: dict[str, dict] = {}

    character_candidates = [
        row for row in character_audit.get("assets", []) if is_zero_ref_character_asset(row)
    ]
    hero1_rows = [
        row
        for row in character_candidates
        if row.get("hero_id") == "Hero_1"
        and "/FriendSlopRaw/" not in (row.get("asset") or "")
    ]
    other_hero_rows = [
        row
        for row in character_candidates
        if row.get("hero_id") and row.get("hero_id") != "Hero_1"
    ]

    add_group(
        groups,
        "hero1_processed_variant_candidates",
        "Old Hero 1 processed/animated/toon/QuadRetro assets superseded by raw FriendSlop Hero 1 rows.",
        hero1_rows,
        "Start here because Hero_1_Chad and Hero_1_Chad_DemoSkin now point at FriendSlopRaw.",
    )
    add_group(
        groups,
        "other_hero_zero_ref_candidates",
        "Other hero assets with zero package refs, zero text refs, and no CharacterVisuals/Heroes data reference.",
        other_hero_rows,
        "Second-wave cleanup after exact proof; do not delete whole hero folders.",
    )

    world_rows_by_group: dict[str, list[dict]] = {}
    for row in world_audit:
        if not is_zero_ref_world_asset(row):
            continue
        group = world_group_for_package(row.get("package", ""))
        if not group:
            continue
        world_rows_by_group.setdefault(group, []).append(row)

    for group, rows in sorted(world_rows_by_group.items()):
        add_group(
            groups,
            group,
            f"First-pass zero-reference world assets in {group.replace('world_', '').replace('_candidates', '')}.",
            rows,
            "World audit is discovery only; run exact wide text and binary package proof before deletion.",
        )

    return groups


def generated_run_rows(runs_root: Path) -> list[dict]:
    if not runs_root.exists():
        return []
    rows = []
    for child in sorted(runs_root.iterdir(), key=lambda p: p.name.lower()):
        if not child.is_dir():
            continue
        action = "keep_review"
        reason = "Current or archived provenance; delete only after import provenance is settled."
        if any(fragment.lower() in child.name.lower() for fragment in GENERATED_DELETE_NAME_FRAGMENTS):
            action = "delete_after_summary"
            reason = "Generated retry, smoke, or guideline output is not durable source once summarized."
        rows.append(
            {
                "path": str(child),
                "name": child.name,
                "last_write_utc": datetime.fromtimestamp(child.stat().st_mtime, timezone.utc).isoformat(),
                "recommended_action": action,
                "reason": reason,
            }
        )
    return rows


def write_package_lists(output: Path, groups: dict) -> str:
    all_packages = []
    for key, group in groups.items():
        package_path = output.with_name(f"{output.stem}_{key}_packages.txt")
        package_path.write_text("\n".join(group["packages"]) + ("\n" if group["packages"] else ""), encoding="utf-8")
        group["package_list_path"] = str(package_path)
        all_packages.extend(group["packages"])
    all_package_path = output.with_name(f"{output.stem}_all_runtime_candidates_packages.txt")
    unique_packages = sorted(dict.fromkeys(all_packages))
    all_package_path.write_text("\n".join(unique_packages) + ("\n" if unique_packages else ""), encoding="utf-8")
    return str(all_package_path)


def main():
    parser = argparse.ArgumentParser(description="Build T66 model cleanup candidate manifest.")
    parser.add_argument("--character-audit", type=Path, default=DEFAULT_CHARACTER_AUDIT)
    parser.add_argument("--world-audit", type=Path, default=DEFAULT_WORLD_AUDIT)
    parser.add_argument("--runs-root", type=Path, default=REPO_ROOT / "Model Generation" / "Runs" / "Pixal3D")
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    args = parser.parse_args()

    character_audit = read_json(args.character_audit)
    world_audit = read_json(args.world_audit)
    groups = build_runtime_groups(character_audit, world_audit)

    output = args.output
    if not output.is_absolute():
        output = REPO_ROOT / output
    output.parent.mkdir(parents=True, exist_ok=True)
    all_package_list_path = write_package_lists(output, groups)

    report = {
        "generated_utc": datetime.now(timezone.utc).isoformat(),
        "project_root": str(REPO_ROOT),
        "inputs": {
            "character_audit": str(args.character_audit),
            "world_audit": str(args.world_audit),
            "runs_root": str(args.runs_root),
        },
        "all_runtime_package_list_path": all_package_list_path,
        "runtime_groups": groups,
        "generated_run_candidates": generated_run_rows(args.runs_root),
        "rules": [
            "Runtime Content packages require exact external package, text, and binary proof before deletion.",
            "Generated run folders are not runtime dependencies; preserve durable summary/provenance before deleting raw outputs.",
            "Do not delete whole hero folders unless the current audit marks them as whole-folder candidates.",
        ],
    }
    output.write_text(json.dumps(report, indent=2), encoding="utf-8")
    print(f"Wrote model cleanup candidate manifest: {output}")


if __name__ == "__main__":
    main()
