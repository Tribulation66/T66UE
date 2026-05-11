from __future__ import annotations

import csv
import hashlib
import math
import random
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable

from PIL import Image, ImageDraw, ImageFilter


ROOT_DIR = Path(__file__).resolve().parents[4]
OUTPUT_DIR = Path(__file__).resolve().parent
MAPS_CSV = ROOT_DIR / "Content" / "TD" / "Data" / "T66TD_Maps.csv"
SIZE = (1920, 1080)


Color = tuple[int, int, int]
Point = tuple[float, float]


@dataclass(frozen=True)
class ThemePalette:
    background_top: Color
    background_bottom: Color
    haze: Color
    accent: Color
    path_outer: Color
    path_fill: Color
    path_inner: Color
    pad_outer: Color
    pad_inner: Color


THEMES: dict[str, ThemePalette] = {
    "Dungeon": ThemePalette(
        background_top=(22, 8, 11),
        background_bottom=(74, 24, 18),
        haze=(226, 117, 45),
        accent=(242, 191, 121),
        path_outer=(35, 18, 14),
        path_fill=(171, 120, 77),
        path_inner=(228, 204, 164),
        pad_outer=(244, 156, 82),
        pad_inner=(255, 224, 170),
    ),
    "Forest": ThemePalette(
        background_top=(8, 24, 13),
        background_bottom=(38, 68, 31),
        haze=(113, 188, 92),
        accent=(219, 224, 136),
        path_outer=(14, 32, 18),
        path_fill=(108, 138, 79),
        path_inner=(176, 200, 128),
        pad_outer=(137, 214, 112),
        pad_inner=(222, 246, 172),
    ),
    "Ocean": ThemePalette(
        background_top=(5, 18, 34),
        background_bottom=(16, 74, 92),
        haze=(74, 192, 208),
        accent=(230, 208, 151),
        path_outer=(6, 30, 43),
        path_fill=(73, 123, 139),
        path_inner=(171, 223, 220),
        pad_outer=(88, 206, 221),
        pad_inner=(200, 246, 238),
    ),
    "Martian": ThemePalette(
        background_top=(34, 17, 13),
        background_bottom=(109, 56, 28),
        haze=(236, 138, 61),
        accent=(239, 208, 137),
        path_outer=(43, 19, 13),
        path_fill=(151, 94, 57),
        path_inner=(230, 176, 120),
        pad_outer=(237, 148, 81),
        pad_inner=(255, 217, 155),
    ),
    "Hell": ThemePalette(
        background_top=(15, 8, 11),
        background_bottom=(83, 9, 17),
        haze=(255, 104, 28),
        accent=(255, 196, 108),
        path_outer=(22, 10, 15),
        path_fill=(122, 42, 35),
        path_inner=(239, 150, 97),
        pad_outer=(255, 100, 52),
        pad_inner=(255, 226, 158),
    ),
}


MAP_LAYOUTS: dict[str, list[list[Point]]] = {
    "Map_Easy_01": [[(0.08, 0.14), (0.88, 0.14), (0.88, 0.84), (0.18, 0.84), (0.18, 0.28), (0.74, 0.28), (0.74, 0.72), (0.31, 0.72), (0.31, 0.42), (0.60, 0.42), (0.60, 0.58), (0.46, 0.58)]],
    "Map_Easy_02": [[(0.09, 0.20), (0.33, 0.20), (0.47, 0.30), (0.62, 0.30), (0.79, 0.47), (0.79, 0.73), (0.61, 0.84), (0.24, 0.84)]],
    "Map_Easy_03": [[(0.09, 0.22), (0.36, 0.22), (0.52, 0.44), (0.70, 0.44), (0.70, 0.74), (0.53, 0.74), (0.53, 0.58), (0.30, 0.58), (0.30, 0.84)]],
    "Map_Easy_04": [[(0.10, 0.18), (0.76, 0.18), (0.76, 0.34), (0.24, 0.34), (0.24, 0.50), (0.82, 0.50), (0.82, 0.67), (0.35, 0.67), (0.35, 0.84), (0.68, 0.84)]],
    "Map_Medium_01": [
        [(0.07, 0.18), (0.33, 0.18), (0.45, 0.36), (0.58, 0.36), (0.70, 0.52), (0.70, 0.82), (0.47, 0.82)],
        [(0.07, 0.78), (0.26, 0.78), (0.40, 0.61), (0.58, 0.61), (0.70, 0.52)],
    ],
    "Map_Medium_02": [
        [(0.10, 0.21), (0.27, 0.21), (0.37, 0.36), (0.37, 0.63), (0.55, 0.77), (0.80, 0.77)],
        [(0.10, 0.77), (0.26, 0.77), (0.45, 0.57), (0.45, 0.30), (0.60, 0.18), (0.82, 0.18)],
    ],
    "Map_Medium_03": [
        [(0.16, 0.22), (0.84, 0.22), (0.84, 0.78), (0.16, 0.78), (0.16, 0.22)],
        [(0.16, 0.50), (0.38, 0.50), (0.50, 0.36), (0.62, 0.50), (0.84, 0.50)],
    ],
    "Map_Medium_04": [
        [(0.10, 0.18), (0.24, 0.18), (0.37, 0.31), (0.50, 0.31), (0.63, 0.46), (0.63, 0.70), (0.84, 0.70)],
        [(0.12, 0.82), (0.28, 0.82), (0.40, 0.64), (0.53, 0.64), (0.66, 0.46)],
    ],
    "Map_Hard_01": [
        [(0.10, 0.18), (0.38, 0.18), (0.50, 0.34), (0.50, 0.74), (0.78, 0.74)],
        [(0.10, 0.74), (0.31, 0.74), (0.42, 0.56), (0.50, 0.56)],
    ],
    "Map_Hard_02": [
        [(0.11, 0.22), (0.81, 0.22), (0.81, 0.79), (0.19, 0.79), (0.19, 0.39), (0.61, 0.39), (0.61, 0.62), (0.39, 0.62)],
        [(0.08, 0.52), (0.19, 0.52)],
    ],
    "Map_Hard_03": [
        [(0.10, 0.18), (0.46, 0.18), (0.61, 0.38), (0.61, 0.77), (0.85, 0.77)],
        [(0.10, 0.76), (0.28, 0.76), (0.43, 0.58), (0.61, 0.58)],
    ],
    "Map_Hard_04": [
        [(0.08, 0.18), (0.26, 0.18), (0.36, 0.32), (0.58, 0.32), (0.70, 0.50), (0.70, 0.82)],
        [(0.08, 0.50), (0.22, 0.50), (0.36, 0.64), (0.54, 0.64), (0.70, 0.50)],
        [(0.08, 0.82), (0.28, 0.82), (0.41, 0.68), (0.54, 0.68)],
    ],
    "Map_VeryHard_01": [
        [(0.09, 0.25), (0.34, 0.25), (0.48, 0.40), (0.68, 0.40), (0.82, 0.62), (0.82, 0.84)],
        [(0.09, 0.69), (0.30, 0.69), (0.43, 0.54), (0.60, 0.54), (0.72, 0.68)],
    ],
    "Map_VeryHard_02": [
        [(0.08, 0.18), (0.26, 0.18), (0.39, 0.34), (0.55, 0.34), (0.69, 0.52), (0.86, 0.52)],
        [(0.08, 0.50), (0.24, 0.50), (0.39, 0.50), (0.52, 0.66), (0.69, 0.66), (0.86, 0.52)],
        [(0.08, 0.82), (0.25, 0.82), (0.41, 0.66), (0.52, 0.66)],
    ],
    "Map_VeryHard_03": [
        [(0.11, 0.20), (0.86, 0.20), (0.86, 0.37), (0.28, 0.37), (0.28, 0.55), (0.74, 0.55), (0.74, 0.74), (0.18, 0.74)],
        [(0.11, 0.46), (0.28, 0.46)],
        [(0.11, 0.82), (0.33, 0.82), (0.47, 0.64)],
    ],
    "Map_VeryHard_04": [
        [(0.08, 0.18), (0.31, 0.18), (0.42, 0.31), (0.42, 0.52), (0.60, 0.64), (0.84, 0.64)],
        [(0.08, 0.50), (0.23, 0.50), (0.35, 0.66), (0.53, 0.66), (0.66, 0.82)],
        [(0.12, 0.84), (0.28, 0.84), (0.42, 0.69)],
    ],
    "Map_Impossible_01": [
        [(0.08, 0.22), (0.24, 0.22), (0.39, 0.38), (0.55, 0.38), (0.68, 0.56), (0.84, 0.56)],
        [(0.08, 0.50), (0.24, 0.50), (0.39, 0.38)],
        [(0.08, 0.78), (0.24, 0.78), (0.39, 0.62), (0.55, 0.62), (0.68, 0.56)],
    ],
    "Map_Impossible_02": [
        [(0.09, 0.19), (0.33, 0.19), (0.46, 0.34), (0.62, 0.34), (0.75, 0.18), (0.88, 0.18)],
        [(0.09, 0.51), (0.26, 0.51), (0.40, 0.67), (0.58, 0.67), (0.71, 0.50), (0.88, 0.50)],
        [(0.09, 0.83), (0.29, 0.83), (0.46, 0.66)],
    ],
    "Map_Impossible_03": [
        [(0.08, 0.16), (0.25, 0.16), (0.40, 0.32), (0.56, 0.32), (0.72, 0.50), (0.86, 0.50)],
        [(0.08, 0.38), (0.23, 0.38), (0.38, 0.50), (0.56, 0.50), (0.72, 0.50)],
        [(0.08, 0.61), (0.25, 0.61), (0.38, 0.50)],
        [(0.08, 0.84), (0.24, 0.84), (0.39, 0.68), (0.56, 0.68), (0.72, 0.50)],
    ],
    "Map_Impossible_04": [
        [(0.08, 0.18), (0.80, 0.18), (0.80, 0.32), (0.25, 0.32), (0.25, 0.46), (0.71, 0.46), (0.71, 0.60), (0.18, 0.60), (0.18, 0.76), (0.84, 0.76)],
        [(0.08, 0.36), (0.25, 0.36)],
        [(0.08, 0.60), (0.18, 0.60)],
        [(0.08, 0.84), (0.29, 0.84), (0.42, 0.68)],
    ],
}


def lerp_color(a: Color, b: Color, t: float) -> Color:
    return tuple(int(a[index] + (b[index] - a[index]) * t) for index in range(3))


def alpha(color: Color, value: int) -> tuple[int, int, int, int]:
    return color[0], color[1], color[2], value


def stable_rng(*parts: str) -> random.Random:
    key = "|".join(parts).encode("utf-8")
    seed = int(hashlib.sha256(key).hexdigest()[:16], 16)
    return random.Random(seed)


def read_map_rows() -> list[dict[str, str]]:
    with MAPS_CSV.open("r", encoding="utf-8", newline="") as handle:
        reader = csv.DictReader(handle)
        rows: list[dict[str, str]] = []
        for row in reader:
            if not row.get("MapID") or not row.get("BackgroundRelativePath"):
                continue
            rows.append(row)
        return rows


def to_px(point: Point) -> tuple[int, int]:
    return int(point[0] * SIZE[0]), int(point[1] * SIZE[1])


def segment_lengths(points: list[Point]) -> tuple[list[float], float]:
    lengths: list[float] = []
    total = 0.0
    for index in range(len(points) - 1):
        ax, ay = points[index]
        bx, by = points[index + 1]
        length = math.dist((ax, ay), (bx, by))
        lengths.append(length)
        total += length
    return lengths, total


def sample_polyline(points: list[Point], distance_ratio: float) -> tuple[Point, Point]:
    lengths, total_length = segment_lengths(points)
    if total_length <= 0.0:
        return points[0], (1.0, 0.0)

    target = max(0.0, min(total_length, distance_ratio * total_length))
    walked = 0.0
    for index, length in enumerate(lengths):
        if walked + length >= target or index == len(lengths) - 1:
            ax, ay = points[index]
            bx, by = points[index + 1]
            local_t = 0.0 if length <= 0.0 else (target - walked) / length
            point = (ax + (bx - ax) * local_t, ay + (by - ay) * local_t)
            direction = (bx - ax, by - ay)
            direction_len = math.hypot(direction[0], direction[1]) or 1.0
            normal = (-direction[1] / direction_len, direction[0] / direction_len)
            return point, normal
        walked += length

    return points[-1], (1.0, 0.0)


def gradient_image(top: Color, bottom: Color) -> Image.Image:
    image = Image.new("RGBA", SIZE, alpha(bottom, 255))
    pixels = image.load()
    for y in range(SIZE[1]):
        t = y / max(1, SIZE[1] - 1)
        row_color = lerp_color(top, bottom, t)
        for x in range(SIZE[0]):
            pixels[x, y] = alpha(row_color, 255)
    return image


def draw_soft_haze(image: Image.Image, palette: ThemePalette, rng: random.Random) -> None:
    overlay = Image.new("RGBA", SIZE, (0, 0, 0, 0))
    drawer = ImageDraw.Draw(overlay)
    for _ in range(16):
        radius = rng.randint(160, 420)
        x = rng.randint(-120, SIZE[0] - 40)
        y = rng.randint(-80, SIZE[1] - 40)
        drawer.ellipse((x - radius, y - radius, x + radius, y + radius), fill=alpha(palette.haze, rng.randint(16, 40)))
    image.alpha_composite(overlay.filter(ImageFilter.GaussianBlur(34)))


def draw_tile_texture(image: Image.Image, palette: ThemePalette, rng: random.Random) -> None:
    overlay = Image.new("RGBA", SIZE, (0, 0, 0, 0))
    drawer = ImageDraw.Draw(overlay)
    grid = 88
    for x in range(-grid, SIZE[0] + grid, grid):
        drawer.line((x, 0, x, SIZE[1]), fill=alpha(palette.path_outer, 26), width=2)
    for y in range(-grid, SIZE[1] + grid, grid):
        drawer.line((0, y, SIZE[0], y), fill=alpha(palette.path_outer, 26), width=2)
    for _ in range(180):
        px = rng.randint(0, SIZE[0] - 1)
        py = rng.randint(0, SIZE[1] - 1)
        length = rng.randint(22, 78)
        angle = rng.random() * math.tau
        qx = px + int(math.cos(angle) * length)
        qy = py + int(math.sin(angle) * length)
        drawer.line((px, py, qx, qy), fill=alpha(palette.path_outer, rng.randint(14, 32)), width=2)
    image.alpha_composite(overlay)


def draw_forest_texture(image: Image.Image, palette: ThemePalette, rng: random.Random) -> None:
    overlay = Image.new("RGBA", SIZE, (0, 0, 0, 0))
    drawer = ImageDraw.Draw(overlay)
    for _ in range(70):
        start_x = rng.randint(-160, SIZE[0] + 160)
        start_y = rng.randint(-40, SIZE[1] + 40)
        points = [(start_x, start_y)]
        for _ in range(4):
            points.append((points[-1][0] + rng.randint(-140, 140), points[-1][1] + rng.randint(-120, 120)))
        drawer.line(points, fill=alpha(palette.path_outer, rng.randint(28, 56)), width=rng.randint(4, 9), joint="curve")
    for _ in range(500):
        x = rng.randint(0, SIZE[0] - 1)
        y = rng.randint(0, SIZE[1] - 1)
        radius = rng.randint(2, 6)
        drawer.ellipse((x - radius, y - radius, x + radius, y + radius), fill=alpha(palette.haze, rng.randint(8, 28)))
    image.alpha_composite(overlay.filter(ImageFilter.GaussianBlur(2)))


def draw_ocean_texture(image: Image.Image, palette: ThemePalette, rng: random.Random) -> None:
    overlay = Image.new("RGBA", SIZE, (0, 0, 0, 0))
    drawer = ImageDraw.Draw(overlay)
    for band in range(18):
        y = int(SIZE[1] * (band / 18.0))
        points = []
        for step in range(0, SIZE[0] + 160, 160):
            points.append((step - 40, y + rng.randint(-26, 26)))
        drawer.line(points, fill=alpha(palette.haze, 28), width=3, joint="curve")
    for _ in range(260):
        x = rng.randint(0, SIZE[0] - 1)
        y = rng.randint(0, SIZE[1] - 1)
        radius = rng.randint(10, 28)
        drawer.arc((x - radius, y - radius, x + radius, y + radius), start=rng.randint(0, 180), end=rng.randint(181, 360), fill=alpha(palette.accent, 44), width=2)
    image.alpha_composite(overlay.filter(ImageFilter.GaussianBlur(1)))


def draw_martian_texture(image: Image.Image, palette: ThemePalette, rng: random.Random) -> None:
    overlay = Image.new("RGBA", SIZE, (0, 0, 0, 0))
    drawer = ImageDraw.Draw(overlay)
    grid = 112
    for x in range(0, SIZE[0] + grid, grid):
        drawer.line((x, 0, x, SIZE[1]), fill=alpha(palette.accent, 22), width=2)
    for y in range(0, SIZE[1] + grid, grid):
        drawer.line((0, y, SIZE[0], y), fill=alpha(palette.accent, 22), width=2)
    for _ in range(42):
        x = rng.randint(0, SIZE[0] - 1)
        y = rng.randint(0, SIZE[1] - 1)
        radius = rng.randint(28, 96)
        drawer.ellipse((x - radius, y - radius, x + radius, y + radius), outline=alpha(palette.path_outer, 60), width=3)
    image.alpha_composite(overlay.filter(ImageFilter.GaussianBlur(1)))


def draw_hell_texture(image: Image.Image, palette: ThemePalette, rng: random.Random) -> None:
    overlay = Image.new("RGBA", SIZE, (0, 0, 0, 0))
    drawer = ImageDraw.Draw(overlay)
    for _ in range(65):
        start_x = rng.randint(-100, SIZE[0] + 100)
        start_y = rng.randint(-100, SIZE[1] + 100)
        points = [(start_x, start_y)]
        for _ in range(5):
            points.append((points[-1][0] + rng.randint(-110, 110), points[-1][1] + rng.randint(-90, 90)))
        drawer.line(points, fill=alpha(palette.haze, rng.randint(36, 88)), width=rng.randint(3, 7), joint="curve")
    for _ in range(520):
        x = rng.randint(0, SIZE[0] - 1)
        y = rng.randint(0, SIZE[1] - 1)
        radius = rng.randint(1, 4)
        drawer.ellipse((x - radius, y - radius, x + radius, y + radius), fill=alpha(palette.accent, rng.randint(18, 96)))
    image.alpha_composite(overlay.filter(ImageFilter.GaussianBlur(2)))


def draw_theme_texture(image: Image.Image, theme_label: str, palette: ThemePalette, rng: random.Random) -> None:
    if theme_label == "Dungeon":
        draw_tile_texture(image, palette, rng)
    elif theme_label == "Forest":
        draw_forest_texture(image, palette, rng)
    elif theme_label == "Ocean":
        draw_ocean_texture(image, palette, rng)
    elif theme_label == "Martian":
        draw_martian_texture(image, palette, rng)
    else:
        draw_hell_texture(image, palette, rng)


def pad_positions(paths: list[list[Point]], count: int, rng: random.Random) -> list[tuple[int, int, int]]:
    if not paths:
        return []

    positions: list[tuple[int, int, int]] = []
    path_index = 0
    for index in range(count):
        path = paths[path_index % len(paths)]
        path_index += 1
        ratio = (index + 1) / (count + 1)
        sample_ratio = max(0.05, min(0.95, ratio + rng.uniform(-0.03, 0.03)))
        point, normal = sample_polyline(path, sample_ratio)
        distance = 86 + (index % 3) * 20 + rng.randint(-10, 16)
        side = -1 if index % 2 == 0 else 1
        px, py = to_px(point)
        pad_x = int(px + normal[0] * distance * side)
        pad_y = int(py + normal[1] * distance * side)
        pad_x = max(70, min(SIZE[0] - 70, pad_x))
        pad_y = max(70, min(SIZE[1] - 70, pad_y))
        positions.append((pad_x, pad_y, 18 + (index % 2) * 5))
    return positions


def draw_paths(image: Image.Image, paths: list[list[Point]], palette: ThemePalette, rng: random.Random) -> None:
    glow = Image.new("RGBA", SIZE, (0, 0, 0, 0))
    glow_draw = ImageDraw.Draw(glow)
    path = Image.new("RGBA", SIZE, (0, 0, 0, 0))
    path_draw = ImageDraw.Draw(path)
    for path_points in paths:
        path_px = [to_px(point) for point in path_points]
        lane_width = rng.randint(56, 72)
        glow_draw.line(path_px, fill=alpha(palette.haze, 118), width=lane_width + 34, joint="curve")
        path_draw.line(path_px, fill=alpha(palette.path_outer, 255), width=lane_width + 18, joint="curve")
        path_draw.line(path_px, fill=alpha(palette.path_fill, 255), width=lane_width, joint="curve")
        path_draw.line(path_px, fill=alpha(palette.path_inner, 255), width=max(10, lane_width - 28), joint="curve")
        for node in path_px:
            path_draw.ellipse((node[0] - 12, node[1] - 12, node[0] + 12, node[1] + 12), fill=alpha(palette.accent, 180))
    image.alpha_composite(glow.filter(ImageFilter.GaussianBlur(18)))
    image.alpha_composite(path)


def draw_pads(image: Image.Image, positions: Iterable[tuple[int, int, int]], palette: ThemePalette) -> None:
    glow = Image.new("RGBA", SIZE, (0, 0, 0, 0))
    glow_draw = ImageDraw.Draw(glow)
    base = Image.new("RGBA", SIZE, (0, 0, 0, 0))
    base_draw = ImageDraw.Draw(base)
    for x, y, radius in positions:
        glow_draw.ellipse((x - radius - 10, y - radius - 10, x + radius + 10, y + radius + 10), fill=alpha(palette.pad_outer, 120))
        base_draw.ellipse((x - radius - 6, y - radius - 6, x + radius + 6, y + radius + 6), fill=alpha((21, 18, 21), 220))
        base_draw.ellipse((x - radius, y - radius, x + radius, y + radius), fill=alpha(palette.pad_outer, 255))
        base_draw.ellipse((x - radius + 7, y - radius + 7, x + radius - 7, y + radius - 7), fill=alpha(palette.pad_inner, 255))
        base_draw.rectangle((x - 7, y - 2, x + 7, y + 2), fill=alpha((30, 23, 18), 235))
        base_draw.rectangle((x - 2, y - 7, x + 2, y + 7), fill=alpha((30, 23, 18), 235))
    image.alpha_composite(glow.filter(ImageFilter.GaussianBlur(12)))
    image.alpha_composite(base)


def draw_vignette(image: Image.Image) -> None:
    vignette = Image.new("RGBA", SIZE, (0, 0, 0, 0))
    drawer = ImageDraw.Draw(vignette)
    for index in range(12):
        inset = index * 22
        alpha_value = min(255, 8 + index * 7)
        drawer.rectangle((inset, inset, SIZE[0] - inset, SIZE[1] - inset), outline=(0, 0, 0, alpha_value), width=28)
    image.alpha_composite(vignette.filter(ImageFilter.GaussianBlur(32)))


def build_map_prompt(row: dict[str, str]) -> str:
    theme = row["ThemeLabel"]
    difficulty = row["DifficultyID"]
    return (
        f"16:9 tower defense map background for Chadpocalypse TD, {row['DisplayName']} in the {theme} theme, "
        f"{row['Description']} High-contrast tactical top-down backdrop, readable route silhouettes, build pad landmarks, "
        f"dark-fantasy atmosphere, premium game UI background, no text, no HUD. Difficulty tier: {difficulty}."
    )


def render_map(row: dict[str, str]) -> None:
    map_id = row["MapID"]
    theme_label = row["ThemeLabel"]
    palette = THEMES[theme_label]
    rng = stable_rng("map", map_id)
    paths = MAP_LAYOUTS[map_id]

    image = gradient_image(palette.background_top, palette.background_bottom)
    draw_soft_haze(image, palette, rng)
    draw_theme_texture(image, theme_label, palette, rng)
    draw_paths(image, paths, palette, rng)
    draw_pads(image, pad_positions(paths, int(row["BuildPadCount"]), rng), palette)
    draw_vignette(image)
    image.save(OUTPUT_DIR / Path(row["BackgroundRelativePath"]).name)


def render_menu_backdrop() -> None:
    palette = THEMES["Dungeon"]
    rng = stable_rng("menu", "backdrop")
    image = gradient_image((13, 7, 10), (72, 18, 20))
    draw_soft_haze(image, palette, rng)
    draw_tile_texture(image, palette, rng)

    glow = Image.new("RGBA", SIZE, (0, 0, 0, 0))
    glow_draw = ImageDraw.Draw(glow)
    arch = Image.new("RGBA", SIZE, (0, 0, 0, 0))
    arch_draw = ImageDraw.Draw(arch)

    main_paths = [
        [to_px(point) for point in [(0.14, 0.74), (0.32, 0.58), (0.48, 0.58), (0.63, 0.42), (0.85, 0.42)]],
        [to_px(point) for point in [(0.14, 0.30), (0.33, 0.30), (0.48, 0.44), (0.63, 0.44), (0.85, 0.70)]],
    ]
    for path_points in main_paths:
        glow_draw.line(path_points, fill=alpha((255, 116, 44), 122), width=84, joint="curve")
        arch_draw.line(path_points, fill=alpha((31, 16, 17), 255), width=56, joint="curve")
        arch_draw.line(path_points, fill=alpha((184, 90, 60), 255), width=44, joint="curve")
        arch_draw.line(path_points, fill=alpha((236, 186, 124), 255), width=18, joint="curve")

    for x in (0.18, 0.82):
        center_x = int(SIZE[0] * x)
        arch_draw.rectangle((center_x - 78, 180, center_x + 78, 980), fill=alpha((24, 13, 16), 190))
        arch_draw.rectangle((center_x - 48, 0, center_x + 48, 1080), fill=alpha((10, 7, 11), 180))
    for _ in range(160):
        px = rng.randint(0, SIZE[0] - 1)
        py = rng.randint(0, SIZE[1] - 1)
        radius = rng.randint(2, 6)
        glow_draw.ellipse((px - radius, py - radius, px + radius, py + radius), fill=alpha((255, 198, 114), rng.randint(26, 84)))

    image.alpha_composite(glow.filter(ImageFilter.GaussianBlur(22)))
    image.alpha_composite(arch)
    draw_vignette(image)
    image.save(OUTPUT_DIR / "TD_Menu_Backdrop_01.png")


def render_menu_foreground() -> None:
    overlay = Image.new("RGBA", SIZE, (0, 0, 0, 0))
    drawer = ImageDraw.Draw(overlay)
    edge = (255, 172, 97, 148)
    dark = (16, 10, 12, 220)

    drawer.rounded_rectangle((24, 24, SIZE[0] - 24, SIZE[1] - 24), radius=28, outline=edge, width=8)
    drawer.rounded_rectangle((46, 46, SIZE[0] - 46, SIZE[1] - 46), radius=24, outline=(255, 204, 142, 64), width=3)
    for x in (96, SIZE[0] - 96):
        drawer.rectangle((x - 10, 0, x + 10, 190), fill=(24, 16, 20, 166))
        drawer.rectangle((x - 10, SIZE[1] - 190, x + 10, SIZE[1]), fill=(24, 16, 20, 166))
        for chain_y in range(60, SIZE[1] - 120, 48):
            drawer.ellipse((x - 20, chain_y - 12, x + 20, chain_y + 12), outline=(255, 190, 124, 112), width=3)

    for left in (120, SIZE[0] - 320):
        drawer.polygon([(left, SIZE[1]), (left + 80, SIZE[1] - 260), (left + 190, SIZE[1]), (left + 110, SIZE[1])], fill=dark)
        drawer.rectangle((left + 70, SIZE[1] - 250, left + 120, SIZE[1]), fill=(9, 8, 10, 232))

    vignette = Image.new("RGBA", SIZE, (0, 0, 0, 0))
    vignette_draw = ImageDraw.Draw(vignette)
    vignette_draw.rectangle((0, 0, SIZE[0], SIZE[1]), fill=(0, 0, 0, 0))
    for index in range(14):
        inset = index * 28
        vignette_draw.rectangle((inset, inset, SIZE[0] - inset, SIZE[1] - inset), outline=(0, 0, 0, min(255, 10 + index * 10)), width=36)
    overlay.alpha_composite(vignette.filter(ImageFilter.GaussianBlur(30)))
    overlay.save(OUTPUT_DIR / "TD_Menu_Foreground_01.png")


def write_prompt_manifest(rows: list[dict[str, str]]) -> None:
    lines = [
        "# Chadpocalypse TD Image Prompts",
        "",
        "These prompts match the TD-owned filenames in `SourceAssets/TD/Maps/Backgrounds/`.",
        "They are ready to reuse in a future image-generation replacement pass.",
        "",
        "## Menu",
        "",
        "- `TD_Menu_Backdrop_01.png`: Wide 16:9 Chadpocalypse TD main menu background, infernal dungeon siege vista, top-down catacomb lanes with molten braziers and cracked stone, readable negative space for UI, premium dark-fantasy game menu art, no text.",
        "- `TD_Menu_Foreground_01.png`: Transparent foreground overlay for Chadpocalypse TD menu, dark gothic frame, chains, ember-lit trim, subtle vignette, no text.",
        "",
        "## Maps",
        "",
    ]
    for row in rows:
        lines.append(f"- `{Path(row['BackgroundRelativePath']).name}`: {build_map_prompt(row)}")
    (OUTPUT_DIR / "TD_ImageGen_Prompts.md").write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> None:
    rows = read_map_rows()
    render_menu_backdrop()
    render_menu_foreground()
    for row in rows:
        render_map(row)
    write_prompt_manifest(rows)


if __name__ == "__main__":
    main()
