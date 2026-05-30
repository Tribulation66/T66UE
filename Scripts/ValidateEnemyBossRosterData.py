"""
Validate the live enemy, boss, and stage roster source data without launching Unreal.

Run:
  python Scripts/ValidateEnemyBossRosterData.py
"""

from __future__ import annotations

import csv
import json
from collections import defaultdict
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
DATA = ROOT / "Content" / "Data"
CONFIG = ROOT / "Config"

DIFFICULTY_RANGES = {
    "Easy": (1, 4, "Dungeon"),
    "Medium": (5, 8, "Forest"),
    "Hard": (9, 12, "Ocean"),
    "VeryHard": (13, 16, "Martian"),
    "Impossible": (17, 20, "Hell"),
}

THEME_TO_DIFFICULTY = {theme: difficulty for difficulty, (_start, _end, theme) in DIFFICULTY_RANGES.items()}
STAGE_SLOTS = tuple(f"Enemy{letter}" for letter in "ABCDEFGHIJKL")
ALLOWED_ARCHETYPES = {
    "Melee",
    "Ranged",
    "Rush",
    "Flying",
}
ALLOWED_FEELINGS = {"MowDown", "Pressure", "DodgeThreat", "Disruptor", "Specialist"}
ALLOWED_RARITIES = {"Core", "Rare", "Late"}
FAMILY_FALLBACK_BY_ARCHETYPE = {
    "Melee": "Melee",
    "Ranged": "Ranged",
    "Rush": "Rush",
    "Flying": "Flying",
}
EXPECTED_ENEMY_COUNT = 60
EXPECTED_ENEMIES_PER_THEME = 12
EXPECTED_PLACEHOLDER_ENEMY_IDS = {
    "CursedCrow",
    "FamishedGhoul",
    "WillOWisp",
    "GoreStag",
    "GullDiver",
    "Hammerjaw",
    "ReconOrb",
    "CarapaceBrute",
    "CinderWraith",
    "BrimstoneBrute",
}
REQUIRED_SPAWN_BUDGET_KEYS = (
    "MobFloorsPerStage",
    "InitialEnemiesPerMobFloor",
    "TotalInitialEnemiesPerStage",
    "RuntimeEnemiesPerWave",
    "RuntimeMaxAliveEnemies",
    "RuntimeSpawnIntervalSeconds",
)
ESTIMATED_SPAWN_BUDGET_KEYS = (
    "EstimatedRuntimeWavesPerStage",
    "EstimatedRuntimeEnemiesPerStage",
    "EstimatedTotalEnemiesPerStage",
)
OLD_PLACEHOLDER_TOKENS = (
    "Roost",
    "Cow",
    "Goat",
    "Pig",
    "Stage_21",
    "Stage_22",
    "Stage_23",
    "Boss_21",
    "Boss_22",
    "Boss_23",
)


def read_csv(name: str) -> list[dict[str, str]]:
    with (DATA / name).open(newline="", encoding="utf-8-sig") as handle:
        return list(csv.DictReader(handle))


def fail(message: str) -> None:
    raise SystemExit(f"ERROR: {message}")


def assert_true(condition: bool, message: str) -> None:
    if not condition:
        fail(message)


def is_empty_slot(value: str) -> bool:
    return (value or "").strip() in {"", "None", "NAME_None"}


def validate_player_experience() -> None:
    rows = json.loads((DATA / "PlayerExperience.json").read_text(encoding="utf-8"))
    by_name = {row["Name"]: row for row in rows}
    assert_true(set(by_name) == set(DIFFICULTY_RANGES), "PlayerExperience difficulties do not match expected set")
    for difficulty in DIFFICULTY_RANGES:
        row = by_name[difficulty]
        budget = row.get("TowerSpawnBudgetBase")
        assert_true(isinstance(budget, dict), f"{difficulty} TowerSpawnBudgetBase must be an object")

        for key in REQUIRED_SPAWN_BUDGET_KEYS:
            assert_true(key in budget, f"{difficulty} TowerSpawnBudgetBase missing {key}")
            value = budget[key]
            assert_true(isinstance(value, (int, float)), f"{difficulty} {key} must be numeric")
            assert_true(value > 0, f"{difficulty} {key} must be greater than zero")

        for key in ESTIMATED_SPAWN_BUDGET_KEYS:
            if key in budget:
                value = budget[key]
                assert_true(isinstance(value, (int, float)), f"{difficulty} {key} must be numeric")
                assert_true(value >= 0, f"{difficulty} {key} must be non-negative")


def validate_enemies(enemies: list[dict[str, str]]) -> dict[str, list[str]]:
    assert_true(len(enemies) == EXPECTED_ENEMY_COUNT, f"expected {EXPECTED_ENEMY_COUNT} enemies got {len(enemies)}")
    by_id = {row["EnemyID"]: row for row in enemies}
    assert_true(len(by_id) == EXPECTED_ENEMY_COUNT, "enemy IDs must be unique")

    roster_by_theme: dict[str, list[str]] = defaultdict(list)
    for row in enemies:
        enemy_id = row["EnemyID"]
        assert_true(row.get("---") == enemy_id, f"{enemy_id} row name must match EnemyID")

        stage_tag = row.get("StageTag", "")
        difficulty = row.get("DifficultyID", "")
        theme = row.get("ThemeID", "")
        archetype = row.get("Archetype", "")
        feeling = row.get("Feeling", "")
        rarity = row.get("Rarity", "")

        assert_true(stage_tag in THEME_TO_DIFFICULTY, f"{enemy_id} has invalid StageTag {stage_tag}")
        assert_true(difficulty == THEME_TO_DIFFICULTY[stage_tag], f"{enemy_id} difficulty does not match StageTag")
        assert_true(theme == stage_tag, f"{enemy_id} ThemeID must match StageTag")
        assert_true(archetype in ALLOWED_ARCHETYPES, f"{enemy_id} has invalid Archetype {archetype}")
        assert_true(feeling in ALLOWED_FEELINGS, f"{enemy_id} has invalid Feeling {feeling}")
        assert_true(rarity in ALLOWED_RARITIES, f"{enemy_id} has invalid Rarity {rarity}")
        assert_true(row.get("FamilyID") == FAMILY_FALLBACK_BY_ARCHETYPE[archetype], f"{enemy_id} FamilyID fallback mismatch")
        assert_true(row.get("RoleID") == row.get("FamilyID"), f"{enemy_id} RoleID must match fallback FamilyID")
        assert_true(row.get("ModelStatus") in {"MeshReady", "Placeholder"}, f"{enemy_id} ModelStatus must be MeshReady or Placeholder")
        assert_true(row.get("StatusEffectOnHit") == "None", f"{enemy_id} StatusEffectOnHit must be None in this pass")
        assert_true(bool(row.get("PrimaryColor")), f"{enemy_id} PrimaryColor missing")
        assert_true(bool(row.get("SecondaryColor")), f"{enemy_id} SecondaryColor missing")
        roster_by_theme[stage_tag].append(enemy_id)

    for _difficulty, (_start, _end, theme) in DIFFICULTY_RANGES.items():
        assert_true(len(roster_by_theme[theme]) == EXPECTED_ENEMIES_PER_THEME, f"{theme} expected {EXPECTED_ENEMIES_PER_THEME} enemies got {len(roster_by_theme[theme])}")

    placeholder_ids = {row["EnemyID"] for row in enemies if row.get("ModelStatus") == "Placeholder"}
    assert_true(placeholder_ids == EXPECTED_PLACEHOLDER_ENEMY_IDS, f"placeholder enemy set mismatch expected {sorted(EXPECTED_PLACEHOLDER_ENEMY_IDS)} got {sorted(placeholder_ids)}")

    return roster_by_theme


def validate_statuses(enemies: list[dict[str, str]], statuses: list[dict[str, str]]) -> None:
    status_ids = {row["StatusEffectID"] for row in statuses}
    for row in enemies:
        effect = row["StatusEffectOnHit"]
        assert_true(effect in status_ids, f"{row['EnemyID']} references missing status {effect}")


def validate_stages(stages: list[dict[str, str]], enemies: list[dict[str, str]]) -> None:
    assert_true(len(stages) == 20, f"expected 20 stages got {len(stages)}")
    by_stage = {int(row["StageNumber"]): row for row in stages}
    assert_true(set(by_stage) == set(range(1, 21)), "stages must be contiguous 1..20")
    enemy_ids = {row["EnemyID"] for row in enemies}
    enemy_ids_by_theme: dict[str, set[str]] = defaultdict(set)
    for row in enemies:
        enemy_ids_by_theme[row["StageTag"]].add(row["EnemyID"])

    for difficulty, (start, end, theme) in DIFFICULTY_RANGES.items():
        referenced_theme_enemy_ids: set[str] = set()
        for stage_number in range(start, end + 1):
            row = by_stage[stage_number]
            local_stage = stage_number - start + 1
            assert_true(row["DifficultyID"] == difficulty, f"stage {stage_number} difficulty mismatch")
            assert_true(row["ThemeID"] == theme, f"stage {stage_number} theme mismatch")
            assert_true(int(row["LocalStageNumber"]) == local_stage, f"stage {stage_number} local stage mismatch")
            expected_finale = local_stage == 4
            assert_true(row["bBossOnlyFinale"].lower() == str(expected_finale).lower(), f"stage {stage_number} finale flag mismatch")
            assert_true(all(slot in row for slot in STAGE_SLOTS), f"stage {stage_number} must have EnemyA..EnemyL columns")
            roster = [row.get(slot, "") for slot in STAGE_SLOTS]
            assert_true(len(roster) == len(STAGE_SLOTS), f"stage {stage_number} must have {len(STAGE_SLOTS)} enemy slot columns")
            filled = [enemy for enemy in roster if not is_empty_slot(enemy)]
            assert_true(all(enemy in enemy_ids for enemy in filled), f"stage {stage_number} has invalid enemy roster")
            referenced_theme_enemy_ids.update(filled)
        assert_true(referenced_theme_enemy_ids == enemy_ids_by_theme[theme], f"{theme} stages must reference all {EXPECTED_ENEMIES_PER_THEME} theme enemies across the 4-stage set")


def validate_bosses(bosses: list[dict[str, str]], encounters: list[dict[str, str]], members: list[dict[str, str]], stages: list[dict[str, str]]) -> None:
    boss_ids = {row["BossID"] for row in bosses}
    encounter_ids = {row["BossEncounterID"] for row in encounters}
    assert_true(len(encounters) == 20, f"expected 20 boss encounters got {len(encounters)}")
    assert_true(len(bosses) == 23, f"expected 23 boss rows including four horsemen got {len(bosses)}")

    stage_encounters = {row["BossEncounterID"] for row in stages}
    assert_true(stage_encounters == encounter_ids, "stage encounter IDs must match BossEncounters rows")

    members_by_encounter: dict[str, list[dict[str, str]]] = defaultdict(list)
    for row in members:
        assert_true(row["BossID"] in boss_ids, f"encounter member references missing boss {row['BossID']}")
        assert_true(row["BossEncounterID"] in encounter_ids, f"encounter member references missing encounter {row['BossEncounterID']}")
        members_by_encounter[row["BossEncounterID"]].append(row)

    for encounter_id in encounter_ids:
        rows = members_by_encounter[encounter_id]
        expected = 4 if encounter_id == "Encounter_Stage_17" else 1
        assert_true(len(rows) == expected, f"{encounter_id} expected {expected} member rows got {len(rows)}")


def validate_no_live_placeholder_tokens() -> None:
    live_files = [
        DATA / "Stages.csv",
        DATA / "Bosses.csv",
        DATA / "Enemies.csv",
        DATA / "BossEncounters.csv",
        DATA / "BossEncounterMembers.csv",
        DATA / "PlayerExperience.json",
        DATA / "Leaderboard_SpeedrunTargets.csv",
        CONFIG / "DefaultT66PlayerExperience.ini",
        CONFIG / "DefaultT66StageProgression.ini",
    ]
    for path in live_files:
        text = path.read_text(encoding="utf-8")
        for token in OLD_PLACEHOLDER_TOKENS:
            assert_true(token not in text, f"old placeholder token {token} remains in {path.relative_to(ROOT)}")


def main() -> None:
    enemies = read_csv("Enemies.csv")
    statuses = read_csv("StatusEffects.csv")
    stages = read_csv("Stages.csv")
    bosses = read_csv("Bosses.csv")
    encounters = read_csv("BossEncounters.csv")
    members = read_csv("BossEncounterMembers.csv")

    validate_player_experience()
    validate_enemies(enemies)
    validate_statuses(enemies, statuses)
    validate_stages(stages, enemies)
    validate_bosses(bosses, encounters, members, stages)
    validate_no_live_placeholder_tokens()

    print("Enemy/boss roster validation passed: 20 stages, 60 enemies, 20 encounters, 23 boss rows.")


if __name__ == "__main__":
    main()
