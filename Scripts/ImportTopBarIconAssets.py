import os

import unreal


LOG = "[TopBarIcons]"
DEST_DIR = "/Game/UI/Assets/TopBar"
SOURCE_RELATIVE = ("SourceAssets", "UI", "Dota", "Generated")
TEXTURES = {
    "frontend_topbar_settings_icon.png": "frontend_topbar_settings_icon",
    "frontend_topbar_language_icon.png": "frontend_topbar_language_icon",
    "frontend_topbar_home_icon.png": "frontend_topbar_home_icon",
    "frontend_topbar_achievement_coins_icon.png": "frontend_topbar_achievement_coins_icon",
    "frontend_topbar_power_coupons_icon.png": "frontend_topbar_power_coupons_icon",
    "frontend_topbar_quit_icon.png": "frontend_topbar_quit_icon",
}


def log(message):
    unreal.log(f"{LOG} {message}")


def ensure_directory(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def import_texture(source_path, asset_name):
    task = unreal.AssetImportTask()
    task.automated = True
    task.save = False
    task.replace_existing = True
    task.replace_existing_settings = True
    task.filename = source_path
    task.destination_path = DEST_DIR
    task.destination_name = asset_name

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

    asset_path = f"{DEST_DIR}/{asset_name}"
    texture = unreal.EditorAssetLibrary.load_asset(asset_path)
    if not texture or not isinstance(texture, unreal.Texture2D):
        raise RuntimeError(f"Texture import failed: {asset_path}")

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
        texture.set_editor_property("filter", unreal.TextureFilter.TF_TRILINEAR)
    except Exception:
        pass
    try:
        texture.set_editor_property("never_stream", True)
    except Exception:
        pass

    unreal.EditorAssetLibrary.save_asset(asset_path)
    log(f"Imported {asset_path}")


def main():
    ensure_directory(DEST_DIR)

    project_dir = unreal.SystemLibrary.get_project_directory().replace("\\", "/").rstrip("/")
    source_dir = os.path.join(project_dir, *SOURCE_RELATIVE).replace("\\", "/")
    if not os.path.isdir(source_dir):
        raise RuntimeError(f"Missing source dir: {source_dir}")

    for source_name, asset_name in TEXTURES.items():
        source_path = os.path.join(source_dir, source_name).replace("\\", "/")
        if not os.path.isfile(source_path):
            raise RuntimeError(f"Missing source texture: {source_path}")
        import_texture(source_path, asset_name)

    log("Top bar icon textures imported")


if __name__ == "__main__":
    main()
