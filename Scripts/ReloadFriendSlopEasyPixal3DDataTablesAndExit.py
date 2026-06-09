"""
Reload DataTables touched by the FriendSlop Easy Pixal3D batch, then exit.
"""

import os
import sys

import unreal

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
if SCRIPT_DIR not in sys.path:
    sys.path.insert(0, SCRIPT_DIR)

import SetupCharacterVisualsDataTable
import SetupCombatRosterDataTables
import SetupMobVertexAnimationsDataTable
import SetupNPCsDataTable
import SetupPetsDataTable
import SetupVehicleInteractablesDataTable
import SetupWorldVisualPropsDataTable
import ApplyFriendSlopRawCharacterVisualRows


REPO_ROOT = os.path.abspath(os.path.join(SCRIPT_DIR, ".."))
HERO1_FRIENDSLOP_MANIFEST = os.path.join(
    REPO_ROOT,
    "Model Generation",
    "Runs",
    "Pixal3D",
    "FriendSlopProbe_Hero1Male_20260604_1415",
    "FriendSlopProbe_Hero1Male_20260604_1415_manifest.json",
)


def main():
    unreal.log("=== ReloadFriendSlopEasyPixal3DDataTablesAndExit ===")
    applied_rows = ApplyFriendSlopRawCharacterVisualRows.apply_manifests(
        [HERO1_FRIENDSLOP_MANIFEST]
    )
    unreal.log(f"Applied FriendSlop raw CharacterVisual row count: {len(applied_rows)}")
    SetupCharacterVisualsDataTable.main()
    SetupPetsDataTable.main()
    SetupNPCsDataTable.main()
    SetupVehicleInteractablesDataTable.main()
    SetupWorldVisualPropsDataTable.main()
    SetupMobVertexAnimationsDataTable.main()
    SetupCombatRosterDataTables.main()
    unreal.log("=== ReloadFriendSlopEasyPixal3DDataTablesAndExit DONE ===")

    try:
        unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")
    except Exception as exc:
        unreal.log_warning(f"Failed to request QUIT_EDITOR: {exc}")


main()
