"""
Reload DataTables touched by interactable/NPC cleanup, then exit the editor.
"""

import os
import sys

import unreal

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
if SCRIPT_DIR not in sys.path:
    sys.path.insert(0, SCRIPT_DIR)

import SetupArcadeInteractablesDataTable
import SetupAudioEventsDataTable
import SetupCharacterVisualsDataTable
import SetupHouseNPCsDataTable
import SetupPlayerExperienceDataTable
import SetupPropsDataTable


def main():
    unreal.log("=== ReloadCleanedInteractableDataTablesAndExit ===")
    SetupPlayerExperienceDataTable.main()
    SetupArcadeInteractablesDataTable.main()
    SetupCharacterVisualsDataTable.main()
    SetupHouseNPCsDataTable.main()
    SetupPropsDataTable.main()
    audio_json_path = SetupAudioEventsDataTable.project_root() / SetupAudioEventsDataTable.JSON_RELATIVE_PATH
    SetupAudioEventsDataTable.reload_datatable(audio_json_path)
    unreal.log("=== ReloadCleanedInteractableDataTablesAndExit DONE ===")

    try:
        unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")
    except Exception as exc:
        unreal.log_warning(f"Failed to request QUIT_EDITOR: {exc}")


main()
