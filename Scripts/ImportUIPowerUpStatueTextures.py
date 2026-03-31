import os

import unreal


LOG = "[POWERUPSTATUES]"
SOURCE_ROOT = ("SourceAssets", "UI", "PowerUp", "Statues", "Generated")
DEST_ROOT = "/Game/UI/PowerUp/Statues"
TEXTURE_FILES = (
    "base",
)
STAT_KEYS = (
    "damage",
    "attack_speed",
    "attack_scale",
    "armor",
    "evasion",
    "luck",
)


def log(message):
    unreal.log(f"{LOG} {message}")


def ensure_directory(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def configure_ui_texture(texture):
    texture.set_editor_property("srgb", True)
    try:
        texture.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_EDITOR_ICON)
    except Exception:
        pass
    try:
        texture.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_UI)
    except Exception:
        pass
    try:
        texture.set_editor_property("mip_gen_settings", unreal.TextureMipGenSettings.TMGS_FROM_TEXTURE_GROUP)
    except Exception:
        pass
    try:
        texture.set_editor_property("filter", unreal.TextureFilter.TF_TRILINEAR)
    except Exception:
        pass
    try:
        texture.set_editor_property("never_stream", True)
    except Exception:
        pass


def import_texture(source_path, dest_dir, asset_name):
    task = unreal.AssetImportTask()
    task.automated = True
    task.save = False
    task.replace_existing = True
    task.replace_existing_settings = True
    task.filename = source_path
    task.destination_path = dest_dir
    task.destination_name = asset_name

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

    asset_path = f"{dest_dir}/{asset_name}"
    texture = unreal.EditorAssetLibrary.load_asset(asset_path)
    if not texture or not isinstance(texture, unreal.Texture2D):
        raise RuntimeError(f"Texture import failed: {asset_path}")

    configure_ui_texture(texture)
    unreal.EditorAssetLibrary.save_asset(asset_path)
    log(f"Imported {asset_path}")


def main():
    ensure_directory(DEST_ROOT)

    project_dir = unreal.SystemLibrary.get_project_directory().replace("\\", "/").rstrip("/")
    source_root = os.path.join(project_dir, *SOURCE_ROOT).replace("\\", "/")
    if not os.path.isdir(source_root):
        raise RuntimeError(f"Missing source dir: {source_root}")

    for stat_key in STAT_KEYS:
        stat_source_dir = os.path.join(source_root, stat_key).replace("\\", "/")
        if not os.path.isdir(stat_source_dir):
            raise RuntimeError(f"Missing source stat dir: {stat_source_dir}")

        stat_dest_dir = f"{DEST_ROOT}/{stat_key}"
        ensure_directory(stat_dest_dir)

        for suffix in TEXTURE_FILES:
            file_name = f"{stat_key}_{suffix}.png"
            source_path = os.path.join(stat_source_dir, file_name).replace("\\", "/")
            if not os.path.isfile(source_path):
                raise RuntimeError(f"Missing source texture: {source_path}")
            import_texture(source_path, stat_dest_dir, f"{stat_key}_{suffix}")

    log("Power Up statue base textures ready")


if __name__ == "__main__":
    main()
