#!/usr/bin/env python3
"""
Build prompt and TRELLIS manifest files for the reduced Type A hero batch.

This script does not call ImageGen. It writes the per-asset prompt files and
the RunPod TRELLIS batch script used after the approved PNG inputs exist.
"""

from __future__ import annotations

import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
BATCH_ROOT = ROOT / "Runs" / "Heroes" / "TypeA_Masculine_Batch01"
PROMPT_DIR = BATCH_ROOT / "Inputs" / "prompts"
APPROVED_DIR = BATCH_ROOT / "Inputs" / "approved_seed_images"
RAW_DIR = BATCH_ROOT / "Raw" / "Trellis"
NOTES_DIR = BATCH_ROOT / "Notes"

SEED = 1337
TEXTURE_SIZE = 2048
DECIMATION = {
    "body": 80000,
    "head": 120000,
    "weapon": 200000,
}

TYPE_A_RULES = (
    "Use the same Type A masculine mannequin proportions as the batch: broad shoulders, "
    "athletic torso mass, strong arms and legs, upright centered front view, arms slightly "
    "away from the sides, feet apart, rigging-friendly silhouette. Avoid robes, capes, "
    "skirts, long coats, dangling cloth, or clothing below the knees. Use an opaque pure "
    "green background, no floor, no cast shadow, no text, no extra props."
)

HEAD_RULES = (
    "Head-only production reference for the same forward-facing A-pose character: show only "
    "the head and short neck stump, large in frame, completely straight-on front view, no "
    "three-quarter turn, no side tilt, no looking left or right, centered vertical face axis, "
    "symmetrical eyes, ears, jaw, and hairline, high-definition facial planes, distinct hair "
    "or head shape, no torso, no shoulders, no weapon, opaque pure green background."
)

BODY_RULES = (
    "Body-only production reference: show the complete body from the neck down, no head, no face, "
    "no weapon, clean hands, clear boots or feet, centered full-body front view, opaque pure green background."
)

WEAPON_RULES = (
    "Weapon-only production reference: one isolated handheld weapon, centered diagonal or side view, "
    "clean silhouette, no hand, no body, no character, opaque pure green background."
)

NO_LIKENESS_RULE = (
    "This is an original stylized game character inspired only by the codename; do not make a portrait "
    "or likeness of any real person."
)

HEROES = [
    {
        "id": "Hero_1",
        "name": "Arthur",
        "standard_done": True,
        "head": "battle-worn knight face, squared jaw, short dark hair, stern brow, heroic but not identical to any reference",
        "body": "heroic knight swim-armor silhouette, compact breastplate-inspired swim vest, board shorts, bracers, sandals",
        "weapon": "foam sword shaped like a noble short sword with beach umbrella color accents",
        "beach_body": "heroic beach knight outfit with board shorts, compact swim-armor vest, bracers, knee-free leg silhouette",
        "beach_weapon": "beach umbrella spear or foam sword, toy-like but still heroic",
    },
    {
        "id": "Hero_3",
        "name": "Lu Bu",
        "standard_done": False,
        "head": "fierce warlord head, angular cheekbones, intense eyes, black topknot, short beard, crimson accents",
        "body": "red and black lamellar-inspired battle armor, bare strong arms, armored belt, fitted trousers, heavy boots",
        "weapon": "crescent halberd with red tassel, broad blade, dark shaft, high contrast silhouette",
        "beach_body": "red-black athletic beach armor, lifeguard straps, board shorts, wrapped forearms, strong legs",
        "beach_weapon": "pool-noodle halberd or lifeguard rescue polearm with crescent shape",
    },
    {
        "id": "Hero_4",
        "name": "Mike",
        "standard_done": False,
        "head": "street brawler head, buzz cut, blunt jaw, broken-nose silhouette, confident scowl",
        "body": "sleeveless leather fight vest, taped fists, utility belt, rugged pants, heavy boots",
        "weapon": "compact demolition hammer with wrapped handle and metal head",
        "beach_body": "brawler beachwear with open short-sleeve shirt, fight wraps, cargo swim shorts, sandals",
        "beach_weapon": "beach volleyball hammer or weighted cooler club",
    },
    {
        "id": "Hero_5",
        "name": "George",
        "standard_done": False,
        "head": "clean disciplined duelist head, neat side-part hair, narrow jaw, calm focused eyes",
        "body": "nautical duelist outfit, fitted short fencing jacket, sash above hips, slim trousers, polished boots",
        "weapon": "slim compact rapier with guarded hilt",
        "beach_body": "clean nautical beachwear, fitted sleeveless sailor vest, tailored swim shorts, deck shoes",
        "beach_weapon": "fencing oar or compact beach saber",
    },
    {
        "id": "Hero_7",
        "name": "Robo",
        "standard_done": False,
        "head": "humanoid robot head, rounded metal jaw, glowing visor eyes, asymmetrical antenna, no human face",
        "body": "humanoid robot chassis, compact waterproof armor plates, broad shoulder frame, visible joints, sturdy feet",
        "weapon": "modular electric baton or compact pressure cannon, mechanical silhouette",
        "beach_body": "waterproof humanoid robot plating with swim decals, sealed joints, bright beach safety colors",
        "beach_weapon": "water cannon or pressure sprayer with transparent tank",
    },
    {
        "id": "Hero_8",
        "name": "Billy",
        "standard_done": False,
        "head": "quick-draw outlaw head, sharp grin, stubble, tilted short-brim hat, alert eyes",
        "body": "outlaw gunslinger outfit, cropped waistcoat, neckerchief, fitted pants, holster belt, boots",
        "weapon": "stylized revolver-like arcane pistol, chunky readable barrel",
        "beach_body": "outlaw beachwear with straw hat, open vest, board shorts, holster belt, sandals",
        "beach_weapon": "stylized water pistol with western grip",
    },
    {
        "id": "Hero_9",
        "name": "Rabbit",
        "standard_done": False,
        "head": "mischievous humanoid rabbit-trickster head, long upright ears, bright eyes, angular grin",
        "body": "agile trickster outfit, sleeveless hooded top, short utility jacket, fitted pants, light boots",
        "weapon": "curved boomerang baton with playful rabbit motif",
        "beach_body": "agile beach trickster outfit, sleeveless rash vest, patterned swim shorts, light sandals",
        "beach_weapon": "inflatable ring shield or beach paddle weapon",
    },
    {
        "id": "Hero_11",
        "name": "Shroud",
        "standard_done": False,
        "head": "silent hunter head, cropped dark hood, half mask, narrow eyes, sharp cheek planes",
        "body": "dark tactical hunter outfit, cropped armored jacket, fitted pants, utility straps, quiet boots",
        "weapon": "compact shadow crossbow or short tactical blade, clean angular profile",
        "beach_body": "dark tactical beachwear, black rashguard, fitted board shorts, compact straps, bare forearms",
        "beach_weapon": "compact harpoon or stealthy water blaster",
    },
    {
        "id": "Hero_12",
        "name": "xQc",
        "standard_done": False,
        "head": "high-energy duelist head, spiky pale hair, wiry face, wide expressive eyes, restless grin",
        "body": "neon athletic duelist outfit, sleeveless tech vest, bright bands, fitted pants, fast shoes",
        "weapon": "chaotic neon baton with asymmetric fins and bright accents",
        "beach_body": "high-energy neon beachwear, bright rash vest, patterned swim shorts, wrist bands, fast sandals",
        "beach_weapon": "oversized neon water blaster with chunky tank",
    },
    {
        "id": "Hero_13",
        "name": "Moist",
        "standard_done": False,
        "head": "deadpan menace head, slick wet hair, heavy eyelids, narrow moustache, flat unimpressed expression",
        "body": "understated damp rogue outfit, short utility jacket, rolled sleeves, fitted dark trousers, boots",
        "weapon": "sludge wand or compact dripping mace, readable wet silhouette",
        "beach_body": "understated beachwear, plain muted trunks, short towel vest, minimal sandals, deadpan style",
        "beach_weapon": "novelty squirt weapon with deadpan simple shape",
    },
    {
        "id": "Hero_14",
        "name": "North",
        "standard_done": False,
        "head": "cold wanderer head, pale eyes, wind-cut hair, frost-scar markings, stern weathered face",
        "body": "cold-road ranger outfit, short fur-collar jacket above hips, layered armor, fitted pants, boots",
        "weapon": "ice axe saber with frosted blade and compact handle",
        "beach_body": "cold-water beachwear, short wetsuit, cropped thermal vest, bare lower legs, water shoes",
        "beach_weapon": "surfboard blade or ice-cooler shield weapon",
    },
    {
        "id": "Hero_15",
        "name": "Asmon",
        "standard_done": False,
        "head": "grim scavenger king head, long unkempt hair, rough beard, hollow eyes, crown-like scrap headband",
        "body": "scavenged warrior armor, mismatched metal plates, short ragged scarf above waist, fitted trousers, heavy boots",
        "weapon": "rusty scavenger greatsword or hooked mace, treasure-scrap silhouette",
        "beach_body": "scavenger beachwear, treasure-hunter vest, rugged swim shorts, rope belt, sandals",
        "beach_weapon": "beach shovel or treasure-hunter hook weapon",
    },
]


def slug(hero: dict[str, object]) -> str:
    return str(hero["name"]).replace(" ", "")


def prompt_for(hero: dict[str, object], variant: str, part: str) -> str:
    name = str(hero["name"])
    hero_id = str(hero["id"])
    subject = f"{hero_id} {name}, original stylized game hero."
    if variant == "standard" and part == "head":
        specific = str(hero["head"])
        rules = HEAD_RULES
    elif variant == "standard" and part == "body":
        specific = str(hero["body"])
        rules = BODY_RULES
    elif variant == "standard" and part == "weapon":
        specific = str(hero["weapon"])
        rules = WEAPON_RULES
    elif variant == "beach_goer" and part == "body":
        specific = str(hero["beach_body"])
        rules = BODY_RULES
    elif variant == "beach_goer" and part == "weapon":
        specific = str(hero["beach_weapon"])
        rules = WEAPON_RULES
    else:
        raise ValueError(f"unsupported prompt variant/part: {variant}/{part}")

    return "\n".join(
        [
            subject,
            specific,
            TYPE_A_RULES
            if part == "body"
            else (
                "Keep head scale, neck stump, and straight-ahead orientation compatible with the same "
                "Type A masculine A-pose body reference."
                if part == "head"
                else "Keep scale and silhouette compatible with the Type A masculine hero batch."
            ),
            rules,
            NO_LIKENESS_RULE,
            "Clean production concept art for single-image-to-3D generation, readable shapes, saturated but controlled colors, no background objects.",
        ]
    )


def make_entry(hero: dict[str, object], variant: str, part: str) -> dict[str, object]:
    hero_id = str(hero["id"])
    name_slug = slug(hero)
    variant_name = "Standard" if variant == "standard" else "BeachGoer"
    part_name = {"body": "BodyOnly", "head": "HeadOnly", "weapon": "WeaponOnly"}[part]
    base = f"{hero_id}_{name_slug}_TypeA_{variant_name}_{part_name}_Green"
    target_png = f"Inputs/approved_seed_images/{base}.png"
    trellis_output = f"Raw/Trellis/{base}_S{SEED}_D{DECIMATION[part]}_Trellis2.glb"
    status = "prompt_ready"
    if (BATCH_ROOT / target_png).exists():
        status = "image_ready"
    if (BATCH_ROOT / trellis_output).exists():
        status = "model_ready"

    return {
        "hero_id": hero_id,
        "name": str(hero["name"]),
        "variant": variant,
        "part": part,
        "prompt_file": f"Inputs/prompts/{base}.prompt.txt",
        "target_png": target_png,
        "trellis_output": trellis_output,
        "seed": SEED,
        "texture_size": TEXTURE_SIZE,
        "decimation": DECIMATION[part],
        "prompt": prompt_for(hero, variant, part),
        "status": status,
    }


def build_entries() -> list[dict[str, object]]:
    entries: list[dict[str, object]] = []
    for hero in HEROES:
        if not bool(hero["standard_done"]):
            entries.extend(
                [
                    make_entry(hero, "standard", "body"),
                    make_entry(hero, "standard", "head"),
                    make_entry(hero, "standard", "weapon"),
                ]
            )
        entries.extend(
            [
                make_entry(hero, "beach_goer", "body"),
                make_entry(hero, "beach_goer", "weapon"),
            ]
        )
    return entries


def write_trellis_script(entries: list[dict[str, object]]) -> None:
    lines = [
        "#!/usr/bin/env bash",
        "set -euo pipefail",
        "",
        'BASE="/workspace/T66/ModelGeneration/Runs/Heroes/TypeA_Masculine_Batch01"',
        'IN="$BASE/Inputs/approved_seed_images"',
        'OUT="$BASE/Raw/Trellis"',
        'LOG="$BASE/Notes/trellis_typea_batch01.log"',
        'FORCE_REGEN="${FORCE_REGEN:-0}"',
        "",
        'mkdir -p "$OUT" "$BASE/Notes"',
        ': > "$LOG"',
        "",
        "run_one() {",
        '  local src="$1"',
        '  local module="$2"',
        '  local decimation="$3"',
        '  local dst="$OUT/${module}_S1337_D${decimation}_Trellis2.glb"',
        "",
        '  if [[ ! -f "$src" ]]; then',
        '    echo "[$(date -Is)] SKIP missing input $src" | tee -a "$LOG"',
        "    return 0",
        "  fi",
        "",
        '  if [[ -f "$dst" && "$FORCE_REGEN" != "1" ]]; then',
        '    size="$(stat -c%s "$dst")"',
        '    echo "[$(date -Is)] SKIP existing output $module $size bytes -> $dst" | tee -a "$LOG"',
        "    return 0",
        "  fi",
        "",
        '  echo "[$(date -Is)] START $module seed=1337 texture=2048 decimation=$decimation" | tee -a "$LOG"',
        "  local start",
        '  start="$(date +%s)"',
        "",
        "  curl --fail --show-error --silent \\",
        "    -X POST http://127.0.0.1:8000/generate \\",
        "    -H 'Content-Type: image/png' \\",
        "    -H 'X-Seed: 1337' \\",
        "    -H 'X-Texture-Size: 2048' \\",
        '    -H "X-Decimation: ${decimation}" \\',
        '    --data-binary "@$src" \\',
        '    -o "$dst"',
        "",
        "  local end size",
        '  end="$(date +%s)"',
        '  size="$(stat -c%s "$dst")"',
        '  echo "[$(date -Is)] DONE $module $size bytes duration=$((end-start))s -> $dst" | tee -a "$LOG"',
        "}",
        "",
    ]

    for entry in entries:
        module = Path(str(entry["trellis_output"])).name
        module = module.removesuffix(f"_S{SEED}_D{entry['decimation']}_Trellis2.glb")
        src_name = Path(str(entry["target_png"])).name
        lines.append(f'run_one "$IN/{src_name}" "{module}" "{entry["decimation"]}"')

    with (NOTES_DIR / "run_trellis_typea_batch01.sh").open("w", encoding="utf-8", newline="\n") as handle:
        handle.write("\n".join(lines) + "\n")


def main() -> None:
    PROMPT_DIR.mkdir(parents=True, exist_ok=True)
    APPROVED_DIR.mkdir(parents=True, exist_ok=True)
    RAW_DIR.mkdir(parents=True, exist_ok=True)
    NOTES_DIR.mkdir(parents=True, exist_ok=True)

    entries = build_entries()
    for entry in entries:
        prompt_path = BATCH_ROOT / str(entry["prompt_file"])
        prompt_path.write_text(str(entry["prompt"]) + "\n", encoding="utf-8")

    manifest = {
        "batch": "TypeA_Masculine_Batch01",
        "active_hero_count": len(HEROES),
        "image_prompt_count": len(entries),
        "seed": SEED,
        "texture_size": TEXTURE_SIZE,
        "decimation": DECIMATION,
        "notes": [
            "Arthur standard head/body/weapon outputs already exist under Runs/Heroes/Hero_1_Arthur.",
            "Beach goer heads reuse accepted standard heads.",
            "Do not generate body+head or weapon-on-body production inputs.",
        ],
        "entries": entries,
    }
    (BATCH_ROOT / "batch_manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    write_trellis_script(entries)

    print(f"Wrote {len(entries)} prompt files")
    print(BATCH_ROOT / "batch_manifest.json")
    print(NOTES_DIR / "run_trellis_typea_batch01.sh")


if __name__ == "__main__":
    main()
