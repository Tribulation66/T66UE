"""
Reload DataTable assets that are affected by active hero roster changes.

Run from the command line:
  UnrealEditor-Cmd.exe T66.uproject -run=pythonscript -script="C:/UE/T66/Scripts/ReloadActiveHeroRosterDataTables.py" -unattended -nop4 -nosplash
"""

import os
import sys


SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
if SCRIPT_DIR not in sys.path:
    sys.path.insert(0, SCRIPT_DIR)

import ImportHeroDataTable
import SetupAudioEventsDataTable
import SetupCharacterVisualsDataTable
import SetupWeaponsDataTable


def reload_audio_events_table_only():
    root = SetupAudioEventsDataTable.project_root()
    json_path = root / SetupAudioEventsDataTable.JSON_RELATIVE_PATH

    SetupAudioEventsDataTable.reload_datatable(json_path)


def main():
    ImportHeroDataTable.main()
    SetupWeaponsDataTable.main()
    SetupCharacterVisualsDataTable.main()
    reload_audio_events_table_only()


if __name__ == "__main__":
    main()
