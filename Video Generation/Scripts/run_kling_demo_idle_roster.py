#!/usr/bin/env python3
"""Generate and optionally install Kling idle videos for the demo roster."""

from __future__ import annotations

import argparse
import json
import shutil
import subprocess
import sys
import time
from datetime import datetime
from pathlib import Path
from typing import Any

import numpy as np
from PIL import Image

from kling_client import (
    DEFAULT_ACCESS_FILE,
    KlingClient,
    KlingClientError,
    encode_image_base64,
    summarize_response,
)


PROJECT_ROOT = Path(__file__).resolve().parents[2]
VIDEO_ROOT = PROJECT_ROOT / "Video Generation"
DEFAULT_MANIFEST = VIDEO_ROOT / "Manifests" / "kling_demo_idle_roster.json"
RUNS_ROOT = VIDEO_ROOT / "Runs"
FRONTEND_JOBS = VIDEO_ROOT / "Manifests" / "frontend_video_jobs.json"
RUNTIME_CATALOG = PROJECT_ROOT / "RuntimeDependencies" / "T66" / "Video" / "frontend_videos.json"
SOURCE_CATALOG = VIDEO_ROOT / "Manifests" / "frontend_videos.json"


def read_json(path: Path) -> Any:
    return json.loads(path.read_text(encoding="utf-8"))


def write_json(path: Path, data: Any) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(data, indent=2, sort_keys=True) + "\n", encoding="utf-8")


def append_jsonl(path: Path, row: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("a", encoding="utf-8") as handle:
        handle.write(json.dumps(row, sort_keys=True) + "\n")


def project_path(relative_path: str) -> Path:
    return PROJECT_ROOT / relative_path


def create_run_dir(root: Path, prefix: str) -> Path:
    stamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    run_dir = root / f"{prefix}_{stamp}"
    run_dir.mkdir(parents=True, exist_ok=False)
    return run_dir


def target_slug(target: dict[str, Any]) -> str:
    bits = [target["entityType"], target["entityId"], target["skinId"]]
    if target.get("bodyType"):
        bits.append(target["bodyType"])
    return "_".join(bits)


def target_matches_job(target: dict[str, Any], job: dict[str, Any]) -> bool:
    return (
        job.get("entityType") == target.get("entityType")
        and job.get("entityId") == target.get("entityId")
        and job.get("skinId") == target.get("skinId")
        and (job.get("bodyType") or "") == (target.get("bodyType") or "")
    )


def first_task_id(response: dict[str, Any]) -> str | None:
    body = response.get("body")
    data = body.get("data") if isinstance(body, dict) else None
    if not isinstance(data, dict):
        return None
    task_id = data.get("task_id")
    return str(task_id) if task_id else None


def task_result_video_url(response: dict[str, Any]) -> str | None:
    body = response.get("body")
    data = body.get("data") if isinstance(body, dict) else None
    result = data.get("task_result") if isinstance(data, dict) else None
    videos = result.get("videos") if isinstance(result, dict) else None
    if not isinstance(videos, list) or not videos or not isinstance(videos[0], dict):
        return None
    url = videos[0].get("url")
    return str(url) if url else None


def poll_image2video(
    client: KlingClient,
    task_id: str,
    status_path: Path,
    interval_seconds: int,
    timeout_seconds: int,
) -> dict[str, Any]:
    started = time.monotonic()
    while True:
        elapsed = time.monotonic() - started
        if elapsed > timeout_seconds:
            raise TimeoutError(f"Timed out after {timeout_seconds}s waiting for Kling task {task_id}")

        response = client.get(f"/videos/image2video/{task_id}")
        summary = summarize_response(response)
        append_jsonl(status_path, {
            "event": "poll",
            "task_id": task_id,
            "elapsed_seconds": round(elapsed, 2),
            "summary": summary,
        })

        status = str(summary.get("task_status") or "").lower()
        if status in {"succeed", "failed"}:
            return response
        time.sleep(interval_seconds)


def find_ffmpeg() -> str | None:
    found = shutil.which("ffmpeg")
    if found:
        return found
    try:
        import imageio_ffmpeg  # type: ignore

        return imageio_ffmpeg.get_ffmpeg_exe()
    except Exception:
        return None


def find_ffprobe() -> str | None:
    return shutil.which("ffprobe")


def probe_video(path: Path) -> dict[str, Any] | None:
    ffprobe = find_ffprobe()
    if not ffprobe:
        return None
    completed = subprocess.run(
        [
            ffprobe,
            "-v",
            "error",
            "-select_streams",
            "v:0",
            "-show_entries",
            "stream=codec_name,pix_fmt,width,height,r_frame_rate,avg_frame_rate,duration,nb_frames",
            "-of",
            "json",
            str(path),
        ],
        check=False,
        capture_output=True,
        text=True,
    )
    if completed.returncode != 0:
        return {"error": completed.stderr.strip()}
    return json.loads(completed.stdout)


def encode_runtime_candidate(source: Path, output: Path) -> dict[str, Any]:
    ffmpeg = find_ffmpeg()
    if not ffmpeg:
        return {"status": "skipped", "reason": "ffmpeg not found"}

    output.parent.mkdir(parents=True, exist_ok=True)
    completed = subprocess.run(
        [
            ffmpeg,
            "-nostdin",
            "-hide_banner",
            "-loglevel",
            "error",
            "-y",
            "-i",
            str(source),
            "-vf",
            "scale=712:680:force_original_aspect_ratio=increase,crop=712:680:(iw-712)/2:(ih-680)/2,fps=30,format=yuv420p,setsar=1",
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
            "18",
            "-movflags",
            "+faststart",
            str(output),
        ],
        check=False,
        capture_output=True,
        text=True,
    )
    if completed.returncode != 0:
        return {"status": "failed", "stderr": completed.stderr.strip()}
    return {"status": "encoded", "path": output.as_posix(), "bytes": output.stat().st_size}


def make_contact_sheet(video: Path, output: Path) -> dict[str, Any]:
    ffmpeg = find_ffmpeg()
    if not ffmpeg:
        return {"status": "skipped", "reason": "ffmpeg not found"}
    output.parent.mkdir(parents=True, exist_ok=True)
    completed = subprocess.run(
        [
            ffmpeg,
            "-nostdin",
            "-hide_banner",
            "-loglevel",
            "error",
            "-y",
            "-i",
            str(video),
            "-vf",
            "fps=1,scale=356:340,tile=3x2",
            "-frames:v",
            "1",
            str(output),
        ],
        check=False,
        capture_output=True,
        text=True,
    )
    if completed.returncode != 0:
        return {"status": "failed", "stderr": completed.stderr.strip()}
    return {"status": "written", "path": output.as_posix(), "bytes": output.stat().st_size}


def extract_loop_frame(video: Path, output: Path, last: bool) -> None:
    ffmpeg = find_ffmpeg()
    if not ffmpeg:
        raise RuntimeError("ffmpeg not found")
    command = [ffmpeg, "-nostdin", "-hide_banner", "-loglevel", "error", "-y"]
    if last:
        command += ["-sseof", "-0.08"]
    command += ["-i", str(video), "-frames:v", "1", str(output)]
    completed = subprocess.run(command, check=False, capture_output=True, text=True)
    if completed.returncode != 0:
        raise RuntimeError(completed.stderr.strip())


def compute_loop_metrics(video: Path, target_dir: Path) -> dict[str, Any]:
    frames_dir = target_dir / "LoopFrames"
    frames_dir.mkdir(parents=True, exist_ok=True)
    first = frames_dir / "first.png"
    last = frames_dir / "last.png"
    try:
        extract_loop_frame(video, first, last=False)
        extract_loop_frame(video, last, last=True)
        a = np.asarray(Image.open(first).convert("RGB"), dtype=np.float32)
        b = np.asarray(Image.open(last).convert("RGB"), dtype=np.float32)
        diff = np.abs(a - b)
        mse = float(np.mean((a - b) ** 2))
        mae = float(np.mean(diff))
        peak = 255.0
        psnr = 99.0 if mse == 0.0 else float(20.0 * np.log10(peak / np.sqrt(mse)))
        status = "pass" if mae <= 18.0 and psnr >= 24.0 else "review"
        return {
            "status": status,
            "firstFrame": first.as_posix(),
            "lastFrame": last.as_posix(),
            "meanAbsoluteError": round(mae, 3),
            "mse": round(mse, 3),
            "psnr": round(psnr, 3),
            "thresholds": {"meanAbsoluteErrorMax": 18.0, "psnrMin": 24.0},
        }
    except Exception as exc:
        return {"status": "failed", "error": str(exc)}


def submit_target(
    client: KlingClient,
    target: dict[str, Any],
    run_dir: Path,
    manifest: dict[str, Any],
    args: argparse.Namespace,
) -> dict[str, Any]:
    slug = target_slug(target)
    target_dir = run_dir / "Targets" / slug
    target_dir.mkdir(parents=True, exist_ok=True)
    status_path = target_dir / "status.jsonl"

    first_frame = project_path(target["firstFrame"])
    if not first_frame.exists():
        raise RuntimeError(f"Missing first frame for {slug}: {first_frame}")

    shutil.copy2(first_frame, target_dir / "first_frame.png")
    poster = project_path(target["poster"])
    if poster.exists():
        shutil.copy2(poster, target_dir / "poster.png")

    prompt = target["videoPrompt"]
    payload = {
        "model_name": args.model_name or manifest.get("defaultModelName", "kling-v1"),
        "image": encode_image_base64(first_frame),
        "prompt": prompt,
        "negative_prompt": manifest.get("negativePrompt", ""),
        "duration": str(args.duration_seconds or manifest.get("defaultDurationSeconds", 5)),
        "mode": args.mode or manifest.get("defaultMode", "std"),
        "cfg_scale": float(manifest.get("defaultCfgScale", 0.5)),
        "external_task_id": f"t66_{slug}_{int(time.time())}",
        "callback_url": None,
    }
    redacted = dict(payload)
    redacted["image"] = f"<base64 redacted length={len(payload['image'])}>"
    redacted["source_image"] = first_frame.as_posix()
    write_json(target_dir / "request_payload_image2video.redacted.json", redacted)
    write_json(target_dir / "target.json", target)
    (target_dir / "video_prompt.txt").write_text(prompt + "\n", encoding="utf-8")

    started = time.time()
    response = client.post("/videos/image2video", payload)
    summary = summarize_response(response)
    append_jsonl(status_path, {
        "event": "submit",
        "target": slug,
        "seconds": round(time.time() - started, 2),
        "summary": summary,
    })
    write_json(target_dir / "submit_response_summary.json", summary)
    if not response.get("ok"):
        return {"status": "submit_failed", "target": slug, "summary": summary}

    task_id = first_task_id(response)
    if not task_id:
        return {"status": "submit_failed_missing_task_id", "target": slug, "summary": summary}

    final_response = poll_image2video(client, task_id, status_path, args.poll_interval_seconds, args.poll_timeout_seconds)
    final_summary = summarize_response(final_response)
    write_json(target_dir / "final_task_summary.json", final_summary)
    if str(final_summary.get("task_status") or "").lower() != "succeed":
        return {"status": "task_not_succeeded", "target": slug, "task_id": task_id, "summary": final_summary}

    video_url = task_result_video_url(final_response)
    if not video_url:
        return {"status": "task_succeeded_missing_video_url", "target": slug, "task_id": task_id}

    raw_path = target_dir / "Raw" / f"{slug}_kling_raw.mp4"
    download_info = client.download(video_url, raw_path)
    raw_probe = probe_video(raw_path)
    if raw_probe is not None:
        write_json(target_dir / "probe_raw.json", raw_probe)

    encoded_path = target_dir / "Encoded" / f"{slug}_712x680.mp4"
    encode_info = encode_runtime_candidate(raw_path, encoded_path)
    encoded_probe = probe_video(encoded_path) if encode_info.get("status") == "encoded" else None
    if encoded_probe is not None:
        write_json(target_dir / "probe_encoded.json", encoded_probe)

    contact = make_contact_sheet(encoded_path, target_dir / "contact_sheet_encoded.png") if encode_info.get("status") == "encoded" else None
    loop_metrics = compute_loop_metrics(encoded_path, target_dir) if encode_info.get("status") == "encoded" else None
    if loop_metrics is not None:
        write_json(target_dir / "loop_metrics.json", loop_metrics)

    result = {
        "status": "succeeded" if encode_info.get("status") == "encoded" else "encode_failed",
        "target": slug,
        "task_id": task_id,
        "download": download_info,
        "rawProbeWritten": raw_probe is not None,
        "encode": encode_info,
        "encodedProbeWritten": encoded_probe is not None,
        "contactSheet": contact,
        "loopMetrics": loop_metrics,
    }
    write_json(target_dir / "result.json", result)
    append_jsonl(run_dir / "status.jsonl", {"event": "target_complete", "target": slug, "result": result})
    return result


def update_asset_in_catalog(obj: Any, movie: str) -> int:
    updated = 0
    if isinstance(obj, dict):
        if obj.get("movie") == movie and obj.get("posterOnly") is True:
            obj["posterOnly"] = False
            updated += 1
        for value in obj.values():
            updated += update_asset_in_catalog(value, movie)
    elif isinstance(obj, list):
        for value in obj:
            updated += update_asset_in_catalog(value, movie)
    return updated


def install_successful_outputs(
    run_dir: Path,
    targets: list[dict[str, Any]],
    results: list[dict[str, Any]],
    update_manifests: bool,
) -> dict[str, Any]:
    installed: list[dict[str, Any]] = []
    result_by_target = {result.get("target"): result for result in results}
    for target in targets:
        slug = target_slug(target)
        result = result_by_target.get(slug)
        if not result or result.get("status") != "succeeded":
            continue
        encoded = Path(result["encode"]["path"])
        destination = PROJECT_ROOT / "Content" / "Movies" / target["movie"]
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(encoded, destination)
        installed.append({
            "target": slug,
            "movie": target["movie"],
            "path": destination.as_posix(),
            "bytes": destination.stat().st_size,
        })

    manifest_updates: dict[str, Any] = {"runtimeCatalogUpdated": 0, "sourceCatalogUpdated": 0, "jobsUpdated": 0}
    if update_manifests and installed:
        movies = {item["movie"] for item in installed}
        runtime_catalog = read_json(RUNTIME_CATALOG)
        for movie in movies:
            manifest_updates["runtimeCatalogUpdated"] += update_asset_in_catalog(runtime_catalog, movie)
        write_json(RUNTIME_CATALOG, runtime_catalog)

        if SOURCE_CATALOG.exists():
            source_catalog = read_json(SOURCE_CATALOG)
            for movie in movies:
                manifest_updates["sourceCatalogUpdated"] += update_asset_in_catalog(source_catalog, movie)
            write_json(SOURCE_CATALOG, source_catalog)

        jobs_data = read_json(FRONTEND_JOBS)
        for job in jobs_data.get("jobs", []):
            if job.get("movie") not in movies:
                continue
            matching = next((target for target in targets if target["movie"] == job.get("movie") and target_matches_job(target, job)), None)
            if not matching:
                continue
            job["status"] = "ai_generated_kling_anime_idle"
            job["generationModel"] = "native Kling image2video"
            job["sourceRun"] = run_dir.relative_to(PROJECT_ROOT).as_posix()
            job["sourceMovie"] = next((item["path"] for item in installed if item["movie"] == job.get("movie")), "")
            job["prompt"] = matching["videoPrompt"]
            manifest_updates["jobsUpdated"] += 1
        write_json(FRONTEND_JOBS, jobs_data)

    result = {"installed": installed, "manifestUpdates": manifest_updates}
    write_json(run_dir / "install_result.json", result)
    return result


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--manifest", type=Path, default=DEFAULT_MANIFEST)
    parser.add_argument("--access-file", type=Path, default=DEFAULT_ACCESS_FILE)
    parser.add_argument("--run-root", type=Path, default=RUNS_ROOT)
    parser.add_argument("--run-dir", type=Path, default=None)
    parser.add_argument("--submit", action="store_true")
    parser.add_argument("--install", action="store_true")
    parser.add_argument("--update-manifests", action="store_true")
    parser.add_argument("--target", action="append", default=[])
    parser.add_argument("--target-limit", type=int, default=0)
    parser.add_argument("--model-name", default="")
    parser.add_argument("--mode", default="")
    parser.add_argument("--duration-seconds", type=int, default=0)
    parser.add_argument("--poll-interval-seconds", type=int, default=10)
    parser.add_argument("--poll-timeout-seconds", type=int, default=900)
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    manifest = read_json(args.manifest)
    run_dir = args.run_dir or create_run_dir(args.run_root, manifest.get("runNamePrefix", "KlingAnimeIdleDemo"))
    run_dir.mkdir(parents=True, exist_ok=True)

    targets = list(manifest["targets"])
    if args.target:
        wanted = set(args.target)
        targets = [target for target in targets if target_slug(target) in wanted]
    if args.target_limit:
        targets = targets[: args.target_limit]

    write_json(run_dir / "run_config.json", {
        "manifest": args.manifest.as_posix(),
        "targetCount": len(targets),
        "targets": [target_slug(target) for target in targets],
        "submit": bool(args.submit),
        "install": bool(args.install),
        "updateManifests": bool(args.update_manifests),
    })

    try:
        if not args.submit:
            write_json(run_dir / "result.json", {"status": "dry_run", "runDir": run_dir.as_posix(), "targetCount": len(targets)})
            print(f"run_dir={run_dir}")
            print("status=dry_run")
            return 0

        client = KlingClient.from_local_access(args.access_file)
        results: list[dict[str, Any]] = []
        for target in targets:
            slug = target_slug(target)
            append_jsonl(run_dir / "status.jsonl", {"event": "target_start", "target": slug})
            try:
                results.append(submit_target(client, target, run_dir, manifest, args))
            except Exception as exc:
                result = {"status": "failed", "target": slug, "error": str(exc)}
                results.append(result)
                write_json(run_dir / "Targets" / slug / "result.json", result)
                append_jsonl(run_dir / "status.jsonl", {"event": "target_failed", "target": slug, "error": str(exc)})

        install_result = install_successful_outputs(run_dir, targets, results, args.update_manifests) if args.install else None
        summary = {
            "status": "completed",
            "runDir": run_dir.as_posix(),
            "targetCount": len(targets),
            "succeeded": sum(1 for result in results if result.get("status") == "succeeded"),
            "failed": sum(1 for result in results if result.get("status") != "succeeded"),
            "results": results,
            "installResult": install_result,
        }
        write_json(run_dir / "result.json", summary)
        print(f"run_dir={run_dir}")
        print(f"status=completed succeeded={summary['succeeded']} failed={summary['failed']}")
        return 0 if summary["failed"] == 0 else 2
    except KlingClientError as exc:
        write_json(run_dir / "result.json", {"status": "client_error", "error": str(exc), "runDir": run_dir.as_posix()})
        print(f"run_dir={run_dir}")
        print(f"status=client_error error={exc}")
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
