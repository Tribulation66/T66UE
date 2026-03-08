"""
Create M_EclipseCorona — procedural eclipse ring-of-fire material.

Unlit, Translucent. Procedural radial gradient:
  - center (dist < InnerRadius): opaque black (eclipsing body)
  - ring (InnerRadius..OuterRadius): bright emissive corona
  - outside (dist > OuterRadius): fully transparent

Parameters: CoronaIntensity (emissive multiplier), CoronaColor (tint).

Run in-editor: Tools > Execute Python Script > Scripts/CreateEclipseCoronaMaterial.py
"""

import unreal

DEST_DIR = "/Game/Lighting"
ASSET_NAME = "M_EclipseCorona"
LOG_PREFIX = "[Eclipse]"


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


def create_eclipse_material():
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

    # --- Material settings: Unlit + Translucent + TwoSided ---
    try:
        _safe_set(mat, "shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
        _safe_set(mat, "blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)
        _safe_set(mat, "two_sided", True)
        unreal.log(f"{LOG_PREFIX} Unlit + Translucent + TwoSided")
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} Material settings: {e}")
        ok = False

    # ============================================================
    # Node graph: procedural radial eclipse
    #
    # TexCoord -> subtract (0.5,0.5) -> distance from center
    # dist < InnerRadius: black opaque
    # InnerRadius < dist < OuterRadius: bright corona ring
    # dist > OuterRadius: transparent
    # ============================================================

    # --- TextureCoordinate ---
    texcoord = None
    try:
        texcoord = mel.create_material_expression(
            mat, unreal.MaterialExpressionTextureCoordinate, -1200, 0)
        if texcoord:
            unreal.log(f"{LOG_PREFIX} TexCoord node created")
        else:
            ok = False
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} TexCoord: {e}")
        ok = False

    # --- Constant2Vector (0.5, 0.5) for centering ---
    center_const = None
    try:
        center_const = mel.create_material_expression(
            mat, unreal.MaterialExpressionConstant2Vector, -1200, 200)
        if center_const:
            _safe_set(center_const, "r", 0.5)
            _safe_set(center_const, "g", 0.5)
        else:
            ok = False
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} Center const: {e}")
        ok = False

    # --- Subtract: UV - 0.5 ---
    sub = None
    try:
        sub = mel.create_material_expression(
            mat, unreal.MaterialExpressionSubtract, -900, 0)
        if sub and texcoord and center_const:
            mel.connect_material_expressions(texcoord, "", sub, "A")
            mel.connect_material_expressions(center_const, "", sub, "B")
            unreal.log(f"{LOG_PREFIX} Subtract (UV - 0.5) connected")
        else:
            ok = False
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} Subtract: {e}")
        ok = False

    # --- VectorLength: distance from center ---
    # UE Python API doesn't have a direct "Length" node as a class name.
    # Use Distance instead: distance(UV-0.5, 0)
    dist_node = None
    try:
        dist_node = mel.create_material_expression(
            mat, unreal.MaterialExpressionDistance, -700, 0)
        if dist_node and texcoord and center_const:
            mel.connect_material_expressions(texcoord, "", dist_node, "A")
            mel.connect_material_expressions(center_const, "", dist_node, "B")
            unreal.log(f"{LOG_PREFIX} Distance node connected")
        else:
            ok = False
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} Distance: {e}")
        ok = False

    # --- ScalarParameter: InnerRadius (default 0.36) ---
    inner_param = None
    try:
        inner_param = mel.create_material_expression(
            mat, unreal.MaterialExpressionScalarParameter, -700, 300)
        if inner_param:
            _safe_set(inner_param, "parameter_name", "InnerRadius")
            _safe_set(inner_param, "default_value", 0.38)
        else:
            ok = False
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} InnerRadius: {e}")
        ok = False

    # --- ScalarParameter: OuterRadius (default 0.48) ---
    outer_param = None
    try:
        outer_param = mel.create_material_expression(
            mat, unreal.MaterialExpressionScalarParameter, -700, 450)
        if outer_param:
            _safe_set(outer_param, "parameter_name", "OuterRadius")
            _safe_set(outer_param, "default_value", 0.45)
        else:
            ok = False
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} OuterRadius: {e}")
        ok = False

    # --- SmoothStep: corona ring mask = smoothstep(inner, outer, dist) ---
    # UE doesn't have a direct SmoothStep node, but we can use
    # (dist - inner) / (outer - inner), clamped, then apply smoothstep math.
    # Simpler: use two LinearInterpolate + Clamp for the ring shape.

    # Actually, let's use a simpler approach with If nodes or step functions.
    # Simplest: use SphereMask-style math.
    #
    # ring_alpha = smoothstep(InnerRadius, OuterRadius, dist) * (1 - smoothstep(OuterRadius, OuterRadius+fade, dist))
    #
    # But UE material Python is limited. Let's do it with basic math:
    #   ring01 = saturate((dist - InnerRadius) / (OuterRadius - InnerRadius))
    #   fade01 = saturate((dist - OuterRadius) / 0.03)
    #   corona_mask = ring01 * (1 - fade01)
    #   body_mask = 1 - saturate(dist / InnerRadius * 10)  -- sharp inner cutoff
    #   opacity = max(body_mask, corona_mask)

    # For simplicity, let's use a different strategy with fewer nodes:
    # Use SphereMask node which does exactly what we need.

    # Actually the cleanest approach: use two SphereMask expressions.
    # SphereMask gives a 0-1 mask based on distance. But it works in 3D space...
    # Let's just keep it simple with math nodes.

    # --- Approach: use OneMinus + Clamp for the ring ---
    # inner_step = saturate((dist - InnerRadius) / 0.02)  -> 0 inside, 1 outside inner
    # outer_step = saturate((dist - OuterRadius) / 0.02)  -> 0 inside, 1 outside outer
    # ring_mask = inner_step * (1 - outer_step)            -> 1 only in the ring
    # body_mask = 1 - inner_step                           -> 1 inside the dark circle
    # opacity = saturate(body_mask + ring_mask)             -> 1 in body+ring, 0 outside

    # inner_step = saturate((dist - InnerRadius) / 0.015)
    # Need: Subtract, Divide, Saturate for inner_step
    # Then same for outer_step

    # Let me build this step by step.

    # --- (dist - InnerRadius) ---
    sub_inner = None
    try:
        sub_inner = mel.create_material_expression(
            mat, unreal.MaterialExpressionSubtract, -400, 0)
        if sub_inner and dist_node and inner_param:
            mel.connect_material_expressions(dist_node, "", sub_inner, "A")
            mel.connect_material_expressions(inner_param, "", sub_inner, "B")
        else:
            ok = False
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} sub_inner: {e}")
        ok = False

    # --- Constant for edge softness ---
    edge_soft = None
    try:
        edge_soft = mel.create_material_expression(
            mat, unreal.MaterialExpressionConstant, -400, 200)
        if edge_soft:
            _safe_set(edge_soft, "r", 0.015)
        else:
            ok = False
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} edge_soft: {e}")
        ok = False

    # --- (dist - InnerRadius) / softness ---
    div_inner = None
    try:
        div_inner = mel.create_material_expression(
            mat, unreal.MaterialExpressionDivide, -200, 0)
        if div_inner and sub_inner and edge_soft:
            mel.connect_material_expressions(sub_inner, "", div_inner, "A")
            mel.connect_material_expressions(edge_soft, "", div_inner, "B")
        else:
            ok = False
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} div_inner: {e}")
        ok = False

    # --- Saturate (clamp 0-1) for inner_step ---
    inner_step = None
    try:
        inner_step = mel.create_material_expression(
            mat, unreal.MaterialExpressionSaturate, 0, 0)
        if inner_step and div_inner:
            mel.connect_material_expressions(div_inner, "", inner_step, "")
        else:
            ok = False
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} inner_step: {e}")
        ok = False

    # --- (dist - OuterRadius) ---
    sub_outer = None
    try:
        sub_outer = mel.create_material_expression(
            mat, unreal.MaterialExpressionSubtract, -400, 400)
        if sub_outer and dist_node and outer_param:
            mel.connect_material_expressions(dist_node, "", sub_outer, "A")
            mel.connect_material_expressions(outer_param, "", sub_outer, "B")
        else:
            ok = False
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} sub_outer: {e}")
        ok = False

    # --- (dist - OuterRadius) / softness ---
    div_outer = None
    try:
        div_outer = mel.create_material_expression(
            mat, unreal.MaterialExpressionDivide, -200, 400)
        if div_outer and sub_outer and edge_soft:
            mel.connect_material_expressions(sub_outer, "", div_outer, "A")
            mel.connect_material_expressions(edge_soft, "", div_outer, "B")
        else:
            ok = False
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} div_outer: {e}")
        ok = False

    # --- Saturate for outer_step ---
    outer_step = None
    try:
        outer_step = mel.create_material_expression(
            mat, unreal.MaterialExpressionSaturate, 0, 400)
        if outer_step and div_outer:
            mel.connect_material_expressions(div_outer, "", outer_step, "")
        else:
            ok = False
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} outer_step: {e}")
        ok = False

    # --- (1 - outer_step) ---
    one_minus_outer = None
    try:
        one_minus_outer = mel.create_material_expression(
            mat, unreal.MaterialExpressionOneMinus, 200, 400)
        if one_minus_outer and outer_step:
            mel.connect_material_expressions(outer_step, "", one_minus_outer, "")
        else:
            ok = False
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} one_minus_outer: {e}")
        ok = False

    # --- ring_mask = inner_step * (1 - outer_step) ---
    ring_mask = None
    try:
        ring_mask = mel.create_material_expression(
            mat, unreal.MaterialExpressionMultiply, 400, 200)
        if ring_mask and inner_step and one_minus_outer:
            mel.connect_material_expressions(inner_step, "", ring_mask, "A")
            mel.connect_material_expressions(one_minus_outer, "", ring_mask, "B")
            unreal.log(f"{LOG_PREFIX} ring_mask = inner_step * (1 - outer_step)")
        else:
            ok = False
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} ring_mask: {e}")
        ok = False

    # --- body_mask = 1 - inner_step ---
    body_mask = None
    try:
        body_mask = mel.create_material_expression(
            mat, unreal.MaterialExpressionOneMinus, 200, 0)
        if body_mask and inner_step:
            mel.connect_material_expressions(inner_step, "", body_mask, "")
        else:
            ok = False
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} body_mask: {e}")
        ok = False

    # --- opacity = saturate(body_mask + ring_mask) ---
    opacity_add = None
    try:
        opacity_add = mel.create_material_expression(
            mat, unreal.MaterialExpressionAdd, 600, 100)
        if opacity_add and body_mask and ring_mask:
            mel.connect_material_expressions(body_mask, "", opacity_add, "A")
            mel.connect_material_expressions(ring_mask, "", opacity_add, "B")
        else:
            ok = False
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} opacity_add: {e}")
        ok = False

    opacity_sat = None
    try:
        opacity_sat = mel.create_material_expression(
            mat, unreal.MaterialExpressionSaturate, 800, 100)
        if opacity_sat and opacity_add:
            mel.connect_material_expressions(opacity_add, "", opacity_sat, "")
        else:
            ok = False
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} opacity_sat: {e}")
        ok = False

    # --- Connect opacity ---
    try:
        if opacity_sat:
            mel.connect_material_property(
                opacity_sat, "", unreal.MaterialProperty.MP_OPACITY)
            unreal.log(f"{LOG_PREFIX} Opacity connected")
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} Opacity output: {e}")
        ok = False

    # --- Corona emissive: ring_mask * CoronaColor * CoronaIntensity ---

    # VectorParameter: CoronaColor
    corona_color = None
    try:
        corona_color = mel.create_material_expression(
            mat, unreal.MaterialExpressionVectorParameter, 200, -300)
        if corona_color:
            _safe_set(corona_color, "parameter_name", "CoronaColor")
            _safe_set(corona_color, "default_value",
                      unreal.LinearColor(1.0, 0.4, 0.05, 1.0))
        else:
            ok = False
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} CoronaColor: {e}")
        ok = False

    # ScalarParameter: CoronaIntensity
    corona_intensity = None
    try:
        corona_intensity = mel.create_material_expression(
            mat, unreal.MaterialExpressionScalarParameter, 200, -150)
        if corona_intensity:
            _safe_set(corona_intensity, "parameter_name", "CoronaIntensity")
            _safe_set(corona_intensity, "default_value", 3.0)
        else:
            ok = False
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} CoronaIntensity: {e}")
        ok = False

    # ring_mask * CoronaColor
    ring_x_color = None
    try:
        ring_x_color = mel.create_material_expression(
            mat, unreal.MaterialExpressionMultiply, 500, -200)
        if ring_x_color and ring_mask and corona_color:
            mel.connect_material_expressions(ring_mask, "", ring_x_color, "A")
            mel.connect_material_expressions(corona_color, "", ring_x_color, "B")
        else:
            ok = False
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} ring_x_color: {e}")
        ok = False

    # (ring_mask * CoronaColor) * CoronaIntensity
    emissive_final = None
    try:
        emissive_final = mel.create_material_expression(
            mat, unreal.MaterialExpressionMultiply, 700, -200)
        if emissive_final and ring_x_color and corona_intensity:
            mel.connect_material_expressions(ring_x_color, "", emissive_final, "A")
            mel.connect_material_expressions(corona_intensity, "", emissive_final, "B")
        else:
            ok = False
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} emissive_final: {e}")
        ok = False

    # --- Connect emissive ---
    try:
        if emissive_final:
            mel.connect_material_property(
                emissive_final, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
            unreal.log(f"{LOG_PREFIX} EmissiveColor connected")
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
    unreal.log(f"{LOG_PREFIX} CreateEclipseCoronaMaterial START")
    unreal.log(f"{'='*60}")

    result = create_eclipse_material()

    unreal.log(f"{'='*60}")
    unreal.log(f"{LOG_PREFIX} DONE — {'OK' if result else 'FAILED'}")
    unreal.log(f"{LOG_PREFIX}   Path: /Game/Lighting/M_EclipseCorona")
    unreal.log(f"{'='*60}")


if __name__ == "__main__":
    main()
