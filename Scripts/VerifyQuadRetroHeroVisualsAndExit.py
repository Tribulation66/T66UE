"""
Verify Quad Retro static hero visuals are wired into DT_CharacterVisuals.

Run with UnrealEditor.exe:
  UnrealEditor.exe T66.uproject -ExecutePythonScript="C:/UE/T66/Scripts/VerifyQuadRetroHeroVisualsAndExit.py" -unattended -nop4 -nosplash
"""

import csv
import json
from pathlib import Path

import unreal


PROJECT_ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
CSV_PATH = PROJECT_ROOT / "Content" / "Data" / "CharacterVisuals.csv"
REPORT_PATH = PROJECT_ROOT / "Saved" / "QuadRetroHeroVisualVerification.json"
DT_PATH = "/Game/Data/DT_CharacterVisuals"

EXPECTED_ROWS = [f"Hero_{hero}_{body}" for hero in range(1, 13) for body in ("Chad", "Stacy")]


def _read_rows():
    with CSV_PATH.open("r", encoding="utf-8-sig", newline="") as handle:
        return {row.get("---", "").strip(): row for row in csv.DictReader(handle)}


def _load_asset_class(object_path):
    asset = unreal.EditorAssetLibrary.load_asset(object_path)
    return asset.get_class().get_name() if asset else ""


def _get_mesh_material_texture_paths(static_mesh):
    results = []
    if not static_mesh or not isinstance(static_mesh, unreal.StaticMesh):
        return results

    slots = list(static_mesh.get_editor_property("static_materials") or [])
    for index, slot in enumerate(slots):
        try:
            material = slot.get_editor_property("material_interface")
        except Exception:
            material = None
        if not material:
            results.append({"slot": index, "material": "", "base_color_texture": ""})
            continue

        texture_path = ""
        for param_name in ("BaseColorTexture", "DiffuseColorMap"):
            try:
                texture = unreal.MaterialEditingLibrary.get_material_instance_texture_parameter_value(
                    material,
                    param_name,
                )
                if isinstance(texture, (tuple, list)):
                    texture = texture[-1] if texture else None
                if texture:
                    texture_path = texture.get_path_name()
                    break
            except Exception:
                pass

        results.append(
            {
                "slot": index,
                "material": material.get_path_name(),
                "base_color_texture": texture_path,
            }
        )
    return results


def _verify():
    errors = []
    rows = _read_rows()

    dt = unreal.EditorAssetLibrary.load_asset(DT_PATH)
    if not dt:
        errors.append(f"Missing DataTable: {DT_PATH}")
        dt_row_names = set()
    else:
        dt_row_names = {str(name) for name in unreal.DataTableFunctionLibrary.get_data_table_row_names(dt)}

    verified = []
    for row_id in EXPECTED_ROWS:
        row = rows.get(row_id)
        if not row:
            errors.append(f"Missing CSV row: {row_id}")
            continue
        if row_id not in dt_row_names:
            errors.append(f"Missing DataTable row: {row_id}")

        skeletal_mesh = (row.get("SkeletalMesh") or "").strip()
        static_mesh = (row.get("StaticMesh") or "").strip()
        if skeletal_mesh:
            errors.append(f"{row_id} still has SkeletalMesh set: {skeletal_mesh}")
        if not static_mesh:
            errors.append(f"{row_id} has no StaticMesh")
            continue

        asset = unreal.EditorAssetLibrary.load_asset(static_mesh)
        asset_class = asset.get_class().get_name() if asset else ""
        if asset_class != "StaticMesh":
            errors.append(f"{row_id} StaticMesh does not load as StaticMesh: {static_mesh} ({asset_class or 'missing'})")

        material_textures = _get_mesh_material_texture_paths(asset)
        if not material_textures:
            errors.append(f"{row_id} StaticMesh has no material texture bindings")
        for texture_info in material_textures:
            texture_path = texture_info["base_color_texture"]
            if not texture_path:
                errors.append(f"{row_id} material slot {texture_info['slot']} has no BaseColorTexture")
            elif "T_White_srgb" in texture_path or "WhiteSquareTexture" in texture_path:
                errors.append(f"{row_id} material slot {texture_info['slot']} still uses fallback white texture: {texture_path}")
            elif "Pixelated_512" not in texture_path:
                errors.append(f"{row_id} material slot {texture_info['slot']} uses unexpected texture: {texture_path}")

        verified.append(
            {
                "row": row_id,
                "static_mesh": static_mesh,
                "class": asset_class,
                "material_textures": material_textures,
                "rotation": row.get("MeshRelativeRotation", ""),
                "scale": row.get("MeshRelativeScale", ""),
                "auto_ground": row.get("bAutoGroundToActorOrigin", ""),
            }
        )

    return {
        "success": not errors,
        "expected_count": len(EXPECTED_ROWS),
        "verified_count": len(verified),
        "errors": errors,
        "verified": verified,
    }


def main():
    unreal.log("[QuadRetroHeroVisualVerification] Starting")
    report = _verify()
    REPORT_PATH.write_text(json.dumps(report, indent=2), encoding="utf-8")
    if report["success"]:
        unreal.log(f"[QuadRetroHeroVisualVerification] Verified {report['verified_count']} Quad Retro hero visuals")
    else:
        for error in report["errors"]:
            unreal.log_error(f"[QuadRetroHeroVisualVerification] {error}")
    unreal.log(f"[QuadRetroHeroVisualVerification] Wrote {REPORT_PATH}")
    unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")


if __name__ == "__main__":
    main()
