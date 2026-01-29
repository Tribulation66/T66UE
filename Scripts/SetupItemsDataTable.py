"""
One-shot setup for DT_Items and Game Instance reference.
1) Creates DT_Items if missing (FItemData struct).
2) Fills DT_Items from Content/Data/Items.csv.
3) Runs T66Setup so BP_T66GameInstance gets ItemsDataTable set.

Run from Unreal Editor: Py Execute Script -> Scripts/SetupItemsDataTable.py
Or from project root (with Unreal Python): 
  UnrealEditor-Cmd.exe T66.uproject -run=pythonscript -script="Scripts/SetupItemsDataTable.py"
"""

import unreal
import os

def get_content_path(relative_path):
    try:
        proj = unreal.SystemLibrary.get_project_directory()
        return os.path.normpath(os.path.join(proj, "Content", relative_path))
    except Exception:
        return os.path.normpath(os.path.join("Content", relative_path))

def main():
    unreal.log("=== T66 Items DataTable Setup ===")

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

    # 1) Create DT_Items if not exists
    dt_path = "/Game/Data/DT_Items"
    if not unreal.EditorAssetLibrary.does_asset_exist(dt_path):
        item_struct = unreal.find_object(None, "/Script/T66.ItemData")
        if not item_struct:
            unreal.log_error("FItemData struct not found. Rebuild the T66 module.")
            return
        factory = unreal.DataTableFactory()
        factory.set_editor_property("struct", item_struct)
        data_table = asset_tools.create_asset("DT_Items", "/Game/Data", None, factory)
        if data_table:
            unreal.log("Created DT_Items")
            unreal.EditorAssetLibrary.save_asset(dt_path)
        else:
            unreal.log_error("Failed to create DT_Items")
            return
    else:
        unreal.log("DT_Items already exists")

    # 2) Fill from CSV
    dt_items = unreal.EditorAssetLibrary.load_asset(dt_path)
    if not dt_items:
        unreal.log_error("Could not load DT_Items")
        return
    csv_path = get_content_path("Data/Items.csv")
    if not os.path.isfile(csv_path):
        unreal.log_error("Items CSV not found: " + csv_path)
        return
    success = unreal.DataTableFunctionLibrary.fill_data_table_from_csv_file(dt_items, csv_path)
    if success:
        unreal.log("Filled DT_Items from Items.csv")
        unreal.EditorAssetLibrary.save_asset(dt_path)
    else:
        unreal.log_error("Failed to fill DT_Items from CSV")

    # 3) Run T66Setup so ConfigureGameInstance sets ItemsDataTable on BP_T66GameInstance
    try:
        world = unreal.EditorLevelLibrary.get_editor_world()
        if world:
            unreal.SystemLibrary.execute_console_command(world, "T66Setup", unreal.EditorLevelLibrary.get_editor_player_controller())
            unreal.log("Ran T66Setup - Game Instance should now reference DT_Items")
        else:
            unreal.log("Could not get editor world; run 'T66Setup' in the console to set Game Instance ItemsDataTable")
    except Exception as e:
        unreal.log("Run 'T66Setup' in the Editor console to set BP_T66GameInstance.ItemsDataTable. Error: " + str(e))

    unreal.log("=== Items Setup Done ===")

if __name__ == "__main__":
    main()
