"""
One-shot setup for local leaderboard target DataTables.

Creates (or fixes) these DataTables under /Game/Data:
  - DT_Leaderboard_BountyTargets   (FT66LeaderboardBountyTargetRow)
  - DT_Leaderboard_SpeedrunTargets (FT66LeaderboardSpeedRunTargetRow)

Then fills them from:
  Content/Data/Leaderboard_BountyTargets.csv
  Content/Data/Leaderboard_SpeedrunTargets.csv

Run from project root:
  UnrealEditor-Cmd.exe T66.uproject -run=pythonscript -script="Scripts/SetupLeaderboardDataTables.py"
"""

import unreal
import os
import time


def get_content_path(relative_path: str) -> str:
    try:
        proj = unreal.SystemLibrary.get_project_directory()
        return os.path.normpath(os.path.join(proj, "Content", relative_path))
    except Exception:
        return os.path.normpath(os.path.join("Content", relative_path))


def _rename_asset_if_exists(asset_path: str, new_name: str) -> bool:
    if not unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        return False

    asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    if not asset:
        return False

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    rename_data = unreal.AssetRenameData(asset, "/Game/Data", new_name)
    results = asset_tools.rename_assets([rename_data])
    return bool(results and results[0])


def _ensure_datatable(dt_name: str, dt_path: str, struct_path: str):
    """
    Ensure the DataTable exists at dt_path and has row struct struct_path.
    If an existing DT has the wrong struct, it is renamed to *_BAD_<timestamp> and recreated.
    """
    desired_struct = unreal.find_object(None, struct_path)
    if not desired_struct:
        unreal.log_error(f"Struct not found: {struct_path}. Rebuild the T66 module.")
        return None

    if unreal.EditorAssetLibrary.does_asset_exist(dt_path):
        existing = unreal.EditorAssetLibrary.load_asset(dt_path)
        if existing:
            try:
                row_struct = existing.get_editor_property("row_struct")
            except Exception:
                row_struct = None

            if row_struct == desired_struct:
                unreal.log(f"{dt_path} already exists with correct row struct.")
                return existing

            # Wrong struct: rename out of the way.
            ts = time.strftime("%Y%m%d_%H%M%S")
            bad_name = f"{dt_name}_BAD_{ts}"
            unreal.log_warning(f"{dt_path} exists with wrong row struct. Renaming to {bad_name} and recreating.")
            renamed = _rename_asset_if_exists(dt_path, bad_name)
            if not renamed:
                unreal.log_warning(f"Rename failed; attempting delete of {dt_path}.")
                unreal.EditorAssetLibrary.delete_asset(dt_path)

    # Create new DT
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    factory = unreal.DataTableFactory()
    factory.set_editor_property("struct", desired_struct)
    created = asset_tools.create_asset(dt_name, "/Game/Data", None, factory)
    if not created:
        unreal.log_error(f"Failed to create {dt_path}")
        return None

    unreal.EditorAssetLibrary.save_asset(dt_path)
    unreal.log(f"Created {dt_path}")
    return created


def _fill_from_csv(dt_asset, csv_rel_path: str, dt_path: str) -> bool:
    csv_path = get_content_path(csv_rel_path)
    if not os.path.isfile(csv_path):
        unreal.log_error("CSV not found: " + csv_path)
        return False

    success = unreal.DataTableFunctionLibrary.fill_data_table_from_csv_file(dt_asset, csv_path)
    if not success:
        unreal.log_error(f"Failed to fill {dt_path} from {csv_rel_path}")
        return False

    unreal.EditorAssetLibrary.save_asset(dt_path)
    row_names = unreal.DataTableFunctionLibrary.get_data_table_row_names(dt_asset)
    unreal.log(f"Filled {dt_path} from {csv_rel_path} ({len(row_names)} rows)")
    return True


def main():
    unreal.log("=== T66 Leaderboard DataTables Setup ===")

    dt_bounty_path = "/Game/Data/DT_Leaderboard_BountyTargets"
    dt_speed_path = "/Game/Data/DT_Leaderboard_SpeedrunTargets"

    # Clean up common "bad import" assets created by dragging the CSV into Content Browser.
    # These typically end up as DataTables with the wrong row struct and will keep reimporting with errors.
    for bad_path in ["/Game/Data/Leaderboard_BountyTargets", "/Game/Data/Leaderboard_SpeedrunTargets"]:
        if unreal.EditorAssetLibrary.does_asset_exist(bad_path):
            unreal.log_warning(f"Removing bad import asset: {bad_path}")
            unreal.EditorAssetLibrary.delete_asset(bad_path)

    bounty_struct_path = "/Script/T66.T66LeaderboardBountyTargetRow"
    speed_struct_path = "/Script/T66.T66LeaderboardSpeedRunTargetRow"

    dt_bounty = _ensure_datatable("DT_Leaderboard_BountyTargets", dt_bounty_path, bounty_struct_path)
    dt_speed = _ensure_datatable("DT_Leaderboard_SpeedrunTargets", dt_speed_path, speed_struct_path)

    ok = True
    if dt_bounty:
        ok = _fill_from_csv(dt_bounty, "Data/Leaderboard_BountyTargets.csv", dt_bounty_path) and ok
    else:
        ok = False

    if dt_speed:
        ok = _fill_from_csv(dt_speed, "Data/Leaderboard_SpeedrunTargets.csv", dt_speed_path) and ok
    else:
        ok = False

    if ok:
        unreal.log("=== Leaderboard DataTables Setup: SUCCESS ===")
    else:
        unreal.log_error("=== Leaderboard DataTables Setup: FAILED ===")


if __name__ == "__main__":
    main()

