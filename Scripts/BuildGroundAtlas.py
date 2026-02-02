"""
Build a 2x2 ground atlas PNG from tile1.png, tile2.png, tile3.png, tile4.png.

Input (in SourceAssets/Ground/):
  tile1.png  (top-left)
  tile2.png  (top-right)
  tile3.png  (bottom-left)
  tile4.png  (bottom-right)

Output:
  SourceAssets/Ground/GroundAtlas_2x2_1024.png  (1024x1024)

Run (PowerShell, from project root):
  python Scripts/BuildGroundAtlas.py

Then run ImportGroundAtlas.py to import into UE and create the material.
"""

from __future__ import annotations

import os
import sys

try:
    from PIL import Image
except ImportError:
    print("BuildGroundAtlas requires Pillow. Install with: pip install Pillow")
    sys.exit(1)

ATLAS_SIZE = 1024
TILE_NAMES = ["tile1.png", "tile2.png", "tile3.png", "tile4.png"]
ATLAS_OUTPUT = "GroundAtlas_2x2_1024.png"


def main() -> int:
    script_dir = os.path.dirname(os.path.abspath(__file__))
    proj_root = os.path.dirname(script_dir)
    ground_dir = os.path.join(proj_root, "SourceAssets", "Ground")
    out_path = os.path.join(ground_dir, ATLAS_OUTPUT)

    half = ATLAS_SIZE // 2

    # Load and optionally resize each tile to 512x512
    tiles = []
    for name in TILE_NAMES:
        path = os.path.join(ground_dir, name)
        if not os.path.isfile(path):
            print(f"ERROR: Missing {path}")
            return 1
        img = Image.open(path).convert("RGB")
        w, h = img.size
        if w != half or h != half:
            img = img.resize((half, half), Image.Resampling.LANCZOS)
        tiles.append(img)

    # Create 2x2 atlas: [tile1, tile2]
    #                [tile3, tile4]
    atlas = Image.new("RGB", (ATLAS_SIZE, ATLAS_SIZE))
    atlas.paste(tiles[0], (0, 0))           # top-left
    atlas.paste(tiles[1], (half, 0))        # top-right
    atlas.paste(tiles[2], (0, half))        # bottom-left
    atlas.paste(tiles[3], (half, half))     # bottom-right

    atlas.save(out_path, "PNG")
    print(f"Built: {out_path}")
    print("Next: Run ImportGroundAtlas.py in UE to import and create the material.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
