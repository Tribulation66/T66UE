"""
Validates that DT_CharacterVisuals rows reference existing assets.

Run from Unreal Editor: Py Execute Script -> Scripts/ValidateCharacterVisuals.py
"""

import unreal
import os
import csv


def _get_content_path(relative_path: str) -> str:
    try:
        proj = unreal.SystemLibrary.get_project_directory()
        return os.path.normpath(os.path.join(proj, "Content", relative_path))
    except Exception:
        return os.path.normpath(os.path.join("Content", relative_path))


def main():
    unreal.log("=== ValidateCharacterVisuals ===")

    csv_path = _get_content_path("Data/CharacterVisuals.csv")
    if not os.path.isfile(csv_path):
        unreal.log_error("CSV not found: " + csv_path)
        return

    missing = 0
    checked = 0

    with open(csv_path, "r", newline="", encoding="utf-8") as f:
        reader = csv.reader(f)
        header = next(reader, None)
        if not header:
            unreal.log_error("CSV is empty: " + csv_path)
            return

        # Expect: ---,SkeletalMesh,LoopingAnimation,...
        idx_mesh = header.index("SkeletalMesh") if "SkeletalMesh" in header else -1
        idx_anim = header.index("LoopingAnimation") if "LoopingAnimation" in header else -1
        idx_row = 0

        if idx_mesh < 0:
            unreal.log_error("CSV missing SkeletalMesh column")
            return

        for row in reader:
            if not row or len(row) <= idx_mesh:
                continue
            row_name = row[idx_row].strip()
            mesh_path = row[idx_mesh].strip().strip('"')
            anim_path = ""
            if idx_anim >= 0 and len(row) > idx_anim:
                anim_path = row[idx_anim].strip().strip('"')

            if mesh_path:
                checked += 1
                if not unreal.EditorAssetLibrary.does_asset_exist(mesh_path):
                    unreal.log_error(f"[{row_name}] Missing SkeletalMesh asset: {mesh_path}")
                    missing += 1

            if anim_path:
                checked += 1
                if not unreal.EditorAssetLibrary.does_asset_exist(anim_path):
                    unreal.log_error(f"[{row_name}] Missing Animation asset: {anim_path}")
                    missing += 1

    unreal.log(f"Checked references: {checked}, missing: {missing}")
    unreal.log("=== ValidateCharacterVisuals Done ===")
    try:
        unreal.SystemLibrary.quit_editor()
    except Exception:
        pass


if __name__ == "__main__":
    main()

