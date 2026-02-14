"""
Import HDRI environment texture from SourceAssets for SkyLight cubemap lighting.

Imports the studio HDRI as an HDR Texture2D, then sets properties for sky lighting use.
After running this script, right-click the imported texture in Content Browser and select
"Create Texture Cube" to produce the cubemap that SpawnLightingIfNeeded() expects.

Source: SourceAssets/studio_small_09_2k.exr
Dest:   /Game/Lighting/T_HDRI_Studio (Texture2D, HDR)
        /Game/Lighting/TC_HDRI_Studio (TextureCube — created manually after import)

Run INSIDE Unreal Editor: Tools -> Execute Python Script -> Scripts/ImportHDRI.py
"""

import os
import unreal


def _proj_dir() -> str:
    try:
        return unreal.SystemLibrary.get_project_directory()
    except Exception:
        return os.path.abspath(os.path.join(os.getcwd(), ".."))


def _ensure_dir(game_path: str):
    if not unreal.EditorAssetLibrary.does_directory_exist(game_path):
        unreal.EditorAssetLibrary.make_directory(game_path)


def main():
    proj = _proj_dir()
    src_file = os.path.join(proj, "SourceAssets", "studio_small_09_2k.exr")

    if not os.path.isfile(src_file):
        unreal.log_error(f"[ImportHDRI] HDRI file not found: {src_file}")
        return

    dest_dir = "/Game/Lighting"
    dest_name = "T_HDRI_Studio"
    _ensure_dir(dest_dir)

    unreal.log("=== ImportHDRI: START ===")

    # Import as Texture2D (HDR)
    task = unreal.AssetImportTask()
    task.automated = True
    task.save = True
    task.replace_existing = True
    task.replace_existing_settings = True
    task.filename = src_file
    task.destination_path = dest_dir
    task.destination_name = dest_name

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

    paths = list(task.imported_object_paths or [])
    unreal.log(f"[ImportHDRI] Imported {len(paths)} asset(s): {paths}")

    # Configure texture for HDR sky lighting use
    asset_path = f"{dest_dir}/{dest_name}"
    tex = unreal.EditorAssetLibrary.load_asset(asset_path)
    if tex and isinstance(tex, unreal.Texture2D):
        tex.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_HDR_COMPRESSED)
        tex.set_editor_property("srgb", False)
        tex.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_SKYBOX)
        tex.set_editor_property("never_stream", True)
        unreal.EditorAssetLibrary.save_asset(asset_path)
        unreal.log(f"[ImportHDRI] Configured {asset_path}: HDR Compressed, Linear, Skybox LOD group")
    else:
        unreal.log_warning(f"[ImportHDRI] Could not load imported texture at {asset_path}")

    unreal.log("=== ImportHDRI: DONE ===")
    unreal.log("")
    unreal.log("NEXT STEP: In Content Browser, right-click T_HDRI_Studio -> Create Texture Cube")
    unreal.log("           Name it TC_HDRI_Studio and save it in /Game/Lighting/")
    unreal.log("           The game code will automatically pick it up as the SkyLight cubemap.")


if __name__ == "__main__":
    main()
