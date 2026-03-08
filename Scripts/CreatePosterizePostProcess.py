"""
Create M_PosterizePostProcess — sky-only color banding post-process material.

Posterizes/quantizes far pixels (sky): floor(color * Steps) / Steps
Near pixels (world geometry, characters) pass through unmodified.

Uses SceneDepth to create a sky mask — pixels beyond SkyDepthThreshold
are considered sky and get posterized.

Run in-editor: Tools > Execute Python Script > Scripts/CreatePosterizePostProcess.py
"""

import unreal

DEST_DIR = "/Game/UI"
ASSET_NAME = "M_PosterizePostProcess"
LOG_PREFIX = "[Posterize]"


def _safe_set(obj, prop, value):
    if obj is None:
        return
    try:
        obj.set_editor_property(prop, value)
    except Exception as e:
        unreal.log_warning(f"{LOG_PREFIX} set_editor_property({prop}): {e}")


def create_posterize_material():
    path = f"{DEST_DIR}/{ASSET_NAME}"

    if unreal.EditorAssetLibrary.does_asset_exist(path):
        unreal.log(f"{LOG_PREFIX} Deleting existing: {path}")
        unreal.EditorAssetLibrary.delete_asset(path)

    mel = unreal.MaterialEditingLibrary
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    factory = unreal.MaterialFactoryNew()
    mat = asset_tools.create_asset(ASSET_NAME, DEST_DIR, unreal.Material, factory)
    if not mat:
        unreal.log_error(f"{LOG_PREFIX} Failed to create {ASSET_NAME}")
        return False

    ok = True

    # --- Material settings: PostProcess domain ---
    try:
        _safe_set(mat, "material_domain", unreal.MaterialDomain.MD_POST_PROCESS)
        try:
            _safe_set(mat, "blendable_location",
                      unreal.BlendableLocation.BL_SCENE_COLOR_AFTER_TONEMAPPING)
        except Exception:
            _safe_set(mat, "blendable_location",
                      unreal.BlendableLocation.BL_AFTER_TONEMAPPING)
        unreal.log(f"{LOG_PREFIX} Domain=PostProcess, BlendableLocation=AfterTonemapping")
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} Material settings: {e}")
        ok = False

    # ============================================================
    # Node graph:
    #   SceneColor = PostProcessInput0
    #   posterized = floor(SceneColor * Steps) / Steps
    #
    #   depth = SceneTexture:SceneDepth
    #   sky_mask = saturate((depth - SkyDepthThreshold) / SkyDepthThreshold)
    #   (sharp cutoff: anything beyond threshold is ~1.0, near is ~0.0)
    #
    #   final_alpha = Intensity * sky_mask
    #   output = lerp(SceneColor, posterized, final_alpha)
    # ============================================================

    scene_tex = None
    steps_param = None
    intensity_param = None
    depth_threshold_param = None

    # --- SceneTexture: PostProcessInput0 ---
    try:
        scene_tex = mel.create_material_expression(
            mat, unreal.MaterialExpressionSceneTexture, -900, 0)
        if scene_tex:
            _safe_set(scene_tex, "scene_texture_id",
                      unreal.SceneTextureId.PPI_POST_PROCESS_INPUT0)
            unreal.log(f"{LOG_PREFIX} SceneTexture(PostProcessInput0) created")
        else:
            ok = False
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} SceneTexture: {e}")
        ok = False

    # --- ScalarParameter "Steps" (default 10) ---
    try:
        steps_param = mel.create_material_expression(
            mat, unreal.MaterialExpressionScalarParameter, -900, 200)
        if steps_param:
            _safe_set(steps_param, "parameter_name", "Steps")
            _safe_set(steps_param, "default_value", 10.0)
        else:
            ok = False
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} Steps: {e}")
        ok = False

    # --- ScalarParameter "Intensity" (default 1.0) ---
    try:
        intensity_param = mel.create_material_expression(
            mat, unreal.MaterialExpressionScalarParameter, -900, 400)
        if intensity_param:
            _safe_set(intensity_param, "parameter_name", "Intensity")
            _safe_set(intensity_param, "default_value", 1.0)
        else:
            ok = False
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} Intensity: {e}")
        ok = False

    # --- ScalarParameter "SkyDepthThreshold" (default 500000) ---
    try:
        depth_threshold_param = mel.create_material_expression(
            mat, unreal.MaterialExpressionScalarParameter, -900, 600)
        if depth_threshold_param:
            _safe_set(depth_threshold_param, "parameter_name", "SkyDepthThreshold")
            _safe_set(depth_threshold_param, "default_value", 500000.0)
        else:
            ok = False
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} SkyDepthThreshold: {e}")
        ok = False

    # --- Multiply: SceneColor * Steps ---
    multiply = None
    try:
        multiply = mel.create_material_expression(
            mat, unreal.MaterialExpressionMultiply, -600, 0)
        if multiply and scene_tex and steps_param:
            mel.connect_material_expressions(scene_tex, "Color", multiply, "A")
            mel.connect_material_expressions(steps_param, "", multiply, "B")
        else:
            ok = False
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} Multiply: {e}")
        ok = False

    # --- Floor(SceneColor * Steps) ---
    floor_node = None
    try:
        floor_node = mel.create_material_expression(
            mat, unreal.MaterialExpressionFloor, -400, 0)
        if floor_node and multiply:
            mel.connect_material_expressions(multiply, "", floor_node, "")
        else:
            ok = False
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} Floor: {e}")
        ok = False

    # --- Divide: Floor / Steps = posterized color ---
    divide = None
    try:
        divide = mel.create_material_expression(
            mat, unreal.MaterialExpressionDivide, -200, 0)
        if divide and floor_node and steps_param:
            mel.connect_material_expressions(floor_node, "", divide, "A")
            mel.connect_material_expressions(steps_param, "", divide, "B")
        else:
            ok = False
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} Divide: {e}")
        ok = False

    # ============================================================
    # Depth-based sky mask
    # ============================================================

    # --- SceneTexture: SceneDepth ---
    depth_tex = None
    try:
        depth_tex = mel.create_material_expression(
            mat, unreal.MaterialExpressionSceneTexture, -900, -300)
        if depth_tex:
            _safe_set(depth_tex, "scene_texture_id",
                      unreal.SceneTextureId.PPI_SCENE_DEPTH)
            unreal.log(f"{LOG_PREFIX} SceneTexture(SceneDepth) created")
        else:
            ok = False
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} SceneDepth: {e}")
        ok = False

    # --- Divide: depth / SkyDepthThreshold ---
    depth_div = None
    try:
        depth_div = mel.create_material_expression(
            mat, unreal.MaterialExpressionDivide, -600, -300)
        if depth_div and depth_tex and depth_threshold_param:
            mel.connect_material_expressions(depth_tex, "Color", depth_div, "A")
            mel.connect_material_expressions(depth_threshold_param, "", depth_div, "B")
        else:
            ok = False
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} depth_div: {e}")
        ok = False

    # --- Saturate: clamp to 0-1 = sky_mask ---
    sky_mask = None
    try:
        sky_mask = mel.create_material_expression(
            mat, unreal.MaterialExpressionSaturate, -400, -300)
        if sky_mask and depth_div:
            mel.connect_material_expressions(depth_div, "", sky_mask, "")
            unreal.log(f"{LOG_PREFIX} sky_mask = saturate(depth / threshold)")
        else:
            ok = False
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} sky_mask: {e}")
        ok = False

    # --- Multiply: Intensity * sky_mask = final lerp alpha ---
    final_alpha = None
    try:
        final_alpha = mel.create_material_expression(
            mat, unreal.MaterialExpressionMultiply, -200, -200)
        if final_alpha and intensity_param and sky_mask:
            mel.connect_material_expressions(intensity_param, "", final_alpha, "A")
            mel.connect_material_expressions(sky_mask, "", final_alpha, "B")
            unreal.log(f"{LOG_PREFIX} final_alpha = Intensity * sky_mask")
        else:
            ok = False
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} final_alpha: {e}")
        ok = False

    # --- Lerp(Original, Posterized, final_alpha) ---
    lerp = None
    try:
        lerp = mel.create_material_expression(
            mat, unreal.MaterialExpressionLinearInterpolate, 100, 0)
        if lerp and scene_tex and divide and final_alpha:
            mel.connect_material_expressions(scene_tex, "Color", lerp, "A")
            mel.connect_material_expressions(divide, "", lerp, "B")
            mel.connect_material_expressions(final_alpha, "", lerp, "Alpha")
            unreal.log(f"{LOG_PREFIX} Lerp(Original, Posterized, Intensity*SkyMask)")
        else:
            ok = False
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} Lerp: {e}")
        ok = False

    # --- Connect Lerp output -> EmissiveColor ---
    try:
        if lerp:
            mel.connect_material_property(
                lerp, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
            unreal.log(f"{LOG_PREFIX} Lerp -> EmissiveColor connected")
        else:
            ok = False
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} Output connection: {e}")
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
    unreal.log(f"{LOG_PREFIX} CreatePosterizePostProcess START")
    unreal.log(f"{'='*60}")

    result = create_posterize_material()

    unreal.log(f"{'='*60}")
    unreal.log(f"{LOG_PREFIX} DONE — {'OK' if result else 'FAILED'}")
    unreal.log(f"{LOG_PREFIX}   Path: /Game/UI/M_PosterizePostProcess")
    unreal.log(f"{'='*60}")


if __name__ == "__main__":
    main()
