"""
Create or reload the enemy, boss, stage, status, and encounter DataTables.

Run from the editor or command line:
  UnrealEditor-Cmd.exe T66.uproject -run=pythonscript -script="C:/UE/T66/Scripts/SetupCombatRosterDataTables.py"
"""

import os

import unreal


TABLES = [
    {
        "dt_path": "/Game/Data/DT_Stages",
        "csv": "Stages.csv",
        "struct_names": ("StageData", "T66StageData"),
    },
    {
        "dt_path": "/Game/Data/DT_Bosses",
        "csv": "Bosses.csv",
        "struct_names": ("BossData", "T66BossData"),
    },
    {
        "dt_path": "/Game/Data/DT_BossAttacks",
        "csv": "BossAttacks.csv",
        "struct_names": ("T66BossAttackOwnershipData", "BossAttackOwnershipData"),
    },
    {
        "dt_path": "/Game/Data/DT_BossAttackDefinitions",
        "csv": "BossAttackDefinitions.csv",
        "struct_names": ("T66BossAttackDefinitionData", "BossAttackDefinitionData"),
    },
    {
        "dt_path": "/Game/Data/DT_BossHazardDefinitions",
        "csv": "BossHazardDefinitions.csv",
        "struct_names": ("T66BossHazardDefinitionData", "BossHazardDefinitionData"),
    },
    {
        "dt_path": "/Game/Data/DT_BossMovementPatterns",
        "csv": "BossMovementPatterns.csv",
        "struct_names": ("T66BossMovementPatternData", "BossMovementPatternData"),
    },
    {
        "dt_path": "/Game/Data/DT_Enemies",
        "csv": "Enemies.csv",
        "struct_names": ("T66EnemyData", "EnemyData"),
    },
    {
        "dt_path": "/Game/Data/DT_StatusEffects",
        "csv": "StatusEffects.csv",
        "struct_names": ("T66StatusEffectData", "StatusEffectData"),
    },
    {
        "dt_path": "/Game/Data/DT_BossEncounters",
        "csv": "BossEncounters.csv",
        "struct_names": ("T66BossEncounterData", "BossEncounterData"),
    },
    {
        "dt_path": "/Game/Data/DT_BossEncounterMembers",
        "csv": "BossEncounterMembers.csv",
        "struct_names": ("T66BossEncounterMemberData", "BossEncounterMemberData"),
    },
]


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


def load_or_create_datatable(dt_path, row_struct):
    if unreal.EditorAssetLibrary.does_asset_exist(dt_path):
        dt = unreal.EditorAssetLibrary.load_asset(dt_path)
        if dt:
            return dt

    factory = unreal.DataTableFactory()
    factory.set_editor_property("struct", row_struct)

    package_path, asset_name = dt_path.rsplit("/", 1)
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    dt = asset_tools.create_asset(asset_name, package_path, unreal.DataTable, factory)
    if not dt:
        unreal.log_error(f"Failed to create DataTable asset at {dt_path}")
        return None

    unreal.log(f"Created DataTable asset at {dt_path}")
    return dt


def reload_table(project_dir, spec):
    row_struct = resolve_row_struct(spec["struct_names"])
    if row_struct is None:
        return False

    csv_path = os.path.normpath(os.path.join(project_dir, "Content", "Data", spec["csv"]))
    if not os.path.exists(csv_path):
        unreal.log_error(f"{spec['csv']} not found at {csv_path}")
        return False

    dt = load_or_create_datatable(spec["dt_path"], row_struct)
    if not dt:
        return False

    success = unreal.DataTableFunctionLibrary.fill_data_table_from_csv_file(dt, csv_path)
    if not success:
        unreal.log_error(f"Failed to fill {spec['dt_path']} from {csv_path}")
        return False

    if not unreal.EditorAssetLibrary.save_asset(spec["dt_path"]):
        unreal.log_error(f"Failed to save {spec['dt_path']}")
        return False

    unreal.log(f"Reloaded {spec['dt_path']} from {csv_path}")
    return True


def main():
    unreal.log("=== SetupCombatRosterDataTables ===")
    project_dir = unreal.SystemLibrary.get_project_directory().replace("\\", "/").rstrip("/")

    ok = True
    for spec in TABLES:
        ok = reload_table(project_dir, spec) and ok

    if ok:
        unreal.log("=== SetupCombatRosterDataTables DONE ===")
    else:
        unreal.log_error("=== SetupCombatRosterDataTables FAILED ===")


if __name__ == "__main__":
    main()
