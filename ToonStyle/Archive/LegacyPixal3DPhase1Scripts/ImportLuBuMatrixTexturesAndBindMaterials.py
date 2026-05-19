import json
from pathlib import Path
import unreal

ROOT = Path(r"C:\UE\T66")
TEXTURE_SOURCE_ROOT = ROOT / "SourceAssets" / "ToonStyle" / "Pixal3D" / "Phase1A" / "LuBu_Matrix" / "ExtractedTextures"
TEXTURE_DEST = "/Game/ToonStyle/TestAssets/LuBu_Matrix/Textures"
MATERIAL_DEST = "/Game/ToonStyle/TestAssets/LuBu_Matrix/Materials"
PARENT_MATERIAL = "/Game/Materials/M_GLB_Unlit"

ENTRIES = [
    ("lubu_r1024_t2048_default_texture.png", "T_LuBu_R1024_T2048_Default", "MI_LuBu_R1024_T2048_Default", "/Game/ToonStyle/TestAssets/LuBu_Matrix/SM_LuBu_R1024_T2048_Default"),
    ("lubu_r1536_t2048_default_texture.png", "T_LuBu_R1536_T2048_Default", "MI_LuBu_R1536_T2048_Default", "/Game/ToonStyle/TestAssets/LuBu_Matrix/SM_LuBu_R1536_T2048_Default"),
    ("lubu_r1024_t4096_default_texture.png", "T_LuBu_R1024_T4096_Default", "MI_LuBu_R1024_T4096_Default", "/Game/ToonStyle/TestAssets/LuBu_Matrix/SM_LuBu_R1024_T4096_Default"),
    ("lubu_r1536_t4096_default_texture.png", "T_LuBu_R1536_T4096_Default", "MI_LuBu_R1536_T4096_Default", "/Game/ToonStyle/TestAssets/LuBu_Matrix/SM_LuBu_R1536_T4096_Default"),
    ("lubu_r1024_t2048_high_texture.png", "T_LuBu_R1024_T2048_High", "MI_LuBu_R1024_T2048_High", "/Game/ToonStyle/TestAssets/LuBu_Matrix/SM_LuBu_R1024_T2048_High"),
    ("lubu_r1536_t4096_high_texture.png", "T_LuBu_R1536_T4096_High", "MI_LuBu_R1536_T4096_High", "/Game/ToonStyle/TestAssets/LuBu_Matrix/SM_LuBu_R1536_T4096_High"),
]

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
parent = unreal.load_asset(PARENT_MATERIAL)
if not parent:
    raise RuntimeError(f"Missing parent material {PARENT_MATERIAL}")

for dest in (TEXTURE_DEST, MATERIAL_DEST):
    if not unreal.EditorAssetLibrary.does_directory_exist(dest):
        unreal.EditorAssetLibrary.make_directory(dest)

results = []
for source_name, texture_name, material_name, mesh_path in ENTRIES:
    source_path = TEXTURE_SOURCE_ROOT / source_name
    if not source_path.exists():
        raise RuntimeError(f"Missing extracted texture {source_path}")

    texture_asset_path = f"{TEXTURE_DEST}/{texture_name}"
    task = unreal.AssetImportTask()
    task.filename = str(source_path)
    task.destination_path = TEXTURE_DEST
    task.destination_name = texture_name
    task.automated = True
    task.replace_existing = True
    task.save = True
    asset_tools.import_asset_tasks([task])

    texture = unreal.load_asset(texture_asset_path)
    if not texture:
        raise RuntimeError(f"Texture import failed: {texture_asset_path}")
    try:
        texture.set_editor_property("srgb", True)
    except Exception:
        pass
    unreal.EditorAssetLibrary.save_loaded_asset(texture)

    material_asset_path = f"{MATERIAL_DEST}/{material_name}"
    material = unreal.load_asset(material_asset_path)
    if not material:
        factory = unreal.MaterialInstanceConstantFactoryNew()
        material = asset_tools.create_asset(material_name, MATERIAL_DEST, unreal.MaterialInstanceConstant, factory)
    if not material:
        raise RuntimeError(f"Material instance creation failed: {material_asset_path}")
    material.set_editor_property("parent", parent)
    unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(material, "DiffuseColorMap", texture)
    unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(material, "BaseColorTexture", texture)
    unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(material, "Brightness", 1.0)
    unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(material, "Tint", unreal.LinearColor(1.0, 1.0, 1.0, 1.0))
    unreal.EditorAssetLibrary.save_loaded_asset(material)

    mesh = unreal.load_asset(mesh_path)
    if not mesh:
        raise RuntimeError(f"Missing static mesh {mesh_path}")
    mesh.set_material(0, material)
    unreal.EditorAssetLibrary.save_loaded_asset(mesh)

    results.append({
        "source": str(source_path),
        "texture": texture_asset_path,
        "material": material_asset_path,
        "mesh": mesh_path,
    })

with open(str(ROOT / "Saved" / "LuBuMatrixTextureBindResults.json"), "w", encoding="utf-8") as f:
    json.dump(results, f, indent=2)

unreal.log(f"[LuBuMatrixTextureBind] Bound {len(results)} textures/materials")
unreal.SystemLibrary.quit_editor()
