"""
Import the generated Mini Chadpocalypse sprite sets into /Game/Mini/Sprites.

This script imports:
- SourceAssets/Mini/Heroes/Singles -> /Game/Mini/Sprites/Heroes
- SourceAssets/Mini/Enemies/Singles -> /Game/Mini/Sprites/Enemies
- SourceAssets/Mini/Bosses/Singles -> /Game/Mini/Sprites/Bosses
- SourceAssets/Mini/NPCs/Singles -> /Game/Mini/Sprites/NPCs
- SourceAssets/Mini/Interactables/Singles -> /Game/Mini/Sprites/Interactables
"""

import os
import unreal


IMPORT_GROUPS = (
    ("SourceAssets/Mini/Heroes/Singles", "/Game/Mini/Sprites/Heroes"),
    ("SourceAssets/Mini/Enemies/Singles", "/Game/Mini/Sprites/Enemies"),
    ("SourceAssets/Mini/Bosses/Singles", "/Game/Mini/Sprites/Bosses"),
    ("SourceAssets/Mini/NPCs/Singles", "/Game/Mini/Sprites/NPCs"),
    ("SourceAssets/Mini/Interactables/Singles", "/Game/Mini/Sprites/Interactables"),
)


def ensure_directory(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def get_project_dir():
    return unreal.SystemLibrary.get_project_directory().replace("\\", "/").rstrip("/")


def configure_texture(asset_path):
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
    configure_texture(f"{dest_dir}/{dest_name}")


def sync_group(project_dir, relative_source_dir, dest_dir):
    source_dir = os.path.normpath(os.path.join(project_dir, *relative_source_dir.split("/")))
    if not os.path.isdir(source_dir):
        unreal.log_warning(f"[ImportMiniSprites] Missing source directory: {source_dir}")
        return (0, 0)

    png_names = sorted(
        os.path.splitext(entry)[0]
        for entry in os.listdir(source_dir)
        if entry.lower().endswith(".png")
    )

    imported_count = 0
    for asset_name in png_names:
        import_texture(os.path.join(source_dir, f"{asset_name}.png"), dest_dir, asset_name)
        imported_count += 1

    expected_paths = {f"{dest_dir}/{asset_name}" for asset_name in png_names}
    existing_assets = unreal.EditorAssetLibrary.list_assets(dest_dir, recursive=False, include_folder=False) or []
    deleted_count = 0
    for asset_path in existing_assets:
        package_path = asset_path.split(".", 1)[0]
        if package_path not in expected_paths:
            if unreal.EditorAssetLibrary.delete_asset(asset_path):
                deleted_count += 1
            else:
                unreal.log_warning(f"[ImportMiniSprites] Failed to delete obsolete asset: {asset_path}")

    unreal.log(f"[ImportMiniSprites] Synced {dest_dir}: imported={imported_count}, deleted={deleted_count}")
    return (imported_count, deleted_count)


def main():
    unreal.log("=== ImportMiniSprites ===")

    project_dir = get_project_dir()
    total_imported = 0
    total_deleted = 0
    for relative_source_dir, dest_dir in IMPORT_GROUPS:
        imported_count, deleted_count = sync_group(project_dir, relative_source_dir, dest_dir)
        total_imported += imported_count
        total_deleted += deleted_count

    unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
    unreal.log(f"[ImportMiniSprites] Total imported={total_imported}, deleted={total_deleted}")
    unreal.log("=== ImportMiniSprites DONE ===")


if __name__ == "__main__":
    main()
