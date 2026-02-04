"""
One-shot setup for DT_CharacterVisuals.
1) Creates DT_CharacterVisuals if missing (FT66CharacterVisualRow struct).
2) Fills it from Content/Data/CharacterVisuals.csv.

Run from Unreal Editor: Py Execute Script -> Scripts/SetupCharacterVisualsDataTable.py
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
    unreal.log("=== T66 Character Visuals DataTable Setup ===")

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

    dt_path = "/Game/Data/DT_CharacterVisuals"
    if not unreal.EditorAssetLibrary.does_asset_exist(dt_path):
        row_struct = unreal.find_object(None, "/Script/T66.T66CharacterVisualRow")
        if not row_struct:
            unreal.log_error("FT66CharacterVisualRow struct not found. Rebuild the T66 module.")
            return

        factory = unreal.DataTableFactory()
        factory.set_editor_property("struct", row_struct)
        data_table = asset_tools.create_asset("DT_CharacterVisuals", "/Game/Data", None, factory)
        if data_table:
            unreal.log("Created DT_CharacterVisuals")
            unreal.EditorAssetLibrary.save_asset(dt_path)
        else:
            unreal.log_error("Failed to create DT_CharacterVisuals")
            return
    else:
        unreal.log("DT_CharacterVisuals already exists")

    dt = unreal.EditorAssetLibrary.load_asset(dt_path)
    if not dt:
        unreal.log_error("Could not load DT_CharacterVisuals")
        return

    csv_path = get_content_path("Data/CharacterVisuals.csv")
    if not os.path.isfile(csv_path):
        unreal.log_error("CharacterVisuals CSV not found: " + csv_path)
        return

    success = unreal.DataTableFunctionLibrary.fill_data_table_from_csv_file(dt, csv_path)
    if success:
        unreal.log("Filled DT_CharacterVisuals from CharacterVisuals.csv")
        unreal.EditorAssetLibrary.save_asset(dt_path)
    else:
        unreal.log_error("Failed to fill DT_CharacterVisuals from CSV")

    unreal.log("=== Character Visuals Setup Done ===")
    # Do not quit the editor; leave it open so you can continue working.


if __name__ == "__main__":
    main()

