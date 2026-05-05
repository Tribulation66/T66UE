#!/usr/bin/env python3
"""Copy the newest built-in ImageGen output into a requested workspace path."""

from __future__ import annotations

import shutil
import sys
from pathlib import Path


def main() -> None:
    if len(sys.argv) != 2:
        raise SystemExit("usage: copy_latest_imagegen_output.py <target.png>")

    target = Path(sys.argv[1]).resolve()
    root = Path.home() / ".codex" / "generated_images"
    if not root.exists():
        raise SystemExit(f"missing ImageGen output folder: {root}")

    candidates = [
        path
        for path in root.rglob("*")
        if path.is_file() and path.suffix.lower() in {".png", ".jpg", ".jpeg", ".webp"}
    ]
    if not candidates:
        raise SystemExit(f"no generated images found under: {root}")

    latest = max(candidates, key=lambda path: path.stat().st_mtime_ns)
    target.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(latest, target)
    print(f"{latest} -> {target}")


if __name__ == "__main__":
    main()
