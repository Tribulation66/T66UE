import argparse
import json
import re
from dataclasses import dataclass
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


@dataclass(frozen=True)
class FamilySpec:
    key: str
    group_name: str
    asset_dir_name: str
    prompt_filename: str
    title: str


FAMILY_SPECS = {
    "topbar": FamilySpec("topbar", "TopBar", "TopBar", "topbar_sheet_prompt.txt", "top bar"),
    "center": FamilySpec("center", "Center", "Center", "center_sheet_prompt.txt", "center"),
    "left_panel": FamilySpec("left_panel", "Left", "LeftPanel", "left_panel_sheet_prompt.txt", "left panel"),
    "right_panel": FamilySpec("right_panel", "Right", "RightPanel", "right_panel_sheet_prompt.txt", "right panel"),
    "decor": FamilySpec("decor", "Decor", "Decor", "decor_sheet_prompt.txt", "decor and icon"),
}
DEFAULT_FAMILIES = ("topbar", "center", "left_panel", "right_panel", "decor")


def to_pascal_case(value: str) -> str:
    spaced = re.sub(r"([a-z0-9])([A-Z])", r"\1 \2", value.strip())
    parts = [part for part in re.split(r"[^A-Za-z0-9]+", spaced) if part]
    if not parts:
        raise ValueError("Screen name must contain at least one alphanumeric character.")
    return "".join(part[:1].upper() + part[1:] for part in parts)


def humanize_pascal(value: str) -> str:
    spaced = re.sub(r"([a-z0-9])([A-Z])", r"\1 \2", value)
    return spaced.strip()


def render_reference_readme(screen_token: str, canvas_width: int, canvas_height: int, families: list[FamilySpec]) -> str:
    family_lines = "\n".join(f"- `{family.asset_dir_name}/`" for family in families)
    slice_lines = "\n".join(f"- `SheetSlices/{family.asset_dir_name}/`" for family in families)
    return f"""# {screen_token} Reference Pack

This folder stores the immutable and derived artifacts used to reconstruct `{screen_token}`.

## Canonical Canvas

- layout master target: `{canvas_width}x{canvas_height}`
- hi-res companion target: `{canvas_width * 2}x{canvas_height * 2}`

## Expected Files

- `reference_layout.json`
- `content_ownership.json`
- `asset_manifest.json`
- `debug/`
- `SpriteSheets/`
- sliced family outputs:
{slice_lines}
- promoted runtime assets:
{family_lines}

Keep the approved layout reference separate from generated family sheets and runtime-ready slices.
"""


def render_screen_intake(
    screen_token: str,
    human_name: str,
    canvas_width: int,
    canvas_height: int,
    reference_root: Path,
    prompt_root: Path,
    header_path: Path,
    families: list[FamilySpec],
) -> str:
    family_lines = "\n".join(f"- `{family.asset_dir_name}`: `[yes/no]`" for family in families)
    prompt_lines = "\n".join(
        f"- `{family.prompt_filename}`: `{(prompt_root / family.prompt_filename).as_posix()}`"
        for family in families
    )
    return f"""# {human_name} Screen Intake

## Identity

- Screen token: `{screen_token}`
- Display name: `{human_name}`
- Owner: `[Name or team]`
- Target platform: `[PC / controller / other]`

## Canonical Canvas

- Canonical layout master path: `{(reference_root / 'screen_master.png').as_posix()}`
- Canonical layout master resolution: `{canvas_width}x{canvas_height}`
- Hi-res art master path: `{(reference_root / 'screen_master_2x.png').as_posix()}`
- Hi-res art master resolution: `{canvas_width * 2}x{canvas_height * 2}`
- Runtime target viewport: `{canvas_width}x{canvas_height}`

## Reference Variants

- Primary comparison frame: `{(reference_root / 'screen_master.png').as_posix()}`
- No-buttons variant: `{(reference_root / 'screen_master_no_buttons.png').as_posix()}`
- No-text variant: `{(reference_root / 'screen_master_no_text.png').as_posix()}`
- No-dynamic variant: `{(reference_root / 'screen_master_no_dynamic.png').as_posix()}`

## Regions

- `[RegionName]` — `static | stateful | live`
- `[RegionName]` — `static | stateful | live`
- `[RegionName]` — `static | stateful | live`

## Dynamic And Live Content

- Live data sources: `[leaderboards, social list, timers, store prices, etc.]`
- Freeze strategy for strict diffing: `[fixture, mock data, staged screenshot state, or mask]`
- Regions excluded from strict diff: `[list]`
- Manual validation required for: `[list]`

## Content Ownership Audit

- Ownership artifact: `{(reference_root / 'content_ownership.json').as_posix()}`
- Ownership audit completed: `[yes/no]`
- `generated-shell` regions: `[list]`
- `runtime-text` regions: `[list]`
- `runtime-image` / `runtime-avatar` regions: `[list]`
- `runtime-icon` / `runtime-media` / `runtime-3d-preview` regions: `[list]`
- Open-aperture regions: `[list]`
- Socket-frame regions: `[list]`
- Empty-backplate regions: `[list]`

## Family Boards Needed

{family_lines}

## Runtime Rules

- Localizable controls must remain text-free in runtime art: `[yes/no]`
- Live numerals must remain text-free in runtime art: `[yes/no]`
- Visible control must be the real button: `[yes/no]`
- Any plate expected to stretch must be authored as a true nine-slice: `[yes/no]`

## Layout Artifacts

- `reference_layout.json`: `{(reference_root / 'reference_layout.json').as_posix()}`
- Generated layout header: `{header_path.as_posix()}`
- Coordinate authority: `[reference layout json / generated header / other]`

## Typography Notes

- Primary display font target: `[font]`
- Secondary/supporting font target: `[font]`
- Special casing or locale rules: `[notes]`

## Prompt Pack

- Master frame prompt: `{(prompt_root / 'master_frame_prompt.txt').as_posix()}`
{prompt_lines}

## Validation

- Packaged screenshot command: `[command]`
- Strict-diff regions: `[list]`
- Manual validation regions: `[list]`
- Acceptance bar: `[exact enough / close with blockers / blocked]`

## Open Questions

- `[question]`
- `[question]`
"""


def render_master_prompt(screen_token: str, human_name: str, canvas_width: int, canvas_height: int) -> str:
    return f"""Use case: ui-mockup
Asset type: layout-authority screen reference
Primary request: create or restage the approved {human_name} screen at a locked canonical canvas with no redesign
Scene/background: match the approved screen composition exactly
Subject: the full {human_name} screen as one deterministic reference frame
Style/medium: exact reconstruction of the approved screen, not a redesign
Composition/framing: fixed frame at {canvas_width}x{canvas_height}, exact aspect ratio, no reframing
Lighting/mood: match the approved source
Constraints: preserve the exact hierarchy, anchors, spacing, and silhouette relationships from the approved reference; no alternate layout; no watermark
Avoid: layout drift, changed framing, extra ornaments, missing chrome, altered proportions
"""


def render_family_prompt(screen_token: str, human_name: str, canvas_width: int, canvas_height: int, family: FamilySpec) -> str:
    return f"""Use case: ui-mockup
Asset type: game UI runtime component sheet
Primary request: reorganize the approved {human_name} reference into a production sprite sheet for the {family.title} family only
Input images: attach the approved full-screen reference plus family-specific crops and the measured boxes from screen_intake.md and reference_layout.json
Scene/background: flat near-black presentation board with generous spacing
Subject: isolated reusable assets for the {family.title} family only, derived from the approved reference
Style/medium: painted game UI asset sheet, exact reconstruction of the approved reference art style, not a redesign
Composition/framing: front-on orthographic presentation, grouped by state, no overlap, no mock screen
Measured reference canvas: {canvas_width}x{canvas_height}
Constraints: keep exact silhouette, border thickness, corner cuts, inset bevel depth, material language, and proportions from the approved reference; no baked localizable text; no watermark
Avoid: fake generic UI kits, flat gradients, softened silhouettes, decorative flourishes not present in the approved reference
"""


def render_layout_json(screen_token: str, canvas_width: int, canvas_height: int, families: list[FamilySpec]) -> str:
    payload = {
        "screen": screen_token,
        "canvas": {
            "width": canvas_width,
            "height": canvas_height,
        },
        "metrics": {},
        "groups": {family.group_name: {} for family in families},
    }
    return json.dumps(payload, indent=2) + "\n"


def render_asset_manifest(screen_token: str, canvas_width: int, canvas_height: int) -> str:
    payload = {
        "screen": screen_token,
        "canvas": {
            "width": canvas_width,
            "height": canvas_height,
        },
        "assets": [],
    }
    return json.dumps(payload, indent=2) + "\n"


def render_content_ownership(screen_token: str, canvas_width: int, canvas_height: int) -> str:
    payload = {
        "screen": screen_token,
        "canvas": {
            "width": canvas_width,
            "height": canvas_height,
        },
        "regions": [
            {
                "id": "example_shell_region",
                "group": "Center",
                "content_type": "generated-shell",
                "render_contract": "shell-only",
                "outer_owner": "generated-shell",
                "inner_owner": "generated-shell",
                "code_refs": [],
                "notes": "Use for pure shell/frame chrome owned entirely by the reconstructed screen art.",
            },
            {
                "id": "example_runtime_slot",
                "group": "Center",
                "content_type": "runtime-image",
                "render_contract": "socket-frame",
                "outer_owner": "generated-shell",
                "inner_owner": "runtime-image",
                "code_refs": [],
                "notes": "Use for image or portrait slots where the frame belongs to the screen but the interior image is injected at runtime.",
            },
            {
                "id": "example_live_viewport",
                "group": "Center",
                "content_type": "runtime-media",
                "render_contract": "open-aperture",
                "outer_owner": "generated-shell",
                "inner_owner": "runtime-media",
                "code_refs": [],
                "notes": "Use for media/video/3D preview regions that should remain open or neutral in generated references and family boards.",
            },
        ],
    }
    return json.dumps(payload, indent=2) + "\n"


def render_layout_header(screen_token: str, canvas_width: int, canvas_height: int, families: list[FamilySpec]) -> str:
    namespace_name = f"T66{screen_token}ReferenceLayout"
    family_blocks = "\n\n".join(
        f"\tnamespace {family.group_name}\n\t{{\n\t\t// Add measured FT66ReferenceRect values here.\n\t}}"
        for family in families
    )
    return f"""// Copyright Tribulation 66. All Rights Reserved.

#pragma once

namespace {namespace_name}
{{
\tinline constexpr float CanvasWidth = {canvas_width:.1f}f;
\tinline constexpr float CanvasHeight = {canvas_height:.1f}f;

{family_blocks}
}}
"""


def write_text_file(path: Path, contents: str, force: bool, dry_run: bool, actions: list[str]) -> None:
    if path.exists() and not force:
        actions.append(f"skip file {path}")
        return
    if dry_run:
        actions.append(f"write file {path}")
        return
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(contents, encoding="utf-8")
    actions.append(f"write file {path}")


def ensure_directory(path: Path, dry_run: bool, actions: list[str]) -> None:
    if path.exists():
        actions.append(f"keep dir  {path}")
        return
    if dry_run:
        actions.append(f"make dir  {path}")
        return
    path.mkdir(parents=True, exist_ok=True)
    actions.append(f"make dir  {path}")


def main() -> int:
    parser = argparse.ArgumentParser(description="Scaffold a reusable UI screen-reference pack.")
    parser.add_argument("screen_name", help="Human-readable or PascalCase screen name.")
    parser.add_argument("--canvas-width", type=int, default=1920, help="Canonical 1x canvas width.")
    parser.add_argument("--canvas-height", type=int, default=1080, help="Canonical 1x canvas height.")
    parser.add_argument(
        "--families",
        nargs="+",
        default=list(DEFAULT_FAMILIES),
        choices=sorted(FAMILY_SPECS.keys()),
        help="Families to scaffold.",
    )
    parser.add_argument("--force", action="store_true", help="Overwrite existing files.")
    parser.add_argument("--dry-run", action="store_true", help="Print planned actions without writing files.")
    args = parser.parse_args()

    screen_token = to_pascal_case(args.screen_name)
    human_name = humanize_pascal(screen_token)
    families = [FAMILY_SPECS[key] for key in args.families]

    reference_root = ROOT / "SourceAssets" / "UI" / f"{screen_token}Reference"
    prompt_root = ROOT / "Docs" / "UI" / "PromptPacks" / f"{screen_token}SpriteSheets"
    header_path = ROOT / "Source" / "T66" / "UI" / "Style" / f"T66{screen_token}ReferenceLayout.generated.h"

    directories = [
        reference_root,
        reference_root / "debug",
        reference_root / "SpriteSheets",
        reference_root / "SheetSlices",
        prompt_root,
    ]
    directories.extend(reference_root / family.asset_dir_name for family in families)
    directories.extend(reference_root / "SheetSlices" / family.asset_dir_name for family in families)

    actions: list[str] = []
    for directory in directories:
        ensure_directory(directory, args.dry_run, actions)

    write_text_file(
        reference_root / "README.md",
        render_reference_readme(screen_token, args.canvas_width, args.canvas_height, families),
        args.force,
        args.dry_run,
        actions,
    )
    write_text_file(
        reference_root / "reference_layout.json",
        render_layout_json(screen_token, args.canvas_width, args.canvas_height, families),
        args.force,
        args.dry_run,
        actions,
    )
    write_text_file(
        reference_root / "asset_manifest.json",
        render_asset_manifest(screen_token, args.canvas_width, args.canvas_height),
        args.force,
        args.dry_run,
        actions,
    )
    write_text_file(
        reference_root / "content_ownership.json",
        render_content_ownership(screen_token, args.canvas_width, args.canvas_height),
        args.force,
        args.dry_run,
        actions,
    )
    write_text_file(
        prompt_root / "screen_intake.md",
        render_screen_intake(
            screen_token,
            human_name,
            args.canvas_width,
            args.canvas_height,
            reference_root,
            prompt_root,
            header_path,
            families,
        ),
        args.force,
        args.dry_run,
        actions,
    )
    write_text_file(
        prompt_root / "master_frame_prompt.txt",
        render_master_prompt(screen_token, human_name, args.canvas_width, args.canvas_height),
        args.force,
        args.dry_run,
        actions,
    )
    for family in families:
        write_text_file(
            prompt_root / family.prompt_filename,
            render_family_prompt(screen_token, human_name, args.canvas_width, args.canvas_height, family),
            args.force,
            args.dry_run,
            actions,
        )
    write_text_file(
        header_path,
        render_layout_header(screen_token, args.canvas_width, args.canvas_height, families),
        args.force,
        args.dry_run,
        actions,
    )

    print(f"Scaffold target: {screen_token}")
    print(f"Canvas: {args.canvas_width}x{args.canvas_height}")
    for action in actions:
        print(action)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
