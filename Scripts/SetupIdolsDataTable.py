"""
One-shot setup for DT_Idols and Game Instance reference.
1) Creates DT_Idols if missing (FIdolData struct).
2) Fills DT_Idols from Content/Data/Idols.csv.
3) Sets BP_T66GameInstance.IdolsDataTable reference.

Run from Unreal Editor: Py Execute Script -> Scripts/SetupIdolsDataTable.py
Or from project root:
  UnrealEditor-Cmd.exe T66.uproject -run=pythonscript -script="Scripts/SetupIdolsDataTable.py"
"""

import unreal
import os


def get_content_path(relative_path):
    try:
        proj = unreal.SystemLibrary.get_project_directory()
        return os.path.normpath(os.path.join(proj, "Content", relative_path))
    except Exception:
        return os.path.normpath(os.path.join("Content", relative_path))


def set_game_instance_idols_dt(dt_asset_path: str):
    bp_path = "/Game/Blueprints/Core/BP_T66GameInstance"
    bp_gc = unreal.EditorAssetLibrary.load_blueprint_class(bp_path)
    if not bp_gc:
        unreal.log_warning(f"Could not load blueprint class: {bp_path}")
        return
    cdo = unreal.get_default_object(bp_gc)
    if not cdo:
        unreal.log_warning("Could not get BP_T66GameInstance CDO")
        return
    dt = unreal.EditorAssetLibrary.load_asset(dt_asset_path)
    if not dt:
        unreal.log_warning(f"Could not load {dt_asset_path} to assign to BP_T66GameInstance")
        return
    try:
        cdo.set_editor_property("IdolsDataTable", dt)
        unreal.EditorAssetLibrary.save_asset(bp_path)
        unreal.log("Set IdolsDataTable on BP_T66GameInstance")
    except Exception as e:
        unreal.log_warning(f"Failed setting IdolsDataTable on BP_T66GameInstance: {e}")


def main():
    unreal.log("=== T66 Idols DataTable Setup ===")

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

    # 1) Create DT_Idols if not exists
    dt_path = "/Game/Data/DT_Idols"
    if not unreal.EditorAssetLibrary.does_asset_exist(dt_path):
        idol_struct = unreal.find_object(None, "/Script/T66.IdolData")
        if not idol_struct:
            unreal.log_error("FIdolData struct not found. Rebuild the T66 module.")
            return
        factory = unreal.DataTableFactory()
        factory.set_editor_property("struct", idol_struct)
        data_table = asset_tools.create_asset("DT_Idols", "/Game/Data", None, factory)
        if data_table:
            unreal.log("Created DT_Idols")
            unreal.EditorAssetLibrary.save_asset(dt_path)
        else:
            unreal.log_error("Failed to create DT_Idols")
            return
    else:
        unreal.log("DT_Idols already exists")

    # 2) Fill from CSV
    dt_idols = unreal.EditorAssetLibrary.load_asset(dt_path)
    if not dt_idols:
        unreal.log_error("Could not load DT_Idols")
        return
    csv_path = get_content_path("Data/Idols.csv")
    if not os.path.isfile(csv_path):
        unreal.log_error("Idols CSV not found: " + csv_path)
        return
    success = unreal.DataTableFunctionLibrary.fill_data_table_from_csv_file(dt_idols, csv_path)
    if success:
        unreal.log("Filled DT_Idols from Idols.csv")
        unreal.EditorAssetLibrary.save_asset(dt_path)
    else:
        unreal.log_error("Failed to fill DT_Idols from CSV")

    # 3) Set BP_T66GameInstance reference
    set_game_instance_idols_dt(dt_path)

    unreal.log("=== Idols Setup Done ===")


if __name__ == "__main__":
    main()

