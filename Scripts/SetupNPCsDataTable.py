"""
Create or reload DT_NPCs DataTable from Content/Data/NPCs.csv.

Run in-editor:
  py "C:/UE/T66/Scripts/SetupNPCsDataTable.py"
"""

import os
import unreal


DT_PATH = "/Game/Data/DT_NPCs"


def resolve_row_struct():
    struct_type = getattr(unreal, "HouseNPCData", None)
    if struct_type is None:
        unreal.log_error("Could not resolve HouseNPCData in Python.")
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
    unreal.log("=== SetupNPCsDataTable ===")

    row_struct = resolve_row_struct()
    if row_struct is None:
        return

    project_dir = unreal.SystemLibrary.get_project_directory().replace("\\", "/").rstrip("/")
    csv_path = os.path.normpath(os.path.join(project_dir, "Content", "Data", "NPCs.csv"))
    if not os.path.exists(csv_path):
        unreal.log_error(f"NPCs.csv not found at {csv_path}")
        return

    data_table = load_or_create_datatable(row_struct)
    if not data_table:
        return

    success = unreal.DataTableFunctionLibrary.fill_data_table_from_csv_file(data_table, csv_path)
    if success:
        unreal.EditorAssetLibrary.save_asset(DT_PATH)
        unreal.log("DT_NPCs reloaded and saved.")
    else:
        unreal.log_error("Failed to fill DT_NPCs from CSV.")

    unreal.log("=== SetupNPCsDataTable DONE ===")


if __name__ == "__main__":
    main()