"""
Revert emissive fill on character material instances.

Sets EmissiveColorMapWeight back to 0.0 on all character MICs that were
modified by AddEmissiveFillToCharacterMaterials.py.

Run INSIDE Unreal Editor:
  Tools -> Execute Python Script -> Scripts/RevertEmissiveFillOnCharacterMaterials.py
"""

import unreal

LOG_PREFIX = "[RevertEmissive]"

CHARACTER_DIRS = [
    "/Game/Characters/Heroes/",
    "/Game/Characters/Companions/",
    "/Game/Characters/Enemies/",
    "/Game/Characters/NPCs/",
]


def _get_material_instances_from_skeletal_mesh(mesh_path: str):
    mesh = unreal.EditorAssetLibrary.load_asset(mesh_path)
    if not mesh or not isinstance(mesh, unreal.SkeletalMesh):
        return []
    materials = mesh.get_editor_property("materials")
    if not materials:
        return []
    result = []
    for mat_slot in materials:
        if not mat_slot:
            continue
        mat = mat_slot.get_editor_property("material_interface")
        if mat and isinstance(mat, unreal.MaterialInstanceConstant):
            result.append(mat)
    return result


def _revert_emissive(mat_instance):
    mat_name = mat_instance.get_name()
    mat_path = mat_instance.get_path_name()

    try:
        current_weight = unreal.MaterialEditingLibrary.get_material_instance_scalar_parameter_value(
            mat_instance, "EmissiveColorMapWeight"
        )
        if isinstance(current_weight, (tuple, list)):
            current_weight = current_weight[-1]
        if isinstance(current_weight, (int, float)) and abs(current_weight) < 0.001:
            return False
    except Exception:
        return False

    try:
        unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(
            mat_instance, "EmissiveColorMapWeight", 0.0
        )
        unreal.EditorAssetLibrary.save_asset(mat_path)
        unreal.log(f"  [{mat_name}] EmissiveColorMapWeight reset to 0.0")
        return True
    except Exception as e:
        unreal.log_warning(f"  [{mat_name}] Failed to revert: {e}")
        return False


def main():
    unreal.log("=" * 60)
    unreal.log(f"{LOG_PREFIX} RevertEmissiveFillOnCharacterMaterials START")
    unreal.log("=" * 60)

    total_reverted = 0
    total_scanned = 0
    seen_material_paths = set()

    for char_dir in CHARACTER_DIRS:
        if not unreal.EditorAssetLibrary.does_directory_exist(char_dir):
            unreal.log(f"[Skip] Directory not found: {char_dir}")
            continue

        unreal.log(f"\nScanning {char_dir} ...")

        asset_paths = unreal.EditorAssetLibrary.list_assets(
            char_dir, recursive=True, include_folder=False
        )
        for asset_path in asset_paths:
            asset = unreal.EditorAssetLibrary.load_asset(asset_path)
            if not asset or not isinstance(asset, unreal.SkeletalMesh):
                continue

            materials = _get_material_instances_from_skeletal_mesh(asset_path)
            for mat in materials:
                mat_path = mat.get_path_name()
                if mat_path in seen_material_paths:
                    continue
                seen_material_paths.add(mat_path)
                total_scanned += 1
                if _revert_emissive(mat):
                    total_reverted += 1

    unreal.log("")
    unreal.log("=" * 60)
    unreal.log(f"{LOG_PREFIX} DONE: Reverted {total_reverted} / {total_scanned} material instances")
    unreal.log("=" * 60)


if __name__ == "__main__":
    main()
