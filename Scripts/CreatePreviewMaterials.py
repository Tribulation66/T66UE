"""
Create parameterized materials for hero/companion preview stages.

M_PreviewGround — Lit grass material with VectorParameter "BaseColor" for green tint.
M_PreviewSky    — Unlit sky dome with gradient (Horizon→Sky) via EmissiveColor.

NOTE: As of the C++ T66PreviewMaterials helper, these materials are auto-created
and saved from C++ (WITH_EDITORONLY_DATA) the first time you enter PIE if the
assets don't exist on disk. This Python script is now an OPTIONAL alternative
for manual regeneration. The C++ path is preferred.

All visual tuning (day/night colors) is done from C++ via MID parameters.
This script only builds the material node graphs.

Run (PowerShell — no -nullrhi, rendering required for material compile):
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" ^
    "C:\UE\T66\T66.uproject" ^
    -run=pythonscript -script="C:\UE\T66\Scripts\CreatePreviewMaterials.py" ^
    -unattended -nop4 -nosplash

Or in-editor: Tools > Execute Python Script > Scripts/CreatePreviewMaterials.py
"""

import unreal

DEST_DIR = "/Game/UI/Preview"
LOG_PREFIX = "[PreviewMaterials]"


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


# ---------------------------------------------------------------------------
# M_PreviewGround — simple green parameterized material
# ---------------------------------------------------------------------------
def create_ground_material():
    path = f"{DEST_DIR}/M_PreviewGround"

    if unreal.EditorAssetLibrary.does_asset_exist(path):
        unreal.log(f"{LOG_PREFIX} Deleting existing: {path}")
        unreal.EditorAssetLibrary.delete_asset(path)

    _ensure_dir(DEST_DIR)

    mel = unreal.MaterialEditingLibrary
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    factory = unreal.MaterialFactoryNew()
    mat = asset_tools.create_asset("M_PreviewGround", DEST_DIR, unreal.Material, factory)
    if not mat:
        unreal.log_error(f"{LOG_PREFIX} Failed to create M_PreviewGround")
        return False

    ok = True

    # --- BaseColor parameter (default: grass green) ---
    try:
        base_color = mel.create_material_expression(mat, unreal.MaterialExpressionVectorParameter, -400, 0)
        if base_color:
            _safe_set(base_color, "parameter_name", "BaseColor")
            _safe_set(base_color, "default_value", unreal.LinearColor(0.15, 0.45, 0.08, 1.0))
            mel.connect_material_property(base_color, "", unreal.MaterialProperty.MP_BASE_COLOR)
            unreal.log(f"{LOG_PREFIX} Ground: BaseColor node OK")
        else:
            unreal.log_error(f"{LOG_PREFIX} Ground: BaseColor node returned None")
            ok = False
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} Ground BaseColor: {e}")
        ok = False

    # --- VariationColor parameter (stored for C++ use; connected to nothing for now) ---
    try:
        var_color = mel.create_material_expression(mat, unreal.MaterialExpressionVectorParameter, -400, 200)
        if var_color:
            _safe_set(var_color, "parameter_name", "VariationColor")
            _safe_set(var_color, "default_value", unreal.LinearColor(0.10, 0.35, 0.05, 1.0))
            unreal.log(f"{LOG_PREFIX} Ground: VariationColor node OK (param only)")
    except Exception as e:
        unreal.log_warning(f"{LOG_PREFIX} Ground VariationColor (optional): {e}")

    # --- Roughness constant (matte grass) ---
    try:
        roughness = mel.create_material_expression(mat, unreal.MaterialExpressionConstant, -400, 400)
        if roughness:
            _safe_set(roughness, "r", 0.85)
            mel.connect_material_property(roughness, "", unreal.MaterialProperty.MP_ROUGHNESS)
            unreal.log(f"{LOG_PREFIX} Ground: Roughness node OK")
    except Exception as e:
        unreal.log_warning(f"{LOG_PREFIX} Ground Roughness: {e}")

    # --- Compile and save ---
    try:
        mel.recompile_material(mat)
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} Ground recompile: {e}")
        ok = False

    try:
        unreal.EditorAssetLibrary.save_asset(path)
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} Ground save: {e}")
        ok = False

    if ok:
        unreal.log(f"{LOG_PREFIX} Created: {path}")
    return ok


# ---------------------------------------------------------------------------
# M_PreviewSky — gradient sky (Horizon → Zenith) via EmissiveColor
#
# Trick: BaseColor = black + EmissiveColor = gradient.
# This makes the sky self-illuminated (no dependence on scene lights)
# WITHOUT needing to set the shading model to Unlit (which can crash).
# ---------------------------------------------------------------------------
def create_sky_material():
    path = f"{DEST_DIR}/M_PreviewSky"

    if unreal.EditorAssetLibrary.does_asset_exist(path):
        unreal.log(f"{LOG_PREFIX} Deleting existing: {path}")
        unreal.EditorAssetLibrary.delete_asset(path)

    _ensure_dir(DEST_DIR)

    mel = unreal.MaterialEditingLibrary
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    factory = unreal.MaterialFactoryNew()
    mat = asset_tools.create_asset("M_PreviewSky", DEST_DIR, unreal.Material, factory)
    if not mat:
        unreal.log_error(f"{LOG_PREFIX} Failed to create M_PreviewSky")
        return False

    ok = True

    # --- Black BaseColor (suppress lit contribution so only Emissive shows) ---
    try:
        black = mel.create_material_expression(mat, unreal.MaterialExpressionConstant3Vector, -200, 300)
        if black:
            _safe_set(black, "constant", unreal.LinearColor(0.0, 0.0, 0.0, 1.0))
            mel.connect_material_property(black, "", unreal.MaterialProperty.MP_BASE_COLOR)
            unreal.log(f"{LOG_PREFIX} Sky: black BaseColor OK")
    except Exception as e:
        unreal.log_warning(f"{LOG_PREFIX} Sky black BaseColor: {e}")

    # --- Full roughness (no specular highlights on the sky dome) ---
    try:
        rough = mel.create_material_expression(mat, unreal.MaterialExpressionConstant, -200, 450)
        if rough:
            _safe_set(rough, "r", 1.0)
            mel.connect_material_property(rough, "", unreal.MaterialProperty.MP_ROUGHNESS)
            unreal.log(f"{LOG_PREFIX} Sky: Roughness OK")
    except Exception as e:
        unreal.log_warning(f"{LOG_PREFIX} Sky Roughness: {e}")

    # --- Gradient: TextureCoordinate → ComponentMask(V) → Lerp alpha ---
    tex_coord = None
    mask_v = None
    try:
        tex_coord = mel.create_material_expression(mat, unreal.MaterialExpressionTextureCoordinate, -900, 0)
        unreal.log(f"{LOG_PREFIX} Sky: TexCoord OK")
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} Sky TexCoord: {e}")
        ok = False

    try:
        mask_v = mel.create_material_expression(mat, unreal.MaterialExpressionComponentMask, -700, 0)
        if mask_v:
            _safe_set(mask_v, "r", False)
            _safe_set(mask_v, "g", True)   # V = green channel of UV
            _safe_set(mask_v, "b", False)
            _safe_set(mask_v, "a", False)
            unreal.log(f"{LOG_PREFIX} Sky: ComponentMask OK")
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} Sky ComponentMask: {e}")
        ok = False

    if tex_coord and mask_v:
        try:
            mel.connect_material_expressions(tex_coord, "", mask_v, "")
            unreal.log(f"{LOG_PREFIX} Sky: TexCoord→Mask connected")
        except Exception as e:
            unreal.log_error(f"{LOG_PREFIX} Sky TexCoord→Mask connection: {e}")
            ok = False

    # --- HorizonColor parameter (bottom of gradient) ---
    horizon_color = None
    try:
        horizon_color = mel.create_material_expression(mat, unreal.MaterialExpressionVectorParameter, -700, -200)
        if horizon_color:
            _safe_set(horizon_color, "parameter_name", "HorizonColor")
            _safe_set(horizon_color, "default_value", unreal.LinearColor(0.65, 0.78, 0.95, 1.0))
            unreal.log(f"{LOG_PREFIX} Sky: HorizonColor OK")
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} Sky HorizonColor: {e}")
        ok = False

    # --- SkyColor parameter (top of gradient / zenith) ---
    sky_color = None
    try:
        sky_color = mel.create_material_expression(mat, unreal.MaterialExpressionVectorParameter, -700, -400)
        if sky_color:
            _safe_set(sky_color, "parameter_name", "SkyColor")
            _safe_set(sky_color, "default_value", unreal.LinearColor(0.35, 0.55, 0.90, 1.0))
            unreal.log(f"{LOG_PREFIX} Sky: SkyColor OK")
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} Sky SkyColor: {e}")
        ok = False

    # --- Lerp(Horizon, Sky, gradient) ---
    lerp_sky = None
    try:
        lerp_sky = mel.create_material_expression(mat, unreal.MaterialExpressionLinearInterpolate, -400, -200)
        unreal.log(f"{LOG_PREFIX} Sky: Lerp node OK")
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} Sky Lerp: {e}")
        ok = False

    # Connect gradient chain
    if horizon_color and lerp_sky:
        try:
            mel.connect_material_expressions(horizon_color, "", lerp_sky, "A")
        except Exception as e:
            unreal.log_error(f"{LOG_PREFIX} Sky Horizon→Lerp.A: {e}")
    if sky_color and lerp_sky:
        try:
            mel.connect_material_expressions(sky_color, "", lerp_sky, "B")
        except Exception as e:
            unreal.log_error(f"{LOG_PREFIX} Sky Sky→Lerp.B: {e}")
    if mask_v and lerp_sky:
        try:
            mel.connect_material_expressions(mask_v, "", lerp_sky, "Alpha")
        except Exception as e:
            unreal.log_error(f"{LOG_PREFIX} Sky Mask→Lerp.Alpha: {e}")

    # Connect Lerp output → EmissiveColor
    if lerp_sky:
        try:
            mel.connect_material_property(lerp_sky, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
            unreal.log(f"{LOG_PREFIX} Sky: Lerp→EmissiveColor connected")
        except Exception as e:
            unreal.log_error(f"{LOG_PREFIX} Sky Lerp→Emissive: {e}")
            ok = False

    # --- Extra parameters for C++ to set (stored but not connected to graph) ---
    for param_name, default_val in [
        ("DetailColor", unreal.LinearColor(1.0, 1.0, 1.0, 1.0)),
    ]:
        try:
            p = mel.create_material_expression(mat, unreal.MaterialExpressionVectorParameter, -900, 200)
            if p:
                _safe_set(p, "parameter_name", param_name)
                _safe_set(p, "default_value", default_val)
        except Exception as e:
            unreal.log_warning(f"{LOG_PREFIX} Sky optional param {param_name}: {e}")

    for param_name, default_val in [
        ("DetailIntensity", 0.25),
        ("DetailScale", 4.0),
        ("Sharpness", 2.0),
    ]:
        try:
            p = mel.create_material_expression(mat, unreal.MaterialExpressionScalarParameter, -900, 350)
            if p:
                _safe_set(p, "parameter_name", param_name)
                _safe_set(p, "default_value", default_val)
        except Exception as e:
            unreal.log_warning(f"{LOG_PREFIX} Sky optional param {param_name}: {e}")

    # --- Compile and save ---
    try:
        mel.recompile_material(mat)
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} Sky recompile: {e}")
        ok = False

    try:
        unreal.EditorAssetLibrary.save_asset(path)
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} Sky save: {e}")
        ok = False

    if ok:
        unreal.log(f"{LOG_PREFIX} Created: {path}")
    return ok


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------
def main():
    unreal.log(f"{'='*60}")
    unreal.log(f"{LOG_PREFIX} CreatePreviewMaterials START")
    unreal.log(f"{'='*60}")

    ground_ok = create_ground_material()
    sky_ok = create_sky_material()

    unreal.log(f"{'='*60}")
    unreal.log(f"{LOG_PREFIX} DONE — Ground: {'OK' if ground_ok else 'FAILED'}, Sky: {'OK' if sky_ok else 'FAILED'}")
    unreal.log(f"{LOG_PREFIX}   Ground: /Game/UI/Preview/M_PreviewGround")
    unreal.log(f"{LOG_PREFIX}   Sky:    /Game/UI/Preview/M_PreviewSky")
    unreal.log(f"{'='*60}")


if __name__ == "__main__":
    main()
