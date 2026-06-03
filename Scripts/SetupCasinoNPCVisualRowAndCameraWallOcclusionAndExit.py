"""
Reload the CasinoNPC visual data row and create the camera wall fade material.

Run with:
  UnrealEditor.exe T66.uproject -ExecutePythonScript="C:/UE/T66/Scripts/SetupCasinoNPCVisualRowAndCameraWallOcclusionAndExit.py"
"""

import os
import sys

import unreal

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
if SCRIPT_DIR not in sys.path:
    sys.path.insert(0, SCRIPT_DIR)

import SetupCharacterVisualsDataTable


MATERIAL_PATH = "/Game/Materials/M_CameraWallOccluderFade"
LOG = "[SetupCasinoNPCVisualRowAndCameraWallOcclusion]"
MEL = unreal.MaterialEditingLibrary


def get_or_create_material(asset_path):
    material = unreal.EditorAssetLibrary.load_asset(asset_path)
    if material:
        return material

    package_path, asset_name = asset_path.rsplit("/", 1)
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    material = asset_tools.create_asset(
        asset_name,
        package_path,
        unreal.Material,
        unreal.MaterialFactoryNew(),
    )
    if not material:
        raise RuntimeError(f"Could not create material: {asset_path}")
    return material


def create_vector_parameter(material, name, value, x, y):
    expr = MEL.create_material_expression(material, unreal.MaterialExpressionVectorParameter, x, y)
    expr.set_editor_property("parameter_name", name)
    expr.set_editor_property("default_value", unreal.LinearColor(*value))
    return expr


def create_scalar_parameter(material, name, value, x, y):
    expr = MEL.create_material_expression(material, unreal.MaterialExpressionScalarParameter, x, y)
    expr.set_editor_property("parameter_name", name)
    expr.set_editor_property("default_value", float(value))
    return expr


def set_optional_property(asset, name, value):
    try:
        asset.set_editor_property(name, value)
    except Exception as exc:
        unreal.log_warning(f"{LOG} Could not set {name}: {exc}")


def rebuild_camera_wall_fade_material():
    material = get_or_create_material(MATERIAL_PATH)
    unreal.log(f"{LOG} Rebuilding {MATERIAL_PATH}")

    MEL.delete_all_material_expressions(material)
    material.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    material.set_editor_property("blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)
    material.set_editor_property("two_sided", True)
    material.set_editor_property("use_material_attributes", False)
    set_optional_property(material, "used_with_instanced_static_meshes", True)
    set_optional_property(material, "used_with_nanite", True)

    fade_color = create_vector_parameter(material, "FadeColor", (0.42, 0.46, 0.48, 1.0), -420, -80)
    opacity = create_scalar_parameter(material, "Opacity", 0.12, -420, 120)

    MEL.connect_material_property(fade_color, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    MEL.connect_material_property(opacity, "", unreal.MaterialProperty.MP_OPACITY)
    MEL.layout_material_expressions(material)
    MEL.recompile_material(material)
    if not unreal.EditorAssetLibrary.save_asset(MATERIAL_PATH):
        unreal.log_warning(f"{LOG} Save returned false for {MATERIAL_PATH}")


def main():
    unreal.log(f"{LOG} START")
    SetupCharacterVisualsDataTable.main()
    rebuild_camera_wall_fade_material()
    unreal.log(f"{LOG} DONE")

    try:
        unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")
    except Exception as exc:
        unreal.log_warning(f"{LOG} Failed to request QUIT_EDITOR: {exc}")


main()
