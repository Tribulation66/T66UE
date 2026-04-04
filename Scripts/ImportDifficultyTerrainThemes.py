"""
Import generated difficulty terrain textures into Unreal assets under /Game/World/Terrain/MegabonkThemes.

Run headless:
  UnrealEditor-Cmd.exe T66.uproject -run=pythonscript -script="C:/UE/T66/Scripts/ImportDifficultyTerrainThemes.py"
"""

import os
import unreal


LOG = "[ImportDifficultyTerrainThemes]"
DEST_ROOT = "/Game/World/Terrain/MegabonkThemes"

THEME_IMPORTS = [
    {
        "source_dir": "veryhard_graveyard",
        "dest_dir": f"{DEST_ROOT}/VeryHardGraveyard",
        "suffix": "VeryHardGraveyard",
    },
    {
        "source_dir": "impossible_north_pole",
        "dest_dir": f"{DEST_ROOT}/ImpossibleNorthPole",
        "suffix": "ImpossibleNorthPole",
    },
    {
        "source_dir": "perdition_mars",
        "dest_dir": f"{DEST_ROOT}/PerditionMars",
        "suffix": "PerditionMars",
    },
    {
        "source_dir": "final_hell",
        "dest_dir": f"{DEST_ROOT}/FinalHell",
        "suffix": "FinalHell",
    },
]

ROLE_IMPORTS = [
    ("block", "T_MegabonkBlock"),
    ("slope", "T_MegabonkSlope"),
]


def log(msg):
    unreal.log(f"{LOG} {msg}")


def fail(msg):
    unreal.log_error(f"{LOG} {msg}")


def get_project_dir():
    return unreal.SystemLibrary.get_project_directory().replace("\\", "/").rstrip("/")


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

    asset.set_editor_property("srgb", True)
    try:
        asset.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_World)
    except Exception:
        pass

    unreal.EditorAssetLibrary.save_asset(asset_path)
    return asset_path


def main():
    project_dir = get_project_dir()
    source_root = os.path.join(project_dir, "output", "imagegen", "difficulty_textures").replace("\\", "/")

    log("START")
    for theme in THEME_IMPORTS:
        theme_root = os.path.join(source_root, theme["source_dir"]).replace("\\", "/")
        if not os.path.isdir(theme_root):
            raise RuntimeError(f"Missing theme source folder: {theme_root}")

        for role_name, base_asset_name in ROLE_IMPORTS:
            source_path = os.path.join(theme_root, f"{role_name}.png").replace("\\", "/")
            if not os.path.isfile(source_path):
                raise RuntimeError(f"Missing source texture: {source_path}")

            asset_name = f"{base_asset_name}_{theme['suffix']}"
            asset_path = import_texture(source_path, theme["dest_dir"], asset_name)
            log(f"Imported {source_path} -> {asset_path}")

    log("DONE")


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        fail(str(exc))
        raise
