from __future__ import annotations

import argparse
import hashlib
import json
import re
import shutil
import sys
import time
from datetime import UTC, datetime
from pathlib import Path

from PIL import Image
from playwright.sync_api import sync_playwright

from server import (
    BridgeError,
    GeneratedAsset,
    RateLimitError,
    connect_and_generate,
    fetch_image_from_page,
    fill_and_send_prompt,
    get_image_candidates,
    open_generation_page,
    save_output,
    slugify,
    wait_for_new_images,
)

RARITY_ORDER = ["Black", "Red", "Yellow", "White"]
RARITY_BACKGROUND = {
    "Black": "deep charcoal, obsidian, dark ash",
    "Red": "crimson, blood red",
    "Yellow": "gold, amber",
    "White": "pale ivory, holy white",
}
SINGLE_IMAGE_CONTEXT = """You are generating a single square item icon for a fantasy action roguelike. The icon must look like a premium Dota 2 item icon: painterly fantasy MOBA item art, high readability, strong silhouette, centered object, dramatic lighting, rich materials, polished game UI finish. Do not make pixel art. Do not make cartoon-goofy props. Do not add text, letters, numbers, borders, UI frames, hands, characters, or cluttered scenes.

This request is for exactly one item icon. Keep the object centered, fill most of the frame, and preserve strong readability at small icon size.

The background must be simple and match the requested rarity color:
- Black rarity: deep charcoal, obsidian, dark ash
- Red rarity: crimson, blood red
- Yellow rarity: gold, amber
- White rarity: pale ivory, holy white

Export exactly one square icon only."""
GRID_SHEET_CONTEXT = """You are generating one final square 2x2 composite item icon sheet for a fantasy action roguelike. The style must look explicitly like a premium Dota 2 item icon sheet: polished hand-painted fantasy MOBA item rendering, rich materials, strong silhouette, elegant ornament, clean presentation, and high readability. Do not make pixel art. Do not make a scene. Do not add text, numbers, labels, borders, UI frames, characters, hands, or clutter.

Output exactly one square image arranged as a 2x2 grid:
- top-left: Black rarity
- top-right: Red rarity
- bottom-left: Yellow rarity
- bottom-right: White rarity

All four quadrants must show the same item from nearly the same camera angle and framing. The item should stay centered and large in each quadrant. Upgrades between rarities should mainly come from materials, trim, ornament, and restrained energy accents.

The backgrounds must be perfectly flat solid rarity colors:
- Black rarity: deep charcoal, obsidian, dark ash
- Red rarity: crimson, blood red
- Yellow rarity: gold, amber
- White rarity: pale ivory, holy white

Do not use gradients, vignettes, spotlighting, texture, paper grain, smoke, haze, painted backdrop effects, or visible lighting falloff in the background. The background should read like a clean uniform color field behind the item.

Lighting direction:
- bright artificial studio-style item lighting
- the item should be clearly lit and readable
- no cast shadow on the background
- no dark grounding shadow under the item
- no moody background lighting
- keep the background bright, even, and flat

Keep glow and effects at a minimum. Avoid smoke, sparks, magical trails, big bloom, noisy lighting, or busy backgrounds. The result should look clean, readable, and game-ready."""


def utc_now() -> str:
    return datetime.now(UTC).isoformat()


def parse_prompt_pack(pack_path: Path) -> tuple[str, list[tuple[str, str]]]:
    text = pack_path.read_text(encoding="utf-8")

    master_match = re.search(
        r"## Master Context Prompt\s+```text\n(.*?)\n```",
        text,
        flags=re.DOTALL,
    )
    if not master_match:
        raise ValueError(f"Could not find master context prompt in {pack_path}.")
    master_context = master_match.group(1).strip()

    prompt_blocks = [
        (match.group(1).strip(), match.group(2).strip())
        for match in re.finditer(
            r"^### (.+?)\n\n```text\n(.*?)\n```",
            text,
            flags=re.MULTILINE | re.DOTALL,
        )
    ]
    if not prompt_blocks:
        raise ValueError(f"Could not find any item prompt blocks in {pack_path}.")
    return master_context, prompt_blocks


def compose_prompt(master_context: str, block_prompt: str) -> str:
    return f"{master_context}\n\n{block_prompt}".strip()


def extract_prompt_components(title: str, block_prompt: str) -> tuple[str, dict[str, str]]:
    lines = [line.rstrip() for line in block_prompt.splitlines()]
    if not lines:
        raise ValueError(f"Prompt block for {title} is empty.")

    intro_lines: list[str] = []
    rarity_descriptions: dict[str, str] = {}
    state = "intro"

    for raw_line in lines[1:]:
        line = raw_line.strip()
        if line == "Rarity variants:":
            state = "rarities"
            continue
        if line == "Requirements:":
            break
        if state == "intro":
            intro_lines.append(raw_line)
            continue
        rarity_match = re.match(r"^\d+\.\s+(Black|Red|Yellow|White) rarity:\s*(.+)$", line)
        if rarity_match:
            rarity_descriptions[rarity_match.group(1)] = rarity_match.group(2).strip()

    intro_text = "\n".join(intro_lines).strip()
    if not intro_text:
        raise ValueError(f"Could not parse intro text for {title}.")
    return intro_text, rarity_descriptions


def split_block_for_rarities(title: str, block_prompt: str) -> list[tuple[str, str]]:
    intro_text, rarity_descriptions = extract_prompt_components(title, block_prompt)
    expanded: list[tuple[str, str]] = []
    for rarity in RARITY_ORDER:
        description = rarity_descriptions.get(rarity)
        if not description:
            raise ValueError(f"Could not find {rarity} rarity description for {title}.")
        prompt = (
            f"{SINGLE_IMAGE_CONTEXT}\n\n"
            f"Generate exactly one square item icon for {title}.\n\n"
            f"{intro_text}\n\n"
            f"Target rarity:\n"
            f"- {rarity} rarity: {description}\n\n"
            f"Requirements:\n"
            f"- generate exactly one image only\n"
            f"- this image should represent the {rarity} rarity version of {title}\n"
            f"- keep the same core item identity used for other rarity versions of this item\n"
            f"- centered object\n"
            f"- clean {RARITY_BACKGROUND[rarity]} background only\n"
            f"- no text\n"
            f"- no border\n"
            f"- no pixel-art look\n"
            f"- no goofy or toy-like style"
        ).strip()
        expanded.append((f"{title} - {rarity}", prompt))

    return expanded


def compose_grid_prompt(title: str, block_prompt: str) -> str:
    intro_text, rarity_descriptions = extract_prompt_components(title, block_prompt)
    missing = [rarity for rarity in RARITY_ORDER if rarity not in rarity_descriptions]
    if missing:
        raise ValueError(f"Missing rarity descriptions for {title}: {', '.join(missing)}")

    return (
        f"{GRID_SHEET_CONTEXT}\n\n"
        f"Generate exactly one 2x2 item sheet for {title}.\n\n"
        f"{intro_text}\n\n"
        f"Quadrant instructions:\n"
        f"- Top-left Black rarity: {rarity_descriptions['Black']}\n"
        f"- Top-right Red rarity: {rarity_descriptions['Red']}\n"
        f"- Bottom-left Yellow rarity: {rarity_descriptions['Yellow']}\n"
        f"- Bottom-right White rarity: {rarity_descriptions['White']}\n\n"
        f"Critical constraints:\n"
        f"- produce exactly one composite 2x2 sheet image\n"
        f"- all four quadrants must keep the same object identity and nearly the same camera framing\n"
        f"- each quadrant background must be a perfectly flat solid rarity color\n"
        f"- no gradient background, no vignette, no textured background, no lighting falloff in the background\n"
        f"- explicit premium Dota 2 item icon look\n"
        f"- bright artificial lighting on the item\n"
        f"- no cast shadow or dark grounding shadow on the background\n"
        f"- keep the background bright and evenly lit\n"
        f"- glow and special effects must be restrained and minimal\n"
        f"- no scenery, no particles, no extra decorative objects, no labels, no frames"
    ).strip()


def write_manifest(manifest_path: Path, payload: dict) -> None:
    manifest_path.write_text(json.dumps(payload, indent=2), encoding="utf-8")


def relocate_assets(assets, run_output_dir: Path, title: str) -> list[str]:
    title_slug = slugify(title, max_length=80)
    target_dir = run_output_dir / title_slug
    target_dir.mkdir(parents=True, exist_ok=True)

    relocated_paths: list[str] = []
    total = len(assets)
    for index, asset in enumerate(assets, start=1):
        suffix = f"-{index:02d}" if total > 1 else ""
        target_path = target_dir / f"{title_slug}{suffix}{asset.path.suffix.lower()}"
        shutil.move(str(asset.path), str(target_path))
        relocated_paths.append(str(target_path))
    return relocated_paths


def relocate_grid_assets(assets, run_output_dir: Path, title: str) -> dict[str, object]:
    title_slug = slugify(title, max_length=80)
    target_dir = run_output_dir / title_slug
    sheets_dir = target_dir / "sheet"
    icons_dir = target_dir / "icons"
    sheets_dir.mkdir(parents=True, exist_ok=True)
    icons_dir.mkdir(parents=True, exist_ok=True)

    sheet_paths: list[str] = []
    for index, asset in enumerate(assets, start=1):
        sheet_name = "sheet" if index == 1 else f"sheet-alt-{index:02d}"
        target_path = sheets_dir / f"{sheet_name}{asset.path.suffix.lower()}"
        shutil.move(str(asset.path), str(target_path))
        sheet_paths.append(str(target_path))

    primary_sheet = Path(sheet_paths[0])
    with Image.open(primary_sheet) as image:
        width, height = image.size
        half_w = width // 2
        half_h = height // 2
        quadrants = {
            "black": (0, 0, half_w, half_h),
            "red": (half_w, 0, width, half_h),
            "yellow": (0, half_h, half_w, height),
            "white": (half_w, half_h, width, height),
        }

        icon_paths: dict[str, str] = {}
        for rarity, box in quadrants.items():
            cropped = image.crop(box)
            icon_path = icons_dir / f"{rarity}.png"
            cropped.save(icon_path)
            icon_paths[rarity] = str(icon_path)

    return {
        "sheet": str(primary_sheet),
        "sheet_variants": sheet_paths,
        "icons": icon_paths,
    }


def persist_candidates(page, candidates, prompt_stem: str) -> list[GeneratedAsset]:
    assets: list[GeneratedAsset] = []
    seen_hashes: set[str] = set()
    total = len(candidates)

    for index, candidate in enumerate(candidates, start=1):
        image_bytes, mime_type = fetch_image_from_page(page, candidate)
        content_hash = hashlib.sha256(image_bytes).hexdigest()
        if content_hash in seen_hashes:
            continue
        seen_hashes.add(content_hash)
        file_stem = prompt_stem if total == 1 else f"{prompt_stem}-{index:02d}"
        output_path = save_output(image_bytes, mime_type, file_stem)
        assets.append(GeneratedAsset(path=output_path, mime_type=mime_type, data=image_bytes))

    if not assets:
        raise BridgeError("Generated images were detected in the DOM, but no unique image files could be extracted.")
    return assets


def harvest_assets_since_baseline(page, baseline_keys: set[str], prompt_stem: str) -> list[GeneratedAsset]:
    candidates = [
        candidate
        for candidate in get_image_candidates(page)
        if candidate.key not in baseline_keys
    ]
    if not candidates:
        return []
    candidates = sorted(candidates, key=lambda candidate: (candidate.dom_index, -candidate.score))
    return persist_candidates(page, candidates, prompt_stem)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Batch-generate ChatGPT web images from Item_Icon_Prompt_Pack.md.")
    parser.add_argument(
        "--pack",
        default=str(Path(__file__).resolve().parents[2] / "Item_Icon_Prompt_Pack.md"),
        help="Path to the markdown prompt pack.",
    )
    parser.add_argument(
        "--manifest",
        default="",
        help="Optional explicit manifest output path. Defaults under Tools/ChatGPTBridge/output/.",
    )
    parser.add_argument(
        "--start-at",
        default="",
        help="Start from the first item title containing this text.",
    )
    parser.add_argument(
        "--filter",
        default="",
        help="Only run items whose titles contain this text.",
    )
    parser.add_argument(
        "--limit",
        type=int,
        default=0,
        help="Run at most N items after filtering.",
    )
    parser.add_argument(
        "--timeout-seconds",
        type=int,
        default=240,
        help="Per-prompt timeout while waiting for generated images.",
    )
    parser.add_argument(
        "--pause-seconds",
        type=float,
        default=8.0,
        help="Pause between prompts so the UI can settle.",
    )
    parser.add_argument(
        "--continue-on-error",
        action="store_true",
        help="Continue batch processing after a failed item.",
    )
    parser.add_argument(
        "--split-rarities",
        action="store_true",
        help="Expand each item prompt into one prompt per rarity so the batch produces one image per rarity.",
    )
    parser.add_argument(
        "--grid-sheet",
        action="store_true",
        help="Generate one 2x2 rarity sheet per item and auto-split it into black/red/yellow/white icon files.",
    )
    parser.add_argument(
        "--same-chat",
        action="store_true",
        help="Reuse the same ChatGPT conversation page for the whole batch instead of opening a fresh chat per prompt.",
    )
    parser.add_argument(
        "--max-rate-limit-retries",
        type=int,
        default=10,
        help="Maximum retries for a prompt when ChatGPT reports an image generation rate limit.",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    pack_path = Path(args.pack).resolve()
    master_context, prompt_blocks = parse_prompt_pack(pack_path)

    if args.split_rarities and args.grid_sheet:
        raise ValueError("Use either --split-rarities or --grid-sheet, not both.")

    if args.start_at:
        start_index = next(
            (
                index
                for index, (title, _) in enumerate(prompt_blocks)
                if args.start_at.lower() in title.lower()
            ),
            None,
        )
        if start_index is None:
            raise ValueError(f"Could not find start-at marker: {args.start_at}")
        prompt_blocks = prompt_blocks[start_index:]

    if args.filter:
        prompt_blocks = [
            (title, block_prompt)
            for title, block_prompt in prompt_blocks
            if args.filter.lower() in title.lower()
        ]

    if args.limit > 0:
        prompt_blocks = prompt_blocks[: args.limit]

    if not prompt_blocks:
        raise ValueError("No prompts matched the requested filters.")

    expanded_prompts: list[tuple[str, str]]
    if args.grid_sheet:
        expanded_prompts = [
            (title, compose_grid_prompt(title, block_prompt))
            for title, block_prompt in prompt_blocks
        ]
    elif args.split_rarities:
        expanded_prompts = []
        for title, block_prompt in prompt_blocks:
            expanded_prompts.extend(split_block_for_rarities(title, block_prompt))
    else:
        expanded_prompts = [
            (title, compose_prompt(master_context, block_prompt))
            for title, block_prompt in prompt_blocks
        ]

    manifest_path = (
        Path(args.manifest).resolve()
        if args.manifest
        else Path(__file__).resolve().parent / "output" / f"batch-{datetime.now(UTC).strftime('%Y%m%d-%H%M%S')}.json"
    )
    manifest_path.parent.mkdir(parents=True, exist_ok=True)
    run_output_dir = manifest_path.parent / manifest_path.stem
    run_output_dir.mkdir(parents=True, exist_ok=True)

    manifest = {
        "started_at": utc_now(),
        "pack_path": str(pack_path),
        "timeout_seconds": args.timeout_seconds,
        "pause_seconds": args.pause_seconds,
        "item_count": len(expanded_prompts),
        "split_rarities": args.split_rarities,
        "grid_sheet": args.grid_sheet,
        "same_chat": args.same_chat,
        "run_output_dir": str(run_output_dir),
        "results": [],
    }
    write_manifest(manifest_path, manifest)

    total = len(expanded_prompts)
    failures = 0
    playwright_manager = None
    playwright = None
    page = None

    try:
        if args.same_chat:
            playwright_manager = sync_playwright()
            playwright = playwright_manager.start()
            _browser, _context, page = open_generation_page(playwright)

        for index, (title, full_prompt) in enumerate(expanded_prompts, start=1):
            print(f"[{index}/{total}] Generating {title}...", flush=True)
            started = time.monotonic()
            prompt_stem = f"{now_stamp()}-{slugify(full_prompt)}"
            while True:
                try:
                    if page is not None:
                        baseline_keys = {candidate.key for candidate in get_image_candidates(page)}
                        fill_and_send_prompt(page, full_prompt)

                        rate_limit_retries = 0
                        while True:
                            try:
                                candidates = wait_for_new_images(page, baseline_keys, args.timeout_seconds)
                                assets = persist_candidates(page, candidates, prompt_stem)
                                break
                            except RateLimitError as exc:
                                rate_limit_retries += 1
                                if rate_limit_retries > args.max_rate_limit_retries:
                                    salvaged_assets = harvest_assets_since_baseline(page, baseline_keys, prompt_stem)
                                    if salvaged_assets:
                                        assets = salvaged_assets
                                        break
                                    raise BridgeError(
                                        f"Exceeded rate-limit retry budget while waiting for {title}."
                                    ) from exc

                                print(
                                    f"[{index}/{total}] Rate limited on {title}; pausing {exc.retry_after_seconds}s and continuing to wait for the same prompt.",
                                    flush=True,
                                )
                                time.sleep(exc.retry_after_seconds)
                            except BridgeError:
                                salvaged_assets = harvest_assets_since_baseline(page, baseline_keys, prompt_stem)
                                if salvaged_assets:
                                    assets = salvaged_assets
                                    break
                                raise
                    else:
                        assets = connect_and_generate(full_prompt, args.timeout_seconds)

                    if args.grid_sheet:
                        grid_result = relocate_grid_assets(assets, run_output_dir, title)
                        duration = round(time.monotonic() - started, 2)
                        result = {
                            "title": title,
                            "status": "ok",
                            "duration_seconds": duration,
                            "sheet": grid_result["sheet"],
                            "sheet_variants": grid_result["sheet_variants"],
                            "icons": grid_result["icons"],
                        }
                        print(
                            f"[{index}/{total}] Completed {title}: sheet + 4 split icons in {duration:.2f}s",
                            flush=True,
                        )
                    else:
                        relocated_files = relocate_assets(assets, run_output_dir, title)
                        duration = round(time.monotonic() - started, 2)
                        result = {
                            "title": title,
                            "status": "ok",
                            "duration_seconds": duration,
                            "image_count": len(relocated_files),
                            "files": relocated_files,
                        }
                        print(
                            f"[{index}/{total}] Completed {title}: {len(relocated_files)} image(s) in {duration:.2f}s",
                            flush=True,
                        )
                    break
                except RateLimitError as exc:
                    print(
                        f"[{index}/{total}] Rate limited on {title}; pausing {exc.retry_after_seconds}s before retrying.",
                        flush=True,
                    )
                    time.sleep(exc.retry_after_seconds)
                    continue
                except Exception as exc:
                    failures += 1
                    duration = round(time.monotonic() - started, 2)
                    result = {
                        "title": title,
                        "status": "error",
                        "duration_seconds": duration,
                        "error": str(exc),
                    }
                    print(
                        f"[{index}/{total}] Failed {title} after {duration:.2f}s: {exc}",
                        flush=True,
                    )
                    break

            manifest["results"].append(result)
            write_manifest(manifest_path, manifest)

            if result["status"] == "error" and not args.continue_on_error:
                break

            if args.pause_seconds > 0 and index < total:
                time.sleep(args.pause_seconds)
    finally:
        if page is not None:
            page.close()
        if playwright is not None:
            playwright.stop()

    manifest["completed_at"] = utc_now()
    manifest["failure_count"] = failures
    manifest["success_count"] = sum(1 for result in manifest["results"] if result["status"] == "ok")
    write_manifest(manifest_path, manifest)

    print(f"Manifest written to {manifest_path}", flush=True)
    return 0 if failures == 0 else 1


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except BridgeError as exc:
        print(str(exc), file=sys.stderr)
        raise SystemExit(1)
