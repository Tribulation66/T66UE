"""
Verify generated DungeonKit01 wall/floor visual meshes are not imported with
complex-as-simple static mesh collision, then request editor shutdown.
"""

import unreal


DEST_DIR = "/Game/World/Terrain/TowerDungeon/GeneratedKit/DungeonKit01"
MESH_NAMES = (
    "DungeonWall_Straight_A_UnrealReady",
    "DungeonWall_Straight_Chains_UnrealReady",
    "DungeonWall_Straight_BonesNiche_UnrealReady",
    "DungeonWall_Doorway_Arch_UnrealReady",
    "DungeonFloor_BonesDrain_A_UnrealReady",
)


def _expected_flag():
    try:
        return unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE
    except Exception:
        return None


def main():
    complex_as_simple = _expected_flag()
    all_ok = complex_as_simple is not None
    for mesh_name in MESH_NAMES:
        mesh_path = f"{DEST_DIR}/{mesh_name}"
        mesh = unreal.EditorAssetLibrary.load_asset(mesh_path)
        if not mesh or not isinstance(mesh, unreal.StaticMesh):
            unreal.log_error(f"[VerifyDungeonKit01Collision] MISSING {mesh_path}")
            all_ok = False
            continue

        body_setup = None
        try:
            body_setup = mesh.get_editor_property("body_setup")
        except Exception as exc:
            unreal.log_error(f"[VerifyDungeonKit01Collision] body_setup error {mesh_path}: {exc}")
            all_ok = False
            continue

        if not body_setup:
            unreal.log_error(f"[VerifyDungeonKit01Collision] NO_BODY_SETUP {mesh_path}")
            all_ok = False
            continue

        try:
            flag = body_setup.get_editor_property("collision_trace_flag")
        except Exception as exc:
            unreal.log_error(f"[VerifyDungeonKit01Collision] flag error {mesh_path}: {exc}")
            all_ok = False
            continue

        ok = complex_as_simple is not None and flag != complex_as_simple
        unreal.log(f"[VerifyDungeonKit01Collision] {'OK' if ok else 'BAD'} {mesh_path} collision_trace_flag={flag}")
        all_ok = all_ok and ok

    unreal.log(f"[VerifyDungeonKit01Collision] all_visual_mesh_collision_lightweight_ok={all_ok}")

    world = None
    try:
        subsystem = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
        if subsystem:
            world = subsystem.get_editor_world()
    except Exception:
        pass

    try:
        unreal.SystemLibrary.execute_console_command(world, "QUIT_EDITOR")
        unreal.log("[VerifyDungeonKit01Collision] QUIT_EDITOR requested")
    except Exception as exc:
        unreal.log_warning(f"[VerifyDungeonKit01Collision] Failed to request QUIT_EDITOR: {exc}")


if __name__ == "__main__":
    main()
