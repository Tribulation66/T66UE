"""
Verify the Royal Chad QuadRetro UAL skeletal assets and visual row wiring.

By default this validates the UALQA row while live Hero_1_Chad remains static.
Set T66_ARTHUR_QUADRETRO_EXPECT_LIVE_PROMOTED=1 after promotion to require the
live row to match the UALQA row.

For paths under folders with spaces, run through:
  C:/UE/T66/Scripts/RunRiggingAnimationToolAndExit.py
"""

import csv
import json
import os
from pathlib import Path

import unreal


PROJECT_DIR = Path(unreal.SystemLibrary.get_project_directory()).resolve()
CSV_PATH = PROJECT_DIR / "Content" / "Data" / "CharacterVisuals.csv"
REPORT_PATH = PROJECT_DIR / "Saved" / "ArthurQuadRetroAnimationVerifyReport.json"
DEST_DIR = os.environ.get("T66_ARTHUR_QUADRETRO_DEST_DIR", "/Game/Characters/Heroes/Hero_1/Chad/QuadRetroUALQA")
# TOOL-ONLY validation row for the Arthur/UAL rigging pipeline. Gameplay uses Hero_1_Chad.
VISUAL_ID = os.environ.get("T66_ARTHUR_QUADRETRO_VISUAL_ID", "Hero_1_Chad_QuadRetroUALQA")
SKELETAL_MESH_NAME = os.environ.get("T66_ARTHUR_QUADRETRO_MESH_NAME", "SK_Hero_1_Chad_QuadRetroUALQA")
SKELETAL_MESH_PATH = f"{DEST_DIR}/{SKELETAL_MESH_NAME}.{SKELETAL_MESH_NAME}"
MATERIAL_NAME = os.environ.get("T66_ARTHUR_QUADRETRO_MATERIAL_NAME", "MI_Hero_1_Chad_QuadRetroUALQA_Unlit")
MATERIAL_PATH = f"{DEST_DIR}/{MATERIAL_NAME}.{MATERIAL_NAME}"
MATERIAL_PARENT_PATH = "/Game/Materials/M_Character_Unlit.M_Character_Unlit"
PIXELATED_TEXTURE_PATH = "/Game/Characters/Heroes/Hero_1/Chad/QuadRetro/RoyalChad_QuadRetro/Textures/RoyalChad_QuadRetro_Pixelated_512.RoyalChad_QuadRetro_Pixelated_512"
LIVE_HERO_1_STATIC_MESH = "/Game/Characters/Heroes/Hero_1/Chad/QuadRetro/SM_Hero_1_Chad_QuadRetro.SM_Hero_1_Chad_QuadRetro"
EXPECT_LIVE_PROMOTED = os.environ.get("T66_ARTHUR_QUADRETRO_EXPECT_LIVE_PROMOTED", "0").strip().lower() in ("1", "true", "yes")
ACTION_PREFIX = os.environ.get("T66_ARTHUR_QUADRETRO_ACTION_PREFIX", "AM_Hero_1_Chad_QuadRetroUALQA_")
EXPECTED = {
    "Idle": f"{DEST_DIR}/{ACTION_PREFIX}Idle.{ACTION_PREFIX}Idle",
    "Walk": f"{DEST_DIR}/{ACTION_PREFIX}Walk.{ACTION_PREFIX}Walk",
    "Jump": f"{DEST_DIR}/{ACTION_PREFIX}Jump.{ACTION_PREFIX}Jump",
    "Roll": f"{DEST_DIR}/{ACTION_PREFIX}Roll.{ACTION_PREFIX}Roll",
}


def _load(path):
    asset = unreal.EditorAssetLibrary.load_asset(path)
    if not asset:
        raise RuntimeError(f"Missing asset: {path}")
    return asset


def _play_length(asset):
    try:
        return float(asset.get_play_length())
    except Exception:
        try:
            return float(asset.get_editor_property("sequence_length"))
        except Exception:
            return 0.0


def _read_visual_row(row_name):
    with CSV_PATH.open("r", newline="", encoding="utf-8-sig") as handle:
        for row in csv.DictReader(handle):
            if row.get("---") == row_name:
                return row
    raise RuntimeError(f"CharacterVisuals.csv missing {row_name}")


def _get_skeleton(asset):
    try:
        return asset.get_editor_property("skeleton")
    except Exception:
        return None


def _mesh_bounds_summary(mesh):
    try:
        bounds = mesh.get_bounds()
        origin = bounds.origin
        extent = bounds.box_extent
        return {
            "origin": [origin.x, origin.y, origin.z],
            "box_extent": [extent.x, extent.y, extent.z],
            "height": extent.z * 2.0,
            "sphere_radius": bounds.sphere_radius,
        }
    except Exception as exc:
        return {"error": str(exc)}


def _skeletal_mesh_material_names(mesh):
    names = []
    try:
        slots = list(mesh.get_editor_property("materials") or [])
    except Exception as exc:
        return [f"error: {exc}"]

    for slot in slots:
        try:
            assigned = slot.get_editor_property("material_interface")
        except Exception as exc:
            names.append(f"error: {exc}")
            continue
        names.append(assigned.get_path_name() if assigned else None)
    return names


def _material_parent_path(material):
    try:
        parent = material.get_editor_property("parent")
    except Exception:
        parent = None
    return parent.get_path_name() if parent else None


def _material_texture_parameter_path(material, parameter_name):
    try:
        texture = unreal.MaterialEditingLibrary.get_material_instance_texture_parameter_value(material, parameter_name)
    except Exception:
        texture = None
    if not texture:
        return None
    return texture.get_path_name()


def main():
    unreal.log("=== VerifyArthurQuadRetroAnimationAndExit ===")
    errors = []
    mesh = _load(SKELETAL_MESH_PATH)
    material = _load(MATERIAL_PATH)
    texture = _load(PIXELATED_TEXTURE_PATH)
    skeleton = _get_skeleton(mesh)
    if not skeleton:
        errors.append(f"{SKELETAL_MESH_PATH} has no skeleton")

    assets = {}
    for label, path in EXPECTED.items():
        asset = _load(path)
        asset_skeleton = _get_skeleton(asset)
        if skeleton and asset_skeleton != skeleton:
            errors.append(f"{label} skeleton mismatch: {asset_skeleton} != {skeleton}")
        length = _play_length(asset)
        if length <= 0.0:
            errors.append(f"{label} has non-positive play length: {length}")
        assets[label] = {
            "path": path,
            "class": asset.get_class().get_name(),
            "play_length": length,
            "skeleton": asset_skeleton.get_path_name() if asset_skeleton else None,
        }

    temp_row = _read_visual_row(VISUAL_ID)
    expected_row_values = {
        "SkeletalMesh": SKELETAL_MESH_PATH,
        "StaticMesh": "",
        "PixelatedTextureAssetPath": PIXELATED_TEXTURE_PATH,
        "LoopingAnimation": EXPECTED["Walk"],
        "AlertAnimation": EXPECTED["Idle"],
        "RunAnimation": EXPECTED["Jump"],
        "RollAnimation": EXPECTED["Roll"],
        "MeshRelativeRotation": "(Pitch=0,Yaw=90.000000,Roll=0)",
        "MeshRelativeScale": "(X=1,Y=1,Z=1)",
        "bLoopAnimation": "true",
        "bAutoGroundToActorOrigin": "true",
    }
    for column, expected in expected_row_values.items():
        actual = temp_row.get(column, "")
        if actual != expected:
            errors.append(f"{VISUAL_ID} {column}: expected {expected!r}, got {actual!r}")

    live_row = _read_visual_row("Hero_1_Chad")
    if EXPECT_LIVE_PROMOTED:
        for column, expected in expected_row_values.items():
            actual = live_row.get(column, "")
            if actual != expected:
                errors.append(f"Hero_1_Chad promoted {column}: expected {expected!r}, got {actual!r}")
    else:
        if live_row.get("SkeletalMesh", ""):
            errors.append(f"Hero_1_Chad must remain static during QA; SkeletalMesh is {live_row.get('SkeletalMesh')!r}")
        if live_row.get("StaticMesh", "") != LIVE_HERO_1_STATIC_MESH:
            errors.append(f"Hero_1_Chad StaticMesh expected {LIVE_HERO_1_STATIC_MESH!r}, got {live_row.get('StaticMesh', '')!r}")

    material_names = _skeletal_mesh_material_names(mesh)
    if material.get_path_name() not in material_names:
        errors.append(f"Skeletal mesh does not reference {material.get_path_name()} in material slots: {material_names}")
    parent_path = _material_parent_path(material)
    if parent_path != MATERIAL_PARENT_PATH:
        errors.append(f"Material parent expected {MATERIAL_PARENT_PATH}, got {parent_path}")
    material_texture_params = {}
    for parameter_name in ("EmissiveTexture", "BaseColorTexture", "DiffuseColorMap"):
        param_path = _material_texture_parameter_path(material, parameter_name)
        material_texture_params[parameter_name] = param_path
        if param_path != PIXELATED_TEXTURE_PATH:
            errors.append(f"Material {parameter_name}: expected {PIXELATED_TEXTURE_PATH}, got {param_path}")

    report = {
        "ok": not errors,
        "errors": errors,
        "expect_live_promoted": EXPECT_LIVE_PROMOTED,
        "visual_id": VISUAL_ID,
        "action_prefix": ACTION_PREFIX,
        "skeletal_mesh": {
            "path": SKELETAL_MESH_PATH,
            "class": mesh.get_class().get_name(),
            "skeleton": skeleton.get_path_name() if skeleton else None,
            "bounds": _mesh_bounds_summary(mesh),
            "materials": material_names,
        },
        "material": material.get_path_name(),
        "material_parent": parent_path,
        "material_texture_params": material_texture_params,
        "pixelated_texture": texture.get_path_name(),
        "assets": assets,
        "temp_row": temp_row,
        "live_hero_1_chad_row": live_row,
    }
    REPORT_PATH.parent.mkdir(parents=True, exist_ok=True)
    REPORT_PATH.write_text(json.dumps(report, indent=2), encoding="utf-8")
    unreal.log(f"[ArthurQuadRetroAnimVerify] wrote {REPORT_PATH}")
    if errors:
        raise RuntimeError("Arthur QuadRetro animation verification failed: " + "; ".join(errors))
    unreal.log("=== VerifyArthurQuadRetroAnimationAndExit DONE ===")


if __name__ == "__main__":
    main()
