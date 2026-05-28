#!/usr/bin/env python3
"""Build review artifacts from an Unreal-owned T66 gameplay video capture.

Manual selected-frame labels are intentionally limited to start, mid, impact,
and dissipate so every VFX packet uses the same review vocabulary.
"""

from __future__ import annotations

import argparse
import json
import re
import shutil
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Any

from PIL import Image, ImageDraw, ImageFont


EXPECTED_LABELS = ("start", "mid", "impact", "dissipate")


@dataclass(frozen=True)
class SelectedFrame:
    label: str
    sequence_index: int
    source: Path
    copied: Path
    timestamp_seconds: float


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Build T66 VFX video evidence bundle.")
    parser.add_argument("--video")
    parser.add_argument("--frame-dir")
    parser.add_argument("--frame-prefix", default="frame")
    parser.add_argument("--frame-rate", type=float, default=12.0)
    parser.add_argument("--capture-mode", default="unknown")
    parser.add_argument("--output-root")
    parser.add_argument("--ffprobe-json")
    parser.add_argument("--selected-frames", default="")
    parser.add_argument("--res-x", type=int)
    parser.add_argument("--res-y", type=int)
    parser.add_argument("--label", default="")
    parser.add_argument("--self-test-root")
    parser.add_argument(
        "--auto-select-frames",
        action="store_true",
        help="Select start/mid/impact/dissipate from saturated frame activity. Manual --selected-frames wins.",
    )
    return parser.parse_args()


def parse_frame_number(path: Path) -> int | None:
    match = re.search(r"(\d+)(?=\.png$)", path.name, re.I)
    return int(match.group(1)) if match else None


def discover_frames(frame_dir: Path, frame_prefix: str) -> list[Path]:
    if not frame_dir.exists():
        raise FileNotFoundError(f"Frame directory does not exist: {frame_dir}")
    frames = sorted(
        frame_dir.glob(f"{frame_prefix}*.png"),
        key=lambda path: (parse_frame_number(path) is None, parse_frame_number(path) or 0, path.name),
    )
    if len(frames) < 4:
        raise ValueError(f"Evidence bundle requires at least 4 PNG frames, found {len(frames)} in {frame_dir}")
    return frames


def default_indices(frame_count: int) -> dict[str, int]:
    last = frame_count - 1
    return {
        "start": 0,
        "mid": round(last * 0.35),
        "impact": round(last * 0.65),
        "dissipate": last,
    }


def parse_selected_frames(raw: str, frame_count: int) -> dict[str, int]:
    selected = default_indices(frame_count)
    if not raw.strip():
        return selected

    for entry in raw.split(","):
        if not entry.strip():
            continue
        if "=" not in entry:
            raise ValueError(f"Selected frame entry must be label=index: {entry}")
        label, value = [part.strip().lower() for part in entry.split("=", 1)]
        if label not in EXPECTED_LABELS:
            raise ValueError(f"Unknown selected frame label '{label}'. Expected one of: {', '.join(EXPECTED_LABELS)}")
        try:
            index = int(value)
        except ValueError as exc:
            raise ValueError(f"Selected frame index must be an integer: {entry}") from exc
        if index < 0 or index >= frame_count:
            raise ValueError(f"Selected frame {label}={index} is outside frame range 0..{frame_count - 1}")
        selected[label] = index
    return selected


def load_analysis_pixels(path: Path, width: int = 320) -> tuple[int, int, list[tuple[int, int, int]]]:
    with Image.open(path) as image:
        rgb = image.convert("RGB")
        ratio = width / max(1, rgb.width)
        height = max(1, round(rgb.height * ratio))
        rgb = rgb.resize((width, height), Image.Resampling.BILINEAR)
        flattened = getattr(rgb, "get_flattened_data", None)
        data = flattened() if flattened is not None else rgb.getdata()
        return rgb.width, rgb.height, list(data)


def frame_activity_metrics(frames: list[Path]) -> list[dict[str, float]]:
    width, height, baseline = load_analysis_pixels(frames[0])
    metrics: list[dict[str, float]] = []
    for frame in frames:
        frame_width, frame_height, pixels = load_analysis_pixels(frame, width)
        if frame_width != width or frame_height != height:
            raise ValueError(f"Frame size changed during activity scan: {frame}")

        active = 0
        min_x = width
        min_y = height
        max_x = -1
        max_y = -1
        for index, ((r, g, b), (base_r, base_g, base_b)) in enumerate(zip(pixels, baseline)):
            channel_max = max(r, g, b)
            channel_min = min(r, g, b)
            saturation = channel_max - channel_min
            delta = abs(r - base_r) + abs(g - base_g) + abs(b - base_b)
            is_changed_vfx = delta > 75 and channel_max > 60
            is_bright_saturated = channel_max > 150 and saturation > 38
            if not (is_changed_vfx or is_bright_saturated):
                continue
            x = index % width
            y = index // width
            active += 1
            min_x = min(min_x, x)
            min_y = min(min_y, y)
            max_x = max(max_x, x)
            max_y = max(max_y, y)

        if active:
            bbox_area = float((max_x - min_x + 1) * (max_y - min_y + 1))
        else:
            bbox_area = 0.0
        score = float(active) + bbox_area * 0.05
        metrics.append(
            {
                "active_pixels": float(active),
                "bbox_area": bbox_area,
                "score": score,
            }
        )
    return metrics


def choose_active_window(scores: list[float]) -> tuple[int, int, int] | None:
    if not scores:
        return None
    peak_score = max(scores)
    if peak_score <= 0:
        return None
    sorted_scores = sorted(scores)
    median = sorted_scores[len(sorted_scores) // 2]
    threshold = max(peak_score * 0.32, median + peak_score * 0.08)
    active_indices = [index for index, score in enumerate(scores) if score >= threshold]
    if not active_indices:
        return None
    peak = max(range(len(scores)), key=lambda index: scores[index])
    start = active_indices[0]
    end = active_indices[-1]
    if end <= start:
        start = max(0, peak - 1)
        end = min(len(scores) - 1, peak + 1)
    return start, peak, end


def auto_select_frame_indices(frames: list[Path]) -> tuple[dict[str, int], dict[str, Any]]:
    metrics = frame_activity_metrics(frames)
    scores = [metric["score"] for metric in metrics]
    window = choose_active_window(scores)
    if window is None:
        return default_indices(len(frames)), {
            "selection_method": "auto_activity_fallback_default",
            "reason": "No saturated or changed activity detected.",
            "scores": scores,
        }

    start, peak, end = window
    mid = round(start + max(0, peak - start) * 0.5)
    impact = peak
    if impact <= mid and end > mid:
        impact = min(end, mid + 1)
    if mid <= start and peak > start:
        mid = start + 1
    if end <= impact and impact < len(frames) - 1:
        end = impact + 1

    selected = {
        "start": max(0, min(start, len(frames) - 1)),
        "mid": max(0, min(mid, len(frames) - 1)),
        "impact": max(0, min(impact, len(frames) - 1)),
        "dissipate": max(0, min(end, len(frames) - 1)),
    }
    metadata = {
        "selection_method": "auto_activity",
        "activity_window": {"start": start, "peak": peak, "end": end},
        "scores": scores,
        "metrics": metrics,
    }
    return selected, metadata


def safe_copy_name(order: int, label: str, source: Path) -> str:
    frame_number = parse_frame_number(source)
    frame_token = f"{frame_number:04d}" if frame_number is not None else f"{order:04d}"
    return f"{order:02d}_{label}_frame_{frame_token}.png"


def select_frames(
    frames: list[Path],
    output_root: Path,
    selected_indices: dict[str, int],
    frame_rate: float,
) -> list[SelectedFrame]:
    selected_dir = output_root / "selected_frames"
    selected_dir.mkdir(parents=True, exist_ok=True)
    for stale_png in selected_dir.glob("*.png"):
        stale_png.unlink()
    selected: list[SelectedFrame] = []
    for order, label in enumerate(EXPECTED_LABELS):
        source = frames[selected_indices[label]]
        copied = selected_dir / safe_copy_name(order, label, source)
        shutil.copy2(source, copied)
        timestamp = selected_indices[label] / frame_rate if frame_rate > 0 else 0.0
        selected.append(SelectedFrame(label, selected_indices[label], source, copied, timestamp))
    return selected


def load_font(size: int) -> ImageFont.ImageFont:
    for candidate in ("arial.ttf", "segoeui.ttf"):
        try:
            return ImageFont.truetype(candidate, size)
        except OSError:
            continue
    return ImageFont.load_default()


def resize_to_box(image: Image.Image, width: int, height: int) -> Image.Image:
    image.thumbnail((width, height), Image.Resampling.LANCZOS)
    canvas = Image.new("RGB", (width, height), (11, 12, 16))
    x = (width - image.width) // 2
    y = (height - image.height) // 2
    canvas.paste(image, (x, y))
    return canvas


def draw_contact_sheet(selected: list[SelectedFrame], output: Path, title: str) -> None:
    thumb_w = 560
    thumb_h = 315
    header_h = 58
    label_h = 42
    padding = 18
    cols = 2
    rows = 2
    width = padding + cols * thumb_w + (cols - 1) * padding + padding
    height = header_h + rows * (thumb_h + label_h) + (rows - 1) * padding + padding
    sheet = Image.new("RGB", (width, height), (18, 18, 24))
    draw = ImageDraw.Draw(sheet)
    title_font = load_font(24)
    label_font = load_font(18)
    small_font = load_font(14)

    draw.text((padding, 16), title, fill=(244, 244, 248), font=title_font)

    for order, frame in enumerate(selected):
        col = order % cols
        row = order // cols
        x = padding + col * (thumb_w + padding)
        y = header_h + row * (thumb_h + label_h + padding)
        with Image.open(frame.source) as image:
            thumb = resize_to_box(image.convert("RGB"), thumb_w, thumb_h)
        sheet.paste(thumb, (x, y))
        draw.rectangle((x, y, x + thumb_w - 1, y + thumb_h - 1), outline=(86, 92, 110), width=2)
        label = f"{frame.label.upper()}  frame {frame.sequence_index}  {frame.timestamp_seconds:.2f}s"
        draw.text((x, y + thumb_h + 8), label, fill=(255, 255, 255), font=label_font)
        draw.text((x, y + thumb_h + 28), frame.source.name, fill=(180, 184, 196), font=small_font)

    output.parent.mkdir(parents=True, exist_ok=True)
    sheet.save(output)


def load_ffprobe(path: Path | None) -> dict[str, Any] | None:
    if path is None or not path.exists():
        return None
    with path.open("r", encoding="utf-8-sig") as handle:
        return json.load(handle)


def write_selected_notes(path: Path, selected: list[SelectedFrame], video: Path, contact_sheet: Path) -> None:
    lines = [
        "# Selected VFX Evidence Frames",
        "",
        f"- Video: `{video}`",
        f"- Contact sheet: `{contact_sheet}`",
        "",
        "| Label | Sequence index | Timestamp | Copied frame | Source frame | Review note |",
        "|---|---:|---:|---|---|---|",
    ]
    for frame in selected:
        lines.append(
            f"| {frame.label} | {frame.sequence_index} | {frame.timestamp_seconds:.2f}s | "
            f"`{frame.copied}` | `{frame.source}` | TODO: describe visual state and mechanism evidence. |"
        )
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def write_visibility_checklist(
    path: Path,
    *,
    capture_mode: str,
    video: Path,
    contact_sheet: Path,
    selected: list[SelectedFrame],
) -> None:
    selected_summary = ", ".join(f"{frame.label}=frame {frame.sequence_index}" for frame in selected)
    lines = [
        "# VFX Capture Visibility Checklist",
        "",
        f"- Capture mode: `{capture_mode}`",
        f"- Video: `{video}`",
        f"- Contact sheet: `{contact_sheet}`",
        f"- Selected frames: {selected_summary}",
        "",
        "This checklist packages evidence; it is not a visual approval by itself.",
        "",
        "## Manual Visibility Review",
        "",
        "- [ ] Target effect is large enough to judge silhouette at the declared review camera.",
        "- [ ] Camera angle shows the attack plane and target contact zone clearly.",
        "- [ ] Enemies, hit point, or intended targets are visible enough to judge alignment.",
        "- [ ] The effect is not hidden by walls, character mesh, HUD, or capture crop.",
        "- [ ] Selected frames include start, mid, impact, and dissipate states.",
        "- [ ] Temporal behavior is judged from frame range or video, not a single still.",
        "- [ ] If this is a diagnostic camera, the packet says it is diagnostic and does not replace the locked acceptance camera.",
        "",
        "## Manual Scores",
        "",
        "| Criterion | Score 0-5 | Notes |",
        "|---|---:|---|",
        "| Target size/readability | TODO |  |",
        "| Angle/plane readability | TODO |  |",
        "| Occlusion/crop safety | TODO |  |",
        "| Temporal coverage | TODO |  |",
        "| Hitbox/contact alignment visibility | TODO |  |",
        "",
        "## Verdict",
        "",
        "- Visibility result: TODO PASS / PARTIAL / FAIL",
        "- Required recapture changes, if any: TODO",
    ]
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def write_manifest(
    path: Path,
    *,
    args: argparse.Namespace,
    frames: list[Path],
    selected: list[SelectedFrame],
    selection_metadata: dict[str, Any],
    contact_sheet: Path,
    selected_notes: Path,
    visibility_checklist: Path,
    ffprobe_data: dict[str, Any] | None,
) -> None:
    manifest = {
        "schema": "t66.vfx.video_evidence_bundle.v1",
        "label": args.label,
        "capture_mode": args.capture_mode,
        "video": str(Path(args.video).resolve()),
        "frame_dir": str(Path(args.frame_dir).resolve()),
        "frame_prefix": args.frame_prefix,
        "frame_count": len(frames),
        "frame_rate": args.frame_rate,
        "resolution": {"x": args.res_x, "y": args.res_y},
        "selection": selection_metadata,
        "artifacts": {
            "contact_sheet": str(contact_sheet.resolve()),
            "selected_frames": [str(frame.copied.resolve()) for frame in selected],
            "selected_notes": str(selected_notes.resolve()),
            "visibility_checklist": str(visibility_checklist.resolve()),
            "ffprobe_json": str(Path(args.ffprobe_json).resolve()) if args.ffprobe_json else None,
        },
        "selected": [
            {
                "label": frame.label,
                "sequence_index": frame.sequence_index,
                "timestamp_seconds": frame.timestamp_seconds,
                "source": str(frame.source.resolve()),
                "copied": str(frame.copied.resolve()),
            }
            for frame in selected
        ],
        "ffprobe": ffprobe_data,
    }
    path.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")


def build_bundle(args: argparse.Namespace) -> None:
    if not args.video:
        raise ValueError("--video is required")
    if not args.frame_dir:
        raise ValueError("--frame-dir is required")
    if not args.output_root:
        raise ValueError("--output-root is required")

    video = Path(args.video)
    if not video.exists():
        raise FileNotFoundError(f"Video does not exist: {video}")

    output_root = Path(args.output_root)
    output_root.mkdir(parents=True, exist_ok=True)
    frames = discover_frames(Path(args.frame_dir), args.frame_prefix)
    if args.selected_frames.strip():
        selected_indices = parse_selected_frames(args.selected_frames, len(frames))
        selection_metadata = {"selection_method": "manual_selected_frames", "raw": args.selected_frames}
    elif args.auto_select_frames:
        selected_indices, selection_metadata = auto_select_frame_indices(frames)
    else:
        selected_indices = parse_selected_frames(args.selected_frames, len(frames))
        selection_metadata = {"selection_method": "default_fixed_indices"}
    selected = select_frames(frames, output_root, selected_indices, args.frame_rate)

    title_bits = ["T66 VFX Evidence"]
    if args.label:
        title_bits.append(args.label)
    if args.capture_mode:
        title_bits.append(f"mode={args.capture_mode}")
    contact_sheet = output_root / "contact_sheet.png"
    selected_notes = output_root / "selected_frames.md"
    visibility_checklist = output_root / "visibility_checklist.md"
    manifest = output_root / "manifest.json"

    draw_contact_sheet(selected, contact_sheet, " | ".join(title_bits))
    write_selected_notes(selected_notes, selected, Path(args.video), contact_sheet)
    write_visibility_checklist(
        visibility_checklist,
        capture_mode=args.capture_mode,
        video=Path(args.video),
        contact_sheet=contact_sheet,
        selected=selected,
    )
    write_manifest(
        manifest,
        args=args,
        frames=frames,
        selected=selected,
        selection_metadata=selection_metadata,
        contact_sheet=contact_sheet,
        selected_notes=selected_notes,
        visibility_checklist=visibility_checklist,
        ffprobe_data=load_ffprobe(Path(args.ffprobe_json) if args.ffprobe_json else None),
    )

    print(f"Evidence bundle: {output_root}")
    print(f"Contact sheet: {contact_sheet}")
    print(f"Manifest: {manifest}")


def create_self_test_frames(root: Path) -> tuple[Path, Path]:
    frame_dir = root / "frames"
    frame_dir.mkdir(parents=True, exist_ok=True)
    for stale_png in frame_dir.glob("*.png"):
        stale_png.unlink()
    for index in range(10):
        image = Image.new("RGB", (320, 180), (20, 28, 42))
        draw = ImageDraw.Draw(image)
        if 3 <= index <= 7:
            width = 30 + (index - 3) * 38
            draw.rectangle((42, 64, 42 + width, 116), fill=(235, 46 + index * 14, 52))
            draw.arc((40, 28, 250, 156), 188, 336, fill=(255, 248, 242), width=3 + index)
        draw.text((12, 12), f"frame {index:04d}", fill=(255, 255, 255), font=load_font(18))
        image.save(frame_dir / f"frame_{index:04d}.png")
    video = root / "synthetic_placeholder.mp4"
    video.write_text("Synthetic placeholder. This file is not a real MP4.\n", encoding="utf-8")
    return frame_dir, video


def create_self_test_args(root: Path, output_root: Path, *, auto_select: bool, selected_frames: str = "") -> argparse.Namespace:
    frame_dir, video = create_self_test_frames(root)

    return argparse.Namespace(
        video=str(video),
        frame_dir=str(frame_dir),
        frame_prefix="frame",
        frame_rate=8.0,
        capture_mode="selftest",
        output_root=str(output_root),
        ffprobe_json="",
        selected_frames=selected_frames,
        res_x=320,
        res_y=180,
        label="self-test",
        self_test_root=str(root),
        auto_select_frames=auto_select,
    )


def read_manifest(path: Path) -> dict[str, Any]:
    with path.open("r", encoding="utf-8") as handle:
        return json.load(handle)


def selected_indices_from_manifest(manifest: dict[str, Any]) -> dict[str, int]:
    return {entry["label"]: int(entry["sequence_index"]) for entry in manifest["selected"]}


def run_self_test(root: Path) -> None:
    root.mkdir(parents=True, exist_ok=True)
    default_output = root / "evidence_default"
    auto_output = root / "evidence_auto"
    manual_output = root / "evidence_manual"

    default_args = create_self_test_args(root, default_output, auto_select=False)
    build_bundle(default_args)
    default_manifest = read_manifest(default_output / "manifest.json")
    expected_default = default_indices(10)
    if selected_indices_from_manifest(default_manifest) != expected_default:
        raise AssertionError(
            f"Default selection changed: {selected_indices_from_manifest(default_manifest)!r} != {expected_default!r}"
        )
    if default_manifest["selection"]["selection_method"] != "default_fixed_indices":
        raise AssertionError(f"Default selection used wrong method: {default_manifest['selection']!r}")

    auto_args = create_self_test_args(root, auto_output, auto_select=True)
    build_bundle(auto_args)
    auto_manifest = read_manifest(auto_output / "manifest.json")
    auto_method = auto_manifest["selection"]["selection_method"]
    if not str(auto_method).startswith("auto_activity"):
        raise AssertionError(f"Auto selection did not record auto method: {auto_manifest['selection']!r}")
    if selected_indices_from_manifest(auto_manifest) == expected_default:
        raise AssertionError("Auto selection unexpectedly matched the default fixed indices in self-test.")

    manual_args = create_self_test_args(
        root,
        manual_output,
        auto_select=True,
        selected_frames="start=1,mid=2,impact=3,dissipate=4",
    )
    build_bundle(manual_args)
    manual_manifest = read_manifest(manual_output / "manifest.json")
    expected_manual = {"start": 1, "mid": 2, "impact": 3, "dissipate": 4}
    if selected_indices_from_manifest(manual_manifest) != expected_manual:
        raise AssertionError("Manual selected frames did not override auto mode.")
    if manual_manifest["selection"]["selection_method"] != "manual_selected_frames":
        raise AssertionError(f"Manual selection used wrong method: {manual_manifest['selection']!r}")

    report = {
        "schema": "t66.vfx.video_evidence_bundle.self_test.v1",
        "default_indices": expected_default,
        "auto_indices": selected_indices_from_manifest(auto_manifest),
        "manual_indices": selected_indices_from_manifest(manual_manifest),
        "default_manifest": str((default_output / "manifest.json").resolve()),
        "auto_manifest": str((auto_output / "manifest.json").resolve()),
        "manual_manifest": str((manual_output / "manifest.json").resolve()),
    }
    (root / "self_test_report.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print("SELF TEST PASSED")


def main() -> int:
    args = parse_args()
    try:
        if args.self_test_root:
            run_self_test(Path(args.self_test_root))
        else:
            build_bundle(args)
    except Exception as exc:
        print(f"Evidence bundle failed: {exc}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
