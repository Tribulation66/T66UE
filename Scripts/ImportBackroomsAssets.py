"""
Import Backrooms generated textures and reload the Backrooms data tables.

Run with:
  UnrealEditor-Cmd.exe T66.uproject -run=pythonscript -script="C:/UE/T66/Scripts/ImportBackroomsAssets.py"
"""

import os

import unreal


SOURCE_RELATIVE = os.path.join("SourceAssets", "Backrooms", "ImageGen", "Backrooms_20260526")

WORLD_TEXTURES = (
    ("T_Backrooms_Wall.png", "/Game/World/Backrooms/Textures", "T_Backrooms_Wall"),
    ("T_Backrooms_Floor.png", "/Game/World/Backrooms/Textures", "T_Backrooms_Floor"),
    ("T_Backrooms_Door.png", "/Game/World/Backrooms/Textures", "T_Backrooms_Door"),
)

UI_TEXTURES = (
    ("Item_BackroomsQuickRevive.png", "/Game/Items/Sprites", "Item_BackroomsQuickRevive"),
)

DATA_TABLES = (
    {
        "dt_path": "/Game/Data/DT_Items",
        "csv": "Items.csv",
        "struct_names": ("ItemData", "T66ItemData"),
        "create_if_missing": False,
    },
    {
        "dt_path": "/Game/Data/DT_UniqueEnemies",
        "csv": "UniqueEnemies.csv",
        "struct_names": ("UniqueEnemyData", "T66UniqueEnemyData"),
        "create_if_missing": True,
    },
)


def get_project_dir():
    return unreal.SystemLibrary.get_project_directory().replace("\\", "/").rstrip("/")


def ensure_directory(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def import_texture(source_path, dest_dir, dest_name, is_ui):
    if not os.path.exists(source_path):
        raise RuntimeError(f"Backrooms texture source not found: {source_path}")

    ensure_directory(dest_dir)

    task = unreal.AssetImportTask()
    task.automated = True
    task.save = False
    task.replace_existing = True
    task.replace_existing_settings = True
    task.filename = source_path
    task.destination_path = dest_dir
    task.destination_name = dest_name

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

    asset_path = f"{dest_dir}/{dest_name}"
    asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    if not asset or not isinstance(asset, unreal.Texture2D):
        raise RuntimeError(f"Texture import failed: {asset_path}")

    try:
        asset.set_editor_property(
            "compression_settings",
            unreal.TextureCompressionSettings.TC_EDITOR_ICON if is_ui else unreal.TextureCompressionSettings.TC_DEFAULT,
        )
    except Exception:
        pass
    try:
        asset.set_editor_property(
            "lod_group",
            unreal.TextureGroup.TEXTUREGROUP_UI if is_ui else unreal.TextureGroup.TEXTUREGROUP_WORLD,
        )
    except Exception:
        pass
    try:
        asset.set_editor_property("mip_gen_settings", unreal.TextureMipGenSettings.TMGS_FROM_TEXTURE_GROUP)
    except Exception:
        pass
    try:
        asset.set_editor_property("filter", unreal.TextureFilter.TF_TRILINEAR)
    except Exception:
        pass
    if is_ui:
        try:
            asset.set_editor_property("never_stream", True)
        except Exception:
            pass
    asset.set_editor_property("srgb", True)
    unreal.EditorAssetLibrary.save_asset(asset_path)
    unreal.log(f"[ImportBackroomsAssets] Imported {source_path} -> {asset_path}")


def resolve_row_struct(struct_names):
    for struct_name in struct_names:
        struct_type = getattr(unreal, struct_name, None)
        if struct_type is None:
            continue
        if hasattr(struct_type, "static_struct"):
            return struct_type.static_struct()
        return struct_type
    unreal.log_error(f"Could not resolve any row struct from {struct_names}")
    return None


def load_or_create_datatable(dt_path, row_struct, create_if_missing):
    if unreal.EditorAssetLibrary.does_asset_exist(dt_path):
        data_table = unreal.EditorAssetLibrary.load_asset(dt_path)
        if data_table:
            return data_table

    if not create_if_missing:
        unreal.log_error(f"DataTable not found at {dt_path}")
        return None

    factory = unreal.DataTableFactory()
    factory.set_editor_property("struct", row_struct)

    package_path, asset_name = dt_path.rsplit("/", 1)
    data_table = unreal.AssetToolsHelpers.get_asset_tools().create_asset(asset_name, package_path, unreal.DataTable, factory)
    if not data_table:
        unreal.log_error(f"Failed to create DataTable asset at {dt_path}")
        return None

    unreal.log(f"[ImportBackroomsAssets] Created DataTable {dt_path}")
    return data_table


def reload_data_table(project_dir, spec):
    row_struct = resolve_row_struct(spec["struct_names"])
    if row_struct is None:
        return False

    csv_path = os.path.normpath(os.path.join(project_dir, "Content", "Data", spec["csv"]))
    if not os.path.exists(csv_path):
        unreal.log_error(f"{spec['csv']} not found at {csv_path}")
        return False

    data_table = load_or_create_datatable(spec["dt_path"], row_struct, spec["create_if_missing"])
    if not data_table:
        return False

    if not unreal.DataTableFunctionLibrary.fill_data_table_from_csv_file(data_table, csv_path):
        unreal.log_error(f"Failed to fill {spec['dt_path']} from {csv_path}")
        return False

    if not unreal.EditorAssetLibrary.save_asset(spec["dt_path"]):
        unreal.log_error(f"Failed to save {spec['dt_path']}")
        return False

    unreal.log(f"[ImportBackroomsAssets] Reloaded {spec['dt_path']} from {csv_path}")
    return True


def main():
    unreal.log("=== ImportBackroomsAssets ===")
    project_dir = get_project_dir()
    source_dir = os.path.normpath(os.path.join(project_dir, SOURCE_RELATIVE))

    if not os.path.isdir(source_dir):
        raise RuntimeError(f"Backrooms source directory not found: {source_dir}")

    for file_name, dest_dir, dest_name in WORLD_TEXTURES:
        import_texture(os.path.join(source_dir, file_name), dest_dir, dest_name, is_ui=False)

    for file_name, dest_dir, dest_name in UI_TEXTURES:
        import_texture(os.path.join(source_dir, file_name), dest_dir, dest_name, is_ui=True)

    ok = True
    for spec in DATA_TABLES:
        ok = reload_data_table(project_dir, spec) and ok

    if not ok:
        raise RuntimeError("One or more Backrooms data tables failed to reload.")

    unreal.log("=== ImportBackroomsAssets DONE ===")


if __name__ == "__main__":
    main()
