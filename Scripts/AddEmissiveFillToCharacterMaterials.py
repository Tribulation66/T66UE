"""
Add emissive fill to imported character material instances.

Targets imported FBX/Interchange MaterialInstanceConstants used by heroes,
companions, enemies, and NPCs. For supported materials it copies the diffuse
texture into the emissive slot and applies a small emissive weight so imported
characters stop rendering overly dark in preview/gameplay.

Run INSIDE Unreal Editor:
  Tools -> Execute Python Script -> Scripts/AddEmissiveFillToCharacterMaterials.py
"""

import unreal


# Emissive weight: 0.0 = no self-illumination, 1.0 = fully emissive.
# 0.12 gives a subtle fill that prevents pitch-black shadows without looking glowing.
EMISSIVE_WEIGHT = 0.16

# Character content directories to scan for skeletal meshes with material instances.
CHARACTER_DIRS = [
    "/Game/Characters/Heroes/",
    "/Game/Characters/Companions/",
    "/Game/Characters/Enemies/",
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


def _is_supported_fbx_material_instance(mat_instance) -> bool:
    """Restrict changes to imported FBX material instances."""
    if not mat_instance or not isinstance(mat_instance, unreal.MaterialInstanceConstant):
        return False

    try:
        parent = mat_instance.get_editor_property("parent")
    except Exception:
        parent = None

    parent_name = ""
    if parent:
        try:
            parent_name = parent.get_name()
        except Exception:
            parent_name = ""

    # Imported character assets in this project use FBXLegacyPhongSurfaceMaterial
    # or children derived from it. Avoid touching authored project materials.
    return "FBXLegacyPhongSurfaceMaterial" in parent_name


def _get_first_texture_param(mat_instance, candidate_names):
    """Return the first existing texture parameter value from the candidates."""
    for param_name in candidate_names:
        try:
            result = unreal.MaterialEditingLibrary.get_material_instance_texture_parameter_value(
                mat_instance, param_name
            )
            if isinstance(result, (tuple, list)):
                tex = result[-1] if len(result) > 1 else result[0]
            else:
                tex = result
            if tex and isinstance(tex, unreal.Texture):
                return param_name, tex
        except Exception:
            pass
    return None, None


def _find_texture_in_directory(mat_path):
    """Find a Texture2D asset in the same directory as the material instance."""
    package_path = mat_path.split(".")[0]
    dir_path = package_path.rsplit("/", 1)[0]
    try:
        asset_paths = unreal.EditorAssetLibrary.list_assets(
            dir_path, recursive=False, include_folder=False
        )
    except Exception:
        return None
    for p in asset_paths:
        asset = unreal.EditorAssetLibrary.load_asset(p)
        if asset and isinstance(asset, unreal.Texture2D):
            return asset
    return None


def _apply_emissive_fill(mat_instance):
    """
    On a MaterialInstanceConstant with FBXLegacyPhongSurfaceMaterial parent,
    copy the diffuse texture into EmissiveColorMap and set EmissiveColorMapWeight.

    Two strategies to find the diffuse texture:
      1. Read it from a known material parameter (DiffuseColorMap etc.)
      2. Fall back to the first Texture2D in the same content directory
         (handles Interchange-imported materials that don't populate params)
    """
    mat_name = mat_instance.get_name()
    mat_path = mat_instance.get_path_name()

    if not _is_supported_fbx_material_instance(mat_instance):
        unreal.log(f"  [{mat_name}] Not an imported FBX legacy material, skipping")
        return False

    diffuse_param_name, diffuse_tex = _get_first_texture_param(
        mat_instance, ["DiffuseColorMap", "BaseColorMap", "BaseColorTexture"]
    )

    if not diffuse_tex:
        diffuse_tex = _find_texture_in_directory(mat_path)
        if diffuse_tex:
            diffuse_param_name = f"(directory: {diffuse_tex.get_name()})"
        else:
            unreal.log_warning(
                f"  [{mat_name}] No diffuse texture param or directory texture found, skipping"
            )
            return False

    # Check current EmissiveColorMapWeight
    try:
        current_weight = unreal.MaterialEditingLibrary.get_material_instance_scalar_parameter_value(
            mat_instance, "EmissiveColorMapWeight"
        )
        if isinstance(current_weight, (tuple, list)):
            current_weight = current_weight[-1]
        if isinstance(current_weight, (int, float)) and abs(current_weight - EMISSIVE_WEIGHT) < 0.001:
            unreal.log(f"  [{mat_name}] Already has emissive fill ({current_weight:.3f}), skipping")
            return False
    except Exception:
        pass

    # Set EmissiveColorMap = diffuse map (same texture for self-illumination)
    unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(
        mat_instance, "EmissiveColorMap", diffuse_tex
    )

    # Set EmissiveColorMapWeight for subtle fill
    unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(
        mat_instance, "EmissiveColorMapWeight", EMISSIVE_WEIGHT
    )

    # Save the modified material instance
    unreal.EditorAssetLibrary.save_asset(mat_path)
    unreal.log(
        f"  [{mat_name}] Set emissive fill: weight={EMISSIVE_WEIGHT}, "
        f"source_param={diffuse_param_name}, texture={diffuse_tex.get_name()}"
    )
    return True


def main():
    unreal.log("=" * 60)
    unreal.log("ADD EMISSIVE FILL TO CHARACTER MATERIALS")
    unreal.log("=" * 60)

    total_updated = 0
    total_scanned = 0
    seen_material_paths = set()

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

            for mat in materials:
                mat_path = mat.get_path_name()
                if mat_path in seen_material_paths:
                    continue
                seen_material_paths.add(mat_path)
                total_scanned += 1
                if _apply_emissive_fill(mat):
                    total_updated += 1

    unreal.log("")
    unreal.log("=" * 60)
    unreal.log(f"DONE: Updated {total_updated} / {total_scanned} material instances")
    unreal.log("=" * 60)


if __name__ == "__main__":
    main()
