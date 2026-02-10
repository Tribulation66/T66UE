"""
Create translucent slash effect material for AoE combat VFX.

M_SlashDisc — Translucent, Unlit, TwoSided.
  VectorParameter "Color" (default orange-red) → EmissiveColor
  ScalarParameter "Opacity" (default 0.8)      → Opacity

C++ spawns AT66SlashEffect with a dynamic material instance,
tints via "Color" and fades via "Opacity".

Run (PowerShell — no -nullrhi, rendering required for material compile):
& "C:/Program Files/Epic Games/UE_5.7/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" `
    "C:/UE/T66/T66.uproject" `
    -run=pythonscript -script="C:/UE/T66/Scripts/CreateSlashEffectMaterial.py" `
    -unattended -nop4 -nosplash

Or in-editor: Tools > Execute Python Script > Scripts/CreateSlashEffectMaterial.py
"""

import unreal

DEST_DIR = "/Game/VFX"
ASSET_NAME = "M_SlashDisc"
LOG_PREFIX = "[SlashVFX]"


def _ensure_dir(game_path):
    try:
        if not unreal.EditorAssetLibrary.does_directory_exist(game_path):
            unreal.EditorAssetLibrary.make_directory(game_path)
    except Exception as e:
        unreal.log_warning(f"{LOG_PREFIX} ensure_dir({game_path}): {e}")


def _safe_set(obj, prop, value):
    """Set an editor property, logging but not crashing on failure."""
    if obj is None:
        return
    try:
        obj.set_editor_property(prop, value)
    except Exception as e:
        unreal.log_warning(f"{LOG_PREFIX} set_editor_property({prop}): {e}")


def create_slash_material():
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

    # --- Material settings: Translucent + Unlit + TwoSided ---
    try:
        _safe_set(mat, "blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)
        _safe_set(mat, "shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
        _safe_set(mat, "two_sided", True)
        unreal.log(f"{LOG_PREFIX} Blend=Translucent, Shading=Unlit, TwoSided=True")
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} Material settings: {e}")
        ok = False

    # --- VectorParameter "Color" → EmissiveColor ---
    try:
        color_param = mel.create_material_expression(
            mat, unreal.MaterialExpressionVectorParameter, -400, 0)
        if color_param:
            _safe_set(color_param, "parameter_name", "Color")
            _safe_set(color_param, "default_value",
                      unreal.LinearColor(1.0, 0.3, 0.1, 1.0))
            mel.connect_material_property(
                color_param, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
            unreal.log(f"{LOG_PREFIX} Color → EmissiveColor OK")
        else:
            unreal.log_error(f"{LOG_PREFIX} Color node returned None")
            ok = False
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} Color param: {e}")
        ok = False

    # --- ScalarParameter "Opacity" → Opacity ---
    try:
        opacity_param = mel.create_material_expression(
            mat, unreal.MaterialExpressionScalarParameter, -400, 200)
        if opacity_param:
            _safe_set(opacity_param, "parameter_name", "Opacity")
            _safe_set(opacity_param, "default_value", 0.8)
            mel.connect_material_property(
                opacity_param, "", unreal.MaterialProperty.MP_OPACITY)
            unreal.log(f"{LOG_PREFIX} Opacity → Opacity OK")
        else:
            unreal.log_error(f"{LOG_PREFIX} Opacity node returned None")
            ok = False
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} Opacity param: {e}")
        ok = False

    # --- Compile and save ---
    try:
        mel.recompile_material(mat)
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} Recompile: {e}")
        ok = False

    try:
        unreal.EditorAssetLibrary.save_asset(path)
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} Save: {e}")
        ok = False

    if ok:
        unreal.log(f"{LOG_PREFIX} Created: {path}")
    return ok


def main():
    unreal.log(f"{'='*60}")
    unreal.log(f"{LOG_PREFIX} CreateSlashEffectMaterial START")
    unreal.log(f"{'='*60}")

    result = create_slash_material()

    unreal.log(f"{'='*60}")
    unreal.log(f"{LOG_PREFIX} DONE — {'OK' if result else 'FAILED'}")
    unreal.log(f"{LOG_PREFIX}   Path: /Game/VFX/M_SlashDisc")
    unreal.log(f"{'='*60}")


if __name__ == "__main__":
    main()
