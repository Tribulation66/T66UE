import os
import sys
import unreal


def main() -> int:
    project_dir = unreal.Paths.project_dir()
    csv_path = os.path.join(project_dir, "Content", "Data", "Heroes.csv")
    asset_path = "/Game/Data/DT_Heroes.DT_Heroes"

    if not os.path.exists(csv_path):
        unreal.log_error(f"Hero CSV not found: {csv_path}")
        return 1

    data_table = unreal.EditorAssetLibrary.load_asset(asset_path)
    if not data_table:
        unreal.log_error(f"Failed to load DataTable asset: {asset_path}")
        return 1

    if not unreal.DataTableFunctionLibrary.fill_data_table_from_csv_file(data_table, csv_path):
        unreal.log_error(f"Failed to import hero CSV into {asset_path}")
        return 1

    if not unreal.EditorAssetLibrary.save_loaded_asset(data_table):
        unreal.log_error(f"Failed to save DataTable asset: {asset_path}")
        return 1

    unreal.log(f"Imported hero CSV into {asset_path} from {csv_path}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
