import os
import sys
import time

import unreal

SCRIPT_DIR = os.path.dirname(__file__)
if SCRIPT_DIR not in sys.path:
    sys.path.append(SCRIPT_DIR)

import SetupZibraFirePreview  # noqa: F401 - importing runs the preview scene build


ACTOR_LABEL = os.environ.get("ZIBRA_ACTOR_LABEL", "Zibra_GroundExplosion_Preview")
FRAME_FRACTION = float(os.environ.get("ZIBRA_FRAME_FRACTION", "0.10"))


def prepare_view() -> None:
    actors = unreal.EditorLevelLibrary.get_all_level_actors()
    target = None
    for actor in actors:
        if actor.get_actor_label() == ACTOR_LABEL:
            target = actor
            break

    if target is None:
        raise RuntimeError(f"Failed to find actor {ACTOR_LABEL}")

    for actor in actors:
        if actor.get_class().get_name() == "ZibraVDBActor":
            playback = actor.get_editor_property("playback_component")
            playback.set_editor_property("visible", actor == target)

    asset_component = target.get_editor_property("asset_component")
    volume = asset_component.get_editor_property("zibra_vdb_volume")
    playback = target.get_editor_property("playback_component")
    frame_count = max(int(volume.get_editor_property("frame_count")), 1)
    frame = max(0, min(int(frame_count * FRAME_FRACTION), frame_count - 1))
    playback.set_editor_property("animate", False)
    playback.set_editor_property("current_frame", float(frame))
    playback.set_editor_property("visible", True)

    origin, extent = target.get_actor_bounds(False)
    extent_max = max(extent.x, extent.y, extent.z, 150.0)
    focus = origin + unreal.Vector(0.0, 0.0, extent.z * 0.25)
    camera_location = focus + unreal.Vector(-extent_max * 1.25, -extent_max * 0.65, extent_max * 0.22)
    camera_rotation = unreal.MathLibrary.find_look_at_rotation(camera_location, focus)

    level_editor = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    level_editor.editor_set_game_view(True)
    level_editor.editor_invalidate_viewports()

    editor_subsystem = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
    editor_subsystem.set_level_viewport_camera_info(camera_location, camera_rotation)

    world = editor_subsystem.get_editor_world()
    unreal.SystemLibrary.execute_console_command(world, "viewmode lit")
    unreal.SystemLibrary.execute_console_command(world, "showflag.modewidgets 0")
    unreal.SystemLibrary.execute_console_command(world, "showflag.sprites 0")
    unreal.SystemLibrary.execute_console_command(world, "showflag.billboards 0")
    unreal.SystemLibrary.execute_console_command(world, "showflag.grid 0")
    unreal.SystemLibrary.execute_console_command(world, "showflag.selectionoutline 0")
    unreal.SystemLibrary.execute_console_command(world, "showflag.bounds 0")
    unreal.log(f"[ZIBRA FIRE PREVIEW] View ready actor={ACTOR_LABEL} fraction={FRAME_FRACTION:.3f}")
    time.sleep(20.0)


prepare_view()
