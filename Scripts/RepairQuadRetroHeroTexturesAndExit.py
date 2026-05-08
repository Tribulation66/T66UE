"""
Repair Quad Retro hero material instances by binding their imported pixelated
textures explicitly.

The first GLB import pass can inherit Unreal's T_White_srgb default from the
Interchange parent material. This pass keeps the imported meshes/materials and
sets each Quad Retro material instance to its sibling *_Pixelated_512 texture.
"""

import json
from pathlib import Path

import unreal


PROJECT_ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
REPORT_PATH = PROJECT_ROOT / "Saved" / "QuadRetroHeroTextureRepairReport.json"

HERO_MAPPINGS = (
    ("Hero_1", "Royal"),
    ("Hero_2", "Chinese"),
    ("Hero_3", "Boxer"),
    ("Hero_4", "Founding"),
    ("Hero_5", "Robo"),
    ("Hero_6", "Billy"),
    ("Hero_7", "Rabbit"),
    ("Hero_8", "CS"),
    ("Hero_9", "Gamba"),
    ("Hero_10", "Monotone"),
    ("Hero_11", "Bald"),
    ("Hero_12", "Roach"),
)

BODY_SUFFIXES = ("Chad", "Stacy")
TEXTURE_PARAMS = ("BaseColorTexture", "DiffuseColorMap")


def _dest_dir(hero_id, body_suffix):
    return f"/Game/Characters/Heroes/{hero_id}/{body_suffix}/QuadRetro"


def _mesh_path(hero_id, body_suffix):
    name = f"SM_{hero_id}_{body_suffix}_QuadRetro"
    return f"{_dest_dir(hero_id, body_suffix)}/{name}.{name}"


def _import_subdir(hero_id, label):
    return f"{_dest_dir(hero_id, label[-5:] if label.endswith('Stacy') else 'Chad')}/{label}_QuadRetro"


def _list_assets(root):
    if not unreal.EditorAssetLibrary.does_directory_exist(root):
        return []
    return list(unreal.EditorAssetLibrary.list_assets(root, recursive=True, include_folder=False) or [])


def _load_texture(texture_root):
    textures = []
    for path in _list_assets(texture_root):
        asset = unreal.EditorAssetLibrary.load_asset(path)
        if asset and isinstance(asset, unreal.Texture2D):
            textures.append(asset)

    if not textures:
        return None

    textures.sort(
        key=lambda tex: (
            0 if "Pixelated_512" in tex.get_name() else 1,
            tex.get_name(),
        )
    )
    return textures[0]


def _load_material_instances(material_root):
    materials = []
    for path in _list_assets(material_root):
        asset = unreal.EditorAssetLibrary.load_asset(path)
        if not asset or not isinstance(asset, unreal.MaterialInterface):
            continue
        try:
            asset.get_editor_property("parent")
        except Exception:
            continue
        materials.append(asset)
    materials.sort(key=lambda mat: mat.get_name())
    return materials


def _set_material_texture(material, texture):
    changed = False
    for param_name in TEXTURE_PARAMS:
        try:
            unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(
                material,
                param_name,
                texture,
            )
            changed = True
        except Exception:
            pass

    try:
        unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(
            material,
            "Brightness",
            1.0,
        )
    except Exception:
        pass

    try:
        unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(
            material,
            "Tint",
            unreal.LinearColor(1.0, 1.0, 1.0, 1.0),
        )
    except Exception:
        pass

    if changed:
        unreal.EditorAssetLibrary.save_loaded_asset(material)
    return changed


def _ensure_mesh_material(mesh_path, materials):
    mesh = unreal.EditorAssetLibrary.load_asset(mesh_path)
    if not mesh or not isinstance(mesh, unreal.StaticMesh) or not materials:
        return False

    slots = list(mesh.get_editor_property("static_materials") or [])
    if not slots:
        return False

    mesh.modify()
    changed = False
    for index, slot in enumerate(slots):
        material = materials[min(index, len(materials) - 1)]
        try:
            if slot.get_editor_property("material_interface") != material:
                slot.set_editor_property("material_interface", material)
                changed = True
        except Exception:
            try:
                mesh.set_material(index, material)
                changed = True
            except Exception:
                pass

    if changed:
        try:
            mesh.set_editor_property("static_materials", slots)
        except Exception:
            pass
        try:
            mesh.post_edit_change()
        except Exception:
            pass
        unreal.EditorAssetLibrary.save_loaded_asset(mesh)
    return True


def _repair_one(hero_id, label, body_suffix):
    import_root = f"{_dest_dir(hero_id, body_suffix)}/{label}_QuadRetro"
    texture = _load_texture(f"{import_root}/Textures")
    materials = _load_material_instances(f"{import_root}/Materials")
    mesh_path = _mesh_path(hero_id, body_suffix)

    errors = []
    if not texture:
        errors.append(f"missing texture under {import_root}/Textures")
    if not materials:
        errors.append(f"missing material instance under {import_root}/Materials")
    if not unreal.EditorAssetLibrary.does_asset_exist(mesh_path.split(".", 1)[0]):
        errors.append(f"missing mesh {mesh_path}")

    material_results = []
    if texture and materials:
        for material in materials:
            applied = _set_material_texture(material, texture)
            material_results.append(
                {
                    "material": material.get_path_name(),
                    "texture": texture.get_path_name(),
                    "applied": applied,
                }
            )
        _ensure_mesh_material(mesh_path, materials)

    return {
        "label": f"{hero_id}_{body_suffix}",
        "source_label": label,
        "mesh": mesh_path,
        "texture": texture.get_path_name() if texture else "",
        "materials": material_results,
        "errors": errors,
    }


def main():
    unreal.log("[QuadRetroHeroTextureRepair] Starting")
    repaired = []
    failures = []

    for hero_id, prefix in HERO_MAPPINGS:
        for body_suffix in BODY_SUFFIXES:
            label = f"{prefix}{body_suffix}"
            row = _repair_one(hero_id, label, body_suffix)
            repaired.append(row)
            if row["errors"] or not all(item["applied"] for item in row["materials"]):
                failures.append(row)
                unreal.log_error(f"[QuadRetroHeroTextureRepair] Failed {label}: {row['errors']}")
            else:
                unreal.log(
                    f"[QuadRetroHeroTextureRepair] Bound {label} -> {row['texture']}")

    report = {
        "success": not failures,
        "count": len(repaired),
        "failure_count": len(failures),
        "repaired": repaired,
    }
    REPORT_PATH.write_text(json.dumps(report, indent=2), encoding="utf-8")
    unreal.log(f"[QuadRetroHeroTextureRepair] Wrote {REPORT_PATH}")
    unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")


if __name__ == "__main__":
    main()
