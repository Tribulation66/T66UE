"""
Import interactable UI sprites from SourceAssets/Import into /Game/UI/Sprites/Interactables.

Run with:
  UnrealEditor-Cmd.exe T66.uproject -run=pythonscript -script="C:/UE/T66/Scripts/ImportInteractableUISprites.py"
"""

import os
import unreal


SPRITES = [
    ("QuickReviveIcon.png", "/Game/UI/Sprites/Interactables", "QuickReviveIcon"),
]


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


def main():
    project_dir = unreal.SystemLibrary.get_project_directory().replace("\\", "/").rstrip("/")
    source_dir = os.path.join(project_dir, "SourceAssets", "Import")
    if not os.path.isdir(source_dir):
        raise RuntimeError(f"Source import folder not found: {source_dir}")

    imported_count = 0
    for source_name, dest_dir, dest_name in SPRITES:
        source_path = os.path.join(source_dir, source_name)
        if not os.path.exists(source_path):
            raise RuntimeError(f"Source sprite not found: {source_path}")
        import_texture(source_path, dest_dir, dest_name)
        imported_count += 1

    unreal.log(f"[ImportInteractableUISprites] Imported {imported_count} sprite(s).")


if __name__ == "__main__":
    main()
