"""Create Phase 1B ToonStyle master materials and material instances.

This script is intentionally kept as text-source for the binary material assets.
Run with UnrealEditor.exe -ExecutePythonScript and set T66_PHASE1B_TOON_MODE:
  masters   - create M_Toon_Character, M_Toon_Environment, M_Toon_Character_Outline
  instances - create/reparent Phase 1B material instances
  all       - both
"""

from __future__ import annotations

import json
import os
from pathlib import Path

import unreal


OUT_DIR = Path(r"C:\UE\T66\Saved\Codex\ToonStyle\Phase1B")
OUT_DIR.mkdir(parents=True, exist_ok=True)

TOON_DIR = "/Game/ToonStyle/Materials"
CHAR_MASTER = f"{TOON_DIR}/M_Toon_Character"
ENV_MASTER = f"{TOON_DIR}/M_Toon_Environment"
OUTLINE_MASTER = f"{TOON_DIR}/M_Toon_Character_Outline"

mel = unreal.MaterialEditingLibrary
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()


def log(message: str) -> None:
    unreal.log(f"[Phase1B][Materials] {message}")


def safe_set(obj, prop: str, value) -> None:
    try:
        obj.set_editor_property(prop, value)
    except Exception as exc:
        unreal.log_warning(f"[Phase1B][Materials] Could not set {prop} on {obj}: {exc}")


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
    # IncludeFilePaths are emitted before Unreal wraps Custom node code in the
    # generated function, and are tracked for cooked shader-map dependencies.
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


def constant(material, value: float, x: int, y: int):
    node = expr(material, unreal.MaterialExpressionConstant, x, y)
    safe_set(node, "r", float(value))
    return node


def texture_param(material, name: str, x: int, y: int):
    node = expr(material, unreal.MaterialExpressionTextureSampleParameter2D, x, y)
    safe_set(node, "parameter_name", name)
    return node


def component_mask(material, source, output: str, r=False, g=False, b=False, a=False, x=0, y=0):
    node = expr(material, unreal.MaterialExpressionComponentMask, x, y)
    safe_set(node, "r", r)
    safe_set(node, "g", g)
    safe_set(node, "b", b)
    safe_set(node, "a", a)
    connect(source, output, node, "")
    return node


def create_character_master():
    material = recreate_asset(CHAR_MASTER, unreal.Material, unreal.MaterialFactoryNew())
    safe_set(material, "shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    safe_set(material, "blend_mode", unreal.BlendMode.BLEND_OPAQUE)
    safe_set(material, "two_sided", False)
    safe_set(material, "used_with_static_lighting", False)
    safe_set(material, "used_with_instanced_static_meshes", True)

    base = texture_param(material, "BaseColorTexture", -1600, -420)
    tint = texture_param(material, "TintTexture", -1600, -140)
    normal = expr(material, unreal.MaterialExpressionPixelNormalWS, -1600, 180)
    view = expr(material, unreal.MaterialExpressionCameraVectorWS, -1600, 420)
    vertex = expr(material, unreal.MaterialExpressionVertexColor, -1600, 680)
    threshold = component_mask(material, vertex, "", r=True, x=-1360, y=680)

    light = vector_param(material, "LightDirection", unreal.LinearColor(-0.4, 0.6, -0.7, 1.0), -1160, -540)
    ramp1 = scalar_param(material, "RampStep1", 0.0, -1160, -300)
    ramp2 = scalar_param(material, "RampStep2", 0.5, -1160, -120)
    shade = vector_param(material, "ShadeColor", unreal.LinearColor(0.35, 0.38, 0.50, 1.0), -1160, 80)
    mid = vector_param(material, "MidtoneColor", unreal.LinearColor(0.7, 0.7, 0.72, 1.0), -1160, 280)
    lit = vector_param(material, "LitColor", unreal.LinearColor(1.0, 1.0, 1.0, 1.0), -1160, 480)
    rim_color = vector_param(material, "RimColor", unreal.LinearColor(1.0, 0.95, 0.85, 1.0), -1160, 680)
    rim_power = scalar_param(material, "RimPower", 4.0, -1160, 880)
    rim_strength = scalar_param(material, "RimStrength", 0.3, -1160, 1040)

    custom = expr(material, unreal.MaterialExpressionCustom, -520, 80)
    safe_set(custom, "description", "ToonCharacterShade")
    safe_set(custom, "output_type", unreal.CustomMaterialOutputType.CMOT_FLOAT3)
    set_custom_includes(custom, ["/Project/ToonStyle/ToonShadingCommon.ush"])
    set_custom_inputs(
        custom,
        [
            "BaseColor",
            "TintColor",
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
        "return ToonCharacterShade(BaseColor, max(TintColor, float3(0.5, 0.5, 0.5)), N, L, V, ThresholdOffset, RampStep1, RampStep2, ShadeColor, MidtoneColor, LitColor, RimColor, RimPower, RimStrength);",
    )

    connect(base, "RGB", custom, "BaseColor")
    connect(tint, "RGB", custom, "TintColor")
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

    uv = expr(material, unreal.MaterialExpressionTextureCoordinate, -1760, -380)
    tile = scalar_param(material, "UVTileScale", 10.0, -1760, -160)
    uv_mul = expr(material, unreal.MaterialExpressionMultiply, -1500, -300)
    connect(uv, "", uv_mul, "A")
    connect(tile, "", uv_mul, "B")

    base = texture_param(material, "BaseColorTexture", -1240, -300)
    connect(uv_mul, "", base, "UVs")
    normal = expr(material, unreal.MaterialExpressionPixelNormalWS, -1240, 40)
    light = vector_param(material, "LightDirection", unreal.LinearColor(-0.4, 0.6, -0.7, 1.0), -1240, 240)
    ramp1 = scalar_param(material, "RampStep1", 0.0, -1240, 440)
    ramp2 = scalar_param(material, "RampStep2", 0.5, -1240, 600)
    shade = vector_param(material, "EnvShadeColor", unreal.LinearColor(0.3, 0.32, 0.42, 1.0), -1240, 780)
    mid = vector_param(material, "EnvMidtoneColor", unreal.LinearColor(0.55, 0.58, 0.62, 1.0), -1240, 980)
    lit = vector_param(material, "EnvLitColor", unreal.LinearColor(0.85, 0.85, 0.90, 1.0), -1240, 1180)

    custom = expr(material, unreal.MaterialExpressionCustom, -520, 220)
    safe_set(custom, "description", "ToonEnvironmentShade")
    safe_set(custom, "output_type", unreal.CustomMaterialOutputType.CMOT_FLOAT3)
    set_custom_includes(custom, ["/Project/ToonStyle/ToonShadingCommon.ush"])
    set_custom_inputs(custom, ["BaseColor", "N", "L", "RampStep1", "RampStep2", "ShadeColor", "MidtoneColor", "LitColor"])
    safe_set(
        custom,
        "code",
        "return ToonEnvironmentShade(BaseColor, N, L, RampStep1, RampStep2, ShadeColor, MidtoneColor, LitColor);",
    )
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


def create_outline_master():
    material = recreate_asset(OUTLINE_MASTER, unreal.Material, unreal.MaterialFactoryNew())
    safe_set(material, "shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    safe_set(material, "blend_mode", unreal.BlendMode.BLEND_MASKED)
    safe_set(material, "two_sided", True)
    safe_set(material, "used_with_instanced_static_meshes", True)
    safe_set(material, "used_with_nanite", True)

    outline_color = vector_param(material, "OutlineColor", unreal.LinearColor(0.0, 0.0, 0.0, 1.0), -1240, -320)
    outline_width = scalar_param(material, "OutlineWidth", 1.5, -1240, -80)
    vertex_normal = expr(material, unreal.MaterialExpressionVertexNormalWS, -1240, 240)
    vertex = expr(material, unreal.MaterialExpressionVertexColor, -1240, 760)
    width_mul = component_mask(material, vertex, "", g=True, x=-980, y=700)
    depth_offset = component_mask(material, vertex, "", b=True, x=-980, y=860)
    mask = constant(material, 1.0, -980, 1020)

    wpo = expr(material, unreal.MaterialExpressionCustom, -520, 260)
    safe_set(wpo, "description", "ToonOutlineWPO")
    safe_set(wpo, "output_type", unreal.CustomMaterialOutputType.CMOT_FLOAT3)
    set_custom_includes(wpo, ["/Project/ToonStyle/ToonOutline.ush"])
    set_custom_inputs(wpo, ["GeometricNormal", "BaseWidth", "VertexWidthMul", "VertexDepthOffset", "VertexOutlineMask"])
    safe_set(
        wpo,
        "code",
        "return ToonOutlineWorldPositionOffset(GeometricNormal, BaseWidth, VertexWidthMul, VertexDepthOffset, VertexOutlineMask);",
    )
    connect(vertex_normal, "", wpo, "GeometricNormal")
    connect(outline_width, "", wpo, "BaseWidth")
    connect(width_mul, "", wpo, "VertexWidthMul")
    connect(depth_offset, "", wpo, "VertexDepthOffset")
    connect(mask, "", wpo, "VertexOutlineMask")
    mel.connect_material_property(wpo, "", unreal.MaterialProperty.MP_WORLD_POSITION_OFFSET)
    mel.connect_material_property(outline_color, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)

    sign = expr(material, unreal.MaterialExpressionTwoSidedSign, -520, 720)
    opacity = expr(material, unreal.MaterialExpressionCustom, -260, 720)
    safe_set(opacity, "description", "BackFaceMask")
    safe_set(opacity, "output_type", unreal.CustomMaterialOutputType.CMOT_FLOAT1)
    set_custom_inputs(opacity, ["Sign", "Mask"])
    safe_set(opacity, "code", "return (Sign < 0.0) ? saturate(Mask) : 0.0;")
    connect(sign, "", opacity, "Sign")
    connect(mask, "", opacity, "Mask")
    mel.connect_material_property(opacity, "", unreal.MaterialProperty.MP_OPACITY_MASK)

    mel.layout_material_expressions(material)
    mel.recompile_material(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material)
    return material


def create_masters() -> dict[str, str]:
    ensure_dir(TOON_DIR)
    create_character_master()
    create_environment_master()
    create_outline_master()
    result = {
        "character_master": CHAR_MASTER,
        "environment_master": ENV_MASTER,
        "outline_master": OUTLINE_MASTER,
        "outline_backface_strategy": "TwoSided masked material using TwoSidedSign; avoids unsupported front-cull automation.",
    }
    (OUT_DIR / "gate_g2_verify.json").write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    log(json.dumps(result, sort_keys=True))
    return result


def load_asset(path: str):
    asset = unreal.EditorAssetLibrary.load_asset(path)
    if not asset:
        raise RuntimeError(f"Missing asset {path}")
    return asset


def texture_for_asset(asset_name: str, target_dir: str):
    texture_dir = f"{target_dir}/Textures"
    primary = f"{texture_dir}/T_{asset_name}"
    numbered = f"{texture_dir}/T_{asset_name}_0"
    for path in (primary, numbered):
        tex = unreal.EditorAssetLibrary.load_asset(path)
        if tex:
            return tex
    raise RuntimeError(f"Missing texture for {asset_name} under {texture_dir}")


def create_material_instance(path: str, parent_path: str, texture=None, uv_tile_scale: float | None = None):
    instance = recreate_asset(path, unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
    parent = load_asset(parent_path)
    safe_set(instance, "parent", parent)
    if texture:
        mel.set_material_instance_texture_parameter_value(instance, "BaseColorTexture", texture)
    if uv_tile_scale is not None:
        mel.set_material_instance_scalar_parameter_value(instance, "UVTileScale", float(uv_tile_scale))
    mel.update_material_instance(instance)
    unreal.EditorAssetLibrary.save_loaded_asset(instance)
    return instance


def create_instances() -> dict[str, object]:
    asset_specs = [
        ("lubu_validation", "/Game/ToonStyle/TestAssets/Validation", "/Game/ToonStyle/TestAssets/Validation/Materials/MI_lubu_validation"),
        ("aria", "/Game/ToonStyle/TestAssets/Lineup", "/Game/ToonStyle/TestAssets/Lineup/Materials/MI_aria"),
        ("gambler", "/Game/ToonStyle/TestAssets/Lineup", "/Game/ToonStyle/TestAssets/Lineup/Materials/MI_gambler"),
        ("slime", "/Game/ToonStyle/TestAssets/Lineup", "/Game/ToonStyle/TestAssets/Lineup/Materials/MI_slime"),
        ("tombspider", "/Game/ToonStyle/TestAssets/Lineup", "/Game/ToonStyle/TestAssets/Lineup/Materials/MI_tombspider"),
        ("cavebat", "/Game/ToonStyle/TestAssets/Lineup", "/Game/ToonStyle/TestAssets/Lineup/Materials/MI_cavebat"),
        ("idolaltar", "/Game/ToonStyle/TestAssets/Lineup", "/Game/ToonStyle/TestAssets/Lineup/Materials/MI_idolaltar"),
        ("arcademachine", "/Game/ToonStyle/TestAssets/Lineup", "/Game/ToonStyle/TestAssets/Lineup/Materials/MI_arcademachine"),
        ("lootchest", "/Game/ToonStyle/TestAssets/Lineup", "/Game/ToonStyle/TestAssets/Lineup/Materials/MI_lootchest"),
        ("lootcrate", "/Game/ToonStyle/TestAssets/Lineup", "/Game/ToonStyle/TestAssets/Lineup/Materials/MI_lootcrate"),
    ]

    created = []
    for asset_name, target_dir, material_path in asset_specs:
        texture = texture_for_asset(asset_name, target_dir)
        mi = create_material_instance(material_path, CHAR_MASTER, texture)
        outline_path = f"{material_path}_Outline"
        create_material_instance(outline_path, OUTLINE_MASTER)
        mesh_path = f"{target_dir}/SM_{asset_name}"
        mesh = unreal.EditorAssetLibrary.load_asset(mesh_path)
        if mesh and isinstance(mesh, unreal.StaticMesh):
            mesh.set_material(0, mi)
            unreal.EditorAssetLibrary.save_loaded_asset(mesh)
        created.append({"asset": asset_name, "material": material_path, "outline_material": outline_path, "texture": texture.get_path_name()})

    wall_texture = load_asset("/Game/ToonStyle/TestAssets/Environment/Textures/T_TestRoom_Wall")
    floor_texture = load_asset("/Game/ToonStyle/TestAssets/Environment/Textures/T_TestRoom_Floor")
    wall = create_material_instance("/Game/ToonStyle/TestAssets/Environment/Materials/MI_TestRoom_Wall", ENV_MASTER, wall_texture, 10.0)
    ceiling = create_material_instance("/Game/ToonStyle/TestAssets/Environment/Materials/MI_TestRoom_Ceiling", ENV_MASTER, wall_texture, 10.0)
    floor = create_material_instance("/Game/ToonStyle/TestAssets/Environment/Materials/MI_TestRoom_Floor", ENV_MASTER, floor_texture, 10.0)

    result = {
        "character_instances": created,
        "environment_instances": [
            wall.get_path_name(),
            ceiling.get_path_name(),
            floor.get_path_name(),
        ],
    }
    (OUT_DIR / "toon_material_instances_verify.json").write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    log(json.dumps(result, sort_keys=True))
    unreal.EditorAssetLibrary.save_directory("/Game/ToonStyle", only_if_is_dirty=False, recursive=True)
    return result


def main() -> int:
    mode = os.environ.get("T66_PHASE1B_TOON_MODE", "all").lower()
    result = {}
    if mode in {"masters", "all"}:
        result["masters"] = create_masters()
    if mode in {"instances", "all"}:
        result["instances"] = create_instances()
    (OUT_DIR / "toon_material_setup_verify.json").write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    if os.environ.get("T66_PHASE1B_QUIT_EDITOR", "1") == "1":
        unreal.SystemLibrary.quit_editor()
    return 0


main()
