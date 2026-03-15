import json
import os

import unreal


REPORT_PATH = os.path.join(
    unreal.SystemLibrary.get_project_directory(),
    "Saved",
    "Logs",
    "InspectImportLightingIssues.json",
)

KNOWN_TEXTURE_PARAMS = [
    "BaseColorTexture",
    "DiffuseColorMap",
    "NormalTexture",
    "NormalMap",
    "MetallicRoughnessTexture",
    "OcclusionTexture",
    "EmissiveTexture",
    "OpacityTexture",
]

ASSETS_TO_INSPECT = [
    "/Game/World/Props/Rock.Rock",
    "/Game/World/Props/Tree2.Tree2",
    "/Game/World/LootBags/Black/SM_LootBag_Black.SM_LootBag_Black",
    "/Game/Characters/Enemies/GoblinThief/Black/BlackGoblinRun.BlackGoblinRun",
    "/Game/Materials/M_GLB_Unlit.M_GLB_Unlit",
    "/Game/Materials/M_Character_Unlit.M_Character_Unlit",
]


def _asset_path(obj):
    if not obj:
        return None
    try:
        return obj.get_path_name()
    except Exception:
        return None


def _texture_info(texture):
    if not texture:
        return None
    info = {
        "path": _asset_path(texture),
        "name": texture.get_name(),
    }
    for prop in [
        "srgb",
        "compression_settings",
        "virtual_texture_streaming",
        "lod_group",
        "mip_gen_settings",
        "filter",
        "never_stream",
    ]:
        try:
            value = texture.get_editor_property(prop)
            info[prop] = str(value)
        except Exception:
            pass
    for name, getter in [
        ("size_x", "blueprint_get_size_x"),
        ("size_y", "blueprint_get_size_y"),
    ]:
        try:
            info[name] = int(getattr(texture, getter)())
        except Exception:
            pass
    return info


def _inspect_material_interface(mat):
    info = {
        "path": _asset_path(mat),
        "name": mat.get_name() if mat else None,
        "class": type(mat).__name__ if mat else None,
    }
    if not mat:
        return info

    try:
        parent = mat.get_editor_property("parent")
        info["parent"] = _asset_path(parent)
    except Exception:
        pass

    texture_params = {}
    for param in KNOWN_TEXTURE_PARAMS:
        try:
            tex = unreal.MaterialEditingLibrary.get_material_instance_texture_parameter_value(mat, param)
            if isinstance(tex, (tuple, list)):
                tex = tex[-1] if tex else None
        except Exception:
            tex = None
        if tex:
            texture_params[param] = _texture_info(tex)
    if texture_params:
        info["texture_parameters"] = texture_params

    try:
        vector_values = mat.get_editor_property("vector_parameter_values")
        if vector_values:
            vectors = {}
            for item in vector_values:
                try:
                    name = str(item.parameter_info.name)
                    value = item.parameter_value
                    vectors[name] = str(value)
                except Exception:
                    continue
            if vectors:
                info["vector_parameters"] = vectors
    except Exception:
        pass

    try:
        scalar_values = mat.get_editor_property("scalar_parameter_values")
        if scalar_values:
            scalars = {}
            for item in scalar_values:
                try:
                    name = str(item.parameter_info.name)
                    scalars[name] = float(item.parameter_value)
                except Exception:
                    continue
            if scalars:
                info["scalar_parameters"] = scalars
    except Exception:
        pass

    return info


def _inspect_material(mat):
    info = _inspect_material_interface(mat)
    if not mat or not isinstance(mat, unreal.Material):
        return info

    for prop in [
        "blend_mode",
        "two_sided",
        "used_with_skeletal_mesh",
        "used_with_instanced_static_meshes",
        "dithered_lod_transition",
    ]:
        try:
            info[prop] = str(mat.get_editor_property(prop))
        except Exception:
            pass
    return info


def _inspect_mesh(mesh):
    info = {
        "path": _asset_path(mesh),
        "name": mesh.get_name() if mesh else None,
        "class": type(mesh).__name__ if mesh else None,
    }
    if not mesh:
        return info

    try:
        bounds = mesh.get_bounds()
        info["bounds"] = {
            "origin": [bounds.origin.x, bounds.origin.y, bounds.origin.z],
            "extent": [bounds.box_extent.x, bounds.box_extent.y, bounds.box_extent.z],
            "sphere_radius": bounds.sphere_radius,
        }
    except Exception:
        pass

    slots = []
    try:
        static_slots = mesh.get_editor_property("static_materials")
        for index, slot in enumerate(static_slots):
            material = None
            slot_name = None
            try:
                material = slot.get_editor_property("material_interface")
            except Exception:
                pass
            try:
                slot_name = str(slot.get_editor_property("material_slot_name"))
            except Exception:
                slot_name = str(index)
            slots.append({
                "index": index,
                "slot_name": slot_name,
                "material": _inspect_material_interface(material),
            })
    except Exception:
        pass

    if not slots:
        try:
            skeletal_slots = mesh.get_editor_property("materials")
            for index, slot in enumerate(skeletal_slots):
                material = None
                slot_name = None
                try:
                    material = slot.get_editor_property("material_interface")
                except Exception:
                    pass
                try:
                    slot_name = str(slot.get_editor_property("material_slot_name"))
                except Exception:
                    slot_name = str(index)
                slots.append({
                    "index": index,
                    "slot_name": slot_name,
                    "material": _inspect_material_interface(material),
                })
        except Exception:
            pass

    if not slots:
        try:
            materials = list(mesh.materials or [])
            for index, material in enumerate(materials):
                slots.append({
                    "index": index,
                    "slot_name": str(index),
                    "material": _inspect_material_interface(material),
                })
        except Exception:
            pass

    info["material_slots"] = slots
    return info


def _inspect_asset(asset_path):
    asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    if not asset:
        return {"path": asset_path, "missing": True}
    if isinstance(asset, unreal.StaticMesh) or isinstance(asset, unreal.SkeletalMesh):
        return _inspect_mesh(asset)
    if isinstance(asset, unreal.Material):
        return _inspect_material(asset)
    if isinstance(asset, unreal.MaterialInterface):
        return _inspect_material_interface(asset)
    return {
        "path": asset_path,
        "name": asset.get_name(),
        "class": type(asset).__name__,
    }


def main():
    report = {
        "assets": [_inspect_asset(path) for path in ASSETS_TO_INSPECT],
    }

    os.makedirs(os.path.dirname(REPORT_PATH), exist_ok=True)
    with open(REPORT_PATH, "w", encoding="utf-8") as handle:
        json.dump(report, handle, indent=2)

    unreal.log(f"[InspectImportLightingIssues] Wrote {REPORT_PATH}")


if __name__ == "__main__":
    main()
