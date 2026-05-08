"""
Reload DT_HouseNPCs DataTable from Content/Data/HouseNPCs.csv.

Run in-editor:
  py "C:/UE/T66/Scripts/SetupHouseNPCsDataTable.py"
"""

import os
import unreal


def main():
    unreal.log("=== SetupHouseNPCsDataTable ===")

    dt_path = "/Game/Data/DT_HouseNPCs"
    dt = unreal.EditorAssetLibrary.load_asset(dt_path)
    if not dt:
        unreal.log_error(f"DT_HouseNPCs not found at {dt_path}. Create it in-editor first.")
        return

    project_dir = unreal.SystemLibrary.get_project_directory().replace("\\", "/").rstrip("/")
    csv_path = os.path.normpath(os.path.join(project_dir, "Content", "Data", "HouseNPCs.csv"))
    if not os.path.exists(csv_path):
        unreal.log_error(f"HouseNPCs.csv not found at {csv_path}")
        return

    success = unreal.DataTableFunctionLibrary.fill_data_table_from_csv_file(dt, csv_path)
    if success:
        unreal.EditorAssetLibrary.save_asset(dt_path)
        unreal.log("DT_HouseNPCs reloaded and saved.")
    else:
        unreal.log_error("Failed to fill DT_HouseNPCs from CSV.")

    unreal.log("=== SetupHouseNPCsDataTable DONE ===")


if __name__ == "__main__":
    main()
