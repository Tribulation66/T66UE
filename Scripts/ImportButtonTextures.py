"""
Import generated button texture PNGs into Unreal as UTexture2D assets.

Source: SourceAssets/Images/Buttons/ButtonDark_N.png (etc.)
Dest:   /Game/UI/Assets/ButtonDark_N  (etc.)

Run in Unreal Editor: Tools -> Execute Python Script -> Scripts/ImportButtonTextures.py
"""

import os
import unreal

LOG_PREFIX = "[ButtonTex]"
DEST_DIR = "/Game/UI/Assets"

TEXTURES = [
    "ButtonDark_N",
    "ButtonDark_H",
    "ButtonDark_P",
    "ButtonLight_N",
    "ButtonLight_H",
    "ButtonLight_P",
]


def _project_dir():
    try:
        return unreal.SystemLibrary.get_project_directory()
    except Exception:
        return os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))


def main():
    unreal.log("=== Import Button Textures ===")
    proj = _project_dir()
    src_dir = os.path.join(proj, "SourceAssets", "Images", "Buttons")

    tasks = []
    for name in TEXTURES:
        png_path = os.path.join(src_dir, f"{name}.png")
        if not os.path.isfile(png_path):
            unreal.log_error(f"{LOG_PREFIX} PNG not found: {png_path}")
            continue

        task = unreal.AssetImportTask()
        task.set_editor_property("filename", png_path)
        task.set_editor_property("destination_path", DEST_DIR)
        task.set_editor_property("destination_name", name)
        task.set_editor_property("replace_existing", True)
        task.set_editor_property("automated", True)
        task.set_editor_property("save", True)
        tasks.append(task)
        unreal.log(f"{LOG_PREFIX} Queued: {png_path} -> {DEST_DIR}/{name}")

    if not tasks:
        unreal.log_error(f"{LOG_PREFIX} No textures found. Run GenerateButtonTextures.py first.")
        return

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)

    # Post-import: set texture settings for UI (no mipmaps, UserInterface2D compression)
    for name in TEXTURES:
        asset_path = f"{DEST_DIR}/{name}.{name}"
        tex = unreal.EditorAssetLibrary.load_asset(asset_path)
        if tex and isinstance(tex, unreal.Texture2D):
            try:
                tex.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_EDITOR_ICON)
                tex.set_editor_property("mip_gen_settings", unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
                tex.set_editor_property("srgb", True)
                tex.modify()
                unreal.EditorAssetLibrary.save_asset(asset_path)
                unreal.log(f"{LOG_PREFIX} Configured: {asset_path}")
            except Exception as e:
                unreal.log_warning(f"{LOG_PREFIX} Config {name}: {e}")
        else:
            unreal.log_warning(f"{LOG_PREFIX} Could not load imported texture: {asset_path}")

    unreal.log("=== Import Button Textures DONE ===")


if __name__ == "__main__":
    main()
