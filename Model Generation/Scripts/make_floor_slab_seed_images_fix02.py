import json
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw, ImageFilter


RUN_ROOT = Path(r"C:\UE\T66\Model Generation\Runs\Environment\CoherentThemeKit01")
SOURCE_DIR = RUN_ROOT / "Inputs" / "approved_seed_images"
OUTPUT_DIR = RUN_ROOT / "Inputs" / "floor_slab_seed_images_fix02"
MANIFEST_PATH = OUTPUT_DIR / "floor_slab_seed_manifest_fix02.json"
MONTAGE_PATH = OUTPUT_DIR / "floor_slab_seed_montage_fix02.png"
FIX_TAG = "FloorFix02"

KEY = (0, 255, 0, 255)

FLOOR_MODULES = [
    "DungeonFloor_StoneSlabs_A",
    "DungeonFloor_Drain_A",
    "OceanFloor_ShellSand_A",
    "MartianFloor_CraterCracks_A",
    "HellFloor_RunePlate_A",
]


def foreground_bbox(image):
    rgba = np.asarray(image.convert("RGBA"))
    r = rgba[:, :, 0].astype(np.int16)
    g = rgba[:, :, 1].astype(np.int16)
    b = rgba[:, :, 2].astype(np.int16)
    a = rgba[:, :, 3]
    green_bg = (g > 165) & (r < 95) & (b < 95) & (a > 0)
    mask = (~green_bg) & (a > 0)
    ys, xs = np.where(mask)
    if len(xs) == 0:
        raise RuntimeError("No foreground pixels detected")
    pad = 10
    return (
        max(int(xs.min()) - pad, 0),
        max(int(ys.min()) - pad, 0),
        min(int(xs.max()) + pad + 1, image.width),
        min(int(ys.max()) + pad + 1, image.height),
    )


def cutout_tile(image):
    crop = image.convert("RGBA").crop(foreground_bbox(image))
    rgba = np.asarray(crop).copy()
    r = rgba[:, :, 0].astype(np.int16)
    g = rgba[:, :, 1].astype(np.int16)
    b = rgba[:, :, 2].astype(np.int16)
    green_bg = (g > 165) & (r < 95) & (b < 95)
    rgba[green_bg, 3] = 0
    crop = Image.fromarray(rgba, "RGBA")
    side = max(crop.width, crop.height)
    square = Image.new("RGBA", (side, side), (0, 0, 0, 0))
    square.alpha_composite(crop, ((side - crop.width) // 2, (side - crop.height) // 2))
    return square.resize((900, 900), Image.Resampling.LANCZOS)


def average_visible_color(image):
    rgba = np.asarray(image.convert("RGBA"))
    visible = rgba[:, :, 3] > 16
    if not np.any(visible):
        return (90, 86, 76)
    rgb = rgba[:, :, :3][visible]
    color = np.median(rgb, axis=0)
    return tuple(int(max(34, min(210, value))) for value in color)


def solid_tile(color, alpha=255):
    base = Image.new("RGBA", (900, 900), (*color, alpha))
    noise = np.random.default_rng(1337).normal(0, 5, (900, 900, 1))
    arr = np.asarray(base).copy().astype(np.int16)
    arr[:, :, :3] = np.clip(arr[:, :, :3] + noise, 0, 255)
    return Image.fromarray(arr.astype(np.uint8), "RGBA")


def darken(image, factor):
    arr = np.asarray(image.convert("RGBA")).copy()
    arr[:, :, :3] = np.clip(arr[:, :, :3].astype(np.float32) * factor, 0, 255).astype(
        np.uint8
    )
    return Image.fromarray(arr, "RGBA")


def warp_to_quad(source, size, quad):
    src = [
        (0, 0),
        (source.width - 1, 0),
        (source.width - 1, source.height - 1),
        (0, source.height - 1),
    ]
    matrix = []
    vector = []
    for (x, y), (u, v) in zip(quad, src):
        matrix.append([x, y, 1, 0, 0, 0, -u * x, -u * y])
        matrix.append([0, 0, 0, x, y, 1, -v * x, -v * y])
        vector.append(u)
        vector.append(v)
    coeffs = np.linalg.solve(np.asarray(matrix, dtype=float), np.asarray(vector, dtype=float))
    return source.transform(
        size,
        Image.Transform.PERSPECTIVE,
        coeffs,
        Image.Resampling.BICUBIC,
        fillcolor=(0, 0, 0, 0),
    )


def face_fill(canvas, poly, color):
    face = Image.new("RGBA", canvas.size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(face)
    draw.polygon(poly, fill=color)
    canvas.alpha_composite(face)


def add_shadow(canvas, top_quad, drop):
    mask = Image.new("L", canvas.size, 0)
    draw = ImageDraw.Draw(mask)
    shadow_quad = [(x + 16, y + 28) for x, y in top_quad]
    shadow_quad[2] = (shadow_quad[2][0], shadow_quad[2][1] + drop)
    shadow_quad[3] = (shadow_quad[3][0], shadow_quad[3][1] + drop)
    draw.polygon(shadow_quad, fill=128)
    mask = mask.filter(ImageFilter.GaussianBlur(20))
    shadow = Image.new("RGBA", canvas.size, (0, 0, 0, 85))
    canvas.alpha_composite(Image.composite(shadow, Image.new("RGBA", canvas.size), mask))


def make_slab(module_id):
    source_path = SOURCE_DIR / f"{module_id}.png"
    source = Image.open(source_path)
    detail_tile = cutout_tile(source)
    base_color = average_visible_color(detail_tile)
    base_tile = solid_tile(base_color)
    top_tile = base_tile.copy()
    top_tile.alpha_composite(detail_tile)

    canvas_size = (1536, 1024)
    canvas = Image.new("RGBA", canvas_size, KEY)

    top_quad = [(274, 242), (1262, 242), (1118, 606), (418, 606)]
    drop = 142
    front_quad = [
        top_quad[3],
        top_quad[2],
        (top_quad[2][0], top_quad[2][1] + drop),
        (top_quad[3][0], top_quad[3][1] + drop),
    ]
    left_quad = [
        top_quad[0],
        top_quad[3],
        (top_quad[3][0], top_quad[3][1] + drop),
        (top_quad[0][0], top_quad[0][1] + drop),
    ]
    right_quad = [
        top_quad[1],
        top_quad[2],
        (top_quad[2][0], top_quad[2][1] + drop),
        (top_quad[1][0], top_quad[1][1] + drop),
    ]

    add_shadow(canvas, top_quad, drop)

    for poly, shade in [(left_quad, 0.52), (right_quad, 0.46), (front_quad, 0.58), (top_quad, 0.92)]:
        fill = tuple(int(channel * shade) for channel in base_color) + (255,)
        face_fill(canvas, poly, fill)

    side_source = darken(base_tile.crop((0, 540, 900, 900)).resize((900, 260), Image.Resampling.LANCZOS), 0.64)
    left_source = darken(base_tile.crop((0, 0, 220, 900)).resize((260, 900), Image.Resampling.LANCZOS), 0.56)
    right_source = darken(base_tile.crop((680, 0, 900, 900)).resize((260, 900), Image.Resampling.LANCZOS), 0.48)

    for face, face_source in [
        (left_quad, left_source),
        (right_quad, right_source),
        (front_quad, side_source),
        (top_quad, top_tile),
    ]:
        canvas.alpha_composite(warp_to_quad(face_source, canvas_size, face))

    outline = Image.new("RGBA", canvas_size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(outline)
    for poly in [left_quad, right_quad, front_quad, top_quad]:
        draw.line(poly + [poly[0]], fill=(16, 18, 16, 180), width=7)
    canvas.alpha_composite(outline.filter(ImageFilter.GaussianBlur(0.25)))

    output_path = OUTPUT_DIR / f"{module_id}_{FIX_TAG}.png"
    canvas.convert("RGB").save(output_path, optimize=True)
    return {
        "module_id": module_id,
        "source": str(source_path),
        "output": str(output_path),
        "fix": "solid continuous slab underlay with original floor details on the top face",
    }


def write_montage(outputs):
    cols = len(outputs)
    tile_w, tile_h = 384, 292
    montage = Image.new("RGB", (cols * tile_w, tile_h), (32, 34, 36))
    draw = ImageDraw.Draw(montage)
    for index, entry in enumerate(outputs):
        img = Image.open(entry["output"]).resize((384, 256), Image.Resampling.LANCZOS)
        x = index * tile_w
        montage.paste(img, (x, 0))
        draw.text((x + 8, 262), entry["module_id"], fill=(235, 235, 235))
    montage.save(MONTAGE_PATH, optimize=True)


def main():
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    outputs = [make_slab(module_id) for module_id in FLOOR_MODULES]
    MANIFEST_PATH.write_text(json.dumps(outputs, indent=2), encoding="utf-8")
    write_montage(outputs)
    print(json.dumps({"count": len(outputs), "output_dir": str(OUTPUT_DIR)}, indent=2))


if __name__ == "__main__":
    main()
