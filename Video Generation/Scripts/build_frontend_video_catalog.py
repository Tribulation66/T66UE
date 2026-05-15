#!/usr/bin/env python3
"""Build the frontend video catalog, prompts, posters, and placeholder clips.

The runtime owns only MP4s under Content/Movies and the JSON catalog under
RuntimeDependencies. This script keeps the editable generation sources under
Video Generation and can be rerun after new heroes, companions, or skins land.
"""

from __future__ import annotations

import argparse
import csv
import hashlib
import json
import os
import shutil
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor, as_completed
from pathlib import Path

from PIL import Image, ImageDraw


PROJECT_ROOT = Path(__file__).resolve().parents[2]
VIDEO_ROOT = PROJECT_ROOT / "Video Generation"
CONTENT_MOVIES = PROJECT_ROOT / "Content" / "Movies"
RUNTIME_VIDEO = PROJECT_ROOT / "RuntimeDependencies" / "T66" / "Video"
POSTER_ROOT = RUNTIME_VIDEO / "Posters"
PROMPT_ROOT = VIDEO_ROOT / "Prompts"
MANIFEST_ROOT = VIDEO_ROOT / "Manifests"

HERO_SOURCE_ROOT = PROJECT_ROOT / "Audit" / "Reference" / "Track1_Normalization" / "Comparisons"
COMPANION_SOURCE_ROOT = PROJECT_ROOT / "SourceAssets" / "Archive" / "FinalPortraits"
HERO_CSV = PROJECT_ROOT / "Content" / "Data" / "Heroes.csv"
COMPANION_CSV = PROJECT_ROOT / "Content" / "Data" / "Companions.csv"

LEGACY_MAIN_MENU_MOVIE = CONTENT_MOVIES / "MainMenuBackground.mp4"
LEGACY_ARTHUR_MOVIE = CONTENT_MOVIES / "HeroSelection" / "Hero_1_Default_Chad.mp4"
MAIN_MENU_POSTER = (
    "RuntimeDependencies/T66/UI/Reference/Screens/MainMenu/ScreenArt/"
    "mainmenu_screen_art_mainmenu_newmm_main_menu_newmm_base_clean_bloodyretro_1920.png"
)
JOB_METADATA_KEYS = [
    "status",
    "generationModel",
    "sourceRun",
    "sourceMovie",
    "generatedMovie",
    "generatedFrames",
]

SKIN_IDS = ["Default", "Beachgoer"]
BODY_TYPES = ["Chad", "Stacy"]
PANEL_SIZE = (712, 680)


def read_csv_rows(path: Path) -> list[dict[str, str]]:
    with path.open("r", encoding="utf-8-sig", newline="") as handle:
        return list(csv.DictReader(handle))


def stable_color(*parts: str) -> tuple[int, int, int]:
    digest = hashlib.sha1("|".join(parts).encode("utf-8")).digest()
    return (digest[0], digest[1], digest[2])


def runtime_path(path: Path) -> str:
    return path.relative_to(PROJECT_ROOT).as_posix()


def movie_path(path: Path) -> str:
    return path.relative_to(CONTENT_MOVIES).as_posix()


def job_key(job: dict) -> tuple[str, str, str, str]:
    return (
        job["entityType"],
        job["entityId"],
        job["skinId"],
        job.get("bodyType", ""),
    )


def ensure_parent(path: Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)


def fit_image(image: Image.Image, max_size: tuple[int, int]) -> Image.Image:
    image = image.convert("RGBA")
    image.thumbnail(max_size, Image.Resampling.LANCZOS)
    return image


def make_plate(source_path: Path, poster_path: Path, entity_id: str, skin_id: str, body_type: str | None) -> None:
    ensure_parent(poster_path)
    width, height = PANEL_SIZE
    accent = stable_color(entity_id, skin_id, body_type or "")
    skin_warmth = 36 if skin_id == "Beachgoer" else 0

    plate = Image.new("RGBA", PANEL_SIZE, (8, 7, 14, 255))
    pixels = plate.load()
    for y in range(height):
        for x in range(width):
            t = (x / width) * 0.55 + (y / height) * 0.45
            r = int(9 + accent[0] * 0.12 * t + skin_warmth * 0.45)
            g = int(8 + accent[1] * 0.10 * t + skin_warmth * 0.30)
            b = int(16 + accent[2] * 0.14 * t + skin_warmth * 0.18)
            pixels[x, y] = (min(r, 80), min(g, 76), min(b, 96), 255)

    draw = ImageDraw.Draw(plate, "RGBA")
    for offset in range(-height, width, 64):
        draw.line([(offset, height), (offset + height, 0)], fill=(accent[0], accent[1], accent[2], 22), width=3)
    draw.rectangle((5, 5, width - 6, height - 6), outline=(accent[0], accent[1], accent[2], 170), width=3)
    draw.rectangle((18, 18, width - 19, height - 19), outline=(255, 255, 255, 24), width=1)

    if source_path.exists():
        source = fit_image(Image.open(source_path), (560, 620))
        shadow = Image.new("RGBA", source.size, (0, 0, 0, 0))
        shadow_alpha = source.getchannel("A") if source.mode == "RGBA" else Image.new("L", source.size, 210)
        shadow.putalpha(shadow_alpha.point(lambda value: int(value * 0.45)))
        sx = (width - source.width) // 2
        sy = height - source.height - 34
        plate.alpha_composite(shadow, (sx + 12, sy + 12))
        plate.alpha_composite(source, (sx, sy))

    if skin_id == "Beachgoer":
        draw.rectangle((0, height - 54, width, height), fill=(28, 92, 120, 64))

    plate.convert("RGB").save(poster_path, "PNG", optimize=True)


def find_ffmpeg() -> str:
    env_path = os.environ.get("T66_FFMPEG_EXE")
    if env_path and Path(env_path).exists():
        return env_path

    found = shutil.which("ffmpeg")
    if found:
        return found

    extra_packages = os.environ.get("T66_PYTHON_PACKAGE_PATH")
    if extra_packages:
        sys.path.insert(0, extra_packages)

    try:
        import imageio_ffmpeg  # type: ignore

        return imageio_ffmpeg.get_ffmpeg_exe()
    except Exception as exc:  # pragma: no cover - used by local operators.
        raise RuntimeError(
            "ffmpeg was not found. Install ffmpeg, set T66_FFMPEG_EXE, or install imageio-ffmpeg."
        ) from exc


def encode_placeholder(ffmpeg: str, poster_path: Path, movie_path_target: Path, force: bool) -> bool:
    if movie_path_target.exists() and not force:
        return False

    ensure_parent(movie_path_target)
    filter_graph = (
        "zoompan=z='min(zoom+0.00035,1.035)':"
        "x='iw/2-(iw/zoom/2)':y='ih/2-(ih/zoom/2)':d=150:s=712x680:fps=30,"
        "noise=alls=1:allf=t+u,format=yuv420p"
    )
    command = [
        ffmpeg,
        "-nostdin",
        "-hide_banner",
        "-loglevel",
        "error",
        "-y" if force else "-n",
        "-loop",
        "1",
        "-framerate",
        "30",
        "-i",
        str(poster_path),
        "-vf",
        filter_graph,
        "-frames:v",
        "150",
        "-an",
        "-c:v",
        "libx264",
        "-profile:v",
        "high",
        "-level",
        "4.1",
        "-pix_fmt",
        "yuv420p",
        "-r",
        "30",
        "-preset",
        "veryfast",
        "-crf",
        "23",
        "-movflags",
        "+faststart",
        str(movie_path_target),
    ]
    subprocess.run(command, check=True)
    return True


def write_prompt(path: Path, title: str, runtime_movie: str, source_image: Path, prompt: str) -> None:
    ensure_parent(path)
    text = f"""# {title}

Runtime movie: `{runtime_movie}`
Source plate: `{runtime_path(source_image) if source_image.exists() else "missing"}`

## Prompt

{prompt}

## Negative Prompt

text, UI, logo, subtitles, extra limbs, malformed hands, duplicate face, camera shake, hard cuts, glitch artifacts, blurred subject, distorted anatomy

## Runtime Notes

Draft clips can be replaced in-place by a reviewed RunPod AI render using the same runtime path.
"""
    path.write_text(text, encoding="utf-8")


def apply_existing_job_metadata(jobs: list[dict]) -> None:
    manifest_path = MANIFEST_ROOT / "frontend_video_jobs.json"
    if not manifest_path.exists():
        return

    try:
        previous = json.loads(manifest_path.read_text(encoding="utf-8"))
    except json.JSONDecodeError:
        return

    previous_by_key = {
        job_key(job): job
        for job in previous.get("jobs", [])
        if all(key in job for key in ["entityType", "entityId", "skinId"])
    }

    for job in jobs:
        previous_job = previous_by_key.get(job_key(job))
        if not previous_job:
            continue
        for key in JOB_METADATA_KEYS:
            if key in previous_job:
                job[key] = previous_job[key]


def build_targets() -> tuple[dict, list[dict]]:
    heroes = read_csv_rows(HERO_CSV)
    companions = read_csv_rows(COMPANION_CSV)

    runtime_catalog: dict = {
        "schemaVersion": 2,
        "mainMenu": {
            "background": {
                "movie": "Frontend/MainMenu/MainMenuBackground.mp4",
                "poster": MAIN_MENU_POSTER,
            }
        },
        "heroSelection": {
            "fallbacks": {},
            "heroes": {},
            "companions": {},
        },
    }
    jobs: list[dict] = []

    for hero in heroes:
        hero_id = hero["HeroID"]
        display_name = hero["DisplayName"]
        runtime_catalog["heroSelection"]["heroes"][hero_id] = {}
        for skin_id in SKIN_IDS:
            runtime_catalog["heroSelection"]["heroes"][hero_id][skin_id] = {}
            for body_type in BODY_TYPES:
                source_path = HERO_SOURCE_ROOT / f"{hero_id}_{body_type}.png"
                movie = CONTENT_MOVIES / "Frontend" / "HeroSelection" / "Heroes" / hero_id / skin_id / f"{body_type}.mp4"
                poster = POSTER_ROOT / "HeroSelection" / "Heroes" / hero_id / skin_id / f"{body_type}.png"
                prompt_path = PROMPT_ROOT / "HeroSelection" / "Heroes" / hero_id / skin_id / f"{body_type}.md"
                runtime_catalog["heroSelection"]["heroes"][hero_id][skin_id][body_type] = {
                    "movie": movie_path(movie),
                    "poster": runtime_path(poster),
                }
                prompt = (
                    f"Locked camera fantasy character-selection loop. {display_name} as the {body_type} body variant "
                    f"wearing the {skin_id} skin stands centered in a dark throne-room vignette, subtle idle motion, "
                    "cloth and light movement, readable silhouette, no interface elements."
                )
                jobs.append(
                    {
                        "entityType": "Hero",
                        "entityId": hero_id,
                        "displayName": display_name,
                        "skinId": skin_id,
                        "bodyType": body_type,
                        "sourceImage": runtime_path(source_path),
                        "movie": movie_path(movie),
                        "poster": runtime_path(poster),
                        "promptFile": runtime_path(prompt_path),
                        "prompt": prompt,
                        "status": "placeholder_generated",
                    }
                )

    for companion in companions:
        companion_id = companion["CompanionID"]
        display_name = companion["DisplayName"]
        runtime_catalog["heroSelection"]["companions"][companion_id] = {}
        for skin_id in SKIN_IDS:
            source_path = COMPANION_SOURCE_ROOT / f"Companion_{display_name}.png"
            movie = CONTENT_MOVIES / "Frontend" / "HeroSelection" / "Companions" / companion_id / f"{skin_id}.mp4"
            poster = POSTER_ROOT / "HeroSelection" / "Companions" / companion_id / f"{skin_id}.png"
            prompt_path = PROMPT_ROOT / "HeroSelection" / "Companions" / companion_id / f"{skin_id}.md"
            runtime_catalog["heroSelection"]["companions"][companion_id][skin_id] = {
                "movie": movie_path(movie),
                "poster": runtime_path(poster),
            }
            prompt = (
                f"Locked camera companion-selection loop. {display_name} the companion wearing the {skin_id} skin "
                "holds a heroic idle pose in a moody fantasy alcove, subtle breathing and light movement, "
                "readable silhouette, no interface elements."
            )
            jobs.append(
                {
                    "entityType": "Companion",
                    "entityId": companion_id,
                    "displayName": display_name,
                    "skinId": skin_id,
                    "sourceImage": runtime_path(source_path),
                    "movie": movie_path(movie),
                    "poster": runtime_path(poster),
                    "promptFile": runtime_path(prompt_path),
                    "prompt": prompt,
                    "status": "placeholder_generated",
                }
            )

    hero_fallbacks = runtime_catalog["heroSelection"]["heroes"]["Hero_1"]["Default"]
    companion_fallback = runtime_catalog["heroSelection"]["companions"]["Companion_01"]["Default"]
    runtime_catalog["heroSelection"]["fallbacks"] = {
        "heroChad": hero_fallbacks["Chad"],
        "heroStacy": hero_fallbacks["Stacy"],
        "hero": hero_fallbacks["Chad"],
        "companion": companion_fallback,
    }

    return runtime_catalog, jobs


def materialize_assets(
    jobs: list[dict],
    generate_videos: bool,
    force: bool,
    jobs_count: int,
    replace_ai_movies: bool,
) -> None:
    ffmpeg = find_ffmpeg() if generate_videos else ""
    encode_tasks: list[tuple[Path, Path, bool]] = []

    for job in jobs:
        source = PROJECT_ROOT / job["sourceImage"]
        poster = PROJECT_ROOT / job["poster"]
        movie = CONTENT_MOVIES / job["movie"]
        make_plate(source, poster, job["entityId"], job["skinId"], job.get("bodyType"))
        write_prompt(PROJECT_ROOT / job["promptFile"], f"{job['entityId']} {job['skinId']} {job.get('bodyType', '')}".strip(), job["movie"], source, job["prompt"])

        if (
            str(job.get("status", "")).startswith("ai_")
            and movie.exists()
            and not replace_ai_movies
        ):
            continue

        if job["entityType"] == "Hero" and job["entityId"] == "Hero_1" and job["skinId"] == "Default" and job.get("bodyType") == "Chad" and LEGACY_ARTHUR_MOVIE.exists():
            ensure_parent(movie)
            if force or not movie.exists():
                shutil.copy2(LEGACY_ARTHUR_MOVIE, movie)
            job["status"] = "ai_accepted"
            job["sourceMovie"] = runtime_path(LEGACY_ARTHUR_MOVIE)
            continue

        encode_tasks.append((poster, movie, force))

    main_menu_target = CONTENT_MOVIES / "Frontend" / "MainMenu" / "MainMenuBackground.mp4"
    if LEGACY_MAIN_MENU_MOVIE.exists():
        ensure_parent(main_menu_target)
        if force or not main_menu_target.exists():
            shutil.copy2(LEGACY_MAIN_MENU_MOVIE, main_menu_target)

    if not generate_videos:
        return

    with ThreadPoolExecutor(max_workers=max(1, jobs_count)) as executor:
        futures = [executor.submit(encode_placeholder, ffmpeg, poster, movie, force) for poster, movie, force in encode_tasks]
        for future in as_completed(futures):
            future.result()


def write_json(path: Path, data: dict | list) -> None:
    ensure_parent(path)
    path.write_text(json.dumps(data, indent=2) + "\n", encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--generate-videos", action="store_true", help="Encode placeholder MP4s.")
    parser.add_argument("--force", action="store_true", help="Overwrite generated posters and MP4s.")
    parser.add_argument("--jobs", type=int, default=2, help="Parallel ffmpeg workers.")
    parser.add_argument(
        "--replace-ai-movies",
        action="store_true",
        help="Allow placeholder encoding to overwrite jobs already marked as AI-generated.",
    )
    args = parser.parse_args()

    catalog, jobs = build_targets()
    apply_existing_job_metadata(jobs)
    materialize_assets(jobs, args.generate_videos, args.force, args.jobs, args.replace_ai_movies)

    write_json(RUNTIME_VIDEO / "frontend_videos.json", catalog)
    write_json(MANIFEST_ROOT / "frontend_videos.json", catalog)
    write_json(MANIFEST_ROOT / "frontend_video_jobs.json", {"schemaVersion": 1, "jobs": jobs})

    hero_count = sum(1 for job in jobs if job["entityType"] == "Hero")
    companion_count = sum(1 for job in jobs if job["entityType"] == "Companion")
    print(f"frontend video catalog written: {hero_count} hero entries, {companion_count} companion entries")
    if args.generate_videos:
        print("placeholder MP4 generation complete")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
