"""
Run Stage 02 Quad Retro processing for WorldNpcInteractablesRetroBatch01.

This wrapper is intentionally thin: it reads the Stage01 Trellis manifest and
invokes the existing repo Quad Retro/Quad Remesher pipeline once per row, in
sequence. It does not launch Blender hidden or run jobs in parallel.
"""

from __future__ import annotations

import argparse
import json
import shutil
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont


PROJECT_ROOT = Path(__file__).resolve().parents[2]
RUN_ROOT = PROJECT_ROOT / "Model Generation" / "Runs" / "Interactables" / "WorldNpcInteractablesRetroBatch01"
STAGE01_PATH = RUN_ROOT / "Reports" / "Stage01_SourceAndTrellisManifest.json"
STAGE02_PATH = RUN_ROOT / "Reports" / "Stage02_QuadRetroManifest.json"
NOTES_DIR = RUN_ROOT / "Notes"
LOG_PATH = NOTES_DIR / "stage02_quad_retro.log"
CONTACT_SHEET_PATH = RUN_ROOT / "QA" / "QuadRetroFrontContactSheet.png"
WRAPPER = PROJECT_ROOT / "Model Generation" / "Scripts" / "RunQuadRetroCharacterPipeline.ps1"


def _rel(path: Path | str | None) -> str:
    if not path:
        return ""
    path = Path(path)
    try:
        return str(path.resolve().relative_to(PROJECT_ROOT)).replace("\\", "/")
    except ValueError:
        return str(path)


def _abs(path: Path | str | None) -> Path | None:
    if not path:
        return None
    path = Path(path)
    if path.is_absolute():
        return path
    return PROJECT_ROOT / path


def _load_json(path: Path) -> dict:
    with path.open("r", encoding="utf-8") as handle:
        return json.load(handle)


def _write_json(path: Path, payload: dict) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", encoding="utf-8") as handle:
        json.dump(payload, handle, indent=2)
        handle.write("\n")


def _append_log(message: str) -> None:
    NOTES_DIR.mkdir(parents=True, exist_ok=True)
    timestamp = datetime.now(timezone.utc).isoformat()
    with LOG_PATH.open("a", encoding="utf-8") as handle:
        handle.write(f"[{timestamp}] {message}\n")
    print(message, flush=True)


def _quad_paths(entry: dict) -> dict:
    category = entry["category"]
    row_id = entry["row_id"]
    output_dir = RUN_ROOT / "Retro" / "QuadRetro" / category / row_id
    model = output_dir / f"{row_id}_QuadRetro.glb"
    report = output_dir / "Reports" / f"{row_id}_QuadRetro_report.json"
    pipeline_model = output_dir / "Models" / f"{row_id}_QuadRetro.glb"
    pipeline_front = output_dir / "Renders" / f"{row_id}_QuadRetro_front.png"
    qa_front = RUN_ROOT / "QA" / "QuadRetroFront" / category / f"{row_id}_front.png"
    return {
        "output_dir": output_dir,
        "model": model,
        "report": report,
        "pipeline_model": pipeline_model,
        "pipeline_front": pipeline_front,
        "qa_front": qa_front,
    }


def _report_metrics(report_path: Path) -> dict:
    if not report_path.exists():
        return {}
    try:
        report = _load_json(report_path)
    except json.JSONDecodeError:
        return {"report_error": "invalid_json"}
    qremesh = report.get("qremesh_report") or {}
    values = report.get("adjustable_values") or {}
    return {
        "retopo_quads": report.get("retopo_quads"),
        "retopo_triangles": report.get("retopo_triangles"),
        "qremesh_last_progress": qremesh.get("last_progress"),
        "texture_size": values.get("texture_size"),
        "palette_mode": values.get("palette_mode"),
        "dither_type": values.get("dither_type"),
        "dither_strength": values.get("dither_strength"),
    }


def _stage_entry(entry: dict) -> dict:
    paths = _quad_paths(entry)
    model_exists = paths["model"].exists()
    report_exists = paths["report"].exists()
    qa_exists = paths["qa_front"].exists()
    status = "ready_with_front_qa" if model_exists and report_exists and qa_exists else "pending_quad_retro"
    if model_exists and report_exists and not qa_exists:
        status = "quad_retro_ready_missing_front_qa"
    elif model_exists and not report_exists:
        status = "quad_retro_model_ready_missing_report"
    staged = {
        "row_id": entry["row_id"],
        "category": entry["category"],
        "source_image": entry.get("source_image", ""),
        "raw_trellis_glb": entry.get("raw_trellis_glb", ""),
        "trellis_front_render": entry.get("trellis_front_render", ""),
        "quad_retro_output_dir": _rel(paths["output_dir"]),
        "quad_retro_glb": _rel(paths["model"]),
        "quad_retro_report": _rel(paths["report"]),
        "quad_retro_front_render": _rel(paths["qa_front"]),
        "quad_retro_status": status,
        "quad_retro_glb_exists": model_exists,
        "quad_retro_report_exists": report_exists,
        "quad_retro_front_render_exists": qa_exists,
        "quad_retro_glb_size_bytes": paths["model"].stat().st_size if model_exists else 0,
    }
    staged.update(_report_metrics(paths["report"]))
    return staged


def _write_manifest(stage01: dict) -> dict:
    entries = [_stage_entry(entry) for entry in stage01.get("entries", []) if entry.get("needs_quad_retro")]
    ready = sum(1 for entry in entries if entry["quad_retro_status"] == "ready_with_front_qa")
    payload = {
        "stage": "stage02_quad_retro",
        "output_root": _rel(RUN_ROOT),
        "summary": {
            "total_stage_entries": len(entries),
            "quad_retro_ready": ready,
            "pending_quad_retro": len(entries) - ready,
            "failed_or_incomplete": [
                entry["row_id"]
                for entry in entries
                if entry["quad_retro_status"] != "ready_with_front_qa"
            ],
        },
        "entries": entries,
        "updated_utc": datetime.now(timezone.utc).isoformat(),
    }
    _write_json(STAGE02_PATH, payload)
    return payload


def _run_quad_retro(
    entry: dict,
    timeout_seconds: int,
    force: bool,
    qremesh_source_target_tris: int,
    target_quads: int,
) -> bool:
    paths = _quad_paths(entry)
    if not force and paths["model"].exists() and paths["report"].exists() and paths["qa_front"].exists():
        _append_log(f"SKIP {entry['category']}/{entry['row_id']} already ready")
        return True

    raw_glb = _abs(entry.get("raw_trellis_glb"))
    if not raw_glb or not raw_glb.exists():
        _append_log(f"FAIL {entry['category']}/{entry['row_id']} missing raw Trellis GLB")
        return False

    paths["output_dir"].mkdir(parents=True, exist_ok=True)
    cmd = [
        "powershell.exe",
        "-NoProfile",
        "-ExecutionPolicy",
        "Bypass",
        "-File",
        str(WRAPPER),
        "-InputModel",
        str(raw_glb),
        "-OutputDir",
        str(paths["output_dir"]),
        "-Label",
        entry["row_id"],
        "-TargetQuads",
        str(target_quads),
        "-AdaptiveSize",
        "50",
        "-TextureSize",
        "512",
        "-PaletteMode",
        "none",
        "-DitherType",
        "none",
        "-DitherStrength",
        "0",
        "-BakeSize",
        "1024",
        "-QRemeshSourceTargetTris",
        str(qremesh_source_target_tris),
        "-RenderQA:true",
        "-Background:false",
        "-TimeoutSeconds",
        str(timeout_seconds),
    ]

    _append_log(f"START {entry['category']}/{entry['row_id']} input={_rel(raw_glb)}")
    try:
        completed = subprocess.run(cmd, cwd=PROJECT_ROOT, timeout=timeout_seconds + 120)
    except subprocess.TimeoutExpired:
        _append_log(f"FAIL {entry['category']}/{entry['row_id']} timed out after {timeout_seconds + 120}s")
        return False

    if completed.returncode != 0:
        _append_log(f"FAIL {entry['category']}/{entry['row_id']} exit={completed.returncode}")
        return False

    if paths["pipeline_model"].exists():
        shutil.copy2(paths["pipeline_model"], paths["model"])
    if paths["pipeline_front"].exists():
        paths["qa_front"].parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(paths["pipeline_front"], paths["qa_front"])

    ok = paths["model"].exists() and paths["report"].exists() and paths["qa_front"].exists()
    _append_log(f"{'DONE' if ok else 'FAIL'} {entry['category']}/{entry['row_id']} output={_rel(paths['model'])}")
    return ok


def _make_contact_sheet(manifest: dict) -> None:
    entries = manifest.get("entries", [])
    thumbs = []
    for entry in entries:
        image_path = _abs(entry.get("quad_retro_front_render"))
        if image_path and image_path.exists():
            with Image.open(image_path) as image:
                thumb = image.convert("RGB")
                thumb.thumbnail((220, 220), Image.Resampling.LANCZOS)
        else:
            thumb = Image.new("RGB", (220, 220), (48, 20, 26))
            draw = ImageDraw.Draw(thumb)
            draw.text((16, 95), "MISSING", fill=(255, 220, 220))
        thumbs.append((entry, thumb.copy()))

    if not thumbs:
        return

    columns = 5
    cell_w = 260
    cell_h = 300
    header_h = 48
    rows = (len(thumbs) + columns - 1) // columns
    sheet = Image.new("RGB", (columns * cell_w, header_h + rows * cell_h), (245, 242, 232))
    draw = ImageDraw.Draw(sheet)
    try:
        font = ImageFont.truetype("arial.ttf", 16)
        small = ImageFont.truetype("arial.ttf", 13)
    except OSError:
        font = ImageFont.load_default()
        small = ImageFont.load_default()
    draw.text((16, 14), "WorldNpcInteractablesRetroBatch01 - Quad Retro Front QA", fill=(26, 24, 28), font=font)

    for idx, (entry, thumb) in enumerate(thumbs):
        col = idx % columns
        row = idx // columns
        x = col * cell_w
        y = header_h + row * cell_h
        draw.rectangle((x + 8, y + 8, x + cell_w - 8, y + cell_h - 8), fill=(255, 255, 255), outline=(190, 184, 174))
        px = x + (cell_w - thumb.width) // 2
        py = y + 18
        sheet.paste(thumb, (px, py))
        label = entry["row_id"]
        if len(label) > 25:
            label = label[:22] + "..."
        draw.text((x + 16, y + 244), label, fill=(26, 24, 28), font=font)
        draw.text((x + 16, y + 268), entry.get("quad_retro_status", ""), fill=(80, 76, 72), font=small)

    CONTACT_SHEET_PATH.parent.mkdir(parents=True, exist_ok=True)
    sheet.save(CONTACT_SHEET_PATH)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--only", nargs="*", default=[], help="Optional row IDs to process.")
    parser.add_argument("--force", action="store_true", help="Rerun rows even when outputs exist.")
    parser.add_argument("--refresh", action="store_true", help="Only refresh Stage02 manifest/contact sheet.")
    parser.add_argument("--timeout-seconds", type=int, default=900)
    parser.add_argument("--target-quads", type=int, default=12000)
    parser.add_argument(
        "--qremesh-source-target-tris",
        type=int,
        default=0,
        help="Optional decimated source triangle target before Quad Remesher. 0 keeps the raw source.",
    )
    args = parser.parse_args()

    if not STAGE01_PATH.exists():
        raise SystemExit(f"Missing Stage01 manifest: {STAGE01_PATH}")
    if not WRAPPER.exists():
        raise SystemExit(f"Missing Quad Retro wrapper: {WRAPPER}")

    stage01 = _load_json(STAGE01_PATH)
    selected = [entry for entry in stage01.get("entries", []) if entry.get("needs_quad_retro")]
    if args.only:
        only = set(args.only)
        selected = [entry for entry in selected if entry.get("row_id") in only]
        missing = only - {entry.get("row_id") for entry in selected}
        if missing:
            raise SystemExit(f"Unknown or non-QuadRetro row IDs: {', '.join(sorted(missing))}")

    if not args.refresh:
        for entry in selected:
            _run_quad_retro(
                entry,
                args.timeout_seconds,
                args.force,
                args.qremesh_source_target_tris,
                args.target_quads,
            )

    manifest = _write_manifest(stage01)
    _make_contact_sheet(manifest)
    print(json.dumps(manifest["summary"], indent=2))
    print(f"Stage02 manifest: {_rel(STAGE02_PATH)}")
    print(f"Contact sheet: {_rel(CONTACT_SHEET_PATH)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
