"""
Create or reload DT_PlayerExperience from Content/Data/PlayerExperience.json.

Run from the editor or command line:
  UnrealEditor-Cmd.exe T66.uproject -run=pythonscript -script="C:/UE/T66/Scripts/SetupPlayerExperienceDataTable.py"
"""

import os
import unreal


DT_PATH = "/Game/Data/DT_PlayerExperience"


def resolve_row_struct():
    struct_type = getattr(unreal, "T66PlayerExperienceDifficultyTuning", None)
    if struct_type is None:
        unreal.log_error("Could not resolve T66PlayerExperienceDifficultyTuning in Python.")
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


def main():
    unreal.log("=== SetupPlayerExperienceDataTable ===")

    row_struct = resolve_row_struct()
    if row_struct is None:
        return

    project_dir = unreal.SystemLibrary.get_project_directory().replace("\\", "/").rstrip("/")
    json_path = os.path.normpath(os.path.join(project_dir, "Content", "Data", "PlayerExperience.json"))
    if not os.path.exists(json_path):
        unreal.log_error(f"PlayerExperience.json not found at {json_path}")
        return

    data_table = load_or_create_datatable(row_struct)
    if not data_table:
        return

    success = unreal.DataTableFunctionLibrary.fill_data_table_from_json_file(data_table, json_path, row_struct)
    if not success:
        unreal.log_error("Failed to fill DT_PlayerExperience from JSON.")
        return

    if not unreal.EditorAssetLibrary.save_asset(DT_PATH):
        unreal.log_error(f"Failed to save {DT_PATH}")
        return

    unreal.log(f"DT_PlayerExperience reloaded from {json_path}")
    unreal.log("=== SetupPlayerExperienceDataTable DONE ===")


if __name__ == "__main__":
    main()
