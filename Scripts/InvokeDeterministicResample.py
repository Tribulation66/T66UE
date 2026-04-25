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
    parser.add_argument(
        "--target-width",
        type=int,
        default=0,
        help="Normalize to an exact output width instead of integer scaling.",
    )
    parser.add_argument(
        "--target-height",
        type=int,
        default=0,
        help="Normalize to an exact output height instead of integer scaling.",
    )
    parser.add_argument(
        "--max-ratio-delta",
        type=float,
        default=0.35,
        help="Reject target normalization when the source aspect is too far from the target.",
    )
    parser.add_argument(
        "--allow-extreme-crop",
        action="store_true",
        help="Allow target normalization even when the source aspect ratio is far from the target.",
    )
    return parser.parse_args()


def normalize_to_target(
    image: Image.Image,
    target_width: int,
    target_height: int,
    max_ratio_delta: float,
    allow_extreme_crop: bool,
) -> Image.Image:
    source_width, source_height = image.size
    source_ratio = source_width / source_height
    target_ratio = target_width / target_height

    if not allow_extreme_crop and abs(source_ratio - target_ratio) > max_ratio_delta:
        raise ValueError(
            "Input aspect ratio is too far from the target canvas: "
            f"{source_width}x{source_height} -> {target_width}x{target_height}. "
            "Regenerate a landscape source or pass --allow-extreme-crop explicitly."
        )

    if source_ratio > target_ratio:
        new_width = int(source_height * target_ratio)
        left = (source_width - new_width) // 2
        image = image.crop((left, 0, left + new_width, source_height))
    elif source_ratio < target_ratio:
        new_height = int(source_width / target_ratio)
        top = (source_height - new_height) // 2
        image = image.crop((0, top, source_width, top + new_height))

    return image.resize((target_width, target_height), Image.Resampling.LANCZOS)


def main() -> int:
    args = parse_args()
    input_path = args.input_image.resolve()
    output_path = args.output_image.resolve()

    if not input_path.exists():
        raise FileNotFoundError(f"Input image does not exist: {input_path}")

    output_path.parent.mkdir(parents=True, exist_ok=True)

    with Image.open(input_path) as image:
        image = image.convert("RGB")
        width, height = image.size
        if args.target_width or args.target_height:
            if not args.target_width or not args.target_height:
                raise ValueError("--target-width and --target-height must be provided together.")
            resized = normalize_to_target(
                image=image,
                target_width=args.target_width,
                target_height=args.target_height,
                max_ratio_delta=args.max_ratio_delta,
                allow_extreme_crop=args.allow_extreme_crop,
            )
            print(f"{input_path} ({width}x{height}) -> {output_path} ({args.target_width}x{args.target_height})")
        else:
            resized = image.resize((width * args.scale, height * args.scale), Image.Resampling.LANCZOS)
            print(f"{input_path} ({width}x{height}) -> {output_path} ({width * args.scale}x{height * args.scale})")
        resized.save(output_path)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
