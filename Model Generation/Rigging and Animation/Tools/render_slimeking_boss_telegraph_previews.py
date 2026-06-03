#!/usr/bin/env python3
"""
Render Blender-first Slime King attack telegraph previews.

This is visual QA only. It produces native Blender MP4s and editable .blend
source scenes for the five current Slime King boss attack parts.
"""

from __future__ import annotations

import argparse
import json
import math
import shutil
from pathlib import Path

import bpy
from mathutils import Vector


ATTACKS = [
    {
        "id": "LeftLobe",
        "slug": "01_LeftLobe_Volley",
        "title": "Left Lobe Volley",
        "description": "Left lobe winds back, flashes, then snaps forward into a five-orb volley.",
    },
    {
        "id": "RightLobe",
        "slug": "02_RightLobe_Volley",
        "title": "Right Lobe Volley",
        "description": "Right lobe mirrors the left lobe volley with a secondary blue-green tint.",
    },
    {
        "id": "LeftBase",
        "slug": "03_LeftBase_LaneBlocker",
        "title": "Left Base Lane Blocker",
        "description": "Left base compresses and slams a cylinder blocker into the left lane.",
    },
    {
        "id": "RightBase",
        "slug": "04_RightBase_LaneBlocker",
        "title": "Right Base Lane Blocker",
        "description": "Right base compresses and slams a cylinder blocker into the right lane.",
    },
    {
        "id": "MouthCore",
        "slug": "05_MouthCore_MassiveShot",
        "title": "Mouth Core Massive Shot",
        "description": "Mouth core inflates, opens, and releases one oversized projectile.",
    },
]


BASE_PARTS = {
    "Core": {"loc": (0.0, 0.0, 1.00), "scale": (1.85, 1.25, 0.86)},
    "LeftLobe": {"loc": (-1.52, 0.02, 1.12), "scale": (0.78, 0.62, 0.58)},
    "RightLobe": {"loc": (1.52, 0.02, 1.12), "scale": (0.78, 0.62, 0.58)},
    "LeftBase": {"loc": (-0.92, -0.08, 0.44), "scale": (0.86, 0.64, 0.34)},
    "RightBase": {"loc": (0.92, -0.08, 0.44), "scale": (0.86, 0.64, 0.34)},
    "MouthCore": {"loc": (0.0, 1.18, 0.92), "scale": (0.65, 0.055, 0.24)},
}


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--out-root", required=True)
    parser.add_argument("--frames", type=int, default=96)
    parser.add_argument("--fps", type=int, default=15)
    parser.add_argument("--width", type=int, default=1280)
    parser.add_argument("--height", type=int, default=720)
    return parser.parse_args(sys_argv_after_double_dash())


def sys_argv_after_double_dash() -> list[str]:
    import sys

    if "--" in sys.argv:
        return sys.argv[sys.argv.index("--") + 1 :]
    return []


def clear_scene() -> None:
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete()
    for datablock in (
        bpy.data.meshes,
        bpy.data.materials,
        bpy.data.images,
        bpy.data.curves,
        bpy.data.cameras,
        bpy.data.lights,
    ):
        for item in list(datablock):
            if item.users == 0:
                datablock.remove(item)


def make_mat(name: str, color: tuple[float, float, float, float], strength: float = 1.0) -> bpy.types.Material:
    mat = bpy.data.materials.new(name)
    mat.diffuse_color = color
    mat.use_nodes = True
    mat.blend_method = "BLEND" if color[3] < 1.0 else "OPAQUE"
    mat.use_screen_refraction = False
    nodes = mat.node_tree.nodes
    bsdf = nodes.get("Principled BSDF")
    if bsdf:
        if "Base Color" in bsdf.inputs:
            bsdf.inputs["Base Color"].default_value = color
        if "Alpha" in bsdf.inputs:
            bsdf.inputs["Alpha"].default_value = color[3]
        if "Emission Color" in bsdf.inputs:
            bsdf.inputs["Emission Color"].default_value = color
        if "Emission Strength" in bsdf.inputs:
            bsdf.inputs["Emission Strength"].default_value = strength
    return mat


def add_sphere(name: str, loc, scale, mat: bpy.types.Material, segments: int = 24, rings: int = 12) -> bpy.types.Object:
    bpy.ops.mesh.primitive_uv_sphere_add(segments=segments, ring_count=rings, radius=1.0, location=loc)
    obj = bpy.context.object
    obj.name = name
    obj.scale = scale
    obj.data.materials.append(mat)
    return obj


def add_cylinder(
    name: str,
    loc,
    radius: float,
    depth: float,
    mat: bpy.types.Material,
    vertices: int = 32,
    rotation=(0.0, 0.0, 0.0),
) -> bpy.types.Object:
    bpy.ops.mesh.primitive_cylinder_add(vertices=vertices, radius=radius, depth=depth, location=loc, rotation=rotation)
    obj = bpy.context.object
    obj.name = name
    obj.data.materials.append(mat)
    return obj


def add_cube(name: str, loc, scale, mat: bpy.types.Material) -> bpy.types.Object:
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=loc)
    obj = bpy.context.object
    obj.name = name
    obj.scale = scale
    obj.data.materials.append(mat)
    return obj


def add_cone(name: str, loc, radius1: float, depth: float, mat: bpy.types.Material) -> bpy.types.Object:
    bpy.ops.mesh.primitive_cone_add(vertices=5, radius1=radius1, radius2=0.08, depth=depth, location=loc)
    obj = bpy.context.object
    obj.name = name
    obj.data.materials.append(mat)
    return obj


def look_at(obj: bpy.types.Object, target) -> None:
    direction = Vector(target) - obj.location
    obj.rotation_euler = direction.to_track_quat("-Z", "Y").to_euler()


def key(obj: bpy.types.Object, frame: int, loc=None, scale=None, rot=None, hide=None) -> None:
    bpy.context.scene.frame_set(frame)
    if loc is not None:
        obj.location = loc
        obj.keyframe_insert(data_path="location", frame=frame)
    if scale is not None:
        obj.scale = scale
        obj.keyframe_insert(data_path="scale", frame=frame)
    if rot is not None:
        obj.rotation_euler = rot
        obj.keyframe_insert(data_path="rotation_euler", frame=frame)
    if hide is not None:
        obj.hide_viewport = hide
        obj.hide_render = hide
        obj.keyframe_insert(data_path="hide_viewport", frame=frame)
        obj.keyframe_insert(data_path="hide_render", frame=frame)


def constant_interpolation() -> None:
    for obj in bpy.context.scene.objects:
        if obj.animation_data and obj.animation_data.action:
            action_fcurves = getattr(obj.animation_data.action, "fcurves", None)
            if action_fcurves is None:
                continue
            for fc in action_fcurves:
                for kp in fc.keyframe_points:
                    kp.interpolation = "CONSTANT"


def configure_scene(frames: int, fps: int, width: int, height: int) -> dict[str, bpy.types.Material]:
    scene = bpy.context.scene
    scene.frame_start = 1
    scene.frame_end = frames
    scene.frame_set(1)
    scene.render.fps = fps
    scene.render.resolution_x = width
    scene.render.resolution_y = height
    scene.render.resolution_percentage = 100
    engine_ids = {item.identifier for item in scene.render.bl_rna.properties["engine"].enum_items}
    scene.render.engine = "BLENDER_EEVEE_NEXT" if "BLENDER_EEVEE_NEXT" in engine_ids else "BLENDER_EEVEE"
    if hasattr(scene, "eevee"):
        scene.eevee.taa_render_samples = 16
    scene.world = bpy.data.worlds.new("SlimeKingPreviewWorld")
    scene.world.color = (0.015, 0.017, 0.022)
    scene.view_settings.view_transform = "Standard"
    scene.view_settings.look = "None"
    scene.view_settings.exposure = 0.0
    scene.view_settings.gamma = 1.0

    try:
        scene.render.image_settings.file_format = "FFMPEG"
        scene.render.ffmpeg.format = "MPEG4"
        scene.render.ffmpeg.codec = "H264"
        scene.render.ffmpeg.constant_rate_factor = "MEDIUM"
        scene.render.ffmpeg.ffmpeg_preset = "GOOD"
        scene["T66OutputMode"] = "ffmpeg"
    except TypeError:
        scene.render.image_settings.file_format = "PNG"
        scene.render.image_settings.color_mode = "RGBA"
        scene["T66OutputMode"] = "png_sequence"

    mats = {
        "slime": make_mat("M_Unlit_SlimeKing_Body", (0.34, 1.0, 0.12, 1.0), 1.25),
        "slime_dark": make_mat("M_Unlit_SlimeKing_DarkGreen", (0.10, 0.56, 0.08, 1.0), 1.1),
        "mouth": make_mat("M_Unlit_MouthBlack", (0.005, 0.004, 0.006, 1.0), 0.2),
        "crown": make_mat("M_Unlit_BoneCrown", (0.93, 0.79, 0.46, 1.0), 1.0),
        "floor": make_mat("M_Unlit_DarkFloor", (0.08, 0.09, 0.10, 1.0), 0.8),
        "grid": make_mat("M_Unlit_FloorGrid", (0.25, 0.28, 0.30, 0.65), 0.9),
        "telegraph": make_mat("M_Unlit_TelegraphYellow", (1.0, 0.78, 0.10, 0.38), 1.6),
        "telegraph_green": make_mat("M_Unlit_TelegraphGreen", (0.30, 1.0, 0.18, 0.42), 1.8),
        "telegraph_blue": make_mat("M_Unlit_TelegraphBlueGreen", (0.10, 0.95, 1.0, 0.40), 1.8),
        "projectile": make_mat("M_Unlit_GreenProjectile", (0.25, 1.0, 0.10, 1.0), 1.9),
        "projectile_blue": make_mat("M_Unlit_BlueGreenProjectile", (0.10, 0.95, 1.0, 1.0), 1.9),
        "massive": make_mat("M_Unlit_MassiveProjectile", (0.55, 1.0, 0.04, 1.0), 2.2),
        "lane": make_mat("M_Unlit_LaneWarning", (1.0, 0.20, 0.08, 0.42), 1.6),
        "blocker": make_mat("M_Unlit_LaneBlocker", (0.16, 1.0, 0.08, 1.0), 1.7),
    }
    return mats


def build_environment(mats: dict[str, bpy.types.Material]) -> None:
    add_cube("Dark preview floor", (0, 1.4, -0.035), (4.2, 4.8, 0.025), mats["floor"])
    for x in [-2.0, -1.0, 0.0, 1.0, 2.0]:
        add_cube(f"Front lane grid X {x}", (x, 1.4, 0.003), (0.012, 4.8, 0.004), mats["grid"])
    for y in [-0.8, 0.2, 1.2, 2.2, 3.2, 4.2]:
        add_cube(f"Front lane grid Y {y}", (0, y, 0.005), (4.2, 0.012, 0.004), mats["grid"])

    bpy.ops.object.camera_add(location=(4.1, 6.9, 3.6))
    cam = bpy.context.object
    cam.name = "Camera_FrontThreeQuarter_PosY"
    look_at(cam, (0.0, 0.75, 0.9))
    cam.data.type = "ORTHO"
    cam.data.ortho_scale = 5.25
    bpy.context.scene.camera = cam


def build_boss(mats: dict[str, bpy.types.Material]) -> dict[str, bpy.types.Object]:
    parts = {}
    parts["Core"] = add_sphere("BossPart_Core_Body", BASE_PARTS["Core"]["loc"], BASE_PARTS["Core"]["scale"], mats["slime"])
    parts["LeftLobe"] = add_sphere("BossPart_LeftLobe_Attack", BASE_PARTS["LeftLobe"]["loc"], BASE_PARTS["LeftLobe"]["scale"], mats["slime"])
    parts["RightLobe"] = add_sphere("BossPart_RightLobe_Attack", BASE_PARTS["RightLobe"]["loc"], BASE_PARTS["RightLobe"]["scale"], mats["slime"])
    parts["LeftBase"] = add_sphere("BossPart_LeftBase_Attack", BASE_PARTS["LeftBase"]["loc"], BASE_PARTS["LeftBase"]["scale"], mats["slime_dark"])
    parts["RightBase"] = add_sphere("BossPart_RightBase_Attack", BASE_PARTS["RightBase"]["loc"], BASE_PARTS["RightBase"]["scale"], mats["slime_dark"])
    parts["MouthCore"] = add_sphere("BossPart_MouthCore_Attack", BASE_PARTS["MouthCore"]["loc"], BASE_PARTS["MouthCore"]["scale"], mats["mouth"], segments=24, rings=8)

    for index, x in enumerate([-0.55, 0.0, 0.55]):
        crown = add_cone(f"Bone crown spike {index}", (x, 0.02, 1.94 + 0.05 * (index == 1)), 0.20, 0.56, mats["crown"])
        crown.rotation_euler[2] = math.radians(18 * index)
        parts[f"Crown{index}"] = crown
    return parts


def base_idle(parts: dict[str, bpy.types.Object], frame: int, active: str | None = None) -> None:
    beat = math.sin(frame * 0.56)
    crunch = 1.0 if frame % 4 < 2 else -1.0
    for name, data in BASE_PARTS.items():
        obj = parts[name]
        loc = Vector(data["loc"])
        scale = Vector(data["scale"])
        if name != "MouthCore":
            loc.z += 0.025 * beat
            scale.x *= 1.0 + 0.025 * crunch
            scale.y *= 1.0 - 0.020 * crunch
            scale.z *= 1.0 + 0.030 * beat
        if name == active:
            loc.z += 0.035 * crunch
        key(obj, frame, loc=loc, scale=scale)


def animate_lobe_attack(parts, mats, attack_id: str, frames: int) -> None:
    active = parts[attack_id]
    is_right = attack_id == "RightLobe"
    side = 1.0 if is_right else -1.0
    projectile_mat = mats["projectile_blue"] if is_right else mats["projectile"]
    telegraph_mat = mats["telegraph_blue"] if is_right else mats["telegraph_green"]

    aura = add_sphere("Active lobe swelling telegraph", active.location, (0.01, 0.01, 0.01), telegraph_mat, segments=18, rings=8)
    ring = add_cylinder("Projectile lane target telegraph", (side * 0.48, 3.15, 0.026), 0.74, 0.025, mats["telegraph"], vertices=48)
    key(ring, 1, scale=(0.15, 0.15, 1.0))

    projectiles = []
    for i in range(5):
        orb = add_sphere(f"Volley projectile {i+1}", (side * 1.5, 1.0, 1.2), (0.001, 0.001, 0.001), projectile_mat, segments=16, rings=8)
        projectiles.append(orb)

    for f in range(1, frames + 1):
        base_idle(parts, f, attack_id)
        t = (f - 1) / max(1, frames - 1)
        charge = min(1.0, max(0.0, (f - 8) / 28.0))
        snap = min(1.0, max(0.0, (f - 40) / 7.0))
        recover = min(1.0, max(0.0, (f - 58) / 22.0))
        base_loc = Vector(BASE_PARTS[attack_id]["loc"])
        base_scale = Vector(BASE_PARTS[attack_id]["scale"])
        loc = base_loc.copy()
        loc.x += side * (-0.20 * charge + 0.58 * snap - 0.25 * recover)
        loc.y += -0.28 * charge + 0.54 * snap - 0.16 * recover
        loc.z += 0.12 * math.sin(f * 0.9)
        scale = base_scale.copy()
        scale.x *= 1.0 + 0.34 * charge - 0.12 * snap
        scale.y *= 1.0 - 0.16 * charge + 0.32 * snap
        scale.z *= 1.0 + 0.18 * charge - 0.10 * snap
        key(active, f, loc=loc, scale=scale)

        aura_scale = 0.35 + 0.95 * min(1.0, max(0.0, (f - 8) / 34.0))
        if f > 52:
            aura_scale *= max(0.0, 1.0 - (f - 52) / 12.0)
        key(aura, f, loc=loc, scale=(aura_scale, aura_scale, aura_scale))
        ring_scale = 0.15 + 1.25 * min(1.0, max(0.0, (f - 18) / 35.0))
        if f > 58:
            ring_scale = max(0.08, ring_scale * (1.0 - (f - 58) / 26.0))
        key(ring, f, scale=(ring_scale, ring_scale, 1.0))

        for i, orb in enumerate(projectiles):
            start = 42 + i * 4
            p = min(1.0, max(0.0, (f - start) / 34.0))
            start_pos = Vector((side * 1.25, 1.10, 1.13 + 0.08 * i))
            end_pos = Vector((side * (-0.75 + 0.38 * i), 4.32, 0.42 + 0.13 * (i % 2)))
            pos = start_pos.lerp(end_pos, p)
            size = 0.001 if f < start else (0.13 + 0.08 * math.sin(p * math.pi))
            key(orb, f, loc=pos, scale=(size, size, size))


def animate_base_attack(parts, mats, attack_id: str, frames: int) -> None:
    active = parts[attack_id]
    is_right = attack_id == "RightBase"
    side = 1.0 if is_right else -1.0
    lane_x = side * 1.45

    lane_warning = add_cube("Lane floor warning strip", (lane_x, 2.3, 0.028), (0.01, 2.30, 0.012), mats["lane"])
    blocker = add_cylinder("Raised lane blocker cylinder", (lane_x, 2.55, 0.16), 0.22, 3.95, mats["blocker"], vertices=32, rotation=(math.radians(90), 0, 0))
    key(blocker, 1, loc=(lane_x, 2.55, -0.22), scale=(1.0, 1.0, 1.0))

    impact_rings = []
    for i in range(3):
        ring = add_cylinder(f"Impact pulse ring {i+1}", (lane_x, 1.05 + i * 0.78, 0.04), 0.22, 0.035, mats["telegraph"], vertices=40)
        impact_rings.append(ring)

    for f in range(1, frames + 1):
        base_idle(parts, f, attack_id)
        charge = min(1.0, max(0.0, (f - 8) / 28.0))
        slam = min(1.0, max(0.0, (f - 42) / 6.0))
        recover = min(1.0, max(0.0, (f - 62) / 24.0))
        base_loc = Vector(BASE_PARTS[attack_id]["loc"])
        base_scale = Vector(BASE_PARTS[attack_id]["scale"])
        loc = base_loc.copy()
        loc.x += side * (0.16 * charge - 0.08 * recover)
        loc.y += 0.08 * math.sin(f * 0.88)
        loc.z += 0.22 * charge - 0.34 * slam + 0.16 * recover
        scale = base_scale.copy()
        scale.x *= 1.0 + 0.20 * charge + 0.22 * slam
        scale.y *= 1.0 + 0.12 * charge
        scale.z *= 1.0 - 0.20 * charge - 0.28 * slam + 0.12 * recover
        key(active, f, loc=loc, scale=scale)

        warning_width = 0.10 + 1.06 * min(1.0, max(0.0, (f - 9) / 34.0))
        if f > 68:
            warning_width *= max(0.0, 1.0 - (f - 68) / 18.0)
        key(lane_warning, f, scale=(warning_width, 2.30, 0.012))

        rise = min(1.0, max(0.0, (f - 44) / 9.0))
        fall = min(1.0, max(0.0, (f - 78) / 16.0))
        z = -0.22 + 0.72 * rise - 0.72 * fall
        key(blocker, f, loc=(lane_x, 2.55, z))

        for i, ring in enumerate(impact_rings):
            start = 42 + i * 4
            p = min(1.0, max(0.0, (f - start) / 18.0))
            pulse = 0.01 if f < start else 0.22 + 0.78 * p
            if f > start + 18:
                pulse = 0.01
            key(ring, f, scale=(pulse, pulse, 1.0))


def animate_mouth_attack(parts, mats, frames: int) -> None:
    mouth = parts["MouthCore"]
    aura = add_sphere("Mouth core large telegraph", BASE_PARTS["MouthCore"]["loc"], (0.01, 0.01, 0.01), mats["telegraph_green"], segments=24, rings=10)
    floor_target = add_cylinder("Massive shot landing warning", (0.0, 3.2, 0.028), 0.55, 0.030, mats["lane"], vertices=56)
    huge_orb = add_sphere("Massive mouth projectile", (0.0, 1.25, 1.0), (0.001, 0.001, 0.001), mats["massive"], segments=24, rings=12)

    for f in range(1, frames + 1):
        base_idle(parts, f, "MouthCore")
        charge = min(1.0, max(0.0, (f - 6) / 44.0))
        release = min(1.0, max(0.0, (f - 58) / 10.0))
        recover = min(1.0, max(0.0, (f - 74) / 20.0))

        for name in ["Core", "LeftLobe", "RightLobe"]:
            obj = parts[name]
            base_loc = Vector(BASE_PARTS[name]["loc"])
            base_scale = Vector(BASE_PARTS[name]["scale"])
            loc = base_loc.copy()
            loc.y -= 0.10 * charge
            loc.z += 0.08 * math.sin(f * 0.52)
            scale = base_scale.copy()
            scale.x *= 1.0 + 0.05 * charge - 0.03 * release
            scale.y *= 1.0 + 0.13 * charge - 0.06 * release
            scale.z *= 1.0 + 0.16 * charge - 0.04 * release
            key(obj, f, loc=loc, scale=scale)

        base_loc = Vector(BASE_PARTS["MouthCore"]["loc"])
        loc = base_loc.copy()
        loc.y += 0.18 * charge - 0.08 * recover
        loc.z += 0.04 * math.sin(f * 1.2)
        scale = Vector(BASE_PARTS["MouthCore"]["scale"])
        scale.x *= 1.0 + 1.05 * charge - 0.25 * recover
        scale.z *= 1.0 + 0.72 * charge - 0.20 * recover
        scale.y *= 1.0 + 0.80 * charge
        key(mouth, f, loc=loc, scale=scale)

        aura_scale = 0.40 + 1.65 * charge
        if f > 66:
            aura_scale *= max(0.0, 1.0 - (f - 66) / 20.0)
        key(aura, f, loc=(0.0, 1.18, 0.98), scale=(aura_scale, aura_scale, aura_scale))

        target_scale = 0.18 + 1.55 * min(1.0, max(0.0, (f - 18) / 46.0))
        if f > 80:
            target_scale *= max(0.0, 1.0 - (f - 80) / 14.0)
        key(floor_target, f, scale=(target_scale, target_scale, 1.0))

        p = min(1.0, max(0.0, (f - 60) / 28.0))
        pos = Vector((0.0, 1.20, 1.00)).lerp(Vector((0.0, 4.35, 0.82)), p)
        size = 0.001 if f < 58 else 0.28 + 0.34 * math.sin(p * math.pi)
        key(huge_orb, f, loc=pos, scale=(size, size, size))


def add_title(title: str, mats: dict[str, bpy.types.Material]) -> None:
    font_curve = bpy.data.curves.new(f"Title_{title}", type="FONT")
    font_curve.body = title
    font_curve.align_x = "CENTER"
    font_curve.size = 0.18
    obj = bpy.data.objects.new(f"Title_{title}", font_curve)
    bpy.context.collection.objects.link(obj)
    obj.location = (0.0, -1.70, 2.70)
    obj.rotation_euler = (math.radians(62.0), 0.0, 0.0)
    obj.data.materials.append(mats["crown"])


def build_attack_scene(attack: dict, args: argparse.Namespace) -> None:
    clear_scene()
    mats = configure_scene(args.frames, args.fps, args.width, args.height)
    build_environment(mats)
    parts = build_boss(mats)
    add_title(attack["title"], mats)

    attack_id = attack["id"]
    if attack_id in {"LeftLobe", "RightLobe"}:
        animate_lobe_attack(parts, mats, attack_id, args.frames)
    elif attack_id in {"LeftBase", "RightBase"}:
        animate_base_attack(parts, mats, attack_id, args.frames)
    elif attack_id == "MouthCore":
        animate_mouth_attack(parts, mats, args.frames)
    else:
        raise ValueError(f"Unsupported attack id: {attack_id}")

    constant_interpolation()


def render_attack(attack: dict, args: argparse.Namespace, out_root: Path) -> dict:
    build_attack_scene(attack, args)
    blend_path = out_root / f"SlimeKing_{attack['slug']}.blend"
    video_path = out_root / f"SlimeKing_{attack['slug']}.mp4"
    frames_dir = out_root / f"Frames_{attack['slug']}"
    bpy.ops.wm.save_as_mainfile(filepath=str(blend_path))
    if bpy.context.scene.get("T66OutputMode") == "ffmpeg":
        bpy.context.scene.render.filepath = str(video_path)
    else:
        if frames_dir.exists():
            shutil.rmtree(frames_dir)
        frames_dir.mkdir(parents=True, exist_ok=True)
        bpy.context.scene.render.filepath = str(frames_dir / "frame_")
    bpy.ops.render.render(animation=True)
    return {
        "attack_id": attack["id"],
        "title": attack["title"],
        "description": attack["description"],
        "blend": str(blend_path),
        "video": str(video_path),
        "frames_dir": str(frames_dir),
        "output_mode": bpy.context.scene.get("T66OutputMode"),
    }


def main() -> None:
    args = parse_args()
    out_root = Path(args.out_root)
    out_root.mkdir(parents=True, exist_ok=True)
    outputs = []
    for attack in ATTACKS:
        outputs.append(render_attack(attack, args, out_root))
    manifest = {
        "tool": "render_slimeking_boss_telegraph_previews.py",
        "blender_version": bpy.app.version_string,
        "fps": args.fps,
        "frames": args.frames,
        "resolution": [args.width, args.height],
        "front_axis": "+Y",
        "scope": "Blender visual QA only; no Unreal import or gameplay wiring.",
        "outputs": outputs,
    }
    (out_root / "SlimeKing_TelegraphPreview_manifest.json").write_text(json.dumps(manifest, indent=2), encoding="utf-8")
    print(json.dumps(manifest, indent=2))


if __name__ == "__main__":
    main()
