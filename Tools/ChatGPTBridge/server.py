from __future__ import annotations

import argparse
import base64
import hashlib
import io
import os
import re
import threading
import time
import zipfile
from dataclasses import dataclass
from datetime import UTC, datetime
from pathlib import Path
from typing import Any

try:
    from flask import Flask, jsonify, request, send_file
except ImportError as exc:  # pragma: no cover
    raise SystemExit(
        "Missing dependency: Flask. Run `pip install -r Tools/ChatGPTBridge/requirements.txt`."
    ) from exc

try:
    from playwright.sync_api import Error as PlaywrightError
    from playwright.sync_api import TimeoutError as PlaywrightTimeoutError
    from playwright.sync_api import sync_playwright
except ImportError as exc:  # pragma: no cover
    raise SystemExit(
        "Missing dependency: playwright. Run `pip install -r Tools/ChatGPTBridge/requirements.txt`."
    ) from exc


BASE_DIR = Path(__file__).resolve().parent
OUTPUT_DIR = BASE_DIR / "output"
OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

DEFAULT_CDP_URL = os.getenv("CHATGPT_CDP_URL", "http://127.0.0.1:9222")
DEFAULT_BASE_URL = os.getenv("CHATGPT_BASE_URL", "https://chatgpt.com/")
DEFAULT_TIMEOUT_SECONDS = int(os.getenv("CHATGPT_IMAGE_TIMEOUT", "180"))
DEFAULT_PAGE_TIMEOUT_MS = int(os.getenv("CHATGPT_PAGE_TIMEOUT_MS", "45000"))
DEFAULT_RENDER_SETTLE_SECONDS = float(os.getenv("CHATGPT_RENDER_SETTLE_SECONDS", "3.0"))
AUTH_TOKEN = os.getenv("CHATGPT_BRIDGE_TOKEN", "").strip()

app = Flask(__name__)
generation_lock = threading.Lock()


@dataclass(frozen=True)
class ImageCandidate:
    scope: str
    dom_index: int
    src: str
    href: str | None
    alt: str
    natural_width: int
    natural_height: int
    rendered_width: float
    rendered_height: float

    @property
    def key(self) -> str:
        return self.href or self.src

    @property
    def score(self) -> float:
        width = max(self.natural_width, int(self.rendered_width))
        height = max(self.natural_height, int(self.rendered_height))
        return float(width * height)


@dataclass(frozen=True)
class GeneratedAsset:
    path: Path
    mime_type: str
    data: bytes

    @property
    def size_bytes(self) -> int:
        return len(self.data)


class BridgeError(RuntimeError):
    pass


class RateLimitError(BridgeError):
    def __init__(self, message: str, retry_after_seconds: int):
        super().__init__(message)
        self.retry_after_seconds = retry_after_seconds


def now_stamp() -> str:
    return datetime.now(UTC).strftime("%Y%m%d-%H%M%S")


def slugify(value: str, max_length: int = 48) -> str:
    cleaned = re.sub(r"[^a-zA-Z0-9]+", "-", value).strip("-").lower()
    if not cleaned:
        cleaned = "image"
    return cleaned[:max_length].rstrip("-") or "image"


def get_request_token() -> str:
    auth_header = request.headers.get("Authorization", "").strip()
    if auth_header.lower().startswith("bearer "):
        return auth_header[7:].strip()
    return request.headers.get("X-Bridge-Token", "").strip()


def require_auth() -> None:
    if not AUTH_TOKEN:
        raise BridgeError(
            "CHATGPT_BRIDGE_TOKEN is not set. Refusing to run without a shared secret."
        )
    token = get_request_token()
    if token != AUTH_TOKEN:
        raise BridgeError("Unauthorized request.")


def coerce_timeout(payload: dict[str, Any]) -> int:
    raw_value = payload.get("timeout_seconds", DEFAULT_TIMEOUT_SECONDS)
    try:
        timeout = int(raw_value)
    except (TypeError, ValueError) as exc:
        raise BridgeError("timeout_seconds must be an integer.") from exc
    return max(30, min(timeout, 600))


def coerce_image_paths(payload: dict[str, Any]) -> list[str]:
    raw_value = payload.get("image_paths", [])
    if raw_value is None:
        return []
    if not isinstance(raw_value, list) or not all(isinstance(item, str) for item in raw_value):
        raise BridgeError("image_paths must be a list of local file paths.")
    return [item.strip() for item in raw_value if item.strip()]


def build_image_candidates(raw_candidates: list[dict[str, Any]], scope: str) -> list[ImageCandidate]:
    return [
        ImageCandidate(
            scope=scope,
            dom_index=int(item["domIndex"]),
            src=str(item["src"]),
            href=str(item["href"]) if item["href"] else None,
            alt=str(item["alt"]),
            natural_width=int(item["naturalWidth"]),
            natural_height=int(item["naturalHeight"]),
            rendered_width=float(item["renderedWidth"]),
            rendered_height=float(item["renderedHeight"]),
        )
        for item in raw_candidates
    ]


def get_image_candidates(page: Any) -> list[ImageCandidate]:
    raw_candidates = page.evaluate(
        """
() => {
  const nodes = Array.from(document.querySelectorAll("main img"));
  return nodes.map((img, domIndex) => {
    const rect = img.getBoundingClientRect();
    return {
      domIndex,
      src: img.currentSrc || img.src || "",
      href: img.closest("a") ? img.closest("a").href : null,
      alt: img.alt || "",
      naturalWidth: img.naturalWidth || 0,
      naturalHeight: img.naturalHeight || 0,
      renderedWidth: rect.width || 0,
      renderedHeight: rect.height || 0
    };
  }).filter((candidate) => {
    const width = Math.max(candidate.naturalWidth, candidate.renderedWidth);
    const height = Math.max(candidate.naturalHeight, candidate.renderedHeight);
    const alt = (candidate.alt || "").toLowerCase();
    return width >= 256 && height >= 256 && !!candidate.src && alt.includes("generated image");
  });
}
"""
    )
    return build_image_candidates(raw_candidates, scope="page")


def get_assistant_turn_count(page: Any) -> int:
    return int(
        page.evaluate(
            """
() => document.querySelectorAll("[data-message-author-role='assistant']").length
"""
        )
    )


def get_latest_assistant_turn_locator(page: Any) -> Any:
    return page.locator("[data-message-author-role='assistant']").last


def get_latest_assistant_image_candidates(page: Any) -> list[ImageCandidate]:
    turn = get_latest_assistant_turn_locator(page)
    if turn.count() == 0:
        return []

    raw_candidates = turn.evaluate(
        """
(root) => {
  const nodes = Array.from(root.querySelectorAll("img"));
  return nodes.map((img, domIndex) => {
    const rect = img.getBoundingClientRect();
    return {
      domIndex,
      src: img.currentSrc || img.src || "",
      href: img.closest("a") ? img.closest("a").href : null,
      alt: img.alt || "",
      naturalWidth: img.naturalWidth || 0,
      naturalHeight: img.naturalHeight || 0,
      renderedWidth: rect.width || 0,
      renderedHeight: rect.height || 0
    };
  }).filter((candidate) => {
    const width = Math.max(candidate.naturalWidth, candidate.renderedWidth);
    const height = Math.max(candidate.naturalHeight, candidate.renderedHeight);
    return width >= 256 && height >= 256 && !!candidate.src;
  });
}
"""
    )

    return build_image_candidates(raw_candidates, scope="assistant")


def get_assistant_image_candidates(page: Any) -> list[ImageCandidate]:
    return get_latest_assistant_image_candidates(page)


def get_candidate_locator(page: Any, candidate: ImageCandidate) -> Any:
    if candidate.scope == "assistant":
        turn = get_latest_assistant_turn_locator(page)
        return turn.locator("img").nth(candidate.dom_index)
    return page.locator("main img").nth(candidate.dom_index)


def get_active_image_candidates(page: Any) -> list[ImageCandidate]:
    assistant_candidates = get_latest_assistant_image_candidates(page)
    if assistant_candidates:
        return assistant_candidates
    return get_image_candidates(page)


def wait_for_render_stability(page: Any, timeout_seconds: float, stable_seconds: float = DEFAULT_RENDER_SETTLE_SECONDS) -> list[ImageCandidate]:
    deadline = time.monotonic() + timeout_seconds
    last_signature: tuple[str, ...] = tuple()
    last_change = time.monotonic()
    stable_candidates: list[ImageCandidate] = []

    while time.monotonic() < deadline:
        candidates = get_active_image_candidates(page)
        if not candidates:
            time.sleep(0.75)
            continue

        signature: list[str] = []
        usable_candidates: list[ImageCandidate] = []

        for candidate in candidates:
            try:
                screenshot_bytes = get_candidate_locator(page, candidate).screenshot(type="png")
            except PlaywrightError:
                continue
            signature.append(hashlib.sha256(screenshot_bytes).hexdigest())
            usable_candidates.append(candidate)

        if not usable_candidates:
            time.sleep(0.75)
            continue

        current_signature = tuple(signature)
        if current_signature != last_signature:
            last_signature = current_signature
            last_change = time.monotonic()
            stable_candidates = usable_candidates
        elif time.monotonic() - last_change >= stable_seconds:
            return stable_candidates

        time.sleep(0.75)

    raise BridgeError(f"Timed out after {timeout_seconds:.0f} seconds waiting for the rendered image to settle.")


def wait_for_composer(page: Any) -> Any:
    composer = page.locator(
        "#prompt-textarea:visible, textarea:visible, div[contenteditable='true']:visible"
    ).first
    composer.wait_for(state="visible", timeout=DEFAULT_PAGE_TIMEOUT_MS)
    return composer


def attach_images_to_prompt(page: Any, image_paths: list[str]) -> None:
    if not image_paths:
        return

    resolved_paths: list[str] = []
    for raw_path in image_paths:
        path = Path(raw_path).expanduser().resolve()
        if not path.is_file():
            raise BridgeError(f"Reference image not found: {path}")
        resolved_paths.append(str(path))

    file_input = page.locator("input[type='file'][accept*='image']").first
    file_input.set_input_files(resolved_paths)
    page.wait_for_timeout(1500)


def fill_and_send_prompt(page: Any, prompt: str, image_paths: list[str] | None = None) -> None:
    composer = wait_for_composer(page)
    if image_paths:
        attach_images_to_prompt(page, image_paths)
    composer.click()
    page.keyboard.press("Control+A")
    page.keyboard.press("Backspace")
    page.keyboard.insert_text(prompt)

    send_button = page.locator(
        "button[data-testid='send-button']:visible:not([disabled]), "
        "button[aria-label*='Send']:visible:not([disabled])"
    ).first
    try:
        send_button.wait_for(state="visible", timeout=5000)
        send_button.click()
    except PlaywrightTimeoutError:
        page.keyboard.press("Enter")


def get_visible_issue_texts(page: Any) -> list[str]:
    try:
        texts = page.evaluate(
            """
() => {
  const selectors = [
    "[role='alert']",
    "[aria-live='assertive']",
    "[data-testid*='toast']",
    "[data-testid*='error']",
    "[data-testid*='notice']"
  ].join(",");

  const isVisible = (el) => {
    if (!el) return false;
    const style = window.getComputedStyle(el);
    if (!style) return false;
    if (style.display === "none" || style.visibility === "hidden" || Number(style.opacity || "1") === 0) return false;
    const rect = el.getBoundingClientRect();
    return rect.width > 0 && rect.height > 0;
  };

  return Array.from(document.querySelectorAll(selectors))
    .filter(isVisible)
    .map((node) => (node.innerText || node.textContent || "").trim())
    .filter(Boolean)
    .slice(-8);
}
"""
        )
    except PlaywrightError:
        return []

    if not isinstance(texts, list):
        return []
    return [str(text) for text in texts if str(text).strip()]


def get_latest_assistant_text(page: Any) -> str:
    try:
        text = page.evaluate(
            """
() => {
  const nodes = Array.from(document.querySelectorAll("[data-message-author-role='assistant']"));
  const last = nodes.at(-1);
  return last ? (last.innerText || last.textContent || "").trim() : "";
}
"""
        )
    except PlaywrightError:
        return ""

    return str(text).strip() if text else ""


def get_page_issue(page: Any, include_latest_assistant: bool = False) -> tuple[str, str, int | None] | None:
    issue_text = "\n".join(get_visible_issue_texts(page))
    if include_latest_assistant:
        latest_assistant_text = get_latest_assistant_text(page)
        if latest_assistant_text:
            issue_text = f"{issue_text}\n{latest_assistant_text}".strip()

    if issue_text:
        rate_limit_match = re.search(r"wait for\s+(\d+)\s+minute", issue_text, re.IGNORECASE)
        if (
            "You're generating images too quickly" in issue_text
            or "rate_limit_exceeded" in issue_text
            or rate_limit_match
        ):
            retry_after_seconds = int(rate_limit_match.group(1)) * 60 + 5 if rate_limit_match else 125
            return ("rate_limit", "ChatGPT image generation rate limit reached.", retry_after_seconds)

        error_markers = [
            "Something went wrong",
            "Unable to generate",
            "There was an error",
        ]
        for marker in error_markers:
            if marker in issue_text:
                return ("error", f"ChatGPT reported an error: {marker}", None)

    try:
        body_text = page.locator("body").inner_text(timeout=1000)
    except PlaywrightError:
        return None

    if "Log in" in body_text or "Sign up" in body_text:
        return ("auth", "ChatGPT session is not logged in inside the remote-debug Chrome profile.", None)
    return None


def wait_for_new_images(
    page: Any,
    baseline_assistant_turn_count: int,
    baseline_page_image_keys: set[str],
    timeout_seconds: int,
) -> list[ImageCandidate]:
    deadline = time.monotonic() + timeout_seconds

    while time.monotonic() < deadline:
        current_assistant_turn_count = get_assistant_turn_count(page)
        issue = get_page_issue(
            page,
            include_latest_assistant=current_assistant_turn_count > baseline_assistant_turn_count,
        )
        if issue:
            kind, message, retry_after_seconds = issue
            if kind == "rate_limit":
                raise RateLimitError(message, retry_after_seconds or 125)
            raise BridgeError(message)

        current_page_candidates = get_image_candidates(page)
        current_page_keys = {candidate.key for candidate in current_page_candidates}
        if (
            current_assistant_turn_count > baseline_assistant_turn_count
            or bool(current_page_keys - baseline_page_image_keys)
        ):
            settled_candidates = wait_for_render_stability(page, deadline - time.monotonic())
            fresh_candidates = [
                candidate for candidate in settled_candidates if candidate.key not in baseline_page_image_keys
            ]
            return sorted(
                fresh_candidates or settled_candidates,
                key=lambda candidate: (candidate.dom_index, -candidate.score),
            )

        time.sleep(1.0)

    raise BridgeError(f"Timed out after {timeout_seconds} seconds waiting for a generated image.")


def fetch_image_from_page(page: Any, candidate: ImageCandidate) -> tuple[bytes, str]:
    target_url = candidate.href or candidate.src
    try:
        response = page.request.get(target_url)
        if response.ok:
            mime_type = response.headers.get("content-type", "image/png").split(";")[0].strip()
            return response.body(), mime_type
    except Exception:
        pass

    try:
        data_url = page.evaluate(
            """
async (url) => {
  const response = await fetch(url, { credentials: "include" });
  if (!response.ok) {
    throw new Error(`Fetch failed with status ${response.status}`);
  }
  const blob = await response.blob();
  return await new Promise((resolve, reject) => {
    const reader = new FileReader();
    reader.onerror = () => reject(new Error("FileReader failed"));
    reader.onloadend = () => resolve(reader.result);
    reader.readAsDataURL(blob);
  });
}
""",
            target_url,
        )
        if not isinstance(data_url, str):
            raise BridgeError("ChatGPT image fetch did not return a data URL.")
        match = re.match(r"^data:(image/[^;]+);base64,(.+)$", data_url, re.DOTALL)
        if not match:
            raise BridgeError("Unexpected data URL format returned from the page.")
        mime_type = match.group(1)
        payload = base64.b64decode(match.group(2))
        return payload, mime_type
    except Exception:
        screenshot_bytes = get_candidate_locator(page, candidate).screenshot(type="png")
        return screenshot_bytes, "image/png"


def save_output(image_bytes: bytes, mime_type: str, file_stem: str) -> Path:
    extension_map = {
        "image/png": ".png",
        "image/jpeg": ".jpg",
        "image/webp": ".webp",
    }
    extension = extension_map.get(mime_type.lower(), ".png")
    filename = f"{file_stem}{extension}"
    output_path = OUTPUT_DIR / filename
    output_path.write_bytes(image_bytes)
    return output_path


def open_generation_page(playwright: Any) -> tuple[Any, Any, Any]:
    browser = playwright.chromium.connect_over_cdp(DEFAULT_CDP_URL)
    if not browser.contexts:
        raise BridgeError("No Chrome context was exposed over CDP. Launch Chrome with remote debugging first.")

    context = browser.contexts[0]
    page = context.new_page()
    page.set_default_timeout(DEFAULT_PAGE_TIMEOUT_MS)
    page.goto(DEFAULT_BASE_URL, wait_until="domcontentloaded")
    wait_for_composer(page)
    return browser, context, page


def generate_on_page(page: Any, prompt: str, timeout_seconds: int, image_paths: list[str] | None = None) -> list[GeneratedAsset]:
    baseline_assistant_turn_count = get_assistant_turn_count(page)
    baseline_page_image_keys = {candidate.key for candidate in get_image_candidates(page)}
    fill_and_send_prompt(page, prompt, image_paths=image_paths)
    candidates = wait_for_new_images(page, baseline_assistant_turn_count, baseline_page_image_keys, timeout_seconds)

    prompt_stem = f"{now_stamp()}-{slugify(prompt)}"
    assets: list[GeneratedAsset] = []
    seen_hashes: set[str] = set()
    for index, candidate in enumerate(candidates, start=1):
        image_bytes, mime_type = fetch_image_from_page(page, candidate)
        content_hash = hashlib.sha256(image_bytes).hexdigest()
        if content_hash in seen_hashes:
            continue
        seen_hashes.add(content_hash)
        file_stem = prompt_stem if len(candidates) == 1 else f"{prompt_stem}-{index:02d}"
        output_path = save_output(image_bytes, mime_type, file_stem)
        assets.append(GeneratedAsset(path=output_path, mime_type=mime_type, data=image_bytes))
    if not assets:
        raise BridgeError("Generated images were detected in the DOM, but no unique image files could be extracted.")
    return assets


def connect_and_generate(prompt: str, timeout_seconds: int, image_paths: list[str] | None = None) -> list[GeneratedAsset]:
    with sync_playwright() as playwright:
        browser, _context, page = open_generation_page(playwright)
        try:
            return generate_on_page(page, prompt, timeout_seconds, image_paths=image_paths)
        finally:
            page.close()


@app.errorhandler(BridgeError)
def handle_bridge_error(error: BridgeError):
    status_code = 401 if "Unauthorized" in str(error) else 400
    return jsonify({"error": str(error)}), status_code


@app.errorhandler(Exception)
def handle_unexpected_error(error: Exception):
    return jsonify({"error": str(error)}), 500


@app.get("/health")
def health():
    return jsonify(
        {
            "ok": True,
            "cdp_url": DEFAULT_CDP_URL,
            "base_url": DEFAULT_BASE_URL,
            "output_dir": str(OUTPUT_DIR),
            "auth_configured": bool(AUTH_TOKEN),
        }
    )


@app.post("/generate")
def generate():
    require_auth()

    payload = request.get_json(silent=True) or {}
    prompt = str(payload.get("prompt", "")).strip()
    if not prompt:
        raise BridgeError("Missing prompt.")

    timeout_seconds = coerce_timeout(payload)
    image_paths = coerce_image_paths(payload)

    if not generation_lock.acquire(blocking=False):
        return jsonify({"error": "Bridge is busy with another generation request."}), 409

    try:
        assets = connect_and_generate(prompt, timeout_seconds, image_paths=image_paths)
    finally:
        generation_lock.release()

    mode = request.args.get("mode", "").strip().lower()
    if mode == "json":
        return jsonify(
            {
                "ok": True,
                "prompt": prompt,
                "image_paths": image_paths,
                "count": len(assets),
                "files": [
                    {
                        "saved_file": str(asset.path),
                        "mime_type": asset.mime_type,
                        "size_bytes": asset.size_bytes,
                    }
                    for asset in assets
                ],
            }
        )

    if len(assets) == 1:
        asset = assets[0]
        response = send_file(
            io.BytesIO(asset.data),
            mimetype=asset.mime_type,
            as_attachment=True,
            download_name=asset.path.name,
        )
        response.headers["X-Bridge-Saved-File"] = str(asset.path)
        response.headers["X-Bridge-Mime-Type"] = asset.mime_type
        return response

    archive_buffer = io.BytesIO()
    with zipfile.ZipFile(archive_buffer, mode="w", compression=zipfile.ZIP_DEFLATED) as archive:
        for asset in assets:
            archive.writestr(asset.path.name, asset.data)
    archive_buffer.seek(0)

    response = send_file(
        archive_buffer,
        mimetype="application/zip",
        as_attachment=True,
        download_name=f"{now_stamp()}-chatgpt-images.zip",
    )
    response.headers["X-Bridge-Image-Count"] = str(len(assets))
    return response


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Local ChatGPT image bridge server.")
    parser.add_argument("--host", default="127.0.0.1", help="Host interface to bind.")
    parser.add_argument("--port", default=5000, type=int, help="Port to listen on.")
    parser.add_argument("--debug", action="store_true", help="Run Flask in debug mode.")
    return parser.parse_args()


def main() -> None:
    if not AUTH_TOKEN:
        raise SystemExit(
            "CHATGPT_BRIDGE_TOKEN is not set. Set it before starting the server."
        )
    args = parse_args()
    app.run(host=args.host, port=args.port, debug=args.debug, threaded=True)


if __name__ == "__main__":
    main()
