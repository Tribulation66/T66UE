"""
Evaluate exact package-reference audit output for a model cleanup group.

The evaluator separates candidate discovery from deletion clearance. Packages
are approved only when they have no external package referencers, no external
binary token matches, and no blocking runtime text references.
"""

from __future__ import annotations

import argparse
import json
from datetime import datetime, timezone
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
BLOCKING_TEXT_PREFIXES = ("Source/", "Config/", "Content/Data/")
NONBLOCKING_TEXT_PREFIXES = ("Reports/", "Saved/", "Model Generation/", "Scripts/")


def read_json(path: Path):
    return json.loads(path.read_text(encoding="utf-8"))


def normalize_package(value: str) -> str:
    value = (value or "").strip().replace("\\", "/")
    if "." in value:
        value = value.split(".", 1)[0]
    return value.rstrip("/")


def content_file_to_package(path_value: str) -> str | None:
    value = path_value.replace("\\", "/")
    if not value.startswith("Content/"):
        return None
    suffix = Path(value).suffix.lower()
    if suffix not in {".uasset", ".umap"}:
        return None
    no_ext = value[: -len(suffix)]
    return "/Game/" + no_ext[len("Content/") :]


def is_blocking_text_path(path_value: str) -> bool:
    value = path_value.replace("\\", "/")
    if value.startswith(NONBLOCKING_TEXT_PREFIXES):
        return False
    return value.startswith(BLOCKING_TEXT_PREFIXES)


def packages_for_groups(manifest: dict, group_names: list[str]) -> tuple[set[str], list[dict]]:
    package_set: set[str] = set()
    groups = []
    for group_name in group_names:
        group = manifest["runtime_groups"][group_name]
        packages = {normalize_package(package) for package in group["packages"]}
        package_set.update(packages)
        groups.append(
            {
                "name": group_name,
                "description": group.get("description", ""),
                "package_count": len(packages),
            }
        )
    return package_set, groups


def evaluate(manifest: dict, exact_audit: dict, group_names: list[str], report_group_name: str) -> dict:
    package_set, group_rows = packages_for_groups(manifest, group_names)

    package_rows = exact_audit.get("packages", [])
    packages_by_name = {normalize_package(row.get("package", "")): row for row in package_rows}
    external_package_refs: dict[str, list[str]] = {}
    missing_packages = []
    for package in sorted(package_set):
        row = packages_by_name.get(package)
        if not row or not row.get("exists"):
            missing_packages.append(package)
            continue
        refs = row.get("referencers") or []
        outside = [normalize_package(ref) for ref in refs if normalize_package(ref) not in package_set]
        if outside:
            external_package_refs[package] = sorted(set(outside))

    external_binary_refs: dict[str, list[str]] = {}
    internal_binary_refs: dict[str, list[str]] = {}
    for match in exact_audit.get("binary_content", {}).get("matches", []):
        ref_package = content_file_to_package(match.get("path", ""))
        if not ref_package:
            continue
        hits = [normalize_package(hit) for hit in match.get("hits", []) if normalize_package(hit) in package_set]
        if not hits:
            continue
        target = internal_binary_refs if ref_package in package_set else external_binary_refs
        target.setdefault(ref_package, []).extend(hits)

    blocking_text_refs = []
    nonblocking_text_refs = []
    for match in exact_audit.get("text", {}).get("matches", []):
        row = {"path": match.get("path", ""), "hits": match.get("hits", [])}
        if is_blocking_text_path(row["path"]):
            blocking_text_refs.append(row)
        else:
            nonblocking_text_refs.append(row)

    approved_packages = []
    skipped_packages = []
    for package in sorted(package_set):
        reasons = []
        if package in missing_packages:
            reasons.append("package_missing")
        if package in external_package_refs:
            reasons.append("external_package_referencers")
        if any(package in hits for hits in external_binary_refs.values()):
            reasons.append("external_binary_token_match")
        if blocking_text_refs:
            if any(package in [normalize_package(hit) for hit in row.get("hits", [])] for row in blocking_text_refs):
                reasons.append("blocking_runtime_text_match")
        if reasons:
            skipped_packages.append({"package": package, "reasons": sorted(set(reasons))})
        else:
            approved_packages.append(package)

    return {
        "generated_utc": datetime.now(timezone.utc).isoformat(),
        "group": report_group_name,
        "groups": group_rows,
        "source_manifest": manifest.get("inputs", {}),
        "package_count": len(package_set),
        "approved_count": len(approved_packages),
        "skipped_count": len(skipped_packages),
        "approved_packages": approved_packages,
        "skipped_packages": skipped_packages,
        "external_package_refs": external_package_refs,
        "external_binary_refs": {key: sorted(set(value)) for key, value in external_binary_refs.items()},
        "internal_binary_refs": {key: sorted(set(value)) for key, value in internal_binary_refs.items()},
        "blocking_text_refs": blocking_text_refs,
        "nonblocking_text_refs": nonblocking_text_refs,
        "notes": [
            "Internal candidate-set references are allowed for chain deletion.",
            "Reports, generated manifests, and script/import references are recorded but not treated as runtime blockers.",
            "Source, Config, and Content/Data text hits are deletion blockers.",
        ],
    }


def main():
    parser = argparse.ArgumentParser(description="Evaluate exact model cleanup audit output.")
    parser.add_argument("--manifest", type=Path, required=True)
    parser.add_argument("--exact-audit", type=Path, required=True)
    parser.add_argument("--group", required=True)
    parser.add_argument(
        "--groups",
        help="Comma-separated manifest runtime group names. Overrides --group for combined cleanup waves.",
    )
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()

    manifest = read_json(args.manifest)
    exact_audit = read_json(args.exact_audit)
    group_names = [part.strip() for part in (args.groups or args.group).split(",") if part.strip()]
    report = evaluate(manifest, exact_audit, group_names, args.group)

    output = args.output
    if not output.is_absolute():
        output = REPO_ROOT / output
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(report, indent=2), encoding="utf-8")
    print(
        f"Wrote model cleanup gate: {output} "
        f"approved={report['approved_count']} skipped={report['skipped_count']}"
    )


if __name__ == "__main__":
    main()
