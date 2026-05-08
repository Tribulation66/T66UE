"""
Verify imported auto-attack projectile meshes, materials, textures, and hero CSV
references.

Run with:
  UnrealEditor-Cmd.exe T66.uproject -run=pythonscript -script=Scripts/VerifyWeaponProjectileMeshesAndExit.py
"""

import csv
import json
import os

import unreal


DEST_DIR = "/Game/Weapons/Projectiles"
HEROES_CSV_RELATIVE = ("Content", "Data", "Heroes.csv")
EXPECTED_PARENT = "/Game/Materials/M_GLB_Unlit.M_GLB_Unlit"
OUTPUT_REPORT = os.path.join(
    unreal.SystemLibrary.get_project_directory(),
    "Saved",
    "Audits",
    "WeaponProjectileMeshVerify.json",
)

PROJECTILES = (
    ("Hero_1", "RoyalChad_Sword"),
    ("Hero_2", "ChineseChad_Guandao"),
    ("Hero_3", "BoxerChad_Glove"),
    ("Hero_4", "FoundingChad_Rapier"),
    ("Hero_5", "RoboChad_GearBlade"),
    ("Hero_6", "BillyChad_Bullet"),
    ("Hero_7", "RabbitChad_Carrot"),
    ("Hero_8", "CSChad_TacticalKnife"),
    ("Hero_9", "GoblinoChad_Cleaver"),
    ("Hero_10", "MonotoneChad_InkShard"),
    ("Hero_11", "BaldChad_Hatchet"),
    ("Hero_12", "RoachChad_RustyCrown"),
)


def _asset_name(projectile_id):
    return f"SM_{projectile_id}"


def _mesh_path(projectile_id):
    name = _asset_name(projectile_id)
    return f"{DEST_DIR}/{name}"


def _object_path(projectile_id):
    name = _asset_name(projectile_id)
    return f"{DEST_DIR}/{name}.{name}"


def _material_path(projectile_id):
    name = _asset_name(projectile_id)
    return f"{DEST_DIR}/Materials/MI_{name}"


def _texture_path(projectile_id):
    name = _asset_name(projectile_id)
    return f"{DEST_DIR}/Textures/{name}_BaseColor_00"


def _path_name(asset):
    return asset.get_path_name() if asset else ""


def _load(path):
    return unreal.EditorAssetLibrary.load_asset(path)


def _texture_param(material, name):
    try:
        value = unreal.MaterialEditingLibrary.get_material_instance_texture_parameter_value(material, name)
    except Exception:
        return None
    if isinstance(value, (tuple, list)):
        return value[-1] if value else None
    return value


def _bounds_size(mesh):
    bounds = mesh.get_bounds()
    return [
        float(bounds.box_extent.x) * 2.0,
        float(bounds.box_extent.y) * 2.0,
        float(bounds.box_extent.z) * 2.0,
    ]


def _read_hero_rows():
    csv_path = os.path.join(unreal.SystemLibrary.get_project_directory(), *HEROES_CSV_RELATIVE)
    with open(csv_path, "r", encoding="utf-8-sig", newline="") as handle:
        return {row["HeroID"]: row for row in csv.DictReader(handle)}


def main():
    errors = []
    report = {"assets": {}, "hero_rows": {}}
    hero_rows = _read_hero_rows()

    for hero_id, projectile_id in PROJECTILES:
        mesh_path = _mesh_path(projectile_id)
        material_path = _material_path(projectile_id)
        texture_path = _texture_path(projectile_id)
        expected_texture_object_path = f"{texture_path}.{texture_path.rsplit('/', 1)[-1]}"
        expected_mesh_object_path = _object_path(projectile_id)

        mesh = _load(mesh_path)
        material = _load(material_path)
        texture = _load(texture_path)
        hero_row = hero_rows.get(hero_id)

        asset_report = {
            "mesh": _path_name(mesh),
            "material": _path_name(material),
            "texture": _path_name(texture),
            "bounds_cm": None,
            "material_parent": "",
            "base_color_texture": "",
            "diffuse_texture": "",
            "hero_csv_mesh": hero_row.get("AutoAttackProjectileMesh", "") if hero_row else "",
        }

        if not mesh or not isinstance(mesh, unreal.StaticMesh):
            errors.append(f"{projectile_id}: missing StaticMesh {mesh_path}")
        else:
            asset_report["bounds_cm"] = _bounds_size(mesh)
            longest = max(asset_report["bounds_cm"])
            if longest < 150.0 or longest > 250.0:
                errors.append(f"{projectile_id}: unexpected longest bounds dimension {longest:.1f}cm")

        if not material or not isinstance(material, unreal.MaterialInstanceConstant):
            errors.append(f"{projectile_id}: missing MaterialInstanceConstant {material_path}")
        else:
            try:
                parent = material.get_editor_property("parent")
            except Exception:
                parent = None
            asset_report["material_parent"] = _path_name(parent)
            if _path_name(parent) != EXPECTED_PARENT:
                errors.append(f"{projectile_id}: material parent is {_path_name(parent)}")

            base_color = _texture_param(material, "BaseColorTexture")
            diffuse = _texture_param(material, "DiffuseColorMap")
            asset_report["base_color_texture"] = _path_name(base_color)
            asset_report["diffuse_texture"] = _path_name(diffuse)
            if _path_name(base_color) != expected_texture_object_path:
                errors.append(f"{projectile_id}: BaseColorTexture is {_path_name(base_color)}")
            if _path_name(diffuse) != expected_texture_object_path:
                errors.append(f"{projectile_id}: DiffuseColorMap is {_path_name(diffuse)}")

        if not texture or not isinstance(texture, unreal.Texture2D):
            errors.append(f"{projectile_id}: missing Texture2D {texture_path}")

        if not hero_row:
            errors.append(f"{projectile_id}: missing hero row {hero_id}")
        elif hero_row.get("AutoAttackProjectileMesh") != expected_mesh_object_path:
            errors.append(
                f"{projectile_id}: hero CSV mesh is {hero_row.get('AutoAttackProjectileMesh')}, "
                f"expected {expected_mesh_object_path}"
            )

        report["assets"][projectile_id] = asset_report
        report["hero_rows"][hero_id] = asset_report["hero_csv_mesh"]

    report["errors"] = errors
    os.makedirs(os.path.dirname(OUTPUT_REPORT), exist_ok=True)
    with open(OUTPUT_REPORT, "w", encoding="utf-8") as handle:
        json.dump(report, handle, indent=2)

    if errors:
        raise RuntimeError("; ".join(errors))

    unreal.log("[VerifyWeaponProjectiles] OK")
    try:
        unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")
    except Exception:
        pass


if __name__ == "__main__":
    main()
