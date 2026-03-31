import os
import subprocess
import sys
import time
import ctypes
import ctypes.wintypes

from PIL import ImageGrab


PROJECT_ROOT = r"C:\UE\T66"
PROJECT_FILE = os.path.join(PROJECT_ROOT, "T66.uproject")
EDITOR_EXE = r"C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe"
REMOTE_EXECUTION_PATH = (
    r"C:\Program Files\Epic Games\UE_5.7\Engine\Plugins\Experimental\PythonScriptPlugin\Content\Python"
)
SETUP_SCRIPT = os.path.join(PROJECT_ROOT, "Scripts", "SetupZibraFirePreview.py")
SHOT_DIR = os.path.join(PROJECT_ROOT, "Saved", "ZibraReview")


def log(message: str) -> None:
    print(f"[ZIBRA FIRE DRIVER] {message}", flush=True)


def wait_for(condition, timeout_seconds: float, label: str) -> None:
    deadline = time.time() + timeout_seconds
    while time.time() < deadline:
        if condition():
            return
        time.sleep(1.0)
    raise TimeoutError(f"Timed out waiting for {label}")


def launch_editor() -> subprocess.Popen:
    return subprocess.Popen([EDITOR_EXE, PROJECT_FILE, "-NoSplash", "-log"], cwd=PROJECT_ROOT)


def connect_remote():
    sys.path.append(REMOTE_EXECUTION_PATH)
    import remote_execution

    session = remote_execution.RemoteExecution()
    session.start()
    wait_for(lambda: len(session.remote_nodes) > 0, 300.0, "an Unreal Editor remote Python node")
    node = session.remote_nodes[0]
    session.open_command_connection(node["node_id"])
    log(f"Connected to node {node['node_id']}")
    return session, remote_execution


def run_remote(session, remote_execution, command: str, exec_mode: str = None):
    mode = exec_mode or remote_execution.MODE_EXEC_FILE
    result = session.run_command(command, exec_mode=mode, raise_on_failure=True)
    log("Remote command completed")
    return result


def wait_for_file(path: str, timeout_seconds: float = 60.0) -> None:
    wait_for(lambda: os.path.exists(path) and os.path.getsize(path) > 0, timeout_seconds, path)


def find_main_window(pid: int):
    user32 = ctypes.windll.user32
    windows = []

    enum_windows = user32.EnumWindows
    enum_windows_proc = ctypes.WINFUNCTYPE(ctypes.c_bool, ctypes.c_void_p, ctypes.c_void_p)
    get_window_thread_process_id = user32.GetWindowThreadProcessId
    is_window_visible = user32.IsWindowVisible
    get_window_text_length = user32.GetWindowTextLengthW
    get_window_text = user32.GetWindowTextW

    def callback(hwnd, _lparam):
        process_id = ctypes.c_ulong()
        get_window_thread_process_id(hwnd, ctypes.byref(process_id))
        if process_id.value != pid or not is_window_visible(hwnd):
            return True

        length = get_window_text_length(hwnd)
        if length <= 0:
            return True

        title_buffer = ctypes.create_unicode_buffer(length + 1)
        get_window_text(hwnd, title_buffer, length + 1)
        title = title_buffer.value
        if title:
            windows.append(hwnd)
        return True

    enum_windows(enum_windows_proc(callback), 0)
    return windows[0] if windows else None


def capture_window(pid: int, path: str) -> None:
    user32 = ctypes.windll.user32
    user32.SetProcessDPIAware()
    hwnd = find_main_window(pid)
    if hwnd is None:
        raise RuntimeError(f"Failed to find Unreal Editor window for pid={pid}")

    SW_RESTORE = 9
    SW_MAXIMIZE = 3
    user32.ShowWindow(hwnd, SW_RESTORE)
    user32.ShowWindow(hwnd, SW_MAXIMIZE)
    user32.SetForegroundWindow(hwnd)
    time.sleep(1.5)

    client_rect = ctypes.wintypes.RECT()
    if not user32.GetClientRect(hwnd, ctypes.byref(client_rect)):
        raise RuntimeError("GetClientRect failed")

    top_left = ctypes.wintypes.POINT(client_rect.left, client_rect.top)
    bottom_right = ctypes.wintypes.POINT(client_rect.right, client_rect.bottom)
    if not user32.ClientToScreen(hwnd, ctypes.byref(top_left)):
        raise RuntimeError("ClientToScreen failed for top-left")
    if not user32.ClientToScreen(hwnd, ctypes.byref(bottom_right)):
        raise RuntimeError("ClientToScreen failed for bottom-right")

    bbox = (top_left.x, top_left.y, bottom_right.x, bottom_right.y)
    image = ImageGrab.grab(bbox=bbox, all_screens=True)
    image.save(path)

    crop_left = int(image.width * 0.12)
    crop_top = int(image.height * 0.06)
    crop_right = int(image.width * 0.88)
    crop_bottom = int(image.height * 0.84)
    cropped = image.crop((crop_left, crop_top, crop_right, crop_bottom))
    cropped.save(path.replace(".png", "_crop.png"))


def prepare_view_code(actor_label: str, frame_fraction: float) -> str:
    return f"""
import unreal

actors = unreal.EditorLevelLibrary.get_all_level_actors()
target = None
for actor in actors:
    if actor.get_actor_label() == "{actor_label}":
        target = actor
        break

if target is None:
    raise RuntimeError("Failed to find actor {actor_label}")

for actor in actors:
    if actor.get_class().get_name() == "ZibraVDBActor":
        playback = actor.get_editor_property("playback_component")
        playback.set_editor_property("visible", actor == target)

asset_component = target.get_editor_property("asset_component")
volume = asset_component.get_editor_property("zibra_vdb_volume")
playback = target.get_editor_property("playback_component")
frame_count = max(int(volume.get_editor_property("frame_count")), 1)
frame = max(0, min(int(frame_count * {frame_fraction}), frame_count - 1))
playback.set_editor_property("animate", False)
playback.set_editor_property("current_frame", float(frame))
playback.set_editor_property("visible", True)

origin, extent = target.get_actor_bounds(False)
extent_max = max(extent.x, extent.y, extent.z, 150.0)
focus = origin + unreal.Vector(0.0, 0.0, extent.z * 0.35)
camera_location = focus + unreal.Vector(-extent_max * 2.0, -extent_max * 1.0, extent_max * 0.55)
camera_rotation = unreal.MathLibrary.find_look_at_rotation(camera_location, focus)

level_editor = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
level_editor.editor_set_game_view(True)
level_editor.editor_invalidate_viewports()

editor_subsystem = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
editor_subsystem.set_level_viewport_camera_info(camera_location, camera_rotation)

world = editor_subsystem.get_editor_world()
unreal.SystemLibrary.execute_console_command(world, 'viewmode lit')
unreal.SystemLibrary.execute_console_command(world, 'showflag.modewidgets 0')
unreal.SystemLibrary.execute_console_command(world, 'showflag.sprites 0')
unreal.SystemLibrary.execute_console_command(world, 'showflag.billboards 0')
unreal.SystemLibrary.execute_console_command(world, 'showflag.grid 0')
unreal.SystemLibrary.execute_console_command(world, 'showflag.selectionoutline 0')
unreal.log("[ZIBRA FIRE PREVIEW] View prepared for {actor_label} at frame fraction {frame_fraction}")
"""


def main() -> int:
    os.makedirs(SHOT_DIR, exist_ok=True)
    editor_process = launch_editor()
    log(f"Launched Unreal Editor (pid={editor_process.pid})")
    session = None
    remote_execution = None

    try:
        session, remote_execution = connect_remote()
        run_remote(session, remote_execution, SETUP_SCRIPT)
        time.sleep(5.0)

        for actor_label, filename, frame_fraction in (
            ("Zibra_GroundExplosion_Preview", "GroundExplosion_review_f08.png", 0.08),
            ("Zibra_GroundExplosion_Preview", "GroundExplosion_review_f14.png", 0.14),
            ("Zibra_ShockwaveExplosion_Preview", "ShockwaveExplosion_review_f05.png", 0.05),
            ("Zibra_ShockwaveExplosion_Preview", "ShockwaveExplosion_review_f10.png", 0.10),
        ):
            shot_path = os.path.join(SHOT_DIR, filename)
            crop_path = shot_path.replace(".png", "_crop.png")
            for stale_path in (shot_path, crop_path):
                if os.path.exists(stale_path):
                    os.remove(stale_path)
            run_remote(session, remote_execution, prepare_view_code(actor_label, frame_fraction), exec_mode=remote_execution.MODE_EXEC_FILE)
            time.sleep(2.0)
            capture_window(editor_process.pid, shot_path)
            wait_for_file(shot_path, 10.0)
            wait_for_file(crop_path, 10.0)
            log(f"Captured {filename}")

        log("Zibra preview capture completed")
        return 0
    finally:
        if session is not None:
            session.stop()
        if editor_process.poll() is None:
            editor_process.terminate()


if __name__ == "__main__":
    raise SystemExit(main())
