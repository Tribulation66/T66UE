"""
Create DT_Props DataTable and fill from CSV.

Run INSIDE Unreal Editor:
  Tools -> Execute Python Script -> Scripts/SetupPropsDataTable.py
"""

import os
import unreal


def get_content_path(relative_path):
    try:
        proj = unreal.SystemLibrary.get_project_directory()
        return os.path.normpath(os.path.join(proj, "Content", relative_path))
    except Exception:
        return os.path.normpath(os.path.join("Content", relative_path))


def main():
    unreal.log("=== T66 Props DataTable Setup ===")

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

    dt_path = "/Game/Data/DT_Props"
    if not unreal.EditorAssetLibrary.does_asset_exist(dt_path):
        row_struct = unreal.find_object(None, "/Script/T66.T66PropRow")
        if not row_struct:
            unreal.log_error("FT66PropRow struct not found. Rebuild the T66 module first.")
            return

        factory = unreal.DataTableFactory()
        factory.set_editor_property("struct", row_struct)
        data_table = asset_tools.create_asset("DT_Props", "/Game/Data", None, factory)
        if data_table:
            unreal.log("Created DT_Props")
            unreal.EditorAssetLibrary.save_asset(dt_path)
        else:
            unreal.log_error("Failed to create DT_Props")
            return
    else:
        unreal.log("DT_Props already exists")

    dt = unreal.EditorAssetLibrary.load_asset(dt_path)
    if not dt:
        unreal.log_error("Could not load DT_Props")
        return

    csv_path = get_content_path("Data/Props.csv")
    if not os.path.isfile(csv_path):
        unreal.log_warning("Props.csv not found, creating default CSV")
        _create_default_csv(csv_path)

    success = unreal.DataTableFunctionLibrary.fill_data_table_from_csv_file(dt, csv_path)
    if success:
        unreal.log("Filled DT_Props from Props.csv")
        unreal.EditorAssetLibrary.save_asset(dt_path)
    else:
        unreal.log_error("Failed to fill DT_Props from CSV")

    unreal.log("=== Props DataTable Setup Done ===")


def _create_default_csv(csv_path):
    os.makedirs(os.path.dirname(csv_path), exist_ok=True)
    lines = [
        '---,Mesh,CountMin,CountMax,ScaleMin,ScaleMax,bRandomYawRotation,MinDistanceBetween',
        'Barn,"/Game/World/Props/Barn.Barn",1,2,"(X=0.9,Y=0.9,Z=0.9)","(X=1.1,Y=1.1,Z=1.1)",true,1200.0',
        'Tree,"/Game/World/Props/Tree.Tree",3,6,"(X=0.8,Y=0.8,Z=0.8)","(X=1.2,Y=1.2,Z=1.2)",true,600.0',
        'Tree1,"/Game/World/Props/Tree1.Tree1",3,6,"(X=0.8,Y=0.8,Z=0.8)","(X=1.2,Y=1.2,Z=1.2)",true,600.0',
        'Hay1,"/Game/World/Props/Hay1.Hay1",4,8,"(X=0.7,Y=0.7,Z=0.7)","(X=1.0,Y=1.0,Z=1.0)",true,400.0',
        'Hay2,"/Game/World/Props/Hay2.Hay2",4,8,"(X=0.7,Y=0.7,Z=0.7)","(X=1.0,Y=1.0,Z=1.0)",true,400.0',
    ]
    with open(csv_path, "w", encoding="utf-8") as f:
        f.write("\n".join(lines) + "\n")
    unreal.log(f"Created default Props.csv at {csv_path}")


if __name__ == "__main__":
    main()
