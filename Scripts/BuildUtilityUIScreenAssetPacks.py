from __future__ import annotations

import json
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont


ROOT = Path(__file__).resolve().parents[1]
UI_ROOT = ROOT / "UI" / "screens"
ANCHOR = UI_ROOT / "main_menu" / "reference" / "canonical_reference_1920x1080.png"
CANVAS = [1920, 1080]


def rect(x: int, y: int, w: int, h: int) -> dict[str, int]:
    return {"x": x, "y": y, "width": w, "height": h}


PACKS: dict[str, dict[str, object]] = {
    "challenges": {
        "screen_key": "Challenges",
        "source": "Source/T66/UI/Screens/T66ChallengesScreen.cpp",
        "regions": [
            ("modal_shell", rect(180, 60, 1560, 940), "generated-shell"),
            ("header", rect(210, 88, 1500, 174), "generated-shell"),
            ("mode_tabs", rect(1280, 98, 370, 58), "runtime-control"),
            ("close_button", rect(1664, 96, 54, 58), "runtime-control"),
            ("source_tabs", rect(230, 170, 520, 60), "runtime-control"),
            ("status_line", rect(222, 238, 1120, 32), "runtime-text"),
            ("list_shell", rect(206, 296, 760, 690), "generated-shell"),
            ("list_row_selected", rect(232, 318, 700, 92), "runtime-control"),
            ("list_row_normal", rect(232, 430, 700, 92), "runtime-control"),
            ("detail_shell", rect(992, 296, 744, 690), "generated-shell"),
            ("detail_title", rect(1020, 326, 620, 70), "runtime-text"),
            ("detail_body", rect(1020, 430, 640, 260), "runtime-text"),
            ("confirm_button", rect(1512, 858, 210, 58), "runtime-control"),
        ],
        "families": ["modal-shell", "header-shell", "list-shell", "detail-shell", "row-plate", "compact-buttons", "status-badge", "checkbox-marker"],
    },
    "daily_climb": {
        "screen_key": "DailyClimb",
        "source": "Source/T66/UI/Screens/T66DailyClimbScreen.cpp",
        "regions": [
            ("scene_plate", rect(0, 0, 1920, 1080), "generated-scene"),
            ("back_button", rect(42, 42, 132, 50), "runtime-control"),
            ("title_group", rect(500, 54, 920, 132), "runtime-text"),
            ("rules_shell", rect(42, 246, 487, 726), "generated-shell"),
            ("rules_status", rect(74, 356, 423, 80), "runtime-text"),
            ("rules_info_cards", rect(74, 452, 423, 210), "runtime-text"),
            ("rules_list", rect(74, 704, 423, 240), "runtime-text"),
            ("cta_stack", rect(738, 650, 445, 311), "generated-shell"),
            ("start_button", rect(766, 671, 388, 92), "runtime-control"),
            ("continue_button", rect(766, 768, 388, 97), "runtime-control"),
            ("cta_note", rect(784, 880, 353, 58), "runtime-text"),
            ("leaderboard_shell", rect(1420, 228, 458, 787), "generated-shell"),
            ("leaderboard_live_rows", rect(1432, 320, 434, 650), "runtime-text"),
        ],
        "families": ["scene-plate", "rules-panel", "cta-stack", "leaderboard-shell", "row-card", "compact-back-button", "wide-cta-buttons"],
    },
    "pause_menu": {
        "screen_key": "PauseMenu",
        "source": "Source/T66/UI/Screens/T66PauseMenuScreen.cpp",
        "regions": [
            ("modal_scrim", rect(0, 0, 1920, 1080), "runtime-overlay"),
            ("pause_shell", rect(900, 84, 220, 396), "generated-shell"),
            ("title_well", rect(920, 112, 180, 56), "runtime-text"),
            ("resume_button", rect(912, 184, 196, 32), "runtime-control"),
            ("save_quit_button", rect(912, 224, 196, 32), "runtime-control"),
            ("restart_button", rect(912, 264, 196, 32), "runtime-control"),
            ("settings_button", rect(912, 304, 196, 32), "runtime-control"),
            ("achievements_button", rect(912, 344, 196, 32), "runtime-control"),
            ("leaderboard_button", rect(912, 384, 196, 32), "runtime-control"),
        ],
        "families": ["modal-shell", "vertical-command-buttons"],
    },
    "report_bug": {
        "screen_key": "ReportBug",
        "source": "Source/T66/UI/Screens/T66ReportBugScreen.cpp",
        "regions": [
            ("modal_scrim", rect(0, 0, 1920, 1080), "runtime-overlay"),
            ("report_shell", rect(620, 314, 684, 450), "generated-shell"),
            ("title_well", rect(724, 360, 472, 72), "runtime-text"),
            ("text_field_shell", rect(682, 458, 556, 156), "generated-shell"),
            ("textbox_live_text", rect(704, 476, 514, 120), "runtime-text"),
            ("submit_button", rect(744, 654, 198, 74), "runtime-control"),
            ("cancel_button", rect(978, 654, 198, 74), "runtime-control"),
        ],
        "families": ["modal-shell", "multiline-input-shell", "compact-buttons"],
    },
    "quit_confirmation_modal": {
        "screen_key": "QuitConfirmation",
        "source": "Source/T66/UI/Screens/T66QuitConfirmationModal.cpp",
        "regions": [
            ("modal_scrim", rect(0, 0, 1920, 1080), "runtime-overlay"),
            ("quit_shell", rect(592, 352, 736, 284), "generated-shell"),
            ("title_well", rect(672, 388, 576, 72), "runtime-text"),
            ("message_well", rect(704, 494, 512, 48), "runtime-text"),
            ("stay_button", rect(610, 556, 346, 64), "runtime-control"),
            ("quit_button", rect(974, 556, 346, 64), "runtime-control"),
        ],
        "families": ["modal-shell", "wide-success-button", "wide-danger-button"],
    },
    "party_invite_modal": {
        "screen_key": "PartyInvite",
        "source": "Source/T66/UI/Screens/T66PartyInviteModal.cpp",
        "regions": [
            ("modal_scrim", rect(0, 0, 1920, 1080), "runtime-overlay"),
            ("invite_shell", rect(468, 336, 984, 412), "generated-shell"),
            ("title_well", rect(672, 382, 576, 70), "runtime-text"),
            ("body_row_shell", rect(530, 502, 860, 84), "generated-shell"),
            ("body_text", rect(578, 520, 764, 48), "runtime-text"),
            ("accept_button", rect(548, 632, 396, 70), "runtime-control"),
            ("reject_button", rect(976, 632, 396, 70), "runtime-control"),
        ],
        "families": ["modal-shell", "row-shell", "wide-success-button", "wide-danger-button"],
    },
    "player_summary_picker": {
        "screen_key": "PlayerSummaryPicker",
        "source": "Source/T66/UI/Screens/T66PlayerSummaryPickerScreen.cpp",
        "regions": [
            ("modal_scrim", rect(0, 0, 1920, 1080), "runtime-overlay"),
            ("empty_shell", rect(496, 364, 928, 242), "generated-shell"),
            ("title_well", rect(748, 400, 424, 58), "runtime-text"),
            ("empty_message_well", rect(704, 502, 512, 42), "runtime-text"),
            ("populated_shell", rect(360, 268, 1200, 520), "generated-shell"),
            ("player_card", rect(432, 390, 330, 300), "generated-shell"),
            ("portrait_socket", rect(470, 426, 254, 160), "runtime-image"),
            ("select_button", rect(486, 620, 220, 58), "runtime-control"),
        ],
        "families": ["empty-modal-shell", "populated-modal-shell", "player-card-shell", "portrait-frame", "select-button"],
    },
    "save_preview": {
        "screen_key": "SavePreview",
        "source": "Source/T66/UI/Screens/T66SavePreviewScreen.cpp",
        "regions": [
            ("preview_underlay", rect(0, 0, 1920, 1080), "runtime-image"),
            ("preview_scrim", rect(0, 0, 1920, 1080), "runtime-overlay"),
            ("bottom_panel_shell", rect(580, 780, 760, 244), "generated-shell"),
            ("title_well", rect(720, 812, 480, 42), "runtime-text"),
            ("subtitle_well", rect(650, 862, 620, 44), "runtime-text"),
            ("back_button", rect(660, 936, 160, 48), "runtime-control"),
            ("load_button", rect(840, 936, 160, 48), "runtime-control"),
        ],
        "families": ["bottom-panel-shell", "compact-neutral-button", "compact-primary-button"],
    },
    "language_select": {
        "screen_key": "LanguageSelect",
        "source": "Source/T66/UI/Screens/T66LanguageSelectScreen.cpp",
        "regions": [
            ("shared_topbar", rect(0, 0, 1920, 144), "shared-topbar"),
            ("content_shell", rect(0, 148, 1920, 932), "generated-shell"),
            ("back_button", rect(30, 210, 212, 66), "runtime-control"),
            ("title_well", rect(620, 160, 680, 92), "runtime-text"),
            ("language_list_shell", rect(90, 308, 1740, 590), "generated-shell"),
            ("language_rows", rect(552, 344, 792, 540), "runtime-control"),
            ("scrollbar", rect(1795, 326, 20, 192), "runtime-control"),
            ("confirm_button", rect(832, 970, 256, 64), "runtime-control"),
        ],
        "families": ["content-shell", "language-row-button", "confirm-button", "back-button", "scrollbar"],
    },
    "account_status": {
        "screen_key": "AccountStatus",
        "source": "Source/T66/UI/Screens/T66AccountStatusScreen.cpp",
        "regions": [
            ("shared_topbar", rect(0, 0, 1920, 144), "shared-topbar"),
            ("back_button", rect(30, 212, 212, 66), "runtime-control"),
            ("outer_frame", rect(0, 232, 1920, 848), "generated-shell"),
            ("tab_strip", rect(730, 236, 460, 62), "runtime-control"),
            ("status_panel", rect(20, 310, 920, 214), "generated-shell"),
            ("progress_panel", rect(20, 548, 920, 526), "generated-shell"),
            ("progress_rows", rect(50, 628, 858, 400), "runtime-text"),
            ("progress_fills", rect(52, 842, 856, 118), "runtime-value"),
            ("pb_filters", rect(1000, 338, 830, 58), "runtime-control"),
            ("score_table", rect(990, 420, 860, 420), "runtime-text"),
            ("speed_table", rect(990, 858, 860, 220), "runtime-text"),
            ("scrollbar", rect(1866, 306, 28, 586), "runtime-control"),
        ],
        "families": ["outer-frame", "section-shell", "row-shell", "tab-button", "dropdown-field", "table-row", "progress-trough", "scrollbar"],
    },
}


def to_xywh(region: tuple[str, dict[str, int], str]) -> list[int]:
    box = region[1]
    return [box["x"], box["y"], box["width"], box["height"]]


def write_json(path: Path, payload: object) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")


def write_overlay(slug: str, regions: list[tuple[str, dict[str, int], str]]) -> None:
    reference_path = UI_ROOT / slug / "reference" / "canonical_reference_1920x1080.png"
    out_path = UI_ROOT / slug / "layout" / "reference_layout_overlay.png"
    image = Image.open(reference_path).convert("RGBA")
    draw = ImageDraw.Draw(image, "RGBA")
    try:
        font = ImageFont.truetype("arial.ttf", 16)
    except OSError:
        font = ImageFont.load_default()

    colors = {
        "generated-shell": (255, 198, 65, 220),
        "generated-scene": (78, 161, 255, 180),
        "runtime-control": (82, 235, 121, 220),
        "runtime-text": (255, 255, 255, 220),
        "runtime-image": (180, 120, 255, 220),
        "runtime-overlay": (255, 92, 92, 180),
        "runtime-value": (255, 255, 255, 220),
        "shared-topbar": (255, 150, 60, 210),
    }
    for name, box, owner in regions:
        x, y, w, h = box["x"], box["y"], box["width"], box["height"]
        color = colors.get(owner, (255, 255, 255, 220))
        draw.rectangle([x, y, x + w, y + h], outline=color, width=3)
        label = f"{name} ({owner})"
        label_box = draw.textbbox((x + 4, y + 4), label, font=font)
        draw.rectangle(label_box, fill=(0, 0, 0, 150))
        draw.text((x + 4, y + 4), label, fill=color, font=font)

    out_path.parent.mkdir(parents=True, exist_ok=True)
    image.save(out_path)


def write_prompt(slug: str, pack: dict[str, object], regions: list[tuple[str, dict[str, int], str]]) -> None:
    prompts_dir = UI_ROOT / slug / "prompts"
    prompts_dir.mkdir(parents=True, exist_ok=True)
    prompt_path = prompts_dir / "screen_chrome_family_prompt.txt"
    instructions_path = prompts_dir / "screen_chrome_family_native_imagegen.md"
    families = ", ".join(str(item) for item in pack["families"])
    region_lines = "\n".join(
        f"- {name}: {box['x']},{box['y']},{box['width']},{box['height']} owner={owner}"
        for name, box, owner in regions
    )
    prompt = f"""Create one production sprite-family board for T66 screen `{slug}`.

Use the attached canonical main-menu style anchor and the attached `{slug}` screen-specific reference. This is exact reconstruction of the reference art style, not a redesign.

Output requirement:
- exactly one 1920x1080 flat neutral presentation board
- front-on orthographic UI components only
- no full-screen screenshot composite
- no baked localizable labels, names, numbers, descriptions, leaderboard rows, progress values, or typed/user text
- no runtime-owned portraits, avatars, screenshots, save previews, or gameplay underlay
- components should be isolated with transparent or flat-neutral spacing so deterministic slicing is possible
- preserve the carved dark stone, gold trim, purple gem, green/blue/purple plate material language from the attached reference

Families required for this screen:
{families}

Reference layout regions:
{region_lines}

Generate text-free runtime assets only. For runtime-owned interiors, produce shell-only frames, empty wells, socket frames, or open apertures. Controls need normal, hover, pressed, disabled, and selected states where applicable with stable anchors.
"""
    prompt_path.write_text(prompt, encoding="utf-8")
    instructions_path.write_text(
        f"""# Native Image Generation Instructions - {slug}

Use Codex-native image generation only. Do not create or use legacy request manifests.

Attach/reference these inputs in the Codex thread:

- `C:\\UE\\T66\\UI\\screens\\main_menu\\reference\\canonical_reference_1920x1080.png`
- `C:\\UE\\T66\\UI\\screens\\{slug}\\reference\\canonical_reference_1920x1080.png`
- `C:\\UE\\T66\\UI\\screens\\{slug}\\layout\\reference_layout_overlay.png`
- `C:\\UE\\T66\\UI\\screens\\{slug}\\prompts\\screen_chrome_family_prompt.txt`

Save accepted outputs under:

- `C:\\UE\\T66\\UI\\screens\\{slug}\\assets\\generated\\`

Reference generation is not completion. After accepted boards are generated, continue through slicing/staging, runtime implementation, packaged capture, and packaged review unless blocked.
""",
        encoding="utf-8",
    )


def write_checklist(slug: str, pack: dict[str, object]) -> None:
    path = UI_ROOT / slug / "assets" / "sprite_family_checklist.md"
    families = "\n".join(f"- `{family}`: TODO generated board validation and slices" for family in pack["families"])
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(
        f"""# Sprite Family Checklist - {slug}

Canonical reference: `C:\\UE\\T66\\UI\\screens\\{slug}\\reference\\canonical_reference_1920x1080.png`

Required families:
{families}

Validation requirements:
- no baked runtime/localizable text in runtime slices
- generated shells keep live wells empty
- button states share stable anchors
- nine-slice margins documented before runtime integration
- no full-screen reference image is used as a runtime overlay
""",
        encoding="utf-8",
    )


def build_pack(slug: str, pack: dict[str, object]) -> None:
    regions = [(str(name), box, str(owner)) for name, box, owner in pack["regions"]]  # type: ignore[index]
    layout = {
        "screen_slug": slug,
        "screen_key": pack["screen_key"],
        "canvas_size": CANVAS,
        "canonical_reference": str(UI_ROOT / slug / "reference" / "canonical_reference_1920x1080.png"),
        "source_file": pack["source"],
        "regions": [
            {
                "name": name,
                "rect": box,
                "owner": owner,
            }
            for name, box, owner in regions
        ],
    }
    write_json(UI_ROOT / slug / "layout" / "reference_layout.json", layout)

    ownership = {
        "screen_slug": slug,
        "canvas_size": CANVAS,
        "canonical_reference": layout["canonical_reference"],
        "runtime_owned": [
            {
                "name": name,
                "rect": to_xywh((name, box, owner)),
                "content_type": owner,
                "render_contract": "live/runtime-owned; do not bake into generated art",
            }
            for name, box, owner in regions
            if owner.startswith("runtime") or owner == "shared-topbar"
        ],
        "generated_owned": [
            {
                "name": name,
                "rect": to_xywh((name, box, owner)),
                "content_type": owner,
                "render_contract": "screen-local generated shell/chrome; no localizable text",
            }
            for name, box, owner in regions
            if owner.startswith("generated")
        ],
        "families_required": pack["families"],
    }
    write_json(UI_ROOT / slug / "assets" / "content_ownership.json", ownership)
    write_json(
        UI_ROOT / slug / "assets" / "asset_manifest.json",
        {
            "screen_slug": slug,
            "canvas_size": CANVAS,
            "assets": [],
            "notes": "Sprite family board generation pending. Runtime must not use the full reference as an overlay.",
        },
    )
    write_overlay(slug, regions)
    write_prompt(slug, pack, regions)
    write_checklist(slug, pack)


def main() -> int:
    for slug, pack in PACKS.items():
        reference_path = UI_ROOT / slug / "reference" / "canonical_reference_1920x1080.png"
        if not reference_path.exists():
            raise SystemExit(f"Missing reference for {slug}: {reference_path}")
        with Image.open(reference_path) as image:
            if list(image.size) != CANVAS:
                raise SystemExit(f"{slug} reference is {image.size}, expected {CANVAS}")
        build_pack(slug, pack)
        print(f"built {slug}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

