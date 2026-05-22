#!/usr/bin/env python3
"""Validate frontend video catalog coverage and local runtime files."""

from __future__ import annotations

import csv
import json
from pathlib import Path


PROJECT_ROOT = Path(__file__).resolve().parents[2]
CATALOG_PATH = PROJECT_ROOT / "RuntimeDependencies" / "T66" / "Video" / "frontend_videos.json"
CONTENT_MOVIES = PROJECT_ROOT / "Content" / "Movies"
HERO_CSV = PROJECT_ROOT / "Content" / "Data" / "Heroes.csv"
COMPANION_CSV = PROJECT_ROOT / "Content" / "Data" / "Companions.csv"
DEFAULT_HERO_SKIN_IDS = ["Default"]
AGENT2_HERO_SKIN_IDS = ["Default", "DemoSkin"]
COMPANION_SKIN_IDS = ["Default"]
BODY_TYPES = ["Chad", "Stacy"]


def hero_skin_ids(hero_id: str) -> list[str]:
    if hero_id in {"Hero_1", "Hero_2", "Hero_3", "Hero_4", "Hero_5"}:
        return AGENT2_HERO_SKIN_IDS
    return DEFAULT_HERO_SKIN_IDS


def read_ids(path: Path, key: str) -> list[str]:
    with path.open("r", encoding="utf-8-sig", newline="") as handle:
        return [row[key] for row in csv.DictReader(handle) if row.get(key)]


def check_asset(asset: dict, errors: list[str], label: str) -> None:
    movie = asset.get("movie", "")
    poster = asset.get("poster", "")
    poster_only = bool(asset.get("posterOnly", False))
    if not movie and not poster_only:
        errors.append(f"{label}: missing movie field")
    elif movie and not poster_only and not (CONTENT_MOVIES / movie).exists():
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
        for skin_id in hero_skin_ids(hero_id):
            for body_type in BODY_TYPES:
                asset = hero_catalog.get(hero_id, {}).get(skin_id, {}).get(body_type)
                if not isinstance(asset, dict):
                    errors.append(f"hero {hero_id}/{skin_id}/{body_type}: missing catalog entry")
                    continue
                check_asset(asset, errors, f"hero {hero_id}/{skin_id}/{body_type}")

    for companion_id in companion_ids:
        for skin_id in COMPANION_SKIN_IDS:
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

    hero_entry_count = sum(len(hero_skin_ids(hero_id)) for hero_id in hero_ids) * len(BODY_TYPES)
    companion_entry_count = len(companion_ids) * len(COMPANION_SKIN_IDS)
    print(
        "frontend video catalog validation passed: "
        f"{hero_entry_count} hero entries, {companion_entry_count} companion entries"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())