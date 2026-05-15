#!/usr/bin/env python3
"""Verify a T66 UI capture against a reference, widget dump, and checklist."""

from __future__ import annotations

import argparse
import json
import math
import re
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Any

from PIL import Image, ImageDraw, ImageFont


@dataclass
class ChecklistItem:
    line: int
    section: str
    tag: str
    prop: str
    expected: str
    tolerance: float | None
    note: str


@dataclass
class Result:
    verdict: str
    item: ChecklistItem
    actual: Any = None
    message: str = ""
    widget: dict[str, Any] | None = None


ALIASES = {
    "exists": "exists",
    "x": "geometry.normalized.x",
    "y": "geometry.normalized.y",
    "w": "geometry.normalized.w",
    "h": "geometry.normalized.h",
    "width": "geometry.normalized.w",
    "height": "geometry.normalized.h",
    "absolute_x": "geometry.absolute_x",
    "absolute_y": "geometry.absolute_y",
    "absolute_width": "geometry.width",
    "absolute_height": "geometry.height",
    "visible": "geometry.visibility",
    "visibility": "geometry.visibility",
    "enabled": "geometry.enabled",
    "hovered": "geometry.hovered",
    "pressed": "geometry.pressed",
    "child_count": "child_count",
    "type": "type",
    "source": "source",
    "text": "text.content",
    "font_size": "text.size",
    "letter_spacing": "text.letter_spacing",
    "text_color": "text.color.hex",
    "border_resource": "border.brush_resource",
    "brush_resource": "border.brush_resource",
    "border_draw_type": "border.draw_type",
    "draw_type": "border.draw_type",
    "border_tint": "border.tint.hex",
    "border_color": "border.background_color.hex",
    "background_color": "border.background_color.hex",
    "button_state": "button_state.state",
    "intended_state": "t66_metadata.intended_state",
    "state": "t66_metadata.intended_state",
    "intended_role": "t66_metadata.intended_role",
    "role": "t66_metadata.intended_role",
    "has_click_handler": "interactivity.has_click_handler",
    "hover_capable": "interactivity.hover_capable",
    "toggle_group": "interactivity.toggle_group",
    "is_label": "t66_metadata.is_label",
}


def flat_style_palette_source() -> Path:
    return Path(__file__).resolve().parents[1] / "Source" / "T66" / "UI" / "Style" / "T66FlatStyle.cpp"


def format_palette_hex(r: int, g: int, b: int, alpha: float = 1.0) -> str:
    a = max(0, min(255, round(alpha * 255)))
    return f"#{r:02X}{g:02X}{b:02X}{a:02X}"


def load_flat_style_palette(source: Path | None = None) -> dict[str, str]:
    source = source or flat_style_palette_source()
    if not source.exists():
        return {}

    text = source.read_text(encoding="utf-8")
    function_pattern = re.compile(
        r"FLinearColor\s+FT66FlatStyle::(?P<name>\w+)\(\)\s*\{\s*return\s+(?P<expr>[^;]+);\s*\}",
        re.S,
    )
    expressions = {match.group("name"): match.group("expr").strip() for match in function_pattern.finditer(text)}
    resolved: dict[str, str] = {}

    def resolve(name: str, stack: set[str] | None = None) -> str | None:
        if name in resolved:
            return resolved[name]
        expr = expressions.get(name)
        if expr is None:
            return None
        stack = set() if stack is None else stack
        if name in stack:
            return None
        stack.add(name)

        hex_match = re.fullmatch(
            r"HexColor\(\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)(?:\s*,\s*([0-9.]+)f?)?\s*\)",
            expr,
        )
        if hex_match:
            alpha = float(hex_match.group(4)) if hex_match.group(4) is not None else 1.0
            value = format_palette_hex(
                int(hex_match.group(1)),
                int(hex_match.group(2)),
                int(hex_match.group(3)),
                alpha,
            )
            resolved[name] = value
            return value

        alias_match = re.fullmatch(r"(\w+)\(\)", expr)
        if alias_match:
            value = resolve(alias_match.group(1), stack)
            if value is not None:
                resolved[name] = value
            return value
        return None

    for name in expressions:
        resolve(name)
    return {**resolved, **{key.lower(): value for key, value in resolved.items()}}


def resolve_palette_expected(value: str, palette: dict[str, str] | None) -> str:
    if not palette:
        return value
    key = value.strip()
    if key.startswith("FT66FlatStyle::"):
        key = key.split("::", 1)[1]
    if key.endswith("()"):
        key = key[:-2]
    return palette.get(key, palette.get(key.lower(), value))


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Verify T66 UI fidelity.")
    parser.add_argument("--reference", required=True)
    parser.add_argument("--capture", required=True)
    parser.add_argument("--dump", required=True)
    parser.add_argument("--checklist", required=True)
    parser.add_argument("--output", required=True)
    parser.add_argument("--contact-sheet", required=True)
    return parser.parse_args()


def parse_checklist(path: Path) -> list[ChecklistItem]:
    items: list[ChecklistItem] = []
    section = "Unsectioned"
    lines = path.read_text(encoding="utf-8").splitlines()
    for number, raw in enumerate(lines, start=1):
        line = raw.strip()
        if not line or line.startswith("#"):
            if line.startswith("## "):
                section = line[3:].strip()
            continue
        if not line.startswith("- ["):
            continue

        if "]" not in line:
            raise ValueError(f"{path}:{number}: malformed checklist item")
        body = line.split("]", 1)[1].strip()
        parts = [part.strip() for part in body.split("|")]
        if len(parts) < 2:
            raise ValueError(f"{path}:{number}: expected '<Tag> | <property>=<expected>'")

        tag = parts[0]
        expectation = parts[1]
        if not tag:
            raise ValueError(f"{path}:{number}: missing tag")
        if "=" not in expectation:
            raise ValueError(f"{path}:{number}: expected property assignment")

        prop, expected = [part.strip() for part in expectation.split("=", 1)]
        if not prop or expected == "":
            raise ValueError(f"{path}:{number}: missing property or expected value")

        tolerance: float | None = None
        note = ""
        for extra in parts[2:]:
            if not extra:
                continue
            if extra.startswith("#"):
                note = extra[1:].strip()
                continue
            match = re.match(r"^(?:tol(?:erance)?\s*=\s*)?([-+]?\d+(?:\.\d+)?)$", extra, re.I)
            if not match:
                raise ValueError(f"{path}:{number}: could not parse tolerance/note segment '{extra}'")
            tolerance = float(match.group(1))

        items.append(ChecklistItem(number, section, tag, prop, expected, tolerance, note))
    return items


def nested_get(obj: dict[str, Any], dotted_path: str) -> Any:
    current: Any = obj
    for part in dotted_path.split("."):
        if isinstance(current, dict) and part in current:
            current = current[part]
        else:
            return None
    return current


def normalize_expected(value: str) -> str:
    stripped = value.strip()
    if len(stripped) >= 2 and stripped[0] == stripped[-1] and stripped[0] in {"'", '"'}:
        return stripped[1:-1]
    return stripped


def to_bool(value: Any) -> bool | None:
    if isinstance(value, bool):
        return value
    if isinstance(value, str):
        lowered = value.strip().lower()
        if lowered in {"true", "yes", "1"}:
            return True
        if lowered in {"false", "no", "0"}:
            return False
    return None


def to_float(value: Any) -> float | None:
    if isinstance(value, bool):
        return None
    if isinstance(value, (int, float)):
        return float(value)
    if isinstance(value, str):
        try:
            return float(value.strip())
        except ValueError:
            return None
    return None


def compare_value(actual: Any, expected_raw: str, tolerance: float | None, palette: dict[str, str] | None = None) -> tuple[str, str]:
    expected = resolve_palette_expected(normalize_expected(expected_raw), palette)
    lowered = expected.lower()

    if lowered in {"any", "present"}:
        return ("PASS", "present") if actual not in {None, ""} else ("FAIL", "missing/empty")

    if lowered in {"none", "null", "empty", "absent"}:
        empty_values = {None, "", "#00000000", "#000000", "transparent"}
        actual_text = str(actual).strip().lower() if actual is not None else None
        return ("PASS", "empty/transparent") if actual is None or actual_text in empty_values else ("FAIL", f"expected empty/transparent, got {actual}")

    actual_bool = to_bool(actual)
    expected_bool = to_bool(expected)
    if expected_bool is not None:
        return ("PASS", str(actual)) if actual_bool == expected_bool else ("FAIL", f"expected {expected_bool}, got {actual}")

    actual_number = to_float(actual)
    if actual_number is not None:
        range_match = re.match(r"^\s*([-+]?\d+(?:\.\d+)?)\s*\.\.\s*([-+]?\d+(?:\.\d+)?)\s*$", expected)
        if range_match:
            low = float(range_match.group(1))
            high = float(range_match.group(2))
            return ("PASS", f"{actual_number:g} in range") if low <= actual_number <= high else ("FAIL", f"{actual_number:g} outside {low:g}..{high:g}")

        op_match = re.match(r"^\s*(<=|>=|<|>)\s*([-+]?\d+(?:\.\d+)?)\s*$", expected)
        if op_match:
            op = op_match.group(1)
            rhs = float(op_match.group(2))
            ok = {
                "<=": actual_number <= rhs,
                ">=": actual_number >= rhs,
                "<": actual_number < rhs,
                ">": actual_number > rhs,
            }[op]
            return ("PASS", f"{actual_number:g} {op} {rhs:g}") if ok else ("FAIL", f"{actual_number:g} not {op} {rhs:g}")

        expected_number = to_float(expected)
        if expected_number is not None:
            allowed = 0.0 if tolerance is None else tolerance
            delta = abs(actual_number - expected_number)
            return ("PASS", f"delta {delta:g}") if delta <= allowed else ("FAIL", f"delta {delta:g} > {allowed:g}")

    if lowered in {"unsure", "content_delta", "visual_delta"}:
        return "UNSURE", "manual/content delta"

    actual_text = str(actual)
    if expected.startswith("#"):
        actual_hex = actual_text.upper()
        expected_hex = expected.upper()
        if len(expected_hex) == 7 and len(actual_hex) == 9 and actual_hex.endswith("FF"):
            actual_hex = actual_hex[:7]
        if len(actual_hex) == 7 and len(expected_hex) == 9 and expected_hex.endswith("FF"):
            expected_hex = expected_hex[:7]
        ok = actual_hex == expected_hex
    else:
        ok = actual_text == expected
    return ("PASS", "matched") if ok else ("FAIL", f"expected '{expected}', got '{actual_text}'")


def evaluate(items: list[ChecklistItem], dump: dict[str, Any], palette: dict[str, str] | None = None) -> list[Result]:
    widgets = list(dump.get("widgets", []))
    top_bar = dump.get("top_bar")
    if isinstance(top_bar, dict):
        widgets.extend(top_bar.get("widgets", []))
    by_tag: dict[str, dict[str, Any]] = {}
    for widget in widgets:
        tag = widget.get("tag")
        if tag and tag not in by_tag:
            by_tag[tag] = widget

    results: list[Result] = []
    for item in items:
        if item.prop == "tag_prefix_count":
            count = sum(1 for tag in by_tag if tag.startswith(item.tag))
            verdict, message = compare_value(count, item.expected, item.tolerance, palette)
            results.append(Result(verdict, item, count, message, None))
            continue

        widget = by_tag.get(item.tag)
        if widget is None:
            results.append(Result("FAIL", item, None, "tag not found", None))
            continue

        if item.prop == "exists" or ALIASES.get(item.prop) == "exists":
            verdict, message = compare_value(True, item.expected, item.tolerance, palette)
            results.append(Result(verdict, item, True, message, widget))
            continue

        path = ALIASES.get(item.prop, item.prop)
        actual = nested_get(widget, path)
        if actual is None:
            results.append(Result("FAIL", item, None, f"property not found: {path}", widget))
            continue

        verdict, message = compare_value(actual, item.expected, item.tolerance, palette)
        if item.prop == "is_label" and to_bool(item.expected) is True and verdict == "PASS":
            border_hex = nested_get(widget, "border.background_color.hex")
            if isinstance(border_hex, str) and border_hex.upper() not in {"", "#00000000", "#000000"}:
                verdict = "FAIL"
                message = f"label has border/background color {border_hex}"
        if verdict == "FAIL" and "content" in item.note.lower():
            verdict = "UNSURE"
            message = f"{message}; marked as content delta"
        results.append(Result(verdict, item, actual, message, widget))
    return results


def load_image(path: Path) -> Image.Image:
    return Image.open(path).convert("RGB")


def draw_contact_sheet(reference: Path, capture: Path, results: list[Result], output: Path) -> None:
    ref_img = load_image(reference)
    cap_img = load_image(capture)
    ref_img = ref_img.resize(cap_img.size, Image.Resampling.LANCZOS)

    sheet = Image.new("RGB", (cap_img.width * 2, cap_img.height), (10, 10, 12))
    sheet.paste(ref_img, (0, 0))
    sheet.paste(cap_img, (cap_img.width, 0))
    draw = ImageDraw.Draw(sheet)
    try:
        font = ImageFont.truetype("arial.ttf", max(14, cap_img.height // 70))
    except OSError:
        font = ImageFont.load_default()

    draw.text((12, 12), "REFERENCE", fill=(255, 255, 255), font=font)
    draw.text((cap_img.width + 12, 12), "CAPTURE", fill=(255, 255, 255), font=font)

    for result in results:
        if result.verdict != "FAIL" or not result.widget:
            continue
        normalized = nested_get(result.widget, "geometry.normalized")
        if not isinstance(normalized, dict):
            continue
        try:
            x = cap_img.width + int(float(normalized.get("x", 0)) * cap_img.width)
            y = int(float(normalized.get("y", 0)) * cap_img.height)
            w = max(2, int(float(normalized.get("w", 0)) * cap_img.width))
            h = max(2, int(float(normalized.get("h", 0)) * cap_img.height))
        except (TypeError, ValueError):
            continue
        draw.rectangle((x, y, x + w, y + h), outline=(255, 40, 55), width=3)
        draw.text((x + 4, max(0, y - 18)), f"{result.item.tag}: {result.item.prop}", fill=(255, 40, 55), font=font)

    output.parent.mkdir(parents=True, exist_ok=True)
    sheet.save(output)


def write_report(results: list[Result], args: argparse.Namespace) -> None:
    counts = {key: 0 for key in ["PASS", "FAIL", "UNSURE"]}
    for result in results:
        counts[result.verdict] += 1

    lines = [
        "# UI Fidelity Verification Report",
        "",
        f"- Reference: `{args.reference}`",
        f"- Capture: `{args.capture}`",
        f"- Dump: `{args.dump}`",
        f"- Checklist: `{args.checklist}`",
        f"- Verdicts: PASS={counts['PASS']} FAIL={counts['FAIL']} UNSURE={counts['UNSURE']}",
        "",
        "## Results",
        "",
        "| Verdict | Section | Line | Tag | Property | Expected | Actual | Note |",
        "| --- | --- | ---: | --- | --- | --- | --- | --- |",
    ]

    for result in results:
        actual = result.actual
        if isinstance(actual, float):
            actual_text = f"{actual:.4f}"
        else:
            actual_text = "" if actual is None else str(actual)
        lines.append(
            "| {verdict} | {section} | {line} | `{tag}` | `{prop}` | `{expected}` | `{actual}` | {note} {message} |".format(
                verdict=result.verdict,
                section=result.item.section.replace("|", "/"),
                line=result.item.line,
                tag=result.item.tag.replace("|", "/"),
                prop=result.item.prop.replace("|", "/"),
                expected=result.item.expected.replace("|", "/"),
                actual=actual_text.replace("|", "/"),
                note=result.item.note.replace("|", "/"),
                message=result.message.replace("|", "/"),
            )
        )

    output = Path(args.output)
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    args = parse_args()
    try:
        checklist = parse_checklist(Path(args.checklist))
    except ValueError as exc:
        print(f"Checklist parse error: {exc}", file=sys.stderr)
        return 2

    with Path(args.dump).open("r", encoding="utf-8") as handle:
        dump = json.load(handle)

    results = evaluate(checklist, dump, load_flat_style_palette())
    write_report(results, args)
    draw_contact_sheet(Path(args.reference), Path(args.capture), results, Path(args.contact_sheet))

    fail_count = sum(1 for result in results if result.verdict == "FAIL")
    unsure_count = sum(1 for result in results if result.verdict == "UNSURE")
    print(f"PASS={len(results) - fail_count - unsure_count} FAIL={fail_count} UNSURE={unsure_count}")
    return 1 if fail_count else 0


if __name__ == "__main__":
    sys.exit(main())
