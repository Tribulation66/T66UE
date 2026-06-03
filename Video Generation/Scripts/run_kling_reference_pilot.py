#!/usr/bin/env python3
"""Run native Kling API smoke checks for T66 hero-selection video work."""

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

from kling_client import (
    DEFAULT_ACCESS_FILE,
    KlingClient,
    KlingClientError,
    encode_image_base64,
    summarize_response,
)


PROJECT_ROOT = Path(__file__).resolve().parents[2]
VIDEO_ROOT = PROJECT_ROOT / "Video Generation"
DEFAULT_MANIFEST = VIDEO_ROOT / "Manifests" / "kling_reference_pilot.json"
FRONTEND_JOBS = VIDEO_ROOT / "Manifests" / "frontend_video_jobs.json"
RUNS_ROOT = VIDEO_ROOT / "Runs"
DEFAULT_NEGATIVE_PROMPT = (
    "text, UI, logo, subtitles, extra limbs, malformed hands, duplicate face, "
    "camera shake, hard cuts, glitch artifacts, blurred subject, distorted anatomy, "
    "melting face, changing identity, changing outfit, cropped head, cropped feet"
)


def project_path(relative_path: str) -> Path:
    return PROJECT_ROOT / relative_path


def read_json(path: Path) -> Any:
    return json.loads(path.read_text(encoding="utf-8"))


def write_json(path: Path, data: Any) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(data, indent=2, sort_keys=True) + "\n", encoding="utf-8")


def append_jsonl(path: Path, row: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("a", encoding="utf-8") as handle:
        handle.write(json.dumps(row, sort_keys=True) + "\n")


def safe_name(job: dict[str, Any]) -> str:
    bits = [job["entityType"], job["entityId"], job["skinId"]]
    if job.get("bodyType"):
        bits.append(job["bodyType"])
    return "_".join(bits)


def load_frontend_jobs() -> list[dict[str, Any]]:
    data = read_json(FRONTEND_JOBS)
    return data["jobs"]


def find_job(target: dict[str, Any], jobs: list[dict[str, Any]]) -> dict[str, Any]:
    for job in jobs:
        if job.get("entityType") != target.get("entityType"):
            continue
        if job.get("entityId") != target.get("entityId"):
            continue
        if job.get("skinId") != target.get("skinId"):
            continue
        if target.get("bodyType") and job.get("bodyType") != target.get("bodyType"):
            continue
        return job
    raise RuntimeError(f"No frontend video job matched target: {target}")


def create_run_dir(root: Path, prefix: str) -> Path:
    stamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    run_dir = root / f"{prefix}_{stamp}"
    run_dir.mkdir(parents=True, exist_ok=False)
    return run_dir


def copy_reference_files(job: dict[str, Any], run_dir: Path) -> dict[str, Any]:
    refs_dir = run_dir / "References"
    refs_dir.mkdir(parents=True, exist_ok=True)
    copied: dict[str, Any] = {}
    for field in ("sourceImage", "poster"):
        relative = job.get(field)
        if not relative:
            copied[field] = {"status": "missing_field"}
            continue
        source = project_path(relative)
        if not source.exists():
            copied[field] = {"status": "missing_file", "source": relative}
            continue
        destination = refs_dir / f"{field}_{source.name}"
        shutil.copy2(source, destination)
        copied[field] = {
            "status": "copied",
            "source": relative,
            "localCopy": destination.relative_to(run_dir).as_posix(),
            "bytes": destination.stat().st_size,
        }
    return copied


def response_has_auth_failure(summary: dict[str, Any]) -> bool:
    status = int(summary.get("http_status") or 0)
    message = str(summary.get("message") or "").lower()
    return status in {401, 403} or "auth" in message or "forbidden" in message


def run_capability_checks(client: KlingClient, checks: list[dict[str, Any]], status_path: Path) -> list[dict[str, Any]]:
    results: list[dict[str, Any]] = []
    for check in checks:
        name = check["name"]
        method = check.get("method", "GET").upper()
        endpoint = check["endpoint"]
        started = time.time()
        try:
            if method == "GET":
                response = client.get(endpoint)
            else:
                response = client.post(endpoint, check.get("payload", {}))
            summary = summarize_response(response)
            result = {
                "event": "capability_check",
                "name": name,
                "method": method,
                "endpoint": endpoint,
                "seconds": round(time.time() - started, 2),
                "summary": summary,
            }
        except Exception as exc:
            result = {
                "event": "capability_check",
                "name": name,
                "method": method,
                "endpoint": endpoint,
                "seconds": round(time.time() - started, 2),
                "error": str(exc),
            }
        append_jsonl(status_path, result)
        results.append(result)
    return results


def first_task_id(response: dict[str, Any]) -> str | None:
    body = response.get("body")
    if not isinstance(body, dict):
        return None
    data = body.get("data")
    if not isinstance(data, dict):
        return None
    task_id = data.get("task_id")
    return str(task_id) if task_id else None


def task_result_video_url(response: dict[str, Any]) -> str | None:
    body = response.get("body")
    if not isinstance(body, dict):
        return None
    data = body.get("data")
    if not isinstance(data, dict):
        return None
    result = data.get("task_result")
    if not isinstance(result, dict):
        return None
    videos = result.get("videos")
    if not isinstance(videos, list) or not videos:
        return None
    first = videos[0]
    if not isinstance(first, dict):
        return None
    url = first.get("url")
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
        row = {
            "event": "poll",
            "task_id": task_id,
            "elapsed_seconds": round(elapsed, 2),
            "summary": summary,
        }
        append_jsonl(status_path, row)

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
    command = [
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
    ]
    completed = subprocess.run(command, check=False, capture_output=True, text=True)
    if completed.returncode != 0:
        return {"error": completed.stderr.strip()}
    return json.loads(completed.stdout)


def encode_runtime_candidate(source: Path, output: Path) -> dict[str, Any]:
    ffmpeg = find_ffmpeg()
    if not ffmpeg:
        return {"status": "skipped", "reason": "ffmpeg not found"}

    output.parent.mkdir(parents=True, exist_ok=True)
    command = [
        ffmpeg,
        "-nostdin",
        "-hide_banner",
        "-loglevel",
        "error",
        "-y",
        "-i",
        str(source),
        "-vf",
        "scale=712:680:force_original_aspect_ratio=increase,crop=712:680,fps=30,format=yuv420p,setsar=1",
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
    ]
    completed = subprocess.run(command, check=False, capture_output=True, text=True)
    if completed.returncode != 0:
        return {"status": "failed", "stderr": completed.stderr.strip()}
    return {"status": "encoded", "path": output.as_posix(), "bytes": output.stat().st_size}


def resolve_generation_prompt(manifest: dict[str, Any], args: argparse.Namespace) -> tuple[str, str]:
    if args.prompt_file:
        prompt_path = args.prompt_file if args.prompt_file.is_absolute() else PROJECT_ROOT / args.prompt_file
        return prompt_path.read_text(encoding="utf-8").strip(), prompt_path.as_posix()
    if args.prompt:
        return args.prompt, "cli"
    if manifest.get("smokePrompt"):
        return str(manifest["smokePrompt"]), "manifest.smokePrompt"
    return (
        "Locked camera T66 toon-shaded 3D character-selection preview. "
        "The hero stays centered with subtle breathing, a small weight shift, "
        "soft forest light movement, and no UI text."
    ), "default"


def submit_smoke_task(
    client: KlingClient,
    manifest: dict[str, Any],
    job: dict[str, Any],
    run_dir: Path,
    status_path: Path,
    args: argparse.Namespace,
) -> dict[str, Any]:
    image_source = args.source_image if args.source_image else project_path(job["poster"])
    if not image_source.is_absolute():
        image_source = PROJECT_ROOT / image_source
    if not image_source.exists():
        raise RuntimeError(f"Smoke source image does not exist: {image_source}")

    target_name = safe_name(job)
    generation_prompt, prompt_source = resolve_generation_prompt(manifest, args)
    payload = {
        "model_name": args.model_name or manifest.get("defaultModelName", "kling-v1"),
        "image": encode_image_base64(image_source),
        "prompt": generation_prompt,
        "negative_prompt": manifest.get("negativePrompt", DEFAULT_NEGATIVE_PROMPT),
        "duration": str(args.duration_seconds or manifest.get("defaultDurationSeconds", 5)),
        "mode": args.mode or manifest.get("defaultMode", "std"),
        "cfg_scale": float(manifest.get("defaultCfgScale", 0.5)),
        "external_task_id": f"t66_{target_name}_{int(time.time())}",
        "callback_url": None,
    }
    redacted_payload = dict(payload)
    redacted_payload["image"] = f"<base64 redacted length={len(payload['image'])}>"
    redacted_payload["source_image"] = image_source.as_posix()
    redacted_payload["prompt_source"] = prompt_source
    write_json(run_dir / "request_payload_image2video_smoke.redacted.json", redacted_payload)

    started = time.time()
    response = client.post("/videos/image2video", payload)
    summary = summarize_response(response)
    submit_row = {
        "event": "submit_smoke_task",
        "target": target_name,
        "seconds": round(time.time() - started, 2),
        "summary": summary,
    }
    append_jsonl(status_path, submit_row)
    write_json(run_dir / "submit_response_summary.json", submit_row)

    if not response.get("ok"):
        return {"status": "submit_failed", "summary": summary}

    task_id = first_task_id(response)
    if not task_id:
        return {"status": "submit_failed_missing_task_id", "summary": summary}

    final_response = poll_image2video(
        client=client,
        task_id=task_id,
        status_path=status_path,
        interval_seconds=args.poll_interval_seconds,
        timeout_seconds=args.poll_timeout_seconds,
    )
    final_summary = summarize_response(final_response)
    write_json(run_dir / "final_task_summary.json", final_summary)

    if str(final_summary.get("task_status") or "").lower() != "succeed":
        return {"status": "task_not_succeeded", "task_id": task_id, "summary": final_summary}

    video_url = task_result_video_url(final_response)
    if not video_url:
        return {"status": "task_succeeded_missing_video_url", "task_id": task_id, "summary": final_summary}

    raw_path = run_dir / "Raw" / f"{target_name}_native_kling_smoke.mp4"
    download_info = client.download(video_url, raw_path)
    append_jsonl(status_path, {"event": "download", "task_id": task_id, "download": download_info})

    raw_probe = probe_video(raw_path)
    if raw_probe is not None:
        write_json(run_dir / "probe_raw.json", raw_probe)

    encoded_path = run_dir / "Encoded" / f"{target_name}_native_kling_smoke_712x680.mp4"
    encode_info = encode_runtime_candidate(raw_path, encoded_path)
    append_jsonl(status_path, {"event": "encode_runtime_candidate", "task_id": task_id, "encode": encode_info})

    if encode_info.get("status") == "encoded":
        encoded_probe = probe_video(encoded_path)
        if encoded_probe is not None:
            write_json(run_dir / "probe_encoded.json", encoded_probe)

    return {
        "status": "succeeded",
        "task_id": task_id,
        "download": download_info,
        "raw_probe_written": raw_probe is not None,
        "encode": encode_info,
    }


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--manifest", type=Path, default=DEFAULT_MANIFEST)
    parser.add_argument("--access-file", type=Path, default=DEFAULT_ACCESS_FILE)
    parser.add_argument("--run-root", type=Path, default=RUNS_ROOT)
    parser.add_argument("--submit-smoke-task", action="store_true")
    parser.add_argument("--model-name", default="")
    parser.add_argument("--mode", default="")
    parser.add_argument("--duration-seconds", type=int, default=0)
    parser.add_argument("--poll-interval-seconds", type=int, default=10)
    parser.add_argument("--poll-timeout-seconds", type=int, default=600)
    parser.add_argument(
        "--source-image",
        type=Path,
        default=None,
        help="Optional image source override for the Kling image-to-video request.",
    )
    parser.add_argument("--prompt", default="", help="Optional prompt override for the Kling image-to-video request.")
    parser.add_argument(
        "--prompt-file",
        type=Path,
        default=None,
        help="Optional prompt file override for the Kling image-to-video request.",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    manifest = read_json(args.manifest)
    run_dir = create_run_dir(args.run_root, manifest.get("runNamePrefix", "KlingReferencePilot"))
    status_path = run_dir / "status.jsonl"

    try:
        client = KlingClient.from_local_access(args.access_file)
        jobs = load_frontend_jobs()
        smoke_target = find_job(manifest["smokeTarget"], jobs)
        reference_info = copy_reference_files(smoke_target, run_dir)
        write_json(run_dir / "run_config.json", {
            "manifest": args.manifest.as_posix(),
            "smokeTarget": manifest["smokeTarget"],
            "resolvedSmokeJob": {
                key: smoke_target.get(key)
                for key in ("entityType", "entityId", "displayName", "skinId", "bodyType", "movie", "poster", "sourceImage")
            },
            "references": reference_info,
            "submitSmokeTask": bool(args.submit_smoke_task),
            "sourceImageOverride": args.source_image.as_posix() if args.source_image else None,
            "promptOverride": args.prompt or None,
            "promptFileOverride": args.prompt_file.as_posix() if args.prompt_file else None,
        })

        capability_results = run_capability_checks(
            client,
            manifest.get("capabilityChecks", []),
            status_path,
        )
        write_json(run_dir / "capability_matrix.json", capability_results)

        summaries = [
            result.get("summary", {})
            for result in capability_results
            if isinstance(result.get("summary"), dict)
        ]
        auth_failures = [summary for summary in summaries if response_has_auth_failure(summary)]
        if auth_failures:
            write_json(run_dir / "result.json", {
                "status": "auth_or_permission_failed",
                "runDir": run_dir.as_posix(),
                "authFailures": auth_failures,
            })
            print(f"run_dir={run_dir}")
            print("status=auth_or_permission_failed")
            return 2

        if args.submit_smoke_task:
            smoke_result = submit_smoke_task(client, manifest, smoke_target, run_dir, status_path, args)
        else:
            smoke_result = {"status": "skipped_submit_smoke_task"}

        write_json(run_dir / "result.json", {
            "status": smoke_result.get("status"),
            "runDir": run_dir.as_posix(),
            "smokeResult": smoke_result,
        })
        print(f"run_dir={run_dir}")
        print(f"status={smoke_result.get('status')}")
        return 0 if smoke_result.get("status") not in {"submit_failed", "submit_failed_missing_task_id"} else 3
    except KlingClientError as exc:
        write_json(run_dir / "result.json", {"status": "client_error", "error": str(exc), "runDir": run_dir.as_posix()})
        print(f"run_dir={run_dir}")
        print(f"status=client_error error={exc}")
        return 1
    except Exception as exc:
        write_json(run_dir / "result.json", {"status": "failed", "error": str(exc), "runDir": run_dir.as_posix()})
        print(f"run_dir={run_dir}")
        print(f"status=failed error={exc}")
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
