"""Create Phase 1C ToonStyle material masters and instances.

This script supersedes the Phase 1B material setup for the TestRoom only:
- outline master uses reversed-winding opaque backface culling, not two-sided mask
- Phase 1C R1 restores cooked-valid graph-distance outline width and B-channel depth offset
- environment master uses non-uniform UVTileU / UVTileV
- all 11 lineup slots, including retained Slime and Loot Bag Yellow, get toon MIs
"""

from __future__ import annotations

import json
import os
from pathlib import Path

import unreal


OUT_DIR = Path(r"C:\UE\T66\Saved\Codex\ToonStyle\Phase1C")
OUT_DIR.mkdir(parents=True, exist_ok=True)

TOON_DIR = "/Game/ToonStyle/Materials"
CHAR_MASTER = f"{TOON_DIR}/M_Toon_Character"
ENV_MASTER = f"{TOON_DIR}/M_Toon_Environment"
OUTLINE_MASTER = f"{TOON_DIR}/M_Toon_Character_Outline"
DISTANCE_TEST_MATERIAL = f"{TOON_DIR}/M_OutlineDistanceTest"
DIAGNOSTIC_DIR = "/Game/ToonStyle/Diagnostic"
VERTEX_B_DIAGNOSTIC_MATERIAL = f"{DIAGNOSTIC_DIR}/M_OutlineDiagnostic_VertexColorB"
TEXTURE_DIR = "/Game/ToonStyle/Textures"
INNER_LINES_DEFAULT_BLACK = f"{TEXTURE_DIR}/T_InnerLines_DefaultBlack"
INNER_LINES_DEFAULT_BLACK_SOURCE = Path(r"C:\UE\T66\SourceAssets\ToonStyle\Textures\T_InnerLines_DefaultBlack.png")
MAP_TRANSITION_DIR = Path(r"C:\UE\T66\Saved\Codex\ToonStyle\MapTransition")
MAP_TRANSITION_DIR.mkdir(parents=True, exist_ok=True)
ENVIRONMENT_KIT_DIR = "/Game/ToonStyle/Environment"
TESTROOM_WALL_TEXTURE = "/Game/ToonStyle/TestAssets/Environment/Textures/T_TestRoom_Wall"
TESTROOM_FLOOR_TEXTURE = "/Game/ToonStyle/TestAssets/Environment/Textures/T_TestRoom_Floor"
ENVIRONMENT_KIT_THEMES = ["Dungeon", "Forest", "Ocean", "Martian", "Hell"]

mel = unreal.MaterialEditingLibrary
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()


def log(message: str) -> None:
    unreal.log(f"[Phase1C][Materials] {message}")


def safe_set(obj, prop: str, value) -> None:
    try:
        obj.set_editor_property(prop, value)
    except Exception as exc:
        unreal.log_warning(f"[Phase1C][Materials] Could not set {prop} on {obj}: {exc}")


def ensure_dir(path: str) -> None:
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def recreate_asset(path: str, cls, factory):
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        if not unreal.EditorAssetLibrary.delete_asset(path):
            raise RuntimeError(f"Could not delete existing asset {path}")
    package, name = path.rsplit("/", 1)
    ensure_dir(package)
    asset = asset_tools.create_asset(name, package, cls, factory)
    if not asset:
        raise RuntimeError(f"Could not create {path}")
    return asset


def expr(material, cls, x: int, y: int):
    node = mel.create_material_expression(material, cls, x, y)
    if not node:
        raise RuntimeError(f"Could not create expression {cls}")
    return node


def connect(src, src_output: str, dst, dst_input: str) -> None:
    mel.connect_material_expressions(src, src_output, dst, dst_input)


def custom_input(name: str):
    value = unreal.CustomInput()
    value.set_editor_property("input_name", name)
    return value


def set_custom_inputs(custom, names: list[str]) -> None:
    custom.set_editor_property("inputs", [custom_input(name) for name in names])


def set_custom_includes(custom, paths: list[str]) -> None:
    custom.set_editor_property("include_file_paths", paths)


def vector_param(material, name: str, default: unreal.LinearColor, x: int, y: int):
    node = expr(material, unreal.MaterialExpressionVectorParameter, x, y)
    safe_set(node, "parameter_name", name)
    safe_set(node, "default_value", default)
    return node


def scalar_param(material, name: str, default: float, x: int, y: int):
    node = expr(material, unreal.MaterialExpressionScalarParameter, x, y)
    safe_set(node, "parameter_name", name)
    safe_set(node, "default_value", float(default))
    return node


def static_switch_param(material, name: str, default: bool, false_input, true_input, x: int, y: int):
    node = expr(material, unreal.MaterialExpressionStaticSwitchParameter, x, y)
    safe_set(node, "parameter_name", name)
    safe_set(node, "default_value", bool(default))
    connect(true_input, "", node, "True")
    connect(false_input, "", node, "False")
    return node


def constant(material, value: float, x: int, y: int):
    node = expr(material, unreal.MaterialExpressionConstant, x, y)
    safe_set(node, "r", float(value))
    return node


def texture_param(material, name: str, x: int, y: int, default_texture=None):
    node = expr(material, unreal.MaterialExpressionTextureSampleParameter2D, x, y)
    safe_set(node, "parameter_name", name)
    if default_texture is not None:
        safe_set(node, "texture", default_texture)
    return node


def ensure_imported_texture(source_path: Path, asset_path: str):
    existing = unreal.EditorAssetLibrary.load_asset(asset_path)
    if existing:
        if not isinstance(existing, unreal.Texture2D):
            raise RuntimeError(f"Existing asset is not a Texture2D: {asset_path}")
        return existing
    if not source_path.exists():
        raise RuntimeError(f"Missing source texture for material generator: {source_path}")
    package, name = asset_path.rsplit("/", 1)
    ensure_dir(package)
    task = unreal.AssetImportTask()
    task.automated = True
    task.save = False
    task.replace_existing = True
    task.filename = str(source_path)
    task.destination_path = package
    task.destination_name = name
    asset_tools.import_asset_tasks([task])
    texture = unreal.EditorAssetLibrary.load_asset(asset_path)
    if not texture or not isinstance(texture, unreal.Texture2D):
        raise RuntimeError(f"Texture import failed: {asset_path}")
    texture.set_editor_property("srgb", True)
    try:
        texture.set_editor_property("filter", unreal.TextureFilter.TF_DEFAULT)
        texture.set_editor_property("mip_gen_settings", unreal.TextureMipGenSettings.TMGS_FROM_TEXTURE_GROUP)
        texture.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_DEFAULT)
        texture.set_editor_property("lod_bias", 0)
    except Exception as exc:
        unreal.log_warning(f"[Phase1C][Materials] Could not set default inner line texture settings: {exc}")
    unreal.EditorAssetLibrary.save_loaded_asset(texture)
    return texture


def component_mask(material, source, output: str, r=False, g=False, b=False, a=False, x=0, y=0):
    node = expr(material, unreal.MaterialExpressionComponentMask, x, y)
    safe_set(node, "r", r)
    safe_set(node, "g", g)
    safe_set(node, "b", b)
    safe_set(node, "a", a)
    connect(source, output, node, "")
    return node


def binary_node(material, cls, lhs, rhs, x: int, y: int):
    node = expr(material, cls, x, y)
    connect(lhs, "", node, "A")
    connect(rhs, "", node, "B")
    return node


def unary_node(material, cls, source, x: int, y: int):
    node = expr(material, cls, x, y)
    connect(source, "", node, "")
    return node


def build_outline_wpo_graph(
    material,
    vertex_normal,
    vertex_color,
    outline_base_width,
    outline_reference_distance,
    outline_fov_tan_half,
    outline_reference_fov_tan_half,
    outline_depth_offset_scalar,
    x_offset: int = 0,
):
    world_position = expr(material, unreal.MaterialExpressionWorldPosition, x_offset - 1040, 40)
    camera_position = expr(material, unreal.MaterialExpressionCameraPositionWS, x_offset - 1040, 220)
    camera_minus_world = binary_node(material, unreal.MaterialExpressionSubtract, camera_position, world_position, x_offset - 800, 120)
    world_minus_camera = binary_node(material, unreal.MaterialExpressionSubtract, world_position, camera_position, x_offset - 800, 300)
    view_dir_to_camera = unary_node(material, unreal.MaterialExpressionNormalize, camera_minus_world, x_offset - 560, 120)
    camera_distance = unary_node(material, unreal.MaterialExpressionLength, world_minus_camera, x_offset - 560, 300)

    width_mul = component_mask(material, vertex_color, "", g=True, x=x_offset - 1040, y=560)
    depth_offset = component_mask(material, vertex_color, "", b=True, x=x_offset - 1040, y=720)

    distance_ratio = binary_node(material, unreal.MaterialExpressionDivide, camera_distance, outline_reference_distance, x_offset - 320, 300)
    fov_ratio = binary_node(material, unreal.MaterialExpressionDivide, outline_fov_tan_half, outline_reference_fov_tan_half, x_offset - 320, 460)
    width_from_vertex = binary_node(material, unreal.MaterialExpressionMultiply, outline_base_width, width_mul, x_offset - 80, 560)
    width_with_distance = binary_node(material, unreal.MaterialExpressionMultiply, width_from_vertex, distance_ratio, x_offset + 160, 480)
    effective_width = binary_node(material, unreal.MaterialExpressionMultiply, width_with_distance, fov_ratio, x_offset + 400, 420)
    normal_extrusion = binary_node(material, unreal.MaterialExpressionMultiply, vertex_normal, effective_width, x_offset + 640, 300)

    negative_one = constant(material, -1.0, x_offset - 320, -80)
    away_from_camera = binary_node(material, unreal.MaterialExpressionMultiply, view_dir_to_camera, negative_one, x_offset - 80, 100)
    depth_scalar = binary_node(material, unreal.MaterialExpressionMultiply, depth_offset, outline_depth_offset_scalar, x_offset + 160, 720)
    depth_wpo = binary_node(material, unreal.MaterialExpressionMultiply, away_from_camera, depth_scalar, x_offset + 400, 620)
    final_wpo = binary_node(material, unreal.MaterialExpressionAdd, normal_extrusion, depth_wpo, x_offset + 880, 420)
    return final_wpo


def create_character_master():
    default_inner_line = ensure_imported_texture(INNER_LINES_DEFAULT_BLACK_SOURCE, INNER_LINES_DEFAULT_BLACK)
    material = recreate_asset(CHAR_MASTER, unreal.Material, unreal.MaterialFactoryNew())
    safe_set(material, "shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    safe_set(material, "blend_mode", unreal.BlendMode.BLEND_OPAQUE)
    safe_set(material, "two_sided", False)
    safe_set(material, "used_with_static_lighting", False)
    safe_set(material, "used_with_instanced_static_meshes", True)

    base = texture_param(material, "BaseColorTexture", -1600, -420)
    tint = texture_param(material, "TintTexture", -1600, -140)
    inner_line = texture_param(material, "InnerLineTexture", -1600, 140, default_inner_line)
    inner_line_mask = component_mask(material, inner_line, "RGB", r=True, x=-1360, y=140)
    normal = expr(material, unreal.MaterialExpressionPixelNormalWS, -1600, 360)
    view = expr(material, unreal.MaterialExpressionCameraVectorWS, -1600, 600)
    vertex = expr(material, unreal.MaterialExpressionVertexColor, -1600, 860)
    threshold = component_mask(material, vertex, "", r=True, x=-1360, y=860)

    light = vector_param(material, "LightDirection", unreal.LinearColor(-0.4, 0.6, -0.7, 1.0), -1160, -540)
    ramp1 = scalar_param(material, "RampStep1", 0.0, -1160, -300)
    ramp2 = scalar_param(material, "RampStep2", 0.5, -1160, -120)
    shade = vector_param(material, "ShadeColor", unreal.LinearColor(0.35, 0.38, 0.50, 1.0), -1160, 80)
    mid = vector_param(material, "MidtoneColor", unreal.LinearColor(0.7, 0.7, 0.72, 1.0), -1160, 280)
    lit = vector_param(material, "LitColor", unreal.LinearColor(0.85, 0.85, 0.85, 1.0), -1160, 480)
    inner_line_color = vector_param(material, "InnerLineColor", unreal.LinearColor(0.0, 0.0, 0.0, 1.0), -1160, 680)
    inner_line_strength = scalar_param(material, "InnerLineStrength", 0.5, -1160, 840)
    rim_color = vector_param(material, "RimColor", unreal.LinearColor(1.0, 0.95, 0.85, 1.0), -1160, 1020)
    rim_power = scalar_param(material, "RimPower", 4.0, -1160, 1220)
    rim_strength = scalar_param(material, "RimStrength", 0.21, -1160, 1380)

    custom = expr(material, unreal.MaterialExpressionCustom, -520, 80)
    safe_set(custom, "description", "ToonCharacterShade")
    safe_set(custom, "output_type", unreal.CustomMaterialOutputType.CMOT_FLOAT3)
    set_custom_includes(custom, ["/Project/ToonStyle/ToonShadingCommon.ush"])
    set_custom_inputs(
        custom,
        [
            "BaseColor",
            "TintColor",
            "InnerLineMask",
            "InnerLineColor",
            "InnerLineStrength",
            "N",
            "L",
            "V",
            "ThresholdOffset",
            "RampStep1",
            "RampStep2",
            "ShadeColor",
            "MidtoneColor",
            "LitColor",
            "RimColor",
            "RimPower",
            "RimStrength",
        ],
    )
    safe_set(
        custom,
        "code",
        "return ToonCharacterShade(BaseColor, max(TintColor, float3(0.4, 0.4, 0.4)), InnerLineMask, InnerLineColor, InnerLineStrength, N, L, V, ThresholdOffset, RampStep1, RampStep2, ShadeColor, MidtoneColor, LitColor, RimColor, RimPower, RimStrength);",
    )

    connect(base, "RGB", custom, "BaseColor")
    connect(tint, "RGB", custom, "TintColor")
    connect(inner_line_mask, "", custom, "InnerLineMask")
    connect(inner_line_color, "", custom, "InnerLineColor")
    connect(inner_line_strength, "", custom, "InnerLineStrength")
    connect(normal, "", custom, "N")
    connect(light, "", custom, "L")
    connect(view, "", custom, "V")
    connect(threshold, "", custom, "ThresholdOffset")
    connect(ramp1, "", custom, "RampStep1")
    connect(ramp2, "", custom, "RampStep2")
    connect(shade, "", custom, "ShadeColor")
    connect(mid, "", custom, "MidtoneColor")
    connect(lit, "", custom, "LitColor")
    connect(rim_color, "", custom, "RimColor")
    connect(rim_power, "", custom, "RimPower")
    connect(rim_strength, "", custom, "RimStrength")
    mel.connect_material_property(custom, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)

    mel.layout_material_expressions(material)
    mel.recompile_material(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material)
    return material


def create_environment_master():
    material = recreate_asset(ENV_MASTER, unreal.Material, unreal.MaterialFactoryNew())
    safe_set(material, "shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    safe_set(material, "blend_mode", unreal.BlendMode.BLEND_OPAQUE)
    safe_set(material, "two_sided", False)
    safe_set(material, "used_with_instanced_static_meshes", True)
    safe_set(material, "used_with_nanite", True)

    uv = expr(material, unreal.MaterialExpressionTextureCoordinate, -2080, -420)
    uv_u = component_mask(material, uv, "", r=True, x=-1840, y=-520)
    uv_v = component_mask(material, uv, "", g=True, x=-1840, y=-300)
    tile_u = scalar_param(material, "UVTileU", 10.0, -1840, -80)
    tile_v = scalar_param(material, "UVTileV", 10.0, -1840, 120)
    u_mul = expr(material, unreal.MaterialExpressionMultiply, -1580, -480)
    v_mul = expr(material, unreal.MaterialExpressionMultiply, -1580, -260)
    connect(uv_u, "", u_mul, "A")
    connect(tile_u, "", u_mul, "B")
    connect(uv_v, "", v_mul, "A")
    connect(tile_v, "", v_mul, "B")
    uv_tiled = expr(material, unreal.MaterialExpressionAppendVector, -1320, -360)
    connect(u_mul, "", uv_tiled, "A")
    connect(v_mul, "", uv_tiled, "B")

    # World-space projection conventions for bUseWorldSpaceUVs=true:
    # (1, 1, 0) projects floors/ceilings onto XY and ignores Z.
    # (1, 0, 1) projects X-running walls onto XZ and ignores Y.
    # (0, 1, 1) projects Y-running walls onto YZ and ignores X.
    world_position = expr(material, unreal.MaterialExpressionWorldPosition, -1600, 420)
    projection_axes = vector_param(material, "ProjectionAxes", unreal.LinearColor(1.0, 1.0, 0.0, 0.0), -1600, 620)
    world_tile_size = scalar_param(material, "WorldSpaceTileSize", 100.0, -1600, 820)
    world_uv = expr(material, unreal.MaterialExpressionCustom, -1320, 560)
    safe_set(world_uv, "description", "ToonEnvironmentWorldSpaceUV")
    safe_set(world_uv, "output_type", unreal.CustomMaterialOutputType.CMOT_FLOAT2)
    set_custom_inputs(world_uv, ["WorldPosition", "ProjectionAxes", "WorldSpaceTileSize"])
    safe_set(
        world_uv,
        "code",
        "float TileSize = max(WorldSpaceTileSize, 1.0);\n"
        "float3 Axes = step(0.5, ProjectionAxes.xyz);\n"
        "float U = (Axes.x > 0.5) ? WorldPosition.x : WorldPosition.y;\n"
        "float V = (Axes.z > 0.5) ? WorldPosition.z : WorldPosition.y;\n"
        "return float2(U, V) / TileSize;",
    )
    connect(world_position, "", world_uv, "WorldPosition")
    connect(projection_axes, "", world_uv, "ProjectionAxes")
    connect(world_tile_size, "", world_uv, "WorldSpaceTileSize")

    final_uv = static_switch_param(material, "bUseWorldSpaceUVs", False, uv_tiled, world_uv, -1080, -80)

    base = texture_param(material, "BaseColorTexture", -840, -300)
    connect(final_uv, "", base, "UVs")
    normal = expr(material, unreal.MaterialExpressionPixelNormalWS, -840, 40)
    light = vector_param(material, "LightDirection", unreal.LinearColor(-0.4, 0.6, -0.7, 1.0), -840, 240)
    ramp1 = scalar_param(material, "RampStep1", 0.0, -840, 440)
    ramp2 = scalar_param(material, "RampStep2", 0.5, -840, 600)
    shade = vector_param(material, "EnvShadeColor", unreal.LinearColor(0.3, 0.32, 0.42, 1.0), -840, 780)
    mid = vector_param(material, "EnvMidtoneColor", unreal.LinearColor(0.55, 0.58, 0.62, 1.0), -840, 980)
    lit = vector_param(material, "EnvLitColor", unreal.LinearColor(0.85, 0.85, 0.90, 1.0), -840, 1180)

    custom = expr(material, unreal.MaterialExpressionCustom, -180, 220)
    safe_set(custom, "description", "ToonEnvironmentShade")
    safe_set(custom, "output_type", unreal.CustomMaterialOutputType.CMOT_FLOAT3)
    set_custom_includes(custom, ["/Project/ToonStyle/ToonShadingCommon.ush"])
    set_custom_inputs(custom, ["BaseColor", "N", "L", "RampStep1", "RampStep2", "ShadeColor", "MidtoneColor", "LitColor"])
    safe_set(custom, "code", "return ToonEnvironmentShade(BaseColor, N, L, RampStep1, RampStep2, ShadeColor, MidtoneColor, LitColor);")
    connect(base, "RGB", custom, "BaseColor")
    connect(normal, "", custom, "N")
    connect(light, "", custom, "L")
    connect(ramp1, "", custom, "RampStep1")
    connect(ramp2, "", custom, "RampStep2")
    connect(shade, "", custom, "ShadeColor")
    connect(mid, "", custom, "MidtoneColor")
    connect(lit, "", custom, "LitColor")
    mel.connect_material_property(custom, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)

    mel.layout_material_expressions(material)
    mel.recompile_material(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material)
    return material


def create_outline_master(path: str = OUTLINE_MASTER):
    material = recreate_asset(path, unreal.Material, unreal.MaterialFactoryNew())
    safe_set(material, "shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    safe_set(material, "blend_mode", unreal.BlendMode.BLEND_OPAQUE)
    safe_set(material, "two_sided", False)
    safe_set(material, "used_with_instanced_static_meshes", True)
    safe_set(material, "used_with_nanite", True)

    outline_color = vector_param(material, "OutlineColor", unreal.LinearColor(0.0, 0.0, 0.0, 1.0), -1240, -520)
    outline_base_width = scalar_param(material, "OutlineBaseWidth", 1.5, -1240, -300)
    outline_reference_distance = scalar_param(material, "OutlineReferenceDistance", 1500.0, -1240, -140)
    outline_fov_tan_half = scalar_param(material, "OutlineFOVTanHalf", 1.0, -1240, 20)
    outline_reference_fov_tan_half = scalar_param(material, "OutlineReferenceFOVTanHalf", 1.0, -1240, 180)
    outline_depth_offset_scalar = scalar_param(material, "OutlineDepthOffsetScalar", 2.5, -1240, 340)
    # Legacy compatibility parameter. Runtime C++ sets both this and OutlineBaseWidth;
    # production graph consumes OutlineBaseWidth.
    scalar_param(material, "OutlineWidth", 1.5, -1240, 500)

    vertex_normal = expr(material, unreal.MaterialExpressionVertexNormalWS, -1240, 760)
    vertex = expr(material, unreal.MaterialExpressionVertexColor, -1240, 980)
    wpo = build_outline_wpo_graph(
        material,
        vertex_normal,
        vertex,
        outline_base_width,
        outline_reference_distance,
        outline_fov_tan_half,
        outline_reference_fov_tan_half,
        outline_depth_offset_scalar,
        x_offset=40,
    )

    mel.connect_material_property(wpo, "", unreal.MaterialProperty.MP_WORLD_POSITION_OFFSET)
    mel.connect_material_property(outline_color, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)

    mel.layout_material_expressions(material)
    mel.recompile_material(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material)
    return material


def create_masters() -> dict[str, str]:
    ensure_dir(TOON_DIR)
    create_character_master()
    create_environment_master()
    create_outline_master(OUTLINE_MASTER)
    result = {
        "character_master": CHAR_MASTER,
        "environment_master": ENV_MASTER,
        "outline_master": OUTLINE_MASTER,
        "outline_backface_strategy": "Reversed-winding outline mesh with opaque one-sided material.",
        "outline_distance_strategy": "Phase 1C R1 graph-distance formula: OutlineBaseWidth * VertexColor.G * (CameraDistance / OutlineReferenceDistance) * (OutlineFOVTanHalf / OutlineReferenceFOVTanHalf).",
        "outline_depth_strategy": "Phase 1C R1 graph-distance view direction: -normalize(CameraPositionWS - WorldPosition) * VertexColor.B * OutlineDepthOffsetScalar.",
        "inner_lines_default_black": INNER_LINES_DEFAULT_BLACK,
        "vertex_color_a_mask": "Unused in WPO after Diagnostic2 proved UE5.7 strips alpha in WPO paths. Mask is folded into VertexColor.G.",
    }
    (OUT_DIR / "phase1c_material_masters_verify.json").write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    log(json.dumps(result, sort_keys=True))
    return result


def load_asset(path: str):
    asset = unreal.EditorAssetLibrary.load_asset(path)
    if not asset:
        raise RuntimeError(f"Missing asset {path}")
    return asset


def texture_for_asset(asset_name: str, target_dir: str):
    texture_dir = f"{target_dir}/Textures"
    for path in (f"{texture_dir}/T_{asset_name}", f"{texture_dir}/T_{asset_name}_0"):
        texture = unreal.EditorAssetLibrary.load_asset(path)
        if texture:
            return texture
    raise RuntimeError(f"Missing texture for {asset_name} under {texture_dir}")


def create_material_instance(
    path: str,
    parent_path: str,
    texture=None,
    uv_tile_u: float | None = None,
    uv_tile_v: float | None = None,
    use_world_space_uvs: bool | None = None,
    projection_axes: unreal.LinearColor | None = None,
    world_space_tile_size: float | None = None,
    outline_color: unreal.LinearColor | None = None,
):
    instance = recreate_asset(path, unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
    parent = load_asset(parent_path)
    safe_set(instance, "parent", parent)
    if texture:
        mel.set_material_instance_texture_parameter_value(instance, "BaseColorTexture", texture)
    if uv_tile_u is not None:
        mel.set_material_instance_scalar_parameter_value(instance, "UVTileU", float(uv_tile_u))
    if uv_tile_v is not None:
        mel.set_material_instance_scalar_parameter_value(instance, "UVTileV", float(uv_tile_v))
    if use_world_space_uvs is not None:
        mel.set_material_instance_static_switch_parameter_value(instance, "bUseWorldSpaceUVs", bool(use_world_space_uvs))
    if projection_axes is not None:
        mel.set_material_instance_vector_parameter_value(instance, "ProjectionAxes", projection_axes)
    if world_space_tile_size is not None:
        mel.set_material_instance_scalar_parameter_value(instance, "WorldSpaceTileSize", float(world_space_tile_size))
    if outline_color is not None:
        mel.set_material_instance_vector_parameter_value(instance, "OutlineColor", outline_color)
    mel.update_material_instance(instance)
    unreal.EditorAssetLibrary.save_loaded_asset(instance)
    return instance


def outline_color_for(asset_name: str) -> tuple[str, unreal.LinearColor]:
    colors = {
        "black": unreal.LinearColor(0.0, 0.0, 0.0, 1.0),
        "red": unreal.LinearColor(1.0, 0.0, 0.0, 1.0),
        "yellow": unreal.LinearColor(1.0, 0.84, 0.0, 1.0),
        "white": unreal.LinearColor(1.0, 1.0, 1.0, 1.0),
    }
    mapping = {
        "lubu_validation": "black",
        "slime": "black",
        "arcademachine": "black",
        "aria": "red",
        "tombspider": "red",
        "lootchest": "red",
        "gambler": "yellow",
        "cavebat": "yellow",
        "lootbag_yellow": "yellow",
        "idolaltar": "white",
        "lootcrate": "white",
    }
    tier = mapping[asset_name]
    return tier, colors[tier]


def create_instances() -> dict[str, object]:
    asset_specs = [
        ("lubu_validation", "/Game/ToonStyle/TestAssets/Validation", "/Game/ToonStyle/TestAssets/Validation/Materials/MI_lubu_validation", False),
        ("aria", "/Game/ToonStyle/TestAssets/Lineup", "/Game/ToonStyle/TestAssets/Lineup/Materials/MI_aria", False),
        ("gambler", "/Game/ToonStyle/TestAssets/Lineup", "/Game/ToonStyle/TestAssets/Lineup/Materials/MI_gambler", False),
        ("slime", "/Game/ToonStyle/TestAssets/Lineup", "/Game/ToonStyle/TestAssets/Lineup/Materials/MI_slime", True),
        ("tombspider", "/Game/ToonStyle/TestAssets/Lineup", "/Game/ToonStyle/TestAssets/Lineup/Materials/MI_tombspider", False),
        ("cavebat", "/Game/ToonStyle/TestAssets/Lineup", "/Game/ToonStyle/TestAssets/Lineup/Materials/MI_cavebat", False),
        ("idolaltar", "/Game/ToonStyle/TestAssets/Lineup", "/Game/ToonStyle/TestAssets/Lineup/Materials/MI_idolaltar", False),
        ("arcademachine", "/Game/ToonStyle/TestAssets/Lineup", "/Game/ToonStyle/TestAssets/Lineup/Materials/MI_arcademachine", False),
        ("lootchest", "/Game/ToonStyle/TestAssets/Lineup", "/Game/ToonStyle/TestAssets/Lineup/Materials/MI_lootchest", False),
        ("lootbag_yellow", "/Game/ToonStyle/TestAssets/Lineup", "/Game/ToonStyle/TestAssets/Lineup/Materials/MI_lootbag_yellow", False),
        ("lootcrate", "/Game/ToonStyle/TestAssets/Lineup", "/Game/ToonStyle/TestAssets/Lineup/Materials/MI_lootcrate", False),
    ]

    created = []
    for asset_name, target_dir, material_path, retained in asset_specs:
        texture = texture_for_asset(asset_name, target_dir)
        mi = create_material_instance(material_path, CHAR_MASTER, texture)
        outline_path = f"{material_path}_Outline"
        color_name, color = outline_color_for(asset_name)
        outline_mi = create_material_instance(outline_path, OUTLINE_MASTER, outline_color=color)

        mesh = unreal.EditorAssetLibrary.load_asset(f"{target_dir}/SM_{asset_name}")
        if mesh and isinstance(mesh, unreal.StaticMesh):
            mesh.set_material(0, mi)
            unreal.EditorAssetLibrary.save_loaded_asset(mesh)
        outline_mesh = unreal.EditorAssetLibrary.load_asset(f"{target_dir}/SM_{asset_name}_Outline")
        if outline_mesh and isinstance(outline_mesh, unreal.StaticMesh):
            outline_mesh.set_material(0, outline_mi)
            unreal.EditorAssetLibrary.save_loaded_asset(outline_mesh)

        created.append(
            {
                "asset": asset_name,
                "retained_from_phase1a": retained,
                "material": material_path,
                "outline_material": outline_path,
                "outline_color": color_name,
                "texture": texture.get_path_name(),
            }
        )

    wall_texture = load_asset("/Game/ToonStyle/TestAssets/Environment/Textures/T_TestRoom_Wall")
    floor_texture = load_asset("/Game/ToonStyle/TestAssets/Environment/Textures/T_TestRoom_Floor")
    wall = create_material_instance("/Game/ToonStyle/TestAssets/Environment/Materials/MI_TestRoom_Wall", ENV_MASTER, wall_texture, 20.0, 2.0)
    ceiling = create_material_instance("/Game/ToonStyle/TestAssets/Environment/Materials/MI_TestRoom_Ceiling", ENV_MASTER, wall_texture, 20.0, 2.0)
    floor = create_material_instance("/Game/ToonStyle/TestAssets/Environment/Materials/MI_TestRoom_Floor", ENV_MASTER, floor_texture, 10.0, 10.0)

    result = {
        "character_instances": created,
        "environment_instances": [
            {"path": wall.get_path_name(), "UVTileU": 20.0, "UVTileV": 2.0},
            {"path": ceiling.get_path_name(), "UVTileU": 20.0, "UVTileV": 2.0},
            {"path": floor.get_path_name(), "UVTileU": 10.0, "UVTileV": 10.0},
        ],
    }
    (OUT_DIR / "phase1c_material_instances_verify.json").write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    log(json.dumps(result, sort_keys=True))
    unreal.EditorAssetLibrary.save_directory("/Game/ToonStyle", only_if_is_dirty=False, recursive=True)
    return result


def create_environment_kit_instances() -> dict[str, object]:
    wall_texture = load_asset(TESTROOM_WALL_TEXTURE)
    floor_texture = load_asset(TESTROOM_FLOOR_TEXTURE)
    surfaces = [
        ("Wall_XZ", wall_texture, unreal.LinearColor(1.0, 0.0, 1.0, 0.0), "wall texture placeholder"),
        ("Wall_YZ", wall_texture, unreal.LinearColor(0.0, 1.0, 1.0, 0.0), "wall texture placeholder"),
        ("Floor", floor_texture, unreal.LinearColor(1.0, 1.0, 0.0, 0.0), "floor texture placeholder"),
        ("Ceiling", wall_texture, unreal.LinearColor(1.0, 1.0, 0.0, 0.0), "ceiling placeholder reuses test-room wall texture"),
    ]

    created = []
    placeholder_lines = [
        "# T66 Map Transition Placeholder Texture Manifest",
        "",
        "All production environment kit MIs currently bind existing test-room textures so the rectangle path can land before final texture authoring.",
        "Pablo can replace these by updating the MI `BaseColorTexture` bindings or replacing assets in a later content pass.",
        "",
    ]
    for theme in ENVIRONMENT_KIT_THEMES:
        ensure_dir(f"{ENVIRONMENT_KIT_DIR}/{theme}/Materials")
        ensure_dir(f"{ENVIRONMENT_KIT_DIR}/{theme}/Textures")
        placeholder_lines.append(f"## {theme}")
        for suffix, texture, axes, note in surfaces:
            material_path = f"{ENVIRONMENT_KIT_DIR}/{theme}/Materials/MI_{theme}_{suffix}"
            instance = create_material_instance(
                material_path,
                ENV_MASTER,
                texture=texture,
                use_world_space_uvs=True,
                projection_axes=axes,
                world_space_tile_size=300.0,
            )
            created.append(
                {
                    "theme": theme,
                    "surface": suffix,
                    "material": instance.get_path_name(),
                    "texture": texture.get_path_name(),
                    "bUseWorldSpaceUVs": True,
                    "ProjectionAxes": [axes.r, axes.g, axes.b, axes.a],
                    "WorldSpaceTileSize": 300.0,
                    "placeholder_note": note,
                }
            )
            placeholder_lines.append(f"- `{instance.get_path_name()}` -> `{texture.get_path_name()}` ({note}; SHARED TEST-ROOM TEXTURE, REPLACE ONLY IF VISUAL TUNING CALLS FOR IT)")
        placeholder_lines.append("")

    result = {
        "environment_kit_material_instances": created,
        "texture_strategy": "All themes intentionally reuse existing test-room wall/floor textures for this implementation pass.",
    }
    (OUT_DIR / "environment_kit_material_instances_verify.json").write_text(
        json.dumps(result, indent=2, sort_keys=True) + "\n",
        encoding="utf-8")
    (MAP_TRANSITION_DIR / "PlaceholderTextureManifest.md").write_text(
        "\n".join(placeholder_lines).rstrip() + "\n",
        encoding="utf-8")
    unreal.EditorAssetLibrary.save_directory(ENVIRONMENT_KIT_DIR, only_if_is_dirty=False, recursive=True)
    log(json.dumps(result, sort_keys=True))
    return result


def create_distance_test_material() -> dict[str, str]:
    create_outline_master(DISTANCE_TEST_MATERIAL)
    result = {
        "material": DISTANCE_TEST_MATERIAL,
        "status": "compiled",
        "note": "Disposable CameraOffset fallback outline material compiled; production outline master uses the same built-in function path plus normal extrusion.",
    }
    (OUT_DIR / "phase1c_outline_distance_test_verify.json").write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    log(json.dumps(result, sort_keys=True))
    return result


def create_vertex_color_b_diagnostic_material() -> dict[str, str]:
    material = recreate_asset(VERTEX_B_DIAGNOSTIC_MATERIAL, unreal.Material, unreal.MaterialFactoryNew())
    safe_set(material, "shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    safe_set(material, "blend_mode", unreal.BlendMode.BLEND_OPAQUE)
    safe_set(material, "two_sided", False)
    safe_set(material, "used_with_instanced_static_meshes", True)
    safe_set(material, "used_with_nanite", True)

    outline_color = vector_param(material, "OutlineColor", unreal.LinearColor(1.0, 0.0, 0.0, 1.0), -1240, -520)
    world_position = expr(material, unreal.MaterialExpressionWorldPosition, -1240, -120)
    camera_position = expr(material, unreal.MaterialExpressionCameraPositionWS, -1240, 60)
    camera_minus_world = binary_node(material, unreal.MaterialExpressionSubtract, camera_position, world_position, -980, 0)
    view_dir_to_camera = unary_node(material, unreal.MaterialExpressionNormalize, camera_minus_world, -720, 0)
    negative_one = constant(material, -1.0, -720, 180)
    away_from_camera = binary_node(material, unreal.MaterialExpressionMultiply, view_dir_to_camera, negative_one, -460, 40)
    vertex = expr(material, unreal.MaterialExpressionVertexColor, -1240, 420)
    vertex_b = component_mask(material, vertex, "", b=True, x=-980, y=420)
    half = constant(material, 0.5, -980, 580)
    depth_amount = binary_node(material, unreal.MaterialExpressionMultiply, vertex_b, half, -720, 500)
    wpo = binary_node(material, unreal.MaterialExpressionMultiply, away_from_camera, depth_amount, -180, 180)

    mel.connect_material_property(wpo, "", unreal.MaterialProperty.MP_WORLD_POSITION_OFFSET)
    mel.connect_material_property(outline_color, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    mel.layout_material_expressions(material)
    mel.recompile_material(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material)

    result = {
        "material": VERTEX_B_DIAGNOSTIC_MATERIAL,
        "status": "compiled_in_editor",
        "wpo": "-normalize(CameraPositionWS - WorldPosition) * VertexColor.B * 0.5",
    }
    out = Path(r"C:\UE\T66\Saved\Codex\ToonStyle\Phase1C\RemediationR1")
    out.mkdir(parents=True, exist_ok=True)
    (out / "vertex_color_b_pregate_material.json").write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    log(json.dumps(result, sort_keys=True))
    return result


def delete_distance_test_material() -> None:
    if unreal.EditorAssetLibrary.does_asset_exist(DISTANCE_TEST_MATERIAL):
        unreal.EditorAssetLibrary.delete_asset(DISTANCE_TEST_MATERIAL)
    if unreal.EditorAssetLibrary.does_asset_exist("/Game/ToonStyle/Materials/M_CameraOffsetProbe"):
        unreal.EditorAssetLibrary.delete_asset("/Game/ToonStyle/Materials/M_CameraOffsetProbe")
    if unreal.EditorAssetLibrary.does_asset_exist(VERTEX_B_DIAGNOSTIC_MATERIAL):
        unreal.EditorAssetLibrary.delete_asset(VERTEX_B_DIAGNOSTIC_MATERIAL)
    if unreal.EditorAssetLibrary.does_directory_exist(DIAGNOSTIC_DIR):
        unreal.EditorAssetLibrary.delete_directory(DIAGNOSTIC_DIR)


def main() -> int:
    mode = os.environ.get("T66_PHASE1C_TOON_MODE", "all").lower()
    result = {}
    if mode in {"r1_vertex_b_pregate", "vertex_b_pregate"}:
        result["vertex_b_pregate"] = create_vertex_color_b_diagnostic_material()
    if mode in {"distance_test", "all"}:
        result["distance_test"] = create_distance_test_material()
    if mode in {"outline_master", "r1_outline_master"}:
        result["outline_master"] = {"outline_master": create_outline_master(OUTLINE_MASTER).get_path_name()}
    if mode in {"masters", "all"}:
        result["masters"] = create_masters()
    if mode in {"environment_master", "env_master"}:
        result["environment_master"] = {"environment_master": create_environment_master().get_path_name()}
    if mode in {"instances", "all"}:
        result["instances"] = create_instances()
    if mode in {"environment_kit_instances", "env_kit_instances", "all"}:
        result["environment_kit_instances"] = create_environment_kit_instances()
    if mode in {"delete_distance_test", "all"}:
        delete_distance_test_material()
    (OUT_DIR / "phase1c_toon_material_setup_verify.json").write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    if os.environ.get("T66_PHASE1C_QUIT_EDITOR", "1") == "1":
        unreal.SystemLibrary.quit_editor()
    return 0


main()
