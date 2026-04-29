"""
Verify generated DungeonKit01 materials are parented to the project unlit
environment master material, then request editor shutdown.
"""

import unreal


DEST_DIR = "/Game/World/Terrain/TowerDungeon/GeneratedKit/DungeonKit01"
EXPECTED_PARENT = "/Game/Materials/M_Environment_Unlit.M_Environment_Unlit"


def main():
    expected = unreal.EditorAssetLibrary.load_asset(EXPECTED_PARENT)
    if not expected:
        unreal.log_error(f"[VerifyDungeonKit01Materials] Missing expected parent: {EXPECTED_PARENT}")
        return

    parent_unlit = False
    try:
        parent_unlit = expected.get_editor_property("shading_model") == unreal.MaterialShadingModel.MSM_UNLIT
    except Exception as exc:
        unreal.log_warning(f"[VerifyDungeonKit01Materials] Could not read parent shading model: {exc}")
    unreal.log(f"[VerifyDungeonKit01Materials] parent_unlit={parent_unlit} parent={EXPECTED_PARENT}")

    all_ok = parent_unlit
    for asset_path in unreal.EditorAssetLibrary.list_assets(DEST_DIR, recursive=False, include_folder=False):
        asset = unreal.EditorAssetLibrary.load_asset(asset_path)
        if not asset or not isinstance(asset, unreal.MaterialInstanceConstant):
            continue

        parent = None
        try:
            parent = asset.get_editor_property("parent")
        except Exception:
            pass

        parent_path = parent.get_path_name() if parent else "<none>"
        ok = parent_path == EXPECTED_PARENT
        all_ok = all_ok and ok
        status = "OK" if ok else "BAD"
        unreal.log(f"[VerifyDungeonKit01Materials] {status} {asset_path} parent={parent_path}")

    unreal.log(f"[VerifyDungeonKit01Materials] all_unlit={all_ok}")

    world = None
    try:
        subsystem = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
        if subsystem:
            world = subsystem.get_editor_world()
    except Exception:
        pass

    try:
        unreal.SystemLibrary.execute_console_command(world, "QUIT_EDITOR")
    except Exception as exc:
        unreal.log_warning(f"[VerifyDungeonKit01Materials] Failed to request QUIT_EDITOR: {exc}")


if __name__ == "__main__":
    main()
