"""
Fixup all UI texture assets to use crisp, uncompressed quality settings.

Scans all texture assets under known UI sprite/icon directories, changes their
import settings to eliminate DXT block artifacts and ensure trilinear filtering,
then re-saves them.

Run with (full editor required for re-save):
  UnrealEditor.exe <uproject> -ExecutePythonScript="C:/UE/T66/Scripts/FixupUITextureQuality.py"

Or in-editor: Tools > Execute Python Script > Scripts/FixupUITextureQuality.py
"""

import unreal

LOG = "[UITexFixup]"

SCAN_DIRS = [
    "/Game/UI/Sprites",
    "/Game/UI/Leaderboard",
    "/Game/UI/PartyPicker",
    "/Game/UI/Assets",
    "/Game/UI/MainMenu",
    "/Game/UI/PowerUp",
    "/Game/Items/Sprites",
    "/Game/Idols/Sprites",
    "/Game/ULTS",
]

SKIP_KEYWORDS = []


def log(msg):
    unreal.log(f"{LOG} {msg}")


def log_warn(msg):
    unreal.log_warning(f"{LOG} {msg}")


def fixup_texture(texture, asset_path):
    changed = False

    try:
        cur = texture.get_editor_property("compression_settings")
        if cur != unreal.TextureCompressionSettings.TC_EDITOR_ICON:
            texture.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_EDITOR_ICON)
            changed = True
    except Exception as e:
        log_warn(f"  compression_settings failed on {asset_path}: {e}")

    try:
        cur = texture.get_editor_property("lod_group")
        if cur != unreal.TextureGroup.TEXTUREGROUP_UI:
            texture.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_UI)
            changed = True
    except Exception as e:
        log_warn(f"  lod_group failed on {asset_path}: {e}")

    try:
        cur = texture.get_editor_property("filter")
        if cur != unreal.TextureFilter.TF_TRILINEAR:
            texture.set_editor_property("filter", unreal.TextureFilter.TF_TRILINEAR)
            changed = True
    except Exception as e:
        log_warn(f"  filter failed on {asset_path}: {e}")

    try:
        cur = texture.get_editor_property("never_stream")
        if not cur:
            texture.set_editor_property("never_stream", True)
            changed = True
    except Exception as e:
        log_warn(f"  never_stream failed on {asset_path}: {e}")

    try:
        cur = texture.get_editor_property("srgb")
        if not cur:
            texture.set_editor_property("srgb", True)
            changed = True
    except Exception as e:
        log_warn(f"  srgb failed on {asset_path}: {e}")

    try:
        cur = texture.get_editor_property("mip_gen_settings")
        if cur == unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS:
            texture.set_editor_property("mip_gen_settings", unreal.TextureMipGenSettings.TMGS_FROM_TEXTURE_GROUP)
            changed = True
    except Exception:
        pass

    return changed


def main():
    eal = unreal.EditorAssetLibrary
    registry = unreal.AssetRegistryHelpers.get_asset_registry()

    total_scanned = 0
    total_fixed = 0
    total_skipped = 0
    total_failed = 0

    for scan_dir in SCAN_DIRS:
        if not eal.does_directory_exist(scan_dir):
            log(f"Directory not found, skipping: {scan_dir}")
            continue

        asset_paths = eal.list_assets(scan_dir, recursive=True, include_folder=False)
        log(f"Scanning {scan_dir}: {len(asset_paths)} assets")

        for asset_path in asset_paths:
            clean_path = asset_path.split(".")[0] if "." in asset_path else asset_path

            asset = eal.load_asset(clean_path)
            if not asset or not isinstance(asset, unreal.Texture2D):
                continue

            total_scanned += 1

            try:
                did_change = fixup_texture(asset, clean_path)
                if did_change:
                    eal.save_asset(clean_path)
                    total_fixed += 1
                    log(f"  FIXED: {clean_path}")
                else:
                    total_skipped += 1
            except Exception as e:
                total_failed += 1
                log_warn(f"  FAILED: {clean_path}: {e}")

    log(f"Done. Scanned={total_scanned}, Fixed={total_fixed}, "
        f"AlreadyOK={total_skipped}, Failed={total_failed}")


if __name__ == "__main__":
    main()
