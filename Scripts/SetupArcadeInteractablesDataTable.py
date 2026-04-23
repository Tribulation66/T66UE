"""
Create or reload DT_ArcadeInteractables from Content/Data/ArcadeInteractables.json.

Run from the editor or command line:
  UnrealEditor-Cmd.exe T66.uproject -run=pythonscript -script="C:/UE/T66/Scripts/SetupArcadeInteractablesDataTable.py"
"""

import os
import unreal


DT_PATH = "/Game/Data/DT_ArcadeInteractables"
BP_GAME_INSTANCE_ASSET_PATH = "/Game/Blueprints/Core/BP_T66GameInstance"
BP_GAME_INSTANCE_CLASS_PATH = "/Game/Blueprints/Core/BP_T66GameInstance.BP_T66GameInstance_C"


def resolve_row_struct():
    struct_type = getattr(unreal, "T66ArcadeInteractableRow", None)
    if struct_type is None:
        unreal.log_error("Could not resolve T66ArcadeInteractableRow in Python.")
        return None

    if hasattr(struct_type, "static_struct"):
        return struct_type.static_struct()

    return struct_type


def load_or_create_datatable(row_struct):
    if unreal.EditorAssetLibrary.does_asset_exist(DT_PATH):
        data_table = unreal.EditorAssetLibrary.load_asset(DT_PATH)
        if data_table:
            return data_table

    factory = unreal.DataTableFactory()
    factory.set_editor_property("struct", row_struct)

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    package_path, asset_name = DT_PATH.rsplit("/", 1)
    data_table = asset_tools.create_asset(asset_name, package_path, unreal.DataTable, factory)
    if not data_table:
        unreal.log_error(f"Failed to create DataTable asset at {DT_PATH}")
        return None

    unreal.log(f"Created DataTable asset at {DT_PATH}")
    return data_table


def assign_game_instance_data_table(data_table):
    bp_class = unreal.load_object(None, BP_GAME_INSTANCE_CLASS_PATH)
    if not bp_class:
        unreal.log_warning(f"Could not load BP_T66GameInstance class at {BP_GAME_INSTANCE_CLASS_PATH}")
        return

    cdo = unreal.get_default_object(bp_class)
    if not cdo:
        unreal.log_warning("Could not resolve BP_T66GameInstance CDO.")
        return

    cdo.set_editor_property("ArcadeInteractablesDataTable", data_table)

    if unreal.EditorAssetLibrary.save_asset(BP_GAME_INSTANCE_ASSET_PATH):
        unreal.log("Assigned ArcadeInteractablesDataTable on BP_T66GameInstance.")
    else:
        unreal.log_warning("Failed to save BP_T66GameInstance after assigning ArcadeInteractablesDataTable.")


def main():
    unreal.log("=== SetupArcadeInteractablesDataTable ===")

    row_struct = resolve_row_struct()
    if row_struct is None:
        return

    project_dir = unreal.SystemLibrary.get_project_directory().replace("\\", "/").rstrip("/")
    json_path = os.path.normpath(os.path.join(project_dir, "Content", "Data", "ArcadeInteractables.json"))
    if not os.path.exists(json_path):
        unreal.log_error(f"ArcadeInteractables.json not found at {json_path}")
        return

    data_table = load_or_create_datatable(row_struct)
    if not data_table:
        return

    success = unreal.DataTableFunctionLibrary.fill_data_table_from_json_file(data_table, json_path, row_struct)
    if not success:
        unreal.log_error("Failed to fill DT_ArcadeInteractables from JSON.")
        return

    if not unreal.EditorAssetLibrary.save_asset(DT_PATH):
        unreal.log_error(f"Failed to save {DT_PATH}")
        return

    assign_game_instance_data_table(data_table)

    unreal.log(f"DT_ArcadeInteractables reloaded from {json_path}")
    unreal.log("=== SetupArcadeInteractablesDataTable DONE ===")


if __name__ == "__main__":
    main()
