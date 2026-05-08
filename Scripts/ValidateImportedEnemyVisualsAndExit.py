import csv
import json
import os

import unreal


CHARACTER_VISUALS_DT = "/Game/Data/DT_CharacterVisuals"
ENEMIES_DT = "/Game/Data/DT_Enemies"
CHARACTER_VISUALS_CSV_RELATIVE = os.path.join("Content", "Data", "CharacterVisuals.csv")
ENEMIES_CSV_RELATIVE = os.path.join("Content", "Data", "Enemies.csv")
REPORT_RELATIVE = os.path.join("Saved", "EnemyQuadRetroUnrealValidationReport.json")


def _project_dir():
    return unreal.SystemLibrary.get_project_directory().replace("\\", "/").rstrip("/")


def _read_csv(path):
    with open(path, "r", encoding="utf-8-sig", newline="") as handle:
        return list(csv.DictReader(handle))


def _load_asset(path):
    asset = unreal.EditorAssetLibrary.load_asset(path)
    if not asset and "." in path:
        asset = unreal.EditorAssetLibrary.load_asset(path.split(".", 1)[0])
    return asset


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


def main():
    project_dir = _project_dir()
    enemies_path = os.path.join(project_dir, ENEMIES_CSV_RELATIVE)
    visuals_path = os.path.join(project_dir, CHARACTER_VISUALS_CSV_RELATIVE)
    report_path = os.path.join(project_dir, REPORT_RELATIVE)

    enemies = _read_csv(enemies_path)
    visual_rows = _read_csv(visuals_path)
    enemy_ids = [row["EnemyID"] for row in enemies]
    visuals_by_id = {row["---"]: row for row in visual_rows}

    visuals_dt = unreal.EditorAssetLibrary.load_asset(CHARACTER_VISUALS_DT)
    enemies_dt = unreal.EditorAssetLibrary.load_asset(ENEMIES_DT)
    if not visuals_dt:
        raise RuntimeError(f"Missing DataTable: {CHARACTER_VISUALS_DT}")
    if not enemies_dt:
        raise RuntimeError(f"Missing DataTable: {ENEMIES_DT}")

    visual_dt_row_names = {str(name) for name in unreal.DataTableFunctionLibrary.get_data_table_row_names(visuals_dt)}
    enemy_dt_row_names = {str(name) for name in unreal.DataTableFunctionLibrary.get_data_table_row_names(enemies_dt)}
    failures = []
    checked = []

    for enemy in enemies:
        enemy_id = enemy["EnemyID"]
        row = visuals_by_id.get(enemy_id)
        if enemy_id not in enemy_dt_row_names:
            failures.append({"enemy_id": enemy_id, "error": "missing DT_Enemies row"})
        if enemy.get("ModelStatus") != "MeshReady":
            failures.append({"enemy_id": enemy_id, "error": f"ModelStatus is {enemy.get('ModelStatus', '')!r}, expected MeshReady"})
        if not row:
            failures.append({"enemy_id": enemy_id, "error": "missing CharacterVisuals.csv row"})
            continue

        static_mesh_path = row.get("StaticMesh", "")
        if enemy_id not in visual_dt_row_names:
            failures.append({"enemy_id": enemy_id, "error": "missing DT_CharacterVisuals row"})
            continue
        if not static_mesh_path:
            failures.append({"enemy_id": enemy_id, "error": "empty StaticMesh path"})
            continue

        asset = _load_asset(static_mesh_path)
        if not asset or not isinstance(asset, unreal.StaticMesh):
            failures.append({"enemy_id": enemy_id, "error": f"failed to load StaticMesh {static_mesh_path}"})
            continue

        material_textures = _get_mesh_material_texture_paths(asset)
        if not material_textures:
            failures.append({"enemy_id": enemy_id, "error": "StaticMesh has no material texture bindings"})
        for texture_info in material_textures:
            texture_path = texture_info["base_color_texture"]
            if not texture_path:
                failures.append({"enemy_id": enemy_id, "error": f"material slot {texture_info['slot']} has no BaseColorTexture"})
            elif "T_White_srgb" in texture_path or "WhiteSquareTexture" in texture_path:
                failures.append({"enemy_id": enemy_id, "error": f"material slot {texture_info['slot']} still uses fallback white texture: {texture_path}"})
            elif "Pixelated_512" not in texture_path:
                failures.append({"enemy_id": enemy_id, "error": f"material slot {texture_info['slot']} uses unexpected texture: {texture_path}"})

        bounds = asset.get_bounds()
        checked.append({
            "enemy_id": enemy_id,
            "static_mesh": static_mesh_path,
            "asset_name": asset.get_name(),
            "bounds_extent": [float(bounds.box_extent.x), float(bounds.box_extent.y), float(bounds.box_extent.z)],
            "mesh_relative_scale": row.get("MeshRelativeScale", ""),
            "material_textures": material_textures,
        })

    payload = {
        "success": len(failures) == 0,
        "expected_count": 25,
        "checked_count": len(checked),
        "failures": failures,
        "character_visuals_data_table": CHARACTER_VISUALS_DT,
        "enemies_data_table": ENEMIES_DT,
        "checked": checked,
    }
    os.makedirs(os.path.dirname(report_path), exist_ok=True)
    with open(report_path, "w", encoding="utf-8") as handle:
        json.dump(payload, handle, indent=2)

    if failures:
        raise RuntimeError(f"Enemy visual Unreal validation failed for {len(failures)} row issue(s); see {report_path}")

    unreal.log(f"[ValidateImportedEnemyVisuals] Validated {len(checked)} enemy StaticMesh rows")
    unreal.log(f"[ValidateImportedEnemyVisuals] Wrote {report_path}")

    world = None
    try:
        subsystem = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
        if subsystem:
            world = subsystem.get_editor_world()
    except Exception:
        pass
    unreal.SystemLibrary.execute_console_command(world, "QUIT_EDITOR")


if __name__ == "__main__":
    main()
