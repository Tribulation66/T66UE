import base64
import hashlib
import hmac
import json
import os
import time
import urllib.error
import urllib.request
import winreg
from pathlib import Path


ROOT = Path(r"C:\UE\T66\SourceAssets\UI\MasterLibrary\ScreenArt\MainMenu\NewMM")
OUT_DIR = ROOT / "VideoGen"
IMAGE_PATH = ROOT / "main_menu_newmm_base_1920.png"
STATIC_MASK_PATH = OUT_DIR / "main_menu_static_freeze_mask.png"
JOB_RECORD_PATH = OUT_DIR / "kling_job_static_mask.json"
KLING_ENDPOINT = "https://api-singapore.klingai.com/v1/videos/image2video"


PROMPT = (
    "Locked camera cinematic game main menu background. The golden idol statue, black eclipse center, "
    "and gold altar remain perfectly still, centered, and unchanged. Only the unmasked environment animates: "
    "tiny stars softly twinkle in place across the black sky, and the fiery eclipse corona flickers like solar "
    "plasma with slow flame licking and glowing heat. Subtle warm gold reflections shimmer gently without changing "
    "the statue shape. Seamless loop feeling, slow elegant ambient motion."
)

NEGATIVE_PROMPT = (
    "camera movement, zoom, pan, statue movement, face morphing, altar deformation, changing geometry, melting gold, "
    "water, text, UI, logo, smoke covering idol, glitch artifacts, distortion, blur, extra objects"
)


def read_user_env(name: str) -> str:
    value = os.environ.get(name)
    if value:
        return value

    with winreg.OpenKey(winreg.HKEY_CURRENT_USER, "Environment") as key:
        return winreg.QueryValueEx(key, name)[0]


def b64url(data: bytes) -> bytes:
    return base64.urlsafe_b64encode(data).rstrip(b"=")


def make_jwt(access_key: str, secret_key: str) -> str:
    header = {"alg": "HS256", "typ": "JWT"}
    now = int(time.time())
    payload = {"iss": access_key, "exp": now + 1800, "nbf": now - 5}
    signing_input = b".".join(
        [
            b64url(json.dumps(header, separators=(",", ":")).encode("utf-8")),
            b64url(json.dumps(payload, separators=(",", ":")).encode("utf-8")),
        ]
    )
    signature = hmac.new(secret_key.encode("utf-8"), signing_input, hashlib.sha256).digest()
    return (signing_input + b"." + b64url(signature)).decode("ascii")


def file_b64(path: Path) -> str:
    return base64.b64encode(path.read_bytes()).decode("ascii")


def main() -> None:
    if not IMAGE_PATH.exists():
        raise FileNotFoundError(IMAGE_PATH)
    if not STATIC_MASK_PATH.exists():
        raise FileNotFoundError(
            f"{STATIC_MASK_PATH} does not exist. Run regenerate_kling_masks.py first."
        )

    access_key = read_user_env("KLING_ACCESS_KEY")
    secret_key = read_user_env("KLING_SECRET_KEY")
    token = make_jwt(access_key, secret_key)

    payload = {
        "model_name": "kling-v1",
        "mode": "std",
        "duration": "5",
        "image": file_b64(IMAGE_PATH),
        "prompt": PROMPT,
        "negative_prompt": NEGATIVE_PROMPT,
        "cfg_scale": 0.45,
        "static_mask": file_b64(STATIC_MASK_PATH),
    }

    request = urllib.request.Request(
        KLING_ENDPOINT,
        data=json.dumps(payload).encode("utf-8"),
        method="POST",
        headers={
            "Authorization": f"Bearer {token}",
            "Content-Type": "application/json",
        },
    )

    try:
        with urllib.request.urlopen(request, timeout=60) as response:
            status = response.status
            body = response.read().decode("utf-8", errors="replace")
    except urllib.error.HTTPError as error:
        status = error.code
        body = error.read().decode("utf-8", errors="replace")

    try:
        parsed = json.loads(body)
    except json.JSONDecodeError:
        parsed = {"raw": body}

    record = {
        "status": status,
        "response": parsed,
        "submitted_at": int(time.time()),
        "payload_summary": {
            "model_name": payload["model_name"],
            "mode": payload["mode"],
            "duration": payload["duration"],
            "has_static_mask": True,
        },
    }
    JOB_RECORD_PATH.write_text(json.dumps(record, indent=2), encoding="utf-8")

    print(f"status={status}")
    print(json.dumps(parsed, indent=2))
    print(f"job_record={JOB_RECORD_PATH}")


if __name__ == "__main__":
    main()
