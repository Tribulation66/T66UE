"""
Shared Unreal Editor helpers for QuadRetro character assets.

These helpers are intentionally small and import-safe so commandlet scripts and
future import scripts can reuse the same texture, LOD, and material defaults.
"""

import os

import unreal


CHARACTER_ROOT = "/Game/Characters"
SHARED_MI_DIR = "/Game/Materials"
SHARED_MI_NAME = "MI_GLB_Unlit_Character_Shared"
SHARED_MI_PATH = f"{SHARED_MI_DIR}/{SHARED_MI_NAME}"
GLB_MASTER_PATH = "/Game/Materials/M_GLB_Unlit"
LEGACY_MI_DIR = "/Game/Characters/_Legacy/MaterialInstances_QuadRetro"

TEXTURE_PARAMETER_NAMES = (
    "EmissiveTexture",
    "BaseColorTexture",
    "DiffuseColorMap",
)

LOD_SPECS = (
    {"percent_triangles": 1.00, "screen_size": 1.00},
    {"percent_triangles": 0.40, "screen_size": 0.60},
    {"percent_triangles": 0.15, "screen_size": 0.25},
    {"percent_triangles": 0.05, "screen_size": 0.10},
)


def game_path_to_uasset_path(asset_path):
    package_path = str(asset_path).split(".", 1)[0]
    if not package_path.startswith("/Game/"):
        return None
    content_dir = unreal.SystemLibrary.get_project_content_directory()
    rel_path = package_path[len("/Game/") :].replace("/", os.sep)
    return os.path.join(content_dir, rel_path + ".uasset")


def safe_save(asset, asset_path=None):
    try:
        return bool(unreal.EditorAssetLibrary.save_loaded_asset(asset))
    except Exception:
        if asset_path:
            try:
                return bool(unreal.EditorAssetLibrary.save_asset(asset_path))
            except Exception:
                return False
    return False


def enum_value(enum_type_name, candidates):
    enum_type = getattr(unreal, enum_type_name, None)
    if not enum_type:
        return None
    for candidate in candidates:
        value = getattr(enum_type, candidate, None)
        if value is not None:
            return value
    return None


def enum_value_from_property(obj, prop_name, candidates):
    try:
        enum_type = type(obj.get_editor_property(prop_name))
    except Exception:
        return None
    for candidate in candidates:
        value = getattr(enum_type, candidate, None)
        if value is not None:
            return value
    return None


def set_first_editor_property(obj, names, value):
    errors = []
    for name in names:
        try:
            current = obj.get_editor_property(name)
            if current == value:
                return name, False
            obj.set_editor_property(name, value)
            return name, True
        except Exception as exc:
            errors.append(f"{name}: {exc}")
    raise RuntimeError("; ".join(errors))


def get_texture_streaming_method(texture):
    try:
        return str(texture.get_texture_streaming_method())
    except Exception:
        pass
    for prop_name in ("texture_streaming_method", "streaming_method"):
        try:
            return str(texture.get_editor_property(prop_name))
        except Exception:
            continue
    return ""


def is_effectively_streamed(texture):
    method = get_texture_streaming_method(texture).upper()
    return "STREAMED" in method and "NOT" not in method and "VIRTUAL" not in method


def texture_probe(texture):
    probe = {
        "size_x": str(texture.blueprint_get_size_x()),
        "size_y": str(texture.blueprint_get_size_y()),
        "lod_group": str(texture.get_editor_property("lod_group")),
        "texture_streaming_method": get_texture_streaming_method(texture),
    }
    try:
        probe["is_possible_to_stream"] = str(texture.is_possible_to_stream())
    except Exception:
        pass
    for prop_name in (
        "never_stream",
        "mip_gen_settings",
        "filter",
        "power_of_two_mode",
        "virtual_texture_streaming",
        "max_texture_size",
        "lod_bias",
    ):
        try:
            probe[prop_name] = str(texture.get_editor_property(prop_name))
        except Exception:
            pass
    return probe


def apply_character_texture_defaults(texture):
    if not texture or not isinstance(texture, unreal.Texture2D):
        return {"ok": False, "error": "asset is not a Texture2D"}

    result = {
        "asset": texture.get_path_name(),
        "changed": False,
        "ok": False,
        "before": texture_probe(texture),
    }

    character_group = enum_value(
        "TextureGroup",
        ("TEXTUREGROUP_CHARACTER", "TEXTUREGROUP_Character"),
    )
    if character_group is None:
        character_group = enum_value_from_property(
            texture,
            "lod_group",
            ("TEXTUREGROUP_CHARACTER", "TEXTUREGROUP_Character"),
        )
    if character_group is None:
        raise RuntimeError("TextureGroup character enum is unavailable")

    _, lod_changed = set_first_editor_property(texture, ("lod_group",), character_group)
    changed = bool(lod_changed)

    nearest_filter = enum_value(
        "TextureFilter",
        ("TF_NEAREST", "TF_Nearest"),
    )
    if nearest_filter is not None:
        try:
            current = texture.get_editor_property("filter")
            if current != nearest_filter:
                texture.set_editor_property("filter", nearest_filter)
                changed = True
        except Exception:
            pass

    default_streaming_method = enum_value(
        "TextureStreamingMethod",
        ("TSM_DEFAULT", "TSM_Default", "TSM_STREAMED", "TSM_Streamed"),
    )
    if default_streaming_method is not None:
        try:
            current = texture.get_editor_property("texture_streaming_method")
            if current != default_streaming_method:
                texture.set_editor_property("texture_streaming_method", default_streaming_method)
                changed = True
        except Exception:
            pass

    for prop_name, desired in (("never_stream", False), ("virtual_texture_streaming", False)):
        try:
            current = texture.get_editor_property(prop_name)
            if current != desired:
                texture.set_editor_property(prop_name, desired)
                changed = True
        except Exception:
            pass

    mip_from_group = enum_value(
        "TextureMipGenSettings",
        ("TMGS_FROM_TEXTURE_GROUP", "TMGS_FromTextureGroup"),
    )
    if mip_from_group is not None:
        try:
            current = texture.get_editor_property("mip_gen_settings")
            if str(current).upper().endswith("NO_MIPMAPS") or "NO_MIPMAPS" in str(current).upper():
                texture.set_editor_property("mip_gen_settings", mip_from_group)
                changed = True
        except Exception:
            pass

    result["changed"] = changed

    try:
        texture.post_edit_change()
    except Exception:
        pass

    result["after"] = texture_probe(texture)
    result["effective_streamed"] = is_effectively_streamed(texture)
    result["streaming_note"] = ""
    if not result["effective_streamed"]:
        result["streaming_note"] = (
            "UE reports this Texture2D as non-streamed after defaults; this commonly "
            "happens for single-mip or non-power-of-two textures. The requested "
            "character LOD group and streaming eligibility flags were still applied."
        )
    result["ok"] = (
        "CHARACTER" in result["after"]["lod_group"].upper()
        and result["after"].get("never_stream", "").upper() == "FALSE"
        and "NO_MIPMAPS" not in result["after"].get("mip_gen_settings", "").upper()
        and (nearest_filter is None or "NEAREST" in result["after"].get("filter", "").upper())
    )
    return result


def load_shared_quadretro_material():
    return unreal.EditorAssetLibrary.load_asset(SHARED_MI_PATH)


def ensure_shared_quadretro_material():
    master = unreal.EditorAssetLibrary.load_asset(GLB_MASTER_PATH)
    if not master:
        raise RuntimeError(f"missing master material: {GLB_MASTER_PATH}")

    asset = unreal.EditorAssetLibrary.load_asset(SHARED_MI_PATH)
    if not asset:
        if not unreal.EditorAssetLibrary.does_directory_exist(SHARED_MI_DIR):
            unreal.EditorAssetLibrary.make_directory(SHARED_MI_DIR)
        factory = unreal.MaterialInstanceConstantFactoryNew()
        asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
            SHARED_MI_NAME,
            SHARED_MI_DIR,
            unreal.MaterialInstanceConstant,
            factory,
        )
        if not asset:
            raise RuntimeError(f"failed to create {SHARED_MI_PATH}")

    try:
        asset.set_editor_property("parent", master)
    except Exception as exc:
        raise RuntimeError(f"failed to parent {SHARED_MI_PATH} to {GLB_MASTER_PATH}: {exc}") from exc

    mel = unreal.MaterialEditingLibrary
    mel.set_material_instance_scalar_parameter_value(asset, "Brightness", 1.0)
    mel.set_material_instance_vector_parameter_value(asset, "EmissiveFactor", unreal.LinearColor(1.0, 1.0, 1.0, 1.0))
    mel.set_material_instance_vector_parameter_value(asset, "BaseColorFactor", unreal.LinearColor(0.0, 0.0, 0.0, 1.0))
    mel.set_material_instance_vector_parameter_value(asset, "Tint", unreal.LinearColor(1.0, 1.0, 1.0, 1.0))
    safe_save(asset, SHARED_MI_PATH)
    return asset


def assign_shared_material_to_mesh(mesh, shared_material=None):
    if not mesh or not isinstance(mesh, unreal.StaticMesh):
        return {"ok": False, "error": "asset is not a StaticMesh"}

    material = shared_material or ensure_shared_quadretro_material()
    slots = list(mesh.get_editor_property("static_materials") or [])
    if not slots:
        return {"ok": False, "error": "StaticMesh has no material slots"}

    before = []
    changed = False
    mesh.modify()
    for index, slot in enumerate(slots):
        current = None
        try:
            current = slot.get_editor_property("material_interface")
        except Exception:
            pass
        before.append(current.get_path_name() if current else "")
        if current == material:
            continue
        try:
            slot.set_editor_property("material_interface", material)
        except Exception:
            mesh.set_material(index, material)
        changed = True

    try:
        mesh.set_editor_property("static_materials", slots)
    except Exception:
        pass
    try:
        mesh.post_edit_change()
    except Exception:
        pass

    after = []
    for slot in list(mesh.get_editor_property("static_materials") or []):
        current = None
        try:
            current = slot.get_editor_property("material_interface")
        except Exception:
            pass
        after.append(current.get_path_name() if current else "")

    return {
        "ok": all(path == material.get_path_name() for path in after),
        "changed": changed,
        "before_materials": before,
        "after_materials": after,
    }


def assign_shared_material_to_mesh_path(mesh_path, shared_material=None):
    mesh = unreal.EditorAssetLibrary.load_asset(mesh_path)
    result = assign_shared_material_to_mesh(mesh, shared_material)
    if mesh and result.get("ok"):
        result["saved"] = safe_save(mesh, mesh_path)
    return result


def find_texture_on_material(material):
    if not material:
        return None

    mel = unreal.MaterialEditingLibrary
    for param_name in TEXTURE_PARAMETER_NAMES:
        try:
            texture = mel.get_material_instance_texture_parameter_value(material, param_name)
            if isinstance(texture, (tuple, list)):
                texture = texture[-1] if texture else None
            if texture and isinstance(texture, unreal.Texture2D):
                texture_path = texture.get_path_name()
                if texture_path.startswith("/Engine/EngineResources/DefaultTexture."):
                    continue
                return texture
        except Exception:
            continue
    return None


def find_pixelated_texture_for_mesh(mesh):
    if not mesh or not isinstance(mesh, unreal.StaticMesh):
        return None

    for slot in list(mesh.get_editor_property("static_materials") or []):
        material = None
        try:
            material = slot.get_editor_property("material_interface")
        except Exception:
            pass
        texture = find_texture_on_material(material)
        if texture:
            return texture

    mesh_package = mesh.get_path_name().split(".", 1)[0]
    mesh_dir = mesh_package.rsplit("/", 1)[0]
    if unreal.EditorAssetLibrary.does_directory_exist(mesh_dir):
        candidates = []
        for asset_path in unreal.EditorAssetLibrary.list_assets(mesh_dir, recursive=True, include_folder=False) or []:
            asset = unreal.EditorAssetLibrary.load_asset(asset_path)
            if asset and isinstance(asset, unreal.Texture2D):
                score = 0 if "Pixelated_512" in asset.get_name() else 1
                candidates.append((score, asset.get_name(), asset))
        if candidates:
            candidates.sort(key=lambda item: (item[0], item[1]))
            return candidates[0][2]

    return None


def lod_count(mesh, subsystem=None):
    for func in (
        lambda: int(mesh.get_num_lods()),
        lambda: int(subsystem.get_lod_count(mesh)) if subsystem else 0,
        lambda: int(unreal.EditorStaticMeshLibrary.get_lod_count(mesh)),
    ):
        try:
            count = func()
            if count > 0:
                return count
        except Exception:
            continue
    return 0


def triangle_count(mesh, lod_index):
    for name in ("get_num_triangles", "get_number_triangles", "get_triangle_count"):
        func = getattr(mesh, name, None)
        if not func:
            continue
        try:
            return int(func(lod_index))
        except Exception:
            continue
    for name in ("get_number_triangles", "get_num_triangles", "get_triangle_count", "get_lod_triangles"):
        func = getattr(unreal.EditorStaticMeshLibrary, name, None)
        if not func:
            continue
        try:
            return int(func(mesh, lod_index))
        except Exception:
            continue
    return None


def make_lod_options():
    if not hasattr(unreal, "StaticMeshReductionOptions"):
        raise RuntimeError("unreal.StaticMeshReductionOptions is unavailable")
    if not hasattr(unreal, "StaticMeshReductionSettings"):
        raise RuntimeError("unreal.StaticMeshReductionSettings is unavailable")

    settings = []
    for spec in LOD_SPECS:
        reduction = unreal.StaticMeshReductionSettings()
        reduction.set_editor_property("percent_triangles", float(spec["percent_triangles"]))
        reduction.set_editor_property("screen_size", float(spec["screen_size"]))
        settings.append(reduction)

    options = unreal.StaticMeshReductionOptions()
    for prop_name in ("auto_compute_lod_screen_size", "b_auto_compute_lod_screen_size"):
        try:
            options.set_editor_property(prop_name, False)
            break
        except Exception:
            continue
    options.set_editor_property("reduction_settings", settings)
    return options


def apply_lod_ladder_to_mesh(mesh, subsystem=None):
    if not mesh or not isinstance(mesh, unreal.StaticMesh):
        return {"ok": False, "error": "asset is not a StaticMesh"}

    if not subsystem:
        try:
            subsystem = unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)
        except Exception:
            subsystem = None

    result = {
        "asset": mesh.get_path_name(),
        "before_lod_count": lod_count(mesh, subsystem),
        "triangles_before": [
            triangle_count(mesh, index) for index in range(max(1, lod_count(mesh, subsystem)))
        ],
        "spec": list(LOD_SPECS),
        "ok": False,
    }

    if result["before_lod_count"] >= 4:
        result.update(
            {
                "returned_lod_count": result["before_lod_count"],
                "after_lod_count": result["before_lod_count"],
                "triangles_after": [
                    triangle_count(mesh, index) for index in range(max(1, result["before_lod_count"]))
                ],
                "already_ready": True,
                "ok": True,
            }
        )
        return result

    if not subsystem or not hasattr(subsystem, "set_lods"):
        result["error"] = "StaticMeshEditorSubsystem.set_lods unavailable"
        return result

    options = make_lod_options()
    returned_count = subsystem.set_lods(mesh, options)
    try:
        mesh.post_edit_change()
    except Exception:
        pass

    after_count = lod_count(mesh, subsystem)
    result.update(
        {
            "returned_lod_count": int(returned_count),
            "after_lod_count": int(after_count),
            "triangles_after": [triangle_count(mesh, index) for index in range(max(1, after_count))],
            "ok": after_count >= 4,
        }
    )
    return result


def apply_lod_ladder_to_mesh_path(mesh_path, subsystem=None):
    mesh = unreal.EditorAssetLibrary.load_asset(mesh_path)
    result = apply_lod_ladder_to_mesh(mesh, subsystem)
    if mesh and result.get("ok"):
        result["saved"] = safe_save(mesh, mesh_path)
    return result


def is_quadretro_static_mesh_path(asset_path):
    text = str(asset_path)
    leaf = str(asset_path).split("/", 1)[-1]
    asset_name = leaf.rsplit("/", 1)[-1].split(".", 1)[0]
    if text.startswith("/Game/Characters/Mobs/") and asset_name.startswith("SM_"):
        return True
    return text.startswith(CHARACTER_ROOT) and asset_name.startswith("SM_") and asset_name.endswith("_QuadRetro")
