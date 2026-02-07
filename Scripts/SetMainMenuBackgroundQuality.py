"""
Set main menu background texture (V3) to HD quality with UI-optimized settings.

Applied settings:
- Compression: UserInterface2D (RGBA) – best for UMG/Slate images.
- Texture Group: UI.
- Mip Gen Settings: NoMipmaps – UI backgrounds shouldn't get mip-blurred.
- Never Stream: True – disable texture streaming for this asset.
- LOD Bias: 0 – no automatic downscale.
- Max texture size 8192 – ensure "Max In-Game" / displayed size is not reduced.

Reimports from SourceAssets/Images/MainMenuBackgroundV3.jpg (or .png).

Run in Unreal Editor: Tools -> Execute Python Script -> Scripts/SetMainMenuBackgroundQuality.py
Or (PowerShell): UnrealEditor-Cmd ... -run=pythonscript -script="...\SetMainMenuBackgroundQuality.py" -unattended -nop4 -nosplash
"""

import os
import unreal


MAIN_MENU_BG_ASSET = "/Game/UI/MainMenu/MainMenuBackgroundV3.MainMenuBackgroundV3"
MAIN_MENU_BG_DIR = "/Game/UI/MainMenu"
MAIN_MENU_BG_NAME = "MainMenuBackgroundV3"
SOURCE_CANDIDATES = [
    "Images/MainMenuBackgroundV3.jpg",
    "Images/MainMenuBackgroundV3.png",
]


def _proj_dir() -> str:
    try:
        return unreal.SystemLibrary.get_project_directory()
    except Exception:
        return os.path.abspath(os.path.join(os.getcwd(), ".."))


def _ensure_dir(game_path: str):
    if not unreal.EditorAssetLibrary.does_directory_exist(game_path):
        unreal.EditorAssetLibrary.make_directory(game_path)


def _apply_ui_background_settings(tex: unreal.Texture2D) -> bool:
    """
    Apply your requested settings for the main menu background.
    Returns True if any change was made.
    """
    if not tex or not isinstance(tex, unreal.Texture2D):
        return False
    changed = False

    # Compression: UserInterface2D (RGBA) – best for UMG images
    try:
        ui2d = getattr(unreal.TextureCompressionSettings, "TC_USERINTERFACE2D", None)
        if ui2d is None:
            ui2d = unreal.TextureCompressionSettings.TC_DEFAULT
        if tex.get_editor_property("compression_settings") != ui2d:
            tex.set_editor_property("compression_settings", ui2d)
            changed = True
    except Exception:
        try:
            if tex.get_editor_property("compression_settings") != unreal.TextureCompressionSettings.TC_DEFAULT:
                tex.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_DEFAULT)
                changed = True
        except Exception:
            pass

    # Texture Group: UI
    try:
        if tex.get_editor_property("lod_group") != unreal.TextureGroup.TEXTUREGROUP_UI:
            tex.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_UI)
            changed = True
    except Exception:
        pass

    # Mip Gen Settings: NoMipmaps (UI backgrounds shouldn't get mip-blurred)
    try:
        if tex.get_editor_property("mip_gen_settings") != unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS:
            tex.set_editor_property("mip_gen_settings", unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
            changed = True
    except Exception:
        pass

    # Disable streaming for this texture (Never Stream = true)
    for prop in ("never_stream", "never_streamable", "disable_streaming"):
        try:
            if hasattr(tex, "get_editor_property") and tex.get_editor_property(prop) != True:
                tex.set_editor_property(prop, True)
                changed = True
            break
        except Exception:
            continue

    # LOD Bias: 0 (no automatic downscale)
    for prop in ("lod_bias", "lod_group_bias"):
        try:
            if hasattr(tex, "get_editor_property"):
                current = tex.get_editor_property(prop)
                if current is not None and current != 0:
                    tex.set_editor_property(prop, 0)
                    changed = True
            break
        except Exception:
            continue

    # Max texture size – ensure not downscaled (8192 allows 4K)
    try:
        max_size = 8192
        if hasattr(tex, "max_texture_size"):
            current = tex.get_editor_property("max_texture_size")
            if current is not None and current < max_size:
                tex.set_editor_property("max_texture_size", max_size)
                changed = True
    except Exception:
        pass

    if changed:
        try:
            if hasattr(tex, "modify"):
                tex.modify()
        except Exception:
            pass
    return changed


def reimport_from_source() -> bool:
    """Reimport main menu background from SourceAssets. Returns True if import succeeded."""
    proj = _proj_dir()
    src_assets = os.path.join(proj, "SourceAssets")
    src_file = None
    for rel in SOURCE_CANDIDATES:
        path = os.path.join(src_assets, rel)
        if os.path.isfile(path):
            src_file = path
            break
    if not src_file:
        unreal.log_warning("[MainMenuBg] No source image found (tried: " + ", ".join(SOURCE_CANDIDATES) + ")")
        return False
    _ensure_dir(MAIN_MENU_BG_DIR)
    task = unreal.AssetImportTask()
    task.automated = True
    task.save = True
    task.replace_existing = True
    task.replace_existing_settings = True
    task.filename = src_file
    task.destination_path = MAIN_MENU_BG_DIR
    task.destination_name = MAIN_MENU_BG_NAME
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    if not task.imported_object_paths:
        unreal.log_warning("[MainMenuBg] Import produced no assets: " + src_file)
        return False
    return True


def main():
    unreal.log("=== SetMainMenuBackgroundQuality (V3) START ===")
    reimport_from_source()
    asset_path = MAIN_MENU_BG_ASSET
    tex = unreal.EditorAssetLibrary.load_asset(asset_path)
    if not tex:
        unreal.log_error("[MainMenuBg] Asset not found: " + asset_path + " – import MainMenuBackgroundV3 into Content/UI/MainMenu/ first.")
        unreal.log("=== SetMainMenuBackgroundQuality: DONE (no asset) ===")
        return
    if _apply_ui_background_settings(tex):
        unreal.EditorAssetLibrary.save_asset(asset_path)
        unreal.log("[MainMenuBg] Applied settings: UserInterface2D compression, UI group, NoMipmaps, Never Stream, LOD Bias 0, max 8192.")
    else:
        unreal.log("[MainMenuBg] Asset already has requested settings.")
    unreal.log("=== SetMainMenuBackgroundQuality: DONE ===")


if __name__ == "__main__":
    main()
