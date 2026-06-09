#!/usr/bin/env python3
"""
Build FriendSlop raw humanoid animation FBXs on the current FriendSlop skeleton.

This reuses the accepted T66 gameplay clip roles from the Quaternius-derived
template path, but writes actions directly onto the raw FriendSlop deform bones
so Unreal imports them against SK_Hero_1_Chad_Male_FriendSlop_Skeleton.
"""

from __future__ import annotations

import json
import math
import os
from pathlib import Path

import bpy


PROJECT_ROOT = Path(os.environ.get("T66_PROJECT_ROOT", r"C:\UE\T66"))
RUN_ROOT = PROJECT_ROOT / "Model Generation" / "Runs" / "Pixal3D" / "FriendSlopProbe_Hero1Male_20260604_1415"
DEFAULT_BLEND = RUN_ROOT / "Blender" / "Rigging" / "Hero_1_Chad_Male_FriendSlop_Rig.blend"
DEFAULT_OUTPUT_ROOT = RUN_ROOT / "Blender" / "Rigging" / "AnimationSources"

SOURCE_BLEND = Path(os.environ.get("T66_FRIENDSLOP_ANIM_SOURCE_BLEND", DEFAULT_BLEND))
OUTPUT_ROOT = Path(os.environ.get("T66_FRIENDSLOP_ANIM_OUTPUT_ROOT", DEFAULT_OUTPUT_ROOT))
VISUAL_ID = os.environ.get("T66_FRIENDSLOP_ANIM_VISUAL_ID", "Hero_1_Chad_Male_FriendSlop")

ACTION_SOURCES = {
    "Idle": "Idle_No_Loop",
    "Walk": "Walk_Fwd_Loop_LegsTorsoOnly",
    "Jump": "DoubleJump_LegsTorsoOnly",
    "Roll": "Roll_LegsTorsoOnly",
}


def find_armature() -> bpy.types.Object:
    armatures = [obj for obj in bpy.data.objects if obj.type == "ARMATURE"]
    if len(armatures) != 1:
        raise RuntimeError(f"Expected exactly one armature, found {len(armatures)}: {[obj.name for obj in armatures]}")
    return armatures[0]


def select_only(objects: list[bpy.types.Object]) -> None:
    bpy.ops.object.mode_set(mode="OBJECT") if bpy.ops.object.mode_set.poll() else None
    for obj in bpy.context.scene.objects:
        obj.select_set(False)
    for obj in objects:
        obj.hide_set(False)
        obj.hide_render = False
        obj.select_set(True)
    bpy.context.view_layer.objects.active = objects[0] if objects else None


def clear_pose(rig: bpy.types.Object) -> None:
    bpy.context.view_layer.objects.active = rig
    rig.select_set(True)
    bpy.ops.object.mode_set(mode="POSE")
    for pose_bone in rig.pose.bones:
        pose_bone.location = (0.0, 0.0, 0.0)
        pose_bone.rotation_mode = "XYZ"
        pose_bone.rotation_euler = (0.0, 0.0, 0.0)
        pose_bone.scale = (1.0, 1.0, 1.0)
    bpy.ops.object.mode_set(mode="OBJECT")


def apply_pose(rig: bpy.types.Object, transforms: dict[str, dict[str, tuple[float, float, float]]]) -> None:
    clear_pose(rig)
    bpy.ops.object.mode_set(mode="POSE")
    for bone_name, values in transforms.items():
        pose_bone = rig.pose.bones.get(bone_name)
        if not pose_bone:
            continue
        pose_bone.rotation_mode = "XYZ"
        if "rot" in values:
            pose_bone.rotation_euler = values["rot"]
        if "loc" in values:
            pose_bone.location = values["loc"]
        if "scale" in values:
            pose_bone.scale = values["scale"]
    bpy.ops.object.mode_set(mode="OBJECT")
    bpy.context.view_layer.update()


def insert_keyframes(rig: bpy.types.Object, frame: int, transforms: dict[str, dict[str, tuple[float, float, float]]]) -> None:
    bpy.context.scene.frame_set(frame)
    apply_pose(rig, transforms)
    bpy.ops.object.mode_set(mode="POSE")
    keyed_names = set(transforms.keys()) | {
        "root",
        "pelvis",
        "spine_01",
        "spine_02",
        "spine_03",
        "thigh_l",
        "calf_l",
        "foot_l",
        "thigh_r",
        "calf_r",
        "foot_r",
        "upperarm_l",
        "lowerarm_l",
        "upperarm_r",
        "lowerarm_r",
    }
    for bone_name in sorted(keyed_names):
        pose_bone = rig.pose.bones.get(bone_name)
        if not pose_bone:
            continue
        pose_bone.keyframe_insert(data_path="location", frame=frame)
        pose_bone.keyframe_insert(data_path="rotation_euler", frame=frame)
        pose_bone.keyframe_insert(data_path="scale", frame=frame)
    bpy.ops.object.mode_set(mode="OBJECT")


def rebuild_action(rig: bpy.types.Object, action_name: str, frames: list[tuple[int, dict[str, dict[str, tuple[float, float, float]]]]]) -> None:
    existing = bpy.data.actions.get(action_name)
    if existing:
        bpy.data.actions.remove(existing)

    action = bpy.data.actions.new(action_name)
    action.use_fake_user = True
    if rig.animation_data is None:
        rig.animation_data_create()
    rig.animation_data.action = action
    rig.animation_data.use_nla = False

    for frame, transforms in frames:
        insert_keyframes(rig, frame, transforms)

    action.frame_start = float(frames[0][0])
    action.frame_end = float(frames[-1][0])
    action.use_frame_range = True


def build_actions(rig: bpy.types.Object) -> None:
    rebuild_action(rig, "Idle_No_Loop", [(0, {}), (40, {}), (80, {})])

    walk_a = {
        "thigh_l": {"rot": (0.34, 0.0, 0.0)},
        "calf_l": {"rot": (-0.18, 0.0, 0.0)},
        "foot_l": {"rot": (0.08, 0.0, 0.0)},
        "thigh_r": {"rot": (-0.34, 0.0, 0.0)},
        "calf_r": {"rot": (0.18, 0.0, 0.0)},
        "spine_01": {"rot": (0.0, 0.0, -0.035)},
        "spine_02": {"rot": (0.0, 0.0, -0.025)},
        "upperarm_l": {"rot": (-0.12, 0.0, 0.0)},
        "upperarm_r": {"rot": (0.12, 0.0, 0.0)},
    }
    walk_b = {
        "thigh_l": {"rot": (-0.34, 0.0, 0.0)},
        "calf_l": {"rot": (0.18, 0.0, 0.0)},
        "thigh_r": {"rot": (0.34, 0.0, 0.0)},
        "calf_r": {"rot": (-0.18, 0.0, 0.0)},
        "foot_r": {"rot": (0.08, 0.0, 0.0)},
        "spine_01": {"rot": (0.0, 0.0, 0.035)},
        "spine_02": {"rot": (0.0, 0.0, 0.025)},
        "upperarm_l": {"rot": (0.12, 0.0, 0.0)},
        "upperarm_r": {"rot": (-0.12, 0.0, 0.0)},
    }
    rebuild_action(
        rig,
        "Walk_Fwd_Loop_LegsTorsoOnly",
        [(0, walk_a), (10, {}), (20, walk_b), (30, {}), (40, walk_a)],
    )

    crouch = {
        "root": {"loc": (0.0, 0.0, -0.08)},
        "pelvis": {"rot": (0.12, 0.0, 0.0)},
        "thigh_l": {"rot": (0.38, 0.0, 0.0)},
        "calf_l": {"rot": (-0.42, 0.0, 0.0)},
        "thigh_r": {"rot": (0.38, 0.0, 0.0)},
        "calf_r": {"rot": (-0.42, 0.0, 0.0)},
    }
    airborne = {
        "root": {"loc": (0.0, 0.0, 0.22)},
        "thigh_l": {"rot": (0.16, 0.0, 0.0)},
        "calf_l": {"rot": (-0.22, 0.0, 0.0)},
        "thigh_r": {"rot": (0.10, 0.0, 0.0)},
        "calf_r": {"rot": (-0.18, 0.0, 0.0)},
    }
    rebuild_action(rig, "DoubleJump_LegsTorsoOnly", [(0, {}), (6, crouch), (14, airborne), (22, crouch), (28, {})])

    roll_a = {
        "root": {"loc": (0.0, 0.0, -0.10), "rot": (0.0, 0.0, 0.0)},
        "pelvis": {"rot": (0.55, 0.0, 0.0)},
        "spine_01": {"rot": (0.55, 0.0, 0.0)},
        "spine_02": {"rot": (0.40, 0.0, 0.0)},
        "thigh_l": {"rot": (0.55, 0.0, 0.0)},
        "thigh_r": {"rot": (0.55, 0.0, 0.0)},
    }
    roll_b = {
        "root": {"loc": (0.0, 0.0, -0.08), "rot": (math.pi, 0.0, 0.0)},
        "pelvis": {"rot": (0.35, 0.0, 0.0)},
        "spine_01": {"rot": (0.45, 0.0, 0.0)},
        "spine_02": {"rot": (0.35, 0.0, 0.0)},
    }
    rebuild_action(rig, "Roll_LegsTorsoOnly", [(0, {}), (8, roll_a), (16, roll_b), (24, roll_a), (32, {})])
    clear_pose(rig)


def export_animation(rig: bpy.types.Object, label: str, action_name: str, output_dir: Path) -> Path:
    action = bpy.data.actions.get(action_name)
    if not action:
        raise RuntimeError(f"Missing action {action_name}")
    if rig.animation_data is None:
        rig.animation_data_create()
    rig.animation_data.action = action
    rig.animation_data.use_nla = False

    start, end = action.frame_range
    bpy.context.scene.frame_start = int(math.floor(start))
    bpy.context.scene.frame_end = int(math.ceil(end))
    bpy.context.scene.frame_set(bpy.context.scene.frame_start)

    path = output_dir / f"{VISUAL_ID}_{label}.fbx"
    select_only([rig])
    bpy.ops.export_scene.fbx(
        filepath=str(path),
        use_selection=True,
        object_types={"ARMATURE"},
        add_leaf_bones=False,
        bake_anim=True,
        bake_anim_use_all_actions=False,
        bake_anim_use_nla_strips=False,
        bake_anim_force_startend_keying=True,
        use_armature_deform_only=True,
    )
    return path


def main() -> None:
    if not SOURCE_BLEND.exists():
        raise FileNotFoundError(SOURCE_BLEND)
    OUTPUT_ROOT.mkdir(parents=True, exist_ok=True)

    bpy.ops.wm.open_mainfile(filepath=str(SOURCE_BLEND))
    rig = find_armature()
    build_actions(rig)

    exported = {
        label: str(export_animation(rig, label, action_name, OUTPUT_ROOT))
        for label, action_name in ACTION_SOURCES.items()
    }
    manifest = {
        "pipeline": "FriendSlopRawHumanoidAnimationSources",
        "source_blend": str(SOURCE_BLEND),
        "output_root": str(OUTPUT_ROOT),
        "visual_id": VISUAL_ID,
        "provenance": {
            "template_method": "Quaternius-derived T66 gameplay clip roles",
            "walk_action_role": "Walk_Fwd_Loop_LegsTorsoOnly",
            "target_skeleton": "SK_Hero_1_Chad_Male_FriendSlop_Skeleton",
        },
        "animations": exported,
    }
    manifest_path = OUTPUT_ROOT / "friendslop_raw_humanoid_animation_sources_manifest.json"
    manifest_path.write_text(json.dumps(manifest, indent=2), encoding="utf-8")
    print(f"[FriendSlopRawHumanoidAnimations] wrote {manifest_path}")


if __name__ == "__main__":
    main()
