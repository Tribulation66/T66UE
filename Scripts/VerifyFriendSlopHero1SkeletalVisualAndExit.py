"""Verify Hero_1_Chad resolves to the FriendSlop skeletal visual and animations."""

from __future__ import annotations

import csv
import io
import json
from pathlib import Path

import unreal


DT_PATH = "/Game/Data/DT_CharacterVisuals"
ROW_NAME = "Hero_1_Chad"
EXPECTED = {
    "SkeletalMesh": "/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/Skeletal/SK_Hero_1_Chad_Male_FriendSlop.SK_Hero_1_Chad_Male_FriendSlop",
    "WalkAnimation": "/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/Skeletal/AM_Hero_1_Chad_Male_FriendSlop_Walk.AM_Hero_1_Chad_Male_FriendSlop_Walk",
    "IdleAnimation": "/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/Skeletal/AM_Hero_1_Chad_Male_FriendSlop_Idle.AM_Hero_1_Chad_Male_FriendSlop_Idle",
    "JumpAnimation": "/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/Skeletal/AM_Hero_1_Chad_Male_FriendSlop_Jump.AM_Hero_1_Chad_Male_FriendSlop_Jump",
    "RollAnimation": "/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/Skeletal/AM_Hero_1_Chad_Male_FriendSlop_Roll.AM_Hero_1_Chad_Male_FriendSlop_Roll",
}
REPORT_PATH = (
    Path(unreal.SystemLibrary.get_project_directory())
    / "Reports"
    / "AgentReviews"
    / "FriendSlopUnrealRagdollImport"
    / "friendslop_hero1_skeletal_visual_verify.json"
)


def normalize_exported_path(value: str) -> str:
    return "" if value in {"", "None", "null"} else value


def main() -> int:
    errors: list[str] = []
    dt = unreal.EditorAssetLibrary.load_asset(DT_PATH)
    if not dt:
        errors.append(f"missing DataTable: {DT_PATH}")
        row = None
    else:
        row_names = {str(name) for name in unreal.DataTableFunctionLibrary.get_data_table_row_names(dt)}
        row = ROW_NAME in row_names
        if not row:
            errors.append(f"missing row: {ROW_NAME}")

    actual: dict[str, str] = {}
    if row:
        exported_csv = unreal.DataTableFunctionLibrary.export_data_table_to_csv_string(dt)
        exported_rows = {
            parsed.get("---", ""): parsed
            for parsed in csv.DictReader(io.StringIO(exported_csv))
        }
        exported_row = exported_rows.get(ROW_NAME)
        if not exported_row:
            errors.append(f"exported DataTable CSV is missing row: {ROW_NAME}")
            exported_row = {}

        for field_name, expected_path in EXPECTED.items():
            actual_path = normalize_exported_path(exported_row.get(field_name, ""))
            actual[field_name] = actual_path
            if actual_path != expected_path:
                errors.append(f"{field_name}: expected {expected_path}, got {actual_path}")

        actual["StaticMesh"] = normalize_exported_path(exported_row.get("StaticMesh", ""))
        if actual["StaticMesh"]:
            errors.append(f"StaticMesh should be empty for {ROW_NAME}, got {actual['StaticMesh']}")

    loaded_assets: dict[str, bool] = {}
    for field_name, asset_path in EXPECTED.items():
        loaded_assets[field_name] = bool(unreal.EditorAssetLibrary.load_asset(asset_path))
        if not loaded_assets[field_name]:
            errors.append(f"{field_name} asset is not loadable: {asset_path}")

    REPORT_PATH.parent.mkdir(parents=True, exist_ok=True)
    REPORT_PATH.write_text(
        json.dumps(
            {
                "ok": not errors,
                "errors": errors,
                "dt_path": DT_PATH,
                "row": ROW_NAME,
                "actual": actual,
                "loaded_assets": loaded_assets,
            },
            indent=2,
        ),
        encoding="utf-8",
    )

    if errors:
        for error in errors:
            unreal.log_error(f"[VerifyFriendSlopHero1SkeletalVisual] {error}")
        raise RuntimeError(f"VerifyFriendSlopHero1SkeletalVisual failed; report={REPORT_PATH}")

    unreal.log(f"[VerifyFriendSlopHero1SkeletalVisual] OK; report={REPORT_PATH}")
    return 0


exit_code = main()
try:
    unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")
except Exception:
    pass
if exit_code:
    raise RuntimeError(f"VerifyFriendSlopHero1SkeletalVisual failed with exit code {exit_code}")
