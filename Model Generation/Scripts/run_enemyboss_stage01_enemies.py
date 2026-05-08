#!/usr/bin/env python3
"""Stage 1 driver for the regular-enemy EnemyBossBatch01 Trellis pass.

This script owns only the 25 regular enemies from Content/Data/Enemies.csv.
It intentionally does not run Quad Remesher, Quad Retro processing, Unreal
import, or staged-build work.
"""

from __future__ import annotations

import argparse
import csv
import hashlib
import io
import json
import posixpath
import shlex
import subprocess
import time
from datetime import datetime, timezone
from pathlib import Path
from urllib.parse import quote, urlencode
from urllib.request import Request, urlopen

from PIL import Image, ImageDraw


REPO_ROOT = Path(__file__).resolve().parents[2]
MODEL_ROOT = REPO_ROOT / "Model Generation"
RUN_ROOT = MODEL_ROOT / "Runs" / "EnemyBosses" / "EnemyBossBatch01"
ENEMIES_CSV = REPO_ROOT / "Content" / "Data" / "Enemies.csv"

PROMPT_DIR = RUN_ROOT / "Inputs" / "Prompts" / "Enemies"
SOURCE_DIR = RUN_ROOT / "Inputs" / "SourceImages" / "Enemies"
TRELLIS_INPUT_DIR = RUN_ROOT / "Inputs" / "TrellisImages" / "Enemies"
RAW_DIR = RUN_ROOT / "Raw" / "Trellis" / "Enemies"
QA_DIR = RUN_ROOT / "QA" / "TrellisFront" / "Enemies"
REPORT_DIR = RUN_ROOT / "Reports"
NOTES_DIR = RUN_ROOT / "Notes"

MANIFEST_PATH = REPORT_DIR / "Stage01_Enemies_TrellisManifest.json"
STATUS_PATH = NOTES_DIR / "ENEMIES_STAGE01_STATUS.md"
SHARED_STATUS_PATH = NOTES_DIR / "STAGE01_STATUS.md"
GEN_LOG_PATH = NOTES_DIR / "enemies_source_image_generation.log"
TRELLIS_LOG_PATH = NOTES_DIR / "enemies_trellis_generation.log"

POD_IP = "69.30.85.78"
POD_PORT = "22064"
SSH_KEY = Path.home() / ".ssh" / "id_ed25519"
POD_ROOT = "/workspace/T66/ModelGeneration/Runs/EnemyBosses/EnemyBossBatch01"

BLENDER_EXE = Path(r"C:\Program Files\Blender Foundation\Blender 5.1\blender.exe")
BLENDER_QA_SCRIPT = MODEL_ROOT / "Scripts" / "blender_glb_qa.py"

DEFAULT_SEED = 1337
DEFAULT_TEXTURE_SIZE = 2048
DEFAULT_DECIMATION = 80000

NEGATIVE_PROMPT = (
    "text, letters, watermark, UI, logo, caption, scene background, environment, "
    "floor, ground plane, cast shadow, contact shadow, drop shadow, gradient "
    "background, poster border, cropped body, headshot, portrait, half body, "
    "multiple characters, duplicate creature, extra random limbs, mutated "
    "anatomy, blurry output, photorealism, pixel art, noisy texture, cinematic "
    "lighting, dramatic shadows, rim light, glow effects, particle effects, "
    "smoke, transparent background, black background, white background"
)

ROW_ORDER = [
    "Dungeon_Slime",
    "Dungeon_Skeleton",
    "Dungeon_WebSpider",
    "Dungeon_RabidRat",
    "Dungeon_Bat",
    "Forest_MushroomBrute",
    "Forest_TreantSapling",
    "Forest_ThornImp",
    "Forest_Boar",
    "Forest_Wasp",
    "Ocean_CrabGuard",
    "Ocean_DrownedSailor",
    "Ocean_Jellyfish",
    "Ocean_SharkPup",
    "Ocean_GhostRay",
    "Martian_DroneGrunt",
    "Martian_CrystalCrawler",
    "Martian_PlasmaSpitter",
    "Martian_RocketLeaper",
    "Martian_SaucerDrone",
    "Hell_Imp",
    "Hell_BoneKnight",
    "Hell_FireSkull",
    "Hellhound",
    "Hell_Gargoyle",
]

QUADRUPED_OR_CREATURE = {
    "Dungeon_Slime",
    "Dungeon_WebSpider",
    "Dungeon_RabidRat",
    "Dungeon_Bat",
    "Forest_Boar",
    "Forest_Wasp",
    "Ocean_CrabGuard",
    "Ocean_Jellyfish",
    "Ocean_SharkPup",
    "Ocean_GhostRay",
    "Martian_CrystalCrawler",
    "Martian_PlasmaSpitter",
    "Martian_SaucerDrone",
    "Hell_FireSkull",
    "Hellhound",
    "Hell_Gargoyle",
}

SPECIFIC_DIRECTION = {
    "Dungeon_Slime": (
        "Exact anatomy constraint: one freestanding smooth green gelatinous slime "
        "blob only, rounded mound body, soft translucent 3D game miniature volume "
        "shown in oblique three-quarter view with visible side curvature, two tiny "
        "eyes, a few small embedded bone chips; absolutely no arms, no legs, no "
        "feet, no claws, no teeth, no insect anatomy, no crab anatomy, no spider "
        "anatomy, no ground shadow, no background object, no sticker border, no "
        "flat icon, no square card, no perfectly front-facing symbol."
    ),
    "Dungeon_Skeleton": (
        "Exact anatomy constraint: small readable skeleton warrior, two arms, two legs, "
        "simple rib cage, cracked skull, one rusty dagger held still."
    ),
    "Dungeon_WebSpider": (
        "Exact anatomy constraint: spider body, eight clear legs, oversized web abdomen, "
        "no humanoid torso."
    ),
    "Dungeon_RabidRat": (
        "Exact anatomy constraint: rat body with four legs, long tail, chipped teeth, "
        "still reconstruction-safe pose rather than motion blur."
    ),
    "Dungeon_Bat": (
        "Exact anatomy constraint: bat with two wide angular wings, small body, visible "
        "ears and feet, no bird feathers."
    ),
    "Forest_MushroomBrute": (
        "Exact anatomy constraint: stocky walking mushroom, big cap, thick stump legs, "
        "short arms, poison spots."
    ),
    "Forest_TreantSapling": (
        "Exact anatomy constraint: small angry tree creature, branch arms, root feet, "
        "leaf clumps, no human clothing, no hollow doorway, no portal, no white "
        "backing behind the branches."
    ),
    "Forest_ThornImp": (
        "Exact anatomy constraint: small goblin-like imp with leafy hood and thorn darts, "
        "two arms and two legs."
    ),
    "Forest_Boar": (
        "Exact anatomy constraint: low boar body, four legs, bark armor plates, sharp "
        "tusks, no humanoid stance."
    ),
    "Forest_Wasp": (
        "Exact anatomy constraint: giant wasp, striped abdomen, six legs, transparent "
        "leaf-like wings, no humanoid features."
    ),
    "Ocean_CrabGuard": (
        "Exact anatomy constraint: armored crab with clear shell, many crab legs, large "
        "coral shield claw, no human face."
    ),
    "Ocean_DrownedSailor": (
        "Exact anatomy constraint: low-detail chunky undead sailor game miniature, "
        "visible skull face, tattered coat, short seaweed beard only, broken cutlass "
        "held still, two arms and two legs; clean silhouette, no long hair curtain, "
        "no dense dangling strands, no mossy full-body noise, no transparent seaweed "
        "sheets, no gray backing."
    ),
    "Ocean_Jellyfish": (
        "Exact anatomy constraint: jellyfish bell with hanging tendrils, no face, no "
        "bones, no humanoid limbs, no white oval backing, no aquarium glass, no "
        "water background."
    ),
    "Ocean_SharkPup": (
        "Exact anatomy constraint: small shark body with readable dorsal fin, short "
        "supporting fins, no legs, no humanoid torso."
    ),
    "Ocean_GhostRay": (
        "Exact anatomy constraint: manta ray spirit silhouette, wide wings, long tail, "
        "no humanoid body."
    ),
    "Martian_DroneGrunt": (
        "Exact anatomy constraint: small robot drone grunt with chunky metal body and "
        "blunt metal arms, no human skin."
    ),
    "Martian_CrystalCrawler": (
        "Exact anatomy constraint: low alien crawler with many legs and faceted purple "
        "crystal shell, no upright humanoid body."
    ),
    "Martian_PlasmaSpitter": (
        "Exact anatomy constraint: alien lizard with glowing plasma throat and red dust "
        "hide, four legs, no humanoid armor."
    ),
    "Martian_RocketLeaper": (
        "Exact anatomy constraint: thin alien soldier, jump rockets, impact helmet, two "
        "arms and two legs, neutral stance, compact rockets attached to boots or back, "
        "no detached floating gear, no speech bubbles, no white side object, no loose "
        "tool silhouettes."
    ),
    "Martian_SaucerDrone": (
        "Exact anatomy constraint: small hovering saucer drone, thick chunky round "
        "saucer body with raised dome and underside pod, short antenna, blue electric "
        "rim lights, no organic limbs, no wafer-thin disc, no paper-thin fins."
    ),
    "Hell_Imp": (
        "Exact anatomy constraint: small horned demon imp, clawed hands, ember skin, two "
        "arms and two legs."
    ),
    "Hell_BoneKnight": (
        "Exact anatomy constraint: skeletal knight in blackened armor, cursed blade held "
        "still, two arms and two legs, freestanding only, no doorway, no poster, no "
        "white slab behind the body."
    ),
    "Hell_FireSkull": (
        "Exact anatomy constraint: floating flaming skull only, no body, no arms, no legs, "
        "no hands, no claws, no crawler body, no demon torso, simple flame mass "
        "attached directly to the skull as hair and side wisps, volumetric 3D "
        "game miniature skull shown in oblique three-quarter view with visible "
        "side planes and cheek depth, not a flat poster, no square flame frame, "
        "no rectangular border, no perfectly front-facing icon."
    ),
    "Hellhound": (
        "Exact anatomy constraint: demonic hound, four legs, lava cracks, canine head, no "
        "humanoid torso."
    ),
    "Hell_Gargoyle": (
        "Exact anatomy constraint: winged stone demon gargoyle, cracked horns, two wings, "
        "clawed hands and feet, crouched but readable, freestanding only, no stone "
        "base, no pedestal, no doorway, no backdrop."
    ),
}

AGGRESSIVE_ALPHA_ROWS = {
    "Dungeon_Slime",
    "Forest_TreantSapling",
    "Ocean_Jellyfish",
    "Ocean_DrownedSailor",
    "Martian_RocketLeaper",
    "Hell_BoneKnight",
    "Hell_FireSkull",
    "Hell_Gargoyle",
}


def rel(path: Path) -> str:
    return path.relative_to(RUN_ROOT).as_posix()


def now_iso() -> str:
    return datetime.now(timezone.utc).replace(microsecond=0).isoformat()


def ensure_dirs() -> None:
    for path in (
        PROMPT_DIR,
        SOURCE_DIR,
        TRELLIS_INPUT_DIR,
        RAW_DIR,
        QA_DIR,
        REPORT_DIR,
        NOTES_DIR,
    ):
        path.mkdir(parents=True, exist_ok=True)


def load_rows() -> list[dict[str, str]]:
    with ENEMIES_CSV.open("r", encoding="utf-8", newline="") as handle:
        rows = list(csv.DictReader(handle))
    by_id = {row["EnemyID"]: row for row in rows}
    missing = [row_id for row_id in ROW_ORDER if row_id not in by_id]
    if missing:
        raise RuntimeError(f"Enemies.csv is missing expected rows: {missing}")
    return [by_id[row_id] for row_id in ROW_ORDER]


def prompt_for(row: dict[str, str]) -> str:
    row_id = row["EnemyID"]
    display_name = row["DisplayName"]
    silhouette = (
        "Whole creature silhouette: full subject visible, all key limbs, wings, "
        "fins, legs, tail, and appendages visible, centered front-facing or clean "
        "orthographic three-quarter front view if needed for creature readability."
        if row_id in QUADRUPED_OR_CREATURE
        else "Humanoid or biped monster silhouette: full body visible, neutral "
        "reconstruction-safe A-pose, arms separated from torso, centered front "
        "view, both hands and feet visible."
    )
    return "\n".join(
        [
            "Use case: stylized-concept",
            "Asset type: Trellis source image for T66 regular enemy reconstruction",
            (
                "Primary request: Create a clean full-body or whole-creature game "
                f"enemy concept for {display_name}, EnemyID {row_id}."
            ),
            (
                f"Theme: {row['ThemeID']} difficulty {row['DifficultyID']}; "
                f"family {row['FamilyID']}; role {row['RoleID']}."
            ),
            f"Visual concept: {row['VisualConcept']}.",
            f"CSV direction: {row['ImagePrompt']}.",
            SPECIFIC_DIRECTION[row_id],
            silhouette,
            (
                "Style: clean painted video game concept art, flat cel-shaded "
                "color blocks, simple readable materials, minimal surface noise, "
                "strong silhouette, no tiny facial detail, no texture clutter."
            ),
            (
                "Camera and framing: centered square image, orthographic-feeling "
                "camera, subject fills about 80 percent of frame, full subject "
                "visible with generous padding."
            ),
            (
                "Background and lighting: pure flat opaque magenta #FF00FF "
                "background, no alpha, no shadow, no floor plane, even ambient "
                "lighting, no highlights or reflections."
            ),
            "Output must contain no text, no UI, no logo, no watermark.",
            f"Negative: {NEGATIVE_PROMPT}.",
        ]
    )


def stable_seed(row_id: str) -> int:
    digest = hashlib.sha256(row_id.encode("utf-8")).hexdigest()
    return 1000 + int(digest[:8], 16) % 900000


def fetch_image(prompt: str, seed: int) -> Image.Image:
    params = urlencode(
        {
            "width": 1024,
            "height": 1024,
            "nologo": "true",
            "private": "true",
            "model": "flux",
            "seed": str(seed),
            "negative_prompt": NEGATIVE_PROMPT,
        }
    )
    url = "https://image.pollinations.ai/prompt/" + quote(prompt) + "?" + params
    request = Request(url, headers={"User-Agent": "Mozilla/5.0"})
    with urlopen(request, timeout=240) as response:
        data = response.read()
    return Image.open(io.BytesIO(data)).convert("RGB")


def flood_magenta_background(image: Image.Image) -> Image.Image:
    image = image.resize((1024, 1024), Image.Resampling.LANCZOS).convert("RGB")
    pixels = image.load()
    width, height = image.size
    corners = [
        pixels[0, 0],
        pixels[width - 1, 0],
        pixels[0, height - 1],
        pixels[width - 1, height - 1],
    ]
    bg = tuple(sum(color[i] for color in corners) // len(corners) for i in range(3))

    def is_background(color: tuple[int, int, int]) -> bool:
        dist = sum((int(color[i]) - int(bg[i])) ** 2 for i in range(3)) ** 0.5
        return dist < 48

    from collections import deque

    seen: set[tuple[int, int]] = set()
    queue: deque[tuple[int, int]] = deque()
    for x in range(width):
        queue.append((x, 0))
        queue.append((x, height - 1))
    for y in range(height):
        queue.append((0, y))
        queue.append((width - 1, y))

    while queue:
        x, y = queue.popleft()
        if (x, y) in seen or not (0 <= x < width and 0 <= y < height):
            continue
        if not is_background(pixels[x, y]):
            continue
        seen.add((x, y))
        pixels[x, y] = (255, 0, 255)
        queue.extend(((x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1)))

    return image


def add_source_label(image: Image.Image, row_id: str) -> Image.Image:
    # The Trellis source itself must remain text-free. Labels are added only to
    # contact sheets, never saved over the source image.
    labeled = image.copy()
    draw = ImageDraw.Draw(labeled)
    draw.rectangle((0, 980, 1024, 1024), fill=(255, 0, 255))
    draw.text((12, 992), row_id, fill=(0, 0, 0))
    return labeled


def write_manual_slime() -> None:
    target = SOURCE_DIR / "Dungeon_Slime.png"
    target.parent.mkdir(parents=True, exist_ok=True)
    image = Image.new("RGB", (1024, 1024), (255, 0, 255))
    draw = ImageDraw.Draw(image)

    # A deliberately simple Trellis-safe source: one rounded blob, embedded
    # dungeon bone chips, clear eyes, no limbs or ground shadow.
    draw.ellipse((220, 250, 810, 780), fill=(112, 190, 58), outline=(34, 86, 38), width=10)
    draw.ellipse((265, 310, 760, 735), fill=(138, 216, 78))
    draw.ellipse((330, 360, 460, 480), fill=(43, 38, 55), outline=(20, 16, 24), width=7)
    draw.ellipse((580, 360, 710, 480), fill=(43, 38, 55), outline=(20, 16, 24), width=7)
    draw.ellipse((375, 386, 415, 426), fill=(204, 230, 242))
    draw.ellipse((625, 386, 665, 426), fill=(204, 230, 242))
    draw.arc((405, 500, 625, 650), start=10, end=170, fill=(42, 70, 30), width=12)

    highlights = [
        (330, 300, 430, 340),
        (500, 285, 630, 322),
        (655, 515, 720, 545),
    ]
    for box in highlights:
        draw.ellipse(box, fill=(189, 240, 112))

    bone_color = (220, 218, 190)
    bone_shadow = (125, 116, 92)
    for box in [(255, 540, 330, 575), (700, 610, 780, 645), (510, 680, 580, 710)]:
        draw.ellipse((box[0] - 4, box[1] + 4, box[2] - 4, box[3] + 4), fill=bone_shadow)
        draw.ellipse(box, fill=bone_color)
    for box in [(680, 310, 715, 340), (300, 640, 332, 672), (470, 345, 505, 375)]:
        draw.ellipse(box, fill=(76, 130, 42), outline=(38, 80, 34), width=4)

    image.save(target)
    prompt_path = PROMPT_DIR / "Dungeon_Slime.txt"
    if not prompt_path.exists():
        row = next(row for row in load_rows() if row["EnemyID"] == "Dungeon_Slime")
        prompt_path.write_text(prompt_for(row), encoding="utf-8")
    print(f"MANUAL SLIME DONE -> {target}")


def prepare_trellis_input(source: Path, target: Path, aggressive: bool = False) -> None:
    image = Image.open(source).convert("RGBA")
    width, height = image.size
    pixels = image.load()

    def is_magentaish(color: tuple[int, int, int, int]) -> bool:
        r, g, b, _a = color
        return r > 175 and b > 150 and g < 135

    def is_edge_background_candidate(color: tuple[int, int, int, int]) -> bool:
        r, g, b, a = color
        if a < 8:
            return True
        if is_magentaish(color):
            return True
        maxc = max(r, g, b)
        minc = min(r, g, b)
        saturation = maxc - minc
        if saturation < 36 and maxc > 118:
            return True
        if r > 210 and g > 210 and b > 210:
            return True
        return False

    def is_aggressive_background_candidate(color: tuple[int, int, int, int]) -> bool:
        r, g, b, a = color
        if a < 8 or is_magentaish(color):
            return True
        maxc = max(r, g, b)
        minc = min(r, g, b)
        saturation = maxc - minc
        if saturation < 44 and maxc > 132:
            return True
        if r > 222 and g > 222 and b > 222:
            return True
        if r > 180 and b > 150 and g < 170:
            return True
        if r > 86 and b > 70 and g < 105 and abs(r - b) < 95:
            return True
        return False

    from collections import deque

    remove: set[tuple[int, int]] = set()
    queue: deque[tuple[int, int]] = deque()
    for x in range(width):
        queue.append((x, 0))
        queue.append((x, height - 1))
    for y in range(height):
        queue.append((0, y))
        queue.append((width - 1, y))

    while queue:
        x, y = queue.popleft()
        if (x, y) in remove or not (0 <= x < width and 0 <= y < height):
            continue
        if not is_edge_background_candidate(pixels[x, y]):
            continue
        remove.add((x, y))
        queue.extend(((x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1)))

    for y in range(height):
        for x in range(width):
            r, g, b, a = pixels[x, y]
            color = (r, g, b, a)
            if (
                (x, y) in remove
                or is_magentaish(color)
                or (aggressive and is_aggressive_background_candidate(color))
            ):
                pixels[x, y] = (0, 0, 0, 0)

    target.parent.mkdir(parents=True, exist_ok=True)
    image.save(target)


def prepare_trellis_inputs(rows: list[dict[str, str]], only: set[str], force: bool) -> None:
    for row in rows:
        row_id = row["EnemyID"]
        if only and row_id not in only:
            continue
        source = SOURCE_DIR / f"{row_id}.png"
        target = TRELLIS_INPUT_DIR / f"{row_id}.png"
        if not source.exists():
            print(f"TRELLIS INPUT SKIP {row_id} missing source")
            continue
        if target.exists() and not force:
            print(f"TRELLIS INPUT SKIP {row_id} existing")
            continue
        prepare_trellis_input(source, target, aggressive=row_id in AGGRESSIVE_ALPHA_ROWS)
        print(f"TRELLIS INPUT DONE {row_id} -> {target}")


def generate_source_images(
    rows: list[dict[str, str]],
    only: set[str],
    force: bool,
    seed_offset: int,
) -> None:
    with GEN_LOG_PATH.open("a", encoding="utf-8") as log:
        for row in rows:
            row_id = row["EnemyID"]
            if only and row_id not in only:
                continue
            target = SOURCE_DIR / f"{row_id}.png"
            if target.exists() and not force:
                print(f"IMAGE SKIP {row_id} existing")
                continue
            prompt = prompt_for(row)
            seed = stable_seed(row_id) + seed_offset
            prompt_path = PROMPT_DIR / f"{row_id}.txt"
            prompt_path.write_text(prompt, encoding="utf-8")
            print(f"IMAGE START {row_id} seed={seed}")
            log.write(f"[{now_iso()}] START {row_id} seed={seed}\n")
            log.flush()
            last_error: Exception | None = None
            for attempt in range(1, 4):
                try:
                    image = fetch_image(prompt, seed + attempt - 1)
                    image = flood_magenta_background(image)
                    target.parent.mkdir(parents=True, exist_ok=True)
                    image.save(target)
                    print(f"IMAGE DONE {row_id} -> {target}")
                    log.write(f"[{now_iso()}] DONE {row_id} -> {target}\n")
                    log.flush()
                    break
                except Exception as exc:  # pragma: no cover - network dependent
                    last_error = exc
                    print(f"IMAGE RETRY {row_id} attempt={attempt} error={exc}")
                    log.write(
                        f"[{now_iso()}] RETRY {row_id} attempt={attempt} error={exc}\n"
                    )
                    log.flush()
                    time.sleep(4)
            else:
                raise RuntimeError(f"failed to generate source image for {row_id}: {last_error}")


def run_cmd(args: list[str], timeout: int | None = None) -> subprocess.CompletedProcess[str]:
    result = subprocess.run(
        args,
        cwd=REPO_ROOT,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        timeout=timeout,
    )
    if result.returncode != 0:
        raise RuntimeError(
            "command failed with exit "
            f"{result.returncode}: {' '.join(args)}\n{result.stdout}"
        )
    return result


def ssh_args(remote_command: str) -> list[str]:
    return [
        "ssh.exe",
        "-p",
        POD_PORT,
        "-i",
        str(SSH_KEY),
        "-o",
        "BatchMode=yes",
        "-o",
        "ConnectTimeout=15",
        f"root@{POD_IP}",
        remote_command,
    ]


def scp_to_pod(local_path: Path, remote_path: str) -> None:
    run_cmd(
        [
            "scp.exe",
            "-P",
            POD_PORT,
            "-i",
            str(SSH_KEY),
            "-o",
            "BatchMode=yes",
            str(local_path),
            f"root@{POD_IP}:{remote_path}",
        ],
        timeout=180,
    )


def scp_from_pod(remote_path: str, local_path: Path) -> None:
    local_path.parent.mkdir(parents=True, exist_ok=True)
    run_cmd(
        [
            "scp.exe",
            "-P",
            POD_PORT,
            "-i",
            str(SSH_KEY),
            "-o",
            "BatchMode=yes",
            f"root@{POD_IP}:{remote_path}",
            str(local_path),
        ],
        timeout=180,
    )


def health_check() -> str:
    result = run_cmd(ssh_args("curl -sS --max-time 10 http://127.0.0.1:8000/health"), timeout=30)
    return result.stdout.strip()


def run_trellis(rows: list[dict[str, str]], only: set[str], force: bool) -> None:
    print(f"TRELLIS HEALTH {health_check()}")
    with TRELLIS_LOG_PATH.open("a", encoding="utf-8") as log:
        for row in rows:
            row_id = row["EnemyID"]
            if only and row_id not in only:
                continue
            source = TRELLIS_INPUT_DIR / f"{row_id}.png"
            if not source.exists():
                source = SOURCE_DIR / f"{row_id}.png"
            local_glb = RAW_DIR / row_id / f"{row_id}_Trellis.glb"
            if not source.exists():
                print(f"TRELLIS SKIP {row_id} missing source image")
                continue
            if local_glb.exists() and local_glb.stat().st_size > 0 and not force:
                print(f"TRELLIS SKIP {row_id} existing")
                continue

            remote_source = f"{POD_ROOT}/Inputs/TrellisImages/Enemies/{row_id}.png"
            remote_glb = f"{POD_ROOT}/Raw/Trellis/Enemies/{row_id}/{row_id}_Trellis.glb"
            run_cmd(
                ssh_args(
                    "mkdir -p "
                    f"{shlex.quote(posixpath.dirname(remote_source))} "
                    f"{shlex.quote(posixpath.dirname(remote_glb))}"
                ),
                timeout=30,
            )
            scp_to_pod(source, remote_source)

            print(
                f"TRELLIS START {row_id} seed={DEFAULT_SEED} "
                f"texture={DEFAULT_TEXTURE_SIZE} decimation={DEFAULT_DECIMATION}"
            )
            log.write(
                f"[{now_iso()}] START {row_id} seed={DEFAULT_SEED} "
                f"texture={DEFAULT_TEXTURE_SIZE} decimation={DEFAULT_DECIMATION}\n"
            )
            log.flush()
            start = time.monotonic()
            remote_command = " ".join(
                [
                    "set -e;",
                    "curl --fail --show-error --silent --max-time 3600",
                    "-X POST http://127.0.0.1:8000/generate",
                    "-H 'Content-Type: image/png'",
                    f"-H 'X-Seed: {DEFAULT_SEED}'",
                    f"-H 'X-Texture-Size: {DEFAULT_TEXTURE_SIZE}'",
                    f"-H 'X-Decimation: {DEFAULT_DECIMATION}'",
                    f"--data-binary @{shlex.quote(remote_source)}",
                    f"-o {shlex.quote(remote_glb)};",
                    f"stat -c%s {shlex.quote(remote_glb)}",
                ]
            )
            result = run_cmd(ssh_args(remote_command), timeout=3900)
            duration = time.monotonic() - start
            size_text = result.stdout.strip().splitlines()[-1] if result.stdout.strip() else "0"
            scp_from_pod(remote_glb, local_glb)
            print(f"TRELLIS DONE {row_id} size={local_glb.stat().st_size} duration={duration:.1f}s")
            log.write(
                f"[{now_iso()}] DONE {row_id} pod_size={size_text} "
                f"local_size={local_glb.stat().st_size} duration={duration:.1f}s\n"
            )
            log.flush()


def render_qa(rows: list[dict[str, str]], only: set[str], force: bool) -> None:
    if not BLENDER_EXE.exists():
        raise RuntimeError(f"Blender executable not found: {BLENDER_EXE}")
    for row in rows:
        row_id = row["EnemyID"]
        if only and row_id not in only:
            continue
        glb = RAW_DIR / row_id / f"{row_id}_Trellis.glb"
        render = QA_DIR / f"{row_id}_front.png"
        metadata = QA_DIR / f"{row_id}_front_metadata.json"
        if not glb.exists():
            print(f"QA SKIP {row_id} missing GLB")
            continue
        if render.exists() and metadata.exists() and not force:
            print(f"QA SKIP {row_id} existing")
            continue
        print(f"QA START {row_id}")
        run_cmd(
            [
                str(BLENDER_EXE),
                "--background",
                "--python",
                str(BLENDER_QA_SCRIPT),
                "--",
                "--input",
                str(glb),
                "--render",
                str(render),
                "--metadata",
                str(metadata),
                "--yaw",
                "0",
                "--pitch",
                "5",
                "--resolution",
                "1024",
            ],
            timeout=600,
        )
        print(f"QA DONE {row_id} -> {render}")


def make_contact_sheet(image_paths: list[Path], target: Path, label_from_path) -> None:
    existing = [path for path in image_paths if path.exists()]
    if not existing:
        return
    thumb_size = 256
    label_height = 28
    columns = 5
    rows = (len(existing) + columns - 1) // columns
    sheet = Image.new("RGB", (columns * thumb_size, rows * (thumb_size + label_height)), (28, 28, 32))
    draw = ImageDraw.Draw(sheet)
    for index, path in enumerate(existing):
        image = Image.open(path).convert("RGB")
        image.thumbnail((thumb_size, thumb_size), Image.Resampling.LANCZOS)
        x = (index % columns) * thumb_size
        y = (index // columns) * (thumb_size + label_height)
        sheet.paste(image, (x + (thumb_size - image.width) // 2, y))
        draw.text((x + 8, y + thumb_size + 7), label_from_path(path), fill=(235, 235, 235))
    target.parent.mkdir(parents=True, exist_ok=True)
    sheet.save(target)


def build_manifest(rows: list[dict[str, str]]) -> dict[str, object]:
    manifest_rows = []
    counts: dict[str, int] = {}
    for row in rows:
        row_id = row["EnemyID"]
        source = SOURCE_DIR / f"{row_id}.png"
        trellis_input = TRELLIS_INPUT_DIR / f"{row_id}.png"
        glb = RAW_DIR / row_id / f"{row_id}_Trellis.glb"
        qa = QA_DIR / f"{row_id}_front.png"
        prompt_path = PROMPT_DIR / f"{row_id}.txt"

        if qa.exists() and glb.exists() and source.exists():
            status = "PendingReview"
        elif glb.exists() and source.exists():
            status = "RawTrellisReady"
        elif source.exists():
            status = "SourceImageReady"
        else:
            status = "PendingSourceImage"
        counts[status] = counts.get(status, 0) + 1

        manifest_rows.append(
            {
                "row_id": row_id,
                "source_table": "Enemies.csv",
                "display_name": row["DisplayName"],
                "difficulty_id": row["DifficultyID"],
                "theme_id": row["ThemeID"],
                "family_or_role": row["FamilyID"],
                "role_id": row["RoleID"],
                "visual_concept": row["VisualConcept"],
                "image_prompt_used": prompt_path.read_text(encoding="utf-8")
                if prompt_path.exists()
                else prompt_for(row),
                "source_image": rel(source),
                "trellis_input_image": rel(trellis_input),
                "raw_trellis_glb": rel(glb),
                "qa_front_render": rel(qa),
                "status": status,
                "source_image_exists": source.exists(),
                "trellis_input_image_exists": trellis_input.exists(),
                "raw_trellis_glb_exists": glb.exists(),
                "qa_front_render_exists": qa.exists(),
                "raw_trellis_glb_size_bytes": glb.stat().st_size if glb.exists() else 0,
                "settings": {
                    "trellis_seed": DEFAULT_SEED,
                    "trellis_texture_size": DEFAULT_TEXTURE_SIZE,
                    "trellis_decimation": DEFAULT_DECIMATION,
                    "source_background": "#FF00FF",
                },
                "notes": "",
            }
        )

    return {
        "batch": "EnemyBossBatch01",
        "stage": "Stage01_Enemies_SourceImages_TrellisRaw_QAFront",
        "source_table": "Content/Data/Enemies.csv",
        "expected_count": 25,
        "row_count": len(rows),
        "updated_utc": now_iso(),
        "rules": [
            "Regular enemies only; bosses are out of scope for this manifest.",
            "No Quad Remesher.",
            "No Quad Retro or Blender retro pass.",
            "No Unreal import.",
            "No stage/cook.",
            "Every artifact is keyed by exact EnemyID.",
            "Trellis requests are serial.",
        ],
        "counts_by_status": counts,
        "rows": manifest_rows,
    }


def write_status(manifest: dict[str, object]) -> None:
    rows = manifest["rows"]
    counts = manifest["counts_by_status"]
    completed = int(counts.get("PendingReview", 0))
    source_ready = int(counts.get("SourceImageReady", 0))
    raw_ready = int(counts.get("RawTrellisReady", 0))
    pending = int(counts.get("PendingSourceImage", 0))

    lines = [
        "# EnemyBossBatch01 Regular Enemies Stage 1 Status",
        "",
        f"Updated UTC: {manifest['updated_utc']}",
        "",
        "Scope: 25 regular enemies from `Content/Data/Enemies.csv`.",
        "",
        "Stage rules: source images, raw Trellis GLBs, and front QA renders only. No Quad Remesher, no Quad Retro pass, no Unreal import, no staged build.",
        "",
        "## Counts",
        "",
        f"- PendingReview: {completed}",
        f"- RawTrellisReady: {raw_ready}",
        f"- SourceImageReady: {source_ready}",
        f"- PendingSourceImage: {pending}",
        "",
        "## Artifacts",
        "",
        "| EnemyID | Status | Source image | Raw Trellis GLB | QA front |",
        "|---|---:|---|---|---|",
    ]
    for row in rows:
        lines.append(
            "| {row_id} | {status} | [{source}]({source}) | [{glb}]({glb}) | [{qa}]({qa}) |".format(
                row_id=row["row_id"],
                status=row["status"],
                source=row["source_image"],
                glb=row["raw_trellis_glb"],
                qa=row["qa_front_render"],
            )
        )
    lines.extend(
        [
            "",
            "## Contact Sheets",
            "",
            "- [Enemy source image contact sheet](../QA/Enemy_SourceImages_ContactSheet.png)",
            "- [Enemy Trellis front QA contact sheet](../QA/Enemy_TrellisFront_ContactSheet.png)",
            "",
            "## Reports",
            "",
            "- [Enemy manifest](../Reports/Stage01_Enemies_TrellisManifest.json)",
        ]
    )
    STATUS_PATH.write_text("\n".join(lines) + "\n", encoding="utf-8")

    shared_lines = [
        "# EnemyBossBatch01 Stage 1 Status",
        "",
        f"Updated UTC: {manifest['updated_utc']}",
        "",
        "This shared status file is intentionally concise. Detailed enemy rows live in `ENEMIES_STAGE01_STATUS.md`; boss rows are owned by the secondary RunPod chat.",
        "",
        "## Regular Enemies",
        "",
        f"- Scope: 25 rows from `Content/Data/Enemies.csv`",
        f"- PendingReview: {completed}",
        f"- RawTrellisReady: {raw_ready}",
        f"- SourceImageReady: {source_ready}",
        f"- PendingSourceImage: {pending}",
        "- Report: [Stage01_Enemies_TrellisManifest.json](../Reports/Stage01_Enemies_TrellisManifest.json)",
        "- Status: [ENEMIES_STAGE01_STATUS.md](ENEMIES_STAGE01_STATUS.md)",
        "",
        "## Bosses",
        "",
        "- Owned by the secondary RunPod chat.",
        "- Existing status, if present: [BOSSES_STAGE01_STATUS.md](BOSSES_STAGE01_STATUS.md)",
        "- Existing report, if present: [Stage01_Bosses_TrellisManifest.json](../Reports/Stage01_Bosses_TrellisManifest.json)",
    ]
    SHARED_STATUS_PATH.write_text("\n".join(shared_lines) + "\n", encoding="utf-8")


def refresh_reports(rows: list[dict[str, str]]) -> dict[str, object]:
    for row in rows:
        prompt_path = PROMPT_DIR / f"{row['EnemyID']}.txt"
        if not prompt_path.exists():
            prompt_path.write_text(prompt_for(row), encoding="utf-8")

    make_contact_sheet(
        [SOURCE_DIR / f"{row['EnemyID']}.png" for row in rows],
        RUN_ROOT / "QA" / "Enemy_SourceImages_ContactSheet.png",
        lambda path: path.stem,
    )
    make_contact_sheet(
        [TRELLIS_INPUT_DIR / f"{row['EnemyID']}.png" for row in rows],
        RUN_ROOT / "QA" / "Enemy_TrellisInputs_ContactSheet.png",
        lambda path: path.stem,
    )
    make_contact_sheet(
        [QA_DIR / f"{row['EnemyID']}_front.png" for row in rows],
        RUN_ROOT / "QA" / "Enemy_TrellisFront_ContactSheet.png",
        lambda path: path.name.replace("_front.png", ""),
    )

    manifest = build_manifest(rows)
    MANIFEST_PATH.write_text(json.dumps(manifest, indent=2), encoding="utf-8")
    write_status(manifest)
    return manifest


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--generate-images", action="store_true")
    parser.add_argument("--run-trellis", action="store_true")
    parser.add_argument("--prepare-trellis-inputs", action="store_true")
    parser.add_argument("--render-qa", action="store_true")
    parser.add_argument("--refresh", action="store_true")
    parser.add_argument("--manual-slime", action="store_true")
    parser.add_argument("--force-images", action="store_true")
    parser.add_argument("--force-trellis", action="store_true")
    parser.add_argument("--force-trellis-inputs", action="store_true")
    parser.add_argument("--force-qa", action="store_true")
    parser.add_argument("--seed-offset", type=int, default=0)
    parser.add_argument("--only", nargs="*", default=[])
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    ensure_dirs()
    rows = load_rows()
    only = set(args.only)

    if args.generate_images:
        generate_source_images(rows, only, args.force_images, args.seed_offset)
        refresh_reports(rows)
    if args.manual_slime:
        write_manual_slime()
        refresh_reports(rows)
    if args.prepare_trellis_inputs:
        prepare_trellis_inputs(rows, only, args.force_trellis_inputs)
        refresh_reports(rows)
    if args.run_trellis:
        run_trellis(rows, only, args.force_trellis)
        refresh_reports(rows)
    if args.render_qa:
        render_qa(rows, only, args.force_qa)
        refresh_reports(rows)
    if args.refresh or not (args.generate_images or args.run_trellis or args.render_qa):
        manifest = refresh_reports(rows)
        print(json.dumps(manifest["counts_by_status"], indent=2))


if __name__ == "__main__":
    main()
