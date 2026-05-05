"""
Audit hero model/sprite assets and character data cleanup candidates.

Read-only by default. It combines DataTable CSV references, Unreal package
referencers, and source/config/data/script text references so cleanup decisions
do not depend on filenames alone.
"""

import csv
import json
import os
import re
from pathlib import Path

import unreal


PROJECT_ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
OUTPUT_PATH = PROJECT_ROOT / "Saved" / "Audits" / "CharacterModelDataAudit.json"
CHARACTER_VISUALS_CSV = PROJECT_ROOT / "Content" / "Data" / "CharacterVisuals.csv"
HEROES_CSV = PROJECT_ROOT / "Content" / "Data" / "Heroes.csv"
AUDIT_ROOTS = (
    "/Game/Characters/Heroes",
    "/Game/UI/Sprites/Heroes",
)
TEXT_SCAN_DIRS = ("Source", "Config", "Content/Data", "Scripts")
TEXT_EXTENSIONS = {".cpp", ".h", ".hpp", ".inl", ".py", ".ini", ".csv", ".json", ".txt", ".md"}
ASSET_PATH_RE = re.compile(r"/Game/[A-Za-z0-9_./]+")


def asset_path_to_package(asset_path):
    return (asset_path or "").split(".", 1)[0]


def read_csv_rows(path):
    if not path.exists():
        return []
    with path.open("r", encoding="utf-8-sig", newline="") as handle:
        return list(csv.DictReader(handle))


def collect_text_blob():
    chunks = []
    for rel_dir in TEXT_SCAN_DIRS:
        root = PROJECT_ROOT / rel_dir
        if not root.exists():
            continue
        for path in root.rglob("*"):
            if not path.is_file() or path.suffix.lower() not in TEXT_EXTENSIONS:
                continue
            try:
                chunks.append(path.read_text(encoding="utf-8", errors="ignore"))
            except Exception:
                continue
    return "\n".join(chunks)


def collect_csv_asset_refs(rows):
    refs = set()
    for row in rows:
        for value in row.values():
            if not value:
                continue
            for match in ASSET_PATH_RE.findall(str(value)):
                refs.add(asset_path_to_package(match.strip().strip('"')))
    return refs


def load_asset_class(asset_path):
    asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    return asset.get_class().get_name() if asset else "Unknown"


def get_referencers(asset_path):
    refs = unreal.EditorAssetLibrary.find_package_referencers_for_asset(asset_path, True)
    return sorted(str(ref) for ref in refs)


def list_assets():
    assets = []
    for root in AUDIT_ROOTS:
        assets.extend(unreal.EditorAssetLibrary.list_assets(root, recursive=True, include_folder=False) or [])
    return sorted(set(assets))


def hero_id_from_game_path(asset_path):
    parts = asset_path.split("/")
    for part in parts:
        if re.match(r"^Hero_\d+$", part):
            return part
    return ""


def analyze_data_rows(character_rows, hero_rows):
    hero_ids = {row.get("HeroID", "").strip() for row in hero_rows if row.get("HeroID", "").strip()}
    visual_ids = {row.get("---", "").strip() for row in character_rows if row.get("---", "").strip()}

    hero_visual_rows = {}
    for row in character_rows:
        row_id = row.get("---", "").strip()
        if row_id.startswith("Hero_"):
            hero_id = "_".join(row_id.split("_")[:2])
            hero_visual_rows.setdefault(hero_id, []).append(row)

    placeholder_stacy_rows = []
    for hero_id, rows in sorted(hero_visual_rows.items()):
        chad = next((row for row in rows if row.get("---") == f"{hero_id}_Chad"), None)
        stacy = next((row for row in rows if row.get("---") == f"{hero_id}_Stacy"), None)
        if not chad or not stacy:
            continue
        if asset_path_to_package(chad.get("SkeletalMesh", "")) == asset_path_to_package(stacy.get("SkeletalMesh", "")):
            placeholder_stacy_rows.append(
                {
                    "hero_id": hero_id,
                    "stacy_row": stacy.get("---", ""),
                    "shared_mesh": stacy.get("SkeletalMesh", ""),
                }
            )

    return {
        "hero_ids": sorted(hero_ids),
        "hero_visual_ids": sorted(visual_ids),
        "hero_visual_row_counts": {hero_id: len(rows) for hero_id, rows in sorted(hero_visual_rows.items())},
        "placeholder_stacy_rows": placeholder_stacy_rows,
    }


def main():
    text_blob = collect_text_blob()
    character_rows = read_csv_rows(CHARACTER_VISUALS_CSV)
    hero_rows = read_csv_rows(HEROES_CSV)
    data_refs = collect_csv_asset_refs(character_rows) | collect_csv_asset_refs(hero_rows)
    data_analysis = analyze_data_rows(character_rows, hero_rows)
    active_hero_ids = set(data_analysis["hero_ids"])
    visual_row_hero_ids = set(data_analysis["hero_visual_row_counts"].keys())

    assets = []
    hero_folder_summary = {}
    legacy_named_assets = []
    redirectors = []
    missing_data_refs = []

    for ref in sorted(data_refs):
        if ref.startswith(AUDIT_ROOTS) and not unreal.EditorAssetLibrary.does_asset_exist(ref):
            missing_data_refs.append(ref)

    for asset_path in list_assets():
        package_path = asset_path_to_package(asset_path)
        name = package_path.rsplit("/", 1)[-1]
        class_name = load_asset_class(asset_path)
        refs = get_referencers(asset_path)
        text_refs = []
        if package_path in text_blob:
            text_refs.append(package_path)
        if asset_path in text_blob:
            text_refs.append(asset_path)
        hero_id = hero_id_from_game_path(asset_path)
        row = {
            "asset": asset_path,
            "package": package_path,
            "name": name,
            "class": class_name,
            "hero_id": hero_id,
            "referencers": refs,
            "referencer_count": len(refs),
            "referencers_outside_audit_roots": [
                ref for ref in refs if not any(ref.startswith(root) for root in AUDIT_ROOTS)
            ],
            "text_references": sorted(set(text_refs)),
            "text_reference_count": len(set(text_refs)),
            "referenced_by_character_data": package_path in data_refs,
        }
        assets.append(row)

        if "TypeA" in asset_path or "TypeB" in asset_path:
            legacy_named_assets.append(row)
        if class_name == "ObjectRedirector":
            redirectors.append(row)

        if hero_id:
            summary = hero_folder_summary.setdefault(
                hero_id,
                {
                    "asset_count": 0,
                    "has_hero_row": hero_id in active_hero_ids,
                    "has_visual_rows": hero_id in visual_row_hero_ids,
                    "character_data_reference_count": 0,
                    "outside_referencer_count": 0,
                    "text_reference_count": 0,
                },
            )
            summary["asset_count"] += 1
            if row["referenced_by_character_data"]:
                summary["character_data_reference_count"] += 1
            summary["outside_referencer_count"] += len(row["referencers_outside_audit_roots"])
            summary["text_reference_count"] += row["text_reference_count"]

    unused_hero_folders = {
        hero_id: summary
        for hero_id, summary in sorted(hero_folder_summary.items())
        if not summary["has_hero_row"]
        and not summary["has_visual_rows"]
        and summary["character_data_reference_count"] == 0
        and summary["outside_referencer_count"] == 0
    }

    report = {
        "roots": AUDIT_ROOTS,
        "character_visual_rows": len(character_rows),
        "hero_rows": len(hero_rows),
        "data_analysis": data_analysis,
        "missing_data_refs": missing_data_refs,
        "legacy_named_assets": legacy_named_assets,
        "redirectors": redirectors,
        "hero_folder_summary": hero_folder_summary,
        "unused_hero_folder_candidates": unused_hero_folders,
        "assets": assets,
    }

    OUTPUT_PATH.parent.mkdir(parents=True, exist_ok=True)
    OUTPUT_PATH.write_text(json.dumps(report, indent=2), encoding="utf-8")

    unreal.log(
        "[AuditCharacterModelData] "
        f"assets={len(assets)} hero_rows={len(hero_rows)} visual_rows={len(character_rows)} "
        f"missing_data_refs={len(missing_data_refs)} legacy_named_assets={len(legacy_named_assets)} "
        f"redirectors={len(redirectors)} unused_hero_folder_candidates={len(unused_hero_folders)} "
        f"placeholder_stacy_rows={len(data_analysis['placeholder_stacy_rows'])} "
        f"output={OUTPUT_PATH}"
    )

    try:
        unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")
    except Exception as exc:
        unreal.log_warning(f"[AuditCharacterModelData] Failed to request QUIT_EDITOR: {exc}")


main()
