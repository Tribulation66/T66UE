"""
Create a tiny solid-white square texture for the retro pixel particle system.

T_PixelParticle -- 4x4 solid white, Nearest filtering, no mipmaps.

Run in-editor: Tools > Execute Python Script > Scripts/CreatePixelParticleTexture.py
"""

import unreal

DEST_DIR = "/Game/VFX"
ASSET_NAME = "T_PixelParticle"
LOG_PREFIX = "[PixelParticle]"


def _ensure_dir(game_path):
    try:
        if not unreal.EditorAssetLibrary.does_directory_exist(game_path):
            unreal.EditorAssetLibrary.make_directory(game_path)
    except Exception as e:
        unreal.log_warning(f"{LOG_PREFIX} ensure_dir({game_path}): {e}")


def _safe_set(obj, prop, value):
    if obj is None:
        return
    try:
        obj.set_editor_property(prop, value)
    except Exception as e:
        unreal.log_warning(f"{LOG_PREFIX} set_editor_property({prop}): {e}")


def create_pixel_texture():
    path = f"{DEST_DIR}/{ASSET_NAME}"

    if unreal.EditorAssetLibrary.does_asset_exist(path):
        unreal.log(f"{LOG_PREFIX} Deleting existing: {path}")
        unreal.EditorAssetLibrary.delete_asset(path)

    _ensure_dir(DEST_DIR)

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    factory = unreal.Texture2DFactoryNew()
    tex = asset_tools.create_asset(ASSET_NAME, DEST_DIR, unreal.Texture2D, factory)
    if not tex:
        unreal.log_error(f"{LOG_PREFIX} Failed to create {ASSET_NAME}")
        return False

    ok = True

    try:
        _safe_set(tex, "filter", unreal.TextureFilter.TF_NEAREST)
        _safe_set(tex, "compression_settings",
                  unreal.TextureCompressionSettings.TC_EDITOR_ICON)
        _safe_set(tex, "mip_gen_settings",
                  unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
        _safe_set(tex, "srgb", False)
        _safe_set(tex, "never_stream", True)
        unreal.log(f"{LOG_PREFIX} Texture settings applied (Nearest, NoMipmaps, sRGB=off)")
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} Texture settings: {e}")
        ok = False

    try:
        unreal.EditorAssetLibrary.save_asset(path)
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} Save: {e}")
        ok = False

    if ok:
        unreal.log(f"{LOG_PREFIX} Created: {path}")
    return ok


def main():
    unreal.log(f"{'='*60}")
    unreal.log(f"{LOG_PREFIX} CreatePixelParticleTexture START")
    unreal.log(f"{'='*60}")

    result = create_pixel_texture()

    unreal.log(f"{'='*60}")
    unreal.log(f"{LOG_PREFIX} DONE -- {'OK' if result else 'FAILED'}")
    unreal.log(f"{LOG_PREFIX}   Path: {DEST_DIR}/{ASSET_NAME}")
    unreal.log(f"{'='*60}")


if __name__ == "__main__":
    main()
