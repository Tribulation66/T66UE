"""
Validate that the live boss roster has Quad Retro visual rows and imported Unreal assets.

Run:
  python Scripts/ValidateBossQuadRetroVisuals.py
"""

from __future__ import annotations

import csv
import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
DATA = ROOT / "Content" / "Data"
MANIFEST = ROOT / "Model Generation" / "Runs" / "EnemyBosses" / "EnemyBossBatch01" / "Reports" / "Stage02_Bosses_QuadRetroManifest.json"


def read_csv(path: Path) -> list[dict[str, str]]:
    with path.open(newline="", encoding="utf-8-sig") as handle:
        return list(csv.DictReader(handle))


def fail(message: str) -> None:
    raise SystemExit(f"ERROR: {message}")


def assert_true(condition: bool, message: str) -> None:
    if not condition:
        fail(message)


def package_to_uasset(object_path: str) -> Path:
    package = object_path.split(".", 1)[0]
    assert_true(package.startswith("/Game/"), f"expected /Game object path, got {object_path}")
    return ROOT / "Content" / (package.removeprefix("/Game/") + ".uasset")


def main() -> None:
    bosses = read_csv(DATA / "Bosses.csv")
    boss_ids = {row["BossID"] for row in bosses}
    assert_true(len(boss_ids) == 23, f"expected 23 boss IDs, got {len(boss_ids)}")

    manifest = json.loads(MANIFEST.read_text(encoding="utf-8-sig"))
    rows = list(manifest.get("rows") or [])
    assert_true(len(rows) == 23, f"expected 23 Stage 2 manifest rows, got {len(rows)}")
    assert_true({row["row_id"] for row in rows} == boss_ids, "Stage 2 manifest boss IDs do not match Bosses.csv")

    for row in rows:
        boss_id = row["row_id"]
        for key in ("quad_retro_glb", "quad_retro_report", "qa_front_render"):
            path = Path(row[key])
            assert_true(path.exists() and path.stat().st_size > 0, f"{boss_id} missing {key}: {path}")

    visual_rows = read_csv(DATA / "CharacterVisuals.csv")
    visuals = {row["---"]: row for row in visual_rows}
    missing_visual_rows = sorted(boss_ids - set(visuals))
    assert_true(not missing_visual_rows, f"missing CharacterVisuals boss rows: {missing_visual_rows}")

    for boss_id in sorted(boss_ids):
        row = visuals[boss_id]
        static_mesh = row.get("StaticMesh", "")
        assert_true(static_mesh, f"{boss_id} CharacterVisuals row has no StaticMesh")
        assert_true(not row.get("SkeletalMesh", ""), f"{boss_id} should use StaticMesh, not SkeletalMesh")
        assert_true(row.get("bAutoGroundToActorOrigin", "").lower() == "true", f"{boss_id} should auto-ground static mesh")
        uasset = package_to_uasset(static_mesh)
        assert_true(uasset.exists() and uasset.stat().st_size > 0, f"{boss_id} imported asset missing: {uasset}")

    print("Boss Quad Retro visual validation passed: 23 boss visual rows and imported StaticMesh assets.")


if __name__ == "__main__":
    main()
