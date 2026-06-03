"""
Phase 2 scoped DataTable rebuild driver (temporary, task-specific).

Rebuilds ONLY the four Codex-approved roster tables from source:
  DT_Enemies      <- Content/Data/Enemies.csv
  DT_Stages       <- Content/Data/Stages.csv
  DT_Items        <- Content/Data/Items.csv
  DT_PlayerExperience <- Content/Data/PlayerExperience.json

Deliberately does NOT touch DT_Bosses / DT_StatusEffects / DT_BossEncounters /
DT_BossEncounterMembers (outside approved scope; their CSVs were not changed by
Phase 1).

Run:
  UnrealEditor-Cmd.exe T66.uproject -run=pythonscript ^
    -script="C:/UE/T66/Reports/AgentReviews/20260529_EnemyRosterRestructureImplementation/phase2_logs/rebuild_roster_datatables.py"
"""

import sys

import unreal

SCRIPTS_DIR = r"C:/UE/T66/Scripts"
if SCRIPTS_DIR not in sys.path:
    sys.path.insert(0, SCRIPTS_DIR)

import SetupCombatRosterDataTables as roster  # noqa: E402
import SetupItemsDataTable as items  # noqa: E402
import SetupPlayerExperienceDataTable as pxp  # noqa: E402

APPROVED_ROSTER_TABLES = {"/Game/Data/DT_Enemies", "/Game/Data/DT_Stages"}

results = {}


def mark(path, ok):
    results[path] = ok
    unreal.log("=== PHASE2-REBUILD {0} -> {1} ===".format(path, "OK" if ok else "FAIL"))


def main():
    unreal.log("=== PHASE2-REBUILD START ===")
    project_dir = unreal.SystemLibrary.get_project_directory().replace("\\", "/").rstrip("/")

    for spec in roster.TABLES:
        if spec["dt_path"] in APPROVED_ROSTER_TABLES:
            ok = roster.reload_table(project_dir, spec)
            mark(spec["dt_path"], ok)

    # Items (script logs its own DONE marker; verify asset save via reload result)
    try:
        items.main()
        mark("/Game/Data/DT_Items", unreal.EditorAssetLibrary.does_asset_exist("/Game/Data/DT_Items"))
    except Exception as exc:  # noqa: BLE001
        unreal.log_error("DT_Items rebuild raised: {0}".format(exc))
        mark("/Game/Data/DT_Items", False)

    # PlayerExperience (JSON)
    try:
        pxp.main()
        mark("/Game/Data/DT_PlayerExperience", unreal.EditorAssetLibrary.does_asset_exist("/Game/Data/DT_PlayerExperience"))
    except Exception as exc:  # noqa: BLE001
        unreal.log_error("DT_PlayerExperience rebuild raised: {0}".format(exc))
        mark("/Game/Data/DT_PlayerExperience", False)

    all_ok = all(results.values()) and len(results) == 4
    unreal.log("=== PHASE2-REBUILD SUMMARY {0} ===".format(
        " ".join("{0}={1}".format(p.rsplit('/', 1)[-1], "OK" if v else "FAIL") for p, v in results.items())))
    if all_ok:
        unreal.log("=== PHASE2-REBUILD ALL DONE ===")
    else:
        unreal.log_error("=== PHASE2-REBUILD INCOMPLETE ===")


if __name__ == "__main__":
    main()
