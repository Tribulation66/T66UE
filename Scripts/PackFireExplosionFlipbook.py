"""
Pack rendered fire explosion frames into a flipbook sheet and preview images.

Run with system Python:
  python "C:/UE/T66/Scripts/PackFireExplosionFlipbook.py"
"""

import os
from glob import glob

from PIL import Image, ImageDraw


PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
OUT_DIR = os.path.join(PROJECT_ROOT, "SourceAssets", "Import", "VFX", "Idol_Fire_Explosion")
FRAMES_DIR = os.path.join(OUT_DIR, "frames")
FLIPBOOK_PATH = os.path.join(OUT_DIR, "FireExplosion_Flipbook_4x4.png")
CONTACT_SHEET_PATH = os.path.join(OUT_DIR, "FireExplosion_ContactSheet.png")
KEYFRAME_PREVIEW_PATH = os.path.join(OUT_DIR, "FireExplosion_Keyframes.png")
GIF_PATH = os.path.join(OUT_DIR, "FireExplosion_Preview.gif")

GRID_COLS = 4
GRID_ROWS = 4


def load_frames():
    paths = sorted(glob(os.path.join(FRAMES_DIR, "fire_explosion_*.png")))
    if not paths:
        raise RuntimeError(f"No rendered frames found in {FRAMES_DIR}")
    return [Image.open(path).convert("RGBA") for path in paths]


def build_flipbook(frames):
    frame_w, frame_h = frames[0].size
    sheet = Image.new("RGBA", (frame_w * GRID_COLS, frame_h * GRID_ROWS), (0, 0, 0, 0))
    for index, frame in enumerate(frames[: GRID_COLS * GRID_ROWS]):
        x = (index % GRID_COLS) * frame_w
        y = (index // GRID_COLS) * frame_h
        sheet.paste(frame, (x, y), frame)
    sheet.save(FLIPBOOK_PATH)


def build_contact_sheet(frames):
    frame_w, frame_h = frames[0].size
    margin = 24
    label_height = 26
    bg = Image.new(
        "RGBA",
        (frame_w * GRID_COLS + margin * 2, (frame_h + label_height) * GRID_ROWS + margin * 2),
        (8, 8, 8, 255),
    )
    draw = ImageDraw.Draw(bg)

    for index, frame in enumerate(frames[: GRID_COLS * GRID_ROWS]):
        x = margin + (index % GRID_COLS) * frame_w
        y = margin + (index // GRID_COLS) * (frame_h + label_height)
        bg.paste(frame, (x, y), frame)
        draw.text((x + 8, y + frame_h + 4), f"Frame {index + 1:02d}", fill=(255, 214, 158, 255))

    bg.save(CONTACT_SHEET_PATH)


def build_keyframe_preview(frames):
    picks = [0, 4, 8, 12]
    selected = [frames[index] for index in picks if index < len(frames)]
    if not selected:
        return

    frame_w, frame_h = selected[0].size
    bg = Image.new("RGBA", (frame_w * len(selected), frame_h), (0, 0, 0, 255))
    for index, frame in enumerate(selected):
        bg.paste(frame, (index * frame_w, 0), frame)
    bg.save(KEYFRAME_PREVIEW_PATH)


def build_gif(frames):
    palette_ready = [frame.convert("P", palette=Image.ADAPTIVE, colors=255) for frame in frames]
    palette_ready[0].save(
        GIF_PATH,
        save_all=True,
        append_images=palette_ready[1:],
        duration=65,
        loop=0,
        disposal=2,
        transparency=0,
    )


def main():
    frames = load_frames()
    build_flipbook(frames)
    build_contact_sheet(frames)
    build_keyframe_preview(frames)
    build_gif(frames)
    print(f"[FireExplosion] Wrote flipbook assets to {OUT_DIR}")


if __name__ == "__main__":
    main()
