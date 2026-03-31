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
SHOT_PATH = os.environ.get("ZIBRA_SHOT_PATH", r"C:\UE\T66\Saved\ZibraReview\zibra_preview.png")

STATE = {
    "start_time": time.time(),
    "callback_handle": None,
}


def quit_editor() -> None:
    unreal.SystemLibrary.execute_console_command(unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world(), "QUIT_EDITOR")


def cleanup_and_quit(message: str, is_error: bool = False) -> None:
    handle = STATE.get("callback_handle")
    if handle is not None:
        unreal.unregister_slate_post_tick_callback(handle)
        STATE["callback_handle"] = None
    if is_error:
        unreal.log_error(message)
    else:
        unreal.log(message)
    quit_editor()


def on_post_tick(_delta_seconds: float) -> None:
    if os.path.exists(SHOT_PATH) and os.path.getsize(SHOT_PATH) > 0:
        cleanup_and_quit(f"[ZIBRA FIRE PREVIEW] Screenshot saved: {SHOT_PATH}")
        return

    if time.time() - STATE["start_time"] > 45.0:
        cleanup_and_quit(f"[ZIBRA FIRE PREVIEW] Timed out waiting for screenshot: {SHOT_PATH}", is_error=True)


def prepare_and_capture() -> None:
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
    focus = origin + unreal.Vector(0.0, 0.0, extent.z * 0.18)
    camera_location = focus + unreal.Vector(-extent_max * 1.8, -extent_max * 0.9, extent_max * 0.38)
    camera_rotation = unreal.MathLibrary.find_look_at_rotation(camera_location, focus)
    camera_actor = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.CameraActor, camera_location, camera_rotation)
    camera_actor.set_actor_label("FirePreview_CaptureCamera")

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

    if os.path.exists(SHOT_PATH):
        os.remove(SHOT_PATH)

    task = unreal.AutomationLibrary.take_high_res_screenshot(
        1920,
        1080,
        SHOT_PATH,
        camera_actor,
        False,
        False,
        unreal.ComparisonTolerance.LOW,
        "",
        0.2,
        True,
    )
    if task is None or not task.is_valid_task():
        raise RuntimeError(f"Failed to start screenshot task for {SHOT_PATH}")

    unreal.log(f"[ZIBRA FIRE PREVIEW] Screenshot task started actor={ACTOR_LABEL} fraction={FRAME_FRACTION:.3f} path={SHOT_PATH}")
    STATE["callback_handle"] = unreal.register_slate_post_tick_callback(on_post_tick)


prepare_and_capture()
