# Copyright Tribulation 66. All Rights Reserved.
#
# MotionRig Blender pipeline (MOTION_RIG.md section 3).
# Headless: builds a fresh 18-bone physics-first rig on the raw FriendSlop GLB,
# smooth-skins it with deterministic distance-based weights (max 4 influences;
# bone-heat is NOT used — it fails silently on generated multi-shell meshes,
# which is what pushed the old lane into rigid 1-influence skinning), authors
# the six pose-target clips procedurally, exports FBXs, writes QA JSON, and
# renders proof images including a debug skeleton pass.
#
# Run:
#   "C:\Program Files\Blender Foundation\Blender 5.1\blender.exe" --background --factory-startup \
#       --python Scripts/MotionRig/BuildMotionRig.py -- \
#       --glb "Model Generation/Runs/Pixal3D/FriendSlopProbe_Hero1Male_20260604_1415/Outputs/Hero_1_Chad_Male.glb" \
#       --out "Model Generation/Runs/Pixal3D/FriendSlopProbe_Hero1Male_20260604_1415/Blender/MotionRig"
#
# Deterministic: no randomness anywhere.

import argparse
import json
import math
import os
import sys

import bpy
import numpy as np
from mathutils import Vector

FPS = 30
TARGET_HEIGHT_M = 1.80  # normalized character height in meters (Blender units)
MAX_INFLUENCES = 4
FALLOFF_POWER = 3.5     # higher = crisper mid-bone ownership, lower = softer joints
MIN_WEIGHT = 0.04       # influences below this are culled then renormalized

# ---------------------------------------------------------------------------
# helpers
# ---------------------------------------------------------------------------

def parse_args():
    argv = sys.argv
    argv = argv[argv.index("--") + 1:] if "--" in argv else []
    parser = argparse.ArgumentParser()
    parser.add_argument("--glb", required=True)
    parser.add_argument("--out", required=True)
    # Character name token used in every exported asset name:
    # SK_MotionRig_<name>.fbx, AM_MotionRig_<name>_<clip>.fbx,
    # T_MotionRig_<name>_BaseColor.png, MotionRig_<name>.blend.
    parser.add_argument("--name", default="Hero1")
    # Source facing override. "auto" trusts toe-direction detection, which
    # chunky boots can fool (measured: Hero2Chad's boot heels out-protrude
    # the toes and the flip silently skipped). Pixal3D GLBs ship facing +y.
    parser.add_argument("--front", choices=["auto", "+y", "-y"], default="auto")
    # Source rest pose. "tpose" = arms straight out horizontally (the best
    # case for the distance-based skinning: hands/forearms far from torso
    # and thighs). The rig is skinned in T, then the arms are rotated down
    # and applied as the NEW rest pose, so clips/export/UE keep the proven
    # hanging-arm conventions.
    parser.add_argument("--pose", choices=["hanging", "tpose"], default="hanging")
    parser.add_argument("--no-render", action="store_true")
    return parser.parse_args(argv)


def reset_scene():
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.context.scene.render.fps = FPS
    bpy.context.scene.unit_settings.system = "METRIC"
    bpy.context.scene.unit_settings.scale_length = 1.0


def import_glb(path):
    bpy.ops.import_scene.gltf(filepath=path)
    meshes = [o for o in bpy.context.scene.objects if o.type == "MESH"]
    if not meshes:
        raise RuntimeError("GLB contained no mesh objects")
    bpy.ops.object.select_all(action="DESELECT")
    for m in meshes:
        m.select_set(True)
    bpy.context.view_layer.objects.active = meshes[0]
    if len(meshes) > 1:
        bpy.ops.object.join()
    mesh = bpy.context.view_layer.objects.active
    mesh.name = "MotionRigMesh"

    for obj in list(bpy.context.scene.objects):
        if obj is not mesh:
            bpy.data.objects.remove(obj, do_unlink=True)
    mesh.parent = None
    mesh.modifiers.clear()
    mesh.vertex_groups.clear()

    # Normal cleanup (the FallGuys look-dev recipe): Pixal3D decimation
    # leaves normal noise that makes glossy sheen band/stripe along the
    # triangulation. Shade-smooth everything + weighted normals (FACE_AREA)
    # — shading-only, silhouette and UVs unchanged. The smoothed normals are
    # exported and UE must IMPORT them (ImportMotionRig.py), not recompute.
    for polygon in mesh.data.polygons:
        polygon.use_smooth = True
    bpy.context.view_layer.objects.active = mesh
    wn = mesh.modifiers.new(name="WeightedNormal", type="WEIGHTED_NORMAL")
    wn.mode = "FACE_AREA"
    wn.keep_sharp = False
    wn.weight = 50
    bpy.ops.object.modifier_apply(modifier=wn.name)
    return mesh


def vertex_array(mesh):
    count = len(mesh.data.vertices)
    coords = np.empty(count * 3, dtype=np.float64)
    mesh.data.vertices.foreach_get("co", coords)
    return coords.reshape(count, 3)


def normalize_mesh(mesh, front="auto"):
    """Scale to TARGET_HEIGHT_M, feet at z=0, centered XY, FRONT = -Y.

    Facing is auto-detected from toe direction (feet protrude toward the
    front) unless `front` overrides it ("+y"/"-y" = the direction the SOURCE
    faces). The raw Pixal3D GLBs have come in facing +Y, so trusting a fixed
    convention is not safe — but chunky boots can fool the toe heuristic
    too, so explicit beats implicit when the source is known."""
    bpy.ops.object.select_all(action="DESELECT")
    mesh.select_set(True)
    bpy.context.view_layer.objects.active = mesh
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)

    v = vertex_array(mesh)
    height = v[:, 2].max() - v[:, 2].min()
    if height <= 0.0:
        raise RuntimeError("Mesh has zero height")

    scale = TARGET_HEIGHT_M / height
    mesh.scale = (scale, scale, scale)
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)

    v = vertex_array(mesh)
    center_x = (v[:, 0].max() + v[:, 0].min()) * 0.5
    center_y = (v[:, 1].max() + v[:, 1].min()) * 0.5
    mesh.location = (-center_x, -center_y, -v[:, 2].min())
    bpy.ops.object.transform_apply(location=True, rotation=False, scale=False)

    # Toe-direction facing check on the foot band (unless overridden).
    v = vertex_array(mesh)
    foot = v[v[:, 2] < 0.07 * TARGET_HEIGHT_M]
    if front == "auto":
        facing_plus_y = abs(foot[:, 1].max()) > abs(foot[:, 1].min())
    else:
        facing_plus_y = front == "+y"

    # Posture straightening: Pixal3D guesses depth from a front-view-only
    # reference and can bake a forward lean (measured 14cm head-vs-ankle
    # drift on StacyTPoseVar1). SHEAR the mesh vertical (y -= k*z) instead
    # of rotating — horizontal slices stay horizontal, so boot soles stay
    # flat on the floor.
    def straighten():
        verts = vertex_array(mesh)
        h_now = verts[:, 2].max() - verts[:, 2].min()
        low = verts[(verts[:, 2] > 0.02 * h_now) & (verts[:, 2] < 0.12 * h_now)]
        high = verts[verts[:, 2] > 0.80 * h_now]
        if not low.size or not high.size:
            return 0.0
        slope = (float(np.median(high[:, 1])) - float(np.median(low[:, 1]))) / \
                (float(np.median(high[:, 2])) - float(np.median(low[:, 2])))
        if abs(slope) < 0.01:
            return 0.0
        from mathutils import Matrix
        shear = Matrix.Identity(4)
        shear[1][2] = -slope  # y -= slope * z
        mesh.data.transform(shear)
        mesh.data.update()
        return slope

    lean_slope = straighten()
    if facing_plus_y:
        # glTF imports arrive in QUATERNION rotation mode — switching the mode
        # first is load-bearing; assigning rotation_euler on a quaternion-mode
        # object is silently ignored and transform_apply applies identity.
        mesh.rotation_mode = "XYZ"
        mesh.rotation_euler = (0.0, 0.0, math.pi)
        bpy.ops.object.transform_apply(location=False, rotation=True, scale=False)
        # Re-center XY after the flip.
        v = vertex_array(mesh)
        center_x = (v[:, 0].max() + v[:, 0].min()) * 0.5
        center_y = (v[:, 1].max() + v[:, 1].min()) * 0.5
        mesh.location = (-center_x, -center_y, 0.0)
        bpy.ops.object.transform_apply(location=True, rotation=False, scale=False)

    return TARGET_HEIGHT_M, bool(facing_plus_y), float(lean_slope)


def measure_landmarks(mesh, height, pose="hanging"):
    """Landmarks from vertex statistics. Front = -Y, left = +X (mirror naming
    follows the character's left, which is +X when it faces -Y)."""
    h = height
    v = vertex_array(mesh)

    def band(z_lo, z_hi):
        return v[(v[:, 2] >= z_lo) & (v[:, 2] <= z_hi)]

    shoulders = band(0.76 * h, 0.84 * h)
    shoulder_half_w = float(np.abs(shoulders[:, 0]).max()) if shoulders.size else 0.22 * h

    # Legs: measure at the KNEE band — below any coat/skirt flare that
    # contaminates a hip-height width measurement. Median |x| per side gives
    # the leg column centers; median y centers the bones inside the leg.
    knees = band(0.24 * h, 0.32 * h)
    left_leg = knees[knees[:, 0] > 0.0] if knees.size else knees
    right_leg = knees[knees[:, 0] < 0.0] if knees.size else knees
    if left_leg.size and right_leg.size:
        leg_half_x = float((np.median(left_leg[:, 0]) + np.median(-right_leg[:, 0])) * 0.5)
        leg_center_y = float(np.median(knees[:, 1]))
    else:
        leg_half_x = 0.10 * h
        leg_center_y = 0.0

    def hand_tip(sign):
        if pose == "tpose":
            # Arms straight out horizontally: the hand/mitt tip is simply the
            # x-extreme in the shoulder-height band.
            band = v[(v[:, 2] > 0.62 * h) & (v[:, 2] < 0.95 * h)]
            side = band[np.sign(band[:, 0]) == sign] if band.size else band
            if side.size:
                tip = side[np.abs(side[:, 0]).argmax()]
                return Vector((float(tip[0]) * 0.97, float(tip[1]), float(tip[2])))
            # fall through to the hanging heuristic if the band is empty

        # Arms hang at the sides: find the x-extreme in the cuff/forearm band,
        # then walk DOWN that vertical column to the fist bottom so flared
        # cuffs cannot fake the hand position.
        low = v[(v[:, 2] > 0.30 * h) & (v[:, 2] < 0.62 * h)]
        side_low = low[np.sign(low[:, 0]) == sign] if low.size else low
        if not side_low.size:
            side_any = v[np.sign(v[:, 0]) == sign]
            tip = side_any[np.abs(side_any[:, 0]).argmax()]
            return Vector((float(tip[0]), float(tip[1]), float(tip[2])))
        tip = side_low[np.abs(side_low[:, 0]).argmax()]
        column = v[(np.abs(v[:, 0] - tip[0]) < 0.08 * h)
                   & (np.abs(v[:, 1] - tip[1]) < 0.08 * h)
                   & (v[:, 2] > 0.25 * h)]
        bottom = column[column[:, 2].argmin()]
        return Vector((float(bottom[0]) * 0.96, float(bottom[1]), float(bottom[2]) + 0.02 * h))

    return {
        "height": h,
        "shoulder_half_w": shoulder_half_w,
        "leg_half_x": leg_half_x,
        "leg_center_y": leg_center_y,
        "left_hand_tip": hand_tip(+1),
        "right_hand_tip": hand_tip(-1),
    }


# ---------------------------------------------------------------------------
# armature
# ---------------------------------------------------------------------------

def build_armature(landmarks):
    h = landmarks["height"]
    hip_x = landmarks["leg_half_x"]
    leg_y = landmarks["leg_center_y"]
    shoulder_x = landmarks["shoulder_half_w"] * 0.62

    z_pelvis = 0.545 * h
    z_spine1 = 0.62 * h
    z_spine2 = 0.72 * h
    z_neck = 0.82 * h
    z_top = 0.985 * h
    z_knee = 0.29 * h
    z_ankle = 0.055 * h
    z_shoulder = 0.795 * h

    def arm_chain(tip, sign):
        shoulder = Vector((sign * shoulder_x, 0.0, z_shoulder))
        wrist = shoulder + (tip - shoulder) * 0.82
        elbow = shoulder + (tip - shoulder) * 0.45
        return shoulder, elbow, wrist, tip

    l_sh, l_el, l_wr, l_tip = arm_chain(landmarks["left_hand_tip"], +1)
    r_sh, r_el, r_wr, r_tip = arm_chain(landmarks["right_hand_tip"], -1)

    bone_defs = {
        "pelvis":     {"head": (0, 0, z_pelvis), "tail": (0, 0, z_spine1), "parent": None, "connect": False},
        "spine_01":   {"head": (0, 0, z_spine1), "tail": (0, 0, z_spine2), "parent": "pelvis", "connect": True},
        "spine_02":   {"head": (0, 0, z_spine2), "tail": (0, 0, z_neck), "parent": "spine_01", "connect": True},
        "head":       {"head": (0, 0, z_neck), "tail": (0, 0, z_top), "parent": "spine_02", "connect": True},
        "clavicle_l": {"head": (0.02 * h, 0, z_shoulder), "tail": tuple(l_sh), "parent": "spine_02", "connect": False},
        "upperarm_l": {"head": tuple(l_sh), "tail": tuple(l_el), "parent": "clavicle_l", "connect": True},
        "lowerarm_l": {"head": tuple(l_el), "tail": tuple(l_wr), "parent": "upperarm_l", "connect": True},
        "hand_l":     {"head": tuple(l_wr), "tail": tuple(l_tip), "parent": "lowerarm_l", "connect": True},
        "clavicle_r": {"head": (-0.02 * h, 0, z_shoulder), "tail": tuple(r_sh), "parent": "spine_02", "connect": False},
        "upperarm_r": {"head": tuple(r_sh), "tail": tuple(r_el), "parent": "clavicle_r", "connect": True},
        "lowerarm_r": {"head": tuple(r_el), "tail": tuple(r_wr), "parent": "upperarm_r", "connect": True},
        "hand_r":     {"head": tuple(r_wr), "tail": tuple(r_tip), "parent": "lowerarm_r", "connect": True},
        "thigh_l":    {"head": (hip_x, leg_y, z_pelvis), "tail": (hip_x, leg_y, z_knee), "parent": "pelvis", "connect": False},
        "calf_l":     {"head": (hip_x, leg_y, z_knee), "tail": (hip_x, leg_y, z_ankle), "parent": "thigh_l", "connect": True},
        "foot_l":     {"head": (hip_x, leg_y, z_ankle), "tail": (hip_x, leg_y - 0.10 * h, 0.012 * h), "parent": "calf_l", "connect": True},
        "thigh_r":    {"head": (-hip_x, leg_y, z_pelvis), "tail": (-hip_x, leg_y, z_knee), "parent": "pelvis", "connect": False},
        "calf_r":     {"head": (-hip_x, leg_y, z_knee), "tail": (-hip_x, leg_y, z_ankle), "parent": "thigh_r", "connect": True},
        "foot_r":     {"head": (-hip_x, leg_y, z_ankle), "tail": (-hip_x, leg_y - 0.10 * h, 0.012 * h), "parent": "calf_r", "connect": True},
    }

    arm_data = bpy.data.armatures.new("MotionRig")
    arm_obj = bpy.data.objects.new("MotionRigArmature", arm_data)
    bpy.context.scene.collection.objects.link(arm_obj)
    bpy.context.view_layer.objects.active = arm_obj
    bpy.ops.object.mode_set(mode="EDIT")

    created = {}
    for name, d in bone_defs.items():
        b = arm_data.edit_bones.new(name)
        b.head = Vector(d["head"])
        b.tail = Vector(d["tail"])
        created[name] = b
    for name, d in bone_defs.items():
        if d["parent"]:
            created[name].parent = created[d["parent"]]
            created[name].use_connect = d["connect"]

    bpy.ops.object.mode_set(mode="OBJECT")
    return arm_obj, {k: {"head": list(v["head"]), "tail": list(v["tail"])} for k, v in bone_defs.items()}


# ---------------------------------------------------------------------------
# skinning — deterministic distance-based smooth weights
# ---------------------------------------------------------------------------

def skin_mesh(mesh, arm_obj, bone_layout):
    """Nearest-segment falloff weighting. Robust on multi-shell generated
    meshes where bone heat fails. Top MAX_INFLUENCES bones per vertex with
    1/d^FALLOFF_POWER falloff, small weights culled, normalized."""
    bone_names = list(bone_layout.keys())
    heads = np.array([bone_layout[n]["head"] for n in bone_names], dtype=np.float64)
    tails = np.array([bone_layout[n]["tail"] for n in bone_names], dtype=np.float64)

    v = vertex_array(mesh)  # (N,3)
    n_verts = v.shape[0]
    n_bones = len(bone_names)

    dists = np.empty((n_verts, n_bones), dtype=np.float64)
    for i in range(n_bones):
        a = heads[i]
        ab = tails[i] - a
        ab2 = max(float(ab.dot(ab)), 1e-12)
        t = np.clip(((v - a) @ ab) / ab2, 0.0, 1.0)
        proj = a + t[:, None] * ab
        dists[:, i] = np.linalg.norm(v - proj, axis=1)

    np.maximum(dists, 1e-5, out=dists)
    k = min(MAX_INFLUENCES, n_bones)
    nearest = np.argpartition(dists, k - 1, axis=1)[:, :k]            # (N,k) bone ids
    near_d = np.take_along_axis(dists, nearest, axis=1)               # (N,k)
    w = 1.0 / np.power(near_d, FALLOFF_POWER)
    w /= w.sum(axis=1, keepdims=True)
    w[w < MIN_WEIGHT] = 0.0
    row_sum = w.sum(axis=1, keepdims=True)
    # Guarantee at least the nearest bone survives.
    dead = (row_sum[:, 0] <= 0.0)
    if dead.any():
        w[dead, 0] = 1.0
        row_sum = w.sum(axis=1, keepdims=True)
    w /= row_sum

    # Write groups with quantized buckets so vg.add() calls stay bounded.
    groups = {name: mesh.vertex_groups.new(name=name) for name in bone_names}
    quant = np.round(w * 50.0).astype(np.int32)  # 0..50 buckets of 0.02
    for col in range(k):
        bone_ids = nearest[:, col]
        buckets = quant[:, col]
        valid = buckets > 0
        ids_valid = bone_ids[valid]
        buckets_valid = buckets[valid]
        verts_valid = np.nonzero(valid)[0]
        for bone_i in np.unique(ids_valid):
            mask_bone = ids_valid == bone_i
            verts_bone = verts_valid[mask_bone]
            buckets_bone = buckets_valid[mask_bone]
            vg = groups[bone_names[bone_i]]
            for level in np.unique(buckets_bone):
                idx = verts_bone[buckets_bone == level]
                vg.add([int(x) for x in idx], float(level) / 50.0, "ADD")

    # Parent with armature modifier (no auto weights).
    mesh.parent = arm_obj
    mod = mesh.modifiers.new(name="Armature", type="ARMATURE")
    mod.object = arm_obj
    mod.use_vertex_groups = True


def rotate_pose_bone_world(pose_bone, axis, angle_deg):
    """Set a pose-bone rotation specified about a WORLD axis (converted into
    the bone's local frame — bone rolls make hand-authored local eulers
    unreliable for this)."""
    from mathutils import Matrix
    world_rotation = Matrix.Rotation(math.radians(angle_deg), 3, axis)
    bone_frame = pose_bone.bone.matrix_local.to_3x3()
    local_rotation = bone_frame.inverted() @ world_rotation @ bone_frame
    pose_bone.rotation_mode = "XYZ"
    pose_bone.rotation_euler = local_rotation.to_euler("XYZ")


def apply_tpose_relaxation(mesh, arm_obj, drop_deg=50.0):
    """T-pose sources only: the rig was SKINNED in T-pose (clean distance
    weights — hands far from thighs, arms far from torso). Now rotate the
    arms down by drop_deg and make that the NEW rest pose for both armature
    and mesh. Weights are untouched.

    drop_deg=50 leaves a Fall-Guys-style A-pose rest (arms ~40 deg out):
    less linear-blend stress on the shoulders AND on-brand. The bake uses
    PRESERVE VOLUME (dual-quaternion) skinning — a plain LBS bake at 75 deg
    crushed the shoulders into permanent candy-wrapper lumps (measured on
    StacyTPoseVar1; the raw Pixal3D mesh was clean)."""
    bpy.ops.object.select_all(action="DESELECT")
    arm_obj.select_set(True)
    bpy.context.view_layer.objects.active = arm_obj
    bpy.ops.object.mode_set(mode="POSE")
    clear_pose(arm_obj)
    # Arm along +X (left): dropping toward -Z is +rotation about world Y;
    # mirrored for the right arm.
    rotate_pose_bone_world(arm_obj.pose.bones["upperarm_l"], "Y", drop_deg)
    rotate_pose_bone_world(arm_obj.pose.bones["upperarm_r"], "Y", -drop_deg)
    bpy.context.view_layer.update()
    bpy.ops.object.mode_set(mode="OBJECT")

    # Bake the posed shape into the mesh data (vertex groups survive) using
    # dual-quaternion deformation, then re-add the armature modifier (plain
    # LBS, matching what UE runs at runtime) and make the pose the rest pose.
    bpy.context.view_layer.objects.active = mesh
    mesh.modifiers["Armature"].use_deform_preserve_volume = True
    bpy.ops.object.modifier_apply(modifier="Armature")
    modifier = mesh.modifiers.new(name="Armature", type="ARMATURE")
    modifier.object = arm_obj
    modifier.use_vertex_groups = True

    bpy.context.view_layer.objects.active = arm_obj
    bpy.ops.object.mode_set(mode="POSE")
    bpy.ops.pose.armature_apply(selected=False)
    bpy.ops.object.mode_set(mode="OBJECT")


def weights_qa(mesh):
    unweighted = 0
    max_influences = 0
    for vert in mesh.data.vertices:
        influences = [g for g in vert.groups if g.weight > 0.001]
        if not influences:
            unweighted += 1
        max_influences = max(max_influences, len(influences))
    return {
        "vertex_count": len(mesh.data.vertices),
        "unweighted": unweighted,
        "max_influences": max_influences,
    }


# ---------------------------------------------------------------------------
# clips — readable pose dictionaries. Degrees, bone-local euler XYZ.
# Physics softens everything downstream; deliberately simple big poses.
# ---------------------------------------------------------------------------

def set_pose(arm_obj, frame, pose):
    bpy.context.scene.frame_set(frame)
    for pb in arm_obj.pose.bones:
        if pb.name not in pose:
            continue
    for bone_name, euler_deg in pose.items():
        pb = arm_obj.pose.bones.get(bone_name)
        if not pb:
            continue
        pb.rotation_mode = "XYZ"
        pb.rotation_euler = tuple(math.radians(a) for a in euler_deg)
        pb.keyframe_insert(data_path="rotation_euler", frame=frame)


def key_all_neutral(arm_obj, frame):
    for pb in arm_obj.pose.bones:
        pb.rotation_mode = "XYZ"
        pb.rotation_euler = (0.0, 0.0, 0.0)
        pb.keyframe_insert(data_path="rotation_euler", frame=frame)


def clear_pose(arm_obj):
    for pb in arm_obj.pose.bones:
        pb.rotation_mode = "XYZ"
        pb.rotation_euler = (0.0, 0.0, 0.0)
        pb.location = (0.0, 0.0, 0.0)


def make_action(arm_obj, name, length_frames, keyer):
    clear_pose(arm_obj)
    action = bpy.data.actions.new(name)
    arm_obj.animation_data_create()
    arm_obj.animation_data.action = action
    keyer(arm_obj)
    # Every bone gets a LOCATION key: keyed channels are unit-converted to cm
    # by the exporter (proven by the pelvis), un-keyed bones fall back to the
    # meter-scale rest pose in UE and collapse onto their parents. One key at
    # frame 1 makes the baked export carry full cm translations per bone.
    for pose_bone in arm_obj.pose.bones:
        pose_bone.keyframe_insert("location", frame=1)
    action.use_fake_user = True
    bpy.context.scene.frame_start = 1
    bpy.context.scene.frame_end = length_frames
    return action


def key_idle(arm):
    for f, lean in ((1, -2.5), (16, 0.0), (31, 2.5), (46, 0.0), (60, -2.5)):
        set_pose(arm, f, {
            "pelvis": (0, lean, 0),
            "spine_01": (1.5, -lean * 0.6, 0),
            "spine_02": (1.0, -lean * 0.4, lean * 0.5),
            "head": (-1.0, lean * 0.3, 0),
            "upperarm_l": (4, 0, 6 + lean), "lowerarm_l": (6, 0, 0),
            "upperarm_r": (4, 0, -6 + lean), "lowerarm_r": (6, 0, 0),
            "thigh_l": (-1, 0, 0), "thigh_r": (-1, 0, 0),
        })


def key_walk(arm):
    # 32f loop = 2 steps. Contact L(1) pass(9) contact R(17) pass(25) loop(32).
    def step(f, swing, counter):
        set_pose(arm, f, {
            "pelvis": (2.0, 0, counter * 4.0),
            "spine_01": (2.0, 0, -counter * 3.0),
            "spine_02": (1.0, 0, -counter * 2.0),
            "head": (-2.0, 0, counter * 1.5),
            "thigh_l": (swing * 38.0, 0, 0),
            "calf_l": (min(0.0, -swing * 18.0) - 6.0, 0, 0),
            "foot_l": (swing * 10.0 + 4.0, 0, 0),
            "thigh_r": (-swing * 38.0, 0, 0),
            "calf_r": (min(0.0, swing * 18.0) - 6.0, 0, 0),
            "foot_r": (-swing * 10.0 + 4.0, 0, 0),
            "upperarm_l": (-swing * 42.0, 0, 8),
            "lowerarm_l": (max(0.0, -swing * 20.0) + 8.0, 0, 0),
            "upperarm_r": (swing * 42.0, 0, -8),
            "lowerarm_r": (max(0.0, swing * 20.0) + 8.0, 0, 0),
        })
    step(1, 1.0, 1.0)
    step(9, 0.0, 0.0)
    step(17, -1.0, -1.0)
    step(25, 0.0, 0.0)
    step(32, 1.0, 1.0)


def key_jump(arm):
    key_all_neutral(arm, 1)
    set_pose(arm, 6, {
        "pelvis": (8, 0, 0), "spine_01": (12, 0, 0), "spine_02": (8, 0, 0), "head": (-8, 0, 0),
        "thigh_l": (-42, 0, 0), "calf_l": (-58, 0, 0), "foot_l": (18, 0, 0),
        "thigh_r": (-42, 0, 0), "calf_r": (-58, 0, 0), "foot_r": (18, 0, 0),
        "upperarm_l": (-30, 0, 10), "upperarm_r": (-30, 0, -10),
    })
    set_pose(arm, 12, {
        "pelvis": (-4, 0, 0), "spine_01": (-6, 0, 0), "spine_02": (-4, 0, 0), "head": (4, 0, 0),
        "thigh_l": (6, 0, 0), "calf_l": (-2, 0, 0), "foot_l": (-22, 0, 0),
        "thigh_r": (6, 0, 0), "calf_r": (-2, 0, 0), "foot_r": (-22, 0, 0),
        "upperarm_l": (140, 0, 12), "lowerarm_l": (10, 0, 0),
        "upperarm_r": (140, 0, -12), "lowerarm_r": (10, 0, 0),
    })
    set_pose(arm, 20, {
        "pelvis": (4, 0, 0), "spine_01": (6, 0, 0), "spine_02": (4, 0, 0), "head": (-4, 0, 0),
        "thigh_l": (-34, 0, 0), "calf_l": (-40, 0, 0),
        "thigh_r": (-34, 0, 0), "calf_r": (-40, 0, 0),
        "upperarm_l": (60, 0, 18), "lowerarm_l": (24, 0, 0),
        "upperarm_r": (60, 0, -18), "lowerarm_r": (24, 0, 0),
    })
    set_pose(arm, 28, {
        "pelvis": (10, 0, 0), "spine_01": (10, 0, 0), "spine_02": (6, 0, 0), "head": (-8, 0, 0),
        "thigh_l": (-38, 0, 0), "calf_l": (-50, 0, 0), "foot_l": (14, 0, 0),
        "thigh_r": (-38, 0, 0), "calf_r": (-50, 0, 0), "foot_r": (14, 0, 0),
        "upperarm_l": (-20, 0, 14), "upperarm_r": (-20, 0, -14),
    })
    key_all_neutral(arm, 34)


def key_dive(arm):
    key_all_neutral(arm, 1)
    set_pose(arm, 5, {
        "pelvis": (6, 0, 0), "spine_01": (8, 0, 0),
        "thigh_l": (-24, 0, 0), "calf_l": (-30, 0, 0),
        "thigh_r": (-24, 0, 0), "calf_r": (-30, 0, 0),
        "upperarm_l": (-50, 0, 10), "upperarm_r": (-50, 0, -10),
    })
    superman = {
        "pelvis": (-6, 0, 0), "spine_01": (-8, 0, 0), "spine_02": (-6, 0, 0), "head": (10, 0, 0),
        "thigh_l": (4, 0, 2), "calf_l": (-4, 0, 0), "foot_l": (-18, 0, 0),
        "thigh_r": (4, 0, -2), "calf_r": (-4, 0, 0), "foot_r": (-18, 0, 0),
        "upperarm_l": (165, 0, 8), "lowerarm_l": (4, 0, 0), "hand_l": (0, 0, 0),
        "upperarm_r": (165, 0, -8), "lowerarm_r": (4, 0, 0), "hand_r": (0, 0, 0),
    }
    set_pose(arm, 10, superman)
    set_pose(arm, 42, superman)


def key_getup_front(arm):
    prone_push = {
        "pelvis": (10, 0, 0), "spine_01": (-18, 0, 0), "spine_02": (-14, 0, 0), "head": (16, 0, 0),
        "upperarm_l": (110, 0, 30), "lowerarm_l": (70, 0, 0),
        "upperarm_r": (110, 0, -30), "lowerarm_r": (70, 0, 0),
        "thigh_l": (-10, 0, 0), "thigh_r": (-10, 0, 0),
    }
    kneel = {
        "pelvis": (14, 0, 0), "spine_01": (8, 0, 0), "spine_02": (6, 0, 0), "head": (-6, 0, 0),
        "thigh_l": (-70, 0, 6), "calf_l": (-80, 0, 0), "foot_l": (30, 0, 0),
        "thigh_r": (-30, 0, -4), "calf_r": (-50, 0, 0), "foot_r": (10, 0, 0),
        "upperarm_l": (30, 0, 14), "upperarm_r": (30, 0, -14),
    }
    rise = {
        "pelvis": (6, 0, 0), "spine_01": (4, 0, 0),
        "thigh_l": (-20, 0, 0), "calf_l": (-24, 0, 0),
        "thigh_r": (-20, 0, 0), "calf_r": (-24, 0, 0),
        "upperarm_l": (10, 0, 8), "upperarm_r": (10, 0, -8),
    }
    set_pose(arm, 1, prone_push)
    set_pose(arm, 16, kneel)
    set_pose(arm, 34, rise)
    key_all_neutral(arm, 50)


def key_getup_back(arm):
    situp = {
        "pelvis": (-6, 0, 0), "spine_01": (26, 0, 0), "spine_02": (18, 0, 0), "head": (-12, 0, 0),
        "thigh_l": (-30, 0, 0), "calf_l": (-20, 0, 0),
        "thigh_r": (-30, 0, 0), "calf_r": (-20, 0, 0),
        "upperarm_l": (40, 0, 16), "upperarm_r": (40, 0, -16),
    }
    kneel = {
        "pelvis": (10, 0, 12), "spine_01": (10, 0, -8), "spine_02": (8, 0, -6),
        "thigh_l": (-66, 0, 8), "calf_l": (-78, 0, 0), "foot_l": (26, 0, 0),
        "thigh_r": (-26, 0, -6), "calf_r": (-44, 0, 0),
        "upperarm_l": (24, 0, 12), "upperarm_r": (36, 0, -18),
    }
    set_pose(arm, 1, situp)
    set_pose(arm, 18, kneel)
    set_pose(arm, 36, {"pelvis": (4, 0, 0), "thigh_l": (-16, 0, 0), "calf_l": (-18, 0, 0), "thigh_r": (-16, 0, 0), "calf_r": (-18, 0, 0)})
    key_all_neutral(arm, 50)


CLIPS = [
    ("Idle", 60, key_idle),
    ("Walk", 32, key_walk),
    ("Jump", 34, key_jump),
    ("Dive", 42, key_dive),
    ("GetUp_Front", 50, key_getup_front),
    ("GetUp_Back", 50, key_getup_back),
]


# ---------------------------------------------------------------------------
# export
# ---------------------------------------------------------------------------

FBX_COMMON = dict(
    use_selection=True,
    # Exporter unit semantics (measured, 3 permutations): the m->cm x100 is
    # ALWAYS applied (apply_unit_scale only adds scene scale_length on top)
    # and with FBX_SCALE_NONE the total factor lands as scale on the TOP
    # OBJECT NODES only — geometry, bones and curves pass through raw. A
    # cm scale_length scene instead x100s GEOMETRY ONLY (kaiju bug). So:
    # data is pre-baked to real cm (convert_scene_to_centimeters) and
    # global_scale=0.01 cancels the invariant x100 -> total factor 1.0,
    # node scales 1, every number in the file is centimeters.
    global_scale=0.01,
    apply_unit_scale=True,
    apply_scale_options="FBX_SCALE_NONE",
    object_types={"ARMATURE", "MESH"},
    use_mesh_modifiers=True,
    mesh_smooth_type="FACE",
    add_leaf_bones=False,
    bake_anim=False,
    path_mode="COPY",
    embed_textures=True,
)


# NOTE on units (raw binary FBX probe, 2026-06-10 — do not "simplify"):
# - The exporter performs NO m->cm value conversion. A meter-scene export
#   writes meter numbers for EVERYTHING (rest bones, anim curves, verts) and
#   compensates with scale=100 on the armature/mesh OBJECT nodes
#   (UnitScaleFactor stays 1.0 = cm). UE turns that armature node into a
#   scale-100 root bone: component-space looks right, but the physics-asset
#   generator and world-space bone writes use unscaled bone locals -> bodies
#   collapse to a point, zero-length anchors, centimeter-sized render.
# - FBX_SCALE_ALL did not fix the bone transforms either (measured: UE ref
#   pose still pelvisZ=0.98 post-reimport).
# - A x100 data bake exported from a METER-declared scene double-converts
#   into an 18,000-unit kaiju (walkcircle_v4).
# The deterministic fix: convert_scene_to_centimeters() right before export —
# scale mesh+armature objects x100 (applied), scale action LOCATION curves
# x100 (rotations are scale-free), set scene units to cm. The exporter's
# unit factor is then exactly 1.0: real cm numbers, scale 1 everywhere.


def convert_scene_to_centimeters(mesh, arm_obj):
    # DATA-level x100 (Mesh.transform / Armature.transform): object scales
    # stay 1 and the parent/child matrix_parent_inverse plumbing is never
    # involved. Object-level scale+apply on the parented mesh+armature pair
    # double-scales the child mesh data (measured: 18,000-unit verts).
    # Scene units stay METRIC 1.0; FBX_COMMON's global_scale=0.01 cancels
    # the exporter's invariant m->cm x100, so the file carries these cm
    # numbers raw and UE reads them as cm (bConvertSceneUnit=false).
    # The bake also rotates the rig +90 deg about Z: authoring faces -Y
    # (Blender front), but the legacy UE importer's axis conversion maps
    # Blender -Y to UE +Y (measured in-game: character faced 90 deg right
    # of travel), and the game wants forward = UE +X. The effective
    # Blender->UE mapping is Y-negation, so facing Blender +X lands facing
    # UE +X. Clips are bone-LOCAL rotations/locations — a global rig
    # rotation leaves them valid.
    from mathutils import Matrix
    bpy.context.view_layer.objects.active = arm_obj
    if arm_obj.mode != "OBJECT":
        bpy.ops.object.mode_set(mode="OBJECT")
    bake_matrix = Matrix.Rotation(math.pi / 2.0, 4, "Z") @ Matrix.Scale(100.0, 4)
    mesh.data.transform(bake_matrix)
    mesh.data.update()
    arm_obj.data.transform(bake_matrix)

    # Pose-bone location keys are armature-space numbers; they do not scale
    # with transform_apply and must be multiplied explicitly.
    def iter_action_fcurves(action):
        if hasattr(action, "fcurves"):  # pre-5.x flat API
            yield from action.fcurves
            return
        for layer in action.layers:  # Blender 5.x slotted actions
            for strip in layer.strips:
                for channelbag in strip.channelbags:
                    yield from channelbag.fcurves

    for action in bpy.data.actions:
        for fcurve in iter_action_fcurves(action):
            if fcurve.data_path.endswith("location"):
                for key in fcurve.keyframe_points:
                    key.co.y *= 100.0
                    key.handle_left.y *= 100.0
                    key.handle_right.y *= 100.0
                fcurve.update()


def make_bind_pose_action(arm_obj):
    """Constant rest-pose action baked INTO the skeletal FBX.

    The exporter converts KEYED channels m->cm but NOT armature rest bones,
    so a plain skeletal export gives UE a meter-scale reference skeleton.
    Importing with use_t0_as_ref_pose=True rebuilds the reference pose AND
    the render bind from this baked cm animation instead — the only path
    that fixes skinning: post-import ref surgery cannot reach the
    bind-dependent LOD render caches built at import time.

    Keys at frames 0 AND 1 with identical values so the T0 sample is exact
    regardless of how the exporter maps frame numbers to FBX time."""
    clear_pose(arm_obj)
    action = bpy.data.actions.new("BindPose")
    arm_obj.animation_data_create()
    arm_obj.animation_data.action = action
    for frame in (0, 1):
        key_all_neutral(arm_obj, frame)
        for pose_bone in arm_obj.pose.bones:
            pose_bone.keyframe_insert("location", frame=frame)
    action.use_fake_user = True
    return action


def export_base_color_png(mesh, out_path):
    """Save the GLB material's base-color image as a REAL png. GLB textures
    can be WebP bytes in .png clothing (UE rejects those); Image.save with
    file_format=PNG decodes to pixels and re-encodes properly."""
    for mat in mesh.data.materials:
        if not mat or not mat.use_nodes:
            continue
        for node in mat.node_tree.nodes:
            if node.type == "TEX_IMAGE" and node.image:
                image = node.image
                image.file_format = "PNG"
                image.save(filepath=out_path)
                return True
    return False


def export_skeletal_fbx(mesh, arm_obj, bind_action, out_path):
    clear_pose(arm_obj)
    arm_obj.animation_data_create()
    arm_obj.animation_data.action = bind_action
    bpy.context.scene.frame_start = 0
    bpy.context.scene.frame_end = 1
    bpy.ops.object.select_all(action="DESELECT")
    mesh.select_set(True)
    arm_obj.select_set(True)
    bpy.context.view_layer.objects.active = arm_obj
    opts = dict(FBX_COMMON)
    opts.update(dict(
        bake_anim=True,
        bake_anim_use_all_bones=True,
        bake_anim_use_nla_strips=False,
        bake_anim_use_all_actions=False,
        bake_anim_force_startend_keying=True,
        bake_anim_step=1.0,
        bake_anim_simplify_factor=0.0,
    ))
    bpy.ops.export_scene.fbx(filepath=out_path, **opts)


def export_clip_fbx(arm_obj, action, length_frames, out_path):
    arm_obj.animation_data_create()
    arm_obj.animation_data.action = action
    bpy.context.scene.frame_start = 1
    bpy.context.scene.frame_end = length_frames
    bpy.ops.object.select_all(action="DESELECT")
    arm_obj.select_set(True)
    bpy.context.view_layer.objects.active = arm_obj
    opts = dict(FBX_COMMON)
    opts.update(dict(
        object_types={"ARMATURE"},
        bake_anim=True,
        bake_anim_use_all_bones=True,
        bake_anim_use_nla_strips=False,
        bake_anim_use_all_actions=False,
        bake_anim_force_startend_keying=True,
        bake_anim_step=1.0,
        bake_anim_simplify_factor=0.0,
    ))
    bpy.ops.export_scene.fbx(filepath=out_path, **opts)


# ---------------------------------------------------------------------------
# proof renders
# ---------------------------------------------------------------------------

def add_bone_proxies(arm_obj, bone_layout):
    """Visible skeleton: a thin emissive cylinder per bone so renders show
    exactly where the rig landed inside (or outside!) the body. Reads the
    LIVE armature data (the T-pose relaxation re-rests the arm bones after
    bone_layout was captured)."""
    proxies = []
    mat = bpy.data.materials.new("BoneProxyMat")
    mat.use_nodes = False
    mat.diffuse_color = (1.0, 0.1, 0.05, 1.0)
    for name in bone_layout.keys():
        bone = arm_obj.data.bones.get(name)
        if not bone:
            continue
        head = Vector(bone.head_local)
        tail = Vector(bone.tail_local)
        mid = (head + tail) * 0.5
        length = max((tail - head).length, 1e-4)
        bpy.ops.mesh.primitive_cylinder_add(radius=0.012, depth=length, location=mid)
        cyl = bpy.context.view_layer.objects.active
        cyl.name = f"BoneProxy_{name}"
        direction = (tail - head).normalized()
        cyl.rotation_mode = "QUATERNION"
        cyl.rotation_quaternion = direction.to_track_quat("Z", "Y")
        cyl.data.materials.append(mat)
        cyl.parent = arm_obj
        proxies.append(cyl)
    return proxies


def remove_bone_proxies(proxies):
    for p in proxies:
        bpy.data.objects.remove(p, do_unlink=True)


def render_proofs(mesh, arm_obj, bone_layout, out_dir):
    scene = bpy.context.scene
    scene.render.engine = "BLENDER_WORKBENCH"
    scene.display.shading.light = "STUDIO"
    scene.display.shading.color_type = "TEXTURE"
    scene.render.resolution_x = 720
    scene.render.resolution_y = 900

    cam_data = bpy.data.cameras.new("ProofCam")
    cam = bpy.data.objects.new("ProofCam", cam_data)
    scene.collection.objects.link(cam)
    scene.camera = cam
    target = Vector((0.0, 0.0, 0.95))

    def shoot(name, offset):
        cam.location = target + Vector(offset)
        direction = target - cam.location
        cam.rotation_euler = direction.to_track_quat("-Z", "Y").to_euler()
        scene.render.filepath = os.path.join(out_dir, f"proof_{name}.png")
        bpy.ops.render.render(write_still=True)

    # Pass 1: skeleton check — mesh half transparent is not available in
    # workbench texture mode, so render skeleton with mesh hidden + with mesh.
    proxies = add_bone_proxies(arm_obj, bone_layout)
    clear_pose(arm_obj)
    if arm_obj.animation_data:
        arm_obj.animation_data.action = None
    scene.frame_set(1)
    shoot("skeleton_front_with_mesh", (0.0, -3.4, 0.1))
    shoot("skeleton_side_with_mesh", (3.4, 0.0, 0.1))
    mesh.hide_render = True
    shoot("skeleton_front_only", (0.0, -3.4, 0.1))
    mesh.hide_render = False
    remove_bone_proxies(proxies)

    # Pass 2: rest + posed deformation checks (weights now drive the mesh).
    shoot("front_rest", (0.0, -3.4, 0.1))
    shoot("side_rest", (3.4, 0.0, 0.1))
    shoot("iso_rest", (2.4, -2.4, 0.7))

    def shoot_pose(name, action_name, frame, offset):
        action = bpy.data.actions.get(action_name)
        if not action:
            return
        arm_obj.animation_data_create()
        arm_obj.animation_data.action = action
        scene.frame_set(frame)
        shoot(name, offset)

    shoot_pose("walk_contact", "Walk", 1, (3.4, 0.0, 0.1))
    shoot_pose("walk_pass", "Walk", 9, (3.4, 0.0, 0.1))
    shoot_pose("jump_crouch", "Jump", 6, (3.4, 0.0, 0.1))
    shoot_pose("dive_superman", "Dive", 20, (3.4, 0.0, 0.1))
    shoot_pose("getupfront_kneel", "GetUp_Front", 16, (3.4, 0.0, 0.1))
    clear_pose(arm_obj)
    if arm_obj.animation_data:
        arm_obj.animation_data.action = None


# ---------------------------------------------------------------------------
# main
# ---------------------------------------------------------------------------

def main():
    args = parse_args()
    glb_path = os.path.abspath(args.glb)
    out_dir = os.path.abspath(args.out)
    anim_dir = os.path.join(out_dir, "AnimationSources")
    proof_dir = os.path.join(out_dir, "ProofRenders")
    for d in (out_dir, anim_dir, proof_dir):
        os.makedirs(d, exist_ok=True)

    reset_scene()
    mesh = import_glb(glb_path)
    height, flipped, lean_slope = normalize_mesh(mesh, args.front)
    landmarks = measure_landmarks(mesh, height, args.pose)
    arm_obj, bone_layout = build_armature(landmarks)
    skin_mesh(mesh, arm_obj, bone_layout)
    if args.pose == "tpose":
        # Skinned in T (clean weights); rest pose becomes relaxed hanging arms
        # so everything downstream keeps the proven conventions.
        apply_tpose_relaxation(mesh, arm_obj)
    qa_weights = weights_qa(mesh)

    # Author clips and shoot proofs at meter scale, then export (the FBX
    # exporter handles the m->cm unit conversion — see units NOTE above).
    clip_actions = {}
    for clip_name, frames, keyer in CLIPS:
        clip_actions[clip_name] = (make_action(arm_obj, clip_name, frames, keyer), frames)

    if not args.no_render:
        render_proofs(mesh, arm_obj, bone_layout, proof_dir)

    # UNITS (final doctrine v2 — raw-FBX-probe ground truth, 2026-06-10):
    # the exporter converts NOTHING m->cm. A meter-scene export writes meter
    # numbers everywhere (rest bones, anim curves, verts) and compensates
    # with scale=100 on the armature/mesh OBJECT nodes. UE then builds a
    # skeleton whose root bone carries scale 100 — which the physics-asset
    # generator and world-space bone writes ignore (bodies collapse to one
    # point, zero-length constraint anchors, centimeter-sized render).
    # The earlier doctrine's "keyed channels arrive cm" was a misread of
    # that node-scale compensation. The only consistent form is REAL cm
    # numbers with scale 1 everywhere: convert_scene_to_centimeters() bakes
    # x100 into mesh+armature (applied) + location fcurves and declares the
    # scene cm, so the exporter's unit factor is exactly 1.0 and no node
    # scale compensation is emitted. (The historical "x100 kaiju" failure
    # was the v4 DOUBLE bake: x100 data exported from a meter-declared
    # scene — not this recipe.)
    convert_scene_to_centimeters(mesh, arm_obj)

    bind_action = make_bind_pose_action(arm_obj)
    skeletal_fbx = os.path.join(out_dir, f"SK_MotionRig_{args.name}.fbx")
    export_skeletal_fbx(mesh, arm_obj, bind_action, skeletal_fbx)

    albedo_png = os.path.join(out_dir, f"T_MotionRig_{args.name}_BaseColor.png")
    albedo_exported = export_base_color_png(mesh, albedo_png)

    clip_files = {}
    for clip_name, (action, frames) in clip_actions.items():
        clip_path = os.path.join(anim_dir, f"AM_MotionRig_{args.name}_{clip_name}.fbx")
        export_clip_fbx(arm_obj, action, frames, clip_path)
        clip_files[clip_name] = {"path": clip_path, "frames": frames}

    bpy.ops.wm.save_as_mainfile(filepath=os.path.join(out_dir, f"MotionRig_{args.name}.blend"))

    qa = {
        "source_glb": glb_path,
        "character_name": args.name,
        "source_pose": args.pose,
        "lean_slope_removed": lean_slope,
        "base_color_exported": albedo_exported,
        "normalized_height_m": height,
        "facing_flip_applied": flipped,
        "bone_count": len(bone_layout),
        "bones": bone_layout,
        "weights": qa_weights,
        "clips": {k: v["frames"] for k, v in clip_files.items()},
        "front_axis_blender": "-Y",
        "expected_unreal_forward": "+X",
        "landmarks": {
            "shoulder_half_w": landmarks["shoulder_half_w"],
            "leg_half_x": landmarks["leg_half_x"],
            "leg_center_y": landmarks["leg_center_y"],
            "left_hand_tip": list(landmarks["left_hand_tip"]),
            "right_hand_tip": list(landmarks["right_hand_tip"]),
        },
        "pass": qa_weights["unweighted"] == 0 and qa_weights["max_influences"] <= MAX_INFLUENCES and len(bone_layout) == 18 and albedo_exported,
    }
    with open(os.path.join(out_dir, "MotionRig_QA.json"), "w") as f:
        json.dump(qa, f, indent=2)

    print("MOTIONRIG_BUILD_RESULT=" + ("PASS" if qa["pass"] else "FAIL"))
    print(json.dumps({k: v for k, v in qa.items() if k != "bones"}, indent=2))


if __name__ == "__main__":
    main()
