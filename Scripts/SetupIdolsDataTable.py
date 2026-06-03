"""
Create or reload DT_Idols from Content/Data/Idols.csv.

Run from the editor or command line:
  UnrealEditor-Cmd.exe T66.uproject -run=pythonscript -script="C:/UE/T66/Scripts/SetupIdolsDataTable.py"
"""

import os

try:
    import unreal
except ImportError:
    unreal = None


DT_PATH = "/Game/Data/DT_Idols"
BP_GAME_INSTANCE_ASSET_PATH = "/Game/Blueprints/Core/BP_T66GameInstance"
BP_GAME_INSTANCE_CLASS_PATH = "/Game/Blueprints/Core/BP_T66GameInstance.BP_T66GameInstance_C"


def resolve_row_struct():
    for name in ("IdolData", "T66IdolData", "FIdolData"):
        struct_type = getattr(unreal, name, None)
        if struct_type is None:
            continue
        if hasattr(struct_type, "static_struct"):
            return struct_type.static_struct()
        return struct_type

    unreal.log_error("Could not resolve FIdolData in Python.")
    return None


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


def assign_game_instance_data_table(data_table):
    bp_class = unreal.load_object(None, BP_GAME_INSTANCE_CLASS_PATH)
    if not bp_class:
        unreal.log_warning(f"Could not load BP_T66GameInstance class at {BP_GAME_INSTANCE_CLASS_PATH}")
        return

    cdo = unreal.get_default_object(bp_class)
    if not cdo:
        unreal.log_warning("Could not resolve BP_T66GameInstance CDO.")
        return

    cdo.set_editor_property("IdolsDataTable", data_table)

    if unreal.EditorAssetLibrary.save_asset(BP_GAME_INSTANCE_ASSET_PATH):
        unreal.log("Assigned IdolsDataTable on BP_T66GameInstance.")
    else:
        unreal.log_warning("Failed to save BP_T66GameInstance after assigning IdolsDataTable.")


def main():
    if unreal is None:
        raise RuntimeError("Unreal Python module is required to create or reload DT_Idols.")

    unreal.log("=== SetupIdolsDataTable ===")

    project_dir = unreal.SystemLibrary.get_project_directory().replace("\\", "/").rstrip("/")
    csv_path = os.path.normpath(os.path.join(project_dir, "Content", "Data", "Idols.csv"))
    if not os.path.exists(csv_path):
        unreal.log_error(f"Idols.csv not found at {csv_path}")
        return

    row_struct = resolve_row_struct()
    if row_struct is None:
        return

    data_table = load_or_create_datatable(row_struct)
    if not data_table:
        return

    success = unreal.DataTableFunctionLibrary.fill_data_table_from_csv_file(data_table, csv_path)
    if not success:
        unreal.log_error("Failed to fill DT_Idols from CSV.")
        return

    if not unreal.EditorAssetLibrary.save_asset(DT_PATH):
        unreal.log_error(f"Failed to save {DT_PATH}")
        return

    assign_game_instance_data_table(data_table)
    unreal.log(f"Reloaded {DT_PATH} from {csv_path}")


if __name__ == "__main__":
    main()
