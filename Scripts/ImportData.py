"""
Simple script to import CSV data into DataTables
Run from Unreal Editor: Py Execute Script -> Scripts/ImportData.py
Or: UnrealEditor-Cmd.exe ProjectPath -run=pythonscript -script="Scripts/ImportData.py"
"""

import unreal
import os

def get_content_path(relative_path):
    """Resolve path under Content/ (works when run from project root or editor)"""
    try:
        proj = unreal.SystemLibrary.get_project_directory()
        return os.path.normpath(os.path.join(proj, "Content", relative_path))
    except Exception:
        return os.path.normpath(os.path.join("Content", relative_path))

def main():
    unreal.log("=== Importing DataTable Data ===")

    # Import Heroes
    dt_heroes = unreal.EditorAssetLibrary.load_asset("/Game/Data/DT_Heroes")
    if dt_heroes:
        csv_path = get_content_path("Data/Heroes.csv")
        success = unreal.DataTableFunctionLibrary.fill_data_table_from_csv_file(dt_heroes, csv_path)
        if success:
            unreal.log("Successfully imported Heroes from CSV")
            unreal.EditorAssetLibrary.save_asset("/Game/Data/DT_Heroes")
            row_names = unreal.DataTableFunctionLibrary.get_data_table_row_names(dt_heroes)
            unreal.log(f"DT_Heroes now has {len(row_names)} rows")
        else:
            unreal.log_error("Failed to import Heroes")
    else:
        unreal.log_error("Could not load DT_Heroes")

    # Import Companions
    dt_companions = unreal.EditorAssetLibrary.load_asset("/Game/Data/DT_Companions")
    if dt_companions:
        csv_path = get_content_path("Data/Companions.csv")
        success = unreal.DataTableFunctionLibrary.fill_data_table_from_csv_file(dt_companions, csv_path)
        if success:
            unreal.log("Successfully imported Companions from CSV")
            unreal.EditorAssetLibrary.save_asset("/Game/Data/DT_Companions")
            row_names = unreal.DataTableFunctionLibrary.get_data_table_row_names(dt_companions)
            unreal.log(f"DT_Companions now has {len(row_names)} rows")
        else:
            unreal.log_error("Failed to import Companions")
    else:
        unreal.log_error("Could not load DT_Companions")

    # Import Items
    dt_items = unreal.EditorAssetLibrary.load_asset("/Game/Data/DT_Items")
    if dt_items:
        csv_path = get_content_path("Data/Items.csv")
        if os.path.isfile(csv_path):
            success = unreal.DataTableFunctionLibrary.fill_data_table_from_csv_file(dt_items, csv_path)
            if success:
                unreal.log("Successfully imported Items from CSV")
                unreal.EditorAssetLibrary.save_asset("/Game/Data/DT_Items")
                row_names = unreal.DataTableFunctionLibrary.get_data_table_row_names(dt_items)
                unreal.log("DT_Items now has {} rows".format(len(row_names)))
            else:
                unreal.log_error("Failed to import Items from CSV")
        else:
            unreal.log_error("Items CSV not found: " + csv_path)
    else:
        unreal.log_error("Could not load DT_Items (create it first via CreateAssets.py)")

    unreal.log("=== Data Import Complete ===")

if __name__ == "__main__":
    main()
