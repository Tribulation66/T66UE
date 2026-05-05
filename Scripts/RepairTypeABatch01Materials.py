"""
Repair Chad Batch01 material instances after FBX import.

Targets only the active Chad hero folders and re-applies imported Image_*
textures to the shared unlit character material parameter.
"""

import json
import os
import sys

import unreal


SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
if SCRIPT_DIR not in sys.path:
    sys.path.append(SCRIPT_DIR)

import MakeCharacterMaterialsUnlit


ACTIVE_HERO_IDS = (1, 3, 4, 5, 7, 8, 9, 11, 12, 13, 14, 15)
OUTPUT_REPORT = os.path.join(
    unreal.SystemLibrary.get_project_directory(),
    "Saved",
    "ChadBatch01MaterialRepairReport.json",
)


def _scan_roots():
    roots = []
    for hero_id in ACTIVE_HERO_IDS:
        roots.append(f"/Game/Characters/Heroes/Hero_{hero_id}/Chad")
        roots.append(f"/Game/Characters/Heroes/Hero_{hero_id}/Chad/Beachgoer")
    return roots


def main():
    roots = _scan_roots()
    unreal.log(f"[RepairTypeABatch01Materials] Repairing {len(roots)} roots")
    results = MakeCharacterMaterialsUnlit.convert_character_materials_unlit(roots)
    report = {
        "roots": roots,
        "results": results,
    }
    os.makedirs(os.path.dirname(OUTPUT_REPORT), exist_ok=True)
    with open(OUTPUT_REPORT, "w", encoding="utf-8") as handle:
        json.dump(report, handle, indent=2)
    unreal.log(f"[RepairTypeABatch01Materials] Wrote {OUTPUT_REPORT}")
    if results.get("errors", 0) or results.get("no_texture", 0):
        raise RuntimeError(f"Chad material repair incomplete: {results}")


if __name__ == "__main__":
    main()
