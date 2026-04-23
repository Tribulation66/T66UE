from __future__ import annotations

import argparse
import json
import os
import shutil
import urllib.error
import urllib.parse
import urllib.request
from pathlib import Path


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Send a prompt plus optional reference images to the local ChatGPT bridge and save JSON metadata."
    )
    parser.add_argument("--endpoint", default="http://127.0.0.1:5000/generate", help="Bridge /generate endpoint.")
    parser.add_argument(
        "--request-file",
        type=Path,
        help="Optional JSON request file. Relative paths inside it are resolved relative to that file.",
    )
    parser.add_argument(
        "--token-env",
        default="CHATGPT_BRIDGE_TOKEN",
        help="Environment variable holding the bridge auth token.",
    )
    parser.add_argument("--prompt", default="", help="Inline prompt text.")
    parser.add_argument("--prompt-file", type=Path, help="Path to a UTF-8 prompt text file.")
    parser.add_argument(
        "--image",
        dest="images",
        action="append",
        default=[],
        help="Reference image path to attach. Repeat for multiple images.",
    )
    parser.add_argument(
        "--timeout-seconds",
        type=int,
        default=180,
        help="Bridge generation timeout in seconds.",
    )
    parser.add_argument(
        "--json-out",
        type=Path,
        help="Optional path to save the JSON response payload.",
    )
    parser.add_argument(
        "--copy-to-dir",
        type=Path,
        help="Optional directory to copy the generated files returned by the bridge.",
    )
    return parser.parse_args()


def load_request_file(path: Path) -> tuple[dict, Path]:
    request_path = path.expanduser().resolve()
    payload = json.loads(request_path.read_text(encoding="utf-8"))
    if not isinstance(payload, dict):
        raise SystemExit(f"Request file must contain a JSON object: {request_path}")
    return payload, request_path.parent


def resolve_maybe_relative(raw_path: str, base_dir: Path | None) -> Path:
    path = Path(raw_path).expanduser()
    if not path.is_absolute() and base_dir is not None:
        path = (base_dir / path).resolve()
    else:
        path = path.resolve()
    return path


def load_prompt(args: argparse.Namespace, request_payload: dict, request_base_dir: Path | None) -> str:
    prompt = args.prompt.strip() or str(request_payload.get("prompt", "")).strip()
    prompt_file = args.prompt_file
    if prompt_file is None and request_payload.get("prompt_file"):
        prompt_file = resolve_maybe_relative(str(request_payload["prompt_file"]), request_base_dir)
    if prompt_file:
        prompt = Path(prompt_file).read_text(encoding="utf-8").strip()
    if not prompt:
        raise SystemExit("Missing prompt. Use --prompt or --prompt-file.")
    return prompt


def resolve_images(images: list[str], base_dir: Path | None) -> list[str]:
    resolved: list[str] = []
    for raw_path in images:
        path = resolve_maybe_relative(raw_path, base_dir)
        if not path.exists():
            raise SystemExit(f"Reference image does not exist: {path}")
        resolved.append(str(path))
    return resolved


def post_generate(
    endpoint: str,
    token: str,
    prompt: str,
    image_paths: list[str],
    timeout_seconds: int,
) -> dict:
    payload = {
        "prompt": prompt,
        "image_paths": image_paths,
        "timeout_seconds": timeout_seconds,
    }
    url = endpoint.rstrip("/") + "?mode=json"
    data = json.dumps(payload).encode("utf-8")
    request = urllib.request.Request(
        url,
        data=data,
        headers={
            "Content-Type": "application/json",
            "X-Bridge-Token": token,
        },
        method="POST",
    )
    try:
        with urllib.request.urlopen(request, timeout=timeout_seconds + 30) as response:
            raw = response.read().decode("utf-8")
            return json.loads(raw)
    except urllib.error.HTTPError as exc:
        body = exc.read().decode("utf-8", errors="replace")
        raise SystemExit(f"Bridge request failed: HTTP {exc.code}\n{body}") from exc
    except urllib.error.URLError as exc:
        raise SystemExit(f"Bridge request failed: {exc.reason}") from exc


def copy_generated_files(response_payload: dict, target_dir: Path) -> list[Path]:
    files = response_payload.get("files", [])
    target_dir.mkdir(parents=True, exist_ok=True)
    copied: list[Path] = []
    for entry in files:
        source = Path(str(entry["saved_file"])).resolve()
        if not source.exists():
            raise SystemExit(f"Bridge reported generated file does not exist: {source}")
        destination = target_dir / source.name
        shutil.copy2(source, destination)
        copied.append(destination)
    return copied


def main() -> int:
    args = parse_args()

    request_payload: dict = {}
    request_base_dir: Path | None = None
    if args.request_file:
        request_payload, request_base_dir = load_request_file(args.request_file)

    prompt = load_prompt(args, request_payload, request_base_dir)
    image_sources = args.images or list(request_payload.get("image_paths", []))
    image_paths = resolve_images(image_sources, request_base_dir)

    endpoint = args.endpoint or str(request_payload.get("endpoint", "")).strip() or "http://127.0.0.1:5000/generate"
    token_env = args.token_env or str(request_payload.get("token_env", "")).strip() or "CHATGPT_BRIDGE_TOKEN"
    timeout_seconds = args.timeout_seconds if args.timeout_seconds != 180 else int(request_payload.get("timeout_seconds", 180))
    json_out = args.json_out
    if json_out is None and request_payload.get("json_out"):
        json_out = resolve_maybe_relative(str(request_payload["json_out"]), request_base_dir)
    copy_to_dir = args.copy_to_dir
    if copy_to_dir is None and request_payload.get("copy_to_dir"):
        copy_to_dir = resolve_maybe_relative(str(request_payload["copy_to_dir"]), request_base_dir)

    token = os.getenv(token_env, "").strip()
    if not token:
        raise SystemExit(f"Missing auth token in environment variable: {token_env}")

    response_payload = post_generate(endpoint, token, prompt, image_paths, timeout_seconds)

    if json_out:
        json_out.parent.mkdir(parents=True, exist_ok=True)
        json_out.write_text(json.dumps(response_payload, indent=2), encoding="utf-8")

    copied_files: list[Path] = []
    if copy_to_dir:
        copied_files = copy_generated_files(response_payload, copy_to_dir)

    print(json.dumps(response_payload, indent=2))
    if copied_files:
        print("\nCopied files:")
        for path in copied_files:
            print(path)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
