"""
Import arcade WAV source assets and reload DT_AudioEvents from Content/Data/AudioEvents.json.

Run from the editor or command line:
  UnrealEditor-Cmd.exe T66.uproject -run=pythonscript -script="C:/UE/T66/Scripts/SetupArcadeAudioAssets.py"
"""

import os
from pathlib import Path

import unreal


DESTINATION_PATH = "/Game/Audio/Arcade"
DT_PATH = "/Game/Data/DT_AudioEvents"
JSON_RELATIVE_PATH = Path("Content") / "Data" / "AudioEvents.json"
SOURCE_RELATIVE_PATH = Path("SourceAssets") / "Audio" / "Arcade"


def resolve_row_struct():
    struct_type = getattr(unreal, "T66AudioEventRow", None)
    if struct_type is None:
        unreal.log_error("Could not resolve T66AudioEventRow in Python. Compile the C++ project first.")
        return None
    if hasattr(struct_type, "static_struct"):
        return struct_type.static_struct()
    return struct_type


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
    return data_table


def import_wav_assets(source_dir):
    if not source_dir.exists():
        unreal.log_error(f"Arcade audio source directory missing: {source_dir}")
        return False

    unreal.EditorAssetLibrary.make_directory(DESTINATION_PATH)
    tasks = []
    for wav_path in sorted(source_dir.glob("*.wav")):
        task = unreal.AssetImportTask()
        task.set_editor_property("filename", str(wav_path))
        task.set_editor_property("destination_path", DESTINATION_PATH)
        task.set_editor_property("destination_name", wav_path.stem)
        task.set_editor_property("automated", True)
        task.set_editor_property("replace_existing", True)
        try:
            task.set_editor_property("replace_existing_settings", True)
        except Exception:
            pass
        task.set_editor_property("save", True)
        tasks.append(task)

    if not tasks:
        unreal.log_warning(f"No arcade WAV assets found in {source_dir}")
        return False

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)
    unreal.log(f"Imported/reimported {len(tasks)} arcade WAV assets into {DESTINATION_PATH}")
    return True


def reload_audio_datatable(project_dir):
    row_struct = resolve_row_struct()
    if row_struct is None:
        return False

    json_path = project_dir / JSON_RELATIVE_PATH
    if not json_path.exists():
        unreal.log_error(f"AudioEvents.json missing: {json_path}")
        return False

    data_table = load_or_create_datatable(row_struct)
    if not data_table:
        return False

    success = unreal.DataTableFunctionLibrary.fill_data_table_from_json_file(data_table, str(json_path), row_struct)
    if not success:
        unreal.log_error("Failed to fill DT_AudioEvents from JSON.")
        return False

    if not unreal.EditorAssetLibrary.save_asset(DT_PATH):
        unreal.log_error(f"Failed to save {DT_PATH}")
        return False

    unreal.log(f"DT_AudioEvents reloaded from {json_path}")
    return True


def main():
    unreal.log("=== SetupArcadeAudioAssets ===")
    project_dir = Path(unreal.SystemLibrary.get_project_directory().replace("\\", "/")).resolve()
    source_dir = project_dir / SOURCE_RELATIVE_PATH
    import_wav_assets(source_dir)
    reload_audio_datatable(project_dir)
    unreal.log("=== SetupArcadeAudioAssets DONE ===")


if __name__ == "__main__":
    main()
