"""
Prepare Stage 01 inputs for WorldNpcInteractablesRetroBatch01.

The script reads MissingInteractablesManifest.json, writes prompt files, copies
valid existing Batch01 source/raw artifacts into the new row-ID layout, and
creates clean procedural source plates for rows that need new Trellis input.
It does not call Trellis.
"""

import json
import math
import shutil
from pathlib import Path

from PIL import Image, ImageDraw


PROJECT_ROOT = Path(r"C:\UE\T66")
RUN_ROOT = PROJECT_ROOT / "Model Generation" / "Runs" / "Interactables" / "WorldNpcInteractablesRetroBatch01"
MANIFEST_PATH = RUN_ROOT / "Reports" / "MissingInteractablesManifest.json"
STAGE01_PATH = RUN_ROOT / "Reports" / "Stage01_SourceAndTrellisManifest.json"
WHITE = (255, 255, 255, 255)
INK = (45, 43, 51, 255)
SHADOW = (92, 84, 84, 255)
METAL = (125, 133, 140, 255)
SCREEN = (62, 101, 156, 255)
WOOD = (151, 91, 43, 255)
GOLD = (224, 161, 42, 255)


def _abs(rel_or_abs):
    if not rel_or_abs:
        return None
    path = Path(rel_or_abs)
    if path.is_absolute():
        return path
    return PROJECT_ROOT / path


def _rel(path):
    if not path:
        return ""
    return str(Path(path).resolve().relative_to(PROJECT_ROOT)).replace("\\", "/")


def _ensure(path):
    Path(path).parent.mkdir(parents=True, exist_ok=True)


def _copy_if_exists(src, dst):
    src_path = _abs(src)
    dst_path = _abs(dst)
    if not src_path or not dst_path or not src_path.exists():
        return False
    _ensure(dst_path)
    shutil.copy2(src_path, dst_path)
    return True


def _canvas():
    scale = 4
    return Image.new("RGBA", (1024 * scale, 1024 * scale), WHITE), scale


def _downsample(img):
    return img.resize((1024, 1024), Image.Resampling.LANCZOS).convert("RGBA")


def _poly(points, scale):
    return [(int(x * scale), int(y * scale)) for x, y in points]


def _rect(draw, xy, fill, outline=INK, width=8, scale=4, radius=0):
    xy = tuple(int(v * scale) for v in xy)
    width = int(width * scale)
    radius = int(radius * scale)
    if radius:
        draw.rounded_rectangle(xy, radius=radius, fill=fill, outline=outline, width=width)
    else:
        draw.rectangle(xy, fill=fill, outline=outline, width=width)


def _ellipse(draw, xy, fill, outline=INK, width=8, scale=4):
    xy = tuple(int(v * scale) for v in xy)
    draw.ellipse(xy, fill=fill, outline=outline, width=int(width * scale))


def _line(draw, points, fill=INK, width=8, scale=4):
    draw.line(_poly(points, scale), fill=fill, width=int(width * scale), joint="curve")


def _polygon(draw, points, fill, outline=INK, width=8, scale=4):
    draw.polygon(_poly(points, scale), fill=fill)
    draw.line(_poly(points + [points[0]], scale), fill=outline, width=int(width * scale), joint="curve")


def _cabinet_base(draw, scale, body=(45, 66, 109, 255), accent=(215, 63, 58, 255)):
    _polygon(draw, [(332, 210), (692, 210), (742, 790), (282, 790)], body, scale=scale)
    _rect(draw, (365, 275, 660, 465), SCREEN, scale=scale, radius=18)
    _rect(draw, (382, 515, 642, 610), (55, 52, 62, 255), scale=scale, radius=16)
    _ellipse(draw, (430, 542, 482, 594), accent, scale=scale)
    _ellipse(draw, (542, 542, 592, 592), (64, 188, 121, 255), scale=scale)
    _rect(draw, (338, 685, 686, 760), accent, scale=scale, radius=8)


def _draw_vehicle(row_id):
    img, scale = _canvas()
    draw = ImageDraw.Draw(img)
    _ellipse(draw, (250, 650, 430, 830), (38, 38, 42, 255), scale=scale)
    _ellipse(draw, (590, 670, 740, 820), (38, 38, 42, 255), scale=scale)
    _ellipse(draw, (305, 705, 375, 775), METAL, scale=scale)
    _ellipse(draw, (635, 715, 700, 780), METAL, scale=scale)
    _polygon(draw, [(250, 620), (355, 500), (590, 500), (730, 640), (690, 700), (280, 700)], (215, 105, 32, 255), scale=scale)
    _rect(draw, (410, 380, 575, 515), (64, 88, 102, 255), scale=scale, radius=12)
    _polygon(draw, [(590, 500), (720, 545), (780, 620), (712, 640)], (232, 139, 45, 255), scale=scale)
    _line(draw, [(470, 380), (440, 300), (535, 285), (570, 380)], width=12, scale=scale)
    return _downsample(img)


def _draw_arcade(row_id):
    img, scale = _canvas()
    draw = ImageDraw.Draw(img)
    palette = {
        "Arcade_WhackAMole": ((75, 120, 70, 255), (219, 128, 63, 255)),
        "Arcade_Topwar": ((53, 80, 142, 255), (72, 176, 215, 255)),
        "Arcade_GoldMiner": ((126, 83, 39, 255), GOLD),
        "Arcade_RuneSwipe": ((87, 52, 145, 255), (178, 88, 222, 255)),
        "Arcade_CartSwitcher": ((120, 83, 55, 255), (197, 125, 51, 255)),
        "Arcade_CrystalDash": ((43, 111, 143, 255), (98, 213, 233, 255)),
        "Arcade_PotionPour": ((53, 124, 83, 255), (117, 222, 145, 255)),
        "Arcade_RelicStack": ((132, 95, 44, 255), (218, 167, 67, 255)),
        "Arcade_ShieldParry": ((56, 88, 151, 255), (144, 181, 236, 255)),
        "Arcade_MimicMemory": ((117, 62, 112, 255), (214, 106, 164, 255)),
        "Arcade_BombSorter": ((125, 54, 45, 255), (232, 83, 58, 255)),
        "Arcade_LanternLeap": ((85, 121, 59, 255), (229, 182, 74, 255)),
        "Arcade_BladeSweep": ((125, 45, 45, 255), (213, 67, 54, 255)),
    }
    body, accent = palette.get(row_id, ((52, 72, 116, 255), (214, 79, 68, 255)))
    _cabinet_base(draw, scale, body=body, accent=accent)
    if row_id == "Arcade_WhackAMole":
        for x in (415, 515, 615):
            _ellipse(draw, (x - 28, 335, x + 28, 385), (67, 50, 44, 255), scale=scale)
        _line(draw, [(570, 300), (625, 250), (665, 287)], fill=accent, width=16, scale=scale)
    elif row_id == "Arcade_Topwar":
        for x in (430, 505, 580):
            _rect(draw, (x, 345, x + 42, 400), accent, scale=scale, radius=8)
    elif row_id == "Arcade_GoldMiner":
        _line(draw, [(515, 300), (515, 365), (455, 410)], fill=INK, width=8, scale=scale)
        _polygon(draw, [(445, 420), (480, 390), (500, 440)], GOLD, scale=scale)
    elif row_id == "Arcade_RuneSwipe":
        _polygon(draw, [(512, 310), (570, 375), (512, 440), (454, 375)], accent, scale=scale)
        _line(draw, [(460, 500), (620, 500)], fill=accent, width=16, scale=scale)
    elif row_id == "Arcade_CartSwitcher":
        _polygon(draw, [(410, 390), (620, 390), (580, 445), (450, 445)], (93, 77, 72, 255), scale=scale)
        _ellipse(draw, (440, 430, 490, 480), METAL, scale=scale)
        _ellipse(draw, (540, 430, 590, 480), METAL, scale=scale)
    elif row_id == "Arcade_CrystalDash":
        _polygon(draw, [(512, 292), (590, 405), (512, 460), (435, 405)], accent, scale=scale)
    elif row_id == "Arcade_PotionPour":
        _ellipse(draw, (425, 350, 605, 455), (65, 58, 65, 255), scale=scale)
        _rect(draw, (470, 300, 545, 370), accent, scale=scale, radius=12)
    elif row_id == "Arcade_RelicStack":
        for i, y in enumerate((405, 355, 305)):
            _rect(draw, (430 + i * 28, y, 590 + i * 28, y + 52), accent, scale=scale, radius=8)
    elif row_id == "Arcade_ShieldParry":
        _polygon(draw, [(512, 295), (600, 340), (570, 435), (512, 465), (454, 435), (424, 340)], accent, scale=scale)
    elif row_id == "Arcade_MimicMemory":
        _rect(draw, (420, 350, 610, 455), (122, 74, 38, 255), scale=scale, radius=12)
        _polygon(draw, [(435, 350), (512, 300), (595, 350)], (152, 88, 49, 255), scale=scale)
    elif row_id == "Arcade_BombSorter":
        for x in (450, 545):
            _ellipse(draw, (x, 335, x + 72, 407), (44, 43, 50, 255), scale=scale)
            _line(draw, [(x + 50, 335), (x + 72, 300)], fill=accent, width=8, scale=scale)
    elif row_id == "Arcade_LanternLeap":
        _rect(draw, (455, 315, 575, 440), (205, 134, 54, 255), scale=scale, radius=18)
        _ellipse(draw, (475, 350, 555, 430), (247, 212, 102, 255), scale=scale)
    elif row_id == "Arcade_BladeSweep":
        _ellipse(draw, (430, 310, 600, 480), (92, 88, 92, 255), scale=scale)
        for angle in range(0, 360, 90):
            rad = math.radians(angle)
            cx, cy = 515, 395
            _polygon(draw, [(cx, cy), (cx + math.cos(rad) * 115, cy + math.sin(rad) * 115), (cx + math.cos(rad + 0.3) * 45, cy + math.sin(rad + 0.3) * 45)], METAL, scale=scale)
    return _downsample(img)


def _draw_crate(row_id):
    img, scale = _canvas()
    draw = ImageDraw.Draw(img)
    _rect(draw, (285, 365, 745, 745), WOOD, scale=scale, radius=8)
    _line(draw, [(285, 365), (745, 745), (745, 365), (285, 745)], fill=(92, 53, 31, 255), width=20, scale=scale)
    for x in (360, 512, 665):
        _line(draw, [(x, 365), (x, 745)], fill=(92, 53, 31, 255), width=18, scale=scale)
    _rect(draw, (265, 335, 765, 405), (103, 61, 36, 255), scale=scale)
    return _downsample(img)


def _draw_loot_bag(row_id):
    color = {
        "LootBag_Black": (45, 45, 55, 255),
        "LootBag_Red": (178, 54, 50, 255),
        "LootBag_Yellow": (226, 174, 56, 255),
        "LootBag_White": (229, 225, 210, 255),
    }.get(row_id, (130, 120, 90, 255))
    img, scale = _canvas()
    draw = ImageDraw.Draw(img)
    side = tuple(max(0, int(c * 0.72)) for c in color[:3]) + (255,)
    light = tuple(min(255, int(c * 1.22 + 20)) for c in color[:3]) + (255,)
    _polygon(draw, [(330, 755), (385, 455), (512, 365), (645, 455), (700, 755)], side, scale=scale)
    _ellipse(draw, (300, 430, 725, 820), color, scale=scale)
    _polygon(draw, [(405, 445), (615, 445), (565, 275), (462, 275)], color, scale=scale)
    _ellipse(draw, (410, 250, 615, 360), color, scale=scale)
    _line(draw, [(385, 455), (635, 455)], fill=(82, 60, 45, 255), width=24, scale=scale)
    _line(draw, [(430, 500), (390, 720)], fill=side, width=14, scale=scale)
    _line(draw, [(595, 500), (640, 720)], fill=side, width=14, scale=scale)
    _ellipse(draw, (438, 540, 540, 660), light, outline=None, width=0, scale=scale)
    _ellipse(draw, (470, 570, 520, 628), (255, 247, 202, 255), outline=None, width=0, scale=scale)
    return _downsample(img)


def _draw_fountain(row_id):
    img, scale = _canvas()
    draw = ImageDraw.Draw(img)
    _rect(draw, (355, 660, 670, 760), (106, 126, 140, 255), scale=scale, radius=18)
    _ellipse(draw, (300, 580, 725, 710), (123, 145, 160, 255), scale=scale)
    _rect(draw, (450, 390, 570, 625), (130, 150, 160, 255), scale=scale, radius=16)
    _ellipse(draw, (370, 385, 650, 500), (142, 165, 176, 255), scale=scale)
    _ellipse(draw, (455, 255, 570, 340), (107, 185, 215, 255), scale=scale)
    _line(draw, [(512, 330), (512, 420)], fill=(87, 166, 205, 255), width=14, scale=scale)
    return _downsample(img)


def _draw_totem(row_id):
    img, scale = _canvas()
    draw = ImageDraw.Draw(img)
    y = 170
    colors = [(125, 84, 49, 255), (151, 95, 54, 255), (111, 71, 48, 255), (166, 105, 56, 255)]
    for i, color in enumerate(colors):
        top = y + i * 145
        _polygon(draw, [(370, top), (612, top), (655, top + 34), (655, top + 135), (370, top + 135)], color, scale=scale)
        _polygon(draw, [(612, top), (655, top + 34), (655, top + 135), (612, top + 112)], tuple(max(0, int(c * 0.68)) for c in color[:3]) + (255,), scale=scale)
        _ellipse(draw, (420, top + 38, 478, top + 96), (238, 179, 72, 255), scale=scale)
        _ellipse(draw, (532, top + 38, 590, top + 96), (238, 179, 72, 255), scale=scale)
        _polygon(draw, [(486, top + 80), (520, top + 80), (504, top + 112)], (77, 52, 37, 255), scale=scale)
        _line(draw, [(440, top + 116), (580, top + 116)], width=12, scale=scale)
    return _downsample(img)


def _draw_idol_altar(row_id):
    img, scale = _canvas()
    draw = ImageDraw.Draw(img)
    _polygon(draw, [(300, 735), (725, 735), (655, 610), (370, 610)], (99, 88, 86, 255), scale=scale)
    _rect(draw, (385, 505, 640, 630), (118, 103, 94, 255), scale=scale, radius=10)
    _ellipse(draw, (445, 270, 585, 430), (185, 139, 57, 255), scale=scale)
    _rect(draw, (470, 410, 560, 520), (185, 139, 57, 255), scale=scale, radius=18)
    _ellipse(draw, (465, 335, 495, 365), INK, scale=scale)
    _ellipse(draw, (535, 335, 565, 365), INK, scale=scale)
    return _downsample(img)


def _draw_vending(row_id):
    img, scale = _canvas()
    draw = ImageDraw.Draw(img)
    _rect(draw, (350, 210, 675, 790), (190, 58, 65, 255), scale=scale, radius=24)
    _rect(draw, (385, 265, 565, 480), (76, 126, 144, 255), scale=scale, radius=16)
    _rect(draw, (585, 285, 640, 500), (48, 48, 56, 255), scale=scale, radius=10)
    _rect(draw, (420, 540, 604, 705), (218, 214, 198, 255), scale=scale, radius=16)
    _rect(draw, (490, 570, 535, 675), (70, 157, 99, 255), scale=scale, radius=6)
    _rect(draw, (460, 600, 565, 645), (70, 157, 99, 255), scale=scale, radius=6)
    return _downsample(img)


def _draw_shroom(row_id):
    img, scale = _canvas()
    draw = ImageDraw.Draw(img)
    _rect(draw, (455, 445, 580, 745), (230, 218, 182, 255), scale=scale, radius=55)
    _ellipse(draw, (285, 250, 745, 545), (164, 62, 143, 255), scale=scale)
    for xy in [(390, 330, 460, 400), (540, 300, 625, 385), (620, 410, 690, 480)]:
        _ellipse(draw, xy, (242, 222, 238, 255), scale=scale)
    return _downsample(img)


def _draw_npc(row_id):
    img, scale = _canvas()
    draw = ImageDraw.Draw(img)
    if row_id == "Saint":
        robe = (216, 211, 193, 255)
        accent = (224, 190, 81, 255)
        _ellipse(draw, (390, 120, 635, 365), (0, 0, 0, 0), outline=(223, 185, 68, 255), width=18, scale=scale)
        _ellipse(draw, (438, 205, 590, 360), (216, 164, 118, 255), scale=scale)
        _ellipse(draw, (460, 250, 488, 278), INK, scale=scale)
        _ellipse(draw, (540, 250, 568, 278), INK, scale=scale)
        _polygon(draw, [(365, 375), (660, 375), (745, 790), (300, 790)], robe, scale=scale)
        _polygon(draw, [(430, 390), (590, 390), (635, 785), (390, 785)], (242, 238, 220, 255), scale=scale)
        _line(draw, [(512, 400), (512, 760)], fill=accent, width=22, scale=scale)
        _line(draw, [(382, 455), (250, 610)], width=34, scale=scale)
        _line(draw, [(645, 455), (770, 610)], width=34, scale=scale)
        _line(draw, [(258, 300), (258, 760)], fill=(88, 63, 38, 255), width=18, scale=scale)
        _polygon(draw, [(258, 255), (302, 318), (258, 382), (214, 318)], accent, scale=scale)
        _ellipse(draw, (720, 560, 810, 650), (86, 161, 101, 255), scale=scale)
    else:
        robe = (42, 112, 73, 255)
        accent = (83, 202, 126, 255)
        _ellipse(draw, (310, 180, 720, 610), (0, 0, 0, 0), outline=accent, width=38, scale=scale)
        _polygon(draw, [(640, 392), (735, 335), (700, 462)], accent, scale=scale)
        _polygon(draw, [(390, 310), (630, 310), (705, 785), (315, 785)], robe, scale=scale)
        _polygon(draw, [(445, 350), (585, 350), (625, 780), (405, 780)], (32, 86, 59, 255), scale=scale)
        _ellipse(draw, (430, 195, 595, 350), (38, 55, 52, 255), scale=scale)
        _polygon(draw, [(392, 270), (512, 160), (635, 270), (590, 368), (432, 368)], (26, 36, 38, 255), scale=scale)
        _ellipse(draw, (460, 275, 492, 307), (225, 55, 52, 255), scale=scale)
        _ellipse(draw, (535, 275, 567, 307), (225, 55, 52, 255), scale=scale)
        _line(draw, [(512, 370), (512, 750)], fill=accent, width=16, scale=scale)
        _ellipse(draw, (650, 520, 745, 615), (140, 87, 190, 255), scale=scale)
    return _downsample(img)


DRAWERS = {
    "Vehicle": _draw_vehicle,
    "Crate": _draw_crate,
    "LootBag_Black": _draw_loot_bag,
    "LootBag_Red": _draw_loot_bag,
    "LootBag_Yellow": _draw_loot_bag,
    "LootBag_White": _draw_loot_bag,
    "Fountain": _draw_fountain,
    "DifficultyTotem": _draw_totem,
    "IdolAltar": _draw_idol_altar,
    "QuickReviveVending": _draw_vending,
    "Shroom": _draw_shroom,
    "Saint": _draw_npc,
    "Ouroboros": _draw_npc,
}


def _draw_source(row_id):
    if row_id.startswith("Arcade_"):
        return _draw_arcade(row_id)
    drawer = DRAWERS.get(row_id)
    if drawer:
        return drawer(row_id)
    return _draw_arcade(row_id)


def main():
    with open(MANIFEST_PATH, "r", encoding="utf-8") as handle:
        manifest = json.load(handle)

    stage_entries = []
    for entry in manifest["entries"]:
        if entry["category"] in ("Floors", "Walls"):
            continue
        if not (entry["needs_source_image"] or entry["needs_trellis"] or entry["needs_quad_retro"]):
            continue

        prompt_path = _abs(entry["source_image"]).parent.parent.parent / "Prompts" / entry["category"] / f"{entry['row_id']}.prompt.txt"
        prompt_path = _abs(entry["prompt_used"]) if str(entry.get("prompt_used", "")).endswith(".txt") and _abs(entry["prompt_used"]).exists() else prompt_path
        target_prompt_path = _abs(entry["source_image"]).parent.parent.parent / "Prompts" / entry["category"] / f"{entry['row_id']}.prompt.txt"
        _ensure(target_prompt_path)
        prompt_text = entry["prompt_used"]
        existing_prompt = _abs(entry.get("prompt_used", "")) if str(entry.get("prompt_used", "")).endswith(".txt") else None
        if existing_prompt and existing_prompt.exists():
            prompt_text = existing_prompt.read_text(encoding="utf-8")
        target_prompt_path.write_text(prompt_text.rstrip() + "\n", encoding="utf-8", newline="\n")

        source_copied = False
        raw_copied = False
        source_status = "not_needed"
        trellis_status = "not_needed"
        if entry.get("existing_source_image") and not entry["needs_source_image"]:
            source_copied = _copy_if_exists(entry["existing_source_image"], entry["source_image"])
            source_status = "copied_existing" if source_copied else "missing_existing_source"
        elif entry["needs_source_image"]:
            source_path = _abs(entry["source_image"])
            _ensure(source_path)
            _draw_source(entry["row_id"]).save(source_path)
            source_status = "generated_procedural_source_plate"

        if entry.get("existing_raw_trellis_glb") and not entry["needs_trellis"]:
            raw_copied = _copy_if_exists(entry["existing_raw_trellis_glb"], entry["raw_trellis_glb"])
            trellis_status = "copied_existing" if raw_copied else "missing_existing_raw"
        elif entry["needs_trellis"]:
            trellis_status = "pending_trellis"

        stage_entries.append({
            "row_id": entry["row_id"],
            "category": entry["category"],
            "source_image": entry["source_image"],
            "prompt_file": _rel(target_prompt_path),
            "raw_trellis_glb": entry["raw_trellis_glb"],
            "trellis_front_render": entry["trellis_front_render"],
            "source_status": source_status,
            "trellis_status": trellis_status,
            "needs_trellis": entry["needs_trellis"],
            "needs_quad_retro": entry["needs_quad_retro"],
        })

    summary = {
        "total_stage_entries": len(stage_entries),
        "source_generated": sum(1 for item in stage_entries if item["source_status"] == "generated_procedural_source_plate"),
        "source_copied": sum(1 for item in stage_entries if item["source_status"] == "copied_existing"),
        "raw_copied": sum(1 for item in stage_entries if item["trellis_status"] == "copied_existing"),
        "pending_trellis": sum(1 for item in stage_entries if item["trellis_status"] == "pending_trellis"),
    }
    report = {
        "stage": "Stage01_SourceAndTrellis",
        "output_root": str(RUN_ROOT),
        "summary": summary,
        "entries": stage_entries,
    }
    _ensure(STAGE01_PATH)
    STAGE01_PATH.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8", newline="\n")
    print(json.dumps(summary, indent=2))


if __name__ == "__main__":
    main()
