"""
Validate the live enemy and boss roster source data without launching Unreal.

Run:
  python Scripts/ValidateEnemyBossRosterData.py
"""

from __future__ import annotations

import csv
import json
from collections import Counter, defaultdict
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

REQUIRED_FAMILY_COUNTS = Counter({"Melee": 2, "Ranged": 1, "Rush": 1, "Flying": 1})
OLD_PLACEHOLDER_TOKENS = ("Roost", "Cow", "Goat", "Pig", "Stage_21", "Stage_22", "Stage_23", "Boss_21", "Boss_22", "Boss_23")


def read_csv(name: str) -> list[dict[str, str]]:
    with (DATA / name).open(newline="", encoding="utf-8-sig") as handle:
        return list(csv.DictReader(handle))


def fail(message: str) -> None:
    raise SystemExit(f"ERROR: {message}")


def assert_true(condition: bool, message: str) -> None:
    if not condition:
        fail(message)


def validate_player_experience() -> None:
    rows = json.loads((DATA / "PlayerExperience.json").read_text(encoding="utf-8"))
    by_name = {row["Name"]: row for row in rows}
    assert_true(set(by_name) == set(DIFFICULTY_RANGES), "PlayerExperience difficulties do not match expected set")
    for difficulty, (start, end, _theme) in DIFFICULTY_RANGES.items():
        row = by_name[difficulty]
        assert_true(row["StartStage"] == start, f"{difficulty} StartStage expected {start} got {row['StartStage']}")
        assert_true(row["EndStage"] == end, f"{difficulty} EndStage expected {end} got {row['EndStage']}")


def validate_enemies(enemies: list[dict[str, str]]) -> None:
    assert_true(len(enemies) == 25, f"expected 25 enemies got {len(enemies)}")
    by_id = {row["EnemyID"]: row for row in enemies}
    assert_true(len(by_id) == 25, "enemy IDs must be unique")

    for difficulty, (_start, _end, theme) in DIFFICULTY_RANGES.items():
        rows = [row for row in enemies if row["DifficultyID"] == difficulty]
        assert_true(len(rows) == 5, f"{difficulty} expected 5 enemies got {len(rows)}")
        assert_true(all(row["ThemeID"] == theme for row in rows), f"{difficulty} enemy theme mismatch")
        family_counts = Counter(row["FamilyID"] for row in rows)
        assert_true(family_counts == REQUIRED_FAMILY_COUNTS, f"{difficulty} family counts expected {REQUIRED_FAMILY_COUNTS} got {family_counts}")


def validate_statuses(enemies: list[dict[str, str]], statuses: list[dict[str, str]]) -> None:
    status_ids = {row["StatusEffectID"] for row in statuses}
    assert_true("Webbed" in status_ids, "Webbed status is required for spider slow")
    for row in enemies:
        effect = row["StatusEffectOnHit"]
        assert_true(effect in status_ids, f"{row['EnemyID']} references missing status {effect}")


def validate_stages(stages: list[dict[str, str]], enemy_ids: set[str]) -> None:
    assert_true(len(stages) == 20, f"expected 20 stages got {len(stages)}")
    by_stage = {int(row["StageNumber"]): row for row in stages}
    assert_true(set(by_stage) == set(range(1, 21)), "stages must be contiguous 1..20")

    for difficulty, (start, end, theme) in DIFFICULTY_RANGES.items():
        for stage_number in range(start, end + 1):
            row = by_stage[stage_number]
            local_stage = stage_number - start + 1
            assert_true(row["DifficultyID"] == difficulty, f"stage {stage_number} difficulty mismatch")
            assert_true(row["ThemeID"] == theme, f"stage {stage_number} theme mismatch")
            assert_true(int(row["LocalStageNumber"]) == local_stage, f"stage {stage_number} local stage mismatch")
            expected_finale = local_stage == 4
            assert_true(row["bBossOnlyFinale"].lower() == str(expected_finale).lower(), f"stage {stage_number} finale flag mismatch")
            roster = [row[f"Enemy{slot}"] for slot in ("A", "B", "C", "D", "E")]
            assert_true(len(roster) == 5 and all(enemy in enemy_ids for enemy in roster), f"stage {stage_number} has invalid enemy roster")


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
    validate_stages(stages, {row["EnemyID"] for row in enemies})
    validate_bosses(bosses, encounters, members, stages)
    validate_no_live_placeholder_tokens()

    print("Enemy/boss roster validation passed: 20 stages, 25 enemies, 20 encounters, 23 boss rows.")


if __name__ == "__main__":
    main()
