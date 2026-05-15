#!/usr/bin/env python3
"""Validate frontend video catalog coverage and local runtime files."""

from __future__ import annotations

import csv
import json
import sys
from pathlib import Path


PROJECT_ROOT = Path(__file__).resolve().parents[2]
CATALOG_PATH = PROJECT_ROOT / "RuntimeDependencies" / "T66" / "Video" / "frontend_videos.json"
CONTENT_MOVIES = PROJECT_ROOT / "Content" / "Movies"
HERO_CSV = PROJECT_ROOT / "Content" / "Data" / "Heroes.csv"
COMPANION_CSV = PROJECT_ROOT / "Content" / "Data" / "Companions.csv"
SKIN_IDS = ["Default", "Beachgoer"]
BODY_TYPES = ["Chad", "Stacy"]


def read_ids(path: Path, key: str) -> list[str]:
    with path.open("r", encoding="utf-8-sig", newline="") as handle:
        return [row[key] for row in csv.DictReader(handle) if row.get(key)]


def check_asset(asset: dict, errors: list[str], label: str) -> None:
    movie = asset.get("movie", "")
    poster = asset.get("poster", "")
    if not movie:
        errors.append(f"{label}: missing movie field")
    elif not (CONTENT_MOVIES / movie).exists():
        errors.append(f"{label}: missing movie Content/Movies/{movie}")
    if not poster:
        errors.append(f"{label}: missing poster field")
    elif not (PROJECT_ROOT / poster).exists():
        errors.append(f"{label}: missing poster {poster}")


def main() -> int:
    catalog = json.loads(CATALOG_PATH.read_text(encoding="utf-8"))
    hero_ids = read_ids(HERO_CSV, "HeroID")
    companion_ids = read_ids(COMPANION_CSV, "CompanionID")

    hero_catalog = catalog.get("heroSelection", {}).get("heroes", {})
    companion_catalog = catalog.get("heroSelection", {}).get("companions", {})
    errors: list[str] = []

    check_asset(catalog.get("mainMenu", {}).get("background", {}), errors, "mainMenu.background")

    for hero_id in hero_ids:
        for skin_id in SKIN_IDS:
            for body_type in BODY_TYPES:
                asset = hero_catalog.get(hero_id, {}).get(skin_id, {}).get(body_type)
                if not isinstance(asset, dict):
                    errors.append(f"hero {hero_id}/{skin_id}/{body_type}: missing catalog entry")
                    continue
                check_asset(asset, errors, f"hero {hero_id}/{skin_id}/{body_type}")

    for companion_id in companion_ids:
        for skin_id in SKIN_IDS:
            asset = companion_catalog.get(companion_id, {}).get(skin_id)
            if not isinstance(asset, dict):
                errors.append(f"companion {companion_id}/{skin_id}: missing catalog entry")
                continue
            check_asset(asset, errors, f"companion {companion_id}/{skin_id}")

    for fallback_key, asset in catalog.get("heroSelection", {}).get("fallbacks", {}).items():
        if not isinstance(asset, dict):
            errors.append(f"fallback {fallback_key}: missing asset object")
            continue
        check_asset(asset, errors, f"fallback {fallback_key}")

    if errors:
        print("frontend video catalog validation failed:")
        for error in errors:
            print(f"- {error}")
        return 1

    print(
        "frontend video catalog validation passed: "
        f"{len(hero_ids) * len(SKIN_IDS) * len(BODY_TYPES)} hero entries, "
        f"{len(companion_ids) * len(SKIN_IDS)} companion entries"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
