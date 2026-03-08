"""
Batch-convert character MaterialInstanceConstants to use M_Character_Unlit.

For each character MIC with FBXLegacyPhongSurfaceMaterial parent:
  1. Read the current diffuse texture (DiffuseColorMap / BaseColorMap / directory fallback)
  2. Change the parent material to M_Character_Unlit
  3. Set the DiffuseColorMap texture parameter on the reparented MIC
  4. Save the asset

Run INSIDE Unreal Editor:
  Tools -> Execute Python Script -> Scripts/ConvertCharacterMaterialsToUnlit.py

IMPORTANT: Run Scripts/CreateCharacterUnlitMaterial.py FIRST to create M_Character_Unlit.
"""

import unreal

UNLIT_MATERIAL_PATH = "/Game/Materials/M_Character_Unlit"
LOG_PREFIX = "[UnlitConvert]"

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


def _is_supported_fbx_material_instance(mat_instance) -> bool:
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
    return "FBXLegacyPhongSurfaceMaterial" in parent_name


def _is_already_unlit(mat_instance) -> bool:
    """Check if the MIC already uses M_Character_Unlit as parent."""
    try:
        parent = mat_instance.get_editor_property("parent")
    except Exception:
        return False
    if parent:
        try:
            return parent.get_path_name().startswith(UNLIT_MATERIAL_PATH)
        except Exception:
            pass
    return False


def _get_first_texture_param(mat_instance, candidate_names):
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


def _convert_to_unlit(mat_instance, unlit_parent):
    mat_name = mat_instance.get_name()
    mat_path = mat_instance.get_path_name()

    if _is_already_unlit(mat_instance):
        unreal.log(f"  [{mat_name}] Already using M_Character_Unlit, skipping")
        return False

    if not _is_supported_fbx_material_instance(mat_instance):
        unreal.log(f"  [{mat_name}] Not an FBX legacy material, skipping")
        return False

    # Step 1: Read the diffuse texture BEFORE changing the parent
    diffuse_param_name, diffuse_tex = _get_first_texture_param(
        mat_instance, ["DiffuseColorMap", "BaseColorMap", "BaseColorTexture"]
    )
    source_desc = diffuse_param_name or ""

    if not diffuse_tex:
        diffuse_tex = _find_texture_in_directory(mat_path)
        if diffuse_tex:
            source_desc = f"(directory: {diffuse_tex.get_name()})"
        else:
            unreal.log_warning(
                f"  [{mat_name}] No diffuse texture found, skipping"
            )
            return False

    tex_name = diffuse_tex.get_name() if diffuse_tex else "None"

    # Step 2: Change the parent material to M_Character_Unlit
    try:
        mat_instance.set_editor_property("parent", unlit_parent)
    except Exception as e:
        unreal.log_error(f"  [{mat_name}] Failed to set parent: {e}")
        return False

    # Step 3: Set the DiffuseColorMap texture parameter on the reparented MIC
    try:
        unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(
            mat_instance, "DiffuseColorMap", diffuse_tex
        )
    except Exception as e:
        unreal.log_error(f"  [{mat_name}] Failed to set DiffuseColorMap: {e}")
        return False

    # Step 4: Save
    try:
        unreal.EditorAssetLibrary.save_asset(mat_path)
    except Exception as e:
        unreal.log_error(f"  [{mat_name}] Failed to save: {e}")
        return False

    unreal.log(
        f"  [{mat_name}] Converted to Unlit — texture={tex_name} (from {source_desc})"
    )
    return True


def main():
    unreal.log("=" * 60)
    unreal.log(f"{LOG_PREFIX} ConvertCharacterMaterialsToUnlit START")
    unreal.log("=" * 60)

    # Load the Unlit master material
    unlit_parent = unreal.EditorAssetLibrary.load_asset(UNLIT_MATERIAL_PATH)
    if not unlit_parent:
        unreal.log_error(
            f"{LOG_PREFIX} M_Character_Unlit not found at {UNLIT_MATERIAL_PATH}. "
            "Run CreateCharacterUnlitMaterial.py first!"
        )
        return

    unreal.log(f"{LOG_PREFIX} Loaded master material: {UNLIT_MATERIAL_PATH}")

    total_converted = 0
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
                if _convert_to_unlit(mat, unlit_parent):
                    total_converted += 1

    unreal.log("")
    unreal.log("=" * 60)
    unreal.log(f"{LOG_PREFIX} DONE: Converted {total_converted} / {total_scanned} material instances to Unlit")
    unreal.log("=" * 60)


if __name__ == "__main__":
    main()
