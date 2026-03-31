"""
Import all idol rarity sprites from SourceAssets/Import into /Game/Idols/Sprites,
then reload DT_Idols from Content/Data/Idols.csv.

Run in-editor:
  py "C:/UE/T66/Scripts/ImportIdolSpritesAndSetup.py"
"""

import os
import re
import unreal


RARITY_TO_FOLDER = {
    "black": "Black",
    "red": "Red",
    "yellow": "Yellow",
    "white": "White",
}


def ensure_directory(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


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


def main():
    unreal.log("=== ImportIdolSpritesAndSetup ===")

    project_dir = unreal.SystemLibrary.get_project_directory().replace("\\", "/").rstrip("/")
    source_dir = os.path.normpath(os.path.join(project_dir, "SourceAssets", "Import"))
    if not os.path.isdir(source_dir):
        raise RuntimeError(f"Source import folder not found: {source_dir}")

    pattern = re.compile(r"^(Idol_.+)_(black|red|yellow|white)\.png$", re.IGNORECASE)
    imported_count = 0

    for entry in sorted(os.listdir(source_dir)):
        match = pattern.match(entry)
        if not match:
            continue

        rarity_key = match.group(2).lower()
        folder_name = RARITY_TO_FOLDER[rarity_key]
        source_path = os.path.join(source_dir, entry)
        dest_dir = f"/Game/Idols/Sprites/{folder_name}"
        dest_name = os.path.splitext(entry)[0]
        import_texture(source_path, dest_dir, dest_name)
        imported_count += 1

    reload_idol_data_table(project_dir)
    unreal.log(f"[ImportIdolSpritesAndSetup] Imported {imported_count} idol rarity textures.")
    unreal.log("=== ImportIdolSpritesAndSetup DONE ===")


if __name__ == "__main__":
    main()
