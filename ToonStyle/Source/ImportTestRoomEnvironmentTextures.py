import json
import os
from pathlib import Path

import unreal


ROOT = Path(r"C:\UE\T66")
WALL_SOURCE = ROOT / "SourceAssets" / "ToonStyle" / "ImageGen" / "Phase1A" / "WallTexture" / "wall_stone_v01.png"
FLOOR_SOURCE = ROOT / "SourceAssets" / "ToonStyle" / "ImageGen" / "Phase1A" / "FloorTexture" / "floor_stone_v01.png"
TEXTURE_DEST = "/Game/ToonStyle/TestAssets/Environment/Textures"
MATERIAL_DEST = "/Game/ToonStyle/TestAssets/Environment/Materials"
PARENT_MATERIAL = "/Game/Materials/M_GLB_Unlit"
VERIFY_PATH = ROOT / "Saved" / "Codex" / "ToonStyle" / "Phase1A2" / "TestRoomEnvironmentTextures.json"


def ensure_dir(path: str) -> None:
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def import_texture(asset_tools, source: Path, texture_name: str):
    if not source.exists():
        raise RuntimeError(f"Missing source texture {source}")

    task = unreal.AssetImportTask()
    task.filename = str(source)
    task.destination_path = TEXTURE_DEST
    task.destination_name = texture_name
    task.automated = True
    task.replace_existing = True
    task.save = True
    asset_tools.import_asset_tasks([task])

    texture_path = f"{TEXTURE_DEST}/{texture_name}"
    texture = unreal.load_asset(texture_path)
    if not texture:
        raise RuntimeError(f"Texture import failed: {texture_path}")

    try:
        texture.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_WORLD)
    except Exception:
        pass
    try:
        texture.set_editor_property("srgb", True)
    except Exception:
        pass
    unreal.EditorAssetLibrary.save_loaded_asset(texture)
    return texture_path, texture


def create_material(asset_tools, parent, material_name: str, texture):
    material_path = f"{MATERIAL_DEST}/{material_name}"
    material = unreal.load_asset(material_path)
    if not material:
        factory = unreal.MaterialInstanceConstantFactoryNew()
        material = asset_tools.create_asset(material_name, MATERIAL_DEST, unreal.MaterialInstanceConstant, factory)
    if not material:
        raise RuntimeError(f"Material instance creation failed: {material_path}")

    material.set_editor_property("parent", parent)
    unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(material, "DiffuseColorMap", texture)
    unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(material, "BaseColorTexture", texture)
    unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(material, "Brightness", 1.0)
    unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(
        material,
        "Tint",
        unreal.LinearColor(1.0, 1.0, 1.0, 1.0),
    )
    unreal.EditorAssetLibrary.save_loaded_asset(material)
    return material_path


def main():
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    parent = unreal.load_asset(PARENT_MATERIAL)
    if not parent:
        raise RuntimeError(f"Missing parent material {PARENT_MATERIAL}")

    ensure_dir(TEXTURE_DEST)
    ensure_dir(MATERIAL_DEST)

    wall_texture_path, wall_texture = import_texture(asset_tools, WALL_SOURCE, "T_TestRoom_Wall")
    floor_texture_path, floor_texture = import_texture(asset_tools, FLOOR_SOURCE, "T_TestRoom_Floor")
    wall_material_path = create_material(asset_tools, parent, "MI_TestRoom_Wall", wall_texture)
    floor_material_path = create_material(asset_tools, parent, "MI_TestRoom_Floor", floor_texture)

    result = {
        "wall_source": str(WALL_SOURCE),
        "floor_source": str(FLOOR_SOURCE),
        "wall_texture": wall_texture_path,
        "floor_texture": floor_texture_path,
        "wall_material": wall_material_path,
        "floor_material": floor_material_path,
        "parent_material": PARENT_MATERIAL,
    }
    VERIFY_PATH.parent.mkdir(parents=True, exist_ok=True)
    VERIFY_PATH.write_text(json.dumps(result, indent=2), encoding="utf-8")
    unreal.log(f"[ImportTestRoomEnvironmentTextures] Imported wall/floor assets: {VERIFY_PATH}")


main()

if os.environ.get("T66_QUIT_EDITOR_AFTER_SCRIPT", "").strip() == "1":
    unreal.SystemLibrary.quit_editor()
