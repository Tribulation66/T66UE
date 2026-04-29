"""
Import weapon sprites into /Game/Weapons/Sprites, then reload DT_Weapons.

Run with:
  UnrealEditor.exe <uproject> -ExecutePythonScript="C:/UE/T66/Scripts/RunImportWeaponSpritesAndExit.py"
"""

import csv
import os

import unreal

import SetupWeaponsDataTable


DEST_ROOT = "/Game/Weapons/Sprites"
RARITY_FOLDERS = {
    "Grey": "Grey",
    "Black": "Black",
    "Red": "Red",
    "Yellow": "Yellow",
    "White": "White",
}


def ensure_directory(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def get_project_dir():
    return unreal.SystemLibrary.get_project_directory().replace("\\", "/").rstrip("/")


def get_source_dir(project_dir):
    return os.path.normpath(os.path.join(project_dir, "SourceAssets", "WeaponSprites"))


def get_weapon_rows(project_dir):
    csv_path = os.path.normpath(os.path.join(project_dir, "Content", "Data", "Weapons.csv"))
    if not os.path.exists(csv_path):
        raise RuntimeError(f"Weapons.csv not found: {csv_path}")
    with open(csv_path, "r", encoding="utf-8-sig", newline="") as handle:
        return [row for row in csv.DictReader(handle) if (row.get("WeaponID") or "").strip()]


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
        asset.set_editor_property("filter", unreal.TextureFilter.TF_NEAREST)
    except Exception:
        pass
    try:
        asset.set_editor_property("never_stream", True)
    except Exception:
        pass
    asset.set_editor_property("srgb", True)
    unreal.EditorAssetLibrary.save_asset(asset_path)


def delete_obsolete_assets(desired_paths):
    existing_paths = unreal.EditorAssetLibrary.list_assets(DEST_ROOT, recursive=True, include_folder=False) or []
    deleted = []
    for asset_path in existing_paths:
        package_path = asset_path.split(".", 1)[0]
        if package_path not in desired_paths:
            if unreal.EditorAssetLibrary.delete_asset(asset_path):
                deleted.append(asset_path)
            else:
                unreal.log_warning(f"[ImportWeaponSpritesAndSetup] Failed to delete obsolete asset: {asset_path}")
    return deleted


def main():
    unreal.log("=== ImportWeaponSpritesAndSetup ===")

    project_dir = get_project_dir()
    source_dir = get_source_dir(project_dir)
    if not os.path.isdir(source_dir):
        raise RuntimeError(f"Source weapon sprite folder not found: {source_dir}")

    # Regenerate/reload Weapons.csv first so icon paths are current before import validation.
    SetupWeaponsDataTable.main()
    rows = get_weapon_rows(project_dir)
    if not rows:
        raise RuntimeError("No weapon rows found in Weapons.csv")

    missing_sources = []
    for row in rows:
        weapon_id = row.get("WeaponID", "").strip()
        source_path = os.path.join(source_dir, f"{weapon_id}.png")
        if not os.path.exists(source_path):
            missing_sources.append(source_path)

    if missing_sources:
        preview = "\n".join(missing_sources[:10])
        suffix = "" if len(missing_sources) <= 10 else f"\n...and {len(missing_sources) - 10} more"
        raise RuntimeError(f"Missing source weapon sprites:\n{preview}{suffix}")

    imported_count = 0
    desired_paths = set()
    for row in rows:
        weapon_id = row["WeaponID"].strip()
        rarity = row.get("Rarity", "Grey").strip() or "Grey"
        folder_name = RARITY_FOLDERS.get(rarity, "Grey")
        dest_dir = f"{DEST_ROOT}/{folder_name}"
        desired_paths.add(f"{dest_dir}/{weapon_id}")
        import_texture(os.path.join(source_dir, f"{weapon_id}.png"), dest_dir, weapon_id)
        imported_count += 1

    # Reload again after texture import so DT_Weapons stores references to live assets.
    SetupWeaponsDataTable.main()
    deleted = delete_obsolete_assets(desired_paths)
    unreal.log(f"[ImportWeaponSpritesAndSetup] Imported {imported_count} weapon textures.")
    unreal.log(f"[ImportWeaponSpritesAndSetup] Deleted {len(deleted)} obsolete weapon sprite assets.")
    unreal.log("=== ImportWeaponSpritesAndSetup DONE ===")


if __name__ == "__main__":
    main()
