"""
Simple script to import CSV data into DataTables
"""

import unreal

def main():
    unreal.log("=== Importing DataTable Data ===")

    # Import Heroes
    dt_heroes = unreal.EditorAssetLibrary.load_asset("/Game/Data/DT_Heroes")
    if dt_heroes:
        csv_path = "C:/UE/T66/Content/Data/Heroes.csv"
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
        csv_path = "C:/UE/T66/Content/Data/Companions.csv"
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

    unreal.log("=== Data Import Complete ===")

if __name__ == "__main__":
    main()
