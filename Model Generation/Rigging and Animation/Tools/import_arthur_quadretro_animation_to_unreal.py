r"""
Import the rigged Royal Chad QuadRetro UAL mesh/actions and wire a QA visual row.

The defaults target the UAL/Rigodotify-derived row, not the rejected manual
AnimQA pass. Keep importing to a QA row first, then promote Hero_1_Chad only
after Blender previews and in-game QA pass.

For scripts under folders with spaces, prefer the no-space wrapper:
  set T66_RIGGING_ANIMATION_TOOL_SCRIPT=C:\UE\T66\Model Generation\Rigging and Animation\Tools\import_arthur_quadretro_animation_to_unreal.py
  UnrealEditor-Cmd.exe T66.uproject -run=pythonscript -script=C:/UE/T66/Scripts/RunRiggingAnimationToolAndExit.py

If FBX import hits the UE 5.7 Slate commandlet assertion, run with full editor:
  UnrealEditor.exe T66.uproject -ExecutePythonScript=... with T66_ARTHUR_QUADRETRO_QUIT_EDITOR=1
"""

import csv
import json
import os
import sys
from pathlib import Path

import unreal


PROJECT_DIR = Path(unreal.SystemLibrary.get_project_directory()).resolve()
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_SCRIPTS_DIR = str(PROJECT_DIR / "Scripts")
for search_path in (SCRIPT_DIR, PROJECT_SCRIPTS_DIR):
    if search_path not in sys.path:
        sys.path.append(search_path)

import SetupCharacterVisualsDataTable


SOURCE_DIR = Path(os.environ.get(
    "T66_ARTHUR_QUADRETRO_EXPORT_DIR",
    PROJECT_DIR / "Model Generation" / "Rigging and Animation" / "Runs" / "Arthur_QuadRetro_UAL_Retarget_20260514" / "Exports",
))
DEST_DIR = os.environ.get("T66_ARTHUR_QUADRETRO_DEST_DIR", "/Game/Characters/Heroes/Hero_1/Chad/QuadRetroUALQA")
VISUAL_ID = os.environ.get("T66_ARTHUR_QUADRETRO_VISUAL_ID", "Hero_1_Chad_QuadRetroUALQA")
PROMOTE_LIVE_ROW = os.environ.get("T66_ARTHUR_QUADRETRO_PROMOTE_LIVE", "0").strip().lower() in ("1", "true", "yes")
SKELETAL_MESH_NAME = os.environ.get("T66_ARTHUR_QUADRETRO_MESH_NAME", "SK_Hero_1_Chad_QuadRetroUALQA")
SKELETAL_MESH_PATH = f"{DEST_DIR}/{SKELETAL_MESH_NAME}.{SKELETAL_MESH_NAME}"
LIVE_STATIC_MESH_PATH = "/Game/Characters/Heroes/Hero_1/Chad/QuadRetro/SM_Hero_1_Chad_QuadRetro.SM_Hero_1_Chad_QuadRetro"
PIXELATED_TEXTURE_PATH = os.environ.get(
    "T66_ARTHUR_QUADRETRO_PIXELATED_TEXTURE",
    "/Game/Characters/Heroes/Hero_1/Chad/QuadRetro/RoyalChad_QuadRetro/Textures/RoyalChad_QuadRetro_Pixelated_512.RoyalChad_QuadRetro_Pixelated_512",
)
SKELETAL_UNLIT_MATERIAL_PARENT = os.environ.get(
    "T66_ARTHUR_QUADRETRO_SKELETAL_MATERIAL_PARENT",
    "/Game/Materials/M_Character_Unlit.M_Character_Unlit",
)
MATERIAL_NAME = os.environ.get("T66_ARTHUR_QUADRETRO_MATERIAL_NAME", "MI_Hero_1_Chad_QuadRetroUALQA_Unlit")
MATERIAL_PATH = f"{DEST_DIR}/{MATERIAL_NAME}.{MATERIAL_NAME}"
CSV_PATH = PROJECT_DIR / "Content" / "Data" / "CharacterVisuals.csv"
REPORT_PATH = PROJECT_DIR / "Saved" / "ArthurQuadRetroAnimationImportReport.json"
ACTION_PREFIX = os.environ.get("T66_ARTHUR_QUADRETRO_ACTION_PREFIX", "AM_Hero_1_Chad_QuadRetroUALQA_")

CLIPS = {
    "Idle": f"{ACTION_PREFIX}Idle",
    "Walk": f"{ACTION_PREFIX}Walk",
    "Jump": f"{ACTION_PREFIX}Jump",
    "Roll": f"{ACTION_PREFIX}Roll",
}


def _load_asset(path):
    asset = unreal.EditorAssetLibrary.load_asset(path)
    if not asset:
        raise RuntimeError(f"Missing Unreal asset: {path}")
    return asset


def _ensure_directory(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def _make_mesh_import_options():
    options = unreal.FbxImportUI()
    options.set_editor_property("automated_import_should_detect_type", False)
    options.set_editor_property("import_as_skeletal", True)
    options.set_editor_property("import_mesh", True)
    options.set_editor_property("import_animations", False)
    options.set_editor_property("import_materials", False)
    options.set_editor_property("import_textures", False)
    options.set_editor_property("create_physics_asset", False)
    try:
        options.set_editor_property("mesh_type_to_import", unreal.FBXImportType.FBXIT_SKELETAL_MESH)
    except Exception as exc:
        unreal.log_warning(f"[ArthurQuadRetroAnim] Could not set FBXIT_SKELETAL_MESH import type: {exc}")
    try:
        import_data = unreal.FbxSkeletalMeshImportData()
        options.set_editor_property("skeletal_mesh_import_data", import_data)
        import_data.set_editor_property("import_meshes_in_bone_hierarchy", True)
        import_data.set_editor_property("convert_scene", True)
        import_data.set_editor_property("force_front_x_axis", False)
        import_data.set_editor_property("import_uniform_scale", 1.0)
    except Exception as exc:
        unreal.log_warning(f"[ArthurQuadRetroAnim] Could not set all skeletal import options: {exc}")
    return options


def _make_animation_import_options(skeleton):
    options = unreal.FbxImportUI()
    options.set_editor_property("automated_import_should_detect_type", False)
    options.set_editor_property("import_as_skeletal", True)
    options.set_editor_property("import_mesh", False)
    options.set_editor_property("import_animations", True)
    options.set_editor_property("import_materials", False)
    options.set_editor_property("import_textures", False)
    options.set_editor_property("create_physics_asset", False)
    options.set_editor_property("skeleton", skeleton)
    try:
        options.set_editor_property("mesh_type_to_import", unreal.FBXImportType.FBXIT_ANIMATION)
    except Exception as exc:
        unreal.log_warning(f"[ArthurQuadRetroAnim] Could not set FBXIT_ANIMATION import type: {exc}")
    try:
        anim_data = unreal.FbxAnimSequenceImportData()
        options.set_editor_property("anim_sequence_import_data", anim_data)
        anim_data.set_editor_property("animation_length", unreal.FBXAnimationLengthImportType.FBXALIT_EXPORTED_TIME)
        anim_data.set_editor_property("import_bone_tracks", True)
        anim_data.set_editor_property("import_meshes_in_bone_hierarchy", False)
        anim_data.set_editor_property("use_default_sample_rate", True)
        anim_data.set_editor_property("snap_to_closest_frame_boundary", True)
    except Exception as exc:
        unreal.log_warning(f"[ArthurQuadRetroAnim] Could not set all animation import options: {exc}")
    return options


def _import_skeletal_mesh():
    source = SOURCE_DIR / f"{SKELETAL_MESH_NAME}.fbx"
    if not source.is_file():
        raise RuntimeError(f"Missing generated skeletal mesh FBX: {source}")
    _ensure_directory(DEST_DIR)
    task = unreal.AssetImportTask()
    task.automated = True
    task.save = True
    task.replace_existing = True
    task.replace_existing_settings = False
    task.filename = str(source)
    task.destination_path = DEST_DIR
    task.destination_name = SKELETAL_MESH_NAME
    task.options = _make_mesh_import_options()
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    mesh = unreal.EditorAssetLibrary.load_asset(SKELETAL_MESH_PATH)
    if not mesh:
        raise RuntimeError(f"Skeletal mesh import did not produce {SKELETAL_MESH_PATH}. Imported: {list(task.imported_object_paths or [])}")
    unreal.EditorAssetLibrary.save_asset(SKELETAL_MESH_PATH)
    return mesh, {
        "source": str(source),
        "asset_path": SKELETAL_MESH_PATH,
        "class": mesh.get_class().get_name(),
        "imported_paths": list(task.imported_object_paths or []),
    }


def _get_mesh_skeleton(mesh):
    try:
        skeleton = mesh.get_editor_property("skeleton")
    except Exception:
        skeleton = None
    if not skeleton:
        raise RuntimeError(f"Imported skeletal mesh has no skeleton: {SKELETAL_MESH_PATH}")
    return skeleton


def _import_clip(label, asset_name, skeleton):
    source = SOURCE_DIR / f"{asset_name}.fbx"
    if not source.is_file():
        raise RuntimeError(f"Missing generated action FBX for {label}: {source}")
    task = unreal.AssetImportTask()
    task.automated = True
    task.save = True
    task.replace_existing = True
    task.replace_existing_settings = False
    task.filename = str(source)
    task.destination_path = DEST_DIR
    task.destination_name = asset_name
    task.options = _make_animation_import_options(skeleton)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    asset_path = f"{DEST_DIR}/{asset_name}.{asset_name}"
    asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    if not asset:
        raise RuntimeError(f"Animation import did not produce {asset_path}. Imported: {list(task.imported_object_paths or [])}")
    unreal.EditorAssetLibrary.save_asset(asset_path)
    return {
        "label": label,
        "source": str(source),
        "asset_path": asset_path,
        "class": asset.get_class().get_name(),
        "imported_paths": list(task.imported_object_paths or []),
    }


def _ensure_quadretro_material():
    existing = unreal.EditorAssetLibrary.load_asset(MATERIAL_PATH)
    if existing:
        material = existing
    else:
        factory = unreal.MaterialInstanceConstantFactoryNew()
        asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
        material = asset_tools.create_asset(MATERIAL_NAME, DEST_DIR, unreal.MaterialInstanceConstant, factory)
        if not material:
            raise RuntimeError(f"Failed to create material instance {MATERIAL_PATH}")

    parent = _load_asset(SKELETAL_UNLIT_MATERIAL_PARENT)
    try:
        if isinstance(parent, unreal.Material):
            changed = False
            for prop, value in (
                ("shading_model", unreal.MaterialShadingModel.MSM_UNLIT),
                ("two_sided", True),
                ("used_with_skeletal_mesh", True),
            ):
                try:
                    if parent.get_editor_property(prop) != value:
                        parent.set_editor_property(prop, value)
                        changed = True
                except Exception:
                    pass
            if changed:
                try:
                    unreal.MaterialEditingLibrary.recompile_material(parent)
                except Exception:
                    pass
                unreal.EditorAssetLibrary.save_loaded_asset(parent)
    except Exception as exc:
        unreal.log_warning(f"[ArthurQuadRetroAnim] Could not ensure skeletal material parent usage flags: {exc}")

    texture = _load_asset(PIXELATED_TEXTURE_PATH)
    b_parent_assigned = False
    try:
        unreal.MaterialEditingLibrary.set_material_instance_parent(material, parent)
        b_parent_assigned = True
    except Exception as exc:
        unreal.log_warning(f"[ArthurQuadRetroAnim] Could not set material parent via MaterialEditingLibrary: {exc}")
    try:
        material.set_editor_property("parent", parent)
        b_parent_assigned = True
    except Exception as exc:
        unreal.log_warning(f"[ArthurQuadRetroAnim] Could not set parent material {SKELETAL_UNLIT_MATERIAL_PARENT}: {exc}")
    if not b_parent_assigned:
        raise RuntimeError(f"Failed to assign material parent {SKELETAL_UNLIT_MATERIAL_PARENT} to {MATERIAL_PATH}")

    for param_name in ("EmissiveTexture", "BaseColorTexture", "DiffuseColorMap"):
        try:
            unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(material, param_name, texture)
        except Exception as exc:
            unreal.log_warning(f"[ArthurQuadRetroAnim] Could not set texture parameter {param_name}: {exc}")
    for param_name, value in (("Brightness", 1.0),):
        try:
            unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(material, param_name, value)
        except Exception:
            pass
    for param_name, value in (("Tint", unreal.LinearColor.WHITE), ("BaseColorFactor", unreal.LinearColor.BLACK), ("EmissiveFactor", unreal.LinearColor.WHITE)):
        try:
            unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(material, param_name, value)
        except Exception:
            pass

    try:
        unreal.MaterialEditingLibrary.update_material_instance(material)
    except Exception:
        pass
    try:
        material.post_edit_change()
    except Exception:
        pass
    unreal.EditorAssetLibrary.save_loaded_asset(material)
    return material


def _assign_material(mesh, material):
    try:
        slots = list(mesh.get_editor_property("materials") or [])
    except Exception as exc:
        raise RuntimeError(f"Could not read skeletal material slots from {SKELETAL_MESH_PATH}: {exc}")

    if not slots:
        raise RuntimeError(f"{SKELETAL_MESH_PATH} has no skeletal material slots to assign")

    for index, slot in enumerate(slots):
        try:
            slot.set_editor_property("material_interface", material)
        except Exception as exc:
            raise RuntimeError(f"Could not assign material slot {index} on {SKELETAL_MESH_PATH}: {exc}")

    try:
        mesh.set_editor_property("materials", slots)
    except Exception as exc:
        raise RuntimeError(f"Could not write skeletal material slots on {SKELETAL_MESH_PATH}: {exc}")

    try:
        mesh.post_edit_change()
    except Exception:
        pass
    unreal.EditorAssetLibrary.save_asset(SKELETAL_MESH_PATH)


def _read_visual_rows():
    with CSV_PATH.open("r", newline="", encoding="utf-8-sig") as handle:
        reader = csv.DictReader(handle)
        return reader.fieldnames, list(reader)


def _write_visual_rows(fieldnames, rows):
    with CSV_PATH.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.DictWriter(handle, fieldnames=fieldnames, quoting=csv.QUOTE_ALL)
        writer.writeheader()
        writer.writerows(rows)


def _update_character_visuals_csv():
    fieldnames, rows = _read_visual_rows()
    if not fieldnames or "---" not in fieldnames:
        raise RuntimeError(f"{CSV_PATH} is missing the row-name column")
    required_columns = [
        "SkeletalMesh",
        "StaticMesh",
        "PixelatedTextureAssetPath",
        "LoopingAnimation",
        "AlertAnimation",
        "RunAnimation",
        "RollAnimation",
        "MeshRelativeLocation",
        "MeshRelativeRotation",
        "MeshRelativeScale",
        "bLoopAnimation",
        "bAutoGroundToActorOrigin",
    ]
    missing = [column for column in required_columns if column not in fieldnames]
    if missing:
        raise RuntimeError(f"{CSV_PATH} is missing columns: {missing}")

    row = next((item for item in rows if item.get("---") == VISUAL_ID), None)
    if not row:
        row = {fieldname: "" for fieldname in fieldnames}
        row["---"] = VISUAL_ID
        live_index = next((index for index, item in enumerate(rows) if item.get("---") == "Hero_1_Chad"), None)
        rows.insert(live_index + 1 if live_index is not None else len(rows), row)

    row_values = {
        "SkeletalMesh": SKELETAL_MESH_PATH,
        "StaticMesh": "",
        "PixelatedTextureAssetPath": PIXELATED_TEXTURE_PATH,
        "LoopingAnimation": f"{DEST_DIR}/{CLIPS['Walk']}.{CLIPS['Walk']}",
        "AlertAnimation": f"{DEST_DIR}/{CLIPS['Idle']}.{CLIPS['Idle']}",
        "RunAnimation": f"{DEST_DIR}/{CLIPS['Jump']}.{CLIPS['Jump']}",
        "RollAnimation": f"{DEST_DIR}/{CLIPS['Roll']}.{CLIPS['Roll']}",
        "MeshRelativeLocation": "(X=0,Y=0,Z=0)",
        "MeshRelativeRotation": "(Pitch=0,Yaw=90.000000,Roll=0)",
        "MeshRelativeScale": "(X=1,Y=1,Z=1)",
        "bLoopAnimation": "true",
        "bAutoGroundToActorOrigin": "true",
    }
    row.update(row_values)
    live_row = next((item for item in rows if item.get("---") == "Hero_1_Chad"), None)
    if PROMOTE_LIVE_ROW:
        if not live_row:
            raise RuntimeError(f"{CSV_PATH} is missing Hero_1_Chad; cannot promote Arthur QuadRetro UAL row")
        live_row.update(row_values)
    _write_visual_rows(fieldnames, rows)
    return row, live_row


def main():
    unreal.log("=== ImportArthurQuadRetroAnimationAndExit ===")
    if not SOURCE_DIR.is_dir():
        raise RuntimeError(f"Missing Arthur QuadRetro export directory: {SOURCE_DIR}")

    mesh, mesh_report = _import_skeletal_mesh()
    skeleton = _get_mesh_skeleton(mesh)
    material = _ensure_quadretro_material()
    _assign_material(mesh, material)

    imported = []
    for label, asset_name in CLIPS.items():
        unreal.log(f"[ArthurQuadRetroAnim] Importing {label}: {asset_name}")
        imported.append(_import_clip(label, asset_name, skeleton))

    temp_row, live_row = _update_character_visuals_csv()
    SetupCharacterVisualsDataTable.main()

    report = {
        "source_dir": str(SOURCE_DIR),
        "dest_dir": DEST_DIR,
        "visual_id": VISUAL_ID,
        "promoted_live_row": PROMOTE_LIVE_ROW,
        "action_prefix": ACTION_PREFIX,
        "skeletal_mesh": mesh_report,
        "skeleton": skeleton.get_path_name(),
        "material": MATERIAL_PATH,
        "material_parent": SKELETAL_UNLIT_MATERIAL_PARENT,
        "pixelated_texture": PIXELATED_TEXTURE_PATH,
        "imported": imported,
        "temp_row": temp_row,
        "live_hero_1_chad_row": live_row,
        "live_static_mesh_expected": LIVE_STATIC_MESH_PATH,
    }
    REPORT_PATH.parent.mkdir(parents=True, exist_ok=True)
    REPORT_PATH.write_text(json.dumps(report, indent=2), encoding="utf-8")
    unreal.log(f"[ArthurQuadRetroAnim] wrote {REPORT_PATH}")
    unreal.log("=== ImportArthurQuadRetroAnimationAndExit DONE ===")

    if os.environ.get("T66_ARTHUR_QUADRETRO_QUIT_EDITOR", "0").strip().lower() in ("1", "true", "yes"):
        unreal.SystemLibrary.quit_editor()


if __name__ == "__main__":
    main()
