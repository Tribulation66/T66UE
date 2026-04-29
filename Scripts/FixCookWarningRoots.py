"""
Repair root causes for standalone cook warnings.

Run with:
  UnrealEditor-Cmd.exe T66.uproject -run=pythonscript -script="C:/UE/T66/Scripts/FixCookWarningRoots.py"
"""

import os

import unreal


MEL = unreal.MaterialEditingLibrary


def log(message):
    unreal.log(f"[FixCookWarningRoots] {message}")


def warn(message):
    unreal.log_warning(f"[FixCookWarningRoots] {message}")


def load_asset(asset_path):
    asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    if not asset:
        raise RuntimeError(f"Could not load asset: {asset_path}")
    return asset


def save_asset(asset_path):
    try:
        saved = unreal.EditorAssetLibrary.save_asset(asset_path, False)
    except TypeError:
        saved = unreal.EditorAssetLibrary.save_asset(asset_path)

    if not saved:
        warn(f"Save failed for {asset_path}")


def save_loaded_asset(asset, asset_path):
    try:
        saved = unreal.EditorAssetLibrary.save_loaded_asset(asset, False)
    except TypeError:
        saved = unreal.EditorAssetLibrary.save_loaded_asset(asset)

    if not saved:
        warn(f"Save failed for {asset_path}")


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


def make_vector_param(material, name, value, x, y):
    expr = MEL.create_material_expression(material, unreal.MaterialExpressionVectorParameter, x, y)
    expr.set_editor_property("parameter_name", name)
    expr.set_editor_property("default_value", unreal.LinearColor(*value))
    return expr


def make_scalar_param(material, name, value, x, y):
    expr = MEL.create_material_expression(material, unreal.MaterialExpressionScalarParameter, x, y)
    expr.set_editor_property("parameter_name", name)
    expr.set_editor_property("default_value", float(value))
    return expr


def rebuild_simple_unlit_material(asset_path, color_param, color, extra_scalar_params=None):
    material = get_or_create_material(asset_path)
    log(f"Rebuilding valid unlit material {asset_path}")

    MEL.delete_all_material_expressions(material)
    material.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    material.set_editor_property("blend_mode", unreal.BlendMode.BLEND_OPAQUE)
    material.set_editor_property("two_sided", True)
    material.set_editor_property("use_material_attributes", False)

    color_expr = make_vector_param(material, color_param, color, -400, 0)
    MEL.connect_material_property(color_expr, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)

    for index, (param_name, param_value) in enumerate(extra_scalar_params or []):
        make_scalar_param(material, param_name, param_value, -400, 220 + index * 120)

    MEL.layout_material_expressions(material)
    MEL.recompile_material(material)
    save_asset(asset_path)


def update_material_instances(asset_paths):
    for asset_path in asset_paths:
        instance = unreal.EditorAssetLibrary.load_asset(asset_path)
        if not instance:
            warn(f"Material instance missing: {asset_path}")
            continue
        update_fn = getattr(MEL, "update_material_instance", None)
        if update_fn:
            update_fn(instance)
        save_asset(asset_path)


def rebuild_warning_materials():
    rebuild_simple_unlit_material(
        "/Game/UI/Preview/M_PreviewSky",
        "SkyColor",
        (2.0, 3.0, 5.0, 1.0),
    )
    rebuild_simple_unlit_material(
        "/Game/UI/Materials/M_UI_RetroSkyBorder",
        "BorderColor",
        (0.55, 0.16, 1.0, 1.0),
        [("PatternScale", 18.0), ("ScrollSpeed", 0.0)],
    )
    rebuild_simple_unlit_material(
        "/Game/UI/Materials/M_UI_RetroSkyBorderOverlay",
        "BorderColor",
        (0.55, 0.16, 1.0, 1.0),
        [("PatternScale", 18.0), ("ScrollSpeed", 0.0)],
    )
    rebuild_simple_unlit_material(
        "/Game/UI/Materials/M_UI_RetroWoodTrim",
        "TrimColor",
        (0.55, 0.16, 1.0, 1.0),
        [("PatternScale", 12.0), ("ScrollSpeed", 0.0)],
    )
    rebuild_simple_unlit_material(
        "/Game/UI/Materials/M_UI_RetroWoodTrimV2",
        "TrimColor",
        (0.55, 0.16, 1.0, 1.0),
        [("PatternScale", 12.0), ("ScrollSpeed", 0.0)],
    )

    update_material_instances([
        "/Game/UI/Materials/MI_UI_RetroSkyBorder_N",
        "/Game/UI/Materials/MI_UI_RetroSkyBorder_H",
        "/Game/UI/Materials/MI_UI_RetroSkyBorder_P",
        "/Game/UI/Materials/MI_UI_RetroSkyBorderOverlay_N",
        "/Game/UI/Materials/MI_UI_RetroSkyBorderOverlay_H",
        "/Game/UI/Materials/MI_UI_RetroSkyBorderOverlay_P",
        "/Game/UI/Materials/MI_UI_RetroWoodTrim_N",
        "/Game/UI/Materials/MI_UI_RetroWoodTrim_H",
        "/Game/UI/Materials/MI_UI_RetroWoodTrim_P",
        "/Game/UI/Materials/MI_UI_RetroWoodTrimV2_N",
        "/Game/UI/Materials/MI_UI_RetroWoodTrimV2_H",
        "/Game/UI/Materials/MI_UI_RetroWoodTrimV2_P",
    ])


def reload_character_visuals_data_table():
    dt_path = "/Game/Data/DT_CharacterVisuals"
    dt = load_asset(dt_path)
    project_dir = unreal.SystemLibrary.get_project_directory().replace("\\", "/").rstrip("/")
    csv_path = os.path.normpath(os.path.join(project_dir, "Content", "Data", "CharacterVisuals.csv"))
    if not os.path.exists(csv_path):
        raise RuntimeError(f"Missing CharacterVisuals.csv: {csv_path}")

    if unreal.DataTableFunctionLibrary.fill_data_table_from_csv_file(dt, csv_path):
        save_asset(dt_path)
        log("Reloaded DT_CharacterVisuals from CSV")
    else:
        raise RuntimeError("Failed to reload DT_CharacterVisuals from CSV")


def remove_party_size_picker_references():
    # BP_T66PlayerController used to serialize ScreenClasses. The controller now uses
    # RuntimeScreenClasses, so the obsolete serialized map is ignored instead of parsed.
    controller_path = "/Game/Blueprints/Core/BP_T66PlayerController"
    controller_bp = load_asset(controller_path)

    compiled = False
    blueprint_library = getattr(unreal, "BlueprintEditorLibrary", None)
    if blueprint_library and hasattr(blueprint_library, "compile_blueprint"):
        blueprint_library.compile_blueprint(controller_bp)
        compiled = True
    else:
        kismet_utilities = getattr(unreal, "KismetEditorUtilities", None)
        if kismet_utilities and hasattr(kismet_utilities, "compile_blueprint"):
            kismet_utilities.compile_blueprint(controller_bp)
            compiled = True

    save_loaded_asset(controller_bp, controller_path)
    compile_status = "compiled and " if compiled else ""
    log(f"BP_T66PlayerController {compile_status}resaved without serialized ScreenClasses")

    picker_path = "/Game/Blueprints/UI/WBP_PartySizePicker"
    if unreal.EditorAssetLibrary.does_asset_exist(picker_path):
        if unreal.EditorAssetLibrary.delete_asset(picker_path):
            log("Deleted stale WBP_PartySizePicker asset")
        else:
            warn("Failed to delete WBP_PartySizePicker asset")
    else:
        log("WBP_PartySizePicker already absent")


def clear_demo_skeleton_legacy_rig():
    skeleton_path = "/Game/Stylized_VFX_StPack/DEMO/Character/SK_Mannequin_Skeleton"
    skeleton = unreal.EditorAssetLibrary.load_asset(skeleton_path)
    if not skeleton:
        log("Demo mannequin skeleton already absent")
        return

    save_asset(skeleton_path)
    log("Resaved demo mannequin skeleton to drop obsolete serialized fields")


def main():
    reload_character_visuals_data_table()
    remove_party_size_picker_references()
    clear_demo_skeleton_legacy_rig()
    rebuild_warning_materials()
    log("Done")


if __name__ == "__main__":
    main()
