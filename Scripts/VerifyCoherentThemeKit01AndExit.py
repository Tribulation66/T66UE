"""
Verify CoherentThemeKit01 generated wall/floor meshes, material instances, and
runtime bounds, then request editor shutdown.
"""

import os
import sys
import unreal


SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
if SCRIPT_DIR not in sys.path:
    sys.path.append(SCRIPT_DIR)

import ImportStaticMeshes


DEST_DIR = ImportStaticMeshes.COHERENT_THEME_KIT_DEST
EXPECTED_PARENT = "/Game/Materials/M_Environment_Unlit"
MODULE_IDS = ImportStaticMeshes.COHERENT_THEME_KIT_MODULES


def _vector_size(bounds):
    extent = bounds.box_extent
    return unreal.Vector(extent.x * 2.0, extent.y * 2.0, extent.z * 2.0)


def _path(asset_name):
    return f"{DEST_DIR}/{asset_name}"


def _parent_path(material):
    try:
        parent = material.get_editor_property("parent")
        return parent.get_path_name().split(".")[0] if parent else ""
    except Exception:
        return ""


def main():
    mesh_count = 0
    material_count = 0
    texture_count = 0
    missing = []
    bad_materials = []
    wall_sizes = []
    floor_sizes = []

    for module_id in MODULE_IDS:
        asset_name = f"{module_id}_UnrealReady"
        mesh = unreal.EditorAssetLibrary.load_asset(_path(asset_name))
        material = unreal.EditorAssetLibrary.load_asset(_path(f"{asset_name}_Mat_00"))
        texture = unreal.EditorAssetLibrary.load_asset(_path(f"{asset_name}_BaseColor_00"))

        if not mesh or not isinstance(mesh, unreal.StaticMesh):
            missing.append(asset_name)
            continue
        mesh_count += 1

        if material and isinstance(material, unreal.MaterialInstanceConstant):
            material_count += 1
            if _parent_path(material) != EXPECTED_PARENT:
                bad_materials.append(material.get_path_name())
        else:
            missing.append(f"{asset_name}_Mat_00")

        if texture and isinstance(texture, unreal.Texture2D):
            texture_count += 1
        else:
            missing.append(f"{asset_name}_BaseColor_00")

        size = _vector_size(mesh.get_bounds())
        if "Wall" in module_id:
            wall_sizes.append(size)
        else:
            floor_sizes.append(size)

    def _range_text(values, attr):
        if not values:
            return "none"
        nums = [getattr(v, attr) for v in values]
        return f"{min(nums):.1f}-{max(nums):.1f}"

    unreal.log(
        "[VerifyCoherentThemeKit01] "
        f"meshes={mesh_count}/40 materials={material_count}/40 textures={texture_count}/40 "
        f"wall_cm_x={_range_text(wall_sizes, 'x')} wall_cm_y={_range_text(wall_sizes, 'y')} "
        f"wall_cm_z={_range_text(wall_sizes, 'z')} "
        f"floor_cm_x={_range_text(floor_sizes, 'x')} floor_cm_y={_range_text(floor_sizes, 'y')} "
        f"floor_cm_z={_range_text(floor_sizes, 'z')}"
    )

    if missing:
        unreal.log_error(f"[VerifyCoherentThemeKit01] missing={missing}")
    if bad_materials:
        unreal.log_error(f"[VerifyCoherentThemeKit01] bad_material_parents={bad_materials}")
    if not missing and not bad_materials:
        unreal.log("[VerifyCoherentThemeKit01] ok=true")

    world = None
    try:
        subsystem = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
        if subsystem:
            world = subsystem.get_editor_world()
    except Exception:
        pass

    try:
        unreal.SystemLibrary.execute_console_command(world, "QUIT_EDITOR")
        unreal.log("[VerifyCoherentThemeKit01] QUIT_EDITOR requested")
    except Exception as exc:
        unreal.log_warning(f"[VerifyCoherentThemeKit01] Failed to request QUIT_EDITOR: {exc}")


if __name__ == "__main__":
    main()
