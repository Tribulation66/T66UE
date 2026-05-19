#!/usr/bin/env python3
"""Remove the background from a source PNG and write an alpha PNG.

This is intentionally small and scriptable for ToonStyle image preflight:
white-background image gen -> rembg alpha isolation -> Pixal3D input.
"""

from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path

from PIL import Image


def ensure_rembg():
    try:
        from rembg import new_session, remove  # type: ignore

        return new_session, remove
    except ImportError:
        subprocess.check_call(
            [
                sys.executable,
                "-m",
                "pip",
                "install",
                "rembg",
                "onnxruntime",
            ]
        )
        from rembg import new_session, remove  # type: ignore

        return new_session, remove


def isolate(source: Path, output: Path, model: str) -> None:
    new_session, remove = ensure_rembg()
    image = Image.open(source).convert("RGBA")
    session = new_session(model)
    result = remove(image, session=session)
    if not isinstance(result, Image.Image):
        result = Image.open(result).convert("RGBA")
    else:
        result = result.convert("RGBA")
    output.parent.mkdir(parents=True, exist_ok=True)
    result.save(output)


def main() -> int:
    parser = argparse.ArgumentParser(description="Create an alpha PNG with rembg.")
    parser.add_argument("source", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--model", default="u2net", help="rembg model name, default: u2net")
    args = parser.parse_args()

    isolate(args.source, args.output, args.model)
    print(f"ISOLATED: {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
