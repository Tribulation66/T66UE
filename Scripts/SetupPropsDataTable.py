"""
Reload DT_Props DataTable from Content/Data/Props.csv.

Run INSIDE Unreal Editor:
  py "C:/UE/T66/Scripts/SetupPropsDataTable.py"
"""

import os
import unreal


def main():
    unreal.log("=== SetupPropsDataTable ===")

    dt_path = "/Game/Data/DT_Props"
    dt = unreal.EditorAssetLibrary.load_asset(dt_path)
    if not dt:
        unreal.log_error(f"DT_Props not found at {dt_path}. Create it in-editor first.")
        return

    try:
        proj = unreal.SystemLibrary.get_project_directory()
    except Exception:
        proj = os.path.abspath(os.path.join(os.getcwd(), ".."))

    csv_path = os.path.normpath(os.path.join(proj, "Content", "Data", "Props.csv"))
    if not os.path.isfile(csv_path):
        unreal.log_error(f"Props.csv not found at {csv_path}")
        return

    unreal.log(f"Loading CSV: {csv_path}")
    success = unreal.DataTableFunctionLibrary.fill_data_table_from_csv_file(dt, csv_path)
    if success:
        unreal.EditorAssetLibrary.save_asset(dt_path)
        unreal.log("DT_Props reloaded and saved.")
    else:
        unreal.log_error("Failed to fill DT_Props from CSV.")

    unreal.log("=== SetupPropsDataTable DONE ===")


if __name__ == "__main__":
    main()
