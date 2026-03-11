"""
Create M_FBX_Unlit — Unlit clone of FBXLegacyPhongSurfaceMaterial.

Has the SAME parameter names as FBXLegacyPhongSurfaceMaterial so that when
MaterialInstanceConstants are reparented from FBXLegacy to this, all texture
overrides (DiffuseColorMap etc.) carry over automatically.

Shading Model: Unlit
Domain: Surface
TwoSided: True

Graph: DiffuseColorMap.RGB -> EmissiveColor
       (NormalMap, SpecularColorMap, EmissiveColorMap exist as params but are not connected)

Run in-editor: Tools > Execute Python Script > Scripts/CreateFBXUnlitMaterial.py
"""

import unreal

DEST_DIR = "/Game/Materials"
ASSET_NAME = "M_FBX_Unlit"
LOG_PREFIX = "[FBXUnlit]"


def _safe_set(obj, prop, value):
    if obj is None:
        return
    try:
        obj.set_editor_property(prop, value)
    except Exception as e:
        unreal.log_warning(f"{LOG_PREFIX} set_editor_property({prop}): {e}")


def _ensure_dir(game_path):
    try:
        if not unreal.EditorAssetLibrary.does_directory_exist(game_path):
            unreal.EditorAssetLibrary.make_directory(game_path)
    except Exception as e:
        unreal.log_warning(f"{LOG_PREFIX} ensure_dir({game_path}): {e}")


def create_fbx_unlit_material():
    path = f"{DEST_DIR}/{ASSET_NAME}"

    if unreal.EditorAssetLibrary.does_asset_exist(path):
        unreal.log(f"{LOG_PREFIX} Deleting existing: {path}")
        unreal.EditorAssetLibrary.delete_asset(path)

    _ensure_dir(DEST_DIR)

    mel = unreal.MaterialEditingLibrary
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    factory = unreal.MaterialFactoryNew()
    mat = asset_tools.create_asset(ASSET_NAME, DEST_DIR, unreal.Material, factory)
    if not mat:
        unreal.log_error(f"{LOG_PREFIX} Failed to create {ASSET_NAME}")
        return False

    ok = True

    # --- Material settings: Unlit + Surface + TwoSided ---
    try:
        _safe_set(mat, "shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
        _safe_set(mat, "two_sided", True)
        unreal.log(f"{LOG_PREFIX} ShadingModel=Unlit, TwoSided=True")
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} Material settings: {e}")
        ok = False

    # =====================================================================
    # Parameters — match FBXLegacyPhongSurfaceMaterial exactly so MIC
    # texture overrides carry over when reparented.
    # =====================================================================

    # --- DiffuseColorMap (the main color texture — this is the one that matters) ---
    tex_param = None
    try:
        tex_param = mel.create_material_expression(
            mat, unreal.MaterialExpressionTextureSampleParameter2D, -400, 0)
        if tex_param:
            _safe_set(tex_param, "parameter_name", "DiffuseColorMap")
            unreal.log(f"{LOG_PREFIX} DiffuseColorMap parameter created")
        else:
            unreal.log_error(f"{LOG_PREFIX} DiffuseColorMap returned None")
            ok = False
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} DiffuseColorMap: {e}")
        ok = False

    # --- NormalMap (exists for compatibility, not connected) ---
    try:
        normal_param = mel.create_material_expression(
            mat, unreal.MaterialExpressionTextureSampleParameter2D, -400, 300)
        if normal_param:
            _safe_set(normal_param, "parameter_name", "NormalMap")
            unreal.log(f"{LOG_PREFIX} NormalMap parameter created (unconnected)")
    except Exception as e:
        unreal.log_warning(f"{LOG_PREFIX} NormalMap: {e}")

    # --- SpecularColorMap (exists for compatibility, not connected) ---
    try:
        spec_param = mel.create_material_expression(
            mat, unreal.MaterialExpressionTextureSampleParameter2D, -400, 600)
        if spec_param:
            _safe_set(spec_param, "parameter_name", "SpecularColorMap")
            unreal.log(f"{LOG_PREFIX} SpecularColorMap parameter created (unconnected)")
    except Exception as e:
        unreal.log_warning(f"{LOG_PREFIX} SpecularColorMap: {e}")

    # --- EmissiveColorMap (exists for compatibility, not connected) ---
    try:
        emissive_param = mel.create_material_expression(
            mat, unreal.MaterialExpressionTextureSampleParameter2D, -400, 900)
        if emissive_param:
            _safe_set(emissive_param, "parameter_name", "EmissiveColorMap")
            unreal.log(f"{LOG_PREFIX} EmissiveColorMap parameter created (unconnected)")
    except Exception as e:
        unreal.log_warning(f"{LOG_PREFIX} EmissiveColorMap: {e}")

    # --- EmissiveColorMapWeight (exists for compatibility, not connected) ---
    try:
        emissive_weight = mel.create_material_expression(
            mat, unreal.MaterialExpressionScalarParameter, -400, 1200)
        if emissive_weight:
            _safe_set(emissive_weight, "parameter_name", "EmissiveColorMapWeight")
            _safe_set(emissive_weight, "default_value", 0.0)
            unreal.log(f"{LOG_PREFIX} EmissiveColorMapWeight parameter created (unconnected)")
    except Exception as e:
        unreal.log_warning(f"{LOG_PREFIX} EmissiveColorMapWeight: {e}")

    # =====================================================================
    # Graph: DiffuseColorMap.RGB -> Emissive Color
    # =====================================================================
    try:
        if tex_param:
            mel.connect_material_property(
                tex_param, "RGB", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
            unreal.log(f"{LOG_PREFIX} DiffuseColorMap.RGB -> EmissiveColor connected")
        else:
            ok = False
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} Emissive output: {e}")
        ok = False

    # --- Compile and save ---
    try:
        mel.recompile_material(mat)
        unreal.log(f"{LOG_PREFIX} Material compiled")
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} Recompile: {e}")
        ok = False

    try:
        unreal.EditorAssetLibrary.save_asset(path)
        unreal.log(f"{LOG_PREFIX} Saved to {path}")
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} Save: {e}")
        ok = False

    if ok:
        unreal.log(f"{LOG_PREFIX} Successfully created: {path}")
    return ok


def main():
    unreal.log(f"{'='*60}")
    unreal.log(f"{LOG_PREFIX} CreateFBXUnlitMaterial START")
    unreal.log(f"{'='*60}")

    result = create_fbx_unlit_material()

    unreal.log(f"{'='*60}")
    unreal.log(f"{LOG_PREFIX} DONE — {'OK' if result else 'FAILED'}")
    unreal.log(f"{LOG_PREFIX}   Path: {DEST_DIR}/{ASSET_NAME}")
    unreal.log(f"{'='*60}")


if __name__ == "__main__":
    main()
