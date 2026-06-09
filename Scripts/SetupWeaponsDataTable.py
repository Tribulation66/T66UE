"""
Generate Content/Data/Weapons.csv from Heroes.csv, then create/reload DT_Weapons.

Run from the editor or command line:
  UnrealEditor-Cmd.exe T66.uproject -run=pythonscript -script="C:/UE/T66/Scripts/SetupWeaponsDataTable.py"
"""

import csv
import os

try:
    import unreal
except ImportError:
    unreal = None


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

RARITY_ID_SLUGS = {
    "Black": "black",
    "Red": "red",
    "Yellow": "yellow",
    "White": "white",
}

BRANCH_ID_SLUGS = {
    "Pierce": "pierce",
    "Bounce": "bounce",
    "AOE": "aoe",
    "DOT": "dot",
}

HERO1_AXE_AOE_CRESCENT_INNER_RADIUS_RATIO = 0.54

# These temporary Hero 1 AOE placeholders use the black tier additive hit
# bonus for every rarity so the damage multipliers below own the requested
# relative damage ladder: black base, red 120%, yellow 150%, white 200%.
HERO1_AXE_AOE_OVERRIDES = {
    "Black": {
        "damage": 1.20,
        "hit": 3,
        "aoe_radius": 120.0,
    },
    "Red": {
        "damage": 1.44,
        "hit": 3,
        "aoe_radius": 322.5,
    },
    "Yellow": {
        "damage": 1.80,
        "hit": 3,
        "aoe_radius": 416.3,
    },
    "White": {
        "damage": 2.40,
        "hit": 3,
        "aoe_radius": 495.0,
    },
}

DEFAULT_WEAPON_PATTERN = {
    "AttackPatternID": "Default",
    "ProjectileCount": "0",
    "SpreadAngleDegrees": "0.0",
}

HERO_AOE_PATTERNS = {
    ("Hero_1", "Black"): {
        "AttackPatternID": "Hero1CrescentSingle",
        "ProjectileCount": "1",
        "SpreadAngleDegrees": "0.0",
    },
    ("Hero_1", "Red"): {
        "AttackPatternID": "Hero1CrescentTriple",
        "ProjectileCount": "3",
        "SpreadAngleDegrees": "0.0",
    },
    ("Hero_1", "Yellow"): {
        "AttackPatternID": "Hero1CrescentFive",
        "ProjectileCount": "5",
        "SpreadAngleDegrees": "0.0",
    },
    ("Hero_1", "White"): {
        "AttackPatternID": "Hero1CrescentFullContact",
        "ProjectileCount": "1",
        "SpreadAngleDegrees": "0.0",
    },
}

CSV_FIELDS = (
    "---",
    "WeaponID",
    "HeroID",
    "DisplayName",
    "Description",
    "Icon",
    "Rarity",
    "Branch",
    "AttackPatternID",
    "ProjectileCount",
    "SpreadAngleDegrees",
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
    "AoeInnerRadiusRatio",
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


WEAPON_NAMES = {
    "Black": {
        "Pierce": "Iron Edge",
        "Bounce": "Rebound Star",
        "AOE": "Ash Burst",
        "DOT": "Grave Venom",
    },
    "Red": {
        "Pierce": "Demon Edge",
        "Bounce": "Blood Comet",
        "AOE": "Hell Nova",
        "DOT": "Demon Rot",
    },
    "Yellow": {
        "Pierce": "Sunpiercer",
        "Bounce": "Solar Ricochet",
        "AOE": "Radiant Blast",
        "DOT": "Solar Blight",
    },
    "White": {
        "Pierce": "Void Edge",
        "Bounce": "Astral Echo",
        "AOE": "Void Singularity",
        "DOT": "Void Plague",
    },
}

DEMO_HERO_WEAPON_FAMILIES = {
    "Hero_1": {
        "family": "axe",
        "names": {
            "Black": {
                "Pierce": "Iron Hewing Axe",
                "Bounce": "Rebound Hatchet",
                "AOE": "Ashfall Cleaver",
                "DOT": "Gravebite Axe",
            },
            "Red": {
                "Pierce": "Demon Splitter",
                "Bounce": "Blood Orbit Axe",
                "AOE": "Hellburst Labrys",
                "DOT": "Rotfang Cleaver",
            },
            "Yellow": {
                "Pierce": "Sunforged Reaver",
                "Bounce": "Solar Tomahawk",
                "AOE": "Radiant War Axe",
                "DOT": "Blightgold Hatchet",
            },
            "White": {
                "Pierce": "Void Hewing Axe",
                "Bounce": "Astral Throwing Axe",
                "AOE": "Singularity Cleaver",
                "DOT": "Pale Plague Axe",
            },
        },
    },
    "Hero_2": {
        "family": "lance",
        "names": {
            "Black": {
                "Pierce": "Iron Lance",
                "Bounce": "Returning Javelin",
                "AOE": "Ashburst Pike",
                "DOT": "Venom Lance",
            },
            "Red": {
                "Pierce": "Blood Pike",
                "Bounce": "Crimson Throwing Lance",
                "AOE": "Hellfire Halberd",
                "DOT": "Rot Spear",
            },
            "Yellow": {
                "Pierce": "Sunpiercer Lance",
                "Bounce": "Solar Glaive",
                "AOE": "Radiant Halberd",
                "DOT": "Blightgold Javelin",
            },
            "White": {
                "Pierce": "Void Lance",
                "Bounce": "Astral Pike",
                "AOE": "Singularity Spear",
                "DOT": "Pale Plague Lance",
            },
        },
    },
    "Hero_3": {
        "family": "boxing glove",
        "names": {
            "Black": {
                "Pierce": "Iron Knuckle Glove",
                "Bounce": "Rebound Mitt",
                "AOE": "Ashburst Glove",
                "DOT": "Venom Wraps",
            },
            "Red": {
                "Pierce": "Demon Knuckles",
                "Bounce": "Bloodwork Gloves",
                "AOE": "Hellfire Mitts",
                "DOT": "Rotwrap Gloves",
            },
            "Yellow": {
                "Pierce": "Sunstrike Gloves",
                "Bounce": "Solar Rebound Mitts",
                "AOE": "Radiant Knuckles",
                "DOT": "Blightgold Wraps",
            },
            "White": {
                "Pierce": "Void Knuckles",
                "Bounce": "Astral Gloves",
                "AOE": "Singularity Mitts",
                "DOT": "Pale Plague Wraps",
            },
        },
    },
    "Hero_4": {
        "family": "pistol",
        "names": {
            "Black": {
                "Pierce": "Iron Peacemaker",
                "Bounce": "Ricochet Revolver",
                "AOE": "Ashburst Sidearm",
                "DOT": "Venom Derringer",
            },
            "Red": {
                "Pierce": "Bloodline Revolver",
                "Bounce": "Demon Repeater",
                "AOE": "Hellfire Handgun",
                "DOT": "Rotshot Pistol",
            },
            "Yellow": {
                "Pierce": "Sunpiercer Pistol",
                "Bounce": "Solar Ricochet",
                "AOE": "Radiant Hand Cannon",
                "DOT": "Blightgold Revolver",
            },
            "White": {
                "Pierce": "Void Peacemaker",
                "Bounce": "Astral Repeater",
                "AOE": "Singularity Sidearm",
                "DOT": "Pale Plague Pistol",
            },
        },
    },
    "Hero_5": {
        "family": "flask",
        "names": {
            "Black": {
                "Pierce": "Iron Tonic Flask",
                "Bounce": "Rebound Ampoule",
                "AOE": "Ashburst Beaker",
                "DOT": "Venom Vial",
            },
            "Red": {
                "Pierce": "Blood Flask",
                "Bounce": "Demon Phial",
                "AOE": "Hellfire Beaker",
                "DOT": "Rot Vial",
            },
            "Yellow": {
                "Pierce": "Solar Flask",
                "Bounce": "Sunlit Ampoule",
                "AOE": "Radiant Alembic",
                "DOT": "Blightgold Vial",
            },
            "White": {
                "Pierce": "Void Flask",
                "Bounce": "Astral Phial",
                "AOE": "Singularity Beaker",
                "DOT": "Pale Plague Vial",
            },
        },
    },
}

BRANCH_DESCRIPTIONS = {
    "Pierce": "piercing",
    "Bounce": "chaining",
    "AOE": "splash",
    "DOT": "damage-over-time",
}


def _no_weapon_row():
    return {
        "---": "Weapon_NoWeapon",
        "WeaponID": "Weapon_NoWeapon",
        "HeroID": "None",
        "DisplayName": "No Weapon",
        "Description": "No weapon. Fire a white single-target placeholder attack and keep weapon synergies inactive.",
        "Icon": "",
        "Rarity": "Black",
        "Branch": "SingleTarget",
        "AttackPatternID": "NoWeaponSingleTarget",
        "ProjectileCount": "1",
        "SpreadAngleDegrees": "0.0",
        "DamageMultiplier": "1.00",
        "AttackSpeedMultiplier": "1.00",
        "AttackScaleMultiplier": "1.00",
        "RangeMultiplier": "1.00",
        "BonusHitDamage": "0",
        "BonusPierceCount": "0",
        "BonusBounceCount": "0",
        "BonusAoeCount": "0",
        "BonusDotSources": "0",
        "BonusAoeRadius": "0.0",
        "AoeInnerRadiusRatio": "0.00",
        "BonusDotDuration": "0.00",
        "BonusDotTickDamageMultiplier": "1.00",
        "FalloffPerHitMultiplier": "1.00",
    }


def _weapon_id(hero_id, rarity, branch):
    rarity_slug = RARITY_ID_SLUGS.get(rarity, rarity.lower())
    branch_slug = BRANCH_ID_SLUGS.get(branch, branch.lower())
    return f"{hero_id}_{rarity_slug}_{branch_slug}"


def _weapon_family(hero_id):
    family_config = DEMO_HERO_WEAPON_FAMILIES.get(hero_id)
    return family_config.get("family") if family_config else "weapon"


def _weapon_display_name(hero_id, rarity, branch):
    family_config = DEMO_HERO_WEAPON_FAMILIES.get(hero_id)
    if family_config:
        return family_config["names"].get(rarity, {}).get(branch, f"{rarity} {branch} {family_config['family']}")
    return WEAPON_NAMES.get(rarity, {}).get(branch, f"{rarity} {branch} Weapon")


def _branch_description(branch, rarity, hero_id=None):
    family = _weapon_family(hero_id) if hero_id else "weapon"
    branch_kind = BRANCH_DESCRIPTIONS.get(branch, branch.lower())
    if branch == "Pierce":
        return f"{rarity} tier. A {branch_kind} {family} auto-attack that cuts through lined-up targets."
    if branch == "Bounce":
        return f"{rarity} tier. A {branch_kind} {family} auto-attack that jumps between nearby targets."
    if branch == "AOE":
        return f"{rarity} tier. A {branch_kind} {family} auto-attack that detonates around the impact point."
    return f"{rarity} tier. A {branch_kind} {family} auto-attack that leaves a lingering wound."


def _weapon_pattern(hero_id, rarity, branch):
    if branch == "AOE":
        return HERO_AOE_PATTERNS.get((hero_id, rarity), {
            "AttackPatternID": "Single",
            "ProjectileCount": "1",
            "SpreadAngleDegrees": "0.0",
        })
    return DEFAULT_WEAPON_PATTERN


def _upgrade_row(hero, rarity, branch, tuning):
    hero_id = hero["HeroID"]
    weapon_id = _weapon_id(hero_id, rarity, branch)
    pattern = _weapon_pattern(hero_id, rarity, branch)
    projectile_count = _int(pattern.get("ProjectileCount"), 0)
    row_tuning = dict(tuning)
    if hero_id == "Hero_1" and branch == "AOE":
        row_tuning.update(HERO1_AXE_AOE_OVERRIDES.get(rarity, {}))
    return {
        "---": weapon_id,
        "WeaponID": weapon_id,
        "HeroID": hero_id,
        "DisplayName": _weapon_display_name(hero_id, rarity, branch),
        "Description": _branch_description(branch, rarity, hero_id),
        "Icon": f"/Game/Weapons/Sprites/{rarity}/{weapon_id}.{weapon_id}",
        "Rarity": rarity,
        "Branch": branch,
        "AttackPatternID": pattern.get("AttackPatternID", "Default"),
        "ProjectileCount": str(projectile_count),
        "SpreadAngleDegrees": f"{_float(pattern.get('SpreadAngleDegrees'), 0.0):.1f}",
        "DamageMultiplier": f"{row_tuning['damage']:.2f}",
        "AttackSpeedMultiplier": f"{row_tuning['speed']:.2f}",
        "AttackScaleMultiplier": f"{row_tuning['scale']:.2f}",
        "RangeMultiplier": f"{row_tuning['range']:.2f}",
        "BonusHitDamage": str(row_tuning["hit"]),
        "BonusPierceCount": str(row_tuning["pierce"] if branch == "Pierce" else 0),
        "BonusBounceCount": str(row_tuning["bounce"] if branch == "Bounce" else 0),
        "BonusAoeCount": str(projectile_count if branch == "AOE" and projectile_count > 0 else (1 if branch == "AOE" else 0)),
        "BonusDotSources": "1" if branch == "DOT" else "0",
        "BonusAoeRadius": f"{row_tuning['aoe_radius'] if branch == 'AOE' else 0.0:.1f}",
        "AoeInnerRadiusRatio": f"{HERO1_AXE_AOE_CRESCENT_INNER_RADIUS_RATIO if hero_id == 'Hero_1' and branch == 'AOE' else 0.0:.2f}",
        "BonusDotDuration": f"{row_tuning['dot_duration'] if branch == 'DOT' else 0.0:.2f}",
        "BonusDotTickDamageMultiplier": f"{row_tuning['dot_tick'] if branch == 'DOT' else 1.0:.2f}",
        "FalloffPerHitMultiplier": f"{row_tuning['falloff'] if branch in ('Pierce', 'Bounce') else 1.0:.2f}",
    }


def generate_weapons_csv(project_dir):
    heroes_path = os.path.join(project_dir, "Content", "Data", "Heroes.csv")
    weapons_path = os.path.join(project_dir, "Content", "Data", "Weapons.csv")

    if not os.path.exists(heroes_path):
        raise RuntimeError(f"Heroes.csv not found at {heroes_path}")

    with open(heroes_path, newline="", encoding="utf-8-sig") as source:
        heroes = [row for row in csv.DictReader(source) if row.get("HeroID")]

    rows = [_no_weapon_row()]
    for hero in heroes:
        primary_branch = hero.get("PrimaryCategory") or "Pierce"
        if primary_branch not in BRANCHES:
            primary_branch = "Pierce"
        for rarity, tuning in RARITY_TUNING.items():
            rows.append(_upgrade_row(hero, rarity, primary_branch, tuning))

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
    if unreal is None:
        raise RuntimeError("Unreal Python module is required to create or reload DT_Weapons.")

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
    if unreal is None:
        project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), os.pardir))
        csv_path, row_count = generate_weapons_csv(project_root)
        print(f"Generated {row_count} weapon rows at {csv_path}")
    else:
        main()
