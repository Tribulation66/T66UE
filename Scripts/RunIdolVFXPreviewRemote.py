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
SETUP_SCRIPT = os.path.join(PROJECT_ROOT, "Scripts", "SetupIdolVFXPreview.py")
SHOT_DIR = os.path.join(PROJECT_ROOT, "Saved", "IdolReview")

SHOTS = (
    ("idol_overview.png", (1050.0, 0.0, 280.0), (-520.0, -3600.0, 1260.0)),
    ("idol_pierce.png", (1050.0, -705.0, 210.0), (-260.0, -2050.0, 760.0)),
    ("idol_bounce.png", (1050.0, -235.0, 210.0), (-260.0, -2050.0, 760.0)),
    ("idol_aoe.png", (1050.0, 235.0, 260.0), (-260.0, -2050.0, 820.0)),
    ("idol_dot.png", (1050.0, 705.0, 300.0), (-260.0, -2050.0, 860.0)),
)


def log(message: str) -> None:
    print(f"[IDOL REVIEW DRIVER] {message}", flush=True)


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


def clear_viewport_overlays(hwnd: int) -> None:
    user32 = ctypes.windll.user32
    WM_KEYDOWN = 0x0100
    WM_KEYUP = 0x0101
    VK_ESCAPE = 0x1B
    WM_LBUTTONDOWN = 0x0201
    WM_LBUTTONUP = 0x0202

    # Clear any stuck viewport context menu / popup before we capture.
    user32.PostMessageW(hwnd, WM_KEYDOWN, VK_ESCAPE, 0)
    user32.PostMessageW(hwnd, WM_KEYUP, VK_ESCAPE, 0)
    time.sleep(0.15)

    client_rect = ctypes.wintypes.RECT()
    if user32.GetClientRect(hwnd, ctypes.byref(client_rect)):
        click_x = max(64, int((client_rect.right - client_rect.left) * 0.60))
        click_y = max(64, int((client_rect.bottom - client_rect.top) * 0.45))
        lparam = (click_y << 16) | (click_x & 0xFFFF)
        user32.PostMessageW(hwnd, WM_LBUTTONDOWN, 1, lparam)
        user32.PostMessageW(hwnd, WM_LBUTTONUP, 0, lparam)
        time.sleep(0.15)


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
    time.sleep(1.3)
    clear_viewport_overlays(hwnd)

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


def prepare_view_code(focus, offset) -> str:
    return f"""
import unreal

focus = unreal.Vector({focus[0]}, {focus[1]}, {focus[2]})
camera_location = focus + unreal.Vector({offset[0]}, {offset[1]}, {offset[2]})
camera_rotation = unreal.MathLibrary.find_look_at_rotation(camera_location, focus)
editor_subsystem = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
editor_subsystem.set_level_viewport_camera_info(camera_location, camera_rotation)
world = editor_subsystem.get_editor_world()
unreal.SystemLibrary.execute_console_command(world, 'viewmode lit')
unreal.SystemLibrary.execute_console_command(world, 'showflag.modewidgets 0')
unreal.SystemLibrary.execute_console_command(world, 'showflag.sprites 0')
unreal.SystemLibrary.execute_console_command(world, 'showflag.billboards 0')
unreal.SystemLibrary.execute_console_command(world, 'showflag.grid 0')
unreal.SystemLibrary.execute_console_command(world, 'showflag.selectionoutline 0')
unreal.log('[IDOL VFX PREVIEW] Camera prepared')
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
        time.sleep(4.0)

        for filename, focus, offset in SHOTS:
            shot_path = os.path.join(SHOT_DIR, filename)
            if os.path.exists(shot_path):
                os.remove(shot_path)
            run_remote(session, remote_execution, prepare_view_code(focus, offset), exec_mode=remote_execution.MODE_EXEC_FILE)
            time.sleep(1.5)
            capture_window(editor_process.pid, shot_path)
            log(f"Captured {filename}")

        log("Idol preview capture completed")
        return 0
    finally:
        if session is not None:
            session.stop()
        if editor_process.poll() is None:
            editor_process.terminate()


if __name__ == "__main__":
    raise SystemExit(main())
