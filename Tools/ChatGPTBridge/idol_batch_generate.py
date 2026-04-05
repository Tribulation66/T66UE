from __future__ import annotations

import argparse
import csv
import json
import re
import time
from dataclasses import dataclass
from datetime import UTC, datetime
from pathlib import Path

from playwright.sync_api import Page, sync_playwright
from PIL import Image


BASE_DIR = Path(__file__).resolve().parent
OUTPUT_DIR = BASE_DIR / "output"
OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

DEFAULT_CHAT_URL = "https://chatgpt.com/c/69d19de8-f4c8-83e9-aa7c-1119c8230ed4"
DEFAULT_TIMEOUT_SECONDS = 900
DEFAULT_PAUSE_SECONDS = 8.0
RARITY_ORDER = ["black", "red", "yellow", "white"]
IDOL_THEME_MAP = {
    "Curse": {
        "color": "saturated cursed violet-purple",
        "contents": "concentric curse seals, occult sigils, floating runes, ritual symbols, sacrificial glyphs",
        "shapes": "clean circular glyph rings, rune bands, sigil stamps, triangular occult marks, precise ritual linework",
    },
    "Lava": {
        "color": "molten orange-red",
        "contents": "molten magma pools, volcanic vents, glowing lava chambers, heavy molten rock heat",
        "shapes": "thick magma rivers, crater-like vents, molten blobs, volcanic channels, lava pockets",
    },
    "Poison": {
        "color": "toxic emerald green",
        "contents": "toxic venom fluid, poison reservoirs, acidic ooze, suspended toxin clouds",
        "shapes": "bubble clusters, dripping poison trails, liquid pockets, ooze channels, droplet chains",
    },
    "Bleed": {
        "color": "deep blood crimson",
        "contents": "blood vessels, ritual blood sigils, thick blood streams, clotted gore pressure",
        "shapes": "vein networks, blood drops, clotted strands, vessel branches, ritual blood marks",
    },
    "Electric": {
        "color": "electric blue-white",
        "contents": "electrical charge, lightning energy, crackling power nodes",
        "shapes": "branching lightning bolts, sharp zigzag arcs, charged focal nodes",
    },
    "Ice": {
        "color": "icy cyan-white",
        "contents": "frozen ice cores, frost bloom, glacial crystal mass, freezing vapor",
        "shapes": "ice shards, snowflake forms, crystal spikes, frost petals, frozen facets",
    },
    "Shadow": {
        "color": "deep indigo-violet black",
        "contents": "void smoke, eclipse darkness, shadow mass, oppressive black-violet mist",
        "shapes": "shadow ribbons, smoky tendrils, eclipse circles, void curls, warped mist bands",
    },
    "Star": {
        "color": "deep cosmic blue with white-gold starlight",
        "contents": "star clusters, astral glow, tiny galaxies, celestial dust, bright starlight",
        "shapes": "constellation lines, orbit rings, starbursts, nebula swirls, clustered points of light",
    },
    "Earth": {
        "color": "earthy amber-brown",
        "contents": "layered stone mass, geode crystal pockets, mineral strata, compacted earth",
        "shapes": "stone plates, mineral shards, geode seams, rock layers, faceted crystal pockets",
    },
    "Water": {
        "color": "clear azure blue",
        "contents": "tidal water flow, liquid currents, water eddies, clear aquatic force",
        "shapes": "wave ribbons, curved current bands, swirling eddies, water droplets, cresting liquid arcs",
    },
    "Black Hole": {
        "color": "deep violet-black with a thin magenta accretion glow",
        "contents": "a singularity core, compressed cosmic matter, gravitational distortion, swallowed starlight",
        "shapes": "central black sphere, accretion ring, warped orbit lines, stretched matter arcs, gravity lens distortion",
    },
    "Storm": {
        "color": "charged teal-blue",
        "contents": "storm clouds, cyclone charge, violent wind pressure, tempest electricity",
        "shapes": "spiral cyclone bands, cloud rings, wind swirls, jagged storm bolts, turbulent vortex arcs",
    },
    "Light": {
        "color": "radiant gold-white",
        "contents": "holy radiance, divine glow, sacred brilliance, sun-like luminous energy",
        "shapes": "concentric halos, clean sun rays, radiant circles, sacred sigils, luminous disks",
    },
    "Steel": {
        "color": "cold silver-gray",
        "contents": "forged steel plates, sharpened metal edges, riveted armor forms, polished iron mass",
        "shapes": "interlocking plates, blade shards, rivets, angular metal segments, faceted steel panels",
    },
    "Wood": {
        "color": "living forest green",
        "contents": "living roots, vines, bark grain, forest growth, natural sap energy",
        "shapes": "roots, twisting vines, branch forks, bark rings, leaf-like growth curls",
    },
    "Bone": {
        "color": "aged ivory bone",
        "contents": "skeletal remains, vertebra stacks, rib bone forms, fossilized ivory structure",
        "shapes": "bone shards, rib arcs, vertebra columns, skull fragments, ossified plates",
    },
}


@dataclass(frozen=True)
class ImageEntry:
    dom_index: int
    src: str
    alt: str
    natural_width: int
    natural_height: int
    rendered_width: float
    rendered_height: float

    @property
    def score(self) -> float:
        return float(max(self.natural_width, int(self.rendered_width)) * max(self.natural_height, int(self.rendered_height)))


def utc_now() -> str:
    return datetime.now(UTC).isoformat()


def slugify(value: str) -> str:
    cleaned = re.sub(r"[^a-zA-Z0-9]+", "-", value).strip("-").lower()
    return cleaned or "idol"


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Batch-generate idol four-image sets in an existing ChatGPT conversation.")
    parser.add_argument(
        "--chat-url",
        default=DEFAULT_CHAT_URL,
        help="Existing ChatGPT conversation URL containing the reference image and approved art direction.",
    )
    parser.add_argument(
        "--csv",
        default=str(Path(__file__).resolve().parents[2] / "Content" / "Data" / "Idols.csv"),
        help="Path to Idols.csv.",
    )
    parser.add_argument(
        "--manifest",
        default="",
        help="Optional explicit manifest path. Defaults under Tools/ChatGPTBridge/output.",
    )
    parser.add_argument(
        "--timeout-seconds",
        type=int,
        default=DEFAULT_TIMEOUT_SECONDS,
        help="Per-idol wait timeout for the new four-image set to appear.",
    )
    parser.add_argument(
        "--pause-seconds",
        type=float,
        default=DEFAULT_PAUSE_SECONDS,
        help="Pause between idols.",
    )
    parser.add_argument(
        "--start-at",
        default="Lava",
        help="Start from the first idol whose display name contains this text.",
    )
    parser.add_argument(
        "--filter",
        default="",
        help="Only run idols whose display name contains this text.",
    )
    parser.add_argument(
        "--limit",
        type=int,
        default=0,
        help="Run at most N idols after filtering.",
    )
    parser.add_argument(
        "--max-rate-limit-waits",
        type=int,
        default=20,
        help="Maximum number of rate-limit sleeps per idol before failing it.",
    )
    return parser.parse_args()


def display_name_from_idol_id(idol_id: str) -> str:
    value = idol_id.replace("Idol_", "")
    value = re.sub(r"([a-z])([A-Z])", r"\1 \2", value)
    return value.strip()


def load_idols(csv_path: Path) -> list[dict[str, str]]:
    rows: list[dict[str, str]] = []
    with csv_path.open("r", encoding="utf-8", newline="") as handle:
        reader = csv.DictReader(handle)
        for row in reader:
            idol_id = (row.get("IdolID") or "").strip().strip('"')
            if not idol_id or idol_id == "Idol_Curse" and False:
                continue
            rows.append(
                {
                    "idol_id": idol_id,
                    "display_name": display_name_from_idol_id(idol_id),
                }
            )
    return rows


def get_generated_images(page: Page) -> list[ImageEntry]:
    raw_items = page.evaluate(
        """
() => Array.from(document.querySelectorAll("main img")).map((img, idx) => {
  const rect = img.getBoundingClientRect();
  return {
    idx,
    src: img.currentSrc || img.src || "",
    alt: img.alt || "",
    nw: img.naturalWidth || 0,
    nh: img.naturalHeight || 0,
    w: rect.width || 0,
    h: rect.height || 0
  };
}).filter((item) => {
  const width = Math.max(item.nw, item.w);
  const height = Math.max(item.nh, item.h);
  const alt = (item.alt || "").toLowerCase();
  return width >= 256 && height >= 256 && alt.includes("generated image") && !!item.src;
});
"""
    )

    by_src: dict[str, ImageEntry] = {}
    for item in raw_items:
        entry = ImageEntry(
            dom_index=int(item["idx"]),
            src=str(item["src"]),
            alt=str(item["alt"]),
            natural_width=int(item["nw"]),
            natural_height=int(item["nh"]),
            rendered_width=float(item["w"]),
            rendered_height=float(item["h"]),
        )
        existing = by_src.get(entry.src)
        if existing is None or entry.dom_index > existing.dom_index or entry.score > existing.score:
            by_src[entry.src] = entry

    return sorted(by_src.values(), key=lambda entry: entry.dom_index)


def body_text(page: Page) -> str:
    try:
        return page.locator("body").inner_text(timeout=1000)
    except Exception:
        return ""


def detect_rate_limit_seconds(page: Page) -> int | None:
    text = body_text(page)
    match = re.search(r"wait for\s+(\d+)\s+minute", text, re.IGNORECASE)
    if "You're generating images too quickly" in text or "rate_limit_exceeded" in text or match:
        return int(match.group(1)) * 60 + 5 if match else 125
    return None


def find_or_open_page(browser, chat_url: str) -> Page:
    for context in browser.contexts:
        for page in context.pages:
            if chat_url in page.url:
                page.bring_to_front()
                page.set_default_timeout(45000)
                return page

    context = browser.contexts[0]
    page = context.new_page()
    page.set_default_timeout(45000)
    page.goto(chat_url, wait_until="domcontentloaded")
    return page


def send_prompt(page: Page, prompt: str) -> None:
    composer = page.locator("#prompt-textarea:visible, textarea:visible, div[contenteditable='true']:visible").first
    composer.wait_for(state="visible", timeout=45000)
    composer.click()
    page.keyboard.press("Control+A")
    page.keyboard.press("Backspace")
    page.keyboard.insert_text(prompt)

    send_button = page.locator(
        "button[data-testid='send-button']:visible:not([disabled]), "
        "button[aria-label*='Send']:visible:not([disabled])"
    ).first
    try:
        send_button.wait_for(state="visible", timeout=5000)
        send_button.click()
    except Exception:
        page.keyboard.press("Enter")


def compose_prompt(display_name: str, theme: dict[str, str]) -> str:
    lightning_rule = (
        "Lightning and electrical branching are allowed only because this idol is Electric or Storm."
        if display_name in {"Electric", "Storm"}
        else "Do not use lightning bolts, electrical branching, or generic energy cracks copied from the reference image."
    )
    return f"""Generate the next idol as one final square 2x2 composite sheet.

Idol: {display_name}

Output exactly one square image arranged as a 2x2 grid:
- top-left = Black rarity
- top-right = Red rarity
- bottom-left = Yellow rarity
- bottom-right = White rarity

Critical rules:
- Output exactly one final composite sheet image.
- The result must be a true edge-to-edge 2x2 sprite sheet with no outer canvas.
- Do not output separate images.
- Do not output only 3 panels.
- Use the reference image already attached in this chat.
- Use the reference image only to lock the crystal skull shell shape, silhouette, proportions, framing, and overall skull design.
- Do not copy the reference image's internal lightning pattern unless this idol explicitly calls for it.
- Keep it a crystal skull.
- Keep the crystal skull shell clear and consistent; the internal magical content is what changes.
- The only thing that changes for this idol is the internal theme and the rarity background color.
- Hard lock the internal dominant color to: {theme["color"]}.
- Do not drift away from that internal color family.
- The interior must read immediately and clearly as: {theme["contents"]}.
- The interior shape language must be dominated by: {theme["shapes"]}.
- {lightning_rule}
- Do not let the internal theme change the skull structure.
- Do not replace the skull with a different object, creature, helmet, rock, or mask.
- All 4 should match each other closely in framing and design.
- The rarity difference should mainly be the perfectly solid background color, not a redesign of the skull.
- Backgrounds must be perfectly solid single colors with no gradient, no vignette, no texture, no smoke, and no shadows.
- Black = solid black or obsidian background.
- Red = solid red background.
- Yellow = solid yellow or gold background.
- White = solid white or ivory background.
- Premium Dota 2 icon look.
- No text, no border, no frame, no rounded white card, no extra objects."""


def fetch_image_bytes(page: Page, src: str) -> bytes:
    response = page.request.get(src)
    if not response.ok:
        raise RuntimeError(f"Failed to fetch image bytes for {src} with status {response.status}.")
    return response.body()


def save_idol_set(page: Page, entries: list[ImageEntry], idol_dir: Path) -> dict[str, object]:
    idol_dir.mkdir(parents=True, exist_ok=True)
    icons_dir = idol_dir / "icons"
    icons_dir.mkdir(parents=True, exist_ok=True)

    if len(entries) == 1:
        entry = entries[0]
        sheet_path = idol_dir / "sheet.png"
        sheet_path.write_bytes(fetch_image_bytes(page, entry.src))
        with Image.open(sheet_path) as image:
            width, height = image.size
            half_w = width // 2
            half_h = height // 2
            boxes = {
                "black": (0, 0, half_w, half_h),
                "red": (half_w, 0, width, half_h),
                "yellow": (0, half_h, half_w, height),
                "white": (half_w, half_h, width, height),
            }
            saved_icons: dict[str, str] = {}
            for rarity, box in boxes.items():
                icon_path = icons_dir / f"{rarity}.png"
                image.crop(box).save(icon_path)
                saved_icons[rarity] = str(icon_path)

        return {
            "sheet": str(sheet_path),
            "saved_icons": saved_icons,
        }

    saved_icons: dict[str, str] = {}
    for rarity, entry in zip(RARITY_ORDER, entries[:4], strict=True):
        output_path = icons_dir / f"{rarity}.png"
        output_path.write_bytes(fetch_image_bytes(page, entry.src))
        saved_icons[rarity] = str(output_path)
    return {
        "saved_icons": saved_icons,
    }


def try_capture_chat_snapshot(page: Page, screenshot_path: Path) -> str | None:
    try:
        page.screenshot(path=str(screenshot_path), full_page=False, timeout=15000)
        return str(screenshot_path)
    except Exception:
        return None


def idol_is_complete(idol_dir: Path) -> bool:
    return all((idol_dir / "icons" / f"{rarity}.png").is_file() for rarity in RARITY_ORDER)


def wait_for_new_output(
    page: Page,
    baseline_keys: set[str],
    timeout_seconds: int,
    max_rate_limit_waits: int,
) -> list[ImageEntry]:
    deadline = time.monotonic() + timeout_seconds
    last_change = time.monotonic()
    last_new_count = -1
    rate_limit_waits = 0

    while time.monotonic() < deadline:
        retry_after = detect_rate_limit_seconds(page)
        if retry_after is not None:
            rate_limit_waits += 1
            if rate_limit_waits > max_rate_limit_waits:
                raise RuntimeError("Exceeded rate-limit wait budget for this idol.")
            time.sleep(retry_after)
            continue

        current_entries = get_generated_images(page)
        new_entries = [entry for entry in current_entries if entry.src not in baseline_keys]
        new_count = len(new_entries)

        if new_count != last_new_count:
            last_new_count = new_count
            last_change = time.monotonic()
        elif new_count == 1 and (time.monotonic() - last_change) >= 10:
            return new_entries[-1:]
        elif new_count == 4 and (time.monotonic() - last_change) >= 10:
            raise RuntimeError(
                "ChatGPT returned four separate images instead of one true 2x2 composite sheet."
            )

        time.sleep(2)

    current_entries = get_generated_images(page)
    new_entries = [entry for entry in current_entries if entry.src not in baseline_keys]
    if len(new_entries) == 1:
        return new_entries[-1:]
    if len(new_entries) == 4:
        raise RuntimeError(
            "Timed out waiting for a true 2x2 sheet; ChatGPT returned four separate images instead."
        )
    raise RuntimeError(f"Timed out waiting for a new idol output. Saw {len(new_entries)} new image(s).")


def write_manifest(path: Path, data: dict) -> None:
    path.write_text(json.dumps(data, indent=2), encoding="utf-8")


def main() -> int:
    args = parse_args()
    csv_path = Path(args.csv).resolve()
    idols = load_idols(csv_path)

    if args.start_at:
        start_index = next(
            (
                index
                for index, item in enumerate(idols)
                if args.start_at.lower() in item["display_name"].lower()
            ),
            None,
        )
        if start_index is None:
            raise ValueError(f"Could not find start-at marker: {args.start_at}")
        idols = idols[start_index:]

    if args.filter:
        idols = [item for item in idols if args.filter.lower() in item["display_name"].lower()]

    if args.limit > 0:
        idols = idols[: args.limit]

    if not idols:
        raise ValueError("No idols matched the requested filters.")

    manifest_path = (
        Path(args.manifest).resolve()
        if args.manifest
        else OUTPUT_DIR / f"idol-batch-{datetime.now(UTC).strftime('%Y%m%d-%H%M%S')}.json"
    )
    manifest_path.parent.mkdir(parents=True, exist_ok=True)
    run_output_dir = manifest_path.parent / manifest_path.stem
    run_output_dir.mkdir(parents=True, exist_ok=True)

    manifest = {
        "started_at": utc_now(),
        "chat_url": args.chat_url,
        "csv_path": str(csv_path),
        "run_output_dir": str(run_output_dir),
        "timeout_seconds": args.timeout_seconds,
        "pause_seconds": args.pause_seconds,
        "results": [],
    }
    write_manifest(manifest_path, manifest)

    with sync_playwright() as p:
        browser = p.chromium.connect_over_cdp("http://127.0.0.1:9222")
        page = find_or_open_page(browser, args.chat_url)

        total = len(idols)
        for index, item in enumerate(idols, start=1):
            display_name = item["display_name"]
            idol_slug = slugify(display_name)
            idol_dir = run_output_dir / idol_slug
            screenshot_path = idol_dir / "chat-snapshot.png"

            if idol_is_complete(idol_dir):
                result = {
                    "idol_id": item["idol_id"],
                    "display_name": display_name,
                    "status": "skipped_existing",
                    "saved_icons": {rarity: str(idol_dir / "icons" / f"{rarity}.png") for rarity in RARITY_ORDER},
                    "finished_at": utc_now(),
                }
                manifest["results"].append(result)
                write_manifest(manifest_path, manifest)
                print(f"[{index}/{total}] Skipping {display_name}; files already exist.", flush=True)
                continue

            theme = IDOL_THEME_MAP.get(display_name)
            if theme is None:
                raise ValueError(f"Missing theme mapping for idol: {display_name}")

            print(f"[{index}/{total}] Generating {display_name}...", flush=True)
            baseline_keys = {entry.src for entry in get_generated_images(page)}
            prompt = compose_prompt(display_name, theme)
            send_prompt(page, prompt)

            try:
                new_entries = wait_for_new_output(
                    page,
                    baseline_keys,
                    args.timeout_seconds,
                    args.max_rate_limit_waits,
                )
                saved_payload = save_idol_set(page, new_entries, idol_dir)
                chat_snapshot = try_capture_chat_snapshot(page, screenshot_path)
                result = {
                    "idol_id": item["idol_id"],
                    "display_name": display_name,
                    "status": "completed",
                    **saved_payload,
                    "screenshot": chat_snapshot,
                    "finished_at": utc_now(),
                }
                manifest["results"].append(result)
                write_manifest(manifest_path, manifest)
                print(f"[{index}/{total}] Completed {display_name}.", flush=True)
            except Exception as exc:
                chat_snapshot = try_capture_chat_snapshot(page, screenshot_path)
                result = {
                    "idol_id": item["idol_id"],
                    "display_name": display_name,
                    "status": "failed",
                    "error": str(exc),
                    "screenshot": chat_snapshot,
                    "finished_at": utc_now(),
                }
                manifest["results"].append(result)
                write_manifest(manifest_path, manifest)
                print(f"[{index}/{total}] Failed {display_name}: {exc}", flush=True)
                return 1

            time.sleep(args.pause_seconds)

    manifest["completed_at"] = utc_now()
    write_manifest(manifest_path, manifest)
    print(f"Manifest: {manifest_path}", flush=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
