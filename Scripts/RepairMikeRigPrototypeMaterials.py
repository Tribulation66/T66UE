"""
Repair the Mike rig prototype materials after FBX import.

The FBX import preserves material slots but does not reliably import the packed
Blender texture images. This script imports the textures extracted from the
Blender rig source, creates unlit material instances, and assigns them to the
prototype skeletal mesh slots.
"""

import json
import os
from pathlib import Path

import unreal


PROJECT_DIR = Path(unreal.SystemLibrary.get_project_directory())
DEST = "/Game/Characters/Heroes/Hero_3/Chad/RigPrototype"
MESH_PATH = f"{DEST}/SK_Hero_3_Mike_Chad_RigPrototype"
CHAR_MASTER_PATH = "/Game/Materials/M_Character_Unlit"
COLOR_MASTER_NAME = "M_MikeRigPrototype_ColorUnlit"
REPORT_PATH = PROJECT_DIR / "Saved" / "MikeRigPrototypeMaterialRepairReport.json"
TEXTURE_DIR = (
    PROJECT_DIR
    / "Model Generation"
    / "Runs"
    / "Heroes"
    / "Chad_Pass02_ProcessBuild"
    / "Rigging"
    / "Mike_Chad_RigPrototype_A03_LiftedNeckBridge"
    / "UnrealMaterialRepair"
    / "ExtractedTextures"
)

TEXTURES = {
    "body": {
        "source": TEXTURE_DIR / "Image_0.png",
        "asset_name": "T_MikeRigPrototype_Body_BaseColor",
        "material_name": "MI_MikeRigPrototype_Body",
    },
    "head": {
        "source": TEXTURE_DIR / "Image_0.001.png",
        "asset_name": "T_MikeRigPrototype_Head_BaseColor",
        "material_name": "MI_MikeRigPrototype_Head",
    },
    "sword": {
        "source": TEXTURE_DIR / "Image_0.002.png",
        "asset_name": "T_MikeRigPrototype_Sword_BaseColor",
        "material_name": "MI_MikeRigPrototype_Sword",
    },
}

SLOT_MATERIALS = {
    "Material_0": "body",
    "Material_0_001": "head",
    "MikeRepair_SkinMaterial": "neck",
    "Material_0_002": "sword",
}


def _asset_path(asset_name):
    return f"{DEST}/{asset_name}"


def _load(path):
    return unreal.EditorAssetLibrary.load_asset(path)


def _safe_set(obj, prop, value):
    try:
        obj.set_editor_property(prop, value)
        return True
    except Exception as exc:
        unreal.log_warning(f"[RepairMikeRigPrototypeMaterials] Could not set {prop}: {exc}")
        return False


def _ensure_directory():
    if not unreal.EditorAssetLibrary.does_directory_exist(DEST):
        unreal.EditorAssetLibrary.make_directory(DEST)


def _import_texture(key, spec):
    source = spec["source"]
    if not source.is_file():
        raise RuntimeError(f"Missing extracted texture for {key}: {source}")

    task = unreal.AssetImportTask()
    task.automated = True
    task.save = True
    task.replace_existing = True
    task.replace_existing_settings = False
    task.filename = str(source)
    task.destination_path = DEST
    task.destination_name = spec["asset_name"]
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

    texture = _load(_asset_path(spec["asset_name"]))
    if not texture or not isinstance(texture, unreal.Texture2D):
        raise RuntimeError(f"Texture import failed for {key}: {source}")
    _safe_set(texture, "srgb", True)
    try:
        texture.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_Character)
    except Exception:
        pass
    unreal.EditorAssetLibrary.save_asset(_asset_path(spec["asset_name"]))
    return texture


def _ensure_material_instance(name, parent):
    path = _asset_path(name)
    material = _load(path)
    if not material:
        material = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
            name,
            DEST,
            unreal.MaterialInstanceConstant,
            unreal.MaterialInstanceConstantFactoryNew(),
        )
    if not material:
        raise RuntimeError(f"Could not create material instance: {path}")
    _safe_set(material, "parent", parent)
    return material


def _build_textured_material(name, texture):
    parent = _load(CHAR_MASTER_PATH)
    if not parent:
        raise RuntimeError(f"Missing character unlit master: {CHAR_MASTER_PATH}")

    material = _ensure_material_instance(name, parent)
    for param in ("DiffuseColorMap", "BaseColorTexture"):
        try:
            unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(
                material,
                param,
                texture,
            )
        except Exception:
            pass
    try:
        unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(
            material,
            "Brightness",
            1.25,
        )
    except Exception:
        pass
    unreal.EditorAssetLibrary.save_asset(_asset_path(name))
    return material


def _ensure_color_master():
    path = _asset_path(COLOR_MASTER_NAME)
    material = _load(path)
    if not material:
        material = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
            COLOR_MASTER_NAME,
            DEST,
            unreal.Material,
            unreal.MaterialFactoryNew(),
        )
        if not material:
            raise RuntimeError(f"Could not create color master: {path}")
        _safe_set(material, "shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
        _safe_set(material, "blend_mode", unreal.BlendMode.BLEND_OPAQUE)
        _safe_set(material, "two_sided", True)
        _safe_set(material, "used_with_skeletal_mesh", True)

        param = unreal.MaterialEditingLibrary.create_material_expression(
            material,
            unreal.MaterialExpressionVectorParameter,
            -300,
            0,
        )
        _safe_set(param, "parameter_name", "BaseColor")
        _safe_set(param, "default_value", unreal.LinearColor(0.58, 0.30, 0.14, 1.0))
        unreal.MaterialEditingLibrary.connect_material_property(
            param,
            "",
            unreal.MaterialProperty.MP_EMISSIVE_COLOR,
        )
    unreal.MaterialEditingLibrary.recompile_material(material)
    unreal.EditorAssetLibrary.save_asset(path)
    return material


def _build_color_material(name, color):
    parent = _ensure_color_master()
    material = _ensure_material_instance(name, parent)
    unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(
        material,
        "BaseColor",
        color,
    )
    unreal.EditorAssetLibrary.save_asset(_asset_path(name))
    return material


def _slot_name(slot, fallback):
    try:
        return str(slot.get_editor_property("material_slot_name"))
    except Exception:
        return str(fallback)


def _slot_material_name(slot):
    try:
        material = slot.get_editor_property("material_interface")
        return material.get_name() if material else ""
    except Exception:
        return ""


def _assign_materials(mesh, materials):
    slots = list(mesh.get_editor_property("materials") or [])
    assigned = []
    for index, slot in enumerate(slots):
        slot_name = _slot_name(slot, index)
        current_name = _slot_material_name(slot)
        lookup = SLOT_MATERIALS.get(slot_name) or SLOT_MATERIALS.get(current_name)
        material = materials.get(lookup)
        if not material:
            assigned.append(
                {
                    "index": index,
                    "slot": slot_name,
                    "previous": current_name,
                    "assigned": None,
                    "warning": "no material mapping",
                }
            )
            continue
        slot.set_editor_property("material_interface", material)
        assigned.append(
            {
                "index": index,
                "slot": slot_name,
                "previous": current_name,
                "assigned": material.get_path_name(),
            }
        )
    mesh.set_editor_property("materials", slots)
    unreal.EditorAssetLibrary.save_asset(MESH_PATH)
    return assigned


def repair():
    _ensure_directory()
    imported_textures = {}
    materials = {}

    for key, spec in TEXTURES.items():
        texture = _import_texture(key, spec)
        imported_textures[key] = texture.get_path_name()
        materials[key] = _build_textured_material(spec["material_name"], texture)

    materials["neck"] = _build_color_material(
        "MI_MikeRigPrototype_NeckBridgeSkin",
        unreal.LinearColor(0.78, 0.42, 0.22, 1.0),
    )

    mesh = _load(MESH_PATH)
    if not mesh or not isinstance(mesh, unreal.SkeletalMesh):
        raise RuntimeError(f"Missing skeletal mesh: {MESH_PATH}")

    assigned = _assign_materials(mesh, materials)
    unreal.EditorAssetLibrary.save_directory(DEST, only_if_is_dirty=True, recursive=True)

    report = {
        "ok": True,
        "mesh": mesh.get_path_name(),
        "textures": imported_textures,
        "materials": {key: material.get_path_name() for key, material in materials.items()},
        "assigned": assigned,
    }
    REPORT_PATH.parent.mkdir(parents=True, exist_ok=True)
    REPORT_PATH.write_text(json.dumps(report, indent=2), encoding="utf-8")
    unreal.log(f"[RepairMikeRigPrototypeMaterials] Wrote {REPORT_PATH}")
    return report


def main():
    repair()


if __name__ == "__main__":
    main()
