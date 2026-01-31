"""
Generate/patch Content/Data/Stages.csv by adding EnemyA/EnemyB/EnemyC columns
from T66_Stages.md.

- Keeps existing stage fields (boss/effects/colors/etc) as-is.
- Converts enemy display names in the markdown into stable FName IDs:
    Mob_<SanitizedName>
  Example: "Ember Imps" -> "Mob_EmberImps"
  Hyphens and punctuation are stripped.

- If a stage only specifies 2 enemies, EnemyC becomes "Mob_StageXX_Extra".

Run:
  python Scripts/GenerateStagesCsvFromStagesMd.py
"""

from __future__ import annotations

import csv
import os
import re
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
MD_PATH = ROOT / "T66_Stages.md"
CSV_PATH = ROOT / "Content" / "Data" / "Stages.csv"


def sanitize_enemy_to_id(enemy_display_name: str) -> str:
    s = enemy_display_name.strip()
    # Drop anything in parentheses.
    s = re.sub(r"\s*\([^)]*\)\s*", " ", s)
    # Convert separators to spaces.
    s = re.sub(r"[-_/]", " ", s)
    # Keep only alphanumerics/spaces.
    s = re.sub(r"[^0-9A-Za-z ]+", "", s)
    parts = [p for p in s.split() if p]
    if not parts:
        return "Mob_Unknown"
    camel = "".join(p[:1].upper() + p[1:] for p in parts)
    return f"Mob_{camel}"


def parse_stage_enemies_from_md(md_text: str) -> dict[int, list[str]]:
    """
    Returns stage_number -> list of enemy display names (len 1..3).
    """
    stage_to_enemies: dict[int, list[str]] = {}
    current_stage: int | None = None
    in_enemies = False

    for raw in md_text.splitlines():
        line = raw.strip()
        m = re.match(r"^##\s*Stage\s+(\d+)\b", line)
        if m:
            current_stage = int(m.group(1))
            stage_to_enemies[current_stage] = []
            in_enemies = False
            continue

        if current_stage is None:
            continue

        if line.startswith("**Enemies**"):
            in_enemies = True
            continue

        if line.startswith("**") and not line.startswith("**Enemies**"):
            # Leaving enemies section
            in_enemies = False
            continue

        if in_enemies and line.startswith("- "):
            enemy = line[2:].strip()
            if enemy:
                stage_to_enemies[current_stage].append(enemy)

    return stage_to_enemies


def main() -> None:
    if not MD_PATH.is_file():
        raise SystemExit(f"Missing markdown file: {MD_PATH}")
    if not CSV_PATH.is_file():
        raise SystemExit(f"Missing stages csv: {CSV_PATH}")

    md = MD_PATH.read_text(encoding="utf-8")
    stage_to_enemies = parse_stage_enemies_from_md(md)

    # Read existing CSV rows as raw strings and patch enemy columns.
    with CSV_PATH.open("r", encoding="utf-8", newline="") as f:
        reader = csv.DictReader(f)
        rows = list(reader)
        fieldnames = list(reader.fieldnames or [])

    required = ["EnemyA", "EnemyB", "EnemyC"]
    for c in required:
        if c not in fieldnames:
            fieldnames.append(c)

    for row in rows:
        try:
            stage_num = int(row.get("StageNumber", "0"))
        except ValueError:
            stage_num = 0

        enemies = stage_to_enemies.get(stage_num, [])
        ids = [sanitize_enemy_to_id(e) for e in enemies[:3]]
        if len(ids) == 2:
            ids.append(f"Mob_Stage{stage_num:02d}_Extra")
        elif len(ids) == 1:
            ids.append(f"Mob_Stage{stage_num:02d}_ExtraB")
            ids.append(f"Mob_Stage{stage_num:02d}_ExtraC")
        elif len(ids) == 0:
            ids = [f"Mob_Stage{stage_num:02d}_A", f"Mob_Stage{stage_num:02d}_B", f"Mob_Stage{stage_num:02d}_C"]

        row["EnemyA"] = ids[0]
        row["EnemyB"] = ids[1]
        row["EnemyC"] = ids[2]

    tmp_path = CSV_PATH.with_suffix(".csv.tmp")
    with tmp_path.open("w", encoding="utf-8", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=fieldnames, quoting=csv.QUOTE_MINIMAL)
        writer.writeheader()
        for r in rows:
            writer.writerow(r)

    os.replace(tmp_path, CSV_PATH)
    print(f"Updated: {CSV_PATH}")


if __name__ == "__main__":
    main()

