"""
Import a 2x2 ground atlas PNG and create a simple world-space tiled material.

Input:
  SourceAssets/Ground/GroundAtlas_2x2_1024.png

  If you have tile1.png..tile4.png instead, run BuildGroundAtlas.py first:
    python Scripts/BuildGroundAtlas.py

Outputs:
  /Game/World/Ground/T_GroundAtlas_2x2_1024
  /Game/World/Ground/M_GroundAtlas_2x2_R0   (0°)
  /Game/World/Ground/M_GroundAtlas_2x2_R90  (90°)
  /Game/World/Ground/M_GroundAtlas_2x2_R180 (180°)
  /Game/World/Ground/M_GroundAtlas_2x2_R270 (270°)

Run (PowerShell):
& "C:/Program Files/Epic Games/UE_5.7/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" "C:/UE/T66/T66.uproject" -run=pythonscript -script="C:/UE/T66/Scripts/ImportGroundAtlas.py" -unattended -nop4 -nosplash -nullrhi
"""

from __future__ import annotations

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


def _import_texture(src_file: str, dest_game_dir: str, dest_asset_name: str) -> str:
    _ensure_dir(dest_game_dir)

    task = unreal.AssetImportTask()
    task.automated = True
    task.save = True
    task.replace_existing = True
    task.replace_existing_settings = True
    task.filename = src_file
    task.destination_path = dest_game_dir
    task.destination_name = dest_asset_name

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    if not task.imported_object_paths:
        unreal.log_error(f"[GroundAtlas] Import produced no assets: {src_file}")
        return ""
    return task.imported_object_paths[0]


def _apply_world_texture_settings(asset_path: str):
    tex = unreal.EditorAssetLibrary.load_asset(asset_path) if asset_path else None
    if not tex or not isinstance(tex, unreal.Texture2D):
        return

    changed = False
    try:
        # World textures should have mips.
        if tex.get_editor_property("mip_gen_settings") != unreal.TextureMipGenSettings.TMGS_FROM_TEXTURE_GROUP:
            tex.set_editor_property("mip_gen_settings", unreal.TextureMipGenSettings.TMGS_FROM_TEXTURE_GROUP)
            changed = True
    except Exception:
        pass

    try:
        if tex.get_editor_property("compression_settings") != unreal.TextureCompressionSettings.TC_DEFAULT:
            tex.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_DEFAULT)
            changed = True
    except Exception:
        pass

    try:
        # Treat as a world texture (affects streaming/LodGroup defaults).
        if tex.get_editor_property("lod_group") != unreal.TextureGroup.TEXTUREGROUP_World:
            tex.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_World)
            changed = True
    except Exception:
        pass

    # Ensure wrap addressing so UVs beyond 0..1 tile.
    try:
        if tex.get_editor_property("address_x") != unreal.TextureAddress.TA_WRAP:
            tex.set_editor_property("address_x", unreal.TextureAddress.TA_WRAP)
            changed = True
    except Exception:
        pass
    try:
        if tex.get_editor_property("address_y") != unreal.TextureAddress.TA_WRAP:
            tex.set_editor_property("address_y", unreal.TextureAddress.TA_WRAP)
            changed = True
    except Exception:
        pass

    if changed:
        try:
            unreal.EditorAssetLibrary.save_loaded_asset(tex)
        except Exception:
            pass
        try:
            unreal.EditorAssetLibrary.save_asset(asset_path)
        except Exception:
            pass


def _create_or_load_material(dest_game_dir: str, asset_name: str) -> unreal.Material | None:
    asset_path = f"{dest_game_dir}/{asset_name}"
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        a = unreal.EditorAssetLibrary.load_asset(asset_path)
        return a if isinstance(a, unreal.Material) else None

    _ensure_dir(dest_game_dir)
    factory = unreal.MaterialFactoryNew()
    return unreal.AssetToolsHelpers.get_asset_tools().create_asset(asset_name, dest_game_dir, unreal.Material, factory)


def _force_unlit(mat: unreal.Material):
    """Force a material to Unlit shading model with verification."""
    mat_name = mat.get_name()

    # Read the current shading model.
    try:
        before = mat.get_editor_property("shading_model")
        unreal.log(f"[GroundAtlas] {mat_name} shading_model BEFORE = {before}")
    except Exception as e:
        unreal.log_warning(f"[GroundAtlas] {mat_name} could not read shading_model: {e}")
        before = None

    # Attempt 1: set_editor_property
    try:
        mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    except Exception as e:
        unreal.log_warning(f"[GroundAtlas] {mat_name} set_editor_property(shading_model) failed: {e}")

    # Verify it stuck.
    try:
        after = mat.get_editor_property("shading_model")
        unreal.log(f"[GroundAtlas] {mat_name} shading_model AFTER set_editor_property = {after}")
    except Exception:
        after = None

    if after != unreal.MaterialShadingModel.MSM_UNLIT:
        # Attempt 2: use integer value directly (MSM_UNLIT = 0 in the enum).
        try:
            mat.set_editor_property("shading_model", 0)
            after2 = mat.get_editor_property("shading_model")
            unreal.log(f"[GroundAtlas] {mat_name} shading_model AFTER int(0) = {after2}")
        except Exception as e2:
            unreal.log_warning(f"[GroundAtlas] {mat_name} int fallback also failed: {e2}")

    # Two-sided
    try:
        mat.set_editor_property("two_sided", True)
    except Exception as e:
        unreal.log_warning(f"[GroundAtlas] {mat_name} set two_sided failed: {e}")


# Unlit graph: TexCoord * TilingScale -> Texture -> EmissiveColor. Two-sided.
def _build_simple_material_graph(mat: unreal.Material, atlas_tex: unreal.Texture2D, _rotation_deg: int):
    mel = unreal.MaterialEditingLibrary

    try:
        mel.delete_all_material_expressions(mat)
    except Exception:
        try:
            for e in mat.get_editor_property("expressions") or []:
                try:
                    mel.delete_material_expression(mat, e)
                except Exception:
                    pass
        except Exception:
            pass

    _force_unlit(mat)

    tex_coord = mel.create_material_expression(mat, unreal.MaterialExpressionTextureCoordinate, -300, -60)
    tiling = mel.create_material_expression(mat, unreal.MaterialExpressionScalarParameter, -300, 80)
    tiling.set_editor_property("parameter_name", "TilingScale")
    tiling.set_editor_property("default_value", 10.0)
    mul = mel.create_material_expression(mat, unreal.MaterialExpressionMultiply, -100, -60)
    mel.connect_material_expressions(tex_coord, "", mul, "A")
    mel.connect_material_expressions(tiling, "", mul, "B")

    tex_sample = mel.create_material_expression(mat, unreal.MaterialExpressionTextureSampleParameter2D, 100, -60)
    tex_sample.set_editor_property("texture", atlas_tex)
    tex_sample.set_editor_property("parameter_name", "GroundAtlas")
    mel.connect_material_expressions(mul, "", tex_sample, "UVs")

    mel.connect_material_property(tex_sample, "RGB", unreal.MaterialProperty.MP_EMISSIVE_COLOR)

    # Verify final shading model before recompile.
    try:
        final_sm = mat.get_editor_property("shading_model")
        unreal.log(f"[GroundAtlas] {mat.get_name()} shading_model FINAL (pre-recompile) = {final_sm}")
    except Exception:
        pass

    try:
        mel.recompile_material(mat)
    except Exception:
        pass

    # Verify shading model survived recompile.
    try:
        post_sm = mat.get_editor_property("shading_model")
        unreal.log(f"[GroundAtlas] {mat.get_name()} shading_model POST-RECOMPILE = {post_sm}")
        if post_sm != unreal.MaterialShadingModel.MSM_UNLIT:
            unreal.log_error(f"[GroundAtlas] {mat.get_name()} IS STILL LIT AFTER RECOMPILE!")
    except Exception:
        pass


def main():
    proj = _proj_dir()
    dest_dir = "/Game/World/Ground"
    tex_asset_name = "T_GroundAtlas_2x2_1024"

    src = os.path.join(proj, "SourceAssets", "Ground", "GroundAtlas_2x2_1024.png")
    existing_tex_check = f"{dest_dir}/{tex_asset_name}"
    if not os.path.isfile(src) and not unreal.EditorAssetLibrary.does_asset_exist(existing_tex_check):
        unreal.log_error(f"[GroundAtlas] Missing source PNG and no existing texture asset: {src}")
        return
    mat_variants = [
        ("M_GroundAtlas_2x2_R0", 0),
        ("M_GroundAtlas_2x2_R90", 90),
        ("M_GroundAtlas_2x2_R180", 180),
        ("M_GroundAtlas_2x2_R270", 270),
    ]

    unreal.log("=== ImportGroundAtlas: START ===")
    unreal.log(f"[GroundAtlas] Source: {src}")

    existing_tex_asset_path = f"{dest_dir}/{tex_asset_name}"
    if unreal.EditorAssetLibrary.does_asset_exist(existing_tex_asset_path):
        tex_path = existing_tex_asset_path
    else:
        tex_path = _import_texture(src, dest_dir, tex_asset_name)
    _apply_world_texture_settings(tex_path)
    atlas_tex = unreal.EditorAssetLibrary.load_asset(tex_path) if tex_path else None
    if not atlas_tex or not isinstance(atlas_tex, unreal.Texture2D):
        unreal.log_error("[GroundAtlas] Failed to load imported texture asset.")
        return

    for name, rot in mat_variants:
        mat = _create_or_load_material(dest_dir, name)
        if not mat:
            unreal.log_error(f"[GroundAtlas] Failed to create/load material: {name}")
            continue
        _build_simple_material_graph(mat, atlas_tex, rot)
        try:
            unreal.EditorAssetLibrary.save_loaded_asset(mat)
        except Exception:
            pass
        try:
            unreal.EditorAssetLibrary.save_asset(f"{dest_dir}/{name}")
        except Exception:
            pass

    unreal.log(f"=== ImportGroundAtlas: DONE ({tex_path} , 4 materials) ===")

    # If launched via -ExecutePythonScript, auto-quit so automation doesn't leave an editor open.
    try:
        get_cmd = getattr(unreal.SystemLibrary, "get_command_line", None)
        cmdline = str(get_cmd()) if callable(get_cmd) else ""
        if "ExecutePythonScript" in cmdline or "executePythonScript" in cmdline:
            unreal.SystemLibrary.quit_editor()
    except Exception:
        pass


if __name__ == "__main__":
    main()

