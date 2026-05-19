#!/usr/bin/env python3
"""
Run Pixal3D batches on a RunPod without holding one long SSH session open.

The runner uploads source PNGs, launches a detached remote Python job, then
polls status JSONL / DONE sentinel files through short SSH calls. This keeps
Codex and PowerShell aware of completion even when individual generations take
several minutes.
"""

from __future__ import annotations

import argparse
import json
import posixpath
import shlex
import subprocess
import sys
import tempfile
import time
from datetime import datetime, timezone
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[3]
DEFAULT_KEY_PATH = Path.home() / ".ssh" / "id_ed25519"
DEFAULT_POD_IP = "69.30.85.50"
DEFAULT_POD_PORT = 22134
DEFAULT_POD_USER = "root"
DEFAULT_SERVER_PORT = 18001
DEFAULT_REMOTE_ROOT = "/workspace/T66/ModelGeneration/Pixal3D_Batches"


REMOTE_JOB_CODE = r'''#!/usr/bin/env python3
from __future__ import annotations

import json
import os
import sys
import time
import traceback
import urllib.error
import urllib.request
from datetime import datetime, timezone


def utc_now():
    return datetime.now(timezone.utc).isoformat()


def write_json(path, payload):
    with open(path, "w", encoding="utf-8") as handle:
        json.dump(payload, handle, indent=2, sort_keys=True)
        handle.write("\n")


def append_jsonl(path, payload):
    with open(path, "a", encoding="utf-8") as handle:
        handle.write(json.dumps(payload, sort_keys=True) + "\n")


def read_bytes(path):
    with open(path, "rb") as handle:
        return handle.read()


def main():
    if len(sys.argv) != 2:
        print("usage: pixal3d_detached_job.py <manifest.json>", file=sys.stderr)
        return 2

    manifest_path = sys.argv[1]
    manifest = json.load(open(manifest_path, "r", encoding="utf-8"))
    status_path = manifest["status_path"]
    done_path = manifest["done_path"]
    server_url = manifest["server_url"]
    headers = manifest["headers"]
    timeout_seconds = int(manifest["generate_timeout_seconds"])

    os.makedirs(os.path.dirname(status_path), exist_ok=True)
    os.makedirs(os.path.dirname(done_path), exist_ok=True)
    open(status_path, "w", encoding="utf-8").close()

    exit_code = 0
    started_at = utc_now()

    for row in manifest["rows"]:
        started = time.time()
        payload = {
            "variant": row["variant"],
            "source_path": row["source_path"],
            "output_path": row["output_path"],
            "started_at": utc_now(),
        }

        try:
            request_headers = {"Content-Type": "image/png"}
            request_headers.update(headers)
            request = urllib.request.Request(
                server_url,
                data=read_bytes(row["source_path"]),
                headers=request_headers,
                method="POST",
            )
            with urllib.request.urlopen(request, timeout=timeout_seconds) as response:
                body = response.read()
                response_headers = dict(response.headers.items())
                status_code = int(response.status)

            os.makedirs(os.path.dirname(row["output_path"]), exist_ok=True)
            with open(row["output_path"], "wb") as handle:
                handle.write(body)

            payload.update(
                {
                    "ok": status_code == 200,
                    "status": status_code,
                    "bytes": os.path.getsize(row["output_path"]),
                    "response_headers": response_headers,
                }
            )
            if status_code != 200:
                exit_code = 1

        except urllib.error.HTTPError as exc:
            exit_code = 1
            error_body = exc.read()
            error_path = row["error_path"]
            os.makedirs(os.path.dirname(error_path), exist_ok=True)
            with open(error_path, "wb") as handle:
                handle.write(error_body)
            payload.update(
                {
                    "ok": False,
                    "status": int(exc.code),
                    "error_path": error_path,
                    "error": error_body[:4000].decode("utf-8", errors="replace"),
                    "response_headers": dict(exc.headers.items()),
                }
            )

        except Exception as exc:
            exit_code = 1
            error_path = row["error_path"]
            os.makedirs(os.path.dirname(error_path), exist_ok=True)
            tb = traceback.format_exc()
            with open(error_path, "w", encoding="utf-8") as handle:
                handle.write(tb)
            payload.update(
                {
                    "ok": False,
                    "status": "exception",
                    "error_path": error_path,
                    "error": str(exc),
                    "traceback_tail": tb[-4000:],
                }
            )

        payload["duration_seconds"] = round(time.time() - started, 1)
        payload["finished_at"] = utc_now()
        append_jsonl(status_path, payload)

    write_json(
        done_path,
        {
            "ok": exit_code == 0,
            "exit_code": exit_code,
            "started_at": started_at,
            "finished_at": utc_now(),
            "status_path": status_path,
            "manifest_path": manifest_path,
        },
    )
    return exit_code


if __name__ == "__main__":
    raise SystemExit(main())
'''


def utc_now() -> str:
    return datetime.now(timezone.utc).isoformat()


def run_local(command: list[str], timeout: int | None = None) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        command,
        cwd=REPO_ROOT,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        timeout=timeout,
        check=False,
    )


def require_success(result: subprocess.CompletedProcess[str], command_label: str) -> None:
    if result.returncode == 0:
        return
    sys.stderr.write(result.stdout)
    sys.stderr.write(result.stderr)
    raise SystemExit(f"{command_label} failed with exit code {result.returncode}")


def ssh_base(args: argparse.Namespace) -> list[str]:
    return [
        "ssh",
        "-p",
        str(args.pod_port),
        "-i",
        str(args.key_path),
        "-o",
        "BatchMode=yes",
        "-o",
        "StrictHostKeyChecking=no",
        "-o",
        "UserKnownHostsFile=NUL",
        f"{args.pod_user}@{args.pod_ip}",
    ]


def scp_base(args: argparse.Namespace) -> list[str]:
    return [
        "scp",
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
    ]


def ssh(args: argparse.Namespace, remote_command: str, timeout: int | None = None) -> subprocess.CompletedProcess[str]:
    return run_local(ssh_base(args) + [remote_command], timeout=timeout)


def scp_to_remote(args: argparse.Namespace, local_path: Path, remote_path: str, timeout: int | None = None) -> None:
    command = scp_base(args) + [str(local_path), f"{args.pod_user}@{args.pod_ip}:{remote_path}"]
    require_success(run_local(command, timeout=timeout), f"scp upload {local_path.name}")


def scp_from_remote(args: argparse.Namespace, remote_path: str, local_path: Path, timeout: int | None = None) -> None:
    local_path.parent.mkdir(parents=True, exist_ok=True)
    command = scp_base(args) + [f"{args.pod_user}@{args.pod_ip}:{remote_path}", str(local_path)]
    require_success(run_local(command, timeout=timeout), f"scp download {posixpath.basename(remote_path)}")


def bash_lc(script: str) -> str:
    return "bash -lc " + shlex.quote(script)


def remote_paths(args: argparse.Namespace) -> dict[str, str]:
    root = args.remote_run_root.rstrip("/")
    return {
        "root": root,
        "sources": posixpath.join(root, "Sources"),
        "outputs": posixpath.join(root, "Outputs"),
        "logs": posixpath.join(root, "Logs"),
        "manifest": posixpath.join(root, "manifest.json"),
        "job": posixpath.join(root, "pixal3d_detached_job.py"),
        "status": posixpath.join(root, "Logs", "pixal3d_generation_status.jsonl"),
        "done": posixpath.join(root, "Logs", "DONE.json"),
        "pid": posixpath.join(root, "Logs", "pixal3d_generation.pid"),
        "stdout": posixpath.join(root, "Logs", "pixal3d_generation_stdout.log"),
    }


def collect_sources(sources_dir: Path) -> list[Path]:
    sources = sorted(path for path in sources_dir.iterdir() if path.suffix.lower() == ".png")
    if not sources:
        raise SystemExit(f"No PNG sources found in {sources_dir}")
    return sources


def pixal_headers(args: argparse.Namespace) -> dict[str, str]:
    return {
        "X-Seed": str(args.seed),
        "X-Resolution": str(args.resolution),
        "X-Texture-Size": str(args.texture_size),
        "X-Decimation": str(args.decimation),
        "X-Remesh": "1" if args.remesh else "0",
        "X-Remesh-Band": str(args.remesh_band),
        "X-Remesh-Project": str(args.remesh_project),
        "X-Extend-Pixel": str(args.extend_pixel),
        "X-Image-Resolution": str(args.image_resolution),
        "X-Max-Num-Tokens": str(args.max_num_tokens),
        "X-Mesh-Scale": str(args.mesh_scale),
        "X-SS-Steps": str(args.ss_steps),
        "X-SS-Guidance": str(args.ss_guidance),
        "X-SS-Guidance-Rescale": str(args.ss_guidance_rescale),
        "X-SS-Rescale-T": str(args.ss_rescale_t),
        "X-Shape-Steps": str(args.shape_steps),
        "X-Shape-Guidance": str(args.shape_guidance),
        "X-Shape-Guidance-Rescale": str(args.shape_guidance_rescale),
        "X-Shape-Rescale-T": str(args.shape_rescale_t),
        "X-Tex-Steps": str(args.tex_steps),
        "X-Tex-Guidance": str(args.tex_guidance),
        "X-Tex-Guidance-Rescale": str(args.tex_guidance_rescale),
        "X-Tex-Rescale-T": str(args.tex_rescale_t),
        "X-Export-Fallback": "1" if args.export_fallback else "0",
        "X-Fallback-Decimation": str(args.fallback_decimation),
        "X-Safe-Fill-Holes-Fallback": "1" if args.safe_fill_holes_fallback else "0",
    }


def build_manifest(args: argparse.Namespace, sources: list[Path]) -> dict[str, object]:
    paths = remote_paths(args)
    rows = []
    for source in sources:
        variant = source.stem
        rows.append(
            {
                "variant": variant,
                "source_path": posixpath.join(paths["sources"], source.name),
                "output_path": posixpath.join(paths["outputs"], variant + ".glb"),
                "error_path": posixpath.join(paths["logs"], variant + "_error.txt"),
            }
        )

    return {
        "created_at": utc_now(),
        "server_url": f"http://127.0.0.1:{args.server_port}/generate",
        "generate_timeout_seconds": args.generate_timeout,
        "headers": pixal_headers(args),
        "status_path": paths["status"],
        "done_path": paths["done"],
        "rows": rows,
    }


def write_temp_json(payload: dict[str, object]) -> Path:
    handle = tempfile.NamedTemporaryFile("w", encoding="utf-8", suffix=".json", delete=False)
    with handle:
        json.dump(payload, handle, indent=2, sort_keys=True)
        handle.write("\n")
    return Path(handle.name)


def write_temp_text(text: str, suffix: str) -> Path:
    handle = tempfile.NamedTemporaryFile("w", encoding="utf-8", suffix=suffix, delete=False, newline="\n")
    with handle:
        handle.write(text)
    return Path(handle.name)


def upload_launch_files(args: argparse.Namespace, manifest: dict[str, object], sources: list[Path]) -> None:
    paths = remote_paths(args)
    mkdir_script = "set -euo pipefail\nmkdir -p {0} {1} {2}".format(
        shlex.quote(paths["sources"]),
        shlex.quote(paths["outputs"]),
        shlex.quote(paths["logs"]),
    )
    require_success(ssh(args, bash_lc(mkdir_script), timeout=30), "remote mkdir")

    for source in sources:
        scp_to_remote(args, source, posixpath.join(paths["sources"], source.name), timeout=120)

    manifest_path = write_temp_json(manifest)
    job_path = write_temp_text(REMOTE_JOB_CODE, ".py")
    try:
        scp_to_remote(args, manifest_path, paths["manifest"], timeout=60)
        scp_to_remote(args, job_path, paths["job"], timeout=60)
    finally:
        manifest_path.unlink(missing_ok=True)
        job_path.unlink(missing_ok=True)


def launch(args: argparse.Namespace) -> None:
    sources = collect_sources(args.sources_dir)
    manifest = build_manifest(args, sources)
    paths = remote_paths(args)
    upload_launch_files(args, manifest, sources)

    script = "\n".join(
        [
            "set -euo pipefail",
            f"rm -f {shlex.quote(paths['status'])} {shlex.quote(paths['done'])} {shlex.quote(paths['stdout'])}",
            f"nohup python3 -u {shlex.quote(paths['job'])} {shlex.quote(paths['manifest'])} > {shlex.quote(paths['stdout'])} 2>&1 < /dev/null &",
            "pid=$!",
            f"echo $pid > {shlex.quote(paths['pid'])}",
            f"printf '%s\\n' \"$pid\"",
        ]
    )
    result = ssh(args, bash_lc(script), timeout=30)
    require_success(result, "remote launch")
    print(f"Launched Pixal3D detached batch on {args.pod_ip}; pid={result.stdout.strip()}")
    print(f"Remote status: {paths['status']}")
    print(f"Remote done:   {paths['done']}")


def read_remote_text(args: argparse.Namespace, remote_path: str) -> str:
    result = ssh(args, bash_lc(f"test -f {shlex.quote(remote_path)} && cat {shlex.quote(remote_path)} || true"), timeout=30)
    require_success(result, f"read remote {posixpath.basename(remote_path)}")
    return result.stdout


def parse_status(text: str) -> list[dict[str, object]]:
    rows = []
    for line in text.splitlines():
        line = line.strip()
        if not line:
            continue
        try:
            rows.append(json.loads(line))
        except json.JSONDecodeError:
            rows.append({"raw": line, "ok": False})
    return rows


def poll(args: argparse.Namespace) -> bool:
    paths = remote_paths(args)
    status_text = read_remote_text(args, paths["status"])
    rows = parse_status(status_text)
    done_text = read_remote_text(args, paths["done"])
    done = json.loads(done_text) if done_text.strip() else None

    print(f"Completed {len(rows)} item(s).")
    for row in rows:
        variant = row.get("variant", "<unknown>")
        ok = row.get("ok")
        duration = row.get("duration_seconds", "?")
        byte_count = row.get("bytes", 0)
        print(f"- {variant}: ok={ok} duration={duration}s bytes={byte_count}")

    if done:
        print(f"DONE ok={done.get('ok')} exit_code={done.get('exit_code')}")
        return True
    print("DONE sentinel not present yet.")
    return False


def download(args: argparse.Namespace) -> None:
    paths = remote_paths(args)
    args.outputs_dir.mkdir(parents=True, exist_ok=True)
    args.logs_dir.mkdir(parents=True, exist_ok=True)

    list_script = "\n".join(
        [
            "set -euo pipefail",
            f"find {shlex.quote(paths['outputs'])} -maxdepth 1 -type f -name '*.glb' -printf '%f\\n' | sort",
        ]
    )
    result = ssh(args, bash_lc(list_script), timeout=30)
    require_success(result, "list remote outputs")
    output_names = [line.strip() for line in result.stdout.splitlines() if line.strip()]
    if not output_names:
        raise SystemExit(f"No remote GLB outputs found in {paths['outputs']}")

    for name in output_names:
        scp_from_remote(args, posixpath.join(paths["outputs"], name), args.outputs_dir / name, timeout=240)

    for name in ["pixal3d_generation_status.jsonl", "DONE.json", "pixal3d_generation.pid", "pixal3d_generation_stdout.log"]:
        remote_file = posixpath.join(paths["logs"], name)
        text = read_remote_text(args, remote_file)
        if text:
            (args.logs_dir / name).write_text(text, encoding="utf-8")

    print(f"Downloaded {len(output_names)} GLB output(s) to {args.outputs_dir}")
    print(f"Downloaded batch logs to {args.logs_dir}")


def run(args: argparse.Namespace) -> None:
    launch(args)
    started = time.monotonic()
    while True:
        elapsed = time.monotonic() - started
        if elapsed > args.wait_timeout:
            raise SystemExit(f"Timed out waiting for DONE sentinel after {args.wait_timeout}s")
        time.sleep(args.poll_interval)
        print("")
        is_done = poll(args)
        if is_done:
            break
    download(args)


def add_common_arguments(parser: argparse.ArgumentParser) -> None:
    parser.add_argument("--pod-ip", default=DEFAULT_POD_IP)
    parser.add_argument("--pod-port", type=int, default=DEFAULT_POD_PORT)
    parser.add_argument("--pod-user", default=DEFAULT_POD_USER)
    parser.add_argument("--key-path", type=Path, default=DEFAULT_KEY_PATH)
    parser.add_argument("--server-port", type=int, default=DEFAULT_SERVER_PORT)
    parser.add_argument("--local-run-root", type=Path, required=True)
    parser.add_argument("--remote-run-root", default=DEFAULT_REMOTE_ROOT)
    parser.add_argument("--sources-dir", type=Path)
    parser.add_argument("--outputs-dir", type=Path)
    parser.add_argument("--logs-dir", type=Path)
    parser.add_argument("--generate-timeout", type=int, default=1800)
    parser.add_argument("--poll-interval", type=int, default=20)
    parser.add_argument("--wait-timeout", type=int, default=7200)


def add_pixal_arguments(parser: argparse.ArgumentParser) -> None:
    parser.add_argument("--seed", type=int, default=1337)
    parser.add_argument("--resolution", type=int, default=1536)
    parser.add_argument("--texture-size", type=int, default=4096)
    parser.add_argument("--decimation", type=int, default=200000)
    parser.add_argument("--remesh", dest="remesh", action="store_true", default=True)
    parser.add_argument("--no-remesh", dest="remesh", action="store_false")
    parser.add_argument("--remesh-band", type=float, default=1.0)
    parser.add_argument("--remesh-project", type=float, default=0.0)
    parser.add_argument("--extend-pixel", type=int, default=0)
    parser.add_argument("--image-resolution", type=int, default=512)
    parser.add_argument("--max-num-tokens", type=int, default=49152)
    parser.add_argument("--mesh-scale", type=float, default=1.0)
    parser.add_argument("--ss-steps", type=int, default=12)
    parser.add_argument("--ss-guidance", type=float, default=7.5)
    parser.add_argument("--ss-guidance-rescale", type=float, default=0.7)
    parser.add_argument("--ss-rescale-t", type=float, default=5.0)
    parser.add_argument("--shape-steps", type=int, default=12)
    parser.add_argument("--shape-guidance", type=float, default=7.5)
    parser.add_argument("--shape-guidance-rescale", type=float, default=0.5)
    parser.add_argument("--shape-rescale-t", type=float, default=3.0)
    parser.add_argument("--tex-steps", type=int, default=12)
    parser.add_argument("--tex-guidance", type=float, default=1.0)
    parser.add_argument("--tex-guidance-rescale", type=float, default=0.0)
    parser.add_argument("--tex-rescale-t", type=float, default=3.0)
    parser.add_argument("--export-fallback", dest="export_fallback", action="store_true", default=True)
    parser.add_argument("--no-export-fallback", dest="export_fallback", action="store_false")
    parser.add_argument("--fallback-decimation", type=int, default=80000)
    parser.add_argument("--safe-fill-holes-fallback", dest="safe_fill_holes_fallback", action="store_true", default=True)
    parser.add_argument("--no-safe-fill-holes-fallback", dest="safe_fill_holes_fallback", action="store_false")


def normalize_paths(args: argparse.Namespace) -> None:
    args.local_run_root = args.local_run_root.resolve()
    if args.sources_dir is None:
        args.sources_dir = args.local_run_root / "Sources"
    else:
        args.sources_dir = args.sources_dir.resolve()
    if args.outputs_dir is None:
        args.outputs_dir = args.local_run_root / "Outputs"
    else:
        args.outputs_dir = args.outputs_dir.resolve()
    if args.logs_dir is None:
        args.logs_dir = args.local_run_root / "Logs"
    else:
        args.logs_dir = args.logs_dir.resolve()
    args.key_path = args.key_path.expanduser().resolve()
    if not args.key_path.exists():
        raise SystemExit(f"SSH key not found: {args.key_path}")


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Run detached Pixal3D batches on RunPod.")
    subparsers = parser.add_subparsers(dest="command", required=True)

    for name in ["launch", "poll", "download", "run"]:
        command = subparsers.add_parser(name)
        add_common_arguments(command)
        if name in {"launch", "run"}:
            add_pixal_arguments(command)

    return parser


def main(argv: list[str] | None = None) -> int:
    args = build_parser().parse_args(argv)
    normalize_paths(args)

    if args.command == "launch":
        launch(args)
    elif args.command == "poll":
        poll(args)
    elif args.command == "download":
        download(args)
    elif args.command == "run":
        run(args)
    else:
        raise SystemExit(f"Unknown command: {args.command}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
