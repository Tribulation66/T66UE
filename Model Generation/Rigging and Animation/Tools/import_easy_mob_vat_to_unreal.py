r"""
Import Easy mob authoring FBXs, bake AnimToTexture VAT assets, and reload DT_MobVertexAnimations.

Run with a forward-slash script path or the no-space wrapper:
  set T66_RIGGING_ANIMATION_TOOL_SCRIPT=C:\UE\T66\Model Generation\Rigging and Animation\Tools\import_easy_mob_vat_to_unreal.py
  UnrealEditor-Cmd.exe T66.uproject -run=pythonscript -script=C:/UE/T66/Scripts/RunRiggingAnimationToolAndExit.py -unattended -nop4 -nosplash -NullRHI
"""

import csv
import json
import os
import sys
from pathlib import Path

import unreal


PROJECT_DIR = Path(unreal.SystemLibrary.get_project_directory()).resolve()
SCRIPT_DIR = Path(__file__).resolve().parent
PROJECT_SCRIPTS_DIR = PROJECT_DIR / "Scripts"
for search_path in (SCRIPT_DIR, PROJECT_SCRIPTS_DIR):
    if str(search_path) not in sys.path:
        sys.path.append(str(search_path))

import SetupMobVertexAnimationsDataTable


RUN_ROOT = Path(os.environ.get(
    "T66_EASY_MOB_VAT_RUN_ROOT",
    PROJECT_DIR / "Model Generation" / "Rigging and Animation" / "Runs" / "Easy_Mob_VAT_20260514",
))
MANIFEST_PATH = RUN_ROOT / "easy_mob_vat_manifest.json"
CHARACTER_VISUALS_CSV = PROJECT_DIR / "Content" / "Data" / "CharacterVisuals.csv"
MOB_VAT_CSV = PROJECT_DIR / "Content" / "Data" / "MobVertexAnimations.csv"
REPORT_PATH = PROJECT_DIR / "Saved" / "EasyMobVATImportReport.json"

DEST_ROOT = os.environ.get("T66_EASY_MOB_VAT_DEST_ROOT", "/Game/Characters/MobsVAT")
MASTER_MATERIAL_DIR = "/Game/Materials"
MASTER_MATERIAL_NAME = "M_EasyMobVAT_Unlit_UV2"
MASTER_MATERIAL_PATH = f"{MASTER_MATERIAL_DIR}/{MASTER_MATERIAL_NAME}"

SAMPLE_DATA_ASSET = "/AnimToTexture/Characters/Mannequin/Data/DA_VertexAnimation.DA_VertexAnimation"
SAMPLE_POSITION_TEXTURE = "/AnimToTexture/Characters/Mannequin/Textures/VertexAnimation/TX_VertexPosition.TX_VertexPosition"
SAMPLE_NORMAL_TEXTURE = "/AnimToTexture/Characters/Mannequin/Textures/VertexAnimation/TX_VertexNormal.TX_VertexNormal"

CLIPS = ("Idle", "Move", "AttackCue", "HitReact", "Death")

CSV_FIELDS = [
    "---",
    "EnemyID",
    "StaticMesh",
    "Material",
    "PixelatedTextureAssetPath",
    "PositionTexture",
    "NormalTexture",
    "MeshRelativeLocation",
    "MeshRelativeRotation",
    "MeshRelativeScale",
    "bEnabled",
    "SampleRate",
    "NumFrames",
    "RowsPerFrame",
    "MinBBox",
    "SizeBBox",
    "IdleStartFrame",
    "IdleEndFrame",
    "IdlePlayRate",
    "MoveStartFrame",
    "MoveEndFrame",
    "MovePlayRate",
    "AttackCueStartFrame",
    "AttackCueEndFrame",
    "AttackCuePlayRate",
    "HitReactStartFrame",
    "HitReactEndFrame",
    "HitReactPlayRate",
    "DeathStartFrame",
    "DeathEndFrame",
    "DeathPlayRate",
]


def log(message):
    unreal.log(f"[EasyMobVATImport] {message}")


def warn(message):
    unreal.log_warning(f"[EasyMobVATImport] {message}")


def safe_set(obj, prop, value):
    try:
        obj.set_editor_property(prop, value)
        return True
    except Exception as exc:
        warn(f"Could not set {getattr(obj, 'get_name', lambda: obj)()}::{prop}: {exc}")
        return False


def ensure_directory(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def object_path(package_path):
    name = package_path.rsplit("/", 1)[-1]
    return f"{package_path}.{name}"


def load_asset(path):
    asset = unreal.EditorAssetLibrary.load_asset(path)
    if not asset:
        asset = unreal.load_asset(path)
    if not asset:
        raise RuntimeError(f"Missing asset: {path}")
    return asset


def scan_anim_to_texture_content():
    registry = unreal.AssetRegistryHelpers.get_asset_registry()
    for path in ("/AnimToTexture", "/AnimToTexture/Characters", "/AnimToTexture/Materials"):
        registry.scan_paths_synchronous([path], force_rescan=True)
    load_asset(SAMPLE_DATA_ASSET)
    load_asset(SAMPLE_POSITION_TEXTURE)
    load_asset(SAMPLE_NORMAL_TEXTURE)


def read_character_visual_rows():
    with CHARACTER_VISUALS_CSV.open("r", encoding="utf-8", newline="") as handle:
        rows = list(csv.DictReader(handle))
    return {row["---"]: row for row in rows}


def load_manifest():
    if not MANIFEST_PATH.is_file():
        raise RuntimeError(f"Missing Easy mob VAT manifest: {MANIFEST_PATH}")
    return json.loads(MANIFEST_PATH.read_text(encoding="utf-8"))


def make_vector_csv(value):
    if isinstance(value, str):
        return value
    if isinstance(value, (list, tuple)) and len(value) >= 3:
        x, y, z = value[0], value[1], value[2]
    else:
        x = getattr(value, "x", getattr(value, "X", 0.0))
        y = getattr(value, "y", getattr(value, "Y", 0.0))
        z = getattr(value, "z", getattr(value, "Z", 0.0))
    return f"(X={float(x):.6f},Y={float(y):.6f},Z={float(z):.6f})"


def make_rotator_csv(value):
    if isinstance(value, str):
        return value
    pitch = getattr(value, "pitch", getattr(value, "Pitch", 0.0))
    yaw = getattr(value, "yaw", getattr(value, "Yaw", 0.0))
    roll = getattr(value, "roll", getattr(value, "Roll", 0.0))
    return f"(Pitch={float(pitch):.6f},Yaw={float(yaw):.6f},Roll={float(roll):.6f})"


def vec_to_tuple(value):
    if isinstance(value, (list, tuple)) and len(value) >= 3:
        return float(value[0]), float(value[1]), float(value[2])
    to_tuple = getattr(value, "to_tuple", None)
    if callable(to_tuple):
        raw = to_tuple()
        if len(raw) >= 3:
            return float(raw[0]), float(raw[1]), float(raw[2])
    return (
        float(getattr(value, "x", getattr(value, "X", 0.0))),
        float(getattr(value, "y", getattr(value, "Y", 0.0))),
        float(getattr(value, "z", getattr(value, "Z", 0.0))),
    )


def get_bake_vector(data_asset, property_names):
    for property_name in property_names:
        try:
            return data_asset.get_editor_property(property_name)
        except Exception:
            continue
    raise RuntimeError(f"Could not read any bake vector property from {property_names}")


def color_to_tuple(value):
    return (
        float(getattr(value, "r", 0.0)),
        float(getattr(value, "g", 0.0)),
        float(getattr(value, "b", 0.0)),
    )


def get_material_vector_tuple(material, parameter_name):
    value = unreal.MaterialEditingLibrary.get_material_instance_vector_parameter_value(material, parameter_name)
    if value is None:
        raise RuntimeError(f"Material {material.get_name()} has no vector parameter {parameter_name}")
    return color_to_tuple(value)


def get_bake_bounds(data_asset, material):
    try:
        min_bbox = get_material_vector_tuple(material, "MinBBox")
        size_bbox = get_material_vector_tuple(material, "SizeBBox")
        if sum(abs(component) for component in size_bbox) > 0.0001:
            return min_bbox, size_bbox
    except Exception as exc:
        warn(f"Could not read material bake bounds from {material.get_name()}: {exc}")
    min_bbox = vec_to_tuple(get_bake_vector(data_asset, ("vertex_min_bbox", "vertex_min_b_box")))
    size_bbox = vec_to_tuple(get_bake_vector(data_asset, ("vertex_size_bbox", "vertex_size_b_box")))
    return min_bbox, size_bbox


def make_mesh_import_options():
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
        warn(f"Could not set skeletal FBX import type: {exc}")
    try:
        import_data = unreal.FbxSkeletalMeshImportData()
        options.set_editor_property("skeletal_mesh_import_data", import_data)
        import_data.set_editor_property("import_meshes_in_bone_hierarchy", True)
        import_data.set_editor_property("convert_scene", True)
        import_data.set_editor_property("force_front_x_axis", False)
        import_data.set_editor_property("import_uniform_scale", 1.0)
    except Exception as exc:
        warn(f"Could not set all skeletal import data fields: {exc}")
    return options


def make_animation_import_options(skeleton):
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
        warn(f"Could not set animation FBX import type: {exc}")
    try:
        anim_data = unreal.FbxAnimSequenceImportData()
        options.set_editor_property("anim_sequence_import_data", anim_data)
        anim_data.set_editor_property("animation_length", unreal.FBXAnimationLengthImportType.FBXALIT_EXPORTED_TIME)
        anim_data.set_editor_property("import_bone_tracks", True)
        anim_data.set_editor_property("use_default_sample_rate", True)
        anim_data.set_editor_property("snap_to_closest_frame_boundary", True)
    except Exception as exc:
        warn(f"Could not set all animation import data fields: {exc}")
    return options


def import_fbx(source, dest_dir, dest_name, options):
    if not Path(source).is_file():
        raise RuntimeError(f"Missing FBX source: {source}")
    ensure_directory(dest_dir)
    task = unreal.AssetImportTask()
    task.set_editor_property("automated", True)
    task.set_editor_property("save", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("replace_existing_settings", False)
    task.set_editor_property("filename", str(source))
    task.set_editor_property("destination_path", dest_dir)
    task.set_editor_property("destination_name", dest_name)
    task.set_editor_property("options", options)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    asset = unreal.EditorAssetLibrary.load_asset(object_path(f"{dest_dir}/{dest_name}"))
    if not asset:
        raise RuntimeError(f"FBX import did not produce {dest_dir}/{dest_name}. Imported={list(task.imported_object_paths or [])}")
    unreal.EditorAssetLibrary.save_loaded_asset(asset)
    return asset, list(task.imported_object_paths or [])


def get_mesh_skeleton(mesh):
    skeleton = mesh.get_editor_property("skeleton")
    if not skeleton:
        raise RuntimeError(f"Imported mesh has no skeleton: {mesh.get_path_name()}")
    return skeleton


def create_texture_duplicate(dest_path, source_path):
    existing = unreal.EditorAssetLibrary.load_asset(dest_path)
    if existing:
        return existing
    texture = unreal.EditorAssetLibrary.duplicate_asset(source_path, dest_path)
    if not texture:
        raise RuntimeError(f"Could not duplicate {source_path} to {dest_path}")
    return texture


def create_data_asset_duplicate(dest_path):
    existing = unreal.EditorAssetLibrary.load_asset(dest_path)
    if existing:
        return existing
    data_asset = unreal.EditorAssetLibrary.duplicate_asset(SAMPLE_DATA_ASSET, dest_path)
    if not data_asset:
        raise RuntimeError(f"Could not duplicate {SAMPLE_DATA_ASSET} to {dest_path}")
    return data_asset


def create_or_load_material_instance(dest_path, parent):
    existing = unreal.EditorAssetLibrary.load_asset(dest_path)
    if existing:
        material = existing
    else:
        package_path, asset_name = dest_path.rsplit("/", 1)
        material = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
            asset_name,
            package_path,
            unreal.MaterialInstanceConstant,
            unreal.MaterialInstanceConstantFactoryNew(),
        )
        if not material:
            raise RuntimeError(f"Could not create material instance {dest_path}")
    unreal.MaterialEditingLibrary.set_material_instance_parent(material, parent)
    return material


def ensure_master_material():
    existing = unreal.EditorAssetLibrary.load_asset(MASTER_MATERIAL_PATH)
    if existing and isinstance(existing, unreal.Material):
        unreal.EditorAssetLibrary.delete_asset(MASTER_MATERIAL_PATH)

    ensure_directory(MASTER_MATERIAL_DIR)
    material = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        MASTER_MATERIAL_NAME,
        MASTER_MATERIAL_DIR,
        unreal.Material,
        unreal.MaterialFactoryNew(),
    )
    if not material:
        raise RuntimeError(f"Could not create {MASTER_MATERIAL_PATH}")

    mel = unreal.MaterialEditingLibrary
    safe_set(material, "shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    safe_set(material, "blend_mode", unreal.BlendMode.BLEND_OPAQUE)
    safe_set(material, "two_sided", True)
    safe_set(material, "used_with_instanced_static_meshes", True)
    safe_set(material, "used_with_nanite", True)

    base_tex = mel.create_material_expression(material, unreal.MaterialExpressionTextureSampleParameter2D, -760, -180)
    tint = mel.create_material_expression(material, unreal.MaterialExpressionVectorParameter, -760, 20)
    brightness = mel.create_material_expression(material, unreal.MaterialExpressionScalarParameter, -760, 220)
    tint_mul = mel.create_material_expression(material, unreal.MaterialExpressionMultiply, -470, -80)
    bright_mul = mel.create_material_expression(material, unreal.MaterialExpressionMultiply, -180, -80)

    uv1 = mel.create_material_expression(material, unreal.MaterialExpressionTextureCoordinate, -760, 520)
    position_tex = mel.create_material_expression(material, unreal.MaterialExpressionTextureObjectParameter, -760, 700)
    frame = mel.create_material_expression(material, unreal.MaterialExpressionScalarParameter, -760, 880)
    rows = mel.create_material_expression(material, unreal.MaterialExpressionScalarParameter, -760, 1040)
    min_bbox = mel.create_material_expression(material, unreal.MaterialExpressionVectorParameter, -760, 1200)
    size_bbox = mel.create_material_expression(material, unreal.MaterialExpressionVectorParameter, -760, 1360)
    custom = mel.create_material_expression(material, unreal.MaterialExpressionCustom, -260, 860)

    base_tex.set_editor_property("parameter_name", "BaseColorTexture")
    try:
        base_tex.set_editor_property("texture", load_asset("/Engine/EngineResources/WhiteSquareTexture.WhiteSquareTexture"))
    except Exception:
        pass
    tint.set_editor_property("parameter_name", "Tint")
    tint.set_editor_property("default_value", unreal.LinearColor(1.0, 1.0, 1.0, 1.0))
    brightness.set_editor_property("parameter_name", "Brightness")
    brightness.set_editor_property("default_value", 0.8)

    uv1.set_editor_property("coordinate_index", 2)
    position_tex.set_editor_property("parameter_name", "PositionTexture")
    position_tex.set_editor_property("texture", load_asset(SAMPLE_POSITION_TEXTURE))
    frame.set_editor_property("parameter_name", "Frame")
    frame.set_editor_property("default_value", 0.0)
    rows.set_editor_property("parameter_name", "RowsPerFrame")
    rows.set_editor_property("default_value", 1.0)
    min_bbox.set_editor_property("parameter_name", "MinBBox")
    min_bbox.set_editor_property("default_value", unreal.LinearColor(0.0, 0.0, 0.0, 0.0))
    size_bbox.set_editor_property("parameter_name", "SizeBBox")
    size_bbox.set_editor_property("default_value", unreal.LinearColor(1.0, 1.0, 1.0, 0.0))

    custom.set_editor_property("description", "EasyMobVAT_WPO")
    custom.set_editor_property("output_type", unreal.CustomMaterialOutputType.CMOT_FLOAT3)
    custom.set_editor_property("code", "\n".join([
        "uint tex_width;",
        "uint tex_height;",
        "PositionTexture.GetDimensions(tex_width, tex_height);",
        "float frame_index = floor(Frame + 0.0001);",
        "float2 sample_uv = UV1;",
        "sample_uv.y += (frame_index * RowsPerFrame) / max(1.0, (float)tex_height);",
        "float3 packed_delta = Texture2DSample(PositionTexture, PositionTextureSampler, sample_uv).rgb;",
        "float3 local_delta = packed_delta * SizeBBox.rgb + MinBBox.rgb;",
        "return TransformLocalVectorToWorld(Parameters, local_delta);",
    ]))
    inputs = []
    for input_name in ("UV1", "PositionTexture", "Frame", "RowsPerFrame", "MinBBox", "SizeBBox"):
        custom_input = unreal.CustomInput()
        custom_input.set_editor_property("input_name", input_name)
        inputs.append(custom_input)
    custom.set_editor_property("inputs", inputs)

    mel.connect_material_expressions(base_tex, "RGB", tint_mul, "A")
    mel.connect_material_expressions(tint, "", tint_mul, "B")
    mel.connect_material_expressions(tint_mul, "", bright_mul, "A")
    mel.connect_material_expressions(brightness, "", bright_mul, "B")
    mel.connect_material_property(bright_mul, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    try:
        mel.connect_material_property(bright_mul, "", unreal.MaterialProperty.MP_BASE_COLOR)
    except Exception:
        pass

    mel.connect_material_expressions(uv1, "", custom, "UV1")
    mel.connect_material_expressions(position_tex, "", custom, "PositionTexture")
    mel.connect_material_expressions(frame, "", custom, "Frame")
    mel.connect_material_expressions(rows, "", custom, "RowsPerFrame")
    mel.connect_material_expressions(min_bbox, "", custom, "MinBBox")
    mel.connect_material_expressions(size_bbox, "", custom, "SizeBBox")
    mel.connect_material_property(custom, "", unreal.MaterialProperty.MP_WORLD_POSITION_OFFSET)

    try:
        mel.layout_material_expressions(material)
    except Exception:
        pass
    mel.recompile_material(material)
    unreal.EditorAssetLibrary.save_asset(MASTER_MATERIAL_PATH)
    return material


def make_anim_sequence_infos(anim_assets):
    infos = []
    for clip in CLIPS:
        info = unreal.AnimToTextureAnimSequenceInfo()
        info.set_editor_property("enabled", True)
        info.set_editor_property("anim_sequence", anim_assets[clip])
        info.set_editor_property("use_custom_range", False)
        infos.append(info)
    return infos


def get_animation_ranges(data_asset, manifest_clip_frames):
    ranges = {}
    try:
        baked_ranges = list(data_asset.get_editor_property("animations"))
    except Exception:
        baked_ranges = []
    for index, clip in enumerate(CLIPS):
        if index < len(baked_ranges):
            item = baked_ranges[index]
            ranges[clip] = (
                int(item.get_editor_property("start_frame")),
                int(item.get_editor_property("end_frame")),
            )
        else:
            start = sum(int(manifest_clip_frames[name]) for name in CLIPS[:index])
            end = start + max(1, int(manifest_clip_frames[clip])) - 1
            ranges[clip] = (start, end)
    return ranges


def configure_texture(texture, srgb=False):
    safe_set(texture, "srgb", bool(srgb))
    safe_set(texture, "never_stream", True)
    try:
        texture.set_editor_property("filter", unreal.TextureFilter.TF_NEAREST)
    except Exception:
        pass
    unreal.EditorAssetLibrary.save_loaded_asset(texture)


def ensure_static_mesh_can_write_vat_uv(static_mesh, lod_index, target_uv_channel):
    """AnimToTexture can only insert the target channel if earlier channels exist."""
    subsystem = unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)
    current = int(subsystem.get_num_uv_channels(static_mesh, lod_index))
    required_channel_count = target_uv_channel + 1
    while current < required_channel_count:
        subsystem.add_uv_channel(static_mesh, lod_index)
        next_count = int(subsystem.get_num_uv_channels(static_mesh, lod_index))
        if next_count <= current:
            raise RuntimeError(
                f"Could not add intermediate UV channel before VAT bake; "
                f"mesh={static_mesh.get_path_name()} lod={lod_index} count={current}"
            )
        current = next_count
    unreal.EditorAssetLibrary.save_loaded_asset(static_mesh)


def configure_material_instance(material, base_texture, data_asset, position_texture, normal_texture):
    mel = unreal.MaterialEditingLibrary
    for param_name in ("BaseColorTexture", "EmissiveTexture", "DiffuseColorMap"):
        try:
            mel.set_material_instance_texture_parameter_value(material, param_name, base_texture)
        except Exception:
            pass
    for param_name, texture in (("PositionTexture", position_texture), ("NormalTexture", normal_texture)):
        try:
            mel.set_material_instance_texture_parameter_value(material, param_name, texture)
        except Exception:
            pass
    try:
        unreal.AnimToTextureBPLibrary.update_material_instance_from_data_asset(data_asset, material)
    except Exception as exc:
        warn(f"AnimToTexture material update skipped for {material.get_name()}: {exc}")
    try:
        mel.set_material_instance_scalar_parameter_value(material, "Brightness", 0.8)
        mel.set_material_instance_vector_parameter_value(material, "Tint", unreal.LinearColor(1.0, 1.0, 1.0, 1.0))
        mel.update_material_instance(material)
    except Exception:
        pass
    unreal.EditorAssetLibrary.save_loaded_asset(material)


def bake_mob(mob, visual_row, master_material, clip_frames):
    enemy_id = mob["enemy_id"]
    dest_dir = f"{DEST_ROOT}/{enemy_id}"
    source_dir = f"{dest_dir}/BakeSource"
    ensure_directory(dest_dir)
    ensure_directory(source_dir)
    log(f"Importing {enemy_id}")

    skeletal_name = f"SKM_EasyMobVAT_{enemy_id}"
    skeletal_mesh, mesh_imported = import_fbx(mob["mesh_export"], source_dir, skeletal_name, make_mesh_import_options())
    skeleton = get_mesh_skeleton(skeletal_mesh)

    anim_assets = {}
    anim_imports = {}
    for clip in CLIPS:
        anim_name = f"AM_EasyMobVAT_{enemy_id}_{clip}"
        anim_asset, imported = import_fbx(mob["action_exports"][clip], source_dir, anim_name, make_animation_import_options(skeleton))
        anim_assets[clip] = anim_asset
        anim_imports[clip] = imported

    static_package = f"{dest_dir}/SM_EasyMobVAT_{enemy_id}"
    static_mesh = unreal.EditorAssetLibrary.load_asset(object_path(static_package))
    if not static_mesh:
        static_mesh = unreal.AnimToTextureBPLibrary.convert_skeletal_mesh_to_static_mesh(skeletal_mesh, static_package, 0)
        if not static_mesh:
            raise RuntimeError(f"ConvertSkeletalMeshToStaticMesh failed for {enemy_id}")
    unreal.EditorAssetLibrary.save_loaded_asset(static_mesh)

    position_path = f"{dest_dir}/TX_EasyMobVAT_{enemy_id}_Position"
    normal_path = f"{dest_dir}/TX_EasyMobVAT_{enemy_id}_Normal"
    position_texture = create_texture_duplicate(position_path, SAMPLE_POSITION_TEXTURE)
    normal_texture = create_texture_duplicate(normal_path, SAMPLE_NORMAL_TEXTURE)
    configure_texture(position_texture, srgb=False)
    configure_texture(normal_texture, srgb=False)

    data_asset_path = f"{dest_dir}/DA_EasyMobVAT_{enemy_id}"
    data_asset = create_data_asset_duplicate(data_asset_path)
    data_asset.set_editor_property("mode", unreal.AnimToTextureMode.VERTEX)
    data_asset.set_editor_property("precision", unreal.AnimToTexturePrecision.SIXTEEN_BITS)
    data_asset.set_editor_property("skeletal_mesh", skeletal_mesh)
    data_asset.set_editor_property("skeletal_lod_index", 0)
    data_asset.set_editor_property("static_mesh", static_mesh)
    data_asset.set_editor_property("static_lod_index", 0)
    data_asset.set_editor_property("uv_channel", 2)
    data_asset.set_editor_property("max_width", 4096)
    data_asset.set_editor_property("max_height", 4096)
    data_asset.set_editor_property("enforce_power_of_two", False)
    data_asset.set_editor_property("sample_rate", 30.0)
    data_asset.set_editor_property("auto_play", False)
    data_asset.set_editor_property("frame", 0)
    data_asset.set_editor_property("vertex_position_texture", position_texture)
    data_asset.set_editor_property("vertex_normal_texture", normal_texture)
    data_asset.set_editor_property("anim_sequences", make_anim_sequence_infos(anim_assets))
    unreal.EditorAssetLibrary.save_loaded_asset(data_asset)

    ensure_static_mesh_can_write_vat_uv(static_mesh, 0, 2)
    if not unreal.AnimToTextureBPLibrary.animation_to_texture(data_asset):
        raise RuntimeError(f"AnimToTexture bake failed for {enemy_id}")

    material_path = f"{dest_dir}/MI_EasyMobVAT_{enemy_id}"
    material = create_or_load_material_instance(material_path, master_material)
    base_texture = load_asset(visual_row["PixelatedTextureAssetPath"])
    configure_material_instance(material, base_texture, data_asset, position_texture, normal_texture)

    for asset in (static_mesh, position_texture, normal_texture, data_asset, material):
        unreal.EditorAssetLibrary.save_loaded_asset(asset)
    unreal.EditorAssetLibrary.save_directory(dest_dir, only_if_is_dirty=False, recursive=True)

    ranges = get_animation_ranges(data_asset, clip_frames)
    min_bbox, size_bbox = get_bake_bounds(data_asset, material)

    row = {
        "---": enemy_id,
        "EnemyID": enemy_id,
        "StaticMesh": object_path(static_package),
        "Material": object_path(material_path),
        "PixelatedTextureAssetPath": visual_row["PixelatedTextureAssetPath"],
        "PositionTexture": object_path(position_path),
        "NormalTexture": object_path(normal_path),
        "MeshRelativeLocation": visual_row["MeshRelativeLocation"],
        "MeshRelativeRotation": visual_row["MeshRelativeRotation"],
        "MeshRelativeScale": visual_row["MeshRelativeScale"],
        "bEnabled": "true",
        "SampleRate": "30.000000",
        "NumFrames": str(int(data_asset.get_editor_property("num_frames"))),
        "RowsPerFrame": str(int(data_asset.get_editor_property("vertex_rows_per_frame"))),
        "MinBBox": make_vector_csv(min_bbox),
        "SizeBBox": make_vector_csv(size_bbox),
    }
    for clip in CLIPS:
        start, end = ranges[clip]
        row[f"{clip}StartFrame"] = str(start)
        row[f"{clip}EndFrame"] = str(end)
        row[f"{clip}PlayRate"] = "1.000000"

    return {
        "enemy_id": enemy_id,
        "skeletal_mesh": skeletal_mesh.get_path_name(),
        "static_mesh": static_mesh.get_path_name(),
        "position_texture": position_texture.get_path_name(),
        "normal_texture": normal_texture.get_path_name(),
        "data_asset": data_asset.get_path_name(),
        "material": material.get_path_name(),
        "num_frames": row["NumFrames"],
        "rows_per_frame": row["RowsPerFrame"],
        "ranges": ranges,
        "mesh_imported": mesh_imported,
        "anim_imports": anim_imports,
    }, row


def write_csv(rows):
    MOB_VAT_CSV.parent.mkdir(parents=True, exist_ok=True)
    with MOB_VAT_CSV.open("w", encoding="utf-8", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=CSV_FIELDS, extrasaction="ignore", quoting=csv.QUOTE_ALL)
        writer.writeheader()
        for row in rows:
            writer.writerow(row)


def main():
    log("=== Easy mob VAT import start ===")
    manifest = load_manifest()
    visual_rows = read_character_visual_rows()
    scan_anim_to_texture_content()
    master_material = ensure_master_material()

    import_report = []
    csv_rows = []
    for mob in manifest["mobs"]:
        enemy_id = mob["enemy_id"]
        visual_row = visual_rows.get(enemy_id)
        if not visual_row:
            raise RuntimeError(f"CharacterVisuals.csv has no row for {enemy_id}")
        report, row = bake_mob(mob, visual_row, master_material, manifest["clips"])
        import_report.append(report)
        csv_rows.append(row)

    write_csv(csv_rows)
    SetupMobVertexAnimationsDataTable.main()

    REPORT_PATH.parent.mkdir(parents=True, exist_ok=True)
    REPORT_PATH.write_text(json.dumps({
        "run_root": str(RUN_ROOT),
        "manifest": str(MANIFEST_PATH),
        "csv": str(MOB_VAT_CSV),
        "master_material": object_path(MASTER_MATERIAL_PATH),
        "mobs": import_report,
    }, indent=2, sort_keys=True), encoding="utf-8")
    log(f"Wrote {REPORT_PATH}")
    log("=== Easy mob VAT import done ===")


if __name__ == "__main__":
    main()
