import ctypes
import ctypes.wintypes
import json
import os
import subprocess
import time

from PIL import Image, ImageGrab


PROJECT_ROOT = r"C:\UE\T66"
PROJECT_FILE = os.path.join(PROJECT_ROOT, "T66.uproject")
EDITOR_EXE = r"C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe"
PREP_SCRIPT = os.path.join(PROJECT_ROOT, "Scripts", "CaptureZibraFireInEditor.py")
SHOT_DIR = os.path.join(PROJECT_ROOT, "Saved", "ZibraReview")
LOG_FILE = os.path.join(PROJECT_ROOT, "Saved", "Logs", "T66.log")

CAPTURES = (
    ("Zibra_GroundExplosion_Preview", "GroundExplosion_review_f08.png", 0.08),
    ("Zibra_GroundExplosion_Preview", "GroundExplosion_review_f14.png", 0.14),
    ("Zibra_ShockwaveExplosion_Preview", "ShockwaveExplosion_review_f05.png", 0.05),
    ("Zibra_ShockwaveExplosion_Preview", "ShockwaveExplosion_review_f10.png", 0.10),
)


def log(message: str) -> None:
    print(f"[ZIBRA FIRE CAPTURE] {message}", flush=True)


def wait_for(condition, timeout_seconds: float, label: str) -> None:
    deadline = time.time() + timeout_seconds
    while time.time() < deadline:
        if condition():
            return
        time.sleep(1.0)
    raise TimeoutError(f"Timed out waiting for {label}")


def wait_for_log(marker: str, timeout_seconds: float = 180.0) -> None:
    def has_marker() -> bool:
        if not os.path.exists(LOG_FILE):
            return False
        with open(LOG_FILE, "r", encoding="utf-8", errors="ignore") as handle:
            return marker in handle.read()

    wait_for(has_marker, timeout_seconds, marker)


def find_main_window(pid: int):
    try:
        command = (
            f"$p = Get-Process -Id {pid} -ErrorAction Stop; "
            "[pscustomobject]@{Title=$p.MainWindowTitle; Handle=$p.MainWindowHandle} | ConvertTo-Json -Compress"
        )
        result = subprocess.check_output(
            ["powershell.exe", "-NoProfile", "-Command", command],
            text=True,
            stderr=subprocess.DEVNULL,
        ).strip()
        if result:
            data = json.loads(result)
            handle = int(data.get("Handle") or 0)
            title = data.get("Title") or ""
            if handle and title:
                return handle
    except Exception:
        pass

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
            windows.append((hwnd, title))
        return True

    enum_windows(enum_windows_proc(callback), 0)
    if not windows:
        return None

    preferred = [item for item in windows if "Unreal Editor" in item[1]]
    if preferred:
        preferred.sort(key=lambda item: (item[1] != "T66 - Unreal Editor", item[1]))
        return preferred[0][0]

    return windows[0][0]


def capture_window(pid: int, path: str) -> None:
    user32 = ctypes.windll.user32
    user32.SetProcessDPIAware()
    hwnd = find_main_window(pid)
    bbox = None
    if hwnd:
        SW_RESTORE = 9
        SW_MAXIMIZE = 3
        user32.ShowWindow(hwnd, SW_RESTORE)
        user32.ShowWindow(hwnd, SW_MAXIMIZE)
        user32.SetForegroundWindow(hwnd)
        time.sleep(1.5)

        window_rect = ctypes.wintypes.RECT()
        if user32.GetWindowRect(hwnd, ctypes.byref(window_rect)):
            bbox = (window_rect.left, window_rect.top, window_rect.right, window_rect.bottom)

    if bbox is None:
        time.sleep(1.0)
        image = ImageGrab.grab(all_screens=True)
    else:
        image = ImageGrab.grab(bbox=bbox, all_screens=True)
    image.save(path)

    crop_left = int(image.width * 0.20)
    crop_top = int(image.height * 0.05)
    crop_right = int(image.width * 0.88)
    crop_bottom = int(image.height * 0.82)
    image.crop((crop_left, crop_top, crop_right, crop_bottom)).save(path.replace(".png", "_crop.png"))


def run_capture(actor_label: str, filename: str, frame_fraction: float) -> None:
    env = os.environ.copy()
    env["ZIBRA_ACTOR_LABEL"] = actor_label
    env["ZIBRA_FRAME_FRACTION"] = str(frame_fraction)
    shot_path = os.path.join(SHOT_DIR, filename)
    crop_path = shot_path.replace(".png", "_crop.png")
    env["ZIBRA_SHOT_PATH"] = shot_path

    if os.path.exists(LOG_FILE):
        os.remove(LOG_FILE)

    for stale_path in (shot_path, crop_path):
        if os.path.exists(stale_path):
            os.remove(stale_path)

    process = subprocess.Popen(
        [EDITOR_EXE, PROJECT_FILE, "-NoSplash", "-log", f"-ExecutePythonScript={PREP_SCRIPT}"],
        cwd=PROJECT_ROOT,
        env=env,
    )
    log(f"Launched Unreal Editor pid={process.pid} for {filename}")

    try:
        wait_for_log(f"[ZIBRA FIRE PREVIEW] Screenshot task started actor={actor_label} fraction={frame_fraction:.3f}", 240.0)
        wait_for(lambda: os.path.exists(shot_path) and os.path.getsize(shot_path) > 0, 10.0, shot_path)
        image = Image.open(shot_path)
        crop_left = int(image.width * 0.18)
        crop_top = int(image.height * 0.08)
        crop_right = int(image.width * 0.82)
        crop_bottom = int(image.height * 0.78)
        image.crop((crop_left, crop_top, crop_right, crop_bottom)).save(crop_path)
        log(f"Captured {filename}")
    finally:
        if process.poll() is None:
            process.terminate()
            try:
                process.wait(timeout=20.0)
            except subprocess.TimeoutExpired:
                process.kill()


def main() -> int:
    os.makedirs(SHOT_DIR, exist_ok=True)
    for actor_label, filename, frame_fraction in CAPTURES:
        run_capture(actor_label, filename, frame_fraction)
    log("All Zibra captures completed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
