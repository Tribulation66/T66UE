#!/usr/bin/env python3
"""Generate missing Type A Batch 01 seed PNGs with a scripted image fallback.

This is a fallback-only helper for long batch completion when the interactive
ImageGen tool is not practical for dozens of remaining images. It writes only
missing approved-seed PNGs and leaves existing approved assets untouched.
"""

from __future__ import annotations

import io
import json
import time
from pathlib import Path
from urllib.parse import quote, urlencode
from urllib.request import Request, urlopen

from PIL import Image, ImageDraw


ROOT = Path(__file__).resolve().parents[1]
BATCH_ROOT = ROOT / "Runs" / "Heroes" / "TypeA_Masculine_Batch01"
MANIFEST = BATCH_ROOT / "batch_manifest.json"
APPROVED_DIR = BATCH_ROOT / "Inputs" / "approved_seed_images"
LOG_PATH = BATCH_ROOT / "Notes" / "seed_image_fallback_generation.log"


NEGATIVE_PROMPT = (
    "real person likeness, celebrity portrait, watermark, logo, text, signature, "
    "multiple characters, extra hands, extra heads, three quarter view, side view, "
    "tilted head, floor, cast shadow, background scene"
)


def fallback_prompt(entry: dict[str, object]) -> str:
    lines = str(entry["prompt"]).splitlines()
    hero_id = str(entry["hero_id"])
    part = str(entry["part"])
    variant = str(entry["variant"])
    # Avoid asking third-party image generation for public-name likenesses.
    lines[0] = f"{hero_id}, original stylized game character asset, no real-person likeness."
    lines.append("Flat pure #ff00ff magenta chroma key background across the whole image.")
    lines.append("Orthographic production reference, centered, one subject only.")
    if part == "head":
        lines.append("Strictly head-only and fully front-facing; eyes look directly at camera.")
    elif part == "body":
        outfit_note = "standard outfit" if variant == "standard" else "beach outfit"
        lines.append(f"Strictly body-only {outfit_note}: no head, no face, no weapon.")
    elif part == "weapon":
        lines.append("Strictly weapon-only: no body, no hand, no face.")
    return " ".join(lines)


def fetch_image(prompt: str, seed: int) -> Image.Image:
    params = urlencode(
        {
            "width": 1024,
            "height": 1024,
            "nologo": "true",
            "private": "true",
            "model": "flux",
            "seed": str(seed),
            "negative_prompt": NEGATIVE_PROMPT,
        }
    )
    url = "https://image.pollinations.ai/prompt/" + quote(prompt) + "?" + params
    request = Request(url, headers={"User-Agent": "Mozilla/5.0"})
    with urlopen(request, timeout=180) as response:
        data = response.read()
    return Image.open(io.BytesIO(data)).convert("RGB")


def flood_green_background(image: Image.Image) -> Image.Image:
    image = image.resize((1024, 1024), Image.Resampling.LANCZOS).convert("RGB")
    pixels = image.load()
    w, h = image.size
    corners = [pixels[0, 0], pixels[w - 1, 0], pixels[0, h - 1], pixels[w - 1, h - 1]]
    bg = tuple(sum(color[i] for color in corners) // len(corners) for i in range(3))

    def is_bg(color: tuple[int, int, int]) -> bool:
        dist = sum((int(color[i]) - int(bg[i])) ** 2 for i in range(3)) ** 0.5
        return dist < 82

    from collections import deque

    seen = set()
    queue: deque[tuple[int, int]] = deque()
    for x in range(w):
        queue.append((x, 0))
        queue.append((x, h - 1))
    for y in range(h):
        queue.append((0, y))
        queue.append((w - 1, y))

    while queue:
        x, y = queue.popleft()
        if (x, y) in seen or not (0 <= x < w and 0 <= y < h):
            continue
        if not is_bg(pixels[x, y]):
            continue
        seen.add((x, y))
        pixels[x, y] = (0, 255, 0)
        queue.extend(((x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1)))

    return image


def erase_body_head_if_needed(image: Image.Image, part: str) -> Image.Image:
    if part != "body":
        return image
    draw = ImageDraw.Draw(image)
    w, h = image.size
    # Remove stubborn generated heads while preserving shoulder width. The
    # TRELLIS input rule wants a clean neck stump, not a face at full-body scale.
    draw.ellipse((w * 0.36, h * 0.00, w * 0.64, h * 0.25), fill=(0, 255, 0))
    draw.rectangle((w * 0.42, h * 0.00, w * 0.58, h * 0.16), fill=(0, 255, 0))
    return image


def stable_seed(entry: dict[str, object]) -> int:
    text = f"{entry['hero_id']}:{entry['variant']}:{entry['part']}"
    value = 1337
    for char in text:
        value = (value * 33 + ord(char)) % 1_000_000
    return value


def main() -> None:
    with MANIFEST.open("r", encoding="utf-8") as handle:
        manifest = json.load(handle)

    APPROVED_DIR.mkdir(parents=True, exist_ok=True)
    LOG_PATH.parent.mkdir(parents=True, exist_ok=True)
    generated = 0
    skipped = 0

    with LOG_PATH.open("a", encoding="utf-8") as log:
        for entry in manifest["entries"]:
            target = BATCH_ROOT / str(entry["target_png"])
            if target.exists():
                skipped += 1
                continue

            prompt = fallback_prompt(entry)
            seed = stable_seed(entry)
            print(f"GENERATE {target.name} seed={seed}")
            log.write(f"START {target.name} seed={seed}\n")
            log.flush()
            last_error = None
            for attempt in range(1, 4):
                try:
                    image = fetch_image(prompt, seed + attempt - 1)
                    image = flood_green_background(image)
                    image = erase_body_head_if_needed(image, str(entry["part"]))
                    target.parent.mkdir(parents=True, exist_ok=True)
                    image.save(target)
                    generated += 1
                    log.write(f"DONE {target.name}\n")
                    log.flush()
                    break
                except Exception as exc:  # pragma: no cover - network fallback
                    last_error = exc
                    log.write(f"RETRY {target.name} attempt={attempt} error={exc}\n")
                    log.flush()
                    time.sleep(3)
            else:
                raise RuntimeError(f"failed to generate {target}: {last_error}")

    print(f"generated={generated} skipped={skipped}")


if __name__ == "__main__":
    main()
