"""
Create M_Environment_Unlit — master material for Unlit environment/map assets.

Shading Model: Unlit
Domain: Surface
TwoSided: True

Graph: DiffuseColorMap.RGB * Brightness * Tint -> EmissiveColor

Parameters:
  DiffuseColorMap (TextureSampleParameter2D)
  Brightness (Scalar, default 1.0)
  Tint (VectorParameter, default 1,1,1)

Run in-editor: Tools > Execute Python Script > Scripts/CreateEnvironmentUnlitMaterial.py
"""

import unreal

DEST_DIR = "/Game/Materials"
ASSET_NAME = "M_Environment_Unlit"
LOG_PREFIX = "[EnvUnlit]"


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


def create_environment_unlit_material():
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

    # --- TextureSampleParameter2D: DiffuseColorMap ---
    tex_param = None
    try:
        tex_param = mel.create_material_expression(
            mat, unreal.MaterialExpressionTextureSampleParameter2D, -500, 0)
        if tex_param:
            _safe_set(tex_param, "parameter_name", "DiffuseColorMap")
            unreal.log(f"{LOG_PREFIX} DiffuseColorMap texture parameter created")
        else:
            unreal.log_error(f"{LOG_PREFIX} TextureSampleParameter2D returned None")
            ok = False
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} DiffuseColorMap: {e}")
        ok = False

    # --- ScalarParameter: Brightness (default 1.0) ---
    brightness_param = None
    try:
        brightness_param = mel.create_material_expression(
            mat, unreal.MaterialExpressionScalarParameter, -500, 250)
        if brightness_param:
            _safe_set(brightness_param, "parameter_name", "Brightness")
            _safe_set(brightness_param, "default_value", 1.0)
            unreal.log(f"{LOG_PREFIX} Brightness parameter created (default=1.0)")
        else:
            ok = False
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} Brightness: {e}")
        ok = False

    # --- VectorParameter: Tint (default white) ---
    tint_param = None
    try:
        tint_param = mel.create_material_expression(
            mat, unreal.MaterialExpressionVectorParameter, -500, 450)
        if tint_param:
            _safe_set(tint_param, "parameter_name", "Tint")
            _safe_set(tint_param, "default_value", unreal.LinearColor(1.0, 1.0, 1.0, 1.0))
            unreal.log(f"{LOG_PREFIX} Tint parameter created (default=1,1,1)")
        else:
            ok = False
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} Tint: {e}")
        ok = False

    # --- Multiply: DiffuseColorMap.RGB * Brightness ---
    mul_bright = None
    try:
        mul_bright = mel.create_material_expression(
            mat, unreal.MaterialExpressionMultiply, -250, 100)
        if mul_bright and tex_param and brightness_param:
            mel.connect_material_expressions(tex_param, "RGB", mul_bright, "A")
            mel.connect_material_expressions(brightness_param, "", mul_bright, "B")
        else:
            ok = False
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} Multiply (tex*brightness): {e}")
        ok = False

    # --- Multiply: (above) * Tint ---
    mul_tint = None
    try:
        mul_tint = mel.create_material_expression(
            mat, unreal.MaterialExpressionMultiply, 0, 100)
        if mul_tint and mul_bright and tint_param:
            mel.connect_material_expressions(mul_bright, "", mul_tint, "A")
            mel.connect_material_expressions(tint_param, "", mul_tint, "B")
            unreal.log(f"{LOG_PREFIX} Multiply: (Tex*Brightness)*Tint connected")
        else:
            ok = False
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} Multiply (tint): {e}")
        ok = False

    # --- Connect final Multiply -> EmissiveColor ---
    try:
        if mul_tint:
            mel.connect_material_property(
                mul_tint, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
            unreal.log(f"{LOG_PREFIX} Output -> EmissiveColor connected")
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
    unreal.log(f"{LOG_PREFIX} CreateEnvironmentUnlitMaterial START")
    unreal.log(f"{'='*60}")

    result = create_environment_unlit_material()

    unreal.log(f"{'='*60}")
    unreal.log(f"{LOG_PREFIX} DONE — {'OK' if result else 'FAILED'}")
    unreal.log(f"{LOG_PREFIX}   Path: {DEST_DIR}/{ASSET_NAME}")
    unreal.log(f"{'='*60}")


if __name__ == "__main__":
    main()
