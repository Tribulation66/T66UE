#!/usr/bin/env python3
"""
Build fresh physics-first Hero 1 Chad rig and animation source files.

This script intentionally starts from the raw Pixal3D GLB. It does not read the
old spike rig, old FBX exports, Quaternius clips, or Animated ToonStyle outputs.
"""

from __future__ import annotations

import json
import math
import shutil
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, Iterable, List, Tuple

import bpy
from mathutils import Matrix, Vector


REPO_ROOT = Path(__file__).resolve().parents[3]
RUN_ROOT = REPO_ROOT / "Model Generation" / "Runs" / "Pixal3D" / "FriendSlopProbe_Hero1Male_20260604_1415"
SOURCE_GLB = RUN_ROOT / "Outputs" / "Hero_1_Chad_Male.glb"
OUTPUT_ROOT = RUN_ROOT / "Blender" / "PhysicsFirstHero"
ANIM_ROOT = OUTPUT_ROOT / "AnimationSources"
PROOF_ROOT = OUTPUT_ROOT / "ProofRenders"

BLEND_PATH = OUTPUT_ROOT / "Hero_1_Chad_PhysicsFirst.blend"
SKELETAL_FBX_PATH = OUTPUT_ROOT / "Hero_1_Chad_PhysicsFirst_Skeletal.fbx"
MANIFEST_PATH = OUTPUT_ROOT / "physics_first_hero1_chad_manifest.json"
QA_PATH = OUTPUT_ROOT / "Hero_1_Chad_PhysicsFirst_QA.json"
REPORT_PATH = OUTPUT_ROOT / "Hero_1_Chad_PhysicsFirst_Rig_Report.md"

TARGET_HEIGHT_CM = 180.0
RAW_FRONT_TO_BLENDER_FRONT = Matrix.Rotation(math.radians(180.0), 4, "Z")


@dataclass(frozen=True)
class BoneDef:
    name: str
    head: Tuple[float, float, float]
    tail: Tuple[float, float, float]
    parent: str | None = None
    deform: bool = True


BONES: List[BoneDef] = [
    BoneDef("root", (0, 0, 0), (0, 0, 10), None, True),
    BoneDef("pelvis", (0, 0, 84), (0, 0, 101), "root"),
    BoneDef("spine_01", (0, 0, 101), (0, 0, 121), "pelvis"),
    BoneDef("spine_02", (0, 0, 121), (0, 0, 141), "spine_01"),
    BoneDef("spine_03", (0, 0, 141), (0, 0, 158), "spine_02"),
    BoneDef("neck_01", (0, 0, 158), (0, 0, 166), "spine_03"),
    BoneDef("head", (0, 0, 166), (0, 0, 184), "neck_01"),
    BoneDef("clavicle_l", (5, 0, 150), (16, 0, 146), "spine_03"),
    BoneDef("upperarm_l", (16, 0, 146), (31, 0, 108), "clavicle_l"),
    BoneDef("lowerarm_l", (31, 0, 108), (40, 0, 76), "upperarm_l"),
    BoneDef("hand_l", (40, 0, 76), (45, -2, 58), "lowerarm_l"),
    BoneDef("clavicle_r", (-5, 0, 150), (-16, 0, 146), "spine_03"),
    BoneDef("upperarm_r", (-16, 0, 146), (-31, 0, 108), "clavicle_r"),
    BoneDef("lowerarm_r", (-31, 0, 108), (-40, 0, 76), "upperarm_r"),
    BoneDef("hand_r", (-40, 0, 76), (-45, -2, 58), "lowerarm_r"),
    BoneDef("thigh_l", (8, 0, 84), (12, 0, 48), "pelvis"),
    BoneDef("calf_l", (12, 0, 48), (10, 0, 13), "thigh_l"),
    BoneDef("foot_l", (10, 0, 13), (12, -18, 5), "calf_l"),
    BoneDef("ball_l", (12, -18, 5), (12, -28, 5), "foot_l"),
    BoneDef("thigh_r", (-8, 0, 84), (-12, 0, 48), "pelvis"),
    BoneDef("calf_r", (-12, 0, 48), (-10, 0, 13), "thigh_r"),
    BoneDef("foot_r", (-10, 0, 13), (-12, -18, 5), "calf_r"),
    BoneDef("ball_r", (-12, -18, 5), (-12, -28, 5), "foot_r"),
]


def ensure_dirs() -> None:
    OUTPUT_ROOT.mkdir(parents=True, exist_ok=True)
    ANIM_ROOT.mkdir(parents=True, exist_ok=True)
    PROOF_ROOT.mkdir(parents=True, exist_ok=True)


def clear_scene() -> None:
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete()
    for collection in (bpy.data.meshes, bpy.data.armatures, bpy.data.actions, bpy.data.materials, bpy.data.images):
        for item in list(collection):
            if item.users == 0:
                collection.remove(item)


def import_source_glb() -> List[bpy.types.Object]:
    if not SOURCE_GLB.exists():
        raise FileNotFoundError(f"Raw source GLB missing: {SOURCE_GLB}")

    bpy.ops.import_scene.gltf(filepath=str(SOURCE_GLB))
    meshes = [obj for obj in bpy.context.scene.objects if obj.type == "MESH"]
    if not meshes:
        raise RuntimeError(f"No mesh objects imported from {SOURCE_GLB}")

    for obj in meshes:
        obj.name = f"Hero_1_Chad_PhysicsFirst_{obj.name}"
        obj.data.name = f"{obj.name}_Mesh"
    return meshes


def iter_world_points(meshes: Iterable[bpy.types.Object]) -> Iterable[Vector]:
    for obj in meshes:
        matrix = obj.matrix_world.copy()
        for vertex in obj.data.vertices:
            yield matrix @ vertex.co


def normalize_meshes(meshes: List[bpy.types.Object]) -> Dict[str, object]:
    rotated_points = [RAW_FRONT_TO_BLENDER_FRONT @ point for point in iter_world_points(meshes)]
    min_v = Vector((min(p.x for p in rotated_points), min(p.y for p in rotated_points), min(p.z for p in rotated_points)))
    max_v = Vector((max(p.x for p in rotated_points), max(p.y for p in rotated_points), max(p.z for p in rotated_points)))
    raw_height = max(max_v.z - min_v.z, 0.0001)
    scale = TARGET_HEIGHT_CM / raw_height
    center_x = (min_v.x + max_v.x) * 0.5
    center_y = (min_v.y + max_v.y) * 0.5

    for obj in meshes:
        mesh = obj.data
        matrix = obj.matrix_world.copy()
        for vertex in mesh.vertices:
            p = RAW_FRONT_TO_BLENDER_FRONT @ (matrix @ vertex.co)
            p = Vector(((p.x - center_x) * scale, (p.y - center_y) * scale, (p.z - min_v.z) * scale))
            vertex.co = p
        obj.matrix_world = Matrix.Identity(4)
        mesh.update()

    normalized_points = list(iter_world_points(meshes))
    nmin = Vector((min(p.x for p in normalized_points), min(p.y for p in normalized_points), min(p.z for p in normalized_points)))
    nmax = Vector((max(p.x for p in normalized_points), max(p.y for p in normalized_points), max(p.z for p in normalized_points)))
    return {
        "source_bounds_after_raw_front_rotation": {"min": list(min_v), "max": list(max_v)},
        "scale_to_cm": scale,
        "normalized_bounds_cm": {"min": list(nmin), "max": list(nmax)},
        "target_height_cm": TARGET_HEIGHT_CM,
        "blender_front_axis": "-Y",
        "expected_unreal_front_axis_after_fbx": "+X",
    }


def create_armature() -> bpy.types.Object:
    arm_data = bpy.data.armatures.new("A_Hero_1_Chad_PhysicsFirst")
    arm_data.display_type = "BBONE"
    arm_obj = bpy.data.objects.new("A_Hero_1_Chad_PhysicsFirst", arm_data)
    bpy.context.collection.objects.link(arm_obj)
    bpy.context.view_layer.objects.active = arm_obj
    arm_obj.select_set(True)
    bpy.ops.object.mode_set(mode="EDIT")

    edit_bones = arm_data.edit_bones
    created: Dict[str, bpy.types.EditBone] = {}
    for bone_def in BONES:
        bone = edit_bones.new(bone_def.name)
        bone.head = Vector(bone_def.head)
        bone.tail = Vector(bone_def.tail)
        bone.use_deform = bone_def.deform
        created[bone_def.name] = bone

    for bone_def in BONES:
        if bone_def.parent:
            created[bone_def.name].parent = created[bone_def.parent]
            created[bone_def.name].use_connect = False

    bpy.ops.object.mode_set(mode="OBJECT")
    arm_obj.show_in_front = True
    return arm_obj


def distance_to_segment(point: Vector, a: Vector, b: Vector) -> float:
    ab = b - a
    denom = max(ab.length_squared, 0.000001)
    t = max(0.0, min(1.0, (point - a).dot(ab) / denom))
    closest = a + ab * t
    return (point - closest).length


def choose_weight_bone(point: Vector, deform_bones: List[str], bone_lookup: Dict[str, Tuple[Vector, Vector]]) -> str:
    """Choose a deform bone for the physics-first MVP weights.

    The first pass is anatomical region routing for extremities, then a nearest
    segment fallback. This keeps chunky arms/hands usable for PhysicsAsset bodies
    without pretending to be final production skinning.
    """

    abs_x = abs(point.x)
    side = "_l" if point.x >= 0 else "_r"
    if abs_x >= 38 and 42 <= point.z <= 88:
        return f"hand{side}"
    if abs_x >= 29 and 70 <= point.z <= 118:
        return f"lowerarm{side}"
    if abs_x >= 17 and 105 <= point.z <= 150:
        return f"upperarm{side}"
    if abs_x >= 8 and 138 <= point.z <= 166:
        return f"clavicle{side}"

    return min(
        deform_bones,
        key=lambda name: distance_to_segment(point, bone_lookup[name][0], bone_lookup[name][1]),
    )


def assign_weights(meshes: List[bpy.types.Object], armature: bpy.types.Object) -> Dict[str, object]:
    bone_lookup = {bone_def.name: (Vector(bone_def.head), Vector(bone_def.tail)) for bone_def in BONES}
    deform_bones = [bone_def.name for bone_def in BONES if bone_def.name != "root"]
    total_vertices = 0
    unweighted_vertices = 0
    max_influences = 0
    assignments: Dict[str, int] = {name: 0 for name in deform_bones}

    for obj in meshes:
        for group in list(obj.vertex_groups):
            obj.vertex_groups.remove(group)
        groups = {name: obj.vertex_groups.new(name=name) for name in deform_bones}

        for vertex in obj.data.vertices:
            point = obj.matrix_world @ vertex.co
            closest_name = choose_weight_bone(point, deform_bones, bone_lookup)
            groups[closest_name].add([vertex.index], 1.0, "REPLACE")
            assignments[closest_name] += 1
            total_vertices += 1
            max_influences = max(max_influences, 1)

        obj.parent = armature
        modifier = obj.modifiers.new("PhysicsFirstArmature", "ARMATURE")
        modifier.object = armature

    return {
        "total_vertices": total_vertices,
        "unweighted_vertices": unweighted_vertices,
        "max_influences_per_vertex": max_influences,
        "influence_model": "single nearest required deform bone; physics-first MVP weights",
        "bone_vertex_assignments": assignments,
        "zero_vertex_deform_bones": [name for name, count in assignments.items() if count == 0],
    }


def set_pose(armature: bpy.types.Object, pose: Dict[str, Dict[str, Tuple[float, float, float]]]) -> None:
    bpy.context.view_layer.objects.active = armature
    if bpy.context.object.mode != "POSE":
        bpy.ops.object.mode_set(mode="POSE")
    for pose_bone in armature.pose.bones:
        pose_bone.rotation_mode = "XYZ"
        pose_bone.location = (0.0, 0.0, 0.0)
        pose_bone.rotation_euler = (0.0, 0.0, 0.0)
        pose_bone.scale = (1.0, 1.0, 1.0)
    for bone_name, transforms in pose.items():
        pose_bone = armature.pose.bones.get(bone_name)
        if not pose_bone:
            continue
        if "rot" in transforms:
            pose_bone.rotation_euler = tuple(math.radians(v) for v in transforms["rot"])
        if "loc" in transforms:
            pose_bone.location = transforms["loc"]
        if "scale" in transforms:
            pose_bone.scale = transforms["scale"]


def key_pose(armature: bpy.types.Object, frame: int, pose: Dict[str, Dict[str, Tuple[float, float, float]]]) -> None:
    bpy.context.scene.frame_set(frame)
    set_pose(armature, pose)
    for pose_bone in armature.pose.bones:
        pose_bone.keyframe_insert(data_path="location", frame=frame)
        pose_bone.keyframe_insert(data_path="rotation_euler", frame=frame)
        pose_bone.keyframe_insert(data_path="scale", frame=frame)


def create_action(
    armature: bpy.types.Object,
    name: str,
    frame_count: int,
    keyed_poses: List[Tuple[int, Dict[str, Dict[str, Tuple[float, float, float]]]]],
) -> bpy.types.Action:
    action = bpy.data.actions.new(f"AM_Hero_1_Chad_PhysicsFirst_{name}")
    armature.animation_data_create()
    armature.animation_data.action = action
    bpy.context.view_layer.objects.active = armature
    bpy.ops.object.mode_set(mode="POSE")
    for frame, pose in keyed_poses:
        key_pose(armature, frame, pose)
    action.frame_range = (1, frame_count)
    action.use_fake_user = True
    return action


def create_actions(armature: bpy.types.Object) -> Dict[str, bpy.types.Action]:
    neutral: Dict[str, Dict[str, Tuple[float, float, float]]] = {}
    idle_left = {
        "pelvis": {"loc": (0.0, 0.0, -1.0), "rot": (0, 0, 1.5)},
        "spine_01": {"rot": (0, 0, -1.0)},
        "spine_03": {"rot": (0, 0, 1.0)},
        "head": {"rot": (0, 0, -0.5)},
        "upperarm_l": {"rot": (0, -4, 2)},
        "upperarm_r": {"rot": (0, -4, -2)},
    }
    idle_right = {
        "pelvis": {"loc": (0.0, 0.0, 1.0), "rot": (0, 0, -1.5)},
        "spine_01": {"rot": (0, 0, 1.0)},
        "spine_03": {"rot": (0, 0, -1.0)},
        "head": {"rot": (0, 0, 0.5)},
        "upperarm_l": {"rot": (0, -3, -1)},
        "upperarm_r": {"rot": (0, -3, 1)},
    }
    walk_a = {
        "pelvis": {"loc": (0, 0, 2), "rot": (0, 0, 3)},
        "spine_01": {"rot": (0, 0, -2)},
        "thigh_l": {"rot": (22, 0, 0)},
        "calf_l": {"rot": (-18, 0, 0)},
        "foot_l": {"rot": (8, 0, 0)},
        "thigh_r": {"rot": (-20, 0, 0)},
        "calf_r": {"rot": (24, 0, 0)},
        "upperarm_l": {"rot": (-18, -5, 2)},
        "lowerarm_l": {"rot": (10, 0, 0)},
        "upperarm_r": {"rot": (18, -5, -2)},
        "lowerarm_r": {"rot": (10, 0, 0)},
    }
    walk_b = {
        "pelvis": {"loc": (0, 0, 2), "rot": (0, 0, -3)},
        "spine_01": {"rot": (0, 0, 2)},
        "thigh_l": {"rot": (-20, 0, 0)},
        "calf_l": {"rot": (24, 0, 0)},
        "thigh_r": {"rot": (22, 0, 0)},
        "calf_r": {"rot": (-18, 0, 0)},
        "foot_r": {"rot": (8, 0, 0)},
        "upperarm_l": {"rot": (18, -5, 2)},
        "lowerarm_l": {"rot": (10, 0, 0)},
        "upperarm_r": {"rot": (-18, -5, -2)},
        "lowerarm_r": {"rot": (10, 0, 0)},
    }
    crouch = {
        "pelvis": {"loc": (0, 0, -8), "rot": (0, 0, 0)},
        "spine_01": {"rot": (8, 0, 0)},
        "spine_03": {"rot": (-4, 0, 0)},
        "thigh_l": {"rot": (-28, 0, 4)},
        "calf_l": {"rot": (42, 0, 0)},
        "thigh_r": {"rot": (-28, 0, -4)},
        "calf_r": {"rot": (42, 0, 0)},
        "upperarm_l": {"rot": (16, -8, 4)},
        "upperarm_r": {"rot": (16, -8, -4)},
    }
    jump_up = {
        "pelvis": {"loc": (0, 0, 8), "rot": (-5, 0, 0)},
        "spine_01": {"rot": (-8, 0, 0)},
        "spine_03": {"rot": (7, 0, 0)},
        "thigh_l": {"rot": (8, 0, 2)},
        "calf_l": {"rot": (-12, 0, 0)},
        "thigh_r": {"rot": (8, 0, -2)},
        "calf_r": {"rot": (-12, 0, 0)},
        "upperarm_l": {"rot": (-34, -8, 8)},
        "upperarm_r": {"rot": (-34, -8, -8)},
    }
    leap_air = {
        "pelvis": {"loc": (0, 0, 6), "rot": (-14, 0, 0)},
        "spine_01": {"rot": (-18, 0, 0)},
        "spine_03": {"rot": (8, 0, 0)},
        "head": {"rot": (8, 0, 0)},
        "thigh_l": {"rot": (22, 0, 4)},
        "calf_l": {"rot": (-16, 0, 0)},
        "thigh_r": {"rot": (34, 0, -2)},
        "calf_r": {"rot": (-20, 0, 0)},
        "upperarm_l": {"rot": (-40, -22, 12)},
        "lowerarm_l": {"rot": (18, 0, 0)},
        "upperarm_r": {"rot": (-40, -22, -12)},
        "lowerarm_r": {"rot": (18, 0, 0)},
    }
    recover_low = {
        "pelvis": {"loc": (0, 0, -22), "rot": (0, 0, 8)},
        "spine_01": {"rot": (35, 0, 0)},
        "spine_03": {"rot": (-12, 0, 0)},
        "head": {"rot": (-10, 0, 0)},
        "upperarm_l": {"rot": (42, -12, 24)},
        "upperarm_r": {"rot": (42, -12, -24)},
        "thigh_l": {"rot": (-46, 0, 8)},
        "calf_l": {"rot": (55, 0, 0)},
        "thigh_r": {"rot": (-38, 0, -8)},
        "calf_r": {"rot": (45, 0, 0)},
    }

    actions = {
        "Idle": create_action(armature, "Idle", 60, [(1, idle_left), (30, idle_right), (60, idle_left)]),
        "Walk": create_action(armature, "Walk", 30, [(1, walk_a), (8, neutral), (15, walk_b), (23, neutral), (30, walk_a)]),
        "Jump": create_action(armature, "Jump", 34, [(1, neutral), (5, crouch), (12, jump_up), (24, jump_up), (30, crouch), (34, neutral)]),
        "Leap": create_action(armature, "Leap", 42, [(1, neutral), (6, crouch), (14, leap_air), (30, leap_air), (37, crouch), (42, neutral)]),
        "RecoverStand": create_action(armature, "RecoverStand", 45, [(1, recover_low), (18, crouch), (45, neutral)]),
        "GetUp_Back": create_action(armature, "GetUp_Back", 50, [(1, recover_low), (22, crouch), (50, neutral)]),
        "GetUp_Front": create_action(armature, "GetUp_Front", 50, [(1, recover_low), (18, crouch), (50, neutral)]),
    }
    bpy.ops.object.mode_set(mode="OBJECT")
    return actions


def export_skeletal(meshes: List[bpy.types.Object], armature: bpy.types.Object) -> None:
    bpy.ops.object.mode_set(mode="OBJECT")
    bpy.ops.object.select_all(action="DESELECT")
    armature.select_set(True)
    for obj in meshes:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = armature
    bpy.ops.export_scene.fbx(
        filepath=str(SKELETAL_FBX_PATH),
        use_selection=True,
        object_types={"ARMATURE", "MESH"},
        add_leaf_bones=False,
        bake_anim=False,
        apply_unit_scale=False,
        use_space_transform=True,
        axis_forward="-Y",
        axis_up="Z",
    )


def export_action(armature: bpy.types.Object, action_name: str, action: bpy.types.Action) -> Path:
    path = ANIM_ROOT / f"AM_Hero_1_Chad_PhysicsFirst_{action_name}.fbx"
    bpy.ops.object.mode_set(mode="OBJECT")
    bpy.ops.object.select_all(action="DESELECT")
    armature.select_set(True)
    bpy.context.view_layer.objects.active = armature
    armature.animation_data_create()
    armature.animation_data.action = action
    bpy.context.scene.frame_start = int(action.frame_range[0])
    bpy.context.scene.frame_end = int(action.frame_range[1])
    bpy.ops.export_scene.fbx(
        filepath=str(path),
        use_selection=True,
        object_types={"ARMATURE"},
        add_leaf_bones=False,
        bake_anim=True,
        bake_anim_use_all_bones=True,
        bake_anim_use_nla_strips=False,
        bake_anim_use_all_actions=False,
        bake_anim_force_startend_keying=True,
        apply_unit_scale=False,
        use_space_transform=True,
        axis_forward="-Y",
        axis_up="Z",
    )
    return path


def save_blend() -> None:
    bpy.ops.wm.save_as_mainfile(filepath=str(BLEND_PATH))


def look_at(obj: bpy.types.Object, target: Vector) -> None:
    direction = target - obj.location
    obj.rotation_euler = direction.to_track_quat("-Z", "Y").to_euler()


def render_proofs(meshes: List[bpy.types.Object], armature: bpy.types.Object) -> Dict[str, str]:
    rendered: Dict[str, str] = {}
    try:
        bpy.context.scene.render.engine = "BLENDER_WORKBENCH"
        bpy.context.scene.display.shading.light = "STUDIO"
        bpy.context.scene.render.resolution_x = 1400
        bpy.context.scene.render.resolution_y = 1400
        light_data = bpy.data.lights.new("PhysicsFirst_KeyLight", "AREA")
        light_obj = bpy.data.objects.new("PhysicsFirst_KeyLight", light_data)
        bpy.context.collection.objects.link(light_obj)
        light_obj.location = (220, -320, 380)
        light_data.energy = 450
        light_data.size = 5
        cam_data = bpy.data.cameras.new("PhysicsFirst_ProofCamera")
        cam_obj = bpy.data.objects.new("PhysicsFirst_ProofCamera", cam_data)
        bpy.context.collection.objects.link(cam_obj)
        bpy.context.scene.camera = cam_obj
        target = Vector((0, 0, 95))

        views = {
            "front_blender_minus_y": (0, -430, 105),
            "side_plus_x": (430, 0, 105),
            "iso": (310, -360, 210),
        }
        for name, loc in views.items():
            cam_obj.location = loc
            look_at(cam_obj, target)
            out_path = PROOF_ROOT / f"Hero_1_Chad_PhysicsFirst_{name}.png"
            bpy.context.scene.render.filepath = str(out_path)
            bpy.ops.render.render(write_still=True)
            rendered[name] = str(out_path.relative_to(REPO_ROOT))
    except Exception as exc:  # Rendering can fail on headless machines; QA records it.
        rendered["render_error"] = str(exc)
    return rendered


def validate_exports(meshes: List[bpy.types.Object], actions: Dict[str, bpy.types.Action]) -> Dict[str, object]:
    required_bones = [bone.name for bone in BONES]
    missing_files = []
    files = [SKELETAL_FBX_PATH, BLEND_PATH] + [ANIM_ROOT / f"AM_Hero_1_Chad_PhysicsFirst_{name}.fbx" for name in actions]
    for path in files:
        if not path.exists() or path.stat().st_size <= 0:
            missing_files.append(str(path.relative_to(REPO_ROOT)))

    return {
        "required_bone_count": len(required_bones),
        "required_bones": required_bones,
        "mesh_object_count": len(meshes),
        "source_glb_size_bytes": SOURCE_GLB.stat().st_size if SOURCE_GLB.exists() else 0,
        "skeletal_fbx_size_bytes": SKELETAL_FBX_PATH.stat().st_size if SKELETAL_FBX_PATH.exists() else 0,
        "animation_files": {
            name: {
                "path": str((ANIM_ROOT / f"AM_Hero_1_Chad_PhysicsFirst_{name}.fbx").relative_to(REPO_ROOT)),
                "size_bytes": (ANIM_ROOT / f"AM_Hero_1_Chad_PhysicsFirst_{name}.fbx").stat().st_size
                if (ANIM_ROOT / f"AM_Hero_1_Chad_PhysicsFirst_{name}.fbx").exists()
                else 0,
                "frame_range": [int(action.frame_range[0]), int(action.frame_range[1])],
            }
            for name, action in actions.items()
        },
        "missing_or_zero_files": missing_files,
    }


def write_report(manifest: Dict[str, object], qa: Dict[str, object]) -> None:
    lines = [
        "# Hero 1 Chad Physics-First Rig Report",
        "",
        "## Source",
        "",
        f"- Raw GLB: `{SOURCE_GLB.relative_to(REPO_ROOT)}`",
        f"- Output root: `{OUTPUT_ROOT.relative_to(REPO_ROOT)}`",
        "- Process: fresh raw FriendSlop GLB import, fresh deformation armature, fresh single-pass physics-first weights, fresh pose-target clips.",
        "- Explicitly not used: old spike rig, old FBX exports, Quaternius clips, Animated ToonStyle assets, Roll clip.",
        "",
        "## Rig",
        "",
        f"- Required bones: {qa['exports']['required_bone_count']}",
        f"- Mesh objects: {qa['exports']['mesh_object_count']}",
        f"- Vertex count: {qa['weights']['total_vertices']}",
        f"- Unweighted vertices: {qa['weights']['unweighted_vertices']}",
        f"- Max influences per vertex: {qa['weights']['max_influences_per_vertex']}",
        f"- Normalized target height: {manifest['normalization']['target_height_cm']} cm",
        f"- Blender proof/front axis: `{manifest['normalization']['blender_front_axis']}`",
        f"- Expected Unreal forward after FBX axis conversion: `{manifest['normalization']['expected_unreal_front_axis_after_fbx']}`",
        "",
        "## Clips",
        "",
    ]
    for name, info in qa["exports"]["animation_files"].items():
        lines.append(f"- `{name}`: `{info['path']}` frames {info['frame_range'][0]}-{info['frame_range'][1]}")
    lines.extend(
        [
            "",
            "## Proof Renders",
            "",
        ]
    )
    for name, path in qa["proof_renders"].items():
        lines.append(f"- `{name}`: `{path}`")
    if qa["exports"]["missing_or_zero_files"]:
        lines.extend(["", "## Blocking QA", ""])
        for missing in qa["exports"]["missing_or_zero_files"]:
            lines.append(f"- Missing or zero-byte file: `{missing}`")
    else:
        lines.extend(["", "## QA Result", "", "- Blender source generation: PASS"])
    REPORT_PATH.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> None:
    ensure_dirs()
    clear_scene()
    meshes = import_source_glb()
    normalization = normalize_meshes(meshes)
    armature = create_armature()
    weights = assign_weights(meshes, armature)
    actions = create_actions(armature)
    export_skeletal(meshes, armature)
    anim_paths = {name: export_action(armature, name, action) for name, action in actions.items()}
    save_blend()
    proof_renders = render_proofs(meshes, armature)

    manifest = {
        "source_glb": str(SOURCE_GLB.relative_to(REPO_ROOT)),
        "output_root": str(OUTPUT_ROOT.relative_to(REPO_ROOT)),
        "skeletal_fbx": str(SKELETAL_FBX_PATH.relative_to(REPO_ROOT)),
        "blend": str(BLEND_PATH.relative_to(REPO_ROOT)),
        "normalization": normalization,
        "animations": {name: str(path.relative_to(REPO_ROOT)) for name, path in anim_paths.items()},
        "retired_inputs_not_used": [
            "old spike Blender/Rigging FBXs",
            "old Idle/Walk/Jump/Roll source exports",
            "Quaternius clips",
            "Animated ToonStyle outputs",
        ],
    }
    exports = validate_exports(meshes, actions)
    qa = {
        "source": manifest["source_glb"],
        "weights": weights,
        "exports": exports,
        "proof_renders": proof_renders,
        "result": "PASS"
        if not exports["missing_or_zero_files"]
        and weights["unweighted_vertices"] == 0
        and not weights["zero_vertex_deform_bones"]
        else "FAIL",
    }
    MANIFEST_PATH.write_text(json.dumps(manifest, indent=2), encoding="utf-8")
    QA_PATH.write_text(json.dumps(qa, indent=2), encoding="utf-8")
    write_report(manifest, qa)
    print(json.dumps({"manifest": str(MANIFEST_PATH), "qa": str(QA_PATH), "result": qa["result"]}, indent=2))


if __name__ == "__main__":
    main()
