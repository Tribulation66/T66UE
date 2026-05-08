#!/usr/bin/env python3
"""
Run Trellis and front QA renders for WorldNpcInteractablesRetroBatch01.

This driver consumes Reports/Stage01_SourceAndTrellisManifest.json. It only
operates on manifest entries that need Trellis unless --include-existing is
passed for QA rendering of reused Batch01 raw GLBs.
"""

from __future__ import annotations

import argparse
import json
import posixpath
import shlex
import subprocess
import time
from datetime import datetime, timezone
from pathlib import Path

from PIL import Image, ImageDraw


REPO_ROOT = Path(r"C:\UE\T66")
MODEL_ROOT = REPO_ROOT / "Model Generation"
RUN_ROOT = MODEL_ROOT / "Runs" / "Interactables" / "WorldNpcInteractablesRetroBatch01"
REPORT_PATH = RUN_ROOT / "Reports" / "Stage01_SourceAndTrellisManifest.json"
TRELLIS_LOG_PATH = RUN_ROOT / "Notes" / "stage01_trellis_generation.log"

POD_IP = "69.30.85.78"
POD_PORT = "22127"
SSH_KEY = Path.home() / ".ssh" / "id_ed25519"
POD_ROOT = "/workspace/T66/ModelGeneration/Runs/Interactables/WorldNpcInteractablesRetroBatch01"

BLENDER_EXE = Path(r"C:\Program Files\Blender Foundation\Blender 5.1\blender.exe")
BLENDER_QA_SCRIPT = MODEL_ROOT / "Scripts" / "blender_glb_qa.py"

DEFAULT_SEED = 1337
DEFAULT_TEXTURE_SIZE = 2048
DEFAULT_DECIMATION = 80000


def now_iso() -> str:
    return datetime.now(timezone.utc).replace(microsecond=0).isoformat()


def abs_path(rel_or_abs: str) -> Path:
    path = Path(rel_or_abs)
    return path if path.is_absolute() else REPO_ROOT / path


def rel(path: Path) -> str:
    return str(path.resolve().relative_to(REPO_ROOT)).replace("\\", "/")


def run_cmd(args: list[str], timeout: int) -> subprocess.CompletedProcess[str]:
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
        "ConnectTimeout=20",
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
        timeout=240,
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
        timeout=240,
    )


def health_check() -> str:
    result = run_cmd(
        ssh_args("curl -sS --max-time 10 http://127.0.0.1:8000/health"),
        timeout=45,
    )
    return result.stdout.strip()


def load_report() -> dict:
    with REPORT_PATH.open("r", encoding="utf-8") as handle:
        return json.load(handle)


def save_report(report: dict) -> None:
    report["updated_utc"] = now_iso()
    for entry in report["entries"]:
        source = abs_path(entry["source_image"])
        raw = abs_path(entry["raw_trellis_glb"])
        qa = abs_path(entry["trellis_front_render"])
        entry["source_image_exists"] = source.exists()
        entry["raw_trellis_glb_exists"] = raw.exists()
        entry["trellis_front_render_exists"] = qa.exists()
        entry["raw_trellis_glb_size_bytes"] = raw.stat().st_size if raw.exists() else 0
        if raw.exists() and qa.exists():
            entry["trellis_status"] = "ready_with_front_qa"
        elif raw.exists():
            entry["trellis_status"] = "raw_trellis_ready"
        elif entry.get("needs_trellis"):
            entry["trellis_status"] = "pending_trellis"

    report["summary"] = {
        "total_stage_entries": len(report["entries"]),
        "source_ready": sum(1 for item in report["entries"] if abs_path(item["source_image"]).exists()),
        "raw_ready": sum(1 for item in report["entries"] if abs_path(item["raw_trellis_glb"]).exists()),
        "front_qa_ready": sum(1 for item in report["entries"] if abs_path(item["trellis_front_render"]).exists()),
        "pending_trellis": sum(1 for item in report["entries"] if item.get("trellis_status") == "pending_trellis"),
    }
    REPORT_PATH.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8", newline="\n")


def make_contact_sheet(report: dict) -> None:
    entries = [item for item in report["entries"] if abs_path(item["trellis_front_render"]).exists()]
    if not entries:
        return
    thumb = 210
    label_h = 34
    cols = 5
    rows = (len(entries) + cols - 1) // cols
    sheet = Image.new("RGB", (cols * thumb, rows * (thumb + label_h)), "white")
    draw = ImageDraw.Draw(sheet)
    for index, entry in enumerate(entries):
        image = Image.open(abs_path(entry["trellis_front_render"])).convert("RGB")
        image.thumbnail((thumb - 12, thumb - 12), Image.Resampling.LANCZOS)
        x0 = (index % cols) * thumb
        y0 = (index // cols) * (thumb + label_h)
        sheet.paste(image, (x0 + (thumb - image.width) // 2, y0 + 4))
        draw.text((x0 + 6, y0 + thumb + 4), entry["row_id"][:26], fill=(0, 0, 0))
    out = RUN_ROOT / "QA" / "TrellisFrontContactSheet.png"
    out.parent.mkdir(parents=True, exist_ok=True)
    sheet.save(out)


def selected_entries(report: dict, only: set[str], include_existing: bool) -> list[dict]:
    entries = []
    for entry in report["entries"]:
        if only and entry["row_id"] not in only:
            continue
        if entry.get("needs_trellis") or include_existing:
            entries.append(entry)
    return entries


def run_trellis(report: dict, only: set[str], force: bool) -> None:
    print(f"TRELLIS HEALTH {health_check()}")
    TRELLIS_LOG_PATH.parent.mkdir(parents=True, exist_ok=True)
    with TRELLIS_LOG_PATH.open("a", encoding="utf-8") as log:
        for entry in selected_entries(report, only, include_existing=False):
            row_id = entry["row_id"]
            source = abs_path(entry["source_image"])
            local_glb = abs_path(entry["raw_trellis_glb"])
            if not source.exists():
                print(f"TRELLIS SKIP {row_id} missing source image")
                continue
            if local_glb.exists() and local_glb.stat().st_size > 0 and not force:
                print(f"TRELLIS SKIP {row_id} existing")
                continue

            remote_source = f"{POD_ROOT}/Inputs/SourceImages/{entry['category']}/{row_id}.png"
            remote_glb = f"{POD_ROOT}/Raw/Trellis/{entry['category']}/{row_id}/{row_id}_Trellis.glb"
            run_cmd(
                ssh_args(
                    "mkdir -p "
                    f"{shlex.quote(posixpath.dirname(remote_source))} "
                    f"{shlex.quote(posixpath.dirname(remote_glb))}"
                ),
                timeout=120,
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
            pod_size = result.stdout.strip().splitlines()[-1] if result.stdout.strip() else "0"
            scp_from_pod(remote_glb, local_glb)
            print(f"TRELLIS DONE {row_id} size={local_glb.stat().st_size} duration={duration:.1f}s")
            log.write(
                f"[{now_iso()}] DONE {row_id} pod_size={pod_size} "
                f"local_size={local_glb.stat().st_size} duration={duration:.1f}s\n"
            )
            log.flush()
            save_report(report)


def render_qa(report: dict, only: set[str], force: bool, include_existing: bool) -> None:
    if not BLENDER_EXE.exists():
        raise RuntimeError(f"Blender executable not found: {BLENDER_EXE}")
    if not BLENDER_QA_SCRIPT.exists():
        raise RuntimeError(f"Blender QA script not found: {BLENDER_QA_SCRIPT}")

    for entry in selected_entries(report, only, include_existing=include_existing):
        row_id = entry["row_id"]
        glb = abs_path(entry["raw_trellis_glb"])
        qa = abs_path(entry["trellis_front_render"])
        metadata = qa.with_name(qa.stem + "_metadata.json")
        if not glb.exists():
            print(f"QA SKIP {row_id} missing GLB")
            continue
        if qa.exists() and metadata.exists() and not force:
            print(f"QA SKIP {row_id} existing")
            continue
        qa.parent.mkdir(parents=True, exist_ok=True)
        print(f"QA START {row_id}")
        try:
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
                    str(qa),
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
        except RuntimeError:
            if qa.exists() and metadata.exists():
                print(f"QA WARN {row_id} blender exited nonzero after writing render and metadata")
            else:
                raise
        print(f"QA DONE {row_id} -> {qa}")
        save_report(report)
    make_contact_sheet(report)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--run-trellis", action="store_true")
    parser.add_argument("--render-qa", action="store_true")
    parser.add_argument("--refresh", action="store_true")
    parser.add_argument("--include-existing", action="store_true")
    parser.add_argument("--force-trellis", action="store_true")
    parser.add_argument("--force-qa", action="store_true")
    parser.add_argument("--only", nargs="*", default=[])
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    report = load_report()
    only = set(args.only)
    if args.run_trellis:
        run_trellis(report, only, args.force_trellis)
    if args.render_qa:
        render_qa(report, only, args.force_qa, args.include_existing)
    if args.refresh or not (args.run_trellis or args.render_qa):
        save_report(report)
        make_contact_sheet(report)
    print(json.dumps(load_report()["summary"], indent=2))


if __name__ == "__main__":
    main()
