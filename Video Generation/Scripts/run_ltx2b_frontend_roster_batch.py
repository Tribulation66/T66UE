#!/usr/bin/env python3
"""Run the T66 frontend video roster through LTX-Video on RunPod.

This script is intended to run on the pod, not inside Unreal. It reads the
frontend job manifest, uses the generated poster plate as image conditioning,
and writes encoded game-ready MP4s using the same relative paths as
`Content/Movies`.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import shutil
import subprocess
import sys
import time
from pathlib import Path


ROOT = Path("/workspace/T66/VideoGeneration")
LTX_REPO = ROOT / "Repos" / "LTX-Video"
CONFIG = ROOT / "t66_ltxv-2b-0.9.8-distilled-no-enhance.yaml"
DEFAULT_INPUT_ROOT = ROOT / "Inputs" / "FrontendRoster"
DEFAULT_OUTPUT_ROOT = ROOT / "OutputsAI" / "FrontendRoster_LTX2B"

BASE_NEGATIVE_PROMPT = (
    "text, UI, logo, subtitles, extra limbs, malformed hands, duplicate face, "
    "camera shake, hard cuts, glitch artifacts, blurred subject, distorted anatomy, "
    "melting face, changing identity, changing outfit, cropped head, cropped feet"
)


def run(command: list[str], cwd: Path | None = None) -> None:
    subprocess.run(command, cwd=str(cwd) if cwd else None, check=True)


def stable_seed(*parts: str) -> int:
    digest = hashlib.sha1("|".join(parts).encode("utf-8")).hexdigest()
    return 100000 + (int(digest[:8], 16) % 800000)


def safe_name(job: dict) -> str:
    bits = [job["entityType"], job["entityId"], job["skinId"]]
    if job.get("bodyType"):
        bits.append(job["bodyType"])
    return "_".join(bits)


def prompt_for_job(job: dict) -> str:
    base = job["prompt"].rstrip(".")
    skin_note = ""
    if job["skinId"].lower() == "beachgoer":
        skin_note = " The beachgoer skin reads clearly through brighter fabric, relaxed summer styling, and warmer rim light."
    if job["entityType"] == "Hero":
        motion = (
            " The character performs a restrained heroic idle loop: breathing, slight weight shift, "
            "cape or cloth motion, metal and weapon highlights, and torchlight flicker."
        )
    else:
        motion = (
            " The companion performs a restrained companion idle loop: breathing, slight head and shoulder motion, "
            "cloth movement, and ambient fantasy light flicker."
        )
    return (
        f"{base}.{skin_note}{motion} Keep the same centered composition and recognizable silhouette from the source plate. "
        "No interface, no text, no cuts, dark fantasy character selection video."
    )


def newest_mp4(folder: Path) -> Path:
    matches = sorted(folder.rglob("*.mp4"), key=lambda path: path.stat().st_mtime, reverse=True)
    if not matches:
        raise FileNotFoundError(f"No MP4 generated under {folder}")
    return matches[0]


def ensure_ltx_ready() -> None:
    if not LTX_REPO.exists():
        LTX_REPO.parent.mkdir(parents=True, exist_ok=True)
        run(["git", "clone", "https://github.com/Lightricks/LTX-Video.git", str(LTX_REPO)])

    run([sys.executable, "-m", "pip", "install", "--upgrade", "pip", "setuptools", "wheel"], cwd=LTX_REPO)
    run([sys.executable, "-m", "pip", "install", "-e", ".[inference]"], cwd=LTX_REPO)
    run([sys.executable, "-m", "pip", "install", "diffusers==0.35.2"], cwd=LTX_REPO)

    if not CONFIG.exists():
        source_config = LTX_REPO / "configs" / "ltxv-2b-0.9.8-distilled.yaml"
        text = source_config.read_text(encoding="utf-8")
        text = text.replace("prompt_enhancement_words_threshold: 120", "prompt_enhancement_words_threshold: 0")
        CONFIG.write_text(text, encoding="utf-8")

    run(
        [
            sys.executable,
            "-c",
            (
                "from huggingface_hub import hf_hub_download\n"
                "for f in ['ltxv-2b-0.9.8-distilled.safetensors','ltxv-spatial-upscaler-0.9.8.safetensors']:\n"
                " print(hf_hub_download(repo_id='Lightricks/LTX-Video', filename=f, repo_type='model'))\n"
            ),
        ]
    )


def generate_job(job: dict, input_root: Path, raw_root: Path, encoded_root: Path, frames: int, force: bool) -> dict:
    name = safe_name(job)
    poster = input_root / job["poster"]
    if not poster.exists():
        raise FileNotFoundError(f"Missing poster for {name}: {poster}")

    raw_dir = raw_root / name
    encoded_path = encoded_root / job["movie"]
    encoded_path.parent.mkdir(parents=True, exist_ok=True)

    if encoded_path.exists() and not force:
        return {"job": name, "status": "skipped_existing", "encoded": str(encoded_path)}

    if raw_dir.exists() and force:
        shutil.rmtree(raw_dir)
    raw_dir.mkdir(parents=True, exist_ok=True)

    seed = stable_seed(job["entityType"], job["entityId"], job["skinId"], job.get("bodyType", ""))
    start = time.time()
    run(
        [
            sys.executable,
            "inference.py",
            "--pipeline_config",
            str(CONFIG),
            "--prompt",
            prompt_for_job(job),
            "--negative_prompt",
            BASE_NEGATIVE_PROMPT,
            "--conditioning_media_paths",
            str(poster),
            "--conditioning_start_frames",
            "0",
            "--conditioning_strengths",
            "0.78",
            "--image_cond_noise_scale",
            "0.10",
            "--height",
            "672",
            "--width",
            "704",
            "--num_frames",
            str(frames),
            "--frame_rate",
            "24",
            "--seed",
            str(seed),
            "--output_path",
            str(raw_dir),
        ],
        cwd=LTX_REPO,
    )

    source_video = newest_mp4(raw_dir)
    run(
        [
            "ffmpeg",
            "-nostdin",
            "-hide_banner",
            "-loglevel",
            "error",
            "-y",
            "-i",
            str(source_video),
            "-vf",
            "scale=712:680:force_original_aspect_ratio=increase,crop=712:680,fps=30,format=yuv420p",
            "-an",
            "-c:v",
            "libx264",
            "-profile:v",
            "high",
            "-level",
            "4.1",
            "-pix_fmt",
            "yuv420p",
            "-preset",
            "slow",
            "-crf",
            "21",
            "-movflags",
            "+faststart",
            str(encoded_path),
        ]
    )

    duration = time.time() - start
    return {
        "job": name,
        "status": "generated",
        "encoded": str(encoded_path),
        "source": str(source_video),
        "seconds": round(duration, 2),
    }


def load_jobs(input_root: Path, statuses: set[str]) -> list[dict]:
    manifest = input_root / "Video Generation" / "Manifests" / "frontend_video_jobs.json"
    data = json.loads(manifest.read_text(encoding="utf-8"))
    return [job for job in data["jobs"] if job.get("status") in statuses]


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--input-root", type=Path, default=DEFAULT_INPUT_ROOT)
    parser.add_argument("--output-root", type=Path, default=DEFAULT_OUTPUT_ROOT)
    parser.add_argument("--status", action="append", default=["placeholder_generated"])
    parser.add_argument("--max-jobs", type=int, default=0)
    parser.add_argument("--start-index", type=int, default=0)
    parser.add_argument("--frames", type=int, default=81)
    parser.add_argument("--force", action="store_true")
    parser.add_argument("--skip-setup", action="store_true")
    args = parser.parse_args()

    os.environ.setdefault("HF_HUB_DISABLE_XET", "1")
    if not args.skip_setup:
        ensure_ltx_ready()

    jobs = load_jobs(args.input_root, set(args.status))
    if args.start_index:
        jobs = jobs[args.start_index :]
    if args.max_jobs > 0:
        jobs = jobs[: args.max_jobs]

    args.output_root.mkdir(parents=True, exist_ok=True)
    raw_root = args.output_root / "Raw"
    encoded_root = args.output_root / "RuntimeEncoded"
    status_path = args.output_root / "frontend_roster_status.jsonl"

    print(f"running {len(jobs)} frontend video jobs")
    with status_path.open("a", encoding="utf-8") as status_file:
        for index, job in enumerate(jobs, start=1):
            name = safe_name(job)
            print(f"[{index}/{len(jobs)}] {name}", flush=True)
            try:
                result = generate_job(job, args.input_root, raw_root, encoded_root, args.frames, args.force)
            except Exception as exc:
                result = {"job": name, "status": "failed", "error": str(exc)}
            status_file.write(json.dumps(result) + "\n")
            status_file.flush()
            print(json.dumps(result), flush=True)

    print(f"status: {status_path}")
    print(f"encoded: {encoded_root}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
