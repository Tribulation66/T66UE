#!/usr/bin/env python3
"""Generate explicit runtime main-menu layers from SourceAssets/NewMMBackground.png."""

from __future__ import annotations

import math
import random
from collections import deque
from pathlib import Path

from PIL import Image, ImageDraw, ImageFilter


CANVAS_W = 1920
CANVAS_H = 1080
ZOOM_SCALE = 0.92
FIRE_LAYER_COUNT = 10
STAR_LAYER_COUNT = 8
WATER_LAYER_COUNT = 6

ORIG_RING_CX = 1015.0
ORIG_RING_CY = 392.0
ORIG_RING_RX = 410.0
ORIG_RING_RY = 305.0
ORIG_WATER_SAFE_Y = 880.0


def clamp_byte(value: float) -> int:
    return max(0, min(255, int(round(value))))


def project_root() -> Path:
    return Path(__file__).resolve().parents[1]


def make_cover_source(source_path: Path) -> Image.Image:
    source = Image.open(source_path).convert("RGB")
    source_w, source_h = source.size
    target_ratio = CANVAS_W / CANVAS_H
    source_ratio = source_w / source_h

    if source_ratio > target_ratio:
        crop_w = int(round(source_h * target_ratio))
        left = (source_w - crop_w) // 2
        crop = source.crop((left, 0, left + crop_w, source_h))
    else:
        crop_h = int(round(source_w / target_ratio))
        top = (source_h - crop_h) // 2
        crop = source.crop((0, top, source_w, top + crop_h))

    return crop.resize((CANVAS_W, CANVAS_H), Image.Resampling.LANCZOS).convert("RGBA")


def make_canvas_source(source_path: Path) -> tuple[Image.Image, tuple[float, float, float, float, int]]:
    cover = make_cover_source(source_path)
    backdrop = cover.filter(ImageFilter.GaussianBlur(radius=5)).point(lambda value: int(value * 0.48))
    zoom_w = int(round(CANVAS_W * ZOOM_SCALE))
    zoom_h = int(round(CANVAS_H * ZOOM_SCALE))
    offset_x = (CANVAS_W - zoom_w) // 2
    offset_y = (CANVAS_H - zoom_h) // 2
    zoomed = cover.resize((zoom_w, zoom_h), Image.Resampling.LANCZOS)

    canvas = backdrop.copy()
    canvas.alpha_composite(zoomed, (offset_x, offset_y))

    ring_cx = offset_x + ORIG_RING_CX * ZOOM_SCALE
    ring_cy = offset_y + ORIG_RING_CY * ZOOM_SCALE
    ring_rx = ORIG_RING_RX * ZOOM_SCALE
    ring_ry = ORIG_RING_RY * ZOOM_SCALE
    water_y = int(round(offset_y + ORIG_WATER_SAFE_Y * ZOOM_SCALE))
    return canvas, (ring_cx, ring_cy, ring_rx, ring_ry, water_y)


def ellipse_distance(x: float, y: float, ring: tuple[float, float, float, float, int]) -> float:
    cx, cy, rx, ry, _ = ring
    return ((x - cx) / rx) ** 2 + ((y - cy) / ry) ** 2


def normal_at(x: float, y: float, ring: tuple[float, float, float, float, int]) -> tuple[float, float]:
    cx, cy, rx, ry, _ = ring
    nx = (x - cx) / (rx * rx)
    ny = (y - cy) / (ry * ry)
    length = math.hypot(nx, ny)
    if length <= 0.0:
        return 0.0, -1.0
    return nx / length, ny / length


def is_fire_pixel(r: int, g: int, b: int) -> bool:
    return r > 108 and r > g * 1.16 and g > b * 1.04 and (r - b) > 70


def make_foreground_mask(ring: tuple[float, float, float, float, int]) -> Image.Image:
    _, _, _, _, water_y = ring
    mask = Image.new("L", (CANVAS_W, CANVAS_H), 0)
    draw = ImageDraw.Draw(mask)

    # Broad but intentional occluder: statue, shoulders, and pyramid. This is
    # drawn above all animated overlays so effects cannot crawl across the idol.
    draw.ellipse((760, 280, 1138, 610), fill=255)
    draw.polygon(
        [
            (760, 605),
            (830, 455),
            (1105, 470),
            (1210, 650),
            (1135, 810),
            (790, 815),
        ],
        fill=255,
    )
    draw.polygon([(735, 655), (1145, 565), (1260, 812), (760, 835)], fill=255)
    draw.polygon(
        [
            (500, water_y + 8),
            (1425, water_y + 8),
            (1275, 650),
            (1135, 560),
            (885, 552),
            (650, 655),
        ],
        fill=255,
    )
    draw.polygon([(590, 830), (1310, 810), (1420, water_y + 6), (505, water_y + 6)], fill=255)

    mask = mask.filter(ImageFilter.GaussianBlur(radius=1.0))
    return mask.point(lambda value: 255 if value > 18 else 0)


def make_foreground_occluder(base: Image.Image, foreground_mask: Image.Image) -> Image.Image:
    occluder = base.copy()
    occluder.putalpha(foreground_mask)
    return occluder


def clear_with_mask(layer: Image.Image, mask: Image.Image) -> Image.Image:
    out = layer.copy()
    pixels = out.load()
    mask_pixels = mask.load()
    for y in range(CANVAS_H):
        for x in range(CANVAS_W):
            if mask_pixels[x, y] > 0:
                pixels[x, y] = (0, 0, 0, 0)
    return out


def multiply_alpha(image: Image.Image, factor: float) -> Image.Image:
    out = image.copy()
    pixels = out.load()
    for y in range(out.height):
        for x in range(out.width):
            r, g, b, a = pixels[x, y]
            if a:
                pixels[x, y] = (r, g, b, clamp_byte(a * factor))
    return out


def collect_fire_anchors(base: Image.Image, ring: tuple[float, float, float, float, int], foreground_mask: Image.Image) -> list[tuple[int, int, int, int, int]]:
    pixels = base.load()
    foreground_pixels = foreground_mask.load()
    _, _, _, _, water_y = ring
    anchors: list[tuple[int, int, int, int, int]] = []

    for y in range(80, water_y - 34):
        for x in range(420, 1500):
            if foreground_pixels[x, y] > 0:
                continue
            r, g, b, _ = pixels[x, y]
            if not is_fire_pixel(r, g, b):
                continue
            d = ellipse_distance(x, y, ring)
            if d < 0.32 or d > 2.10:
                continue
            anchors.append((x, y, r, g, b))
    return anchors


def make_fire_core(base: Image.Image, ring: tuple[float, float, float, float, int], foreground_mask: Image.Image) -> Image.Image:
    out = Image.new("RGBA", base.size, (0, 0, 0, 0))
    out_pixels = out.load()

    for x, y, r, g, b in collect_fire_anchors(base, ring, foreground_mask):
        heat = max(0.0, min(1.0, (r - 105) / 150.0))
        alpha = clamp_byte(48 + heat * 132)
        if alpha <= 16:
            continue

        out_pixels[x, y] = (
            clamp_byte(r * 1.18 + 24),
            clamp_byte(g * 1.02 + 18),
            clamp_byte(b * 0.48 + 6),
            min(alpha, 190),
        )

    return out.filter(ImageFilter.GaussianBlur(radius=0.18))


def draw_curved_flame(
    draw: ImageDraw.ImageDraw,
    base_x: float,
    base_y: float,
    nx: float,
    ny: float,
    rng: random.Random,
    alpha_scale: float,
) -> None:
    tangent_x = -ny
    tangent_y = nx
    length = rng.uniform(18.0, 54.0) * alpha_scale
    bend = rng.uniform(-12.0, 12.0)
    lift = rng.uniform(4.0, 16.0)
    points = []

    for step in range(6):
        t = step / 5.0
        curve = math.sin(t * math.pi)
        px = base_x + nx * length * t + tangent_x * bend * curve
        py = base_y + ny * length * t + tangent_y * bend * curve - lift * t
        points.append((px, py))

    outer_alpha = rng.randint(74, 170)
    inner_alpha = rng.randint(86, 190)
    draw.line(points, fill=(255, rng.randint(66, 118), rng.randint(0, 24), outer_alpha), width=rng.randint(2, 4))
    draw.line(points[1:], fill=(255, rng.randint(175, 232), rng.randint(36, 80), inner_alpha), width=rng.randint(1, 2))


def make_fire_plume_layer(
    fire_anchors: list[tuple[int, int, int, int, int]],
    ring: tuple[float, float, float, float, int],
    foreground_mask: Image.Image,
    seed: int,
) -> Image.Image:
    cx, cy, rx, ry, water_y = ring
    rng = random.Random(seed)
    layer = Image.new("RGBA", (CANVAS_W, CANVAS_H), (0, 0, 0, 0))
    draw = ImageDraw.Draw(layer, "RGBA")
    foreground_pixels = foreground_mask.load()
    left_fire_anchors = [anchor for anchor in fire_anchors if anchor[0] < cx - 20]
    right_fire_anchors = [anchor for anchor in fire_anchors if anchor[0] > cx + 20]

    if not fire_anchors:
        return layer

    for index in range(214):
        if right_fire_anchors and index % 2 == 1:
            anchor_x, anchor_y, _, _, _ = right_fire_anchors[rng.randrange(len(right_fire_anchors))]
        elif left_fire_anchors:
            anchor_x, anchor_y, _, _, _ = left_fire_anchors[rng.randrange(len(left_fire_anchors))]
        else:
            anchor_x, anchor_y, _, _, _ = fire_anchors[rng.randrange(len(fire_anchors))]
        base_x = float(anchor_x)
        base_y = float(anchor_y)
        if base_y > water_y - 48:
            continue
        ix = int(round(base_x))
        iy = int(round(base_y))
        if ix < 0 or ix >= CANVAS_W or iy < 0 or iy >= CANVAS_H or foreground_pixels[ix, iy] > 0:
            continue

        nx, ny = normal_at(base_x, base_y, ring)
        if ny > 0.72:
            continue

        draw_curved_flame(draw, base_x, base_y, nx, ny, rng, rng.uniform(0.68, 1.18))

    pixels = layer.load()
    for y in range(CANVAS_H):
        if y >= water_y - 24:
            for x in range(CANVAS_W):
                pixels[x, y] = (0, 0, 0, 0)
            continue
        for x in range(CANVAS_W):
            r, g, b, a = pixels[x, y]
            if a == 0:
                continue
            d = ellipse_distance(x, y, ring)
            if d < 0.28 or d > 2.18 or foreground_pixels[x, y] > 0:
                pixels[x, y] = (0, 0, 0, 0)
    return layer.filter(ImageFilter.GaussianBlur(radius=0.30))


def write_fire_layers(base: Image.Image, ring: tuple[float, float, float, float, int], foreground_mask: Image.Image, out_dir: Path) -> None:
    core = make_fire_core(base, ring, foreground_mask)
    fire_anchors = collect_fire_anchors(base, ring, foreground_mask)
    layers: list[Image.Image] = [
        multiply_alpha(core, 0.92),
        multiply_alpha(core.filter(ImageFilter.GaussianBlur(radius=2.6)), 0.46),
    ]
    for index in range(FIRE_LAYER_COUNT - len(layers)):
        layers.append(Image.new("RGBA", base.size, (0, 0, 0, 0)))

    for index, layer in enumerate(layers):
        layer.save(out_dir / f"main_menu_newmm_fire_layer_{index:02d}.png")


def is_star_candidate(r: int, g: int, b: int) -> bool:
    brightness = max(r, g, b)
    contrast = brightness - min(r, g, b)
    return brightness >= 126 and contrast <= 112 and r >= 78 and g >= 78 and b >= 70


def find_star_centers(base: Image.Image, foreground_mask: Image.Image, ring: tuple[float, float, float, float, int]) -> list[tuple[int, int, int]]:
    pixels = base.load()
    foreground_pixels = foreground_mask.load()
    _, _, _, _, water_y = ring
    candidate = bytearray(CANVAS_W * CANVAS_H)

    for y in range(32, min(water_y, 870)):
        for x in range(54, CANVAS_W - 54):
            if foreground_pixels[x, y] > 0:
                continue
            if ellipse_distance(x, y, ring) < 0.78:
                continue
            r, g, b, _ = pixels[x, y]
            if is_star_candidate(r, g, b):
                candidate[y * CANVAS_W + x] = 1

    visited = bytearray(CANVAS_W * CANVAS_H)
    centers: list[tuple[int, int, int]] = []
    for y in range(32, min(water_y, 870)):
        for x in range(54, CANVAS_W - 54):
            idx = y * CANVAS_W + x
            if not candidate[idx] or visited[idx]:
                continue

            queue: deque[tuple[int, int]] = deque([(x, y)])
            visited[idx] = 1
            count = 0
            sum_x = 0
            sum_y = 0
            max_brightness = 0
            min_x = x
            max_x = x
            min_y = y
            max_y = y

            while queue:
                qx, qy = queue.popleft()
                count += 1
                sum_x += qx
                sum_y += qy
                min_x = min(min_x, qx)
                max_x = max(max_x, qx)
                min_y = min(min_y, qy)
                max_y = max(max_y, qy)
                r, g, b, _ = pixels[qx, qy]
                max_brightness = max(max_brightness, max(r, g, b))

                for nx, ny in ((qx + 1, qy), (qx - 1, qy), (qx, qy + 1), (qx, qy - 1)):
                    if nx < 54 or nx >= CANVAS_W - 54 or ny < 32 or ny >= min(water_y, 870):
                        continue
                    nidx = ny * CANVAS_W + nx
                    if candidate[nidx] and not visited[nidx]:
                        visited[nidx] = 1
                        queue.append((nx, ny))

            width = max_x - min_x + 1
            height = max_y - min_y + 1
            if 1 <= count <= 96 and width <= 26 and height <= 26:
                centers.append((round(sum_x / count), round(sum_y / count), max_brightness))

    return centers


def draw_star_sparkle(draw: ImageDraw.ImageDraw, x: int, y: int, brightness: int, rng: random.Random) -> None:
    radius = 2 + (brightness // 92) + rng.randint(0, 2)
    alpha = clamp_byte(128 + (brightness - 120) * 0.72)
    halo = max(1, radius // 2)
    draw.ellipse((x - halo, y - halo, x + halo, y + halo), fill=(255, 244, 196, int(alpha * 0.36)))
    draw.line([(x - radius, y), (x + radius, y)], fill=(255, 248, 214, alpha), width=1)
    draw.line([(x, y - radius), (x, y + radius)], fill=(255, 248, 214, alpha), width=1)
    if radius >= 4:
        diag = max(1, radius // 2)
        draw.line([(x - diag, y - diag), (x + diag, y + diag)], fill=(230, 226, 255, int(alpha * 0.68)), width=1)
        draw.line([(x - diag, y + diag), (x + diag, y - diag)], fill=(230, 226, 255, int(alpha * 0.68)), width=1)


def write_star_layers(base: Image.Image, ring: tuple[float, float, float, float, int], foreground_mask: Image.Image, out_dir: Path) -> None:
    centers = find_star_centers(base, foreground_mask, ring)
    outputs = [Image.new("RGBA", base.size, (0, 0, 0, 0)) for _ in range(STAR_LAYER_COUNT)]
    draws = [ImageDraw.Draw(image, "RGBA") for image in outputs]

    for index, (x, y, brightness) in enumerate(centers):
        layer_index = (index * 5 + x // 13 + y // 17) % STAR_LAYER_COUNT
        rng = random.Random(x * 73856093 ^ y * 19349663)
        draw_star_sparkle(draws[layer_index], x, y, brightness, rng)

    for index, image in enumerate(outputs):
        image.filter(ImageFilter.GaussianBlur(radius=0.08)).save(out_dir / f"main_menu_newmm_star_twinkle_{index:02d}.png")


def write_water_layers(base: Image.Image, ring: tuple[float, float, float, float, int], foreground_mask: Image.Image, out_dir: Path) -> None:
    _, _, _, _, water_y = ring
    outputs = [Image.new("RGBA", base.size, (0, 0, 0, 0)) for _ in range(WATER_LAYER_COUNT)]
    output_pixels = [image.load() for image in outputs]
    pixels = base.load()
    foreground_pixels = foreground_mask.load()

    for y in range(water_y + 4, CANVAS_H):
        vertical_bias = min(1.0, max(0.0, (y - water_y) / 120.0))
        wave = math.sin(y * 0.11)
        for x in range(CANVAS_W):
            if foreground_pixels[x, y] > 0:
                continue
            r, g, b, _ = pixels[x, y]
            brightness = max(r, g, b)
            if brightness < 24:
                continue

            reflection_score = (brightness - 22) * 2.55 + max(0, r - b) * 0.42 + max(0, b - r) * 0.26
            alpha = clamp_byte(reflection_score * vertical_bias)
            if alpha <= 10:
                continue
            group = ((x // 46) + int(wave * 3.0) + (y // 8) * 3) % WATER_LAYER_COUNT
            output_pixels[group][x, y] = (
                clamp_byte(r * 1.10 + 8),
                clamp_byte(g * 1.06 + 5),
                clamp_byte(b * 1.04 + 3),
                min(alpha, 220),
            )

    for index, image in enumerate(outputs):
        image.filter(ImageFilter.GaussianBlur(radius=0.24)).save(out_dir / f"main_menu_newmm_water_shimmer_{index:02d}.png")


def clean_generated_assets(out_dir: Path) -> None:
    for path in out_dir.glob("main_menu_newmm_*.png"):
        path.unlink()


def main() -> None:
    root = project_root()
    source_path = root / "SourceAssets" / "NewMMBackground.png"
    out_dir = root / "SourceAssets" / "UI" / "MasterLibrary" / "ScreenArt" / "MainMenu" / "NewMM"
    out_dir.mkdir(parents=True, exist_ok=True)
    clean_generated_assets(out_dir)

    base, ring = make_canvas_source(source_path)
    foreground_mask = make_foreground_mask(ring)
    foreground_occluder = make_foreground_occluder(base, foreground_mask)

    base.save(out_dir / "main_menu_newmm_base_1920.png")
    foreground_occluder.save(out_dir / "main_menu_newmm_foreground_occluder.png")
    write_fire_layers(base, ring, foreground_mask, out_dir)
    write_star_layers(base, ring, foreground_mask, out_dir)
    write_water_layers(base, ring, foreground_mask, out_dir)

    print(f"Generated NewMM hard-mask layers in {out_dir}")


if __name__ == "__main__":
    main()
