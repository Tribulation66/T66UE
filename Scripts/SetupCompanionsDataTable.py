"""
Reload DT_Companions DataTable from Content/Data/Companions.csv.

Run in-editor:
  py "C:/UE/T66/Scripts/SetupCompanionsDataTable.py"
"""

import os
import unreal


def main():
    unreal.log("=== SetupCompanionsDataTable ===")

    dt_path = "/Game/Data/DT_Companions"
    dt = unreal.EditorAssetLibrary.load_asset(dt_path)
    if not dt:
        unreal.log_error(f"DT_Companions not found at {dt_path}. Create it in-editor first.")
        return

    proj = unreal.SystemLibrary.get_project_directory().replace("\\", "/").rstrip("/")
    csv_path = os.path.normpath(os.path.join(proj, "Content", "Data", "Companions.csv"))
    if not os.path.exists(csv_path):
        unreal.log_error(f"Companions.csv not found at {csv_path}")
        return

    success = unreal.DataTableFunctionLibrary.fill_data_table_from_csv_file(dt, csv_path)
    if success:
        unreal.EditorAssetLibrary.save_asset(dt_path)
        unreal.log("DT_Companions reloaded and saved.")
    else:
        unreal.log_error("Failed to fill DT_Companions from CSV.")

    unreal.log("=== SetupCompanionsDataTable DONE ===")


if __name__ == "__main__":
    main()
