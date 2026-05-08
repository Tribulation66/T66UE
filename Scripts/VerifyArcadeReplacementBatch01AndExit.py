"""
Verify ArcadeReplacementBatch01 imported meshes/materials/textures and remove
old canonical chest texture leftovers only when they are unreferenced.
"""

import json
import os
import unreal


EXPECTED = {
    "GamblerDemonStand": {
        "mesh": "/Game/Characters/NPCs/Gambler/GamblerDemonStand/GamblerDemonStand",
        "material": "/Game/Characters/NPCs/Gambler/GamblerDemonStand/Materials/M_GamblerDemonStand",
        "texture": "/Game/Characters/NPCs/Gambler/GamblerDemonStand/Textures/GamblerDemonStand_BaseColor_00",
    },
    "ArcadeMachine": {
        "mesh": "/Game/World/Interactables/ArcadeMachine/ArcadeMachine",
        "material": "/Game/World/Interactables/ArcadeMachine/Materials/M_ArcadeMachine",
        "texture": "/Game/World/Interactables/ArcadeMachine/Textures/ArcadeMachine_BaseColor_00",
    },
    "ArcadeAmplifierPickup": {
        "mesh": "/Game/World/Interactables/ArcadeAmplifierPickup/ArcadeAmplifierPickup",
        "material": "/Game/World/Interactables/ArcadeAmplifierPickup/Materials/M_ArcadeAmplifierPickup",
        "texture": "/Game/World/Interactables/ArcadeAmplifierPickup/Textures/ArcadeAmplifierPickup_BaseColor_00",
    },
    "ArcadeAmplifierPickup_Charged": {
        "mesh": "/Game/World/Interactables/ArcadeAmplifierPickup/ArcadeAmplifierPickup_Charged",
        "material": "/Game/World/Interactables/ArcadeAmplifierPickup/Materials/M_ArcadeAmplifierPickup_Charged",
        "texture": "/Game/World/Interactables/ArcadeAmplifierPickup/Textures/ArcadeAmplifierPickup_Charged_BaseColor_00",
    },
    "Chest": {
        "mesh": "/Game/World/Interactables/Chests/ChestModel/Chest",
        "material": "/Game/World/Interactables/Chests/ChestModel/Materials/M_Chest",
        "texture": "/Game/World/Interactables/Chests/ChestModel/Textures/Chest_BaseColor_00",
    },
}

EXPECTED_PARENT = "/Game/Materials/M_Environment_Unlit.M_Environment_Unlit"
OLD_CHEST_TEXTURES = (
    "/Game/World/Interactables/Chests/ChestModel/Textures/T_Chest_BaseColor",
    "/Game/World/Interactables/Chests/ChestModel/Textures/T_Chest_Detail",
)


def _load(path):
    return unreal.EditorAssetLibrary.load_asset(path)


def _path_name(asset):
    return asset.get_path_name() if asset else ""


def _texture_param(material, name):
    try:
        value = unreal.MaterialEditingLibrary.get_material_instance_texture_parameter_value(material, name)
    except Exception:
        return None
    if isinstance(value, (tuple, list)):
        return value[-1] if value else None
    return value


def _referencers(path):
    try:
        return list(unreal.EditorAssetLibrary.find_package_referencers_for_asset(path, True) or [])
    except Exception as exc:
        return [f"referencer lookup failed: {exc}"]


def _report_path():
    project_dir = unreal.SystemLibrary.get_project_directory()
    audit_dir = os.path.join(project_dir, "Saved", "Audits")
    os.makedirs(audit_dir, exist_ok=True)
    return os.path.join(audit_dir, "ArcadeReplacementBatch01Verify.json")


def main():
    errors = []
    report = {
        "assets": {},
        "old_chest_textures": {},
    }

    for name, spec in EXPECTED.items():
        mesh = _load(spec["mesh"])
        material = _load(spec["material"])
        texture = _load(spec["texture"])
        asset_report = {
            "mesh": _path_name(mesh),
            "material": _path_name(material),
            "texture": _path_name(texture),
            "material_parent": "",
            "diffuse_texture": "",
            "base_color_texture": "",
        }

        if not mesh or not isinstance(mesh, unreal.StaticMesh):
            errors.append(f"{name}: missing StaticMesh {spec['mesh']}")
        if not material or not isinstance(material, unreal.MaterialInstanceConstant):
            errors.append(f"{name}: missing MaterialInstanceConstant {spec['material']}")
        if not texture or not isinstance(texture, unreal.Texture2D):
            errors.append(f"{name}: missing Texture2D {spec['texture']}")

        if material and isinstance(material, unreal.MaterialInstanceConstant):
            parent = None
            try:
                parent = material.get_editor_property("parent")
            except Exception:
                parent = None
            asset_report["material_parent"] = _path_name(parent)
            if _path_name(parent) != EXPECTED_PARENT:
                errors.append(f"{name}: material parent is {_path_name(parent)}")

            diffuse = _texture_param(material, "DiffuseColorMap")
            base_color = _texture_param(material, "BaseColorTexture")
            asset_report["diffuse_texture"] = _path_name(diffuse)
            asset_report["base_color_texture"] = _path_name(base_color)
            expected_texture = spec["texture"] + "." + spec["texture"].rsplit("/", 1)[-1]
            if _path_name(diffuse) != expected_texture:
                errors.append(f"{name}: DiffuseColorMap is {_path_name(diffuse)}")
            if _path_name(base_color) != expected_texture:
                errors.append(f"{name}: BaseColorTexture is {_path_name(base_color)}")

        report["assets"][name] = asset_report

    for old_path in OLD_CHEST_TEXTURES:
        refs = _referencers(old_path)
        deleted = False
        if unreal.EditorAssetLibrary.does_asset_exist(old_path) and not refs:
            deleted = bool(unreal.EditorAssetLibrary.delete_asset(old_path))
        report["old_chest_textures"][old_path] = {
            "referencers": refs,
            "deleted": deleted,
            "exists_after": unreal.EditorAssetLibrary.does_asset_exist(old_path),
        }

    report["errors"] = errors
    with open(_report_path(), "w", encoding="utf-8") as handle:
        json.dump(report, handle, indent=2)

    if errors:
        raise RuntimeError("; ".join(errors))

    unreal.log("[VerifyArcadeReplacementBatch01] OK")
    try:
        unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")
    except Exception:
        pass


if __name__ == "__main__":
    main()
