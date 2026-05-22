"""
Create Blender source rigs, behavior-specific actions, preview frames, and FBX exports
for the Difficulty 1 / Easy mob VAT batch.

Run with Blender:
  blender --background --python "Model Generation/Rigging and Animation/Tools/create_easy_mob_vat_sources.py"
"""

from __future__ import annotations

import json
import math
from dataclasses import dataclass
from pathlib import Path

import bpy
from mathutils import Vector


PROJECT_ROOT = Path(__file__).resolve().parents[3]
RUN_ROOT = PROJECT_ROOT / "Model Generation" / "Rigging and Animation" / "Runs" / "Easy_Mob_VAT_20260514"
EXPORT_ROOT = RUN_ROOT / "Exports"
PREVIEW_ROOT = RUN_ROOT / "PreviewFrames"
BLEND_PATH = RUN_ROOT / "Easy_Mob_VAT_Source.blend"
MANIFEST_PATH = RUN_ROOT / "easy_mob_vat_manifest.json"
SOURCE_ROOT = PROJECT_ROOT / "Model Generation" / "Production" / "Roster_v1"

ACTION_PREFIX = "AM_EasyMobVAT_"
MESH_PREFIX = "SKM_EasyMobVAT_"


@dataclass(frozen=True)
class MobSpec:
    enemy_id: str
    agent: str
    profile: str
    live_static_mesh: str
    live_scale: float
    row_rotation_yaw: float = 90.0

    @property
    def source_glb(self) -> Path:
        return SOURCE_ROOT / self.agent / self.enemy_id / f"{self.enemy_id}.glb"


MOBS = [
    MobSpec("Slime", "AgentA", "slime", "/Game/Characters/Mobs/Slime/SM_Slime.SM_Slime", 1.834685),
    MobSpec("CaveBat", "AgentB", "bat", "/Game/Characters/Mobs/CaveBat/SM_CaveBat.SM_CaveBat", 1.824371),
    MobSpec("BoneWalker", "AgentB", "walker", "/Game/Characters/Mobs/BoneWalker/SM_BoneWalker.SM_BoneWalker", 1.999062),
    MobSpec("RatPack", "AgentA", "swarm", "/Game/Characters/Mobs/RatPack/SM_RatPack.SM_RatPack", 1.829114),
    MobSpec("TombSpider", "AgentB", "spider", "/Game/Characters/Mobs/TombSpider/SM_TombSpider.SM_TombSpider", 1.955138),
    MobSpec("HexSlinger", "AgentA", "caster", "/Game/Characters/Mobs/HexSlinger/SM_HexSlinger.SM_HexSlinger", 1.880113),
    MobSpec("StoneSentinel", "AgentA", "sentinel", "/Game/Characters/Mobs/StoneSentinel/SM_StoneSentinel.SM_StoneSentinel", 2.089124),
    MobSpec("MimicLure", "AgentB", "mimic", "/Game/Characters/Mobs/MimicLure/SM_MimicLure.SM_MimicLure", 2.174346),
    MobSpec("BoneConjurer", "AgentA", "conjurer", "/Game/Characters/Mobs/BoneConjurer/SM_BoneConjurer.SM_BoneConjurer", 2.065492),
    MobSpec("CryptWraith", "AgentB", "wraith", "/Game/Characters/Mobs/CryptWraith/SM_CryptWraith.SM_CryptWraith", 1.973075),
]

CLIPS = {
    "Idle": 48,
    "Move": 32,
    "AttackCue": 24,
    "HitReact": 16,
    "Death": 36,
}

VIEWS = {
    "front": (Vector((0.0, -6.0, 2.2)), math.radians(70.0), 0.0),
    "side": (Vector((6.0, 0.0, 2.1)), math.radians(68.0), math.radians(90.0)),
    "three_quarter": (Vector((4.6, -4.6, 2.4)), math.radians(66.0), math.radians(45.0)),
    "gameplay": (Vector((4.5, -6.5, 5.0)), math.radians(52.0), math.radians(34.0)),
}


def ensure_dirs() -> None:
    for path in (RUN_ROOT, EXPORT_ROOT, PREVIEW_ROOT):
        path.mkdir(parents=True, exist_ok=True)


def reset_scene() -> None:
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete()
    for collection in (
        bpy.data.meshes,
        bpy.data.armatures,
        bpy.data.actions,
        bpy.data.materials,
        bpy.data.images,
        bpy.data.cameras,
        bpy.data.lights,
    ):
        for item in list(collection):
            collection.remove(item)


def setup_render_scene() -> None:
    scene = bpy.context.scene
    scene.render.engine = "BLENDER_EEVEE"
    scene.render.resolution_x = 384
    scene.render.resolution_y = 384
    scene.eevee.taa_render_samples = 32
    scene.view_settings.view_transform = "Standard"
    scene.view_settings.look = "Medium High Contrast"
    scene.world = bpy.data.worlds.new("EasyMobVAT_World") if not scene.world else scene.world
    scene.world.color = (0.78, 0.78, 0.78)

    key = bpy.data.lights.new("Key_Light", "AREA")
    key.energy = 450.0
    key.size = 4.0
    key_obj = bpy.data.objects.new("Key_Light", key)
    bpy.context.collection.objects.link(key_obj)
    key_obj.location = (2.5, -4.0, 5.5)

    fill = bpy.data.lights.new("Fill_Light", "POINT")
    fill.energy = 80.0
    fill_obj = bpy.data.objects.new("Fill_Light", fill)
    bpy.context.collection.objects.link(fill_obj)
    fill_obj.location = (-4.0, 3.0, 3.0)

    camera_data = bpy.data.cameras.new("Preview_Camera")
    camera = bpy.data.objects.new("Preview_Camera", camera_data)
    bpy.context.collection.objects.link(camera)
    scene.camera = camera


def import_glb(path: Path) -> bpy.types.Object:
    if not path.is_file():
        raise FileNotFoundError(path)
    before = set(bpy.data.objects)
    bpy.ops.import_scene.gltf(filepath=str(path))
    imported = [obj for obj in bpy.data.objects if obj not in before]
    meshes = [obj for obj in imported if obj.type == "MESH"]
    if not meshes:
        raise RuntimeError(f"No mesh imported from {path}")

    bpy.ops.object.select_all(action="DESELECT")
    for obj in meshes:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = meshes[0]
    if len(meshes) > 1:
        bpy.ops.object.join()
    mesh = bpy.context.view_layer.objects.active
    mesh.name = path.stem + "_SourceMesh"
    mesh.data.name = path.stem + "_SourceMeshData"
    bpy.ops.object.transform_apply(location=False, rotation=True, scale=True)
    return mesh


def bounds_for(obj: bpy.types.Object) -> tuple[Vector, Vector, Vector]:
    coords = [obj.matrix_world @ Vector(corner) for corner in obj.bound_box]
    min_v = Vector((min(c.x for c in coords), min(c.y for c in coords), min(c.z for c in coords)))
    max_v = Vector((max(c.x for c in coords), max(c.y for c in coords), max(c.z for c in coords)))
    center = (min_v + max_v) * 0.5
    return min_v, max_v, center


def create_armature(spec: MobSpec, mesh: bpy.types.Object) -> bpy.types.Object:
    min_v, max_v, center = bounds_for(mesh)
    size = max_v - min_v
    height = max(size.z, 0.1)
    sx = max(size.x, 0.1)
    sy = max(size.y, 0.1)

    arm_data = bpy.data.armatures.new(f"ARM_{spec.enemy_id}_EasyMobVAT")
    arm_obj = bpy.data.objects.new(f"ARM_{spec.enemy_id}_EasyMobVAT", arm_data)
    bpy.context.collection.objects.link(arm_obj)
    arm_obj.show_in_front = True

    bpy.context.view_layer.objects.active = arm_obj
    arm_obj.select_set(True)
    bpy.ops.object.mode_set(mode="EDIT")

    def add_bone(name: str, head: Vector, tail: Vector, parent_name: str | None = "root", deform: bool = True):
        bone = arm_data.edit_bones.new(name)
        bone.head = head
        bone.tail = tail
        bone.use_deform = deform
        if parent_name:
            bone.parent = arm_data.edit_bones[parent_name]
        return bone

    root = arm_data.edit_bones.new("root")
    root.head = Vector((center.x, center.y, min_v.z))
    root.tail = Vector((center.x, center.y, min_v.z + height * 0.25))
    root.use_deform = False

    add_bone("body", Vector((center.x, center.y, min_v.z + height * 0.12)), Vector((center.x, center.y, min_v.z + height * 0.92)))
    add_bone("top", Vector((center.x, center.y, min_v.z + height * 0.55)), Vector((center.x, center.y, max_v.z + height * 0.12)))
    add_bone("lower", Vector((center.x, center.y, min_v.z)), Vector((center.x, center.y, min_v.z + height * 0.38)))
    add_bone("front", Vector((center.x, center.y + sy * 0.12, min_v.z + height * 0.28)), Vector((center.x, max_v.y + sy * 0.25, min_v.z + height * 0.62)))
    add_bone("back", Vector((center.x, center.y - sy * 0.12, min_v.z + height * 0.28)), Vector((center.x, min_v.y - sy * 0.25, min_v.z + height * 0.62)))
    add_bone("left", Vector((center.x - sx * 0.12, center.y, min_v.z + height * 0.32)), Vector((min_v.x - sx * 0.18, center.y, min_v.z + height * 0.60)))
    add_bone("right", Vector((center.x + sx * 0.12, center.y, min_v.z + height * 0.32)), Vector((max_v.x + sx * 0.18, center.y, min_v.z + height * 0.60)))
    add_bone("left_low", Vector((center.x - sx * 0.18, center.y, min_v.z + height * 0.05)), Vector((min_v.x - sx * 0.20, center.y, min_v.z + height * 0.32)))
    add_bone("right_low", Vector((center.x + sx * 0.18, center.y, min_v.z + height * 0.05)), Vector((max_v.x + sx * 0.20, center.y, min_v.z + height * 0.32)))
    add_bone("left_front", Vector((center.x - sx * 0.15, center.y + sy * 0.12, min_v.z + height * 0.12)), Vector((min_v.x - sx * 0.20, max_v.y + sy * 0.18, min_v.z + height * 0.28)))
    add_bone("right_front", Vector((center.x + sx * 0.15, center.y + sy * 0.12, min_v.z + height * 0.12)), Vector((max_v.x + sx * 0.20, max_v.y + sy * 0.18, min_v.z + height * 0.28)))
    add_bone("left_back", Vector((center.x - sx * 0.15, center.y - sy * 0.12, min_v.z + height * 0.12)), Vector((min_v.x - sx * 0.20, min_v.y - sy * 0.18, min_v.z + height * 0.28)))
    add_bone("right_back", Vector((center.x + sx * 0.15, center.y - sy * 0.12, min_v.z + height * 0.12)), Vector((max_v.x + sx * 0.20, min_v.y - sy * 0.18, min_v.z + height * 0.28)))
    add_bone("wing_left", Vector((center.x - sx * 0.05, center.y, min_v.z + height * 0.55)), Vector((min_v.x - sx * 0.35, center.y, min_v.z + height * 0.62)))
    add_bone("wing_right", Vector((center.x + sx * 0.05, center.y, min_v.z + height * 0.55)), Vector((max_v.x + sx * 0.35, center.y, min_v.z + height * 0.62)))

    bpy.ops.object.mode_set(mode="OBJECT")
    mesh.parent = arm_obj
    modifier = mesh.modifiers.new("EasyMobVAT_Armature", "ARMATURE")
    modifier.object = arm_obj
    modifier.use_vertex_groups = True
    return arm_obj


def smoothstep(value: float, edge0: float, edge1: float) -> float:
    if edge0 == edge1:
        return 1.0 if value >= edge1 else 0.0
    t = max(0.0, min(1.0, (value - edge0) / (edge1 - edge0)))
    return t * t * (3.0 - 2.0 * t)


def assign_weights(spec: MobSpec, mesh: bpy.types.Object) -> dict[str, int]:
    min_v, max_v, center = bounds_for(mesh)
    size = max_v - min_v
    half_x = max(size.x * 0.5, 0.001)
    half_y = max(size.y * 0.5, 0.001)
    height = max(size.z, 0.001)
    bone_names = [
        "body", "top", "lower", "front", "back", "left", "right", "left_low", "right_low",
        "left_front", "right_front", "left_back", "right_back", "wing_left", "wing_right",
    ]
    groups = {name: mesh.vertex_groups.new(name=name) for name in bone_names}
    counts = {name: 0 for name in bone_names}

    for vertex in mesh.data.vertices:
        world = mesh.matrix_world @ vertex.co
        nx = (world.x - center.x) / half_x
        ny = (world.y - center.y) / half_y
        nz = (world.z - min_v.z) / height
        weights: dict[str, float] = {"body": 0.34}

        lower = 1.0 - smoothstep(nz, 0.20, 0.55)
        upper = smoothstep(nz, 0.45, 0.82)
        left = smoothstep(-nx, 0.10, 0.75)
        right = smoothstep(nx, 0.10, 0.75)
        front = smoothstep(ny, 0.12, 0.75)
        back = smoothstep(-ny, 0.12, 0.75)

        weights["top"] = upper * 0.52
        weights["lower"] = lower * 0.46
        weights["front"] = front * 0.46
        weights["back"] = back * 0.40
        weights["left"] = left * 0.38
        weights["right"] = right * 0.38
        weights["left_low"] = left * lower * 0.72
        weights["right_low"] = right * lower * 0.72
        weights["left_front"] = left * front * lower * 0.85
        weights["right_front"] = right * front * lower * 0.85
        weights["left_back"] = left * back * lower * 0.85
        weights["right_back"] = right * back * lower * 0.85

        if spec.profile == "bat":
            weights["wing_left"] = left * smoothstep(nz, 0.18, 0.78) * 2.20
            weights["wing_right"] = right * smoothstep(nz, 0.18, 0.78) * 2.20
            weights["left"] *= 0.14
            weights["right"] *= 0.14
        else:
            weights["wing_left"] = 0.0
            weights["wing_right"] = 0.0

        if spec.profile == "spider":
            weights["left_front"] *= 1.85
            weights["right_front"] *= 1.85
            weights["left_back"] *= 1.85
            weights["right_back"] *= 1.85
            weights["left_low"] *= 1.35
            weights["right_low"] *= 1.35
            weights["lower"] *= 0.52
        elif spec.profile in {"walker", "caster", "conjurer"}:
            weights["left_low"] *= 1.10
            weights["right_low"] *= 1.10
            weights["top"] *= 1.05
        elif spec.profile == "slime":
            weights["lower"] *= 1.58
            weights["top"] *= 0.70
            weights["left_low"] *= 0.50
            weights["right_low"] *= 0.50

        total = sum(max(0.0, value) for value in weights.values())
        if total <= 0.0:
            weights = {"body": 1.0}
            total = 1.0

        for name, value in weights.items():
            normalized = max(0.0, value) / total
            if normalized > 0.0001:
                groups[name].add([vertex.index], normalized, "ADD")
                counts[name] += 1
    return counts


def set_pose_defaults(armature: bpy.types.Object) -> None:
    for pbone in armature.pose.bones:
        pbone.rotation_mode = "XYZ"
        pbone.location = (0.0, 0.0, 0.0)
        pbone.rotation_euler = (0.0, 0.0, 0.0)
        pbone.scale = (1.0, 1.0, 1.0)


def key_bone(armature: bpy.types.Object, frame: int, name: str, loc=None, rot=None, scale=None) -> None:
    pbone = armature.pose.bones.get(name)
    if not pbone:
        return
    if loc is not None:
        pbone.location = loc
        pbone.keyframe_insert("location", frame=frame)
    if rot is not None:
        pbone.rotation_euler = rot
        pbone.keyframe_insert("rotation_euler", frame=frame)
    if scale is not None:
        pbone.scale = scale
        pbone.keyframe_insert("scale", frame=frame)


def key_all_defaults(armature: bpy.types.Object, frame: int) -> None:
    set_pose_defaults(armature)
    for pbone in armature.pose.bones:
        if pbone.name == "root":
            continue
        pbone.keyframe_insert("location", frame=frame)
        pbone.keyframe_insert("rotation_euler", frame=frame)
        pbone.keyframe_insert("scale", frame=frame)


def phase_keyframes(length: int, steps: int = 5) -> list[tuple[int, float]]:
    frames = []
    for index in range(steps):
        t = index / float(steps - 1)
        frames.append((1 + round((length - 1) * t), t * math.tau))
    return frames


def iter_action_fcurves(action: bpy.types.Action):
    yielded = False
    for fcurve in getattr(action, "fcurves", []) or []:
        yielded = True
        yield fcurve
    if not yielded and hasattr(action, "layers"):
        for layer in action.layers:
            for strip in layer.strips:
                for channelbag in strip.channelbags:
                    for fcurve in channelbag.fcurves:
                        yield fcurve


def create_action(spec: MobSpec, armature: bpy.types.Object, clip: str, length: int) -> bpy.types.Action:
    action = bpy.data.actions.new(f"{ACTION_PREFIX}{spec.enemy_id}_{clip}")
    armature.animation_data_create()
    armature.animation_data.action = action
    bpy.context.scene.frame_start = 1
    bpy.context.scene.frame_end = length
    key_all_defaults(armature, 1)
    key_all_defaults(armature, length)

    if spec.profile == "slime" and clip == "Move":
        hop_frames = 8
        for frame in range(1, length + 1):
            set_pose_defaults(armature)
            hop_u = ((frame - 1) % hop_frames) / float(hop_frames)
            travel_u = (frame - 1) / max(1.0, float(length - 1))
            lift = max(0.0, math.sin(math.pi * hop_u))
            contact = max(0.0, math.cos(math.tau * hop_u)) ** 2
            launch = max(0.0, math.sin(math.tau * hop_u))
            landing = max(0.0, -math.sin(math.tau * hop_u))
            wobble = math.sin(math.tau * travel_u)
            wobble_c = math.cos(math.tau * travel_u)

            key_bone(
                armature,
                frame,
                "body",
                loc=(0.0, 0.018 * launch - 0.012 * landing, 0.095 * lift - 0.020 * contact),
                rot=(-0.055 * launch + 0.040 * landing, 0.0, 0.035 * wobble),
                scale=(
                    1.0 + 0.18 * contact - 0.07 * lift,
                    1.0 + 0.20 * contact + 0.04 * launch - 0.05 * lift,
                    1.0 - 0.30 * contact + 0.22 * lift,
                ),
            )
            key_bone(
                armature,
                frame,
                "top",
                loc=(0.014 * wobble, 0.018 * launch, 0.080 * lift - 0.035 * contact),
                rot=(0.050 * landing - 0.035 * launch, 0.0, -0.050 * wobble_c),
                scale=(1.0 - 0.04 * lift, 1.0 - 0.04 * lift, 1.0 + 0.15 * lift),
            )
            key_bone(
                armature,
                frame,
                "front",
                loc=(0.0, 0.130 * launch + 0.045 * lift, 0.042 * lift - 0.012 * contact),
                scale=(1.0 + 0.05 * launch, 1.0 + 0.11 * launch, 1.0 - 0.08 * contact),
            )
            key_bone(
                armature,
                frame,
                "back",
                loc=(0.0, -0.085 * landing, -0.012 * contact),
                scale=(1.0 + 0.04 * landing, 1.0 + 0.08 * landing, 1.0 - 0.05 * contact),
            )
            key_bone(armature, frame, "lower", scale=(1.0 + 0.20 * contact, 1.0 + 0.16 * contact, 1.0 - 0.22 * contact))

            for pbone in armature.pose.bones:
                if pbone.name == "root":
                    continue
                pbone.keyframe_insert("location", frame=frame)
                pbone.keyframe_insert("rotation_euler", frame=frame)
                pbone.keyframe_insert("scale", frame=frame)

        for fcurve in iter_action_fcurves(action):
            for key in fcurve.keyframe_points:
                key.interpolation = "CONSTANT"
        return action

    for frame, phase in phase_keyframes(length, 6 if clip in {"Idle", "Move"} else 5):
        set_pose_defaults(armature)
        s = math.sin(phase)
        c = math.cos(phase)
        if spec.profile == "slime":
            if clip == "Idle":
                key_bone(armature, frame, "body", scale=(1.0 + 0.035 * s, 1.0 + 0.035 * s, 1.0 - 0.055 * s))
                key_bone(armature, frame, "top", loc=(0.0, 0.0, 0.018 * s))
            elif clip == "Move":
                key_bone(armature, frame, "body", scale=(1.0 - 0.06 * s, 1.0 + 0.14 * abs(s), 1.0 - 0.12 * abs(s)))
                key_bone(armature, frame, "front", loc=(0.0, 0.075 * max(0.0, s), 0.018 * max(0.0, s)))
                key_bone(armature, frame, "back", loc=(0.0, -0.050 * max(0.0, -s), -0.012 * abs(s)))
                key_bone(armature, frame, "lower", scale=(1.04, 1.10, 0.94))
            elif clip == "AttackCue":
                snap = math.sin(min(math.pi, phase * 0.75))
                key_bone(armature, frame, "body", scale=(1.0 - 0.12 * snap, 1.0 + 0.22 * snap, 1.0 - 0.16 * snap))
                key_bone(armature, frame, "front", loc=(0.0, 0.16 * snap, 0.035 * snap))
            elif clip == "HitReact":
                key_bone(armature, frame, "body", scale=(1.0 + 0.18 * s, 1.0 - 0.10 * s, 1.0 - 0.12 * abs(s)))
                key_bone(armature, frame, "top", loc=(0.035 * s, -0.025 * s, -0.025 * abs(s)))
            elif clip == "Death":
                t = (frame - 1) / max(1, length - 1)
                key_bone(armature, frame, "body", scale=(1.0 + 0.52 * t, 1.0 + 0.40 * t, 1.0 - 0.74 * t))
                key_bone(armature, frame, "top", loc=(0.0, 0.0, -0.26 * t), scale=(1.0 + 0.20 * t, 1.0 + 0.20 * t, 1.0 - 0.55 * t))
        elif spec.profile == "bat":
            amp = 1.05 if clip == "Idle" else 1.58
            if clip == "AttackCue":
                amp = 1.90
            if clip == "HitReact":
                amp = 1.15
            if clip == "Death":
                t = (frame - 1) / max(1, length - 1)
                key_bone(armature, frame, "body", loc=(0.0, 0.0, -0.25 * t), rot=(0.24 * t, 0.0, 0.18 * s))
                key_bone(armature, frame, "wing_left", loc=(-0.06 * t, 0.0, -0.08 * t), rot=(0.24 * t, 1.45 * t, -0.42))
                key_bone(armature, frame, "wing_right", loc=(0.06 * t, 0.0, -0.08 * t), rot=(0.24 * t, -1.45 * t, 0.42))
            else:
                key_bone(armature, frame, "body", loc=(0.0, 0.0, 0.085 * s), rot=(0.12 * s, 0.0, 0.055 * c))
                key_bone(armature, frame, "wing_left", loc=(-0.10 * abs(s), 0.0, 0.055 * c), rot=(0.26 * s, amp * s, -0.30 * c))
                key_bone(armature, frame, "wing_right", loc=(0.10 * abs(s), 0.0, 0.055 * c), rot=(0.26 * s, -amp * s, 0.30 * c))
                if clip == "AttackCue":
                    key_bone(armature, frame, "front", loc=(0.0, 0.11 * abs(s), -0.045 * abs(s)))
        elif spec.profile == "spider":
            walk = clip == "Move"
            leg_amp = 0.72 if walk else 0.25
            lift = 0.040 if walk else 0.014
            key_bone(armature, frame, "body", loc=(0.0, 0.018 * c if walk else 0.0, 0.030 * s), rot=(0.055 * s, 0.0, 0.045 * c))
            key_bone(armature, frame, "left_front", loc=(-0.030 * abs(s), 0.030 * c, lift * abs(s)), rot=(leg_amp * s, 0.0, 0.34 * c))
            key_bone(armature, frame, "right_back", loc=(0.030 * abs(s), -0.030 * c, lift * abs(s)), rot=(leg_amp * s, 0.0, -0.34 * c))
            key_bone(armature, frame, "right_front", loc=(0.030 * abs(c), 0.030 * s, lift * abs(c)), rot=(-leg_amp * s, 0.0, -0.34 * c))
            key_bone(armature, frame, "left_back", loc=(-0.030 * abs(c), -0.030 * s, lift * abs(c)), rot=(-leg_amp * s, 0.0, 0.34 * c))
            if clip == "AttackCue":
                key_bone(armature, frame, "front", loc=(0.0, 0.13 * abs(s), 0.055 * abs(s)), rot=(-0.25 * abs(s), 0.0, 0.0))
            elif clip == "HitReact":
                key_bone(armature, frame, "body", loc=(0.0, -0.055 * abs(s), 0.035 * abs(s)), rot=(-0.12 * s, 0.0, 0.0))
            elif clip == "Death":
                t = (frame - 1) / max(1, length - 1)
                key_bone(armature, frame, "body", loc=(0.0, 0.0, -0.22 * t), scale=(1.0, 1.0, 1.0 - 0.22 * t))
                for name in ("left_front", "left_back", "right_front", "right_back"):
                    side = -1.0 if "left" in name else 1.0
                    key_bone(armature, frame, name, rot=(0.55 * t, 0.0, side * 0.72 * t))
        else:
            caster = spec.profile in {"caster", "conjurer"}
            heavy = spec.profile == "sentinel"
            swarm = spec.profile == "swarm"
            mimic = spec.profile == "mimic"
            wraith = spec.profile == "wraith"

            if clip == "Idle":
                key_bone(armature, frame, "body", loc=(0.0, 0.0, 0.022 * s if not heavy else 0.006 * s), rot=(0.028 * s, 0.0, 0.035 * c))
                key_bone(armature, frame, "top", rot=(0.0, 0.0, 0.060 * s if caster or wraith else 0.025 * s))
                if swarm:
                    key_bone(armature, frame, "left", loc=(0.030 * s, 0.018 * c, 0.0))
                    key_bone(armature, frame, "right", loc=(-0.025 * c, -0.018 * s, 0.0))
            elif clip == "Move":
                if swarm:
                    key_bone(armature, frame, "body", loc=(0.0, 0.0, 0.018 * abs(s)), rot=(0.0, 0.0, 0.045 * s))
                    key_bone(armature, frame, "left_front", loc=(0.040 * s, 0.035 * c, 0.012 * abs(s)))
                    key_bone(armature, frame, "right_back", loc=(-0.035 * s, -0.030 * c, 0.012 * abs(c)))
                    key_bone(armature, frame, "right_front", loc=(-0.025 * c, 0.028 * s, 0.010 * abs(c)))
                    key_bone(armature, frame, "left_back", loc=(0.020 * c, -0.026 * s, 0.010 * abs(s)))
                elif heavy:
                    key_bone(armature, frame, "body", loc=(0.0, 0.0, 0.012 * abs(s)), rot=(0.035 * s, 0.0, 0.020 * c))
                    key_bone(armature, frame, "lower", loc=(0.0, 0.0, -0.006 * abs(s)))
                elif wraith:
                    key_bone(armature, frame, "body", loc=(0.0, 0.0, 0.038 * s), rot=(0.025 * s, 0.0, 0.045 * c))
                    key_bone(armature, frame, "top", loc=(0.028 * c, 0.0, 0.030 * s), rot=(0.0, 0.0, 0.12 * s))
                else:
                    amp = 0.40 if not caster else 0.22
                    key_bone(armature, frame, "body", loc=(0.0, 0.0, 0.020 * abs(s)), rot=(0.040 * s, 0.0, 0.030 * c))
                    key_bone(armature, frame, "left_low", rot=(amp * s, 0.0, 0.0))
                    key_bone(armature, frame, "right_low", rot=(-amp * s, 0.0, 0.0))
                    key_bone(armature, frame, "left", rot=(-0.20 * s, 0.0, 0.0))
                    key_bone(armature, frame, "right", rot=(0.20 * s, 0.0, 0.0))
            elif clip == "AttackCue":
                snap = abs(math.sin(phase * 0.5))
                if caster or heavy:
                    key_bone(armature, frame, "top", rot=(-0.18 * snap, 0.0, 0.20 * s), loc=(0.0, 0.05 * snap, 0.025 * snap))
                    key_bone(armature, frame, "front", loc=(0.0, 0.13 * snap, 0.02 * snap))
                elif mimic:
                    key_bone(armature, frame, "top", rot=(-0.42 * snap, 0.0, 0.0), loc=(0.0, 0.02 * snap, 0.08 * snap))
                    key_bone(armature, frame, "front", loc=(0.0, 0.16 * snap, 0.02 * snap), scale=(1.05, 1.12, 0.95))
                elif wraith:
                    key_bone(armature, frame, "top", loc=(0.0, 0.10 * snap, 0.03 * snap), rot=(-0.24 * snap, 0.0, 0.18 * s))
                    key_bone(armature, frame, "left", loc=(-0.08 * snap, 0.06 * snap, 0.0))
                    key_bone(armature, frame, "right", loc=(0.08 * snap, 0.06 * snap, 0.0))
                else:
                    key_bone(armature, frame, "body", loc=(0.0, -0.04 * snap, 0.02 * snap), rot=(-0.15 * snap, 0.0, 0.0))
                    key_bone(armature, frame, "front", loc=(0.0, 0.12 * snap, 0.025 * snap))
            elif clip == "HitReact":
                key_bone(armature, frame, "body", loc=(0.0, -0.065 * abs(s), 0.024 * abs(s)), rot=(-0.12 * s, 0.0, 0.075 * c))
                key_bone(armature, frame, "top", rot=(0.10 * s, 0.0, -0.11 * c))
                key_bone(armature, frame, "left", loc=(-0.040 * abs(s), 0.0, 0.0))
                key_bone(armature, frame, "right", loc=(0.040 * abs(s), 0.0, 0.0))
            elif clip == "Death":
                t = (frame - 1) / max(1, length - 1)
                key_bone(armature, frame, "body", loc=(0.0, -0.035 * t, -0.25 * t), rot=(-0.45 * t, 0.0, 0.20 * s * t), scale=(1.0 + 0.10 * t, 1.0, 1.0 - 0.22 * t))
                key_bone(armature, frame, "top", loc=(0.0, -0.04 * t, -0.16 * t), rot=(-0.35 * t, 0.0, 0.14 * t))
                key_bone(armature, frame, "left", loc=(-0.06 * t, 0.0, -0.06 * t), rot=(0.0, 0.0, -0.35 * t))
                key_bone(armature, frame, "right", loc=(0.06 * t, 0.0, -0.06 * t), rot=(0.0, 0.0, 0.35 * t))

        for pbone in armature.pose.bones:
            if pbone.name == "root":
                continue
            pbone.keyframe_insert("location", frame=frame)
            pbone.keyframe_insert("rotation_euler", frame=frame)
            pbone.keyframe_insert("scale", frame=frame)

    for fcurve in iter_action_fcurves(action):
        for key in fcurve.keyframe_points:
            key.interpolation = "BEZIER"
    return action


def export_mesh_fbx(armature: bpy.types.Object, mesh: bpy.types.Object, out_path: Path) -> None:
    bpy.ops.object.select_all(action="DESELECT")
    armature.select_set(True)
    mesh.select_set(True)
    bpy.context.view_layer.objects.active = armature
    bpy.ops.export_scene.fbx(
        filepath=str(out_path),
        use_selection=True,
        object_types={"ARMATURE", "MESH"},
        add_leaf_bones=False,
        bake_anim=False,
        use_mesh_modifiers=False,
    )


def export_action_fbx(armature: bpy.types.Object, mesh: bpy.types.Object, action: bpy.types.Action, out_path: Path) -> None:
    bpy.ops.object.select_all(action="DESELECT")
    armature.select_set(True)
    mesh.select_set(True)
    bpy.context.view_layer.objects.active = armature
    armature.animation_data_create()
    armature.animation_data.action = action
    if hasattr(action, "slots") and action.slots:
        armature.animation_data.action_slot = action.slots[0]
    start = int(math.floor(action.frame_range[0]))
    end = int(math.ceil(action.frame_range[1]))
    bpy.context.scene.frame_start = start
    bpy.context.scene.frame_end = end
    bpy.ops.export_scene.fbx(
        filepath=str(out_path),
        use_selection=True,
        object_types={"ARMATURE", "MESH"},
        add_leaf_bones=False,
        bake_anim=True,
        bake_anim_use_all_actions=False,
        bake_anim_use_nla_strips=False,
        bake_anim_simplify_factor=0.0,
    )


def hide_all_mobs(records, hide: bool = True) -> None:
    for record in records:
        for obj in (record["mesh"], record["armature"]):
            obj.hide_set(hide)
            obj.hide_render = hide


def aim_camera(camera: bpy.types.Object, target: Vector) -> None:
    direction = target - camera.location
    camera.rotation_euler = direction.to_track_quat("-Z", "Y").to_euler()


def render_preview_frames(records) -> dict:
    camera = bpy.context.scene.camera
    manifest = {
        "run_root": str(RUN_ROOT),
        "views": list(VIEWS.keys()),
        "items": [],
    }
    for record in records:
        spec: MobSpec = record["spec"]
        mesh = record["mesh"]
        armature = record["armature"]
        min_v, max_v, center = bounds_for(mesh)
        height = max(max_v.z - min_v.z, 0.1)
        focus = Vector((center.x, center.y, min_v.z + height * 0.50))
        radius = max(max_v.x - min_v.x, max_v.y - min_v.y, height, 0.5)

        hide_all_mobs(records, True)
        mesh.hide_set(False)
        mesh.hide_render = False
        armature.hide_set(False)
        armature.hide_render = True

        for clip, action in record["actions"].items():
            length = int(CLIPS[clip])
            frames = sorted(set([1, max(1, length // 4), max(1, length // 2), max(1, (length * 3) // 4), length]))
            item = {
                "enemy_id": spec.enemy_id,
                "clip": clip,
                "action": action.name,
                "frames": frames,
                "views": {name: [] for name in VIEWS},
            }
            armature.animation_data_create()
            armature.animation_data.action = action
            if hasattr(action, "slots") and action.slots:
                armature.animation_data.action_slot = action.slots[0]
            for view_name, (offset, _pitch, _yaw) in VIEWS.items():
                camera.location = focus + offset.normalized() * radius * 2.4
                if view_name == "gameplay":
                    camera.location = focus + Vector((radius * 1.4, -radius * 1.9, radius * 1.7))
                aim_camera(camera, focus)
                for frame in frames:
                    bpy.context.scene.frame_set(frame)
                    out_dir = PREVIEW_ROOT / spec.enemy_id
                    out_dir.mkdir(parents=True, exist_ok=True)
                    out_path = out_dir / f"{spec.enemy_id}_{clip}_{view_name}_f{frame:03d}.png"
                    bpy.context.scene.render.filepath = str(out_path)
                    bpy.ops.render.render(write_still=True)
                    item["views"][view_name].append(str(out_path))
            manifest["items"].append(item)
    (PREVIEW_ROOT / "preview_manifest.json").write_text(json.dumps(manifest, indent=2), encoding="utf-8")
    hide_all_mobs(records, False)
    return manifest


def main() -> None:
    ensure_dirs()
    reset_scene()
    setup_render_scene()

    records = []
    for spec in MOBS:
        print(f"[EasyMobVAT] Building {spec.enemy_id}")
        mesh = import_glb(spec.source_glb)
        mesh.name = f"{MESH_PREFIX}{spec.enemy_id}_Mesh"
        mesh.data.name = f"{MESH_PREFIX}{spec.enemy_id}_MeshData"
        armature = create_armature(spec, mesh)
        weight_counts = assign_weights(spec, mesh)
        actions = {clip: create_action(spec, armature, clip, length) for clip, length in CLIPS.items()}

        mob_export_dir = EXPORT_ROOT / spec.enemy_id
        mob_export_dir.mkdir(parents=True, exist_ok=True)
        mesh_export = mob_export_dir / f"{MESH_PREFIX}{spec.enemy_id}.fbx"
        export_mesh_fbx(armature, mesh, mesh_export)
        action_exports = {}
        for clip, action in actions.items():
            action_path = mob_export_dir / f"{action.name}.fbx"
            export_action_fbx(armature, mesh, action, action_path)
            action_exports[clip] = str(action_path)

        min_v, max_v, _center = bounds_for(mesh)
        records.append({
            "spec": spec,
            "mesh": mesh,
            "armature": armature,
            "actions": actions,
            "weight_counts": weight_counts,
            "mesh_export": str(mesh_export),
            "action_exports": action_exports,
            "bounds": {
                "min": [min_v.x, min_v.y, min_v.z],
                "max": [max_v.x, max_v.y, max_v.z],
            },
        })

    bpy.ops.wm.save_as_mainfile(filepath=str(BLEND_PATH))
    preview_manifest = render_preview_frames(records)

    manifest = {
        "blender_version": bpy.app.version_string,
        "run_root": str(RUN_ROOT),
        "blend_path": str(BLEND_PATH),
        "preview_manifest": str(PREVIEW_ROOT / "preview_manifest.json"),
        "export_root": str(EXPORT_ROOT),
        "clips": CLIPS,
        "views": list(VIEWS.keys()),
        "mobs": [
            {
                "enemy_id": record["spec"].enemy_id,
                "profile": record["spec"].profile,
                "source_glb": str(record["spec"].source_glb),
                "live_static_mesh": record["spec"].live_static_mesh,
                "live_scale": record["spec"].live_scale,
                "mesh_export": record["mesh_export"],
                "action_exports": record["action_exports"],
                "weight_counts": record["weight_counts"],
                "bounds": record["bounds"],
            }
            for record in records
        ],
        "preview_items": len(preview_manifest["items"]),
    }
    MANIFEST_PATH.write_text(json.dumps(manifest, indent=2), encoding="utf-8")
    print(json.dumps({"manifest": str(MANIFEST_PATH), "blend": str(BLEND_PATH), "mobs": len(records)}, indent=2))


if __name__ == "__main__":
    main()
