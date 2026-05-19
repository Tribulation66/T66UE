"""
Import the normalized ToonStyle Lu Bu Pixal3D matrix as static meshes.

Run with:
  UnrealEditor.exe T66.uproject -ExecutePythonScript="C:/UE/T66/ToonStyle/Source/ImportLuBuMatrixStaticMeshesAndExit.py" -unattended -nop4 -nosplash
"""

from __future__ import annotations

import os
import sys

import unreal


REPO_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
SCRIPTS_DIR = os.path.join(REPO_ROOT, "Scripts")
if SCRIPTS_DIR not in sys.path:
    sys.path.insert(0, SCRIPTS_DIR)

import ImportStaticMeshes  # noqa: E402


SOURCE_DIR = os.path.join(
    REPO_ROOT,
    "SourceAssets",
    "ToonStyle",
    "Pixal3D",
    "Phase1A",
    "LuBu_Matrix",
    "Normalized",
).replace("\\", "/")

DEST_DIR = "/Game/ToonStyle/TestAssets/LuBu_Matrix"

IMPORTS = (
    ("lubu_r1024_t2048_default_normalized.glb", "SM_LuBu_R1024_T2048_Default"),
    ("lubu_r1536_t2048_default_normalized.glb", "SM_LuBu_R1536_T2048_Default"),
    ("lubu_r1024_t4096_default_normalized.glb", "SM_LuBu_R1024_T4096_Default"),
    ("lubu_r1536_t4096_default_normalized.glb", "SM_LuBu_R1536_T4096_Default"),
    ("lubu_r1024_t2048_high_normalized.glb", "SM_LuBu_R1024_T2048_High"),
    ("lubu_r1536_t4096_high_normalized.glb", "SM_LuBu_R1536_T4096_High"),
)


def import_fbx(source_path: str, asset_name: str) -> list[str]:
    fbx_source = source_path.replace("/Normalized/", "/FBX/").replace("_normalized.glb", "_normalized.fbx")
    if not os.path.isfile(fbx_source):
        unreal.log_error(f"[LuBuMatrixImport] FBX fallback source missing: {fbx_source}")
        return []

    task = unreal.AssetImportTask()
    task.automated = True
    task.save = True
    task.replace_existing = True
    task.replace_existing_settings = True
    task.filename = fbx_source
    task.destination_path = DEST_DIR
    task.destination_name = asset_name

    options = unreal.FbxImportUI()
    options.set_editor_property("automated_import_should_detect_type", False)
    options.set_editor_property("mesh_type_to_import", unreal.FBXImportType.FBXIT_STATIC_MESH)
    options.set_editor_property("import_mesh", True)
    options.set_editor_property("import_as_skeletal", False)
    options.set_editor_property("import_materials", True)
    options.set_editor_property("import_textures", True)
    options.set_editor_property("import_animations", False)

    static_mesh_data = options.get_editor_property("static_mesh_import_data")
    if static_mesh_data:
        for prop_name, value in {
            "combine_meshes": True,
            "auto_generate_collision": False,
            "generate_lightmap_u_vs": False,
            "normal_import_method": unreal.FBXNormalImportMethod.FBXNIM_IMPORT_NORMALS_AND_TANGENTS,
        }.items():
            try:
                static_mesh_data.set_editor_property(prop_name, value)
            except Exception as exc:
                unreal.log_warning(f"[LuBuMatrixImport] Could not set FBX {prop_name}: {exc}")

    task.options = options
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    return list(task.imported_object_paths or [])


def import_one(source_name: str, asset_name: str) -> bool:
    source = os.path.join(SOURCE_DIR, source_name).replace("\\", "/")
    if not os.path.isfile(source):
        unreal.log_error(f"[LuBuMatrixImport] Missing source GLB: {source}")
        return False

    ImportStaticMeshes._ensure_game_dir(DEST_DIR)
    ImportStaticMeshes._cleanup_existing_import_artifacts(DEST_DIR, asset_name, {"mode": "asset_subtree"})
    imported_paths = ImportStaticMeshes.import_glb(source, DEST_DIR, asset_name)
    if not imported_paths:
        unreal.log_warning(f"[LuBuMatrixImport] GLB import returned no assets; trying FBX fallback for {asset_name}")
        imported_paths = import_fbx(source, asset_name)
        unreal.log(f"[LuBuMatrixImport] FBX fallback returned {len(imported_paths)} object path(s)")

    unreal.log(f"[LuBuMatrixImport] Imported {source_name}: {len(imported_paths)} object path(s)")
    for path in imported_paths:
        unreal.log(f"    {path}")

    final_path = ImportStaticMeshes._flatten_interchange_asset(DEST_DIR, asset_name)
    if not final_path:
        unreal.log_error(f"[LuBuMatrixImport] Could not locate imported StaticMesh for {asset_name}")
        return False

    ImportStaticMeshes._apply_static_mesh_build_settings(final_path, {})
    asset = unreal.EditorAssetLibrary.load_asset(final_path)
    if not asset or not isinstance(asset, unreal.StaticMesh):
        unreal.log_error(f"[LuBuMatrixImport] Final asset is not a StaticMesh: {final_path}")
        return False

    unreal.EditorAssetLibrary.save_asset(final_path)
    unreal.log(f"[LuBuMatrixImport] Final StaticMesh: {final_path}")
    return True


def main() -> int:
    unreal.log("[LuBuMatrixImport] START")
    success = 0
    failed = 0

    for source_name, asset_name in IMPORTS:
        if import_one(source_name, asset_name):
            success += 1
        else:
            failed += 1

    try:
        unreal.EditorAssetLibrary.save_directory(DEST_DIR, only_if_is_dirty=False, recursive=True)
    except Exception as exc:
        unreal.log_warning(f"[LuBuMatrixImport] save_directory warning: {exc}")

    unreal.log(f"[LuBuMatrixImport] DONE success={success} failed={failed}")
    try:
        unreal.SystemLibrary.quit_editor()
    except Exception as exc:
        unreal.log_warning(f"[LuBuMatrixImport] quit_editor warning: {exc}")

    return 0 if failed == 0 else 1


main()
