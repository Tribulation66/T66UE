"""
Build the dungeon-vision post-process material used by the dungeon lighting preset.

Run in Unreal commandlet mode:
  UnrealEditor-Cmd.exe T66.uproject -run=pythonscript -script="C:/UE/T66/Scripts/BuildDungeonVisionPostProcess.py"
"""

import unreal

LOG = "[DungeonVision]"
ASSET_DIR = "/Game/UI"
ASSET_NAME = "M_DungeonVisionPostProcess"
ASSET_PATH = f"{ASSET_DIR}/{ASSET_NAME}"

mel = unreal.MaterialEditingLibrary
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()


def log(message):
    unreal.log(f"{LOG} {message}")


def safe_set(obj, prop, value):
    try:
        obj.set_editor_property(prop, value)
    except Exception as exc:
        unreal.log_warning(f"{LOG} failed to set {prop}: {exc}")


def connect(src, src_output, dst, dst_input):
    mel.connect_material_expressions(src, src_output, dst, dst_input)


def create_expression(material, expr_class, x, y):
    expr = mel.create_material_expression(material, expr_class, x, y)
    if not expr:
        raise RuntimeError(f"Failed to create expression {expr_class}")
    return expr


def recreate_asset(asset_path, asset_class, factory):
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        if not unreal.EditorAssetLibrary.delete_asset(asset_path):
            raise RuntimeError(f"Failed to delete existing asset {asset_path}")

    asset_name = asset_path.rsplit("/", 1)[-1]
    package_path = asset_path.rsplit("/", 1)[0]
    asset = asset_tools.create_asset(asset_name, package_path, asset_class, factory)
    if not asset:
        raise RuntimeError(f"Failed to create asset {asset_path}")
    return asset


def ensure_directory(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def build_material():
    material = recreate_asset(ASSET_PATH, unreal.Material, unreal.MaterialFactoryNew())
    safe_set(material, "material_domain", unreal.MaterialDomain.MD_POST_PROCESS)
    safe_set(material, "blendable_location", unreal.BlendableLocation.BL_SCENE_COLOR_AFTER_TONEMAPPING)

    scene_tex = create_expression(material, unreal.MaterialExpressionSceneTexture, -1800, 0)
    safe_set(scene_tex, "scene_texture_id", unreal.SceneTextureId.PPI_POST_PROCESS_INPUT0)

    world_position = create_expression(material, unreal.MaterialExpressionWorldPosition, -1800, -360)

    player_position = create_expression(material, unreal.MaterialExpressionVectorParameter, -1800, -760)
    safe_set(player_position, "parameter_name", "PlayerWorldPosition")
    safe_set(player_position, "default_value", unreal.LinearColor(0.0, 0.0, 0.0, 0.0))

    vision_radius = create_expression(material, unreal.MaterialExpressionScalarParameter, -1800, 240)
    safe_set(vision_radius, "parameter_name", "VisionRadius")
    safe_set(vision_radius, "default_value", 2200.0)

    falloff_distance = create_expression(material, unreal.MaterialExpressionScalarParameter, -1800, 460)
    safe_set(falloff_distance, "parameter_name", "FalloffDistance")
    safe_set(falloff_distance, "default_value", 650.0)

    ambient_visibility = create_expression(material, unreal.MaterialExpressionScalarParameter, -1800, 680)
    safe_set(ambient_visibility, "parameter_name", "AmbientVisibility")
    safe_set(ambient_visibility, "default_value", 0.0)

    world_xy = create_expression(material, unreal.MaterialExpressionComponentMask, -1480, -420)
    safe_set(world_xy, "r", True)
    safe_set(world_xy, "g", True)
    connect(world_position, "", world_xy, "")

    player_xy = create_expression(material, unreal.MaterialExpressionComponentMask, -1480, -760)
    safe_set(player_xy, "r", True)
    safe_set(player_xy, "g", True)
    connect(player_position, "", player_xy, "")

    delta_xy = create_expression(material, unreal.MaterialExpressionSubtract, -1180, -540)
    connect(world_xy, "", delta_xy, "A")
    connect(player_xy, "", delta_xy, "B")

    distance_xy = create_expression(material, unreal.MaterialExpressionLength, -900, -540)
    connect(delta_xy, "", distance_xy, "")

    distance_minus_radius = create_expression(material, unreal.MaterialExpressionSubtract, -620, -420)
    connect(distance_xy, "", distance_minus_radius, "A")
    connect(vision_radius, "", distance_minus_radius, "B")

    min_falloff = create_expression(material, unreal.MaterialExpressionConstant, -900, 120)
    safe_set(min_falloff, "r", 1.0)

    safe_falloff = create_expression(material, unreal.MaterialExpressionMax, -620, 120)
    connect(falloff_distance, "", safe_falloff, "A")
    connect(min_falloff, "", safe_falloff, "B")

    darkness_alpha = create_expression(material, unreal.MaterialExpressionDivide, -340, -320)
    connect(distance_minus_radius, "", darkness_alpha, "A")
    connect(safe_falloff, "", darkness_alpha, "B")

    darkness_alpha_saturated = create_expression(material, unreal.MaterialExpressionSaturate, -80, -320)
    connect(darkness_alpha, "", darkness_alpha_saturated, "")

    reveal_alpha = create_expression(material, unreal.MaterialExpressionOneMinus, 180, -320)
    connect(darkness_alpha_saturated, "", reveal_alpha, "")

    visibility = create_expression(material, unreal.MaterialExpressionLinearInterpolate, 480, -200)
    connect(ambient_visibility, "", visibility, "A")
    connect(reveal_alpha, "", visibility, "Alpha")
    safe_set(visibility, "const_b", 1.0)

    black_color = create_expression(material, unreal.MaterialExpressionConstant3Vector, 180, 80)
    safe_set(black_color, "constant", unreal.LinearColor(0.0, 0.0, 0.0, 0.0))

    final_color = create_expression(material, unreal.MaterialExpressionLinearInterpolate, 780, -20)
    connect(black_color, "", final_color, "A")
    connect(scene_tex, "Color", final_color, "B")
    connect(visibility, "", final_color, "Alpha")

    mel.connect_material_property(final_color, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    mel.layout_material_expressions(material)
    mel.recompile_material(material)
    unreal.EditorAssetLibrary.save_asset(ASSET_PATH)
    log(f"Built {ASSET_PATH}")


def main():
    ensure_directory(ASSET_DIR)
    build_material()
    log("DONE")


if __name__ == "__main__":
    main()
