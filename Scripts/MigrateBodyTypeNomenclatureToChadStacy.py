"""
Rename canonical body-style assets/data from TypeA/TypeB to Chad/Stacy.

This script intentionally leaves Unreal redirectors at the old package paths so
existing editor references can resolve while canonical DataTables point at the
new packages.
"""

import json
import os

import unreal


PROJECT_DIR = unreal.Paths.project_dir()
REPORT_PATH = os.path.join(PROJECT_DIR, "Saved", "BodyTypeNomenclatureMigrationReport.json")
ROOTS = (
    "/Game/Characters/Heroes",
    "/Game/UI/Sprites/Heroes",
)
DATA_TABLE_IMPORTS = (
    ("/Game/Data/DT_CharacterVisuals", os.path.join(PROJECT_DIR, "Content", "Data", "CharacterVisuals.csv")),
    ("/Game/Data/DT_Heroes", os.path.join(PROJECT_DIR, "Content", "Data", "Heroes.csv")),
)


def canonicalize(text):
    return text.replace("TypeA", "Chad").replace("TypeB", "Stacy")


def load_asset_data(asset_path):
    data = unreal.EditorAssetLibrary.find_asset_data(asset_path)
    if not data or not data.is_valid():
        return None
    return data


def build_rename_data():
    rename_data = []
    skipped = []
    seen_targets = set()

    for root in ROOTS:
        for asset_path in unreal.EditorAssetLibrary.list_assets(root, recursive=True, include_folder=False):
            data = load_asset_data(asset_path)
            if not data:
                skipped.append({"asset": asset_path, "reason": "missing_asset_data"})
                continue

            old_package_path = str(data.package_path)
            old_asset_name = str(data.asset_name)
            new_package_path = canonicalize(old_package_path)
            new_asset_name = canonicalize(old_asset_name)

            if old_package_path == new_package_path and old_asset_name == new_asset_name:
                continue

            target_key = f"{new_package_path}/{new_asset_name}"
            if target_key in seen_targets:
                skipped.append({"asset": asset_path, "reason": "duplicate_target", "target": target_key})
                continue
            seen_targets.add(target_key)

            if unreal.EditorAssetLibrary.does_asset_exist(f"{new_package_path}/{new_asset_name}"):
                skipped.append({"asset": asset_path, "reason": "target_exists", "target": target_key})
                continue

            asset = data.get_asset()
            if not asset:
                skipped.append({"asset": asset_path, "reason": "asset_load_failed"})
                continue

            rename_data.append(unreal.AssetRenameData(asset, new_package_path, new_asset_name))

    return rename_data, skipped


def reload_data_tables():
    imported = []
    errors = []
    for dt_path, csv_path in DATA_TABLE_IMPORTS:
        dt = unreal.EditorAssetLibrary.load_asset(dt_path)
        if not dt:
            errors.append(f"Missing DataTable: {dt_path}")
            continue
        if not os.path.exists(csv_path):
            errors.append(f"Missing CSV: {csv_path}")
            continue
        if not unreal.DataTableFunctionLibrary.fill_data_table_from_csv_file(dt, csv_path):
            errors.append(f"Failed to import {csv_path} into {dt_path}")
            continue
        unreal.EditorAssetLibrary.save_loaded_asset(dt)
        imported.append({"data_table": dt_path, "csv": csv_path})
    return imported, errors


def main():
    unreal.log("[BodyTypeMigration] Building asset rename list")
    rename_data, skipped = build_rename_data()
    unreal.log(f"[BodyTypeMigration] Renaming {len(rename_data)} assets")

    renamed = []
    if rename_data:
        if not unreal.AssetToolsHelpers.get_asset_tools().rename_assets(rename_data):
            raise RuntimeError("AssetTools.rename_assets reported failure")
        for item in rename_data:
            old_object_path = item.asset.get_path_name()
            renamed.append(
                {
                    "old_asset_name": item.asset.get_name(),
                    "new_package_path": item.new_package_path,
                    "new_name": item.new_name,
                    "post_rename_object_path": old_object_path,
                }
            )

    imported, errors = reload_data_tables()
    if errors:
        raise RuntimeError("; ".join(errors))

    unreal.EditorAssetLibrary.save_directory("/Game/Characters/Heroes", only_if_is_dirty=True, recursive=True)
    unreal.EditorAssetLibrary.save_directory("/Game/UI/Sprites/Heroes", only_if_is_dirty=True, recursive=True)
    unreal.EditorAssetLibrary.save_directory("/Game/Data", only_if_is_dirty=True, recursive=True)

    report = {
        "renamed_count": len(rename_data),
        "skipped": skipped,
        "data_tables": imported,
    }
    os.makedirs(os.path.dirname(REPORT_PATH), exist_ok=True)
    with open(REPORT_PATH, "w", encoding="utf-8") as handle:
        json.dump(report, handle, indent=2)
    unreal.log(f"[BodyTypeMigration] Wrote {REPORT_PATH}")


if __name__ == "__main__":
    main()
