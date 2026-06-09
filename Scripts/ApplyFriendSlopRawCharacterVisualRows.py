"""
Apply manifest-driven CharacterVisuals rows for raw FriendSlop Pixal3D meshes.

This keeps raw FriendSlop runtime wiring explicit instead of relying on legacy
hero or mob import defaults. Assets opt in through manifest fields:

  character_visual_rows: ["Hero_1_Chad", "Hero_1_Chad_DemoSkin"]
  character_visual_yaw: 90.0

The mesh is not processed or reoriented here. CharacterVisuals is the existing
runtime adapter that maps raw static mesh local forward into gameplay forward.
"""

from __future__ import annotations

import csv
import json
from pathlib import Path
from typing import Iterable


REPO_ROOT = Path(__file__).resolve().parents[1]
CHARACTER_VISUALS_CSV = REPO_ROOT / "Content" / "Data" / "CharacterVisuals.csv"
DEFAULT_LOCATION = "(X=0,Y=0,Z=0)"
DEFAULT_SCALE = "(X=1,Y=1,Z=1)"


def _load_manifest(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8-sig"))


def _object_ref(target_dir: str, asset_id: str) -> str:
    name = f"SM_{asset_id}"
    return f"{target_dir.rstrip('/')}/{name}.{name}"


def _format_rotation(yaw: float) -> str:
    return f"(Pitch=0,Yaw={yaw:.6f},Roll=0)"


def _read_rows(path: Path) -> tuple[list[str], list[dict[str, str]]]:
    with path.open("r", encoding="utf-8-sig", newline="") as handle:
        reader = csv.DictReader(handle)
        if not reader.fieldnames:
            raise RuntimeError(f"CharacterVisuals.csv has no header: {path}")
        return list(reader.fieldnames), [dict(row) for row in reader]


def _write_rows(path: Path, fieldnames: list[str], rows: list[dict[str, str]]) -> None:
    with path.open("w", encoding="utf-8", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=fieldnames, lineterminator="\n")
        writer.writeheader()
        for row in rows:
            writer.writerow({field: row.get(field, "") for field in fieldnames})


def _row_targets(asset: dict) -> list[str]:
    values = asset.get("character_visual_rows", [])
    if isinstance(values, str):
        values = [values]
    return [str(value).strip() for value in values if str(value).strip()]


def apply_manifest(manifest_path: Path, csv_path: Path = CHARACTER_VISUALS_CSV) -> list[dict[str, str]]:
    manifest = _load_manifest(manifest_path)
    fieldnames, rows = _read_rows(csv_path)
    by_id = {str(row.get("---", "")).strip(): row for row in rows if row.get("---")}
    applied: list[dict[str, str]] = []

    for asset in manifest.get("assets", []):
        asset_id = str(asset.get("asset_id", "")).strip()
        target_dir = str(asset.get("target_dir", "")).strip()
        visual_rows = _row_targets(asset)
        if not asset_id or not target_dir or not visual_rows:
            continue

        yaw = float(asset.get("character_visual_yaw", 90.0))
        static_mesh = _object_ref(target_dir, asset_id)
        for visual_id in visual_rows:
            row = by_id.get(visual_id)
            if row is None:
                row = {field: "" for field in fieldnames}
                row["---"] = visual_id
                rows.append(row)
                by_id[visual_id] = row

            for field in fieldnames:
                row.setdefault(field, "")

            row["SkeletalMesh"] = ""
            row["StaticMesh"] = static_mesh
            row["OutlineStaticMesh"] = ""
            row["PixelatedTextureAssetPath"] = ""
            row["WalkAnimation"] = ""
            row["IdleAnimation"] = ""
            row["JumpAnimation"] = ""
            row["RollAnimation"] = ""
            row["MeshRelativeLocation"] = str(asset.get("character_visual_location", DEFAULT_LOCATION))
            row["MeshRelativeRotation"] = _format_rotation(yaw)
            row["MeshRelativeScale"] = str(asset.get("character_visual_scale", DEFAULT_SCALE))
            row["bLoopAnimation"] = "false"
            row["bAutoGroundToActorOrigin"] = "true"
            applied.append({"row": visual_id, "static_mesh": static_mesh, "yaw": f"{yaw:.6f}"})

    if applied:
        _write_rows(csv_path, fieldnames, rows)
    return applied


def apply_manifests(manifest_paths: Iterable[Path], csv_path: Path = CHARACTER_VISUALS_CSV) -> list[dict[str, str]]:
    applied: list[dict[str, str]] = []
    for manifest_path in manifest_paths:
        applied.extend(apply_manifest(Path(manifest_path), csv_path))
    return applied


def main() -> None:
    import argparse

    parser = argparse.ArgumentParser(description="Apply raw FriendSlop CharacterVisuals rows from manifest metadata.")
    parser.add_argument("manifests", nargs="+", type=Path)
    parser.add_argument("--csv", type=Path, default=CHARACTER_VISUALS_CSV)
    args = parser.parse_args()

    applied = apply_manifests(args.manifests, args.csv)
    print(json.dumps({"applied_count": len(applied), "applied": applied}, indent=2, sort_keys=True))


if __name__ == "__main__":
    main()
