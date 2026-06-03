"""
Import the generated tower descent gate texture into /Game/World/Tower/Textures.

Run with:
  UnrealEditor.exe <uproject> -ExecutePythonScript="C:/UE/T66/Scripts/RunImportTowerDescentGateTextureAndExit.py"
"""

import os

import unreal


SOURCE_RELATIVE = os.path.join(
    "SourceAssets",
    "ToonStyle",
    "ImageGen",
    "WeaponIdolGate_20260523",
    "FloorGate",
    "T66_floor_descent_gate_closed_v01.png",
)
DEST_DIR = "/Game/World/Tower/Textures"
DEST_NAME = "T_TowerDescentGate_Closed"


def get_project_dir():
    return unreal.SystemLibrary.get_project_directory().replace("\\", "/").rstrip("/")


def ensure_directory(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def import_texture(source_path):
    if not os.path.exists(source_path):
        raise RuntimeError(f"Tower gate source texture not found: {source_path}")

    ensure_directory(DEST_DIR)

    task = unreal.AssetImportTask()
    task.automated = True
    task.save = False
    task.replace_existing = True
    task.replace_existing_settings = True
    task.filename = source_path
    task.destination_path = DEST_DIR
    task.destination_name = DEST_NAME

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

    asset_path = f"{DEST_DIR}/{DEST_NAME}"
    asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    if not asset or not isinstance(asset, unreal.Texture2D):
        raise RuntimeError(f"Tower gate texture import failed: {asset_path}")

    try:
        asset.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_DEFAULT)
    except Exception:
        pass
    try:
        asset.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_WORLD)
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
    asset.set_editor_property("srgb", True)
    unreal.EditorAssetLibrary.save_asset(asset_path)
    return asset_path


def main():
    unreal.log("=== ImportTowerDescentGateTexture ===")
    source_path = os.path.normpath(os.path.join(get_project_dir(), SOURCE_RELATIVE))
    asset_path = import_texture(source_path)
    unreal.log(f"[ImportTowerDescentGateTexture] Imported {source_path} -> {asset_path}")
    unreal.log("=== ImportTowerDescentGateTexture DONE ===")


if __name__ == "__main__":
    main()
