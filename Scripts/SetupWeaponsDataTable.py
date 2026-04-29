"""
Generate Content/Data/Weapons.csv from Heroes.csv, then create/reload DT_Weapons.

Run from the editor or command line:
  UnrealEditor-Cmd.exe T66.uproject -run=pythonscript -script="C:/UE/T66/Scripts/SetupWeaponsDataTable.py"
"""

import csv
import os

import unreal


DT_PATH = "/Game/Data/DT_Weapons"

RARITY_TUNING = {
    "Black": {
        "damage": 1.20,
        "speed": 1.03,
        "scale": 1.05,
        "range": 1.03,
        "hit": 3,
        "pierce": 2,
        "bounce": 2,
        "aoe_radius": 120.0,
        "dot_duration": 1.0,
        "dot_tick": 1.15,
        "falloff": 0.85,
    },
    "Red": {
        "damage": 1.35,
        "speed": 1.06,
        "scale": 1.10,
        "range": 1.06,
        "hit": 6,
        "pierce": 3,
        "bounce": 3,
        "aoe_radius": 180.0,
        "dot_duration": 1.5,
        "dot_tick": 1.30,
        "falloff": 0.80,
    },
    "Yellow": {
        "damage": 1.55,
        "speed": 1.10,
        "scale": 1.15,
        "range": 1.10,
        "hit": 10,
        "pierce": 4,
        "bounce": 4,
        "aoe_radius": 260.0,
        "dot_duration": 2.25,
        "dot_tick": 1.50,
        "falloff": 0.75,
    },
    "White": {
        "damage": 1.80,
        "speed": 1.15,
        "scale": 1.25,
        "range": 1.15,
        "hit": 16,
        "pierce": 6,
        "bounce": 6,
        "aoe_radius": 360.0,
        "dot_duration": 3.0,
        "dot_tick": 1.75,
        "falloff": 0.70,
    },
}

BRANCHES = ("Pierce", "Bounce", "AOE", "DOT")

CSV_FIELDS = (
    "---",
    "WeaponID",
    "HeroID",
    "DisplayName",
    "Description",
    "Icon",
    "Rarity",
    "Branch",
    "DamageMultiplier",
    "AttackSpeedMultiplier",
    "AttackScaleMultiplier",
    "RangeMultiplier",
    "BonusHitDamage",
    "BonusPierceCount",
    "BonusBounceCount",
    "BonusAoeCount",
    "BonusDotSources",
    "BonusAoeRadius",
    "BonusDotDuration",
    "BonusDotTickDamageMultiplier",
    "FalloffPerHitMultiplier",
)


def _float(value, default=0.0):
    try:
        return float(value)
    except (TypeError, ValueError):
        return default


def _int(value, default=0):
    try:
        return int(float(value))
    except (TypeError, ValueError):
        return default


def _starter_row(hero):
    hero_id = hero["HeroID"]
    branch = hero.get("PrimaryCategory") or "Pierce"
    base_hit = _int(hero.get("BaseHitDamage"), 20)
    weapon_id = f"{hero_id}_Grey_Base"
    return {
        "---": weapon_id,
        "WeaponID": weapon_id,
        "HeroID": hero_id,
        "DisplayName": f"{hero.get('DisplayName', hero_id)}'s Starter Weapon",
        "Description": f"Core {branch} auto-attack loadout. Grey starter tier.",
        "Icon": f"/Game/Weapons/Sprites/Grey/{weapon_id}.{weapon_id}",
        "Rarity": "Grey",
        "Branch": branch,
        "DamageMultiplier": "1.10",
        "AttackSpeedMultiplier": "1.00",
        "AttackScaleMultiplier": "1.00",
        "RangeMultiplier": "1.00",
        "BonusHitDamage": str(max(2, round(base_hit * 0.10))),
        "BonusPierceCount": "0",
        "BonusBounceCount": "0",
        "BonusAoeCount": "0",
        "BonusDotSources": "0",
        "BonusAoeRadius": "0.0",
        "BonusDotDuration": "0.0",
        "BonusDotTickDamageMultiplier": "1.00",
        "FalloffPerHitMultiplier": "1.00",
    }


def _branch_description(branch, rarity):
    if branch == "Pierce":
        return f"{rarity} branch: heavier damage falloff control and more pierce targets."
    if branch == "Bounce":
        return f"{rarity} branch: stronger chaining and more bounce targets."
    if branch == "AOE":
        return f"{rarity} branch: wider auto-attack splash and area pressure."
    return f"{rarity} branch: longer damaging ticks attached to the auto-attack payload."


def _upgrade_row(hero, rarity, branch, tuning):
    hero_id = hero["HeroID"]
    weapon_id = f"{hero_id}_{rarity}_{branch}"
    return {
        "---": weapon_id,
        "WeaponID": weapon_id,
        "HeroID": hero_id,
        "DisplayName": f"{rarity} {branch} {hero.get('DisplayName', hero_id)} Weapon",
        "Description": _branch_description(branch, rarity),
        "Icon": f"/Game/Weapons/Sprites/{rarity}/{weapon_id}.{weapon_id}",
        "Rarity": rarity,
        "Branch": branch,
        "DamageMultiplier": f"{tuning['damage']:.2f}",
        "AttackSpeedMultiplier": f"{tuning['speed']:.2f}",
        "AttackScaleMultiplier": f"{tuning['scale']:.2f}",
        "RangeMultiplier": f"{tuning['range']:.2f}",
        "BonusHitDamage": str(tuning["hit"]),
        "BonusPierceCount": str(tuning["pierce"] if branch == "Pierce" else 0),
        "BonusBounceCount": str(tuning["bounce"] if branch == "Bounce" else 0),
        "BonusAoeCount": "1" if branch == "AOE" else "0",
        "BonusDotSources": "1" if branch == "DOT" else "0",
        "BonusAoeRadius": f"{tuning['aoe_radius'] if branch == 'AOE' else 0.0:.1f}",
        "BonusDotDuration": f"{tuning['dot_duration'] if branch == 'DOT' else 0.0:.2f}",
        "BonusDotTickDamageMultiplier": f"{tuning['dot_tick'] if branch == 'DOT' else 1.0:.2f}",
        "FalloffPerHitMultiplier": f"{tuning['falloff'] if branch in ('Pierce', 'Bounce') else 1.0:.2f}",
    }


def generate_weapons_csv(project_dir):
    heroes_path = os.path.join(project_dir, "Content", "Data", "Heroes.csv")
    weapons_path = os.path.join(project_dir, "Content", "Data", "Weapons.csv")

    if not os.path.exists(heroes_path):
        raise RuntimeError(f"Heroes.csv not found at {heroes_path}")

    with open(heroes_path, newline="", encoding="utf-8-sig") as source:
        heroes = [row for row in csv.DictReader(source) if row.get("HeroID")]

    rows = []
    for hero in heroes:
        rows.append(_starter_row(hero))
        for rarity, tuning in RARITY_TUNING.items():
            for branch in BRANCHES:
                rows.append(_upgrade_row(hero, rarity, branch, tuning))

    with open(weapons_path, "w", newline="", encoding="utf-8") as output:
        writer = csv.DictWriter(output, fieldnames=CSV_FIELDS, quoting=csv.QUOTE_ALL)
        writer.writeheader()
        writer.writerows(rows)

    return weapons_path, len(rows)


def resolve_row_struct():
    for name in ("WeaponData", "T66WeaponData", "FWeaponData"):
        struct_type = getattr(unreal, name, None)
        if struct_type is None:
            continue
        if hasattr(struct_type, "static_struct"):
            return struct_type.static_struct()
        return struct_type

    unreal.log_error("Could not resolve FWeaponData in Python.")
    return None


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
        return None

    unreal.log(f"Created DataTable asset at {DT_PATH}")
    return data_table


def main():
    unreal.log("=== SetupWeaponsDataTable ===")

    project_dir = unreal.SystemLibrary.get_project_directory().replace("\\", "/").rstrip("/")
    csv_path, row_count = generate_weapons_csv(project_dir)
    unreal.log(f"Generated {row_count} weapon rows at {csv_path}")

    row_struct = resolve_row_struct()
    if row_struct is None:
        return

    data_table = load_or_create_datatable(row_struct)
    if not data_table:
        return

    success = unreal.DataTableFunctionLibrary.fill_data_table_from_csv_file(data_table, csv_path)
    if not success:
        unreal.log_error("Failed to fill DT_Weapons from CSV.")
        return

    if not unreal.EditorAssetLibrary.save_asset(DT_PATH):
        unreal.log_error(f"Failed to save {DT_PATH}")
        return

    unreal.log(f"DT_Weapons reloaded from {csv_path}")
    unreal.log("=== SetupWeaponsDataTable DONE ===")


if __name__ == "__main__":
    main()
