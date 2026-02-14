"""
Add emissive fill to all character material instances (heroes + companions).

Sets EmissiveColorMap = DiffuseColorMap and EmissiveColorMapWeight = 0.12 on each
material instance that uses FBXLegacyPhongSurfaceMaterial as parent.
This ensures characters have a minimum brightness regardless of scene lighting,
replicating the "always lit" look of asset preview thumbnails.

Run INSIDE Unreal Editor: Tools -> Execute Python Script -> Scripts/AddEmissiveFillToCharacterMaterials.py
"""

import unreal


# Emissive weight: 0.0 = no self-illumination, 1.0 = fully emissive.
# 0.12 gives a subtle fill that prevents pitch-black shadows without looking glowing.
EMISSIVE_WEIGHT = 0.12

# Character content directories to scan for skeletal meshes with material instances.
CHARACTER_DIRS = [
    "/Game/Characters/Heroes/",
    "/Game/Characters/Companions/",
    "/Game/Characters/NPCs/",
]


def _get_material_instances_from_skeletal_mesh(mesh_path: str):
    """Load a skeletal mesh and return its material interface list."""
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


def _apply_emissive_fill(mat_instance):
    """
    On a MaterialInstanceConstant with FBXLegacyPhongSurfaceMaterial parent,
    copy DiffuseColorMap to EmissiveColorMap and set EmissiveColorMapWeight.
    """
    mat_name = mat_instance.get_name()
    mat_path = mat_instance.get_path_name()

    # Read current DiffuseColorMap texture parameter
    diffuse_tex = None
    success, diffuse_tex = unreal.MaterialEditingLibrary.get_material_instance_texture_parameter_value(
        mat_instance, "DiffuseColorMap"
    )
    if not success or not diffuse_tex:
        unreal.log_warning(f"  [{mat_name}] No DiffuseColorMap found, skipping emissive fill")
        return False

    # Check current EmissiveColorMapWeight
    success_w, current_weight = unreal.MaterialEditingLibrary.get_material_instance_scalar_parameter_value(
        mat_instance, "EmissiveColorMapWeight"
    )

    if success_w and abs(current_weight - EMISSIVE_WEIGHT) < 0.001:
        unreal.log(f"  [{mat_name}] Already has emissive fill ({current_weight:.3f}), skipping")
        return False

    # Set EmissiveColorMap = DiffuseColorMap (same texture for self-illumination)
    unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(
        mat_instance, "EmissiveColorMap", diffuse_tex
    )

    # Set EmissiveColorMapWeight for subtle fill
    unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(
        mat_instance, "EmissiveColorMapWeight", EMISSIVE_WEIGHT
    )

    # Save the modified material instance
    unreal.EditorAssetLibrary.save_asset(mat_path)
    unreal.log(f"  [{mat_name}] Set emissive fill: weight={EMISSIVE_WEIGHT}, texture={diffuse_tex.get_name()}")
    return True


def main():
    unreal.log("=" * 60)
    unreal.log("ADD EMISSIVE FILL TO CHARACTER MATERIALS")
    unreal.log("=" * 60)

    total_updated = 0
    total_scanned = 0

    for char_dir in CHARACTER_DIRS:
        if not unreal.EditorAssetLibrary.does_directory_exist(char_dir):
            unreal.log(f"[Skip] Directory not found: {char_dir}")
            continue

        unreal.log(f"\nScanning {char_dir} ...")

        # Find all SkeletalMesh assets in this directory tree
        asset_paths = unreal.EditorAssetLibrary.list_assets(char_dir, recursive=True, include_folder=False)
        for asset_path in asset_paths:
            asset = unreal.EditorAssetLibrary.load_asset(asset_path)
            if not asset or not isinstance(asset, unreal.SkeletalMesh):
                continue

            mesh_name = asset.get_name()
            materials = _get_material_instances_from_skeletal_mesh(asset_path)
            if not materials:
                continue

            unreal.log(f"\n  Mesh: {mesh_name} ({len(materials)} material instance(s))")
            total_scanned += len(materials)

            for mat in materials:
                if _apply_emissive_fill(mat):
                    total_updated += 1

    unreal.log("")
    unreal.log("=" * 60)
    unreal.log(f"DONE: Updated {total_updated} / {total_scanned} material instances")
    unreal.log("=" * 60)


if __name__ == "__main__":
    main()
