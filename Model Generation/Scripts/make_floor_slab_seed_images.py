import json
import os
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw, ImageFilter


RUN_ROOT = Path(r"C:\UE\T66\Model Generation\Runs\Environment\CoherentThemeKit01")
SOURCE_DIR = RUN_ROOT / "Inputs" / "approved_seed_images"
OUTPUT_DIR = RUN_ROOT / "Inputs" / "floor_slab_seed_images"
MANIFEST_PATH = OUTPUT_DIR / "floor_slab_seed_manifest.json"
MONTAGE_PATH = OUTPUT_DIR / "floor_slab_seed_montage.png"

KEY = (0, 255, 0, 255)


FLOOR_MODULES = [
    "DungeonFloor_StoneSlabs_A",
    "DungeonFloor_Drain_A",
    "DungeonFloor_Cracked_A",
    "DungeonFloor_Bones_A",
    "ForestFloor_RootMat_A",
    "ForestFloor_MossStone_A",
    "ForestFloor_LeafCrack_A",
    "ForestFloor_BrambleEdge_A",
    "OceanFloor_ReefStone_A",
    "OceanFloor_ShellSand_A",
    "OceanFloor_CoralCrack_A",
    "OceanFloor_TidePool_A",
    "MartianFloor_RuinTile_A",
    "MartianFloor_RegolithPlates_A",
    "MartianFloor_CrystalDust_A",
    "MartianFloor_CraterCracks_A",
    "HellFloor_RunePlate_A",
    "HellFloor_Obsidian_A",
    "HellFloor_EmberFissure_A",
    "HellFloor_BoneAsh_A",
]


def foreground_bbox(image):
    rgba = np.asarray(image.convert("RGBA"))
    r = rgba[:, :, 0].astype(np.int16)
    g = rgba[:, :, 1].astype(np.int16)
    b = rgba[:, :, 2].astype(np.int16)
    a = rgba[:, :, 3]
    green_bg = (g > 165) & (r < 90) & (b < 90) & (a > 0)
    mask = (~green_bg) & (a > 0)
    ys, xs = np.where(mask)
    if len(xs) == 0:
        raise RuntimeError("No foreground pixels detected")
    pad = 8
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

    # Pillow's perspective transform maps output pixels back to source pixels,
    # so solve coefficients from destination quad to source rectangle.
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


def add_shadow(canvas, top_quad, drop):
    mask = Image.new("L", canvas.size, 0)
    draw = ImageDraw.Draw(mask)
    shadow_quad = [(x + 18, y + 30) for x, y in top_quad]
    shadow_quad[2] = (shadow_quad[2][0], shadow_quad[2][1] + drop)
    shadow_quad[3] = (shadow_quad[3][0], shadow_quad[3][1] + drop)
    draw.polygon(shadow_quad, fill=135)
    mask = mask.filter(ImageFilter.GaussianBlur(22))
    shadow = Image.new("RGBA", canvas.size, (0, 0, 0, 85))
    canvas.alpha_composite(Image.composite(shadow, Image.new("RGBA", canvas.size), mask))


def make_slab(module_id):
    source_path = SOURCE_DIR / f"{module_id}.png"
    source = Image.open(source_path)
    tile = cutout_tile(source)

    canvas_size = (1536, 1024)
    canvas = Image.new("RGBA", canvas_size, KEY)

    top_quad = [(286, 236), (1250, 236), (1116, 620), (420, 620)]
    drop = 178
    front_quad = [top_quad[3], top_quad[2], (top_quad[2][0], top_quad[2][1] + drop), (top_quad[3][0], top_quad[3][1] + drop)]
    left_quad = [top_quad[0], top_quad[3], (top_quad[3][0], top_quad[3][1] + drop), (top_quad[0][0], top_quad[0][1] + drop)]
    right_quad = [top_quad[1], top_quad[2], (top_quad[2][0], top_quad[2][1] + drop), (top_quad[1][0], top_quad[1][1] + drop)]

    add_shadow(canvas, top_quad, drop)

    side_source = tile.crop((0, int(tile.height * 0.62), tile.width, tile.height))
    left_source = tile.crop((0, 0, int(tile.width * 0.22), tile.height))
    right_source = tile.crop((int(tile.width * 0.78), 0, tile.width, tile.height))

    for face, face_source, shade in [
        (left_quad, left_source.resize((240, 900), Image.Resampling.LANCZOS), 0.55),
        (right_quad, right_source.resize((240, 900), Image.Resampling.LANCZOS), 0.48),
        (front_quad, side_source.resize((900, 260), Image.Resampling.LANCZOS), 0.62),
        (top_quad, tile, 1.03),
    ]:
        warped = warp_to_quad(darken(face_source, shade), canvas_size, face)
        canvas.alpha_composite(warped)

    outline = Image.new("RGBA", canvas_size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(outline)
    for poly in [left_quad, right_quad, front_quad, top_quad]:
        draw.line(poly + [poly[0]], fill=(18, 24, 20, 150), width=5)
    outline = outline.filter(ImageFilter.GaussianBlur(0.35))
    canvas.alpha_composite(outline)

    output_path = OUTPUT_DIR / f"{module_id}_FloorFix01.png"
    canvas.convert("RGB").save(output_path, optimize=True)
    return {
        "module_id": module_id,
        "source": str(source_path),
        "output": str(output_path),
        "fix": "perspective low-angle thick floor slab with visible side faces",
    }


def write_montage(outputs):
    thumbs = []
    for entry in outputs:
        img = Image.open(entry["output"]).resize((384, 256), Image.Resampling.LANCZOS)
        thumbs.append((entry["module_id"], img))

    cols = 4
    rows = (len(thumbs) + cols - 1) // cols
    tile_w, tile_h = 384, 292
    montage = Image.new("RGB", (cols * tile_w, rows * tile_h), (32, 34, 36))
    draw = ImageDraw.Draw(montage)
    for index, (name, img) in enumerate(thumbs):
        x = (index % cols) * tile_w
        y = (index // cols) * tile_h
        montage.paste(img, (x, y))
        draw.text((x + 8, y + 262), name, fill=(235, 235, 235))
    montage.save(MONTAGE_PATH, optimize=True)


def main():
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    outputs = [make_slab(module_id) for module_id in FLOOR_MODULES]
    MANIFEST_PATH.write_text(json.dumps(outputs, indent=2), encoding="utf-8")
    write_montage(outputs)
    print(json.dumps({"count": len(outputs), "output_dir": str(OUTPUT_DIR)}, indent=2))


if __name__ == "__main__":
    main()
