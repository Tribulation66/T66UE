"""
Import Content/Data/Stages.csv into /Game/Data/DT_Stages.

Run:
  UnrealEditor-Cmd.exe T66.uproject -run=pythonscript -script="Scripts/SetupStagesDataTable.py"
"""

import unreal
import os


def get_content_path(relative_path: str) -> str:
    try:
        proj = unreal.SystemLibrary.get_project_directory()
        return os.path.normpath(os.path.join(proj, "Content", relative_path))
    except Exception:
        return os.path.normpath(os.path.join("Content", relative_path))


def main():
    unreal.log("=== Importing DT_Stages from CSV ===")

    dt_stages = unreal.EditorAssetLibrary.load_asset("/Game/Data/DT_Stages")
    if not dt_stages:
        unreal.log_error("Could not load /Game/Data/DT_Stages")
        return

    csv_path = get_content_path("Data/Stages.csv")
    if not os.path.isfile(csv_path):
        unreal.log_error("Stages CSV not found: " + csv_path)
        return

    success = unreal.DataTableFunctionLibrary.fill_data_table_from_csv_file(dt_stages, csv_path)
    if success:
        unreal.log("Successfully imported Stages from CSV")
        unreal.EditorAssetLibrary.save_asset("/Game/Data/DT_Stages")
        row_names = unreal.DataTableFunctionLibrary.get_data_table_row_names(dt_stages)
        unreal.log("DT_Stages now has {} rows".format(len(row_names)))
    else:
        unreal.log_error("Failed to import Stages from CSV")


if __name__ == "__main__":
    main()

