from __future__ import annotations

import argparse
import csv
import math
import shutil
import statistics
from dataclasses import dataclass
from pathlib import Path

from PIL import Image, ImageDraw, ImageFilter, ImageFont


ROOT = Path(__file__).resolve().parents[2]
PASS_ROOT = ROOT / "SourceAssets" / "ItemSprites" / "_ReimaginedItemPass_20260427"
LIVE_SPRITES_DIR = ROOT / "SourceAssets" / "ItemSprites"
ITEMS_CSV = ROOT / "Content" / "Data" / "Items.csv"
SHEETS_DIR = PASS_ROOT / "raw_sheets"
SOURCE_DIR = PASS_ROOT / "source_icons"
PIXELATED_DIR = PASS_ROOT / "pixelated_icons"
CONTACT_DIR = PASS_ROOT / "contact_sheets"

RARITY_POSITIONS = {
    "black": (0, 0),
    "red": (1, 0),
    "yellow": (0, 1),
    "white": (1, 1),
}


@dataclass(frozen=True)
class Variant:
    rarity: str
    object_name: str
    fantasy_name: str


@dataclass(frozen=True)
class Series:
    family: str
    series: str
    sheet_name: str
    variants: tuple[Variant, ...]


SERIES: tuple[Series, ...] = (
    Series("Damage", "AoeDamage", "Damage_AoeDamage_sheet.png", (
        Variant("black", "cracked smoke detector", "Halo of Ruin"),
        Variant("red", "dented saucepan lid", "Thunder Moon"),
        Variant("yellow", "broken speaker cone", "Cathedral Boomer"),
        Variant("white", "flattened hubcap", "Sainted Shockring"),
    )),
    Series("Damage", "BounceDamage", "Damage_BounceDamage_sheet.png", (
        Variant("black", "bent bottle cap", "Ricochet Crown"),
        Variant("red", "cracked ping-pong ball", "Moon-Hop Pearl"),
        Variant("yellow", "loose drawer knob", "Imp Orbit"),
        Variant("white", "rubber stopper", "Rebound Relic"),
    )),
    Series("Damage", "PierceDamage", "Damage_PierceDamage_sheet.png", (
        Variant("black", "rusty nail", "Needle of Judgment"),
        Variant("red", "snapped umbrella rib", "Storm Thorn"),
        Variant("yellow", "broken pen nib", "Scholar's Fang"),
        Variant("white", "bent sewing pin", "Silver Whisper"),
    )),
    Series("Damage", "DotDamage", "Damage_DotDamage_sheet.png", (
        Variant("black", "burnt matchbook", "Ashen Scripture"),
        Variant("red", "corroded battery", "Leaking Sun"),
        Variant("yellow", "old tea bag tag", "Witch's Flag"),
        Variant("white", "stained cork", "Slow Curse Seal"),
    )),
    Series("Accuracy", "CritDamage", "Accuracy_CritDamage_sheet.png", (
        Variant("black", "cracked magnifier", "Eye of Catastrophe"),
        Variant("red", "broken watch gear", "Fate Tooth"),
        Variant("yellow", "chipped chess queen", "Execution Idol"),
        Variant("white", "loose camera lens", "Doomglass"),
    )),
    Series("AttackSpeed", "AoeSpeed", "AttackSpeed_AoeSpeed_sheet.png", (
        Variant("black", "soda pull tab", "Cyclone Halo"),
        Variant("red", "fan screw", "Wind Saint's Nail"),
        Variant("yellow", "toy propeller", "Zephyr Relic"),
        Variant("white", "bottle opener ring", "Storm Handle"),
    )),
    Series("AttackSpeed", "BounceSpeed", "AttackSpeed_BounceSpeed_sheet.png", (
        Variant("black", "rubber band", "Serpent Loop"),
        Variant("red", "yo-yo string spool", "Orbit Spinner"),
        Variant("yellow", "spring from a pen", "Jester Coil"),
        Variant("white", "plastic wheel", "Runaway Moon"),
    )),
    Series("AttackSpeed", "PierceSpeed", "AttackSpeed_PierceSpeed_sheet.png", (
        Variant("black", "bent paperclip", "Lightning Hook"),
        Variant("red", "zipper tooth strip", "Serpent Teeth"),
        Variant("yellow", "snapped drill bit", "Haste Needle"),
        Variant("white", "broken dart tip", "Swift Verdict"),
    )),
    Series("AttackSpeed", "DotSpeed", "AttackSpeed_DotSpeed_sheet.png", (
        Variant("black", "burnt fuse", "Blackwick Psalm"),
        Variant("red", "frayed shoelace", "Creeping Thread"),
        Variant("yellow", "cracked egg timer dial", "Rot Clock"),
        Variant("white", "incense ash tray", "Patient Doom"),
    )),
    Series("Accuracy", "CritChance", "Accuracy_CritChance_sheet.png", (
        Variant("black", "lucky penny", "Oracle Coin"),
        Variant("red", "cracked compact mirror", "Truth Shard"),
        Variant("yellow", "broken dice cube", "Fate Apostate"),
        Variant("white", "lottery ticket scrap", "Prophet's Receipt"),
    )),
    Series("AttackScale", "AoeScale", "AttackScale_AoeScale_sheet.png", (
        Variant("black", "crushed pie tin", "Expanding Moon"),
        Variant("red", "jar lid", "Giant's Seal"),
        Variant("yellow", "old CD", "Mirror Disc of Heaven"),
        Variant("white", "drain strainer", "World-Eater Grate"),
    )),
    Series("AttackScale", "BounceScale", "AttackScale_BounceScale_sheet.png", (
        Variant("black", "rubber washer", "Titan Loop"),
        Variant("red", "bracelet clasp", "Monarch Ringlet"),
        Variant("yellow", "toy tire", "Rolling Shrine"),
        Variant("white", "curtain grommet", "Orbit Gate"),
    )),
    Series("AttackScale", "PierceScale", "AttackScale_PierceScale_sheet.png", (
        Variant("black", "wooden skewer", "Lance of the Table God"),
        Variant("red", "broken comb tooth", "Ivory Spine"),
        Variant("yellow", "cocktail umbrella stick", "Festival Spear"),
        Variant("white", "cracked ruler shard", "Lawblade"),
    )),
    Series("AttackScale", "DotScale", "AttackScale_DotScale_sheet.png", (
        Variant("black", "coffee filter", "Brown Eclipse Veil"),
        Variant("red", "wax paper scrap", "Grease Prophet"),
        Variant("yellow", "dryer lint clump", "Dust Familiar"),
        Variant("white", "burnt toast corner", "Charred Covenant"),
    )),
    Series("Accuracy", "AttackRange", "Accuracy_AttackRange_sheet.png", (
        Variant("black", "broken antenna", "Far-Calling Wand"),
        Variant("red", "cracked telescope toy", "Plastic Stargazer"),
        Variant("yellow", "old key fob", "Distant Omen"),
        Variant("white", "snapped tape measure", "Horizon Tongue"),
    )),
    Series("Accuracy", "Accuracy", "Accuracy_Accuracy_sheet.png", (
        Variant("black", "laser pointer shell", "Red Dot Oracle"),
        Variant("red", "eyeglass screw", "Tiny Seer"),
        Variant("yellow", "camera shutter button", "Moment Judge"),
        Variant("white", "crosshair sticker scrap", "Perfect Mark"),
    )),
    Series("Armor", "Taunt", "Armor_Taunt_sheet.png", (
        Variant("black", "torn party horn", "Insult Trumpet"),
        Variant("red", "novelty badge", "King of Nothing"),
        Variant("yellow", "cracked sunglasses", "Defiant Shades"),
        Variant("white", "bottle label face", "Mocking Banner"),
    )),
    Series("Armor", "ReflectDamage", "Armor_ReflectDamage_sheet.png", (
        Variant("black", "foil gum wrapper", "Mirror Leaf"),
        Variant("red", "broken compact lid", "Vanity Aegis"),
        Variant("yellow", "scratched CD fragment", "Silver Rebuttal"),
        Variant("white", "chrome faucet handle", "Return Star"),
    )),
    Series("Armor", "HpRegen", "Armor_HpRegen_sheet.png", (
        Variant("black", "rain gutter seed pod", "Green Returner"),
        Variant("red", "wilted houseplant tag", "Root Promise"),
        Variant("yellow", "cracked watering cap", "Gentle Reservoir"),
        Variant("white", "compostable cup shard", "Second Spring"),
    )),
    Series("Armor", "Crush", "Armor_Crush_sheet.png", (
        Variant("black", "brick chip", "Pebble Tyrant"),
        Variant("red", "cracked padlock", "Prison Heart"),
        Variant("yellow", "metal nut", "Iron Seed"),
        Variant("white", "broken caster wheel", "Weight of Ages"),
    )),
    Series("Armor", "DamageReduction", "Armor_DamageReduction_sheet.png", (
        Variant("black", "cardboard corner guard", "Soft Fortress"),
        Variant("red", "packing foam block", "Cloud Bastion"),
        Variant("yellow", "cracked phone case", "Palm Shield"),
        Variant("white", "rubber shoe sole", "Roadhide Plate"),
    )),
    Series("Evasion", "Invisibility", "Evasion_Invisibility_sheet.png", (
        Variant("black", "transparent tape wad", "Ghost Ribbon"),
        Variant("red", "clear plastic wrapper", "Air Cloak"),
        Variant("yellow", "smudged screen protector", "Vanishing Skin"),
        Variant("white", "glass bottle shard", "Invisible Fang"),
    )),
    Series("Evasion", "CounterAttack", "Evasion_CounterAttack_sheet.png", (
        Variant("black", "bent mousetrap spring", "Biteback Coil"),
        Variant("red", "snap bracelet strip", "Ambush Band"),
        Variant("yellow", "broken clothespin", "Retort Jaw"),
        Variant("white", "safety pin", "Revenge Hook"),
    )),
    Series("Evasion", "LifeSteal", "Evasion_LifeSteal_sheet.png", (
        Variant("black", "spent straw", "Siphon Reed"),
        Variant("red", "cracked vacuum nozzle", "Hungry Mouth"),
        Variant("yellow", "leaky hose connector", "Thief's Valve"),
        Variant("white", "old juice box straw wrap", "Sweet Theft"),
    )),
    Series("Evasion", "Assassinate", "Evasion_Assassinate_sheet.png", (
        Variant("black", "snapped razor cap", "Silent Edge"),
        Variant("red", "black button", "Night Eye"),
        Variant("yellow", "broken craft blade", "Whisper Guillotine"),
        Variant("white", "chipped black tile", "Midnight Warrant"),
    )),
    Series("Evasion", "EvasionChance", "Evasion_EvasionChance_sheet.png", (
        Variant("black", "lost receipt", "Alibi Scroll"),
        Variant("red", "bus ticket stub", "Escape Token"),
        Variant("yellow", "bent playing card", "Trickster's Leave"),
        Variant("white", "loose hoodie drawstring tip", "Slip Charm"),
    )),
    Series("Luck", "LootCrate", "Luck_LootCrate_sheet.png", (
        Variant("black", "torn cardboard tab", "Box Saint's Relic"),
        Variant("red", "shipping label scrap", "Promise Sticker"),
        Variant("yellow", "bubble wrap square", "Treasure Blister"),
        Variant("white", "crate staple", "Parcel Tooth"),
    )),
    Series("Luck", "TreasureChest", "Luck_TreasureChest_sheet.png", (
        Variant("black", "bent key", "Almost-Key of Kings"),
        Variant("red", "zipper pull", "False Latch"),
        Variant("yellow", "candy tin hinge", "Pocket Vault"),
        Variant("white", "suitcase tag", "Pilgrim's Hoardmark"),
    )),
    Series("Luck", "Cheating", "Luck_Cheating_sheet.png", (
        Variant("black", "loaded-looking die", "Fraud Cube"),
        Variant("red", "calculator button", "Forbidden Number"),
        Variant("yellow", "marked card corner", "Dealer's Sin"),
        Variant("white", "game token", "Housebreaker Coin"),
    )),
    Series("Luck", "Stealing", "Luck_Stealing_sheet.png", (
        Variant("black", "broken wallet clip", "Pilfer Crown"),
        Variant("red", "coat button", "Pocket Moon"),
        Variant("yellow", "zipper slider", "Silent Grabber"),
        Variant("white", "price tag string", "Thief's Halo"),
    )),
)


def slug(text: str) -> str:
    return "".join(ch if ch.isalnum() else "_" for ch in text).strip("_").replace("__", "_")


def ensure_dirs() -> None:
    for path in (SHEETS_DIR, SOURCE_DIR, PIXELATED_DIR, CONTACT_DIR):
        path.mkdir(parents=True, exist_ok=True)


def color_distance(a: tuple[int, int, int, int], b: tuple[int, int, int, int]) -> float:
    return math.sqrt(sum((a[i] - b[i]) ** 2 for i in range(3)))


def corner_samples(image: Image.Image, size: int = 28) -> list[tuple[int, int, int, int]]:
    rgba = image.convert("RGBA")
    w, h = rgba.size
    px = rgba.load()
    samples: list[tuple[int, int, int, int]] = []
    for x in range(min(size, w)):
        for y in range(min(size, h)):
            samples.append(px[x, y])
            samples.append(px[w - 1 - x, y])
            samples.append(px[x, h - 1 - y])
            samples.append(px[w - 1 - x, h - 1 - y])
    return samples


def median_color(samples: list[tuple[int, int, int, int]]) -> tuple[int, int, int, int]:
    return tuple(int(statistics.median(sample[i] for sample in samples)) for i in range(4))


def flatten_background(image: Image.Image) -> Image.Image:
    image = image.convert("RGBA").resize((512, 512), Image.Resampling.LANCZOS)
    bg = median_color(corner_samples(image))
    source = image.load()
    mask = Image.new("L", image.size, 0)
    mask_px = mask.load()
    w, h = image.size
    for y in range(h):
        for x in range(w):
            pixel = source[x, y]
            if pixel[3] > 8 and color_distance(pixel, bg) > 16:
                mask_px[x, y] = 255
    mask = mask.filter(ImageFilter.MaxFilter(5)).filter(ImageFilter.MinFilter(3))
    flat = Image.new("RGBA", image.size, bg)
    return Image.composite(image, flat, mask)


def pixelate(image: Image.Image, grid_size: int = 128, colors: int = 56) -> Image.Image:
    small = image.convert("RGBA").resize((grid_size, grid_size), Image.Resampling.BOX)
    quantized = small.convert("RGB").quantize(
        colors=colors,
        method=Image.Quantize.MEDIANCUT,
        dither=Image.Dither.NONE,
    ).convert("RGBA")
    quantized.putalpha(small.getchannel("A"))
    return quantized.resize((512, 512), Image.Resampling.NEAREST)


def output_stem(series: Series, variant: Variant) -> str:
    return f"{series.family}_{series.series}_{variant.rarity}_{slug(variant.fantasy_name)}"


def split_sheet(series: Series) -> list[Path]:
    sheet_path = SHEETS_DIR / series.sheet_name
    if not sheet_path.exists():
        raise FileNotFoundError(f"Missing sheet: {sheet_path}")

    sheet = Image.open(sheet_path).convert("RGBA")
    w, h = sheet.size
    cell_w = w // 2
    cell_h = h // 2
    outputs: list[Path] = []

    for variant in series.variants:
        col, row = RARITY_POSITIONS[variant.rarity]
        crop = sheet.crop((col * cell_w, row * cell_h, (col + 1) * cell_w, (row + 1) * cell_h))
        flat = flatten_background(crop)
        stem = output_stem(series, variant)
        source_path = SOURCE_DIR / f"{stem}_source.png"
        pixel_path = PIXELATED_DIR / f"{stem}_pixelated.png"
        flat.save(source_path)
        pixelate(flat).save(pixel_path)
        outputs.append(pixel_path)

    return outputs


def process_all(allow_missing: bool = False) -> None:
    ensure_dirs()
    missing = [series.sheet_name for series in SERIES if not (SHEETS_DIR / series.sheet_name).exists()]
    if missing and not allow_missing:
        raise SystemExit("Missing raw sheets:\n" + "\n".join(missing))
    for series in SERIES:
        if allow_missing and not (SHEETS_DIR / series.sheet_name).exists():
            continue
        split_sheet(series)


def build_contact_sheet() -> Path:
    ensure_dirs()
    thumb = 128
    label_h = 62
    pad = 10
    cols = 4
    rows = len(SERIES)
    sheet_w = cols * thumb + (cols + 1) * pad
    sheet_h = rows * (thumb + label_h) + (rows + 1) * pad
    sheet = Image.new("RGBA", (sheet_w, sheet_h), (20, 20, 20, 255))
    draw = ImageDraw.Draw(sheet)
    try:
        font = ImageFont.truetype("arial.ttf", 12)
        small_font = ImageFont.truetype("arial.ttf", 10)
    except OSError:
        font = ImageFont.load_default()
        small_font = font

    for row, series in enumerate(SERIES):
        y = pad + row * (thumb + label_h + pad)
        for col, variant in enumerate(series.variants):
            x = pad + col * (thumb + pad)
            path = PIXELATED_DIR / f"{output_stem(series, variant)}_pixelated.png"
            if path.exists():
                image = Image.open(path).convert("RGBA").resize((thumb, thumb), Image.Resampling.NEAREST)
                sheet.alpha_composite(image, (x, y))
            draw.text((x, y + thumb + 4), f"{series.family}/{series.series}", fill=(235, 235, 235, 255), font=font)
            draw.text((x, y + thumb + 20), variant.rarity.title(), fill=(195, 195, 195, 255), font=small_font)
            draw.text((x, y + thumb + 34), variant.fantasy_name[:24], fill=(195, 195, 195, 255), font=small_font)

    out_path = CONTACT_DIR / "ReimaginedItems_pixelated_contact_sheet.png"
    sheet.convert("RGB").save(out_path)
    return out_path


def live_item_ids_by_secondary_stat() -> dict[str, str]:
    item_ids: dict[str, str] = {}
    with ITEMS_CSV.open("r", encoding="utf-8", newline="") as handle:
        for row in csv.DictReader(handle):
            item_id = (row.get("ItemID") or "").strip()
            secondary_stat = (row.get("SecondaryStatType") or "").strip()
            if item_id and secondary_stat and secondary_stat != "Alchemy":
                item_ids[secondary_stat] = item_id
    return item_ids


def promote_live_sprites() -> list[Path]:
    ensure_dirs()
    LIVE_SPRITES_DIR.mkdir(parents=True, exist_ok=True)
    item_ids = live_item_ids_by_secondary_stat()
    promoted: list[Path] = []
    missing: list[str] = []

    for series in SERIES:
        item_id = item_ids.get(series.series)
        if not item_id:
            missing.append(f"{series.series}: no live item row")
            continue
        for variant in series.variants:
            source = PIXELATED_DIR / f"{output_stem(series, variant)}_pixelated.png"
            dest = LIVE_SPRITES_DIR / f"{item_id}_{variant.rarity}.png"
            if not source.exists():
                missing.append(str(source))
                continue
            shutil.copy2(source, dest)
            promoted.append(dest)

    if missing:
        raise SystemExit("Missing live promotion inputs:\n" + "\n".join(missing))

    return promoted


def write_prompt_pack() -> Path:
    ensure_dirs()
    out_path = PASS_ROOT / "imagegen_sheet_prompts.txt"
    lines: list[str] = []
    for series in SERIES:
        variants = ", ".join(
            f"{variant.rarity}: {variant.object_name} named {variant.fantasy_name}"
            for variant in series.variants
        )
        lines.append(f"{series.sheet_name}\n{series.family}/{series.series}: {variants}\n")
    out_path.write_text("\n".join(lines), encoding="utf-8")
    return out_path


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--write-prompts", action="store_true")
    parser.add_argument("--process", action="store_true")
    parser.add_argument("--allow-missing", action="store_true")
    parser.add_argument("--contact-sheet", action="store_true")
    parser.add_argument("--promote-live", action="store_true")
    args = parser.parse_args()

    ensure_dirs()
    if args.write_prompts:
        print(write_prompt_pack())
    if args.process:
        process_all(args.allow_missing)
    if args.contact_sheet:
        print(build_contact_sheet())
    if args.promote_live:
        print(f"promoted={len(promote_live_sprites())}")
    if not (args.write_prompts or args.process or args.contact_sheet or args.promote_live):
        print(f"series={len(SERIES)} variants={sum(len(series.variants) for series in SERIES)}")
        print(PASS_ROOT)


if __name__ == "__main__":
    main()
