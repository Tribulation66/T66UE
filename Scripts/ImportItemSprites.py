"""
Import the live item sprite set from SourceAssets/ItemSprites into /Game/Items/Sprites.

This script:
1. Reads live item IDs from Content/Data/Items.csv
2. Imports black/red/yellow/white PNGs for each live item
3. Applies UI/pixel-friendly texture settings
4. Reloads DT_Items from Content/Data/Items.csv
5. Deletes obsolete legacy single-icon item sprite assets

Run with:
  UnrealEditor.exe <uproject> -ExecutePythonScript="C:/UE/T66/Scripts/ImportItemSprites.py"
"""

import csv
import os
import unreal


RARITIES = ("black", "red", "yellow", "white")
DEST_DIR = "/Game/Items/Sprites"


def ensure_directory(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def get_project_dir():
    return unreal.SystemLibrary.get_project_directory().replace("\\", "/").rstrip("/")


def get_import_dir(project_dir):
    candidates = [
        os.path.normpath(os.path.join(project_dir, "SourceAssets", "ItemSprites")),
        os.path.normpath(os.path.join(project_dir, "SourceAssets", "Import")),
    ]
    for candidate in candidates:
        if os.path.isdir(candidate):
            return candidate
    return candidates[0]


def get_live_item_ids(csv_path):
    item_ids = []
    with open(csv_path, "r", encoding="utf-8", newline="") as handle:
        for row in csv.DictReader(handle):
            item_id = (row.get("ItemID") or "").strip()
            if item_id:
                item_ids.append(item_id)
    return item_ids


def desired_asset_names(item_ids):
    return [f"{item_id}_{rarity}" for item_id in item_ids for rarity in RARITIES]


def import_texture(source_path, dest_dir, dest_name):
    task = unreal.AssetImportTask()
    task.automated = True
    task.save = False
    task.replace_existing = True
    task.replace_existing_settings = True
    task.filename = source_path
    task.destination_path = dest_dir
    task.destination_name = dest_name

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

    asset_path = f"{dest_dir}/{dest_name}"
    asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    if not asset or not isinstance(asset, unreal.Texture2D):
        raise RuntimeError(f"Texture import failed: {asset_path}")

    try:
        asset.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_EDITOR_ICON)
    except Exception:
        pass
    try:
        asset.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_UI)
    except Exception:
        pass
    try:
        asset.set_editor_property("mip_gen_settings", unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
    except Exception:
        pass
    try:
        asset.set_editor_property("filter", unreal.TextureFilter.TF_NEAREST)
    except Exception:
        pass
    try:
        asset.set_editor_property("never_stream", True)
    except Exception:
        pass

    asset.set_editor_property("srgb", True)
    unreal.EditorAssetLibrary.save_asset(asset_path)


def reload_items_data_table(project_dir):
    dt_path = "/Game/Data/DT_Items"
    dt = unreal.EditorAssetLibrary.load_asset(dt_path)
    if not dt:
        raise RuntimeError(f"DT_Items not found at {dt_path}")

    csv_path = os.path.normpath(os.path.join(project_dir, "Content", "Data", "Items.csv"))
    if not os.path.exists(csv_path):
        raise RuntimeError(f"Items.csv not found at {csv_path}")

    success = unreal.DataTableFunctionLibrary.fill_data_table_from_csv_file(dt, csv_path)
    if not success:
        raise RuntimeError("Failed to reload DT_Items from CSV.")

    unreal.EditorAssetLibrary.save_asset(dt_path)
    unreal.log("[ImportItemSprites] Reloaded /Game/Data/DT_Items")


def delete_obsolete_assets(desired_names):
    existing_paths = unreal.EditorAssetLibrary.list_assets(DEST_DIR, recursive=False, include_folder=False) or []
    desired_paths = {f"{DEST_DIR}/{name}" for name in desired_names}
    deleted = []
    for asset_path in existing_paths:
        package_path = asset_path.split(".", 1)[0]
        if package_path not in desired_paths:
            if unreal.EditorAssetLibrary.delete_asset(asset_path):
                deleted.append(asset_path)
            else:
                unreal.log_warning(f"[ImportItemSprites] Failed to delete obsolete asset: {asset_path}")
    return deleted


def main():
    unreal.log("=== ImportItemSprites ===")
    ensure_directory(DEST_DIR)

    project_dir = get_project_dir()
    csv_path = os.path.normpath(os.path.join(project_dir, "Content", "Data", "Items.csv"))
    import_dir = get_import_dir(project_dir)

    if not os.path.exists(csv_path):
        raise RuntimeError(f"Items.csv not found: {csv_path}")
    if not os.path.isdir(import_dir):
        raise RuntimeError(f"Import directory not found: {import_dir}")

    item_ids = get_live_item_ids(csv_path)
    if not item_ids:
        raise RuntimeError("No live item IDs found in Items.csv")

    desired_names = desired_asset_names(item_ids)
    missing_sources = []
    for asset_name in desired_names:
        source_path = os.path.join(import_dir, f"{asset_name}.png")
        if not os.path.exists(source_path):
            missing_sources.append(source_path)

    if missing_sources:
        preview = "\n".join(missing_sources[:10])
        suffix = "" if len(missing_sources) <= 10 else f"\n...and {len(missing_sources) - 10} more"
        raise RuntimeError(f"Missing source item sprites:\n{preview}{suffix}")

    for asset_name in desired_names:
        source_path = os.path.join(import_dir, f"{asset_name}.png")
        import_texture(source_path, DEST_DIR, asset_name)

    reload_items_data_table(project_dir)
    deleted = delete_obsolete_assets(desired_names)
    unreal.log(f"[ImportItemSprites] Imported {len(desired_names)} textures into {DEST_DIR}")
    unreal.log(f"[ImportItemSprites] Deleted {len(deleted)} obsolete item sprite assets")
    unreal.log("=== ImportItemSprites DONE ===")


if __name__ == "__main__":
    main()
