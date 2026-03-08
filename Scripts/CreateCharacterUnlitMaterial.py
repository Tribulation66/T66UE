"""
Create M_Character_Unlit — master material for Unlit characters.

Shading Model: Unlit
Domain: Surface
TwoSided: True

Graph: TextureSampleParameter2D("DiffuseColorMap") * ScalarParameter("Brightness", 1.0) -> EmissiveColor

Run in-editor: Tools > Execute Python Script > Scripts/CreateCharacterUnlitMaterial.py
"""

import unreal

DEST_DIR = "/Game/Materials"
ASSET_NAME = "M_Character_Unlit"
LOG_PREFIX = "[CharUnlit]"


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


def create_unlit_material():
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
            mat, unreal.MaterialExpressionTextureSampleParameter2D, -400, 0)
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
            mat, unreal.MaterialExpressionScalarParameter, -400, 300)
        if brightness_param:
            _safe_set(brightness_param, "parameter_name", "Brightness")
            _safe_set(brightness_param, "default_value", 1.0)
            unreal.log(f"{LOG_PREFIX} Brightness parameter created (default=1.0)")
        else:
            unreal.log_error(f"{LOG_PREFIX} Brightness node returned None")
            ok = False
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} Brightness: {e}")
        ok = False

    # --- Multiply: DiffuseColorMap.RGB * Brightness ---
    multiply = None
    try:
        multiply = mel.create_material_expression(
            mat, unreal.MaterialExpressionMultiply, -100, 0)
        if multiply and tex_param and brightness_param:
            mel.connect_material_expressions(tex_param, "RGB", multiply, "A")
            mel.connect_material_expressions(brightness_param, "", multiply, "B")
            unreal.log(f"{LOG_PREFIX} Multiply: Texture.RGB * Brightness connected")
        else:
            unreal.log_error(f"{LOG_PREFIX} Multiply node issue")
            ok = False
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} Multiply: {e}")
        ok = False

    # --- Connect Multiply output -> EmissiveColor ---
    try:
        if multiply:
            mel.connect_material_property(
                multiply, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
            unreal.log(f"{LOG_PREFIX} Multiply -> EmissiveColor connected")
        else:
            ok = False
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} Emissive output: {e}")
        ok = False

    # --- Connect texture alpha -> OpacityMask (for future use if needed) ---
    # Skipping for now — characters are opaque.

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
    unreal.log(f"{LOG_PREFIX} CreateCharacterUnlitMaterial START")
    unreal.log(f"{'='*60}")

    result = create_unlit_material()

    unreal.log(f"{'='*60}")
    unreal.log(f"{LOG_PREFIX} DONE — {'OK' if result else 'FAILED'}")
    unreal.log(f"{LOG_PREFIX}   Path: {DEST_DIR}/{ASSET_NAME}")
    unreal.log(f"{'='*60}")


if __name__ == "__main__":
    main()
