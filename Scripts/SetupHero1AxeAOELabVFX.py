"""
Create or refresh the isolated Niagara assets for the Hero 1 axe AOE VFX lab.

Run with:
  "C:/Program Files/Epic Games/UE_5.7/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" ^
    "C:/UE/T66/T66.uproject" -run=pythonscript -script="C:/UE/T66/Scripts/SetupHero1AxeAOELabVFX.py" -unattended -nop4 -nosplash

Then generate the slash arc mesh and bind it to the Niagara mesh renderer with:
  "C:/Program Files/Epic Games/UE_5.7/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" ^
    "C:/UE/T66/T66.uproject" -run=T66Hero1AxeAOEVFX -unattended -nop4 -nosplash
"""

import os
import re

import unreal


COMMAND_LINE_TEXT = ""
try:
    COMMAND_LINE_TEXT = unreal.SystemLibrary.get_command_line()
except Exception:
    COMMAND_LINE_TEXT = ""

IS_PRODUCTION_TARGET = (
    "-T66Hero1AxeAOEProduction" in COMMAND_LINE_TEXT
    or os.environ.get("T66_HERO1_AXE_AOE_TARGET", "").lower() == "production"
)
LOG = "[Hero1AxeAOEProductionVFX]" if IS_PRODUCTION_TARGET else "[Hero1AxeAOELabVFX]"
AOE_DIR = "/Game/VFX/Hero1/Axe/AOE" if IS_PRODUCTION_TARGET else "/Game/VFXLab/Hero1Axe/AOE"
SHARED_DIR = "/Game/VFX/Hero1/Axe/Shared" if IS_PRODUCTION_TARGET else "/Game/VFXLab/Hero1Axe/Shared"
LAB_NIAGARA = f"{AOE_DIR}/NS_Hero1AxeAOE_MeshSlash"
SLASH_MESH = f"{SHARED_DIR}/SM_Hero1AxeAOE_SlashArc"
SOURCE_TEXTURE_DIR = r"C:/UE/T66/SourceAssets/VFX/Hero1Axe/AOE"
TEXTURE_SPECS = {
    "T_Hero1AxeAOE_StreakMask": os.path.join(SOURCE_TEXTURE_DIR, "T_Hero1AxeAOE_StreakMask.png"),
    "T_Hero1AxeAOE_DissolveNoise": os.path.join(SOURCE_TEXTURE_DIR, "T_Hero1AxeAOE_DissolveNoise.png"),
    "T_Hero1AxeAOE_ImpactMask": os.path.join(SOURCE_TEXTURE_DIR, "T_Hero1AxeAOE_ImpactMask.png"),
}
SLASH_LAYER_CONFIGS = [
    {
        "name": "Bright",
        "asset_path": f"{SHARED_DIR}/M_Hero1AxeAOE_Slash_Bright",
        "blend": unreal.BlendMode.BLEND_MASKED,
        "base_color": unreal.LinearColor(0.0, 0.0, 0.0, 1.0),
        "core_color": unreal.LinearColor(0.0, 0.0, 0.0, 1.0),
        "tip_color": unreal.LinearColor(0.0, 0.0, 0.0, 1.0),
        "impact_color": unreal.LinearColor(0.0, 0.0, 0.0, 1.0),
        "dissolve_pan": 0.92,
        "streak_strength": 0.0,
        "dissolve_strength": 0.0,
        "detail_floor": 1.0,
        "reveal_lead": 2.80,
        "reveal_sharpness": 24.0,
        "fade_start": 0.78,
        "fade_sharpness": 14.0,
        "width_softness": 6.2,
        "width_power": 0.42,
        "core_band_sharpness": 4.2,
        "tip_warmth_sharpness": 4.4,
        "impact_strength": 0.0,
        "impact_alpha": 0.0,
        "glow_strength": 0.0,
        "opacity_boost": 0.96,
        "radial_bias_mode": "neutral",
    },
    {
        "name": "Body",
        "asset_path": f"{SHARED_DIR}/M_Hero1AxeAOE_Slash_Body",
        "blend": unreal.BlendMode.BLEND_MASKED,
        "base_color": unreal.LinearColor(0.0, 0.0, 0.0, 1.0),
        "core_color": unreal.LinearColor(0.0, 0.0, 0.0, 1.0),
        "tip_color": unreal.LinearColor(0.0, 0.0, 0.0, 1.0),
        "impact_color": unreal.LinearColor(0.0, 0.0, 0.0, 1.0),
        "dissolve_pan": 0.66,
        "streak_strength": 0.0,
        "dissolve_strength": 0.0,
        "detail_floor": 1.0,
        "reveal_lead": 2.60,
        "reveal_sharpness": 16.0,
        "fade_start": 0.86,
        "fade_sharpness": 9.0,
        "width_softness": 4.8,
        "width_power": 0.70,
        "core_band_sharpness": 2.8,
        "tip_warmth_sharpness": 3.6,
        "impact_strength": 0.0,
        "impact_alpha": 0.0,
        "glow_strength": 0.0,
        "opacity_boost": 0.98,
        "radial_bias_mode": "neutral",
    },
    {
        "name": "Dark",
        "asset_path": f"{SHARED_DIR}/M_Hero1AxeAOE_Slash_Dark",
        "blend": unreal.BlendMode.BLEND_MASKED,
        "base_color": unreal.LinearColor(0.0, 0.0, 0.0, 1.0),
        "core_color": unreal.LinearColor(0.0, 0.0, 0.0, 1.0),
        "tip_color": unreal.LinearColor(0.0, 0.0, 0.0, 1.0),
        "impact_color": unreal.LinearColor(0.0, 0.0, 0.0, 1.0),
        "dissolve_pan": 0.33,
        "streak_strength": 0.0,
        "dissolve_strength": 0.0,
        "detail_floor": 1.0,
        "reveal_lead": 2.35,
        "reveal_sharpness": 11.0,
        "fade_start": 0.94,
        "fade_sharpness": 6.0,
        "width_softness": 4.0,
        "width_power": 0.85,
        "core_band_sharpness": 2.2,
        "tip_warmth_sharpness": 2.8,
        "impact_strength": 0.0,
        "impact_alpha": 0.0,
        "glow_strength": 0.0,
        "opacity_boost": 0.96,
        "radial_bias_mode": "neutral",
    },
]
SUPPORT_MATERIAL_CONFIGS = [
    {
        "name": "ImpactFlare",
        "asset_path": f"{SHARED_DIR}/M_Hero1AxeAOE_ImpactFlare",
        "blend": unreal.BlendMode.BLEND_ADDITIVE,
        "texture": "T_Hero1AxeAOE_ImpactMask",
        "color": unreal.LinearColor(0.88, 0.94, 1.0, 1.0),
        "core_color": unreal.LinearColor(1.0, 1.0, 1.0, 1.0),
        "glow_strength": 13.0,
        "opacity": 0.86,
        "fade_power": 1.45,
    },
    {
        "name": "DirectionalSpark",
        "asset_path": f"{SHARED_DIR}/M_Hero1AxeAOE_DirectionalSpark",
        "blend": unreal.BlendMode.BLEND_ADDITIVE,
        "texture": "T_Hero1AxeAOE_StreakMask",
        "color": unreal.LinearColor(1.0, 0.025, 0.045, 1.0),
        "core_color": unreal.LinearColor(1.0, 0.92, 0.92, 1.0),
        "glow_strength": 3.4,
        "opacity": 0.14,
        "fade_power": 1.2,
    },
    {
        "name": "Mote",
        "asset_path": f"{SHARED_DIR}/M_Hero1AxeAOE_Mote",
        "blend": unreal.BlendMode.BLEND_ADDITIVE,
        "texture": "T_Hero1AxeAOE_DissolveNoise",
        "color": unreal.LinearColor(0.02, 0.26, 1.0, 1.0),
        "core_color": unreal.LinearColor(0.86, 0.95, 1.0, 1.0),
        "glow_strength": 2.0,
        "opacity": 0.12,
        "fade_power": 1.9,
    },
    {
        "name": "GroundTrace",
        "asset_path": f"{SHARED_DIR}/M_Hero1AxeAOE_GroundTrace",
        "blend": unreal.BlendMode.BLEND_TRANSLUCENT,
        "texture": "T_Hero1AxeAOE_StreakMask",
        "color": unreal.LinearColor(0.02, 0.10, 0.40, 1.0),
        "core_color": unreal.LinearColor(0.75, 0.05, 0.08, 1.0),
        "glow_strength": 0.8,
        "opacity": 0.08,
        "fade_power": 2.3,
    },
]

DEPRECATED_ASSETS = [
    f"{AOE_DIR}/NS_Hero1AxeAOE_CrescentSlash",
    f"{AOE_DIR}/P_Hero1AxeAOE_WeaponSlashSeed",
    f"{AOE_DIR}/P_Hero1AxeAOE_ShockwaveSeed",
    f"{SHARED_DIR}/M_Hero1AxeAOE_Slash_Reveal",
    f"{SHARED_DIR}/M_Hero1AxeAOE_CrescentSlash",
    f"{SHARED_DIR}/T_Hero1AxeAOE_CrescentSlash",
    f"{SHARED_DIR}/M_Hero1AxeAOE_VertexAdditive",
    f"{SHARED_DIR}/M_Hero1AxeAOE_VertexTranslucent",
    f"{SHARED_DIR}/M_Hero1AxeAOE_VertexOpaque",
    f"{SHARED_DIR}/M_Hero1AxeAOE_AxeConstant",
    f"{SHARED_DIR}/M_Hero1AxeAOE_WeaponArt",
    f"{SHARED_DIR}/M_Hero1AxeAOE_WeaponArtGlow",
]


mel = unreal.MaterialEditingLibrary
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()


COMMAND_LINE = COMMAND_LINE_TEXT
FORCE_TEXTURE_IMPORTS = "-T66Hero1AxeAOEForceTextureImport" in COMMAND_LINE
ONLY_TEXTURE_IMPORT = ""
texture_import_match = re.search(r"-T66Hero1AxeAOEImportTextureName=([A-Za-z0-9_]+)", COMMAND_LINE)
if texture_import_match:
    ONLY_TEXTURE_IMPORT = texture_import_match.group(1)


def log(message):
    unreal.log(f"{LOG} {message}")


def warn(message):
    unreal.log_warning(f"{LOG} {message}")


def safe_set(obj, prop, value):
    try:
        obj.set_editor_property(prop, value)
    except Exception as exc:
        warn(f"failed to set {prop}: {exc}")


def connect(src, src_output, dst, dst_input):
    if not mel.connect_material_expressions(src, src_output, dst, dst_input):
        raise RuntimeError(
            f"Failed to connect {src.get_class().get_name()}:{src_output or '<default>'} "
            f"to {dst.get_class().get_name()}:{dst_input or '<default>'}"
        )


def ensure_directory(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def delete_asset(asset_path):
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        if not unreal.EditorAssetLibrary.delete_asset(asset_path):
            raise RuntimeError(f"Failed to delete asset {asset_path}")
        log(f"Deleted {asset_path}")


def delete_deprecated_assets():
    for asset_path in DEPRECATED_ASSETS:
        delete_asset(asset_path)


def recreate_asset(asset_path, asset_class, factory):
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        existing_asset = unreal.EditorAssetLibrary.load_asset(asset_path)
        if existing_asset and asset_class == unreal.Material:
            log(f"Reusing existing material shell {asset_path}")
            mel.delete_all_material_expressions(existing_asset)
            existing_asset.modify()
            return existing_asset

    delete_asset(asset_path)
    asset_name = asset_path.rsplit("/", 1)[-1]
    package_path = asset_path.rsplit("/", 1)[0]
    asset = asset_tools.create_asset(asset_name, package_path, asset_class, factory)
    if not asset:
        raise RuntimeError(f"Failed to create asset {asset_path}")
    return asset


def create_expression(material, expr_class, x, y):
    expr = mel.create_material_expression(material, expr_class, x, y)
    if not expr:
        raise RuntimeError(f"Failed to create expression {expr_class}")
    return expr


def import_texture(asset_name, source_path):
    asset_path = f"{SHARED_DIR}/{asset_name}.{asset_name}"
    package_asset_path = f"{SHARED_DIR}/{asset_name}"
    existing_texture = None
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        existing_texture = unreal.EditorAssetLibrary.load_asset(asset_path)
        if ONLY_TEXTURE_IMPORT and ONLY_TEXTURE_IMPORT != asset_name:
            log(f"Using existing {asset_path}; targeted import requested for {ONLY_TEXTURE_IMPORT}")
            return existing_texture
        if not FORCE_TEXTURE_IMPORTS and ONLY_TEXTURE_IMPORT != asset_name:
            log(f"Using existing {asset_path}")
            return existing_texture

    if not os.path.exists(source_path):
        raise RuntimeError(f"Missing source texture {source_path}")

    if IS_PRODUCTION_TARGET:
        lab_package_asset_path = f"/Game/VFXLab/Hero1Axe/Shared/{asset_name}"
        lab_asset_path = f"{lab_package_asset_path}.{asset_name}"
        if unreal.EditorAssetLibrary.does_asset_exist(lab_asset_path):
            if existing_texture:
                log(f"Deleting stale production texture asset before duplicate: {asset_path}")
                if not unreal.EditorAssetLibrary.delete_asset(package_asset_path):
                    raise RuntimeError(f"Failed to delete stale texture asset {asset_path}")
            texture = unreal.EditorAssetLibrary.duplicate_asset(lab_package_asset_path, package_asset_path)
            if not texture:
                raise RuntimeError(f"Failed to duplicate {lab_asset_path} to {asset_path}")
            safe_set(texture, "srgb", False)
            safe_set(texture, "never_stream", True)
            safe_set(texture, "mip_gen_settings", unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
            safe_set(texture, "filter", unreal.TextureFilter.TF_BILINEAR)
            try:
                texture.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_MASKS)
            except Exception:
                pass
            unreal.EditorAssetLibrary.save_loaded_asset(texture)
            log(f"Duplicated {lab_asset_path} to {asset_path}")
            return texture

    if existing_texture:
        log(f"Deleting stale texture asset before fresh import: {asset_path}")
        if not unreal.EditorAssetLibrary.delete_asset(asset_path):
            raise RuntimeError(f"Failed to delete stale texture asset {asset_path}")

    task = unreal.AssetImportTask()
    task.set_editor_property("filename", source_path)
    task.set_editor_property("destination_path", SHARED_DIR)
    task.set_editor_property("destination_name", asset_name)
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("replace_existing_settings", True)
    task.set_editor_property("save", True)
    asset_tools.import_asset_tasks([task])

    texture = unreal.EditorAssetLibrary.load_asset(asset_path)
    if not texture:
        raise RuntimeError(f"Failed to import {source_path} to {asset_path}")

    safe_set(texture, "srgb", False)
    safe_set(texture, "never_stream", True)
    safe_set(texture, "mip_gen_settings", unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
    safe_set(texture, "filter", unreal.TextureFilter.TF_BILINEAR)
    try:
        texture.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_MASKS)
    except Exception:
        pass
    unreal.EditorAssetLibrary.save_loaded_asset(texture)
    log(f"Imported {asset_path}")
    return texture


def import_textures():
    return {asset_name: import_texture(asset_name, source_path) for asset_name, source_path in TEXTURE_SPECS.items()}


def component_mask(material, source, channel, x, y):
    mask = create_expression(material, unreal.MaterialExpressionComponentMask, x, y)
    safe_set(mask, "r", channel == "r")
    safe_set(mask, "g", channel == "g")
    safe_set(mask, "b", channel == "b")
    safe_set(mask, "a", channel == "a")
    connect(source, "", mask, "")
    return mask


def scalar(material, value, x, y):
    expr = create_expression(material, unreal.MaterialExpressionConstant, x, y)
    safe_set(expr, "r", value)
    return expr


def scalar_param(material, name, value, x, y):
    expr = create_expression(material, unreal.MaterialExpressionScalarParameter, x, y)
    safe_set(expr, "parameter_name", name)
    safe_set(expr, "default_value", value)
    return expr


def texture_param(material, name, texture, uv_source, x, y):
    expr = create_expression(material, unreal.MaterialExpressionTextureSampleParameter2D, x, y)
    safe_set(expr, "parameter_name", name)
    safe_set(expr, "texture", texture)
    connect(uv_source, "", expr, "UVs")
    return expr


def append_vector(material, a, b, x, y):
    expr = create_expression(material, unreal.MaterialExpressionAppendVector, x, y)
    connect(a, "", expr, "A")
    connect(b, "", expr, "B")
    return expr


def add(material, a, b, x, y):
    expr = create_expression(material, unreal.MaterialExpressionAdd, x, y)
    connect(a, "", expr, "A")
    connect(b, "", expr, "B")
    return expr


def multiply(material, a, b, x, y):
    expr = create_expression(material, unreal.MaterialExpressionMultiply, x, y)
    connect(a, "", expr, "A")
    connect(b, "", expr, "B")
    return expr


def subtract(material, a, b, x, y):
    expr = create_expression(material, unreal.MaterialExpressionSubtract, x, y)
    connect(a, "", expr, "A")
    connect(b, "", expr, "B")
    return expr


def saturate(material, src, x, y):
    expr = create_expression(material, unreal.MaterialExpressionSaturate, x, y)
    connect(src, "", expr, "")
    return expr


def one_minus(material, src, x, y):
    expr = create_expression(material, unreal.MaterialExpressionOneMinus, x, y)
    connect(src, "", expr, "")
    return expr


def build_slash_material(textures, config):
    required = [
        "MaterialExpressionDynamicParameter",
        "MaterialExpressionTextureCoordinate",
        "MaterialExpressionTextureSampleParameter2D",
        "MaterialExpressionComponentMask",
        "MaterialExpressionScalarParameter",
        "MaterialExpressionVectorParameter",
        "MaterialExpressionAppendVector",
        "MaterialExpressionAdd",
        "MaterialExpressionLinearInterpolate",
        "MaterialExpressionOneMinus",
        "MaterialExpressionSaturate",
        "MaterialExpressionPower",
    ]
    missing = [name for name in required if not hasattr(unreal, name)]
    if missing:
        raise RuntimeError(f"Missing required material expression classes: {missing}")

    material = recreate_asset(config["asset_path"], unreal.Material, unreal.MaterialFactoryNew())
    safe_set(material, "shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    safe_set(material, "blend_mode", config["blend"])
    safe_set(material, "two_sided", True)
    safe_set(material, "used_with_niagara_mesh_particles", True)
    safe_set(material, "opacity_mask_clip_value", 0.08)

    texcoord = create_expression(material, unreal.MaterialExpressionTextureCoordinate, -2500, -160)
    u_coord = component_mask(material, texcoord, "r", -2260, -260)
    v_coord = component_mask(material, texcoord, "g", -2260, -40)
    age = create_expression(material, unreal.MaterialExpressionDynamicParameter, -2260, -560)
    safe_set(age, "param_names", ["SlashAge", "HoldOne", "HoldOne", "HoldOne"])
    safe_set(age, "default_value", unreal.LinearColor(0.0, 1.0, 1.0, 1.0))
    safe_set(age, "parameter_index", 0)
    age_scalar = component_mask(material, age, "r", -2100, -560)

    dissolve_pan = multiply(
        material,
        age_scalar,
        scalar_param(material, "DissolvePan", config["dissolve_pan"], -2500, 520),
        -2260,
        520,
    )
    zero_offset = scalar(material, 0.0, -2260, 690)
    dissolve_uv_offset = append_vector(material, dissolve_pan, zero_offset, -1960, 590)
    dissolve_uv = add(material, texcoord, dissolve_uv_offset, -1700, 520)
    streak_tex = texture_param(
        material,
        "StreakMask",
        textures["T_Hero1AxeAOE_StreakMask"],
        texcoord,
        -1420,
        520,
    )
    dissolve_tex = texture_param(
        material,
        "DissolveNoise",
        textures["T_Hero1AxeAOE_DissolveNoise"],
        dissolve_uv,
        -1420,
        760,
    )
    impact_tex = texture_param(
        material,
        "ImpactMask",
        textures["T_Hero1AxeAOE_ImpactMask"],
        texcoord,
        -1420,
        1000,
    )
    streak_mask = component_mask(material, streak_tex, "r", -1180, 520)
    dissolve_mask = component_mask(material, dissolve_tex, "r", -1180, 760)
    impact_mask = component_mask(material, impact_tex, "r", -1180, 1000)
    authored_detail = saturate(
        material,
        add(
            material,
            multiply(material, streak_mask, scalar_param(material, "StreakStrength", config["streak_strength"], -940, 520), -940, 620),
            multiply(material, dissolve_mask, scalar_param(material, "DissolveStrength", config["dissolve_strength"], -940, 760), -940, 860),
            -680,
            640,
        ),
        -430,
        640,
    )
    detail_floor = scalar_param(material, "DetailFloor", config["detail_floor"], -430, 850)
    detail_one = scalar(material, 1.0, -430, 1020)
    detail_lerp = create_expression(material, unreal.MaterialExpressionLinearInterpolate, -190, 770)
    connect(detail_floor, "", detail_lerp, "A")
    connect(detail_one, "", detail_lerp, "B")
    connect(authored_detail, "", detail_lerp, "Alpha")

    reveal_lead = create_expression(material, unreal.MaterialExpressionMultiply, -1950, -640)
    connect(age_scalar, "", reveal_lead, "A")
    connect(scalar_param(material, "RevealLead", config["reveal_lead"], -2260, -760), "", reveal_lead, "B")
    reveal_delta = subtract(material, reveal_lead, u_coord, -1680, -470)
    reveal = saturate(
        material,
        multiply(
            material,
            reveal_delta,
                scalar_param(material, "RevealSharpness", config["reveal_sharpness"], -1680, -720),
            -1420,
            -470,
        ),
        -1180,
        -470,
    )

    fade_delta = create_expression(material, unreal.MaterialExpressionSubtract, -1680, -980)
    connect(age_scalar, "", fade_delta, "A")
    connect(scalar(material, config["fade_start"], -1960, -1010), "", fade_delta, "B")
    fade_out = one_minus(
        material,
        saturate(
            material,
            multiply(
                material,
                fade_delta,
                scalar_param(material, "FadeSharpness", config["fade_sharpness"], -1680, -1240),
                -1420,
                -980,
            ),
            -1180,
            -980,
        ),
        -940,
        -980,
    )

    width_softness = saturate(
        material,
        multiply(
            material,
            multiply(material, v_coord, one_minus(material, v_coord, -1960, 180), -1680, 90),
            scalar_param(material, "WidthSoftness", config["width_softness"], -1680, 330),
            -1420,
            90,
        ),
        -1180,
        90,
    )
    width_power = create_expression(material, unreal.MaterialExpressionPower, -940, 90)
    connect(width_softness, "", width_power, "Base")
    safe_set(width_power, "const_exponent", config["width_power"])

    radial_bias_weight = scalar(material, 1.0, -940, -80)
    radial_bias_mode = config.get("radial_bias_mode", "neutral")
    if radial_bias_mode in ("inner", "outer"):
        radial_source = one_minus(material, v_coord, -1420, -110) if radial_bias_mode == "inner" else v_coord
        radial_power = create_expression(material, unreal.MaterialExpressionPower, -1180, -80)
        connect(radial_source, "", radial_power, "Base")
        safe_set(radial_power, "const_exponent", config.get("radial_bias_power", 1.0))
        radial_bias_lerp = create_expression(material, unreal.MaterialExpressionLinearInterpolate, -940, -80)
        connect(scalar_param(material, "RadialBiasMin", config.get("radial_bias_min", 0.25), -1180, -260), "", radial_bias_lerp, "A")
        connect(scalar(material, 1.0, -1180, -420), "", radial_bias_lerp, "B")
        connect(radial_power, "", radial_bias_lerp, "Alpha")
        radial_bias_weight = radial_bias_lerp

    alpha = multiply(
        material,
        multiply(
            material,
            multiply(material, multiply(material, reveal, fade_out, -680, -470), width_power, -430, -260),
            radial_bias_weight,
            -300,
            -340,
        ),
        detail_lerp,
        -190,
        -260,
    )

    core_band = saturate(
        material,
        multiply(
            material,
            subtract(material, width_softness, scalar(material, 0.48, -940, 330), -680, 190),
            scalar_param(material, "CoreBandSharpness", config["core_band_sharpness"], -680, 430),
            -430,
            190,
        ),
        -190,
        190,
    )
    tip_band = saturate(
        material,
        multiply(
            material,
            subtract(material, u_coord, scalar(material, 0.72, -940, -150), -680, -110),
            scalar_param(material, "TipWarmthSharpness", config["tip_warmth_sharpness"], -680, 30),
            -430,
            -110,
        ),
        -190,
        -110,
    )

    base_color = create_expression(material, unreal.MaterialExpressionVectorParameter, -190, -720)
    safe_set(base_color, "parameter_name", "SlashBaseColor")
    safe_set(base_color, "default_value", config["base_color"])

    core_color = create_expression(material, unreal.MaterialExpressionVectorParameter, -190, -500)
    safe_set(core_color, "parameter_name", "SlashCoreColor")
    safe_set(core_color, "default_value", config["core_color"])

    tip_color = create_expression(material, unreal.MaterialExpressionVectorParameter, -190, -900)
    safe_set(tip_color, "parameter_name", "SlashTipColor")
    safe_set(tip_color, "default_value", config["tip_color"])

    impact_color = create_expression(material, unreal.MaterialExpressionVectorParameter, -190, -1120)
    safe_set(impact_color, "parameter_name", "ImpactColor")
    safe_set(impact_color, "default_value", config["impact_color"])

    core_lerp = create_expression(material, unreal.MaterialExpressionLinearInterpolate, 80, -610)
    connect(base_color, "", core_lerp, "A")
    connect(core_color, "", core_lerp, "B")
    connect(core_band, "", core_lerp, "Alpha")

    tip_lerp = create_expression(material, unreal.MaterialExpressionLinearInterpolate, 340, -610)
    connect(core_lerp, "", tip_lerp, "A")
    connect(tip_color, "", tip_lerp, "B")
    connect(tip_band, "", tip_lerp, "Alpha")

    impact_band = saturate(
        material,
        multiply(material, impact_mask, scalar_param(material, "ImpactStrength", config["impact_strength"], -940, 1020), -680, 1020),
        -430,
        1020,
    )
    impact_lerp = create_expression(material, unreal.MaterialExpressionLinearInterpolate, 520, -610)
    connect(tip_lerp, "", impact_lerp, "A")
    connect(impact_color, "", impact_lerp, "B")
    connect(impact_band, "", impact_lerp, "Alpha")

    alpha_with_impact = saturate(
        material,
        add(
            material,
            alpha,
            multiply(material, impact_band, scalar(material, config["impact_alpha"], 340, 1020), 520, 1020),
            760,
            1020,
        ),
        1010,
        1020,
    )

    emissive_tint = multiply(
        material,
        impact_lerp,
        scalar_param(material, "GlowStrength", config["glow_strength"], 620, -720),
        890,
        -430,
    )
    emissive = multiply(material, emissive_tint, alpha_with_impact, 1160, -430)
    opacity = multiply(material, alpha_with_impact, scalar_param(material, "OpacityBoost", config["opacity_boost"], 620, -170), 890, -170)

    mel.connect_material_property(impact_lerp, "", unreal.MaterialProperty.MP_BASE_COLOR)
    mel.connect_material_property(emissive, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    mel.connect_material_property(opacity, "", unreal.MaterialProperty.MP_OPACITY)
    mel.connect_material_property(opacity, "", unreal.MaterialProperty.MP_OPACITY_MASK)
    mel.layout_material_expressions(material)
    mel.recompile_material(material)
    unreal.EditorAssetLibrary.save_asset(config["asset_path"])
    log(f"Built {config['asset_path']} ({config['name']})")


def build_support_material(textures, config):
    material = recreate_asset(config["asset_path"], unreal.Material, unreal.MaterialFactoryNew())
    safe_set(material, "shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    safe_set(material, "blend_mode", config["blend"])
    safe_set(material, "two_sided", True)
    safe_set(material, "used_with_niagara_sprites", True)

    texcoord = create_expression(material, unreal.MaterialExpressionTextureCoordinate, -1600, -80)
    support_tex = texture_param(material, "SupportMask", textures[config["texture"]], texcoord, -1320, -80)
    support_mask = component_mask(material, support_tex, "r", -1080, -80)

    age = create_expression(material, unreal.MaterialExpressionDynamicParameter, -1320, -430)
    safe_set(age, "param_names", ["SlashAge", "HoldOne", "HoldOne", "HoldOne"])
    safe_set(age, "default_value", unreal.LinearColor(0.0, 1.0, 1.0, 1.0))
    safe_set(age, "parameter_index", 0)
    age_scalar = component_mask(material, age, "r", -1080, -430)

    alive = one_minus(material, saturate(material, age_scalar, -840, -430), -610, -430)
    fade_power = create_expression(material, unreal.MaterialExpressionPower, -360, -430)
    connect(alive, "", fade_power, "Base")
    safe_set(fade_power, "const_exponent", config["fade_power"])

    support_alpha = multiply(material, support_mask, fade_power, -120, -180)

    color = create_expression(material, unreal.MaterialExpressionVectorParameter, -360, -760)
    safe_set(color, "parameter_name", "SupportColor")
    safe_set(color, "default_value", config["color"])

    core_color = create_expression(material, unreal.MaterialExpressionVectorParameter, -360, -980)
    safe_set(core_color, "parameter_name", "SupportCoreColor")
    safe_set(core_color, "default_value", config["core_color"])

    core_lerp = create_expression(material, unreal.MaterialExpressionLinearInterpolate, -100, -860)
    connect(color, "", core_lerp, "A")
    connect(core_color, "", core_lerp, "B")
    connect(support_mask, "", core_lerp, "Alpha")

    emissive_tint = multiply(
        material,
        core_lerp,
        scalar_param(material, "GlowStrength", config["glow_strength"], 160, -860),
        390,
        -650,
    )
    emissive = multiply(material, emissive_tint, support_alpha, 650, -650)
    opacity = multiply(material, support_alpha, scalar_param(material, "OpacityBoost", config["opacity"], 160, -180), 390, -180)

    mel.connect_material_property(core_lerp, "", unreal.MaterialProperty.MP_BASE_COLOR)
    mel.connect_material_property(emissive, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    mel.connect_material_property(opacity, "", unreal.MaterialProperty.MP_OPACITY)
    mel.layout_material_expressions(material)
    mel.recompile_material(material)
    unreal.EditorAssetLibrary.save_asset(config["asset_path"])
    log(f"Built {config['asset_path']} ({config['name']})")


def prepare_niagara_package():
    delete_asset(LAB_NIAGARA)
    log(f"Prepared empty Niagara package slot for commandlet-owned {LAB_NIAGARA}")


def main():
    ensure_directory(AOE_DIR)
    ensure_directory(SHARED_DIR)
    delete_deprecated_assets()
    delete_asset(SLASH_MESH)
    textures = import_textures()
    for config in SLASH_LAYER_CONFIGS:
        build_slash_material(textures, config)
    for config in SUPPORT_MATERIAL_CONFIGS:
        build_support_material(textures, config)
    prepare_niagara_package()
    log("DONE")


if __name__ == "__main__":
    main()
