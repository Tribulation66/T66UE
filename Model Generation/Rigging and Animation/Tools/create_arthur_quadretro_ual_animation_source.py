"""
Create a UAL-driven rigged source for the live Arthur/Royal Chad QuadRetro mesh.

This is the production-oriented path for Arthur animation. It imports the same
QuadRetro GLB that feeds the selected Hero_1_Chad visual, builds a
Rigodotify/Quaternius-named deformation skeleton around that mesh, samples the
Universal Animation Library source rig, and bakes the UAL motion onto Arthur.

Run with Blender:
  blender --background --python Tools/create_arthur_quadretro_ual_animation_source.py -- --out-root Runs/Arthur_QuadRetro_UAL_Retarget_YYYYMMDD
"""

import argparse
import json
import math
import sys
from pathlib import Path

import bpy
from mathutils import Matrix, Vector


ACTION_PREFIX = "AM_Hero_1_Chad_QuadRetroUALQA_"
VISUAL_ID = "Hero_1_Chad_QuadRetroUALQA"
LIVE_VISUAL_ID = "Hero_1_Chad"
TARGET_ARMATURE_NAME = "Arthur_QuadRetro_UAL_Target_Armature"
SOURCE_ARMATURE_NAME = "UAL1_Source_Armature"
DEFAULT_QUADRETRO_GLB = (
    r"C:\UE\T66\Model Generation\Runs\Pixal3D\HeroArthur01\Post\QuadRetro"
    r"\arthur_royal_chad\Models\arthur_royal_chad_QuadRetro.glb"
)
DEFAULT_PIXELATED_TEXTURE = (
    r"C:\UE\T66\Model Generation\Runs\Pixal3D\HeroArthur01\Post\QuadRetro"
    r"\arthur_royal_chad\Textures\arthur_royal_chad_QuadRetro_Pixelated_512.png"
)
DEFAULT_UAL1_BLEND = (
    r"C:\UE\T66\Model Generation\Rigging and Animation\External\Quaternius"
    r"\Universal Animation Library Source\UAL1.blend"
)
LIVE_ROW_SCALE = 1.011123

DEFORM_BONES = [
    "root",
    "pelvis",
    "spine_01",
    "spine_02",
    "spine_03",
    "neck_01",
    "Head",
    "clavicle_l",
    "upperarm_l",
    "lowerarm_l",
    "hand_l",
    "index_01_l",
    "index_02_l",
    "index_03_l",
    "index_04_leaf_l",
    "middle_01_l",
    "middle_02_l",
    "middle_03_l",
    "middle_04_leaf_l",
    "pinky_01_l",
    "pinky_02_l",
    "pinky_03_l",
    "pinky_04_leaf_l",
    "ring_01_l",
    "ring_02_l",
    "ring_03_l",
    "ring_04_leaf_l",
    "thumb_01_l",
    "thumb_02_l",
    "thumb_03_l",
    "thumb_04_leaf_l",
    "clavicle_r",
    "upperarm_r",
    "lowerarm_r",
    "hand_r",
    "index_01_r",
    "index_02_r",
    "index_03_r",
    "index_04_leaf_r",
    "middle_01_r",
    "middle_02_r",
    "middle_03_r",
    "middle_04_leaf_r",
    "pinky_01_r",
    "pinky_02_r",
    "pinky_03_r",
    "pinky_04_leaf_r",
    "ring_01_r",
    "ring_02_r",
    "ring_03_r",
    "ring_04_leaf_r",
    "thumb_01_r",
    "thumb_02_r",
    "thumb_03_r",
    "thumb_04_leaf_r",
    "thigh_l",
    "calf_l",
    "foot_l",
    "ball_l",
    "ball_leaf_l",
    "thigh_r",
    "calf_r",
    "foot_r",
    "ball_r",
    "ball_leaf_r",
]

FINGER_CHAINS = {
    "index": ["index_01", "index_02", "index_03", "index_04_leaf"],
    "middle": ["middle_01", "middle_02", "middle_03", "middle_04_leaf"],
    "pinky": ["pinky_01", "pinky_02", "pinky_03", "pinky_04_leaf"],
    "ring": ["ring_01", "ring_02", "ring_03", "ring_04_leaf"],
    "thumb": ["thumb_01", "thumb_02", "thumb_03", "thumb_04_leaf"],
}

RETARGET_BONES = [
    "root",
    "pelvis",
    "spine_01",
    "spine_02",
    "spine_03",
    "neck_01",
    "Head",
    "clavicle_l",
    "upperarm_l",
    "lowerarm_l",
    "hand_l",
    "clavicle_r",
    "upperarm_r",
    "lowerarm_r",
    "hand_r",
    "thigh_l",
    "calf_l",
    "foot_l",
    "ball_l",
    "thigh_r",
    "calf_r",
    "foot_r",
    "ball_r",
]

DEFAULT_RETARGET_STRENGTHS = {
    "root": 0.70,
    "pelvis": 0.60,
    "spine_01": 0.48,
    "spine_02": 0.45,
    "spine_03": 0.42,
    "neck_01": 0.22,
    "Head": 0.20,
    "clavicle_l": 0.25,
    "upperarm_l": 0.28,
    "lowerarm_l": 0.25,
    "hand_l": 0.22,
    "clavicle_r": 0.25,
    "upperarm_r": 0.28,
    "lowerarm_r": 0.25,
    "hand_r": 0.22,
    "thigh_l": 0.82,
    "calf_l": 0.82,
    "foot_l": 0.72,
    "ball_l": 0.58,
    "thigh_r": 0.82,
    "calf_r": 0.82,
    "foot_r": 0.72,
    "ball_r": 0.58,
}

ACTION_STRENGTH_OVERRIDES = {
    "Idle": {
        "pelvis": 0.30,
        "spine_01": 0.28,
        "spine_02": 0.26,
        "spine_03": 0.24,
        "thigh_l": 0.24,
        "calf_l": 0.24,
        "foot_l": 0.20,
        "thigh_r": 0.24,
        "calf_r": 0.24,
        "foot_r": 0.20,
    },
    "Jump": {
        "root": 0.54,
        "pelvis": 0.50,
        "spine_01": 0.42,
        "spine_02": 0.40,
        "spine_03": 0.38,
        "upperarm_l": 0.20,
        "lowerarm_l": 0.18,
        "hand_l": 0.16,
        "upperarm_r": 0.20,
        "lowerarm_r": 0.18,
        "hand_r": 0.16,
    },
    "Roll": {
        "root": 0.48,
        "pelvis": 0.50,
        "spine_01": 0.40,
        "spine_02": 0.38,
        "spine_03": 0.36,
        "clavicle_l": 0.16,
        "upperarm_l": 0.16,
        "lowerarm_l": 0.14,
        "hand_l": 0.12,
        "clavicle_r": 0.16,
        "upperarm_r": 0.16,
        "lowerarm_r": 0.14,
        "hand_r": 0.12,
        "thigh_l": 0.62,
        "calf_l": 0.62,
        "foot_l": 0.50,
        "thigh_r": 0.62,
        "calf_r": 0.62,
        "foot_r": 0.50,
    },
    "Walk": {
        "spine_01": 0.38,
        "spine_02": 0.36,
        "spine_03": 0.34,
        "upperarm_l": 0.20,
        "lowerarm_l": 0.18,
        "hand_l": 0.16,
        "upperarm_r": 0.20,
        "lowerarm_r": 0.18,
        "hand_r": 0.16,
    },
}

ROLL_FORWARD_MIRROR_LOCAL_X_BONES = set(RETARGET_BONES)


def reset_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete()
    for collection in (
        bpy.data.meshes,
        bpy.data.armatures,
        bpy.data.actions,
        bpy.data.materials,
        bpy.data.images,
    ):
        for item in list(collection):
            collection.remove(item)


def load_ual_source(path):
    bpy.ops.wm.open_mainfile(filepath=str(path))
    armatures = [obj for obj in bpy.data.objects if obj.type == "ARMATURE"]
    if not armatures:
        raise RuntimeError(f"No source armature in {path}")
    source_armature = armatures[0]
    source_armature.name = SOURCE_ARMATURE_NAME
    source_armature.data.name = SOURCE_ARMATURE_NAME
    source_armature.hide_viewport = False
    source_armature.hide_render = True
    for obj in bpy.data.objects:
        if obj == source_armature:
            continue
        if obj.type == "MESH":
            obj.hide_viewport = True
            obj.hide_render = True
            if obj.name.startswith("WGT-") or obj.name.lower().startswith("mannequin"):
                obj.name = f"UAL1_Source_{obj.name}"
    return source_armature


def import_quadretro_mesh(path):
    before = set(bpy.data.objects)
    bpy.ops.import_scene.gltf(filepath=str(path))
    imported = [obj for obj in bpy.data.objects if obj not in before]
    meshes = [obj for obj in imported if obj.type == "MESH"]
    if not meshes:
        raise RuntimeError(f"No meshes imported from {path}")
    if len(meshes) > 1:
        bpy.ops.object.select_all(action="DESELECT")
        for obj in meshes:
            obj.select_set(True)
        bpy.context.view_layer.objects.active = meshes[0]
        bpy.ops.object.join()
        mesh = bpy.context.view_layer.objects.active
    else:
        mesh = meshes[0]
    mesh.name = "Arthur_QuadRetro_VisibleMesh"
    mesh.data.name = "Arthur_QuadRetro_VisibleMesh"
    bpy.ops.object.select_all(action="DESELECT")
    bpy.context.view_layer.objects.active = mesh
    mesh.select_set(True)
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    return mesh


def apply_uniform_mesh_scale(mesh, scale):
    if abs(scale - 1.0) < 0.00001:
        return
    for vertex in mesh.data.vertices:
        vertex.co *= scale
    mesh.data.update()


def mesh_bounds(mesh):
    bpy.context.view_layer.update()
    coords = [mesh.matrix_world @ vertex.co for vertex in mesh.data.vertices]
    return {
        "min": [min(v[i] for v in coords) for i in range(3)],
        "max": [max(v[i] for v in coords) for i in range(3)],
    }


def create_target_bones(bounds):
    min_x, min_y, min_z = bounds["min"]
    max_x, max_y, max_z = bounds["max"]
    height = max_z - min_z
    center_x = (min_x + max_x) * 0.5
    center_y = (min_y + max_y) * 0.5
    width = max_x - min_x
    depth = max_y - min_y
    hip_x = max(0.085, width * 0.07)
    shoulder_x = max(0.26, width * 0.18)
    elbow_x = max(0.39, width * 0.27)
    hand_x = max(0.48, width * 0.34)
    arm_y = -depth * 0.06
    foot_y = -depth * 0.20
    toe_y = -depth * 0.34

    def p(x, y, z):
        return (center_x + x, center_y + y, min_z + z * height)

    bones = {
        "root": (p(0.0, 0.0, 0.00), p(0.0, 0.12, 0.00), None),
        "pelvis": (p(0.0, 0.0, 0.44), p(0.0, 0.0, 0.52), "root"),
        "spine_01": (p(0.0, 0.0, 0.52), p(0.0, 0.0, 0.61), "pelvis"),
        "spine_02": (p(0.0, 0.0, 0.61), p(0.0, 0.0, 0.70), "spine_01"),
        "spine_03": (p(0.0, 0.0, 0.70), p(0.0, 0.0, 0.78), "spine_02"),
        "neck_01": (p(0.0, 0.0, 0.78), p(0.0, 0.0, 0.83), "spine_03"),
        "Head": (p(0.0, 0.0, 0.83), p(0.0, 0.0, 0.98), "neck_01"),
        "clavicle_l": (p(0.06, 0.0, 0.755), p(shoulder_x, arm_y, 0.735), "spine_03"),
        "upperarm_l": (p(shoulder_x, arm_y, 0.735), p(elbow_x, arm_y, 0.56), "clavicle_l"),
        "lowerarm_l": (p(elbow_x, arm_y, 0.56), p(hand_x, arm_y, 0.39), "upperarm_l"),
        "hand_l": (p(hand_x, arm_y, 0.39), p(hand_x, arm_y - depth * 0.02, 0.31), "lowerarm_l"),
        "clavicle_r": (p(-0.06, 0.0, 0.755), p(-shoulder_x, arm_y, 0.735), "spine_03"),
        "upperarm_r": (p(-shoulder_x, arm_y, 0.735), p(-elbow_x, arm_y, 0.56), "clavicle_r"),
        "lowerarm_r": (p(-elbow_x, arm_y, 0.56), p(-hand_x, arm_y, 0.39), "upperarm_r"),
        "hand_r": (p(-hand_x, arm_y, 0.39), p(-hand_x, arm_y - depth * 0.02, 0.31), "lowerarm_r"),
        "thigh_l": (p(hip_x, 0.0, 0.44), p(hip_x, 0.0, 0.25), "pelvis"),
        "calf_l": (p(hip_x, 0.0, 0.25), p(hip_x, 0.0, 0.075), "thigh_l"),
        "foot_l": (p(hip_x, 0.0, 0.075), p(hip_x, foot_y, 0.035), "calf_l"),
        "ball_l": (p(hip_x, foot_y, 0.035), p(hip_x, toe_y, 0.025), "foot_l"),
        "ball_leaf_l": (p(hip_x, toe_y, 0.025), p(hip_x, toe_y - depth * 0.04, 0.025), "ball_l"),
        "thigh_r": (p(-hip_x, 0.0, 0.44), p(-hip_x, 0.0, 0.25), "pelvis"),
        "calf_r": (p(-hip_x, 0.0, 0.25), p(-hip_x, 0.0, 0.075), "thigh_r"),
        "foot_r": (p(-hip_x, 0.0, 0.075), p(-hip_x, foot_y, 0.035), "calf_r"),
        "ball_r": (p(-hip_x, foot_y, 0.035), p(-hip_x, toe_y, 0.025), "foot_r"),
        "ball_leaf_r": (p(-hip_x, toe_y, 0.025), p(-hip_x, toe_y - depth * 0.04, 0.025), "ball_r"),
    }

    for side, sign in (("l", 1.0), ("r", -1.0)):
        hand_head = Vector(bones[f"hand_{side}"][0])
        hand_tail = Vector(bones[f"hand_{side}"][1])
        for chain_index, chain_names in enumerate(FINGER_CHAINS.values()):
            lateral = (chain_index - 2) * width * 0.012 * sign
            start = hand_tail + Vector((lateral, -depth * 0.01, -height * 0.008))
            previous = f"hand_{side}"
            for segment_index, base_name in enumerate(chain_names):
                name = f"{base_name}_{side}"
                end = start + Vector((lateral * 0.10, -depth * 0.012, -height * 0.010))
                bones[name] = (tuple(start), tuple(end), previous)
                start = end
                previous = name
    return bones


def create_target_armature(bounds):
    bones = create_target_bones(bounds)
    arm_data = bpy.data.armatures.new("Arthur_QuadRetro_UAL_Skeleton")
    armature = bpy.data.objects.new(TARGET_ARMATURE_NAME, arm_data)
    bpy.context.collection.objects.link(armature)
    bpy.context.view_layer.objects.active = armature
    armature.select_set(True)
    bpy.ops.object.mode_set(mode="EDIT")
    for name in DEFORM_BONES:
        if name not in bones:
            raise RuntimeError(f"Missing target bone definition: {name}")
        head, tail, parent_name = bones[name]
        bone = arm_data.edit_bones.new(name)
        bone.head = head
        bone.tail = tail
        bone.use_deform = True
        if parent_name:
            bone.parent = arm_data.edit_bones[parent_name]
            bone.use_connect = False
    bpy.ops.object.mode_set(mode="OBJECT")
    for pose_bone in armature.pose.bones:
        pose_bone.rotation_mode = "QUATERNION"
    return armature, bones


def point_segment_distance(point, a, b):
    pa = point - a
    ba = b - a
    denom = ba.dot(ba)
    if denom <= 0.000001:
        return pa.length
    t = max(0.0, min(1.0, pa.dot(ba) / denom))
    return (pa - ba * t).length


def choose_weight_bone(co, bounds, bone_vectors):
    min_x, min_y, min_z = bounds["min"]
    max_x, max_y, max_z = bounds["max"]
    height = max_z - min_z
    z_norm = (co.z - min_z) / max(height, 0.0001)
    x_center = (min_x + max_x) * 0.5
    y_center = (min_y + max_y) * 0.5
    x_rel = co.x - x_center
    y_rel = co.y - y_center
    width = max_x - min_x
    depth = max_y - min_y
    body_half_width = max(0.24, width * 0.18)
    side = "l" if x_rel >= 0 else "r"

    # The long visible prop sits far out from the body. Keep it rigidly on the
    # hand side so the prop remains intact; the arm animation is damped later.
    if 0.10 <= z_norm <= 0.85 and x_rel > width * 0.32 and abs(y_rel) > depth * 0.12:
        return "hand_l"
    if z_norm < 0.10:
        return f"foot_{side}"
    # Keep the actual legs out of the robe/skirt fallback below.
    if z_norm < 0.45 and body_half_width * 0.45 < abs(x_rel) < width * 0.30 and abs(y_rel) < depth * 0.16:
        return f"calf_{side}" if z_norm < 0.27 else f"thigh_{side}"
    # The QuadRetro remesh is thousands of small disconnected islands. Robe,
    # skirt, and rear-gear islands need explicit torso ownership or they get
    # captured by nearby thigh/hand bones and tear during UAL jump/roll poses.
    if 0.22 <= z_norm <= 0.54 and abs(x_rel) < body_half_width * 0.95 and -depth * 0.10 <= y_rel <= depth * 0.24:
        return "pelvis" if z_norm < 0.38 else "spine_01"
    if 0.24 <= z_norm <= 0.72 and y_rel > depth * 0.12 and abs(x_rel) < body_half_width * 1.40:
        if z_norm < 0.40:
            return "pelvis"
        if z_norm < 0.56:
            return "spine_01"
        return "spine_02"

    if z_norm >= 0.80 and abs(x_rel) < body_half_width * 1.55:
        return "Head"
    if z_norm >= 0.69 and abs(x_rel) < body_half_width * 1.25:
        return "spine_03"
    if z_norm >= 0.57 and abs(x_rel) < body_half_width * 1.15:
        return "spine_02"
    if z_norm >= 0.46 and abs(x_rel) < body_half_width * 1.10:
        return "spine_01"
    if z_norm < 0.45 and abs(x_rel) < body_half_width * 1.25:
        return f"calf_{side}" if z_norm < 0.27 else f"thigh_{side}"
    if abs(x_rel) > body_half_width * 1.05 and 0.20 <= z_norm <= 0.78:
        if z_norm < 0.40 or abs(y_rel) > (max_y - min_y) * 0.28:
            return f"hand_{side}"
        if z_norm < 0.57:
            return f"lowerarm_{side}"
        return f"upperarm_{side}"

    candidates = [
        "pelvis",
        "spine_01",
        "spine_02",
        "spine_03",
        "Head",
        "upperarm_l",
        "lowerarm_l",
        "hand_l",
        "upperarm_r",
        "lowerarm_r",
        "hand_r",
        "thigh_l",
        "calf_l",
        "foot_l",
        "thigh_r",
        "calf_r",
        "foot_r",
    ]
    best_name = "pelvis"
    best_dist = float("inf")
    for name in candidates:
        head, tail = bone_vectors[name]
        dist = point_segment_distance(co, head, tail)
        if dist < best_dist:
            best_name = name
            best_dist = dist
    return best_name


def bind_mesh_to_armature(mesh, armature, bones, bounds):
    for group in list(mesh.vertex_groups):
        mesh.vertex_groups.remove(group)
    groups = {name: mesh.vertex_groups.new(name=name) for name in DEFORM_BONES}
    bone_vectors = {
        name: (Vector(head), Vector(tail))
        for name, (head, tail, _parent) in bones.items()
        if name in DEFORM_BONES
    }
    counts = {name: 0 for name in DEFORM_BONES}
    for vertex in mesh.data.vertices:
        world_co = mesh.matrix_world @ vertex.co
        bone_name = choose_weight_bone(world_co, bounds, bone_vectors)
        groups[bone_name].add([vertex.index], 1.0, "REPLACE")
        counts[bone_name] += 1

    modifier = mesh.modifiers.new("Arthur_QuadRetro_UAL_Armature", "ARMATURE")
    modifier.object = armature
    mesh.parent = armature
    return counts


def clear_pose(armature):
    for pose_bone in armature.pose.bones:
        pose_bone.location = (0.0, 0.0, 0.0)
        pose_bone.rotation_quaternion = (1.0, 0.0, 0.0, 0.0)
        pose_bone.scale = (1.0, 1.0, 1.0)


def set_source_action(source_armature, action_name):
    action = bpy.data.actions.get(action_name)
    if not action:
        raise RuntimeError(f"Missing UAL action: {action_name}")
    source_armature.animation_data_create()
    source_armature.animation_data.action = action
    if action.slots:
        source_armature.animation_data.action_slot = action.slots[0]
    return action


def local_pose_delta(source_armature, bone_name):
    pose_bone = source_armature.pose.bones[bone_name]
    rest = source_armature.data.bones[bone_name].matrix_local.copy()
    parent = pose_bone.parent
    if parent and parent.name in source_armature.data.bones:
        parent_pose = parent.matrix.copy()
        parent_rest = source_armature.data.bones[parent.name].matrix_local.copy()
        local_pose = parent_pose.inverted() @ pose_bone.matrix.copy()
        local_rest = parent_rest.inverted() @ rest
    else:
        local_pose = pose_bone.matrix.copy()
        local_rest = rest
    return local_rest.inverted() @ local_pose


def retarget_strengths_for_action(target_name):
    strengths = dict(DEFAULT_RETARGET_STRENGTHS)
    strengths.update(ACTION_STRENGTH_OVERRIDES.get(target_name, {}))
    return strengths


def blend_delta(delta, strength):
    if strength >= 0.999:
        return delta
    location, rotation, scale = delta.decompose()
    location *= strength
    rotation = rotation.slerp(rotation.__class__(), 1.0 - strength)
    scale = Vector((1.0, 1.0, 1.0)).lerp(scale, strength)
    return Matrix.LocRotScale(location, rotation, scale)


def correct_roll_forward_rotation(target_name, bone_name, rotation):
    if target_name != "Roll" or bone_name not in ROLL_FORWARD_MIRROR_LOCAL_X_BONES:
        return rotation
    euler = rotation.to_euler("XYZ")
    euler.x *= -1.0
    return euler.to_quaternion()


def apply_delta_to_target(target_armature, bone_name, delta, in_place, strength, target_name=None):
    pose_bone = target_armature.pose.bones[bone_name]
    # The UAL source action is sampled as a local rest-space delta. Decompose
    # it into normal pose channels so Blender 5.1 keys the values on the action
    # rather than leaving them only in the transient matrix_basis cache.
    delta = blend_delta(delta, strength)
    location, rotation, scale = delta.decompose()
    rotation = correct_roll_forward_rotation(target_name, bone_name, rotation)
    pose_bone.location = location
    pose_bone.rotation_quaternion = rotation
    pose_bone.scale = scale
    if in_place and bone_name == "root":
        pose_bone.location.x = 0.0
        pose_bone.location.y = 0.0


def key_target_pose(target_armature, frame):
    for pose_bone in target_armature.pose.bones:
        pose_bone.keyframe_insert(data_path="location", frame=frame)
        pose_bone.keyframe_insert(data_path="rotation_quaternion", frame=frame)
        pose_bone.keyframe_insert(data_path="scale", frame=frame)


def sample_source_deltas(source_armature):
    return {
        bone_name: local_pose_delta(source_armature, bone_name)
        for bone_name in RETARGET_BONES
        if bone_name in source_armature.pose.bones
    }


def bake_source_frame(source_armature, target_armature, source_frame, target_frame, in_place, base_deltas, strengths, target_name=None):
    bpy.context.scene.frame_set(int(source_frame))
    bpy.context.view_layer.update()
    clear_pose(target_armature)
    for bone_name in RETARGET_BONES:
        if bone_name not in source_armature.pose.bones or bone_name not in target_armature.pose.bones:
            continue
        source_delta = local_pose_delta(source_armature, bone_name)
        base_delta = base_deltas.get(bone_name, Matrix.Identity(4))
        delta = base_delta.inverted() @ source_delta
        apply_delta_to_target(target_armature, bone_name, delta, in_place, strengths.get(bone_name, 1.0), target_name=target_name)
    bpy.context.view_layer.update()
    key_target_pose(target_armature, target_frame)


def new_target_action(target_armature, name):
    action = bpy.data.actions.new(f"{ACTION_PREFIX}{name}")
    action.use_fake_user = True
    target_armature.animation_data_create()
    target_armature.animation_data.action = action
    return action


def bake_action(source_armature, target_armature, target_name, source_name, in_place=True, source_step=1):
    source_action = set_source_action(source_armature, source_name)
    target_action = new_target_action(target_armature, target_name)
    source_start = int(math.floor(source_action.frame_range[0]))
    source_end = int(math.ceil(source_action.frame_range[1]))
    bpy.context.scene.frame_set(source_start)
    bpy.context.view_layer.update()
    base_deltas = sample_source_deltas(source_armature)
    strengths = retarget_strengths_for_action(target_name)
    target_frame = 1
    for source_frame in range(source_start, source_end + 1, source_step):
        bake_source_frame(source_armature, target_armature, source_frame, target_frame, in_place, base_deltas, strengths, target_name=target_name)
        target_frame += 1
    target_action["ual_source_actions"] = source_name
    target_action["retarget_notes"] = "Sampled source pose deltas from the UAL1 Rigify/Rigodotify source rig and baked to the Arthur QuadRetro target skeleton."
    if target_name == "Roll":
        target_action["retarget_notes"] += " The sagittal local-X roll component is mirrored so the in-place T66 clip visually tumbles forward while gameplay supplies forward launch motion."
    return target_action


def bake_composite_action(source_armature, target_armature, target_name, segments, in_place=True):
    target_action = new_target_action(target_armature, target_name)
    target_frame = 1
    used_segments = []
    first_source_name, first_start_trim, _first_end_trim = segments[0]
    first_source_action = set_source_action(source_armature, first_source_name)
    first_source_start = int(math.floor(first_source_action.frame_range[0])) + first_start_trim
    bpy.context.scene.frame_set(first_source_start)
    bpy.context.view_layer.update()
    base_deltas = sample_source_deltas(source_armature)
    strengths = retarget_strengths_for_action(target_name)
    for source_name, start_trim, end_trim in segments:
        source_action = set_source_action(source_armature, source_name)
        source_start = int(math.floor(source_action.frame_range[0])) + start_trim
        source_end = int(math.ceil(source_action.frame_range[1])) - end_trim
        for source_frame in range(source_start, source_end + 1):
            bake_source_frame(source_armature, target_armature, source_frame, target_frame, in_place, base_deltas, strengths, target_name=target_name)
            target_frame += 1
        used_segments.append({"action": source_name, "start": source_start, "end": source_end})
    target_action["ual_source_actions"] = json.dumps(used_segments)
    target_action["retarget_notes"] = "Composite UAL jump made from start, airborne, and land source clips."
    return target_action


def bake_ual_actions(source_armature, target_armature):
    actions = [
        bake_action(source_armature, target_armature, "Idle", "Idle_Loop", in_place=True),
        bake_action(source_armature, target_armature, "Walk", "Walk_Formal_Loop", in_place=True),
        bake_composite_action(
            source_armature,
            target_armature,
            "Jump",
            [
                ("Jump_Start", 0, 8),
                ("Jump_Loop", 20, 35),
                ("Jump_Land", 3, 0),
            ],
            in_place=True,
        ),
        bake_action(source_armature, target_armature, "Roll", "Roll_RM", in_place=True),
    ]
    for action in actions:
        for fcurve in iter_action_fcurves(action):
            for keyframe in fcurve.keyframe_points:
                keyframe.interpolation = "BEZIER"
    return actions


def iter_action_fcurves(action):
    yielded = False
    for fcurve in getattr(action, "fcurves", []):
        yielded = True
        yield fcurve
    if not yielded and hasattr(action, "layers"):
        for layer in action.layers:
            for strip in layer.strips:
                for channelbag in strip.channelbags:
                    for fcurve in channelbag.fcurves:
                        yield fcurve


def export_mesh_fbx(armature, mesh, out_path):
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


def export_action_fbx(armature, mesh, action, out_path):
    bpy.ops.object.select_all(action="DESELECT")
    armature.select_set(True)
    mesh.select_set(True)
    bpy.context.view_layer.objects.active = armature
    armature.animation_data_create()
    armature.animation_data.action = action
    if action.slots:
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


def make_manifest(args, out_root, export_root, blend_path, mesh_export, actions, bounds, weight_counts):
    action_exports = []
    for action in actions:
        label = action.name.replace(ACTION_PREFIX, "")
        action_exports.append(
            {
                "label": label,
                "action": action.name,
                "fbx": str(export_root / f"{action.name}.fbx"),
                "frame_start": int(math.floor(action.frame_range[0])),
                "frame_end": int(math.ceil(action.frame_range[1])),
                "ual_source_actions": action.get("ual_source_actions", ""),
            }
        )
    return {
        "visual_id": VISUAL_ID,
        "live_visual_id": LIVE_VISUAL_ID,
        "action_prefix": ACTION_PREFIX,
        "target_armature": TARGET_ARMATURE_NAME,
        "source_armature": SOURCE_ARMATURE_NAME,
        "quadretro_glb": str(Path(args.quadretro_glb).resolve()),
        "pixelated_texture": str(Path(args.pixelated_texture).resolve()),
        "ual1_blend": str(Path(args.ual1_blend).resolve()),
        "bake_live_scale": args.bake_live_scale,
        "blend": str(blend_path),
        "exports": str(export_root),
        "skeletal_mesh_fbx": str(mesh_export),
        "actions": action_exports,
        "mesh_bounds": bounds,
        "vertex_weight_counts": weight_counts,
        "retarget_map": {name: name for name in RETARGET_BONES},
        "notes": [
            "Visible mesh is the live QuadRetro Royal Chad source, not the old SK_Hero_1_Chad pilot mesh.",
            "UAL1 source actions were baked in Blender from the Rigify/Rigodotify-style source rig onto a Rigodotify-named Arthur target skeleton.",
            "Root XY is held in-place because T66 movement and roll direction are actor-driven at runtime.",
            "Roll uses the UAL Roll_RM source with root motion stripped and a local-X sagittal mirror so the in-place clip reads as a forward roll instead of a backflip.",
            "The live row scale is baked into the Blender source/export so skeletal hero runtime scale can remain 1.0.",
        ],
    }


def main(argv):
    parser = argparse.ArgumentParser()
    parser.add_argument("--quadretro-glb", default=DEFAULT_QUADRETRO_GLB)
    parser.add_argument("--pixelated-texture", default=DEFAULT_PIXELATED_TEXTURE)
    parser.add_argument("--ual1-blend", default=DEFAULT_UAL1_BLEND)
    parser.add_argument("--out-root", required=True)
    parser.add_argument("--bake-live-scale", type=float, default=LIVE_ROW_SCALE)
    args = parser.parse_args(argv)

    out_root = Path(args.out_root).resolve()
    export_root = out_root / "Exports"
    export_root.mkdir(parents=True, exist_ok=True)

    reset_scene()
    source_armature = load_ual_source(Path(args.ual1_blend))
    mesh = import_quadretro_mesh(Path(args.quadretro_glb))
    apply_uniform_mesh_scale(mesh, args.bake_live_scale)
    bounds = mesh_bounds(mesh)
    target_armature, bones = create_target_armature(bounds)
    weight_counts = bind_mesh_to_armature(mesh, target_armature, bones, bounds)
    actions = bake_ual_actions(source_armature, target_armature)
    source_armature.hide_viewport = True
    source_armature.hide_render = True

    mesh_export = export_root / "SK_Hero_1_Chad_QuadRetroUALQA.fbx"
    export_mesh_fbx(target_armature, mesh, mesh_export)
    for action in actions:
        export_action_fbx(target_armature, mesh, action, export_root / f"{action.name}.fbx")

    blend_path = out_root / "Arthur_QuadRetro_UAL_Retarget.blend"
    bpy.ops.wm.save_as_mainfile(filepath=str(blend_path))

    manifest = make_manifest(args, out_root, export_root, blend_path, mesh_export, actions, bounds, weight_counts)
    manifest_path = out_root / "arthur_quadretro_ual_retarget_manifest.json"
    manifest_path.write_text(json.dumps(manifest, indent=2), encoding="utf-8")
    print(json.dumps(manifest, indent=2))


if __name__ == "__main__":
    argv = sys.argv
    if "--" in argv:
        argv = argv[argv.index("--") + 1 :]
    else:
        argv = []
    main(argv)
