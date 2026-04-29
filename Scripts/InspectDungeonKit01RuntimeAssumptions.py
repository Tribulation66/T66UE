"""
Print generated DungeonKit01 mesh bounds and material/collision assumptions.
This is a diagnostic helper for runtime placement issues.
"""

import unreal


MESH_PATHS = (
    "/Game/World/Terrain/TowerDungeon/GeneratedKit/DungeonKit01/DungeonWall_Straight_A_UnrealReady",
    "/Game/World/Terrain/TowerDungeon/GeneratedKit/DungeonKit01/DungeonWall_Straight_Chains_UnrealReady",
    "/Game/World/Terrain/TowerDungeon/GeneratedKit/DungeonKit01/DungeonWall_Straight_BonesNiche_UnrealReady",
    "/Game/World/Terrain/TowerDungeon/GeneratedKit/DungeonKit01/DungeonWall_Doorway_Arch_UnrealReady",
    "/Game/World/Terrain/TowerDungeon/GeneratedKit/DungeonKit01/DungeonFloor_BonesDrain_A_UnrealReady",
)


def _vector_text(v):
    return f"({v.x:.3f}, {v.y:.3f}, {v.z:.3f})"


def main():
    for mesh_path in MESH_PATHS:
        mesh = unreal.EditorAssetLibrary.load_asset(mesh_path)
        if not mesh or not isinstance(mesh, unreal.StaticMesh):
            unreal.log_error(f"[InspectDungeonKit01Runtime] MISSING {mesh_path}")
            continue

        bounds = mesh.get_bounds()
        origin = bounds.origin
        extent = bounds.box_extent
        size = unreal.Vector(extent.x * 2.0, extent.y * 2.0, extent.z * 2.0)

        body_setup = None
        collision_flag = None
        try:
            body_setup = mesh.get_editor_property("body_setup")
            if body_setup:
                collision_flag = body_setup.get_editor_property("collision_trace_flag")
        except Exception as exc:
            collision_flag = f"ERROR {exc}"

        materials = []
        try:
            for index in range(mesh.get_num_sections(0)):
                material = mesh.get_material(index)
                materials.append(material.get_path_name() if material else "None")
        except Exception:
            try:
                materials.append(mesh.get_material(0).get_path_name())
            except Exception:
                materials.append("UNKNOWN")

        unreal.log(
            "[InspectDungeonKit01Runtime] "
            f"{mesh_path} origin={_vector_text(origin)} extent={_vector_text(extent)} "
            f"size={_vector_text(size)} collision_trace_flag={collision_flag} "
            f"materials={materials}"
        )

    try:
        unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")
        unreal.log("[InspectDungeonKit01Runtime] QUIT_EDITOR requested")
    except Exception as exc:
        unreal.log_warning(f"[InspectDungeonKit01Runtime] Failed to request QUIT_EDITOR: {exc}")


if __name__ == "__main__":
    main()
