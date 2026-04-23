from __future__ import annotations

import argparse
import json
from collections import deque
from pathlib import Path

from PIL import Image


def flood_clear_background(image: Image.Image, threshold: int) -> Image.Image:
    rgba = image.convert("RGBA")
    width, height = rgba.size
    pixels = rgba.load()
    background = pixels[0, 0][:3]

    def is_background(x: int, y: int) -> bool:
        pixel = pixels[x, y]
        return max(abs(pixel[i] - background[i]) for i in range(3)) <= threshold

    visited = [[False for _ in range(width)] for _ in range(height)]
    queue: deque[tuple[int, int]] = deque()

    for x in range(width):
        queue.append((x, 0))
        queue.append((x, height - 1))
    for y in range(height):
        queue.append((0, y))
        queue.append((width - 1, y))

    while queue:
        x, y = queue.popleft()
        if x < 0 or y < 0 or x >= width or y >= height or visited[y][x]:
            continue
        visited[y][x] = True
        if not is_background(x, y):
            continue
        pixels[x, y] = (0, 0, 0, 0)
        queue.extend(((x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1)))

    return rgba


def build_mask(image: Image.Image, threshold: int) -> list[list[bool]]:
    rgba = image.convert("RGBA")
    width, height = rgba.size
    pixels = rgba.load()
    background = pixels[0, 0][:3]
    mask: list[list[bool]] = [[False for _ in range(width)] for _ in range(height)]

    for y in range(height):
        for x in range(width):
            pixel = pixels[x, y]
            if max(abs(pixel[i] - background[i]) for i in range(3)) > threshold:
                mask[y][x] = True

    return mask


def connected_components(mask: list[list[bool]]) -> list[tuple[int, int, int, int]]:
    height = len(mask)
    width = len(mask[0]) if height else 0
    visited = [[False for _ in range(width)] for _ in range(height)]
    boxes: list[tuple[int, int, int, int]] = []

    for y in range(height):
        for x in range(width):
            if not mask[y][x] or visited[y][x]:
                continue

            queue: deque[tuple[int, int]] = deque([(x, y)])
            visited[y][x] = True
            min_x = max_x = x
            min_y = max_y = y

            while queue:
                cx, cy = queue.popleft()
                min_x = min(min_x, cx)
                min_y = min(min_y, cy)
                max_x = max(max_x, cx)
                max_y = max(max_y, cy)

                for nx, ny in ((cx + 1, cy), (cx - 1, cy), (cx, cy + 1), (cx, cy - 1)):
                    if nx < 0 or ny < 0 or nx >= width or ny >= height:
                        continue
                    if visited[ny][nx] or not mask[ny][nx]:
                        continue
                    visited[ny][nx] = True
                    queue.append((nx, ny))

            boxes.append((min_x, min_y, max_x + 1, max_y + 1))

    return boxes


def sort_boxes(boxes: list[tuple[int, int, int, int]]) -> list[tuple[int, int, int, int]]:
    return sorted(boxes, key=lambda box: (box[1], box[0], box[3] - box[1], box[2] - box[0]))


def main() -> int:
    parser = argparse.ArgumentParser(description="Auto-slice a UI sprite board with a flat background.")
    parser.add_argument("input_board", type=Path)
    parser.add_argument("output_dir", type=Path)
    parser.add_argument("--manifest", type=Path)
    parser.add_argument("--prefix", default="slice")
    parser.add_argument("--threshold", type=int, default=10)
    parser.add_argument("--min-width", type=int, default=16)
    parser.add_argument("--min-height", type=int, default=16)
    parser.add_argument("--min-area", type=int, default=512)
    args = parser.parse_args()

    image = Image.open(args.input_board).convert("RGBA")
    mask = build_mask(image, args.threshold)
    boxes = connected_components(mask)
    boxes = [
        box
        for box in boxes
        if (box[2] - box[0]) >= args.min_width
        and (box[3] - box[1]) >= args.min_height
        and ((box[2] - box[0]) * (box[3] - box[1])) >= args.min_area
    ]
    boxes = sort_boxes(boxes)

    args.output_dir.mkdir(parents=True, exist_ok=True)
    manifest_items: list[dict[str, object]] = []

    for index, box in enumerate(boxes, start=1):
        crop = image.crop(box)
        crop = flood_clear_background(crop, threshold=args.threshold)
        bounds = crop.getbbox()
        if bounds is not None:
            crop = crop.crop(bounds)

        name = f"{args.prefix}_{index:02d}"
        output_path = args.output_dir / f"{name}.png"
        crop.save(output_path)

        manifest_items.append(
            {
                "name": name,
                "source": args.input_board.as_posix(),
                "box": [int(v) for v in box],
                "output": output_path.as_posix(),
                "size": [crop.width, crop.height],
            }
        )

    manifest_path = args.manifest or (args.output_dir / f"{args.prefix}_manifest.json")
    manifest_path.write_text(json.dumps({"source": args.input_board.as_posix(), "items": manifest_items}, indent=2))

    print(manifest_path)
    print(len(manifest_items))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
