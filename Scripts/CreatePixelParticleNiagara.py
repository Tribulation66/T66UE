"""
Create NS_PixelParticle by duplicating the existing VFX_Attack1 Niagara system.

VFX_Attack1 already has working emitters, User.Tint color parameter,
sprite rendering, and particle spawning. The duplicate inherits all of
that configuration and works immediately.

To get the square pixel look: open NS_PixelParticle in the Niagara editor,
find the Sprite Renderer, and swap its texture/material to use T_PixelParticle
(the 4x4 white square created by CreatePixelParticleTexture.py).

Run in-editor: Tools > Execute Python Script > Scripts/CreatePixelParticleNiagara.py
"""

import unreal

SOURCE_PATH = "/Game/VFX/VFX_Attack1"
DEST_DIR = "/Game/VFX"
DEST_NAME = "NS_PixelParticle"
DEST_PATH = f"{DEST_DIR}/{DEST_NAME}"
LOG_PREFIX = "[PixelNiagara]"


def _ensure_dir(game_path):
    try:
        if not unreal.EditorAssetLibrary.does_directory_exist(game_path):
            unreal.EditorAssetLibrary.make_directory(game_path)
    except Exception as e:
        unreal.log_warning(f"{LOG_PREFIX} ensure_dir({game_path}): {e}")


def create_pixel_niagara():
    if not unreal.EditorAssetLibrary.does_asset_exist(SOURCE_PATH):
        unreal.log_error(
            f"{LOG_PREFIX} Source asset not found: {SOURCE_PATH}. "
            "Make sure VFX_Attack1 exists in /Game/VFX/."
        )
        return False

    if unreal.EditorAssetLibrary.does_asset_exist(DEST_PATH):
        unreal.log(f"{LOG_PREFIX} Deleting existing: {DEST_PATH}")
        unreal.EditorAssetLibrary.delete_asset(DEST_PATH)

    _ensure_dir(DEST_DIR)

    result = unreal.EditorAssetLibrary.duplicate_asset(SOURCE_PATH, DEST_PATH)
    if not result:
        unreal.log_error(f"{LOG_PREFIX} Failed to duplicate {SOURCE_PATH} -> {DEST_PATH}")
        return False

    unreal.log(f"{LOG_PREFIX} Duplicated {SOURCE_PATH} -> {DEST_PATH}")

    try:
        unreal.EditorAssetLibrary.save_asset(DEST_PATH)
        unreal.log(f"{LOG_PREFIX} Saved: {DEST_PATH}")
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} Save: {e}")
        return False

    unreal.log(f"{LOG_PREFIX}")
    unreal.log(f"{LOG_PREFIX} NS_PixelParticle is a copy of VFX_Attack1 and works immediately.")
    unreal.log(f"{LOG_PREFIX} To get the square pixel look:")
    unreal.log(f"{LOG_PREFIX}   1. Open {DEST_PATH} in Niagara Editor")
    unreal.log(f"{LOG_PREFIX}   2. Find the Sprite Renderer module")
    unreal.log(f"{LOG_PREFIX}   3. Swap the sprite texture/material to T_PixelParticle")
    unreal.log(f"{LOG_PREFIX}      (the 4x4 white square from CreatePixelParticleTexture.py)")
    unreal.log(f"{LOG_PREFIX}   4. This gives hard-edged square pixels instead of round balls")
    return True


def main():
    unreal.log(f"{'='*60}")
    unreal.log(f"{LOG_PREFIX} CreatePixelParticleNiagara START")
    unreal.log(f"{'='*60}")

    result = create_pixel_niagara()

    unreal.log(f"{'='*60}")
    status = "OK (ready to use)" if result else "FAILED"
    unreal.log(f"{LOG_PREFIX} DONE -- {status}")
    unreal.log(f"{LOG_PREFIX}   Path: {DEST_PATH}")
    unreal.log(f"{'='*60}")


if __name__ == "__main__":
    main()
