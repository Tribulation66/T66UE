"""
Import animated ToonStyle hero FBXs into Unreal and update CharacterVisuals.csv.

Run in the full Unreal editor after C++ has been rebuilt with Walk/Idle/Jump/Roll
visual row fields:

  UnrealEditor.exe C:/UE/T66/T66.uproject -ExecutePythonScript="C:/UE/T66/Model Generation/Rigging and Animation/Tools/import_animated_toonstyle_heroes_to_unreal.py"

The script consumes the manifest written by create_animated_toonstyle_hero_sources.py.
It reuses each hero's existing Pixal3DToonStyle material instance so the skeletal
mesh keeps the same ToonStyle tint, inner-line, and base-color treatment as the
static production import.
"""

from __future__ import annotations

import csv
import json
import os
from pathlib import Path

import unreal


PROJECT_DIR = Path(unreal.SystemLibrary.get_project_directory())
PROJECT_ROOT = PROJECT_DIR
MANIFEST_PATH = Path(
    os.environ.get(
        "T66_ANIMATED_TOONSTYLE_MANIFEST",
        PROJECT_ROOT
        / "Model Generation"
        / "Rigging and Animation"
        / "Runs"
        / "AnimatedToonStyleHeroes_20260522"
        / "animated_toonstyle_hero_sources_manifest.json",
    )
)
CSV_PATH = PROJECT_DIR / "Content" / "Data" / "CharacterVisuals.csv"
REPORT_PATH = Path(
    os.environ.get(
        "T66_ANIMATED_TOONSTYLE_IMPORT_REPORT",
        PROJECT_DIR / "Saved" / "AnimatedToonStyleHeroImportReport.json",
    )
)
TOON_CHARACTER_PARENT_MATERIAL = "/Game/ToonStyle/Materials/M_Toon_Character.M_Toon_Character"
DEFAULT_INNER_LINE_TEXTURE = "/Game/ToonStyle/Textures/T_InnerLines_DefaultBlack.T_InnerLines_DefaultBlack"


def asset_ref(package_path: str) -> str:
    leaf = package_path.rsplit("/", 1)[-1]
    return f"{package_path}.{leaf}"


def ensure_dir(path: str) -> None:
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def hero_target_dir(hero_id: str) -> str:
    _, index, body = hero_id.split("_", 2)
    return f"/Game/Characters/Heroes/Hero_{index}/{body}/AnimatedToonStyle"


def record_target_dir(record: dict[str, object]) -> str:
    explicit_target_dir = str(record.get("target_dir") or "").strip()
    if explicit_target_dir:
        return explicit_target_dir
    return hero_target_dir(str(record.get("visual_id") or record["hero_id"]))


def hero_static_material_path(hero_id: str) -> str:
    _, index, body = hero_id.split("_", 2)
    return asset_ref(f"/Game/Characters/Heroes/Hero_{index}/{body}/Pixal3DToonStyle/Materials/MI_{hero_id}")


def animated_material_package(visual_id: str, target_dir: str) -> str:
    return f"{target_dir}/Materials/MI_{visual_id}_AnimatedToonStyle"


def animated_texture_package(visual_id: str, target_dir: str, parameter_name: str) -> str:
    suffix_by_parameter = {
        "BaseColorTexture": "BaseColor",
        "TintTexture": "Tint",
        "InnerLineTexture": "InnerLines",
    }
    suffix = suffix_by_parameter.get(parameter_name, parameter_name)
    return f"{target_dir}/Textures/T_{visual_id}_Animated_{suffix}"


def make_mesh_import_options(import_source_materials: bool = False) -> unreal.FbxImportUI:
    options = unreal.FbxImportUI()
    options.set_editor_property("import_as_skeletal", True)
    options.set_editor_property("import_mesh", True)
    options.set_editor_property("import_animations", False)
    options.set_editor_property("import_materials", import_source_materials)
    options.set_editor_property("import_textures", import_source_materials)
    options.set_editor_property("automated_import_should_detect_type", False)
    options.set_editor_property("mesh_type_to_import", unreal.FBXImportType.FBXIT_SKELETAL_MESH)
    data = options.get_editor_property("skeletal_mesh_import_data")
    # The Blender FBXs carry meter scene units. With convert_scene_unit enabled,
    # Unreal converts them to centimeters; an extra 100x import scale makes the
    # skeletal bounds fail the runtime invisibility guard.
    data.set_editor_property("import_uniform_scale", 1.0)
    data.set_editor_property("convert_scene", True)
    data.set_editor_property("convert_scene_unit", True)
    data.set_editor_property("force_front_x_axis", False)
    return options


def make_anim_import_options(skeleton: unreal.Skeleton) -> unreal.FbxImportUI:
    options = unreal.FbxImportUI()
    options.set_editor_property("import_as_skeletal", True)
    options.set_editor_property("import_mesh", False)
    options.set_editor_property("import_animations", True)
    options.set_editor_property("import_materials", False)
    options.set_editor_property("import_textures", False)
    options.set_editor_property("automated_import_should_detect_type", False)
    options.set_editor_property("skeleton", skeleton)
    try:
        options.set_editor_property("mesh_type_to_import", unreal.FBXImportType.FBXIT_ANIMATION)
    except Exception:
        pass
    anim_data = options.get_editor_property("anim_sequence_import_data")
    try:
        anim_data.set_editor_property("import_bone_tracks", True)
        anim_data.set_editor_property("remove_redundant_keys", False)
    except Exception as exc:
        unreal.log_warning(f"[AnimatedToonStyleHeroes] Could not set all animation import options: {exc}")
    return options


def import_task(filename: str, destination_path: str, destination_name: str, options: unreal.FbxImportUI) -> list[str]:
    ensure_dir(destination_path)
    task = unreal.AssetImportTask()
    task.automated = True
    task.save = True
    task.replace_existing = True
    task.replace_existing_settings = True
    task.filename = filename
    task.destination_path = destination_path
    task.destination_name = destination_name
    task.options = options
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    return list(task.imported_object_paths or [])


def import_texture_task(filename: str, destination_package: str) -> str | None:
    source_path = Path(filename)
    if not source_path.exists():
        unreal.log_warning(f"[AnimatedToonStyleHeroes] Missing texture source {filename}")
        return None

    destination_path, destination_name = destination_package.rsplit("/", 1)
    ensure_dir(destination_path)
    task = unreal.AssetImportTask()
    task.automated = True
    task.save = True
    task.replace_existing = True
    task.replace_existing_settings = False
    task.filename = str(source_path)
    task.destination_path = destination_path
    task.destination_name = destination_name
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

    expected_ref = asset_ref(destination_package)
    texture = unreal.EditorAssetLibrary.load_asset(expected_ref)
    if not texture:
        for imported_path in task.imported_object_paths or []:
            candidate = unreal.EditorAssetLibrary.load_asset(imported_path)
            if candidate and candidate.get_class().get_name() in {"Texture2D", "Texture"}:
                if imported_path != expected_ref:
                    unreal.EditorAssetLibrary.rename_asset(imported_path.split(".", 1)[0], destination_package)
                texture = unreal.EditorAssetLibrary.load_asset(expected_ref)
                break

    if not texture:
        unreal.log_warning(f"[AnimatedToonStyleHeroes] Failed to import texture {filename}")
        return None

    unreal.EditorAssetLibrary.save_loaded_asset(texture)
    return expected_ref


def import_record_textures(visual_id: str, target_dir: str, record: dict[str, object]) -> dict[str, str]:
    imported: dict[str, str] = {}
    for parameter_name, source_path in (record.get("texture_sources") or {}).items():
        destination_package = animated_texture_package(visual_id, target_dir, str(parameter_name))
        texture_ref = import_texture_task(str(source_path), destination_package)
        if texture_ref:
            imported[str(parameter_name)] = texture_ref
    return imported


def set_material_texture_parameter(material, parameter_name: str, texture_ref: str | None) -> None:
    if not texture_ref:
        return
    texture = unreal.EditorAssetLibrary.load_asset(texture_ref)
    if not texture:
        unreal.log_warning(f"[AnimatedToonStyleHeroes] Texture parameter {parameter_name} missing asset {texture_ref}")
        return
    try:
        unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(material, parameter_name, texture)
    except Exception as exc:
        unreal.log_warning(f"[AnimatedToonStyleHeroes] Could not set {parameter_name} on {material.get_path_name()}: {exc}")


def set_material_scalar_parameter(material, parameter_name: str, value: float) -> None:
    try:
        unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(material, parameter_name, value)
    except Exception:
        pass


def set_material_vector_parameter(material, parameter_name: str, value: unreal.LinearColor) -> None:
    try:
        unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(material, parameter_name, value)
    except Exception:
        pass


def ensure_skeletal_material_usage(material) -> None:
    try:
        base_material = material.get_base_material()
    except Exception:
        base_material = None
    if not base_material:
        return
    try:
        unreal.MaterialEditingLibrary.set_material_usage(base_material, unreal.MaterialUsage.MATUSAGE_SKELETAL_MESH)
        unreal.EditorAssetLibrary.save_loaded_asset(base_material)
    except Exception as exc:
        unreal.log_warning(f"[AnimatedToonStyleHeroes] Could not set skeletal usage on {base_material.get_path_name()}: {exc}")


def ensure_material_ref_skeletal_usage(material_ref: str | None) -> None:
    if not material_ref:
        return
    material = unreal.EditorAssetLibrary.load_asset(material_ref)
    if material:
        ensure_skeletal_material_usage(material)


def create_material_instance(material_package: str, parent_ref: str):
    material_ref = asset_ref(material_package)
    material = load_asset_if_exists(material_ref)
    if not material:
        ensure_dir(material_package.rsplit("/", 1)[0])
        material = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
            material_package.rsplit("/", 1)[-1],
            material_package.rsplit("/", 1)[0],
            unreal.MaterialInstanceConstant,
            unreal.MaterialInstanceConstantFactoryNew(),
        )
    parent = unreal.EditorAssetLibrary.load_asset(parent_ref)
    if material and parent:
        unreal.MaterialEditingLibrary.set_material_instance_parent(material, parent)
    return material


def ensure_skeletal_mesh_material_usage(skeletal_mesh: unreal.SkeletalMesh) -> None:
    for slot in list(skeletal_mesh.get_editor_property("materials") or []):
        material = None
        try:
            material = slot.get_editor_property("material_interface")
        except Exception:
            material = None
        if material:
            ensure_skeletal_material_usage(material)


def build_animated_toonstyle_material(
    visual_id: str,
    target_dir: str,
    texture_refs: dict[str, str],
    source_material_ref: str | None = None,
) -> str:
    if not texture_refs:
        return source_material_ref or hero_static_material_path(visual_id)

    material_package = animated_material_package(visual_id, target_dir)
    material_ref = asset_ref(material_package)
    material = load_asset_if_exists(material_ref)
    if not material:
        source_material_ref = source_material_ref or (hero_static_material_path(visual_id) if visual_id.startswith("Hero_") else None)
        source_material = load_asset_if_exists(source_material_ref) if source_material_ref else None
        if source_material:
            ensure_dir(material_package.rsplit("/", 1)[0])
            unreal.EditorAssetLibrary.duplicate_asset(source_material_ref.split(".", 1)[0], material_package)
            material = load_asset_if_exists(material_ref)
        else:
            material = create_material_instance(material_package, TOON_CHARACTER_PARENT_MATERIAL)

    if not material:
        fallback = source_material_ref or (hero_static_material_path(visual_id) if visual_id.startswith("Hero_") else "")
        unreal.log_warning(f"[AnimatedToonStyleHeroes] Could not create animated material for {visual_id}; using fallback {fallback}")
        return fallback

    set_material_texture_parameter(material, "BaseColorTexture", texture_refs.get("BaseColorTexture"))
    set_material_texture_parameter(material, "DiffuseColorMap", texture_refs.get("BaseColorTexture"))
    set_material_texture_parameter(material, "EmissiveTexture", texture_refs.get("BaseColorTexture"))
    set_material_texture_parameter(material, "TintTexture", texture_refs.get("TintTexture"))
    set_material_texture_parameter(material, "InnerLineTexture", texture_refs.get("InnerLineTexture") or DEFAULT_INNER_LINE_TEXTURE)
    set_material_scalar_parameter(material, "Brightness", 1.0)
    set_material_vector_parameter(material, "Tint", unreal.LinearColor(1.0, 1.0, 1.0, 1.0))
    set_material_vector_parameter(material, "BaseColorFactor", unreal.LinearColor(1.0, 1.0, 1.0, 1.0))
    set_material_vector_parameter(material, "EmissiveFactor", unreal.LinearColor(1.0, 1.0, 1.0, 1.0))
    ensure_skeletal_material_usage(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material)
    return material_ref


def assign_toonstyle_material(skeletal_mesh: unreal.SkeletalMesh, material_path: str) -> dict[str, object]:
    material = unreal.EditorAssetLibrary.load_asset(material_path)
    if not material:
        return {"ok": False, "material_path": material_path, "error": "missing ToonStyle material instance"}

    slots = list(skeletal_mesh.get_editor_property("materials") or [])
    if not slots:
        return {"ok": False, "material_path": material_path, "error": "skeletal mesh has no material slots"}

    for slot in slots:
        slot.set_editor_property("material_interface", material)
    skeletal_mesh.set_editor_property("materials", slots)
    unreal.EditorAssetLibrary.save_loaded_asset(skeletal_mesh)
    return {"ok": True, "material_path": material_path, "slot_count": len(slots)}


def describe_skeletal_materials(skeletal_mesh: unreal.SkeletalMesh) -> dict[str, object]:
    slots = list(skeletal_mesh.get_editor_property("materials") or [])
    material_paths: list[str] = []
    for slot in slots:
        material = None
        try:
            material = slot.get_editor_property("material_interface")
        except Exception:
            material = None
        material_paths.append(material.get_path_name() if material else "")
    return {
        "ok": bool(material_paths),
        "slot_count": len(slots),
        "material_paths": material_paths,
        "preserved_source_materials": True,
    }


def find_or_load(path: str):
    return unreal.EditorAssetLibrary.load_asset(path)


def load_asset_if_exists(path: str):
    package_path = path.split(".", 1)[0]
    if not unreal.EditorAssetLibrary.does_asset_exist(package_path):
        return None
    return unreal.EditorAssetLibrary.load_asset(path)


def import_hero(record: dict[str, object]) -> dict[str, object]:
    hero_id = str(record.get("visual_id") or record["hero_id"])
    target_dir = record_target_dir(record)
    mesh_name = f"SK_{hero_id}"
    mesh_package = f"{target_dir}/{mesh_name}"
    mesh_ref = asset_ref(mesh_package)
    preserve_source_materials = bool(record.get("preserve_source_materials"))
    unreal.log(f"[AnimatedToonStyleHeroes] {hero_id}: preserve_source_materials={preserve_source_materials}")

    mesh_imported = import_task(
        str(record["mesh_fbx"]),
        target_dir,
        mesh_name,
        make_mesh_import_options(import_source_materials=preserve_source_materials),
    )
    skeletal_mesh = find_or_load(mesh_ref)
    if not skeletal_mesh:
        raise RuntimeError(f"{hero_id}: failed to load imported skeletal mesh {mesh_ref}; imported={mesh_imported}")

    skeleton = skeletal_mesh.get_editor_property("skeleton")
    if not skeleton:
        raise RuntimeError(f"{hero_id}: imported skeletal mesh has no skeleton")

    texture_refs: dict[str, str] = {}
    if preserve_source_materials:
        ensure_skeletal_mesh_material_usage(skeletal_mesh)
        ensure_material_ref_skeletal_usage(hero_static_material_path(hero_id))
        material_report = describe_skeletal_materials(skeletal_mesh)
    else:
        texture_refs = import_record_textures(hero_id, target_dir, record)
        material_path = build_animated_toonstyle_material(hero_id, target_dir, texture_refs, record.get("source_material_ref"))
        material_report = assign_toonstyle_material(skeletal_mesh, material_path)

    anim_refs: dict[str, str] = {}
    anim_imported: dict[str, list[str]] = {}
    animations = record.get("animations") or {}
    for label in ("Idle", "Walk", "Jump", "Roll"):
        anim_fbx = str(animations[label])
        anim_name = f"AM_{hero_id}_{label}"
        anim_package = f"{target_dir}/{anim_name}"
        expected_ref = asset_ref(anim_package)
        imported = import_task(anim_fbx, target_dir, anim_name, make_anim_import_options(skeleton))
        anim_imported[label] = imported

        asset = find_or_load(expected_ref)
        if not asset:
            for imported_path in imported:
                imported_asset = find_or_load(imported_path)
                if imported_asset and imported_asset.get_class().get_name() == "AnimSequence":
                    if imported_path != expected_ref:
                        unreal.EditorAssetLibrary.rename_asset(imported_path.split(".", 1)[0], anim_package)
                    asset = find_or_load(expected_ref)
                    break
        if not asset:
            raise RuntimeError(f"{hero_id}: failed to import {label} animation; imported={imported}")

        unreal.EditorAssetLibrary.save_loaded_asset(asset)
        anim_refs[label] = expected_ref

    return {
        "hero_id": hero_id,
        "visual_id": hero_id,
        "target_dir": target_dir,
        "skeletal_mesh": mesh_ref,
        "skeleton": skeleton.get_path_name(),
        "mesh_imported": mesh_imported,
        "animations": anim_refs,
        "anim_imported": anim_imported,
        "material": material_report,
        "textures": texture_refs,
        "preserve_source_materials": preserve_source_materials,
        "asset_kind": record.get("asset_kind", "hero"),
        "csv": {
            "mesh_relative_location": str(record.get("mesh_relative_location") or "(X=0,Y=0,Z=0)"),
            "mesh_relative_rotation": str(record.get("mesh_relative_rotation") or "(Pitch=0,Yaw=-90.000000,Roll=0)"),
            "mesh_relative_scale": str(record.get("mesh_relative_scale") or "(X=1,Y=1,Z=1)"),
            "b_auto_ground_to_actor_origin": str(record.get("b_auto_ground_to_actor_origin") or "true"),
        },
    }


def update_character_visuals_csv(imports: list[dict[str, object]]) -> None:
    with CSV_PATH.open("r", encoding="utf-8", newline="") as handle:
        reader = csv.DictReader(handle)
        rows = list(reader)
        fieldnames = list(reader.fieldnames or [])

    required_fields = [
        "---",
        "SkeletalMesh",
        "StaticMesh",
        "OutlineStaticMesh",
        "PixelatedTextureAssetPath",
        "WalkAnimation",
        "IdleAnimation",
        "JumpAnimation",
        "RollAnimation",
        "MeshRelativeLocation",
        "MeshRelativeRotation",
        "MeshRelativeScale",
        "bLoopAnimation",
        "bAutoGroundToActorOrigin",
    ]
    if fieldnames != required_fields:
        raise RuntimeError(f"Unexpected CharacterVisuals.csv fields: {fieldnames}")

    by_id = {str(item.get("visual_id") or item["hero_id"]): item for item in imports}
    rows_by_id = {row.get("---", ""): row for row in rows}
    for visual_id, imported in by_id.items():
        if visual_id in rows_by_id:
            continue
        rows_by_id[visual_id] = {
            "---": visual_id,
            "SkeletalMesh": "",
            "StaticMesh": "",
            "OutlineStaticMesh": "",
            "PixelatedTextureAssetPath": "",
            "WalkAnimation": "",
            "IdleAnimation": "",
            "JumpAnimation": "",
            "RollAnimation": "",
            "MeshRelativeLocation": "(X=0,Y=0,Z=0)",
            "MeshRelativeRotation": "(Pitch=0,Yaw=-90.000000,Roll=0)",
            "MeshRelativeScale": "(X=1,Y=1,Z=1)",
            "bLoopAnimation": "true",
            "bAutoGroundToActorOrigin": "true",
        }
        rows.append(rows_by_id[visual_id])

    for row in rows:
        row_id = row.get("---", "")
        if row_id not in by_id:
            continue
        imported = by_id[row_id]
        animations = imported["animations"]
        csv_values = imported.get("csv") or {}
        row["SkeletalMesh"] = str(imported["skeletal_mesh"])
        row["WalkAnimation"] = str(animations["Walk"])
        row["IdleAnimation"] = str(animations["Idle"])
        row["JumpAnimation"] = str(animations["Jump"])
        row["RollAnimation"] = str(animations["Roll"])
        row["MeshRelativeLocation"] = str(csv_values.get("mesh_relative_location") or "(X=0,Y=0,Z=0)")
        row["MeshRelativeRotation"] = str(csv_values.get("mesh_relative_rotation") or "(Pitch=0,Yaw=-90.000000,Roll=0)")
        row["MeshRelativeScale"] = str(csv_values.get("mesh_relative_scale") or "(X=1,Y=1,Z=1)")
        row["bAutoGroundToActorOrigin"] = str(csv_values.get("b_auto_ground_to_actor_origin") or "true").lower()
        textures = imported.get("textures") or {}
        if textures.get("BaseColorTexture"):
            row["PixelatedTextureAssetPath"] = str(textures["BaseColorTexture"])
        row["bLoopAnimation"] = "true"

    with CSV_PATH.open("w", encoding="utf-8", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=fieldnames, lineterminator="\n")
        writer.writeheader()
        writer.writerows(rows)


def reload_character_visuals_data_table() -> bool:
    dt_path = "/Game/Data/DT_CharacterVisuals"
    dt = unreal.EditorAssetLibrary.load_asset(dt_path)
    if not dt:
        unreal.log_error(f"[AnimatedToonStyleHeroes] Missing {dt_path}")
        return False
    ok = unreal.DataTableFunctionLibrary.fill_data_table_from_csv_file(dt, str(CSV_PATH))
    if ok:
        unreal.EditorAssetLibrary.save_asset(dt_path)
    return bool(ok)


def main() -> None:
    if not MANIFEST_PATH.exists():
        raise FileNotFoundError(MANIFEST_PATH)

    manifest = json.loads(MANIFEST_PATH.read_text(encoding="utf-8"))
    manifest_records = manifest.get("characters") or manifest.get("heroes", [])
    imports = [import_hero(record) for record in manifest_records]
    update_character_visuals_csv(imports)
    dt_reloaded = reload_character_visuals_data_table()

    report = {
        "manifest": str(MANIFEST_PATH),
        "csv": str(CSV_PATH),
        "dt_reloaded": dt_reloaded,
        "imports": imports,
    }
    REPORT_PATH.parent.mkdir(parents=True, exist_ok=True)
    REPORT_PATH.write_text(json.dumps(report, indent=2), encoding="utf-8")
    unreal.log(f"[AnimatedToonStyleHeroes] wrote {REPORT_PATH}")
    if not dt_reloaded:
        raise RuntimeError("DT_CharacterVisuals reload failed")


if __name__ == "__main__":
    main()
