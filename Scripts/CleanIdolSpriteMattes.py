"""
Strip the baked edge matte and dark backdrop from live idol source sprites and refit the artwork on a square canvas.

This keeps the source art reproducible and ready for the existing import pipeline:
    python Scripts/CleanIdolSpriteMattes.py
"""

from __future__ import annotations

from collections import deque
from pathlib import Path

from PIL import Image


PROJECT_DIR = Path(__file__).resolve().parents[1]
SOURCE_DIR = PROJECT_DIR / "SourceAssets" / "IdolSprites"
TARGET_SIZE = 512
TARGET_PADDING = 16
WHITE_THRESHOLD = 240
DARK_THRESHOLD = 12
DARK_SEED_MARGIN = 28


def is_edge_white_matte(pixel: tuple[int, int, int, int]) -> bool:
    r, g, b, a = pixel
    return a > 0 and r >= WHITE_THRESHOLD and g >= WHITE_THRESHOLD and b >= WHITE_THRESHOLD


def is_edge_dark_backdrop(pixel: tuple[int, int, int, int]) -> bool:
    r, g, b, a = pixel
    return a > 0 and r <= DARK_THRESHOLD and g <= DARK_THRESHOLD and b <= DARK_THRESHOLD


def clear_edge_connected_pixels(
    image: Image.Image,
    predicate,
) -> bool:
    width, height = image.size
    pixels = image.load()
    visited = bytearray(width * height)
    queue: deque[tuple[int, int]] = deque()

    def try_enqueue(x: int, y: int) -> None:
        idx = y * width + x
        if visited[idx]:
            return
        visited[idx] = 1
        if predicate(pixels[x, y]):
            queue.append((x, y))

    for x in range(width):
        try_enqueue(x, 0)
        try_enqueue(x, height - 1)
    for y in range(height):
        try_enqueue(0, y)
        try_enqueue(width - 1, y)

    changed = False
    while queue:
        x, y = queue.popleft()
        r, g, b, a = pixels[x, y]
        if a != 0:
            pixels[x, y] = (r, g, b, 0)
            changed = True

        if x > 0:
            try_enqueue(x - 1, y)
        if x + 1 < width:
            try_enqueue(x + 1, y)
        if y > 0:
            try_enqueue(x, y - 1)
        if y + 1 < height:
            try_enqueue(x, y + 1)

    return changed


def clear_edge_connected_white_matte(image: Image.Image) -> bool:
    return clear_edge_connected_pixels(image, is_edge_white_matte)


def clear_edge_connected_dark_backdrop(image: Image.Image) -> bool:
    alpha = image.getchannel("A")
    bbox = alpha.getbbox()
    if not bbox:
        return False

    cropped = image.crop(bbox)
    width, height = cropped.size
    pixels = cropped.load()
    alpha_pixels = cropped.getchannel("A").load()
    visited = bytearray(width * height)
    queue: deque[tuple[int, int]] = deque()

    def try_enqueue(x: int, y: int) -> None:
        idx = y * width + x
        if visited[idx]:
            return
        visited[idx] = 1
        if is_edge_dark_backdrop(pixels[x, y]):
            queue.append((x, y))

    for y in range(height):
        for x in range(width):
            if alpha_pixels[x, y] <= 0:
                continue
            if (
                x < DARK_SEED_MARGIN
                or y < DARK_SEED_MARGIN
                or x >= width - DARK_SEED_MARGIN
                or y >= height - DARK_SEED_MARGIN
            ):
                try_enqueue(x, y)

    changed = False
    while queue:
        x, y = queue.popleft()
        r, g, b, a = pixels[x, y]
        if a != 0:
            pixels[x, y] = (r, g, b, 0)
            changed = True

        if x > 0:
            try_enqueue(x - 1, y)
        if x + 1 < width:
            try_enqueue(x + 1, y)
        if y > 0:
            try_enqueue(x, y - 1)
        if y + 1 < height:
            try_enqueue(x, y + 1)

    if changed:
        image.paste(cropped, bbox[:2])
    return changed


def refit_to_canvas(image: Image.Image) -> tuple[Image.Image, bool]:
    alpha = image.getchannel("A")
    bbox = alpha.getbbox()
    if not bbox:
        return image, False

    cropped = image.crop(bbox)
    inner_size = TARGET_SIZE - (TARGET_PADDING * 2)
    scale = min(inner_size / cropped.width, inner_size / cropped.height)

    resized_size = (
        max(1, int(round(cropped.width * scale))),
        max(1, int(round(cropped.height * scale))),
    )
    resized = cropped.resize(resized_size, Image.Resampling.LANCZOS)

    canvas = Image.new("RGBA", (TARGET_SIZE, TARGET_SIZE), (0, 0, 0, 0))
    offset = (
        (TARGET_SIZE - resized.width) // 2,
        (TARGET_SIZE - resized.height) // 2,
    )
    canvas.alpha_composite(resized, dest=offset)

    changed = bbox != (0, 0, image.width, image.height) or resized.size != image.size
    return canvas, changed


def process_file(path: Path) -> bool:
    original = Image.open(path).convert("RGBA")
    working = original.copy()

    matte_changed = clear_edge_connected_white_matte(working)
    backdrop_changed = clear_edge_connected_dark_backdrop(working)
    refit_image, refit_changed = refit_to_canvas(working)

    if not matte_changed and not backdrop_changed and not refit_changed:
        return False

    refit_image.save(path)
    return True


def main() -> None:
    if not SOURCE_DIR.is_dir():
        raise RuntimeError(f"Idol sprite source directory not found: {SOURCE_DIR}")

    processed = 0
    changed = 0
    for path in sorted(SOURCE_DIR.glob("*.png")):
        processed += 1
        if process_file(path):
            changed += 1
            print(f"updated {path.name}")

    print(f"processed {processed} idol sprites")
    print(f"updated {changed} idol sprites")


if __name__ == "__main__":
    main()
