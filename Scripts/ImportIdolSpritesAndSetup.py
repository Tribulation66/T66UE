"""
Import the live rarity-specific idol sprite set into /Game/Idols/Sprites, then reload DT_Idols from Content/Data/Idols.csv.

This script:
1. Reads live idol IDs from Content/Data/Idols.csv
2. Imports Black/Red/Yellow/White PNGs for each live idol
3. Applies UI texture settings
4. Reloads DT_Idols from Content/Data/Idols.csv
5. Deletes obsolete legacy idol sprite assets

Run with:
  UnrealEditor.exe <uproject> -ExecutePythonScript="C:/UE/T66/Scripts/RunImportIdolSpritesAndExit.py"
"""

import csv
import os
import unreal


DEST_ROOT = "/Game/Idols/Sprites"
RARITY_SPECS = [
    ("Black", "black", "BlackIcon"),
    ("Red", "red", "RedIcon"),
    ("Yellow", "yellow", "YellowIcon"),
    ("White", "white", "WhiteIcon"),
]


def ensure_directory(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def get_project_dir():
    return unreal.SystemLibrary.get_project_directory().replace("\\", "/").rstrip("/")


def get_import_dir(project_dir):
    candidates = [
        os.path.normpath(os.path.join(project_dir, "SourceAssets", "IdolSprites")),
        os.path.normpath(os.path.join(project_dir, "SourceAssets", "Import")),
    ]
    for candidate in candidates:
        if os.path.isdir(candidate):
            return candidate
    return candidates[0]


def get_live_idol_ids(csv_path):
    idol_ids = []
    with open(csv_path, "r", encoding="utf-8", newline="") as handle:
        for row in csv.DictReader(handle):
            idol_id = (row.get("IdolID") or "").strip().strip('"')
            if idol_id:
                idol_ids.append(idol_id)
    return idol_ids


def source_path_for(source_dir, idol_id, rarity_folder, rarity_suffix):
    return os.path.join(source_dir, rarity_folder, f"{idol_id}_{rarity_suffix}.png")


def dest_dir_for(rarity_folder):
    return f"{DEST_ROOT}/{rarity_folder}"


def dest_name_for(idol_id, rarity_suffix):
    return f"{idol_id}_{rarity_suffix}"


def desired_asset_entries(idol_ids):
    entries = []
    for idol_id in idol_ids:
        for rarity_folder, rarity_suffix, _csv_column in RARITY_SPECS:
            entries.append(
                {
                    "idol_id": idol_id,
                    "rarity_folder": rarity_folder,
                    "rarity_suffix": rarity_suffix,
                    "dest_dir": dest_dir_for(rarity_folder),
                    "dest_name": dest_name_for(idol_id, rarity_suffix),
                }
            )
    return entries


def import_texture(source_path, dest_dir, dest_name):
    ensure_directory(dest_dir)

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
        asset.set_editor_property("mip_gen_settings", unreal.TextureMipGenSettings.TMGS_FROM_TEXTURE_GROUP)
    except Exception:
        pass
    try:
        asset.set_editor_property("filter", unreal.TextureFilter.TF_TRILINEAR)
    except Exception:
        pass
    try:
        asset.set_editor_property("never_stream", True)
    except Exception:
        pass
    asset.set_editor_property("srgb", True)
    unreal.EditorAssetLibrary.save_asset(asset_path)


def reload_idol_data_table(project_dir):
    dt_path = "/Game/Data/DT_Idols"
    dt = unreal.EditorAssetLibrary.load_asset(dt_path)
    if not dt:
        raise RuntimeError(f"DT_Idols not found at {dt_path}")

    csv_path = os.path.normpath(os.path.join(project_dir, "Content", "Data", "Idols.csv"))
    if not os.path.exists(csv_path):
        raise RuntimeError(f"Idols.csv not found at {csv_path}")

    success = unreal.DataTableFunctionLibrary.fill_data_table_from_csv_file(dt, csv_path)
    if not success:
        raise RuntimeError("Failed to reload DT_Idols from CSV.")

    unreal.EditorAssetLibrary.save_asset(dt_path)
    unreal.log("[ImportIdolSpritesAndSetup] Reloaded /Game/Data/DT_Idols")


def delete_obsolete_assets(desired_entries):
    existing_paths = unreal.EditorAssetLibrary.list_assets(DEST_ROOT, recursive=True, include_folder=False) or []
    desired_paths = {
        f"{entry['dest_dir']}/{entry['dest_name']}"
        for entry in desired_entries
    }

    deleted = []
    for asset_path in existing_paths:
        package_path = asset_path.split(".", 1)[0]
        if package_path not in desired_paths:
            if unreal.EditorAssetLibrary.delete_asset(asset_path):
                deleted.append(asset_path)
            else:
                unreal.log_warning(f"[ImportIdolSpritesAndSetup] Failed to delete obsolete asset: {asset_path}")
    return deleted


def main():
    unreal.log("=== ImportIdolSpritesAndSetup ===")

    project_dir = get_project_dir()
    source_dir = get_import_dir(project_dir)
    csv_path = os.path.normpath(os.path.join(project_dir, "Content", "Data", "Idols.csv"))

    if not os.path.exists(csv_path):
        raise RuntimeError(f"Idols.csv not found: {csv_path}")
    if not os.path.isdir(source_dir):
        raise RuntimeError(f"Source import folder not found: {source_dir}")

    idol_ids = get_live_idol_ids(csv_path)
    if not idol_ids:
        raise RuntimeError("No live idol IDs found in Idols.csv")

    desired_entries = desired_asset_entries(idol_ids)
    missing_sources = []
    for entry in desired_entries:
        source_path = source_path_for(
            source_dir,
            entry["idol_id"],
            entry["rarity_folder"],
            entry["rarity_suffix"],
        )
        if not os.path.exists(source_path):
            missing_sources.append(source_path)

    if missing_sources:
        preview = "\n".join(missing_sources[:10])
        suffix = "" if len(missing_sources) <= 10 else f"\n...and {len(missing_sources) - 10} more"
        raise RuntimeError(f"Missing source idol sprites:\n{preview}{suffix}")

    imported_count = 0

    for entry in desired_entries:
        source_path = source_path_for(
            source_dir,
            entry["idol_id"],
            entry["rarity_folder"],
            entry["rarity_suffix"],
        )
        import_texture(source_path, entry["dest_dir"], entry["dest_name"])
        imported_count += 1

    reload_idol_data_table(project_dir)
    deleted = delete_obsolete_assets(desired_entries)
    unreal.log(f"[ImportIdolSpritesAndSetup] Imported {imported_count} idol rarity textures.")
    unreal.log(f"[ImportIdolSpritesAndSetup] Deleted {len(deleted)} obsolete idol sprite assets.")
    unreal.log("=== ImportIdolSpritesAndSetup DONE ===")


if __name__ == "__main__":
    main()
