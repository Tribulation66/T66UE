#!/usr/bin/env python3
"""Run a Pixal3D end-to-end smoke test against a RunPod pod.

The test creates four deterministic source plates, uploads them to the pod,
calls the Pixal3D /generate endpoint, downloads raw GLBs, runs Blender QA, and
optionally runs Quad Retro on character-like outputs.
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
PIXAL_ROOT = MODEL_ROOT / "Pixal3D"
SERVER_PATH = PIXAL_ROOT / "Server" / "pixal3d_server.py"
BOOTSTRAP_PATH = PIXAL_ROOT / "Scripts" / "bootstrap_pixal3d_pod.sh"
BLENDER_EXE = Path(r"C:\Program Files\Blender Foundation\Blender 5.1\blender.exe")
BLENDER_QA_SCRIPT = MODEL_ROOT / "Scripts" / "Core" / "Blender" / "blender_glb_qa.py"
QUAD_RETRO_WRAPPER = MODEL_ROOT / "Scripts" / "Core" / "QuadRetro" / "RunQuadRetroCharacterPipeline.ps1"

DEFAULT_RUN_ROOT = MODEL_ROOT / "Runs" / "Pixal3D" / "PipelineSmoke01"
DEFAULT_REMOTE_ROOT = "/workspace/T66/ModelGeneration/Runs/Pixal3D/PipelineSmoke01"

SOURCE_SPECS = [
    {
        "id": "tree_organic_prop",
        "kind": "tree",
        "display_name": "Tree organic prop",
        "quad_retro": False,
    },
    {
        "id": "stone_wall_module",
        "kind": "wall",
        "display_name": "Stone wall module",
        "quad_retro": False,
    },
    {
        "id": "humanoid_character",
        "kind": "character",
        "display_name": "Humanoid character",
        "quad_retro": True,
    },
    {
        "id": "horned_monster",
        "kind": "monster",
        "display_name": "Horned monster",
        "quad_retro": True,
    },
]


def now_iso() -> str:
    return datetime.now(timezone.utc).replace(microsecond=0).isoformat()


def rel(path: Path, root: Path) -> str:
    return path.resolve().relative_to(root.resolve()).as_posix()


def run_cmd(args: list[str], timeout: int | None = None, cwd: Path = REPO_ROOT) -> subprocess.CompletedProcess[str]:
    result = subprocess.run(
        args,
        cwd=cwd,
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


def ssh_args(args: argparse.Namespace, remote_command: str) -> list[str]:
    return [
        "ssh.exe",
        "-p",
        str(args.pod_port),
        "-i",
        str(args.key_path),
        "-o",
        "BatchMode=yes",
        "-o",
        "ConnectTimeout=20",
        "-o",
        "StrictHostKeyChecking=no",
        "-o",
        "UserKnownHostsFile=NUL",
        f"{args.user}@{args.pod_ip}",
        remote_command,
    ]


def bash_lc(script: str) -> str:
    return "bash -lc " + shlex.quote(script)


def scp_to_pod(args: argparse.Namespace, local_path: Path, remote_path: str, timeout: int = 240) -> None:
    run_cmd(
        [
            "scp.exe",
            "-P",
            str(args.pod_port),
            "-i",
            str(args.key_path),
            "-o",
            "BatchMode=yes",
            "-o",
            "StrictHostKeyChecking=no",
            "-o",
            "UserKnownHostsFile=NUL",
            str(local_path),
            f"{args.user}@{args.pod_ip}:{remote_path}",
        ],
        timeout=timeout,
    )


def scp_from_pod(args: argparse.Namespace, remote_path: str, local_path: Path, timeout: int = 240) -> None:
    local_path.parent.mkdir(parents=True, exist_ok=True)
    run_cmd(
        [
            "scp.exe",
            "-P",
            str(args.pod_port),
            "-i",
            str(args.key_path),
            "-o",
            "BatchMode=yes",
            "-o",
            "StrictHostKeyChecking=no",
            "-o",
            "UserKnownHostsFile=NUL",
            f"{args.user}@{args.pod_ip}:{remote_path}",
            str(local_path),
        ],
        timeout=timeout,
    )


def draw_tree(path: Path) -> None:
    image = Image.new("RGBA", (1024, 1024), (0, 0, 0, 0))
    draw = ImageDraw.Draw(image)
    draw.polygon([(455, 760), (560, 760), (545, 350), (475, 350)], fill=(117, 78, 45, 255), outline=(67, 42, 26, 255))
    draw.rectangle((488, 610, 530, 765), fill=(91, 58, 35, 255))
    for box, color in [
        ((330, 260, 690, 590), (45, 142, 70, 255)),
        ((250, 390, 520, 720), (55, 165, 78, 255)),
        ((500, 380, 780, 710), (37, 125, 68, 255)),
        ((385, 170, 640, 430), (70, 185, 92, 255)),
    ]:
        draw.ellipse(box, fill=color, outline=(24, 84, 42, 255), width=8)
    draw.ellipse((405, 245, 445, 285), fill=(150, 220, 122, 255))
    draw.ellipse((600, 470, 640, 510), fill=(145, 214, 110, 255))
    image.save(path)


def draw_wall(path: Path) -> None:
    image = Image.new("RGBA", (1024, 1024), (0, 0, 0, 0))
    draw = ImageDraw.Draw(image)
    x0, y0, x1, y1 = 180, 260, 845, 780
    draw.rounded_rectangle((x0, y0, x1, y1), radius=12, fill=(116, 117, 121, 255), outline=(40, 42, 47, 255), width=10)
    brick_h = 86
    for row in range(6):
        y = y0 + row * brick_h
        offset = 0 if row % 2 == 0 else 70
        draw.line((x0, y, x1, y), fill=(46, 48, 54, 255), width=7)
        x = x0 - offset
        while x < x1:
            draw.line((x, y, x, min(y + brick_h, y1)), fill=(51, 53, 59, 255), width=6)
            x += 140
    draw.line((x0, y1, x1, y1), fill=(46, 48, 54, 255), width=7)
    for crack in [((330, 330), (360, 390), (342, 448)), ((650, 410), (620, 470), (670, 535)), ((470, 620), (520, 650), (505, 720))]:
        draw.line(crack, fill=(30, 32, 36, 255), width=6)
    image.save(path)


def draw_character(path: Path) -> None:
    image = Image.new("RGBA", (1024, 1024), (0, 0, 0, 0))
    draw = ImageDraw.Draw(image)
    draw.ellipse((430, 130, 595, 295), fill=(229, 176, 122, 255), outline=(70, 42, 30, 255), width=7)
    draw.polygon([(390, 160), (510, 80), (635, 170), (590, 145), (548, 118), (480, 122)], fill=(78, 44, 31, 255))
    draw.rectangle((425, 305, 600, 570), fill=(46, 102, 176, 255), outline=(20, 42, 78, 255), width=8)
    draw.rectangle((445, 570, 508, 805), fill=(55, 65, 88, 255), outline=(22, 25, 35, 255), width=7)
    draw.rectangle((520, 570, 583, 805), fill=(55, 65, 88, 255), outline=(22, 25, 35, 255), width=7)
    draw.rectangle((390, 330, 435, 615), fill=(229, 176, 122, 255), outline=(70, 42, 30, 255), width=6)
    draw.rectangle((590, 330, 635, 615), fill=(229, 176, 122, 255), outline=(70, 42, 30, 255), width=6)
    draw.rectangle((395, 790, 515, 835), fill=(40, 32, 26, 255))
    draw.rectangle((515, 790, 635, 835), fill=(40, 32, 26, 255))
    draw.ellipse((470, 195, 492, 220), fill=(24, 24, 30, 255))
    draw.ellipse((538, 195, 560, 220), fill=(24, 24, 30, 255))
    draw.arc((474, 220, 562, 260), start=10, end=170, fill=(110, 55, 55, 255), width=5)
    image.save(path)


def draw_monster(path: Path) -> None:
    image = Image.new("RGBA", (1024, 1024), (0, 0, 0, 0))
    draw = ImageDraw.Draw(image)
    draw.ellipse((320, 230, 720, 700), fill=(126, 70, 160, 255), outline=(54, 24, 80, 255), width=10)
    draw.polygon([(390, 260), (330, 110), (470, 215)], fill=(214, 196, 126, 255), outline=(75, 65, 38, 255))
    draw.polygon([(650, 260), (700, 110), (560, 215)], fill=(214, 196, 126, 255), outline=(75, 65, 38, 255))
    draw.ellipse((410, 360, 480, 430), fill=(240, 237, 190, 255), outline=(46, 30, 56, 255), width=5)
    draw.ellipse((560, 360, 630, 430), fill=(240, 237, 190, 255), outline=(46, 30, 56, 255), width=5)
    draw.ellipse((438, 382, 462, 410), fill=(18, 14, 22, 255))
    draw.ellipse((588, 382, 612, 410), fill=(18, 14, 22, 255))
    draw.arc((430, 470, 620, 590), start=0, end=180, fill=(24, 16, 30, 255), width=12)
    for x in [455, 500, 545, 590]:
        draw.polygon([(x, 505), (x + 24, 505), (x + 12, 565)], fill=(237, 231, 200, 255), outline=(90, 78, 58, 255))
    draw.rectangle((370, 670, 440, 835), fill=(90, 48, 120, 255), outline=(54, 24, 80, 255), width=7)
    draw.rectangle((590, 670, 660, 835), fill=(90, 48, 120, 255), outline=(54, 24, 80, 255), width=7)
    draw.rectangle((235, 455, 350, 520), fill=(90, 48, 120, 255), outline=(54, 24, 80, 255), width=7)
    draw.rectangle((675, 455, 790, 520), fill=(90, 48, 120, 255), outline=(54, 24, 80, 255), width=7)
    image.save(path)


DRAWERS = {
    "tree": draw_tree,
    "wall": draw_wall,
    "character": draw_character,
    "monster": draw_monster,
}


def ensure_sources(run_root: Path) -> list[dict]:
    source_dir = run_root / "Inputs" / "SourceImages"
    source_dir.mkdir(parents=True, exist_ok=True)
    entries = []
    for spec in SOURCE_SPECS:
        path = source_dir / f"{spec['id']}.png"
        if not path.exists():
            DRAWERS[spec["kind"]](path)
        entries.append({**spec, "source_image": path})
    return entries


def write_manifest(run_root: Path, rows: list[dict]) -> None:
    reports = run_root / "Reports"
    reports.mkdir(parents=True, exist_ok=True)
    report_path = reports / "Pixal3D_PipelineSmoke01.json"
    merged_rows = {}
    if report_path.exists():
        try:
            existing = json.loads(report_path.read_text(encoding="utf-8"))
            for item in existing.get("rows", []):
                if "id" in item:
                    merged_rows[item["id"]] = item
        except Exception:
            merged_rows = {}

    for row in rows:
        item = dict(row)
        for key in ["source_image", "raw_pixal3d_glb", "qa_front_render", "qa_metadata", "quad_retro_glb", "quad_retro_report"]:
            if key in item and isinstance(item[key], Path):
                item[key] = rel(item[key], run_root)
        merged_rows[item["id"]] = {**merged_rows.get(item["id"], {}), **item}

    ordered_rows = []
    for spec in SOURCE_SPECS:
        row = merged_rows.get(spec["id"])
        if row is not None:
            ordered_rows.append(row)

    payload = {
        "batch": "Pixal3D_PipelineSmoke01",
        "updated_utc": now_iso(),
        "rows": ordered_rows,
    }
    report_path.write_text(
        json.dumps(payload, indent=2) + "\n",
        encoding="utf-8",
        newline="\n",
    )


def upload_support_files(args: argparse.Namespace) -> None:
    if not SERVER_PATH.exists():
        raise RuntimeError(f"Missing server file: {SERVER_PATH}")
    if not BOOTSTRAP_PATH.exists():
        raise RuntimeError(f"Missing bootstrap file: {BOOTSTRAP_PATH}")
    run_cmd(ssh_args(args, f"mkdir -p {shlex.quote(args.remote_repo_dir)} /tmp/t66_pixal3d"), timeout=60)
    scp_to_pod(args, SERVER_PATH, f"{args.remote_repo_dir}/pixal3d_server.py")
    scp_to_pod(args, BOOTSTRAP_PATH, "/tmp/t66_pixal3d/bootstrap_pixal3d_pod.sh")


def start_server(args: argparse.Namespace) -> None:
    pid_file = "/tmp/t66_pixal3d_server.pid"
    log_file = f"{args.remote_repo_dir}/pixal3d_server.log"
    script = "\n".join(
        [
            "set -eo pipefail",
            f"pid_file={shlex.quote(pid_file)}",
            f"log_file={shlex.quote(log_file)}",
            "if [[ -f \"$pid_file\" ]]; then",
            "  old_pid=$(cat \"$pid_file\" 2>/dev/null || true)",
            "  if [[ -n \"$old_pid\" ]] && kill -0 \"$old_pid\" 2>/dev/null; then",
            "    kill \"$old_pid\" 2>/dev/null || true",
            "    sleep 2",
            "  fi",
            "fi",
            "source /opt/conda/etc/profile.d/conda.sh",
            "conda activate pixal3d",
            f"export PYTHONPATH={shlex.quote(args.remote_repo_dir)}",
            f"export PIXAL3D_PORT={args.server_port}",
            f"export PIXAL3D_ATTN_BACKEND={shlex.quote(args.attention_backend)}",
            f"export PIXAL3D_SPARSE_ATTN_BACKEND={shlex.quote(args.attention_backend)}",
            f"export PIXAL3D_SPARSE_CONV_BACKEND={shlex.quote(args.sparse_conv_backend)}",
            f"export PIXAL3D_LOW_VRAM={'1' if args.low_vram else '0'}",
            f"cd {shlex.quote(args.remote_repo_dir)}",
            f"nohup python -u {shlex.quote(args.remote_repo_dir + '/pixal3d_server.py')} > \"$log_file\" 2>&1 < /dev/null &",
            "server_pid=$!",
            "echo \"$server_pid\" > \"$pid_file\"",
            "sleep 2",
            "if ! kill -0 \"$server_pid\" 2>/dev/null; then",
            "  tail -n 200 \"$log_file\" || true",
            "  exit 1",
            "fi",
            "printf 'PIXAL3D_SERVER_PID=%s\\n' \"$server_pid\"",
            "tail -n 20 \"$log_file\" || true",
        ]
    )
    result = run_cmd(ssh_args(args, bash_lc(script)), timeout=60)
    print(result.stdout.strip())


def wait_for_health(args: argparse.Namespace, timeout_seconds: int) -> str:
    deadline = time.monotonic() + timeout_seconds
    last = ""
    while time.monotonic() < deadline:
        result = subprocess.run(
            ssh_args(args, f"curl -sS --max-time 10 http://127.0.0.1:{args.server_port}/health"),
            cwd=REPO_ROOT,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
        )
        last = result.stdout.strip()
        if result.returncode == 0 and '"status":"ok"' in last.replace(" ", ""):
            return last
        time.sleep(10)
    raise RuntimeError(f"Pixal3D health check timed out. Last output:\n{last}")


def generation_attempt_plan(args: argparse.Namespace) -> list[dict]:
    attempts = [
        {
            "label": "requested",
            "decimation": args.decimation,
            "remesh": args.remesh,
        }
    ]
    if args.disable_generation_fallback:
        return attempts

    safe_decimation = min(args.decimation, args.fallback_decimation)
    seen = {(args.decimation, args.remesh)}
    for candidate in [
        {"label": "safe_decimation", "decimation": safe_decimation, "remesh": args.remesh},
        {"label": "safe_decimation_no_remesh", "decimation": safe_decimation, "remesh": False},
    ]:
        key = (candidate["decimation"], candidate["remesh"])
        if key not in seen:
            attempts.append(candidate)
            seen.add(key)
    return attempts


def generation_output_paths(args: argparse.Namespace, run_root: Path, row_id: str, attempt: dict) -> tuple[str, Path]:
    remesh_tag = "Rmx1" if attempt["remesh"] else "Rmx0"
    suffix = (
        f"{row_id}_Pixal3D_R{args.resolution}_S{args.seed}_"
        f"D{attempt['decimation']}_{remesh_tag}_{attempt['label']}.glb"
    )
    remote_glb = f"{args.remote_run_root}/Raw/Pixal3D/{row_id}/{suffix}"
    local_glb = run_root / "Raw" / "Pixal3D" / row_id / suffix
    return remote_glb, local_glb


def parse_generation_headers(stdout: str) -> dict:
    headers = {}
    for line in stdout.splitlines():
        if ":" not in line:
            continue
        key, value = line.split(":", 1)
        key = key.strip()
        if key.lower().startswith("x-pixal3d-"):
            headers[key] = value.strip()
    return headers


def run_generation_attempt(
    args: argparse.Namespace,
    row_id: str,
    remote_source: str,
    remote_glb: str,
    attempt: dict,
) -> tuple[subprocess.CompletedProcess[str], float]:
    print(
        f"PIXAL3D START {row_id} attempt={attempt['label']} resolution={args.resolution} "
        f"seed={args.seed} texture={args.texture_size} decimation={attempt['decimation']} "
        f"remesh={1 if attempt['remesh'] else 0}"
    )
    remote_command = " ".join(
        [
            "set -e;",
            "headers=$(mktemp);",
            f"curl --fail --show-error --silent --max-time {args.generate_timeout}",
            f"-D \"$headers\"",
            f"-X POST http://127.0.0.1:{args.server_port}/generate",
            "-H 'Content-Type: image/png'",
            f"-H 'X-Seed: {args.seed}'",
            f"-H 'X-Texture-Size: {args.texture_size}'",
            f"-H 'X-Decimation: {attempt['decimation']}'",
            f"-H 'X-Remesh: {1 if attempt['remesh'] else 0}'",
            f"-H 'X-Resolution: {args.resolution}'",
            f"-H 'X-SS-Guidance: {args.ss_guidance}'",
            f"-H 'X-SS-Steps: {args.ss_steps}'",
            f"-H 'X-Shape-Guidance: {args.shape_guidance}'",
            f"-H 'X-Shape-Steps: {args.shape_steps}'",
            f"-H 'X-Export-Fallback: {0 if args.disable_server_export_fallback else 1}'",
            f"-H 'X-Fallback-Decimation: {args.fallback_decimation}'",
            f"--data-binary @{shlex.quote(remote_source)}",
            f"-o {shlex.quote(remote_glb)};",
            "cat \"$headers\";",
            "rm -f \"$headers\";",
            f"printf '\\nPIXAL3D_REMOTE_SIZE=%s\\n' \"$(stat -c%s {shlex.quote(remote_glb)})\"",
        ]
    )
    start = time.monotonic()
    result = run_cmd(ssh_args(args, remote_command), timeout=args.generate_timeout + 120)
    return result, time.monotonic() - start


def run_generation(args: argparse.Namespace, run_root: Path, rows: list[dict]) -> None:
    print(f"PIXAL3D HEALTH {wait_for_health(args, args.health_timeout)}")
    for row in rows:
        source = Path(row["source_image"])
        row_id = row["id"]
        remote_source = f"{args.remote_run_root}/Inputs/SourceImages/{row_id}.png"
        attempts = generation_attempt_plan(args)
        requested_remote_glb, requested_local_glb = generation_output_paths(args, run_root, row_id, attempts[0])

        if requested_local_glb.exists() and requested_local_glb.stat().st_size > 0 and not args.force_generation:
            row["raw_pixal3d_glb"] = requested_local_glb
            row["raw_pixal3d_glb_size_bytes"] = requested_local_glb.stat().st_size
            row["generation_decimation"] = attempts[0]["decimation"]
            row["generation_remesh"] = attempts[0]["remesh"]
            print(f"PIXAL3D SKIP {row_id} existing")
            continue

        run_cmd(
            ssh_args(
                args,
                "mkdir -p "
                f"{shlex.quote(posixpath.dirname(remote_source))} "
                f"{shlex.quote(posixpath.dirname(requested_remote_glb))}",
            ),
            timeout=60,
        )
        scp_to_pod(args, source, remote_source)

        row["generation_attempts"] = []
        last_error = ""
        for attempt_index, attempt in enumerate(attempts, start=1):
            remote_glb, local_glb = generation_output_paths(args, run_root, row_id, attempt)
            try:
                result, duration = run_generation_attempt(args, row_id, remote_source, remote_glb, attempt)
                headers = parse_generation_headers(result.stdout)
                size_text = "0"
                for line in result.stdout.splitlines():
                    if line.startswith("PIXAL3D_REMOTE_SIZE="):
                        size_text = line.split("=", 1)[1].strip()
                scp_from_pod(args, remote_glb, local_glb)
                row["raw_pixal3d_glb"] = local_glb
                row["raw_pixal3d_glb_size_bytes"] = local_glb.stat().st_size
                row["generation_duration_seconds"] = round(duration, 1)
                row["generation_attempt_index"] = attempt_index
                row["generation_decimation"] = attempt["decimation"]
                row["generation_remesh"] = attempt["remesh"]
                row["generation_fallback_used"] = attempt_index > 1
                row["generation_server_export_headers"] = headers
                row["generation_attempts"].append(
                    {
                        **attempt,
                        "status": "ok",
                        "duration_seconds": round(duration, 1),
                        "pod_size_bytes": int(size_text) if size_text.isdigit() else size_text,
                        "local_size_bytes": local_glb.stat().st_size,
                        "server_export_headers": headers,
                    }
                )
                print(
                    f"PIXAL3D DONE {row_id} attempt={attempt['label']} pod_size={size_text} "
                    f"local_size={local_glb.stat().st_size} duration={duration:.1f}s"
                )
                break
            except Exception as exc:
                last_error = str(exc)
                row["generation_attempts"].append({**attempt, "status": "failed", "error": last_error[-4000:]})
                print(f"PIXAL3D ATTEMPT FAILED {row_id} attempt={attempt['label']}: {last_error[-1000:]}")
                if attempt_index < len(attempts) and args.restart_server_on_failure:
                    print("PIXAL3D RESTART server before fallback attempt")
                    start_server(args)
                    print(f"PIXAL3D HEALTH {wait_for_health(args, args.health_timeout)}")
        else:
            write_manifest(run_root, rows)
            raise RuntimeError(f"All Pixal3D generation attempts failed for {row_id}:\n{last_error}")
        write_manifest(run_root, rows)


def run_blender_qa(args: argparse.Namespace, run_root: Path, rows: list[dict]) -> None:
    if not BLENDER_EXE.exists():
        raise RuntimeError(f"Blender executable missing: {BLENDER_EXE}")
    for row in rows:
        row_id = row["id"]
        glb = Path(row.get("raw_pixal3d_glb", ""))
        if not glb.exists():
            print(f"QA SKIP {row_id} missing GLB")
            continue
        render = run_root / "QA" / "Pixal3DFront" / f"{row_id}_front.png"
        metadata = run_root / "QA" / "Pixal3DFront" / f"{row_id}_front_metadata.json"
        row["qa_front_render"] = render
        row["qa_metadata"] = metadata
        if render.exists() and metadata.exists() and not args.force_qa:
            try:
                meta = json.loads(metadata.read_text(encoding="utf-8"))
                row["qa_raw_triangles"] = meta.get("raw_triangles")
                row["qa_bounds"] = meta.get("bounds")
            except Exception:
                pass
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
                str(args.qa_resolution),
            ],
            timeout=args.qa_timeout,
        )
        try:
            meta = json.loads(metadata.read_text(encoding="utf-8"))
            row["qa_raw_triangles"] = meta.get("raw_triangles")
            row["qa_bounds"] = meta.get("bounds")
        except Exception:
            pass
        print(f"QA DONE {row_id} -> {render}")
        write_manifest(run_root, rows)


def run_quad_retro(args: argparse.Namespace, run_root: Path, rows: list[dict]) -> None:
    if not QUAD_RETRO_WRAPPER.exists():
        raise RuntimeError(f"Quad Retro wrapper missing: {QUAD_RETRO_WRAPPER}")
    for row in rows:
        if not row.get("quad_retro"):
            continue
        row_id = row["id"]
        glb = Path(row.get("raw_pixal3d_glb", ""))
        if not glb.exists():
            print(f"QUAD RETRO SKIP {row_id} missing GLB")
            continue
        out_dir = run_root / "Post" / "QuadRetro" / row_id
        expected_glb = out_dir / "Models" / f"{row_id}_QuadRetro.glb"
        expected_report = out_dir / "Reports" / f"{row_id}_QuadRetro_report.json"
        row["quad_retro_glb"] = expected_glb
        row["quad_retro_report"] = expected_report
        if expected_glb.exists() and expected_report.exists() and not args.force_quad_retro:
            print(f"QUAD RETRO SKIP {row_id} existing")
            continue
        print(f"QUAD RETRO START {row_id}")
        run_cmd(
            [
                "powershell.exe",
                "-ExecutionPolicy",
                "Bypass",
                "-File",
                str(QUAD_RETRO_WRAPPER),
                "-InputModel",
                str(glb),
                "-OutputDir",
                str(out_dir),
                "-Label",
                row_id,
                "-TargetQuads",
                str(args.target_quads),
                "-TextureSize",
                str(args.retro_texture_size),
                "-PaletteMode",
                "none",
                "-DitherType",
                "none",
                "-DitherStrength",
                "0",
                "-RenderQA",
                "true",
                "-Background",
                "false",
                "-TimeoutSeconds",
                str(args.quad_retro_timeout),
            ],
            timeout=args.quad_retro_timeout,
        )
        print(f"QUAD RETRO DONE {row_id} -> {expected_glb}")
        write_manifest(run_root, rows)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--pod-ip", required=True)
    parser.add_argument("--pod-port", required=True, type=int)
    parser.add_argument("--user", default="root")
    parser.add_argument("--key-path", type=Path, default=Path.home() / ".ssh" / "id_ed25519")
    parser.add_argument("--run-root", type=Path, default=DEFAULT_RUN_ROOT)
    parser.add_argument("--remote-run-root", default=DEFAULT_REMOTE_ROOT)
    parser.add_argument("--remote-repo-dir", default="/workspace/Pixal3D")
    parser.add_argument("--server-port", type=int, default=18001)
    parser.add_argument("--attention-backend", default="flash_attn_3")
    parser.add_argument("--sparse-conv-backend", default="flex_gemm")
    parser.add_argument("--low-vram", action="store_true")
    parser.add_argument("--upload-support", action="store_true")
    parser.add_argument("--start-server", action="store_true")
    parser.add_argument("--skip-generation", action="store_true")
    parser.add_argument("--skip-qa", action="store_true")
    parser.add_argument("--run-quad-retro", action="store_true")
    parser.add_argument("--force-generation", action="store_true")
    parser.add_argument("--force-qa", action="store_true")
    parser.add_argument("--force-quad-retro", action="store_true")
    parser.add_argument("--resolution", type=int, default=1024, choices=(1024, 1536))
    parser.add_argument("--seed", type=int, default=1337)
    parser.add_argument("--texture-size", type=int, default=2048)
    parser.add_argument("--decimation", type=int, default=80000)
    parser.add_argument("--fallback-decimation", type=int, default=30000)
    parser.add_argument("--remesh", dest="remesh", action="store_true", default=True)
    parser.add_argument("--no-remesh", dest="remesh", action="store_false")
    parser.add_argument("--disable-generation-fallback", action="store_true")
    parser.add_argument("--disable-server-export-fallback", action="store_true")
    parser.add_argument("--restart-server-on-failure", dest="restart_server_on_failure", action="store_true", default=True)
    parser.add_argument("--no-restart-server-on-failure", dest="restart_server_on_failure", action="store_false")
    parser.add_argument("--ss-guidance", type=float, default=7.5)
    parser.add_argument("--ss-steps", type=int, default=12)
    parser.add_argument("--shape-guidance", type=float, default=7.5)
    parser.add_argument("--shape-steps", type=int, default=12)
    parser.add_argument("--health-timeout", type=int, default=2400)
    parser.add_argument("--generate-timeout", type=int, default=3600)
    parser.add_argument("--qa-timeout", type=int, default=600)
    parser.add_argument("--qa-resolution", type=int, default=1024)
    parser.add_argument("--quad-retro-timeout", type=int, default=1800)
    parser.add_argument("--target-quads", type=int, default=12000)
    parser.add_argument("--retro-texture-size", type=int, default=512)
    parser.add_argument("--only", nargs="*", default=[])
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    run_root = args.run_root
    run_root.mkdir(parents=True, exist_ok=True)
    rows = ensure_sources(run_root)
    if args.only:
        only = set(args.only)
        rows = [row for row in rows if row["id"] in only]
        missing = only - {row["id"] for row in rows}
        if missing:
            raise SystemExit(f"Unknown source IDs: {', '.join(sorted(missing))}")
    write_manifest(run_root, rows)

    if args.upload_support:
        upload_support_files(args)
    if args.start_server:
        start_server(args)
    if not args.skip_generation:
        run_generation(args, run_root, rows)
    if not args.skip_qa:
        run_blender_qa(args, run_root, rows)
    if args.run_quad_retro:
        run_quad_retro(args, run_root, rows)

    write_manifest(run_root, rows)
    print(json.dumps({"run_root": str(run_root), "rows": len(rows)}, indent=2))


if __name__ == "__main__":
    main()
