"""
Reload DT_Items DataTable from Content/Data/Items.csv.

Run in-editor:
  py "C:/UE/T66/Scripts/SetupItemsDataTable.py"
"""

import os
import unreal


def main():
    unreal.log("=== SetupItemsDataTable ===")

    dt_path = "/Game/Data/DT_Items"
    dt = unreal.EditorAssetLibrary.load_asset(dt_path)
    if not dt:
        unreal.log_error(f"DT_Items not found at {dt_path}. Create it in-editor first.")
        return

    proj = unreal.SystemLibrary.get_project_directory().replace("\\", "/").rstrip("/")
    csv_path = os.path.normpath(os.path.join(proj, "Content", "Data", "Items.csv"))
    if not os.path.exists(csv_path):
        unreal.log_error(f"Items.csv not found at {csv_path}")
        return

    success = unreal.DataTableFunctionLibrary.fill_data_table_from_csv_file(dt, csv_path)
    if success:
        unreal.EditorAssetLibrary.save_asset(dt_path)
        unreal.log("DT_Items reloaded and saved.")
    else:
        unreal.log_error("Failed to fill DT_Items from CSV.")

    unreal.log("=== SetupItemsDataTable DONE ===")


if __name__ == "__main__":
    main()
