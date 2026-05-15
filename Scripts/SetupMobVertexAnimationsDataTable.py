"""
Create or reload DT_MobVertexAnimations from Content/Data/MobVertexAnimations.csv.

Run from the editor or command line:
  UnrealEditor-Cmd.exe T66.uproject -run=pythonscript -script="C:/UE/T66/Scripts/SetupMobVertexAnimationsDataTable.py"
"""

import os

import unreal


DT_PATH = "/Game/Data/DT_MobVertexAnimations"
CSV_NAME = "MobVertexAnimations.csv"
STRUCT_NAMES = ("T66MobVertexAnimationRow", "MobVertexAnimationRow")


def resolve_row_struct():
    for struct_name in STRUCT_NAMES:
        struct_type = getattr(unreal, struct_name, None)
        if struct_type is None:
            continue
        if hasattr(struct_type, "static_struct"):
            return struct_type.static_struct()
        return struct_type
    unreal.log_error(f"Could not resolve mob VAT row struct from {STRUCT_NAMES}")
    return None


def load_or_create_datatable(row_struct):
    if unreal.EditorAssetLibrary.does_asset_exist(DT_PATH):
        dt = unreal.EditorAssetLibrary.load_asset(DT_PATH)
        if dt:
            return dt

    factory = unreal.DataTableFactory()
    factory.set_editor_property("struct", row_struct)
    package_path, asset_name = DT_PATH.rsplit("/", 1)
    dt = unreal.AssetToolsHelpers.get_asset_tools().create_asset(asset_name, package_path, unreal.DataTable, factory)
    if not dt:
        unreal.log_error(f"Failed to create DataTable asset at {DT_PATH}")
        return None

    unreal.log(f"Created DataTable asset at {DT_PATH}")
    return dt


def main():
    unreal.log("=== SetupMobVertexAnimationsDataTable ===")

    row_struct = resolve_row_struct()
    if row_struct is None:
        return

    project_dir = unreal.SystemLibrary.get_project_directory().replace("\\", "/").rstrip("/")
    csv_path = os.path.normpath(os.path.join(project_dir, "Content", "Data", CSV_NAME))
    if not os.path.exists(csv_path):
        unreal.log_error(f"{CSV_NAME} not found at {csv_path}")
        return

    dt = load_or_create_datatable(row_struct)
    if not dt:
        return

    success = unreal.DataTableFunctionLibrary.fill_data_table_from_csv_file(dt, csv_path)
    if not success:
        unreal.log_error(f"Failed to fill {DT_PATH} from {csv_path}")
        return

    if unreal.EditorAssetLibrary.save_asset(DT_PATH):
        unreal.log(f"Reloaded {DT_PATH} from {csv_path}")
    else:
        unreal.log_error(f"Failed to save {DT_PATH}")

    unreal.log("=== SetupMobVertexAnimationsDataTable DONE ===")


if __name__ == "__main__":
    main()
