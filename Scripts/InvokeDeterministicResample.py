from __future__ import annotations

import argparse
from pathlib import Path

from PIL import Image


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Create a deterministic high-quality resample of an image for UI reference work."
    )
    parser.add_argument("input_image", type=Path)
    parser.add_argument("output_image", type=Path)
    parser.add_argument("--scale", type=int, choices=(2, 3, 4), default=2)
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    input_path = args.input_image.resolve()
    output_path = args.output_image.resolve()

    if not input_path.exists():
        raise FileNotFoundError(f"Input image does not exist: {input_path}")

    output_path.parent.mkdir(parents=True, exist_ok=True)

    with Image.open(input_path) as image:
        width, height = image.size
        resized = image.resize((width * args.scale, height * args.scale), Image.Resampling.LANCZOS)
        resized.save(output_path)

    print(output_path)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
