"""
Import the raw FriendSlop humanoid skeletal rig into Unreal for the TestRoom
ragdoll/PAC spike.

This intentionally preserves the raw Pixal3D material identity. It does not
touch CharacterVisuals.csv, ToonStyle assets, Quad Retro assets, tint textures,
outline sidecars, or gameplay DataTables.
"""

from __future__ import annotations

import json
import os
from pathlib import Path

import unreal


LOG_PREFIX = "[FriendSlopRawHumanoidImport]"
PROJECT_ROOT = Path(unreal.SystemLibrary.get_project_directory())
RUN_ROOT = PROJECT_ROOT / "Model Generation" / "Runs" / "Pixal3D" / "FriendSlopProbe_Hero1Male_20260604_1415"
DEFAULT_FBX = RUN_ROOT / "Blender" / "Rigging" / "Hero_1_Chad_Male_FriendSlop_Skeletal.fbx"
DEFAULT_TEXTURE = RUN_ROOT / "RawTexturedFBX" / "Hero_1_Chad_Male" / "Textures" / "Hero_1_Chad_Male_00_Image_0.png"
DEFAULT_ANIMATION_ROOT = RUN_ROOT / "Blender" / "Rigging" / "AnimationSources"
TARGET_DIR = "/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/Skeletal"
MESH_NAME = "SK_Hero_1_Chad_Male_FriendSlop"
ANIMATION_SOURCES = {
    "idle": ("Hero_1_Chad_Male_FriendSlop_Idle.fbx", "AM_Hero_1_Chad_Male_FriendSlop_Idle"),
    "walk": ("Hero_1_Chad_Male_FriendSlop_Walk.fbx", "AM_Hero_1_Chad_Male_FriendSlop_Walk"),
    "jump": ("Hero_1_Chad_Male_FriendSlop_Jump.fbx", "AM_Hero_1_Chad_Male_FriendSlop_Jump"),
    "roll": ("Hero_1_Chad_Male_FriendSlop_Roll.fbx", "AM_Hero_1_Chad_Male_FriendSlop_Roll"),
}
PARENT_MATERIAL = "/Game/Materials/M_GLB_Unlit"
STATIC_BASE_COLOR_TEXTURE = (
    "/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/Textures/"
    "T_Hero_1_Chad_Male_BaseColor.T_Hero_1_Chad_Male_BaseColor"
)
REPORT_PATH = PROJECT_ROOT / "Reports" / "AgentReviews" / "FriendSlopUnrealRagdollImport" / "friendslop_humanoid_skeletal_import_report.json"


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


def import_texture_if_needed(source_path: Path) -> tuple[str, object | None, str]:
    existing = unreal.EditorAssetLibrary.load_asset(STATIC_BASE_COLOR_TEXTURE)
    if existing:
        return existing.get_path_name(), existing, ""

    if not source_path.exists() or source_path.stat().st_size <= 0:
        return STATIC_BASE_COLOR_TEXTURE, None, f"missing base color source texture: {source_path}"

    texture_dir = f"{TARGET_DIR}/Textures"
    texture_name = "T_Hero_1_Chad_Male_FriendSlop_BaseColor"
    ensure_dir(texture_dir)

    task = unreal.AssetImportTask()
    task.automated = True
    task.save = True
    task.replace_existing = True
    task.replace_existing_settings = True
    task.filename = str(source_path)
    task.destination_path = texture_dir
    task.destination_name = texture_name
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    scan_path(texture_dir)

    texture_ref = asset_ref(f"{texture_dir}/{texture_name}")
    texture = unreal.EditorAssetLibrary.load_asset(texture_ref)
    if not texture:
        for imported in task.imported_object_paths or []:
            candidate = unreal.EditorAssetLibrary.load_asset(imported)
            if candidate and candidate.get_class().get_name() in {"Texture", "Texture2D"}:
                texture = candidate
                texture_ref = candidate.get_path_name()
                break

    if not texture:
        return texture_ref, None, f"texture import did not produce a Texture2D: {source_path}"

    try_set(texture, "srgb", True)
    try:
        texture.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_World)
    except Exception:
        pass
    unreal.EditorAssetLibrary.save_loaded_asset(texture)
    return texture.get_path_name(), texture, ""


def ensure_material(texture: object) -> tuple[str, object | None, str]:
    material_dir = f"{TARGET_DIR}/Materials"
    material_name = "MI_SK_Hero_1_Chad_Male_FriendSlop"
    material_ref = asset_ref(f"{material_dir}/{material_name}")
    ensure_dir(material_dir)

    material = unreal.EditorAssetLibrary.load_asset(material_ref)
    if not material:
        material = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
            material_name,
            material_dir,
            unreal.MaterialInstanceConstant,
            unreal.MaterialInstanceConstantFactoryNew(),
        )
    if not material:
        return material_ref, None, f"failed to create material instance: {material_ref}"

    parent = unreal.EditorAssetLibrary.load_asset(PARENT_MATERIAL)
    if not parent:
        return material.get_path_name(), material, f"missing parent material: {PARENT_MATERIAL}"

    try:
        unreal.MaterialEditingLibrary.set_material_instance_parent(material, parent)
    except Exception:
        try_set(material, "parent", parent)

    for parameter_name in ("BaseColorTexture", "DiffuseColorMap"):
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
    for parameter_name, value in (("Brightness", 1.0), ("Opacity", 1.0)):
        try:
            unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(material, parameter_name, value)
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
            ("import_uniform_scale", 1.0),
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
            ("convert_scene", True),
            ("convert_scene_unit", True),
            ("force_front_x_axis", False),
        ):
            try_set(data, property_name, value)
        try_set(data, "animation_length", unreal.FBXAnimationLengthImportType.FBXALIT_EXPORTED_TIME)
    return options


def import_skeletal_mesh(source_fbx: Path) -> tuple[str, object | None, list[str], str]:
    if not source_fbx.exists() or source_fbx.stat().st_size <= 0:
        return asset_ref(f"{TARGET_DIR}/{MESH_NAME}"), None, [], f"missing skeletal FBX: {source_fbx}"

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

    mesh_ref = asset_ref(f"{TARGET_DIR}/{MESH_NAME}")
    mesh = unreal.EditorAssetLibrary.load_asset(mesh_ref)
    if not mesh:
        for imported in task.imported_object_paths or []:
            candidate = unreal.EditorAssetLibrary.load_asset(imported)
            if candidate and candidate.get_class().get_name() == "SkeletalMesh":
                mesh = candidate
                mesh_ref = candidate.get_path_name()
                break

    if not mesh:
        return mesh_ref, None, list(task.imported_object_paths or []), f"skeletal import produced no SkeletalMesh: {task.imported_object_paths}"

    unreal.EditorAssetLibrary.save_loaded_asset(mesh)
    return mesh.get_path_name(), mesh, list(task.imported_object_paths or []), ""


def import_animation(source_fbx: Path, destination_name: str, skeleton: object) -> tuple[str, object | None, list[str], str]:
    destination_ref = asset_ref(f"{TARGET_DIR}/{destination_name}")
    if not skeleton:
        return destination_ref, None, [], f"cannot import animation {destination_name}; mesh skeleton is missing"
    if not source_fbx.exists() or source_fbx.stat().st_size <= 0:
        return destination_ref, None, [], f"missing animation FBX: {source_fbx}"

    ensure_dir(TARGET_DIR)
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

    animation = unreal.EditorAssetLibrary.load_asset(destination_ref)
    if not animation:
        for imported in task.imported_object_paths or []:
            candidate = unreal.EditorAssetLibrary.load_asset(imported)
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
    try_set(animation, "use_normalized_root_motion_scale", False)
    unreal.EditorAssetLibrary.save_loaded_asset(animation)
    return animation.get_path_name(), animation, list(task.imported_object_paths or []), ""


def assign_material(mesh: object, material: object) -> dict[str, object]:
    slots = []
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
        mat = None
        try:
            mat = slot.get_editor_property("material_interface")
        except Exception:
            mat = None
        material_paths.append(mat.get_path_name() if mat else "")
    return {"ok": True, "slot_count": len(slots), "material_paths": material_paths}


def mesh_bounds(mesh: object) -> dict[str, list[float]]:
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
    source_fbx = Path(os.environ.get("T66_FRIENDSLOP_HUMANOID_FBX", DEFAULT_FBX))
    source_texture = Path(os.environ.get("T66_FRIENDSLOP_HUMANOID_TEXTURE", DEFAULT_TEXTURE))
    report_path = Path(os.environ.get("T66_FRIENDSLOP_HUMANOID_IMPORT_REPORT", REPORT_PATH))
    report_path.parent.mkdir(parents=True, exist_ok=True)

    errors: list[str] = []
    texture_ref, texture, texture_error = import_texture_if_needed(source_texture)
    if texture_error:
        errors.append(texture_error)

    material_ref = ""
    material = None
    material_error = "skipped material creation because texture is missing"
    if texture:
        material_ref, material, material_error = ensure_material(texture)
        if material_error:
            errors.append(material_error)

    mesh_ref, mesh, imported_paths, mesh_error = import_skeletal_mesh(source_fbx)
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

    animation_root = Path(os.environ.get("T66_FRIENDSLOP_HUMANOID_ANIMATION_ROOT", DEFAULT_ANIMATION_ROOT))
    animation_report: dict[str, dict[str, object]] = {}
    if mesh and skeleton:
        for animation_key, (source_name, destination_name) in ANIMATION_SOURCES.items():
            animation_ref, _animation, animation_imported_paths, animation_error = import_animation(
                animation_root / source_name,
                destination_name,
                skeleton,
            )
            animation_report[animation_key] = {
                "source_fbx": str(animation_root / source_name),
                "asset": animation_ref,
                "imported_object_paths": animation_imported_paths,
                "ok": not animation_error,
                "error": animation_error,
            }
            if animation_error:
                errors.append(animation_error)
    else:
        animation_report["skipped"] = {
            "ok": False,
            "error": "mesh or skeleton missing",
        }

    report = {
        "ok": not errors,
        "errors": errors,
        "source_fbx": str(source_fbx),
        "source_texture": str(source_texture),
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
        "process_boundary": {
            "character_visuals_csv_touched": False,
            "toonstyle_touched": False,
            "quad_retro_touched": False,
            "outline_sidecar_touched": False,
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
