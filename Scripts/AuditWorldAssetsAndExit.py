"""
Audit /Game/World assets and report package referencers.

This is intentionally read-only. It combines Unreal's asset registry package
referencers with a text scan of code/data/script config files so cleanup
decisions do not depend on filenames alone.
"""

import json
import os
from pathlib import Path

import unreal


PROJECT_ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
OUTPUT_PATH = PROJECT_ROOT / "Saved" / "Audits" / "WorldAssetAudit.json"
WORLD_ROOT = "/Game/World"
TEXT_SCAN_DIRS = ["Source", "Config", "Content/Data"]
TEXT_EXTENSIONS = {".cpp", ".h", ".hpp", ".inl", ".py", ".ini", ".csv", ".json", ".txt", ".md"}


def asset_path_to_package(asset_path):
    return asset_path.split(".", 1)[0]


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


def get_referencers(asset_path):
    refs = unreal.EditorAssetLibrary.find_package_referencers_for_asset(asset_path, True)
    return sorted(str(ref) for ref in refs)


def main():
    text_blob = collect_text_blob()
    assets = unreal.EditorAssetLibrary.list_assets(WORLD_ROOT, recursive=True, include_folder=False)
    rows = []
    for asset_path in sorted(assets):
        package_path = asset_path_to_package(asset_path)
        name = package_path.rsplit("/", 1)[-1]
        asset = unreal.EditorAssetLibrary.load_asset(asset_path)
        class_name = asset.get_class().get_name() if asset else "Unknown"
        refs = get_referencers(asset_path)
        text_refs = []
        if package_path in text_blob:
            text_refs.append(package_path)
        if asset_path in text_blob:
            text_refs.append(asset_path)
        if name and name in text_blob and package_path.startswith("/Game/World/Terrain"):
            text_refs.append(name)
        if name.endswith("_UnrealReady"):
            module_name = name[: -len("_UnrealReady")]
            if module_name in text_blob:
                text_refs.append(module_name)
        if name.endswith("_UnrealReady_BaseColor_00"):
            module_name = name[: -len("_UnrealReady_BaseColor_00")]
            if module_name in text_blob:
                text_refs.append(module_name)
        if name.endswith("_UnrealReady_Mat_00"):
            module_name = name[: -len("_UnrealReady_Mat_00")]
            if module_name in text_blob:
                text_refs.append(module_name)
        rows.append(
            {
                "asset": asset_path,
                "package": package_path,
                "name": name,
                "class": class_name,
                "referencers": refs,
                "referencer_count": len(refs),
                "text_references": sorted(set(text_refs)),
                "text_reference_count": len(set(text_refs)),
            }
        )

    OUTPUT_PATH.parent.mkdir(parents=True, exist_ok=True)
    OUTPUT_PATH.write_text(json.dumps(rows, indent=2), encoding="utf-8")

    orphan_rows = [row for row in rows if row["referencer_count"] == 0 and row["text_reference_count"] == 0]
    unreal.log(f"[AuditWorldAssets] assets={len(rows)} orphan_candidates={len(orphan_rows)} output={OUTPUT_PATH}")
    for row in orphan_rows[:100]:
        unreal.log(f"[AuditWorldAssets] ORPHAN {row['class']} {row['asset']}")

    try:
        unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")
        unreal.log("[AuditWorldAssets] QUIT_EDITOR requested")
    except Exception as exc:
        unreal.log_warning(f"[AuditWorldAssets] Failed to request QUIT_EDITOR: {exc}")


main()
