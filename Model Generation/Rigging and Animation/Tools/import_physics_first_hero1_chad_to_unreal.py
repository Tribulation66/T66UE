"""
Import the fresh physics-first Hero 1 Chad rig and pose-target clips into Unreal.

This importer targets a new PhysicsFirst content folder and does not reuse or
overwrite the old FriendSlop raw skeletal spike outputs.
"""

from __future__ import annotations

import json
import os
from pathlib import Path

import unreal


LOG_PREFIX = "[PhysicsFirstHero1Import]"
PROJECT_ROOT = Path(unreal.SystemLibrary.get_project_directory())
RUN_ROOT = PROJECT_ROOT / "Model Generation" / "Runs" / "Pixal3D" / "FriendSlopProbe_Hero1Male_20260604_1415"
SOURCE_ROOT = RUN_ROOT / "Blender" / "PhysicsFirstHero"
SOURCE_FBX = SOURCE_ROOT / "Hero_1_Chad_PhysicsFirst_Skeletal.fbx"
SOURCE_TEXTURE = RUN_ROOT / "RawTexturedFBX" / "Hero_1_Chad_Male" / "Textures" / "Hero_1_Chad_Male_00_Image_0.png"
ANIMATION_ROOT = SOURCE_ROOT / "AnimationSources"

TARGET_DIR = "/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/PhysicsFirst"
MESH_NAME = "SK_Hero_1_Chad_PhysicsFirst"
MATERIAL_NAME = "MI_SK_Hero_1_Chad_PhysicsFirst"
TEXTURE_NAME = "T_Hero_1_Chad_PhysicsFirst_BaseColor"
PARENT_MATERIAL = "/Game/Materials/M_GLB_Unlit.M_GLB_Unlit"
REPORT_PATH = PROJECT_ROOT / "Reports" / "AgentReviews" / "FallGuysHeroRiggingStage2Implementation" / "physics_first_hero1_unreal_import_report.json"
IMPORT_UNIFORM_SCALE = 0.01

ANIMATION_SOURCES = {
    "Idle": ("AM_Hero_1_Chad_PhysicsFirst_Idle.fbx", "AM_Hero_1_Chad_PhysicsFirst_Idle"),
    "Walk": ("AM_Hero_1_Chad_PhysicsFirst_Walk.fbx", "AM_Hero_1_Chad_PhysicsFirst_Walk"),
    "Jump": ("AM_Hero_1_Chad_PhysicsFirst_Jump.fbx", "AM_Hero_1_Chad_PhysicsFirst_Jump"),
    "Leap": ("AM_Hero_1_Chad_PhysicsFirst_Leap.fbx", "AM_Hero_1_Chad_PhysicsFirst_Leap"),
    "RecoverStand": ("AM_Hero_1_Chad_PhysicsFirst_RecoverStand.fbx", "AM_Hero_1_Chad_PhysicsFirst_RecoverStand"),
    "GetUp_Back": ("AM_Hero_1_Chad_PhysicsFirst_GetUp_Back.fbx", "AM_Hero_1_Chad_PhysicsFirst_GetUp_Back"),
    "GetUp_Front": ("AM_Hero_1_Chad_PhysicsFirst_GetUp_Front.fbx", "AM_Hero_1_Chad_PhysicsFirst_GetUp_Front"),
}


def asset_ref(package_path: str) -> str:
    leaf = package_path.rsplit("/", 1)[-1]
    return f"{package_path}.{leaf}"


def ensure_dir(path: str) -> None:
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def scan_path(path: str) -> None:
    try:
        unreal.AssetRegistryHelpers.get_asset_registry().scan_paths_synchronous([path], True)
    except Exception:
        pass


def try_set(target: object, property_name: str, value: object) -> bool:
    try:
        target.set_editor_property(property_name, value)
        return True
    except Exception:
        return False


def load_asset(path: str):
    return unreal.EditorAssetLibrary.load_asset(path)


def import_texture(source_path: Path) -> tuple[str, object | None, str]:
    texture_dir = f"{TARGET_DIR}/Textures"
    texture_ref = asset_ref(f"{texture_dir}/{TEXTURE_NAME}")
    existing = load_asset(texture_ref)
    if existing:
        return existing.get_path_name(), existing, ""

    if not source_path.exists() or source_path.stat().st_size <= 0:
        return texture_ref, None, f"missing base color source texture: {source_path}"

    ensure_dir(texture_dir)
    task = unreal.AssetImportTask()
    task.automated = True
    task.save = True
    task.replace_existing = True
    task.replace_existing_settings = True
    task.filename = str(source_path)
    task.destination_path = texture_dir
    task.destination_name = TEXTURE_NAME
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    scan_path(texture_dir)

    texture = load_asset(texture_ref)
    if not texture:
        for imported in task.imported_object_paths or []:
            candidate = load_asset(imported)
            if candidate and candidate.get_class().get_name() in {"Texture", "Texture2D"}:
                texture = candidate
                texture_ref = candidate.get_path_name()
                break

    if not texture:
        return texture_ref, None, f"texture import produced no Texture2D: {source_path}"

    try_set(texture, "srgb", True)
    unreal.EditorAssetLibrary.save_loaded_asset(texture)
    return texture.get_path_name(), texture, ""


def ensure_material(texture: object) -> tuple[str, object | None, str]:
    material_dir = f"{TARGET_DIR}/Materials"
    material_ref = asset_ref(f"{material_dir}/{MATERIAL_NAME}")
    ensure_dir(material_dir)

    material = load_asset(material_ref)
    if not material:
        material = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
            MATERIAL_NAME,
            material_dir,
            unreal.MaterialInstanceConstant,
            unreal.MaterialInstanceConstantFactoryNew(),
        )
    if not material:
        return material_ref, None, f"failed to create material instance: {material_ref}"

    parent = load_asset(PARENT_MATERIAL)
    if not parent:
        return material.get_path_name(), material, f"missing parent material: {PARENT_MATERIAL}"

    try:
        unreal.MaterialEditingLibrary.set_material_instance_parent(material, parent)
    except Exception:
        try_set(material, "parent", parent)

    for parameter_name in ("BaseColorTexture", "DiffuseColorMap", "EmissiveTexture"):
        try:
            unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(material, parameter_name, texture)
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
    try:
        base_material = material.get_base_material()
        if base_material:
            unreal.MaterialEditingLibrary.set_material_usage(base_material, unreal.MaterialUsage.MATUSAGE_SKELETAL_MESH)
            unreal.EditorAssetLibrary.save_loaded_asset(base_material)
    except Exception as exc:
        unreal.log_warning(f"{LOG_PREFIX} Could not set skeletal material usage: {exc}")

    unreal.EditorAssetLibrary.save_loaded_asset(material)
    return material.get_path_name(), material, ""


def make_skeletal_import_options() -> unreal.FbxImportUI:
    options = unreal.FbxImportUI()
    try_set(options, "automated_import_should_detect_type", False)
    try_set(options, "mesh_type_to_import", unreal.FBXImportType.FBXIT_SKELETAL_MESH)
    try_set(options, "import_as_skeletal", True)
    try_set(options, "import_mesh", True)
    try_set(options, "import_animations", False)
    try_set(options, "import_materials", False)
    try_set(options, "import_textures", False)
    try_set(options, "create_physics_asset", False)

    data = options.get_editor_property("skeletal_mesh_import_data")
    if data:
        for property_name, value in (
            ("import_uniform_scale", IMPORT_UNIFORM_SCALE),
            ("convert_scene", True),
            ("convert_scene_unit", True),
            ("force_front_x_axis", False),
            ("import_meshes_in_bone_hierarchy", True),
        ):
            try_set(data, property_name, value)
    return options


def make_animation_import_options(skeleton: object) -> unreal.FbxImportUI:
    options = unreal.FbxImportUI()
    try_set(options, "automated_import_should_detect_type", False)
    try_set(options, "mesh_type_to_import", unreal.FBXImportType.FBXIT_ANIMATION)
    try_set(options, "import_as_skeletal", True)
    try_set(options, "import_mesh", False)
    try_set(options, "import_animations", True)
    try_set(options, "import_materials", False)
    try_set(options, "import_textures", False)
    try_set(options, "skeleton", skeleton)

    data = options.get_editor_property("anim_sequence_import_data")
    if data:
        for property_name, value in (
            ("import_bone_tracks", True),
            ("remove_redundant_keys", False),
            ("import_uniform_scale", IMPORT_UNIFORM_SCALE),
            ("convert_scene", True),
            ("convert_scene_unit", True),
            ("force_front_x_axis", False),
        ):
            try_set(data, property_name, value)
        try_set(data, "animation_length", unreal.FBXAnimationLengthImportType.FBXALIT_EXPORTED_TIME)
    return options


def import_skeletal_mesh(source_fbx: Path) -> tuple[str, object | None, list[str], str]:
    mesh_ref = asset_ref(f"{TARGET_DIR}/{MESH_NAME}")
    if not source_fbx.exists() or source_fbx.stat().st_size <= 0:
        return mesh_ref, None, [], f"missing skeletal FBX: {source_fbx}"

    ensure_dir(TARGET_DIR)
    task = unreal.AssetImportTask()
    task.automated = True
    task.save = True
    task.replace_existing = True
    task.replace_existing_settings = True
    task.filename = str(source_fbx)
    task.destination_path = TARGET_DIR
    task.destination_name = MESH_NAME
    task.options = make_skeletal_import_options()
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    scan_path(TARGET_DIR)

    mesh = load_asset(mesh_ref)
    if not mesh:
        for imported in task.imported_object_paths or []:
            candidate = load_asset(imported)
            if candidate and candidate.get_class().get_name() == "SkeletalMesh":
                mesh = candidate
                mesh_ref = candidate.get_path_name()
                break

    if not mesh:
        return mesh_ref, None, list(task.imported_object_paths or []), (
            f"skeletal import produced no SkeletalMesh: {task.imported_object_paths}"
        )

    unreal.EditorAssetLibrary.save_loaded_asset(mesh)
    return mesh.get_path_name(), mesh, list(task.imported_object_paths or []), ""


def import_animation(source_fbx: Path, destination_name: str, skeleton: object) -> tuple[str, object | None, list[str], str]:
    destination_ref = asset_ref(f"{TARGET_DIR}/{destination_name}")
    if not skeleton:
        return destination_ref, None, [], f"cannot import animation {destination_name}; mesh skeleton is missing"
    if not source_fbx.exists() or source_fbx.stat().st_size <= 0:
        return destination_ref, None, [], f"missing animation FBX: {source_fbx}"

    task = unreal.AssetImportTask()
    task.automated = True
    task.save = True
    task.replace_existing = True
    task.replace_existing_settings = True
    task.filename = str(source_fbx)
    task.destination_path = TARGET_DIR
    task.destination_name = destination_name
    task.options = make_animation_import_options(skeleton)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    scan_path(TARGET_DIR)

    animation = load_asset(destination_ref)
    if not animation:
        for imported in task.imported_object_paths or []:
            candidate = load_asset(imported)
            if candidate and candidate.get_class().get_name() == "AnimSequence":
                animation = candidate
                destination_ref = candidate.get_path_name()
                break

    if not animation:
        return destination_ref, None, list(task.imported_object_paths or []), (
            f"animation import produced no AnimSequence for {destination_name}: {task.imported_object_paths}"
        )

    try_set(animation, "enable_root_motion", False)
    try_set(animation, "force_root_lock", True)
    unreal.EditorAssetLibrary.save_loaded_asset(animation)
    return animation.get_path_name(), animation, list(task.imported_object_paths or []), ""


def assign_material(mesh: object, material: object) -> dict[str, object]:
    try:
        slots = list(mesh.get_editor_property("materials") or [])
    except Exception:
        slots = []
    if not slots:
        return {"ok": False, "slot_count": 0, "error": "skeletal mesh has no material slots"}

    for slot in slots:
        try:
            slot.set_editor_property("material_interface", material)
        except Exception:
            pass
    try:
        mesh.set_editor_property("materials", slots)
    except Exception:
        pass
    try:
        mesh.post_edit_change()
    except Exception:
        pass
    unreal.EditorAssetLibrary.save_loaded_asset(mesh)

    material_paths = []
    for slot in slots:
        try:
            mat = slot.get_editor_property("material_interface")
        except Exception:
            mat = None
        material_paths.append(mat.get_path_name() if mat else "")
    return {"ok": True, "slot_count": len(slots), "material_paths": material_paths}


def mesh_bounds(mesh: object) -> dict[str, object]:
    try:
        bounds = mesh.get_bounds()
        return {
            "origin_cm": [float(bounds.origin.x), float(bounds.origin.y), float(bounds.origin.z)],
            "extent_cm": [float(bounds.box_extent.x), float(bounds.box_extent.y), float(bounds.box_extent.z)],
            "size_cm": [
                float(bounds.box_extent.x) * 2.0,
                float(bounds.box_extent.y) * 2.0,
                float(bounds.box_extent.z) * 2.0,
            ],
        }
    except Exception as exc:
        return {"error": str(exc)}


def main() -> int:
    report_path = Path(os.environ.get("T66_PHYSICS_FIRST_HERO1_IMPORT_REPORT", REPORT_PATH))
    report_path.parent.mkdir(parents=True, exist_ok=True)
    errors: list[str] = []

    texture_ref, texture, texture_error = import_texture(SOURCE_TEXTURE)
    if texture_error:
        errors.append(texture_error)

    material_ref = ""
    material = None
    if texture:
        material_ref, material, material_error = ensure_material(texture)
        if material_error:
            errors.append(material_error)
    else:
        errors.append("skipped material creation because texture is missing")

    mesh_ref, mesh, imported_paths, mesh_error = import_skeletal_mesh(SOURCE_FBX)
    if mesh_error:
        errors.append(mesh_error)

    material_report: dict[str, object] = {"ok": False, "error": "mesh or material missing"}
    if mesh and material:
        material_report = assign_material(mesh, material)
        if not material_report.get("ok"):
            errors.append(str(material_report.get("error") or "failed to assign material"))

    skeleton_ref = ""
    skeleton = None
    physics_asset_ref = ""
    if mesh:
        try:
            skeleton = mesh.get_editor_property("skeleton")
            skeleton_ref = skeleton.get_path_name() if skeleton else ""
        except Exception:
            skeleton_ref = ""
        try:
            physics_asset = mesh.get_editor_property("physics_asset")
            physics_asset_ref = physics_asset.get_path_name() if physics_asset else ""
        except Exception:
            physics_asset_ref = ""

    animation_report: dict[str, dict[str, object]] = {}
    if mesh and skeleton:
        for label, (source_name, destination_name) in ANIMATION_SOURCES.items():
            animation_ref, _animation, animation_imported_paths, animation_error = import_animation(
                ANIMATION_ROOT / source_name,
                destination_name,
                skeleton,
            )
            animation_report[label] = {
                "source_fbx": str(ANIMATION_ROOT / source_name),
                "asset": animation_ref,
                "imported_object_paths": animation_imported_paths,
                "ok": not animation_error,
                "error": animation_error,
            }
            if animation_error:
                errors.append(animation_error)
    else:
        animation_report["skipped"] = {"ok": False, "error": "mesh or skeleton missing"}

    report = {
        "ok": not errors,
        "errors": errors,
        "source_fbx": str(SOURCE_FBX),
        "source_texture": str(SOURCE_TEXTURE),
        "target_dir": TARGET_DIR,
        "skeletal_mesh": mesh_ref,
        "skeleton": skeleton_ref,
        "physics_asset_after_import": physics_asset_ref,
        "texture": texture_ref,
        "material": material_ref,
        "animations": animation_report,
        "imported_object_paths": imported_paths,
        "material_assignment": material_report,
        "bounds": mesh_bounds(mesh) if mesh else {},
        "import_uniform_scale": IMPORT_UNIFORM_SCALE,
        "process_boundary": {
            "old_spike_assets_read": False,
            "character_visuals_csv_touched": False,
            "active_ragdoll_runtime_touched": False,
        },
    }
    report_path.write_text(json.dumps(report, indent=2), encoding="utf-8")

    if errors:
        for error in errors:
            unreal.log_error(f"{LOG_PREFIX} {error}")
        unreal.log_error(f"{LOG_PREFIX} failed; report={report_path}")
        return 1

    unreal.log(f"{LOG_PREFIX} imported {mesh_ref}; report={report_path}")
    return 0


exit_code = main()
try:
    unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")
except Exception:
    pass
if exit_code:
    raise RuntimeError(f"{LOG_PREFIX} failed with exit code {exit_code}")
