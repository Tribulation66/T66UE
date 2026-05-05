import importlib.util
import json
import math
from pathlib import Path

import bpy
from mathutils import Matrix, Vector


MODEL_GEN_ROOT = Path(__file__).resolve().parents[1]
RIG_MODULE_PATH = MODEL_GEN_ROOT / "Scripts" / "rig_typea_mike_prototype.py"

SOURCE_GLB = (
    MODEL_GEN_ROOT
    / "Runs"
    / "Heroes"
    / "Chad_Pass02_ProcessBuild"
    / "Assembly"
    / "HeadBody"
    / "Hero_3_Mike_Chad_RigPrototype_A03_LiftedNeckBridge.glb"
)
SWORD_GLB = MODEL_GEN_ROOT / "Runs" / "Arthur" / "Raw" / "Arthur_ExcaliburProxy_FlatGreen_Tight_S1337_Trellis2.glb"

OUT_ROOT = (
    MODEL_GEN_ROOT
    / "Runs"
    / "Heroes"
    / "Chad_Pass02_ProcessBuild"
    / "Rigging"
    / "Mike_Chad_RigPrototype_A03_LiftedNeckBridge"
)
SCENE_OUT = (
    MODEL_GEN_ROOT
    / "Scenes"
    / "Chad_Pass02_Mike_Rigging"
    / "Mike_Chad_RigPrototype_A03_LiftedNeckBridge_Rigged.blend"
)

TARGET_HEIGHT_M = 2.0

REST_VIEWS = {
    "front": 0.0,
    "right": 90.0,
    "back": 180.0,
    "left_oblique": -35.0,
}


def load_rig_module():
    spec = importlib.util.spec_from_file_location("rig_typea_mike_prototype", str(RIG_MODULE_PATH))
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


rig = load_rig_module()


def vector_to_list(value):
    return [float(value.x), float(value.y), float(value.z)]


def mesh_vertex_bounds(objects):
    min_v = Vector((math.inf, math.inf, math.inf))
    max_v = Vector((-math.inf, -math.inf, -math.inf))
    for obj in objects:
        if obj.type != "MESH":
            continue
        world = obj.matrix_world
        for vertex in obj.data.vertices:
            point = world @ vertex.co
            min_v.x = min(min_v.x, point.x)
            min_v.y = min(min_v.y, point.y)
            min_v.z = min(min_v.z, point.z)
            max_v.x = max(max_v.x, point.x)
            max_v.y = max(max_v.y, point.y)
            max_v.z = max(max_v.z, point.z)
    return min_v, max_v, max_v - min_v


def bake_world_transform(obj):
    world = obj.matrix_world.copy()
    obj.parent = None
    obj.matrix_world = Matrix.Identity(4)
    obj.data.transform(world)
    obj.matrix_world = Matrix.Identity(4)
    obj.data.update()


def remove_orphan_empties(imported_objects=None):
    imported = set(imported_objects) if imported_objects else None
    for obj in list(bpy.context.scene.objects):
        if obj.type != "EMPTY":
            continue
        if imported is not None and obj not in imported:
            continue
        if obj.children:
            continue
        bpy.data.objects.remove(obj, do_unlink=True)


def import_character():
    bpy.ops.import_scene.gltf(filepath=str(SOURCE_GLB))
    meshes = [obj for obj in bpy.context.scene.objects if obj.type == "MESH"]
    if len(meshes) < 3:
        raise RuntimeError(f"Expected body, head, and neck bridge meshes from {SOURCE_GLB.name}, got {len(meshes)}")

    body = next((obj for obj in meshes if "Body" in obj.name), None)
    head = next((obj for obj in meshes if "Head" in obj.name), None)
    neck = next((obj for obj in meshes if "NeckBridge" in obj.name or "MikeRepair" in obj.name), None)
    if not body or not head or not neck:
        by_height = sorted(meshes, key=lambda obj: mesh_vertex_bounds([obj])[2].z, reverse=True)
        body = body or by_height[0]
        head = head or by_height[1]
        neck = neck or by_height[-1]

    for obj in (body, head, neck):
        bake_world_transform(obj)

    remove_orphan_empties()

    body.name = "Mike_Chad_Body"
    body.data.name = "Mike_Chad_Body_Mesh"
    head.name = "Mike_Chad_Head"
    head.data.name = "Mike_Chad_Head_Mesh"
    neck.name = "Mike_Chad_NeckBridge"
    neck.data.name = "Mike_Chad_NeckBridge_Mesh"

    return body, head, neck


def normalize_to_feet_origin(meshes):
    original_min, original_max, original_size = mesh_vertex_bounds(meshes)
    if original_size.z <= 0.001:
        raise RuntimeError("Cannot normalize rig source with near-zero height")

    foot_center = Vector(
        (
            (original_min.x + original_max.x) * 0.5,
            (original_min.y + original_max.y) * 0.5,
            original_min.z,
        )
    )
    scale = TARGET_HEIGHT_M / original_size.z
    transform = Matrix.Scale(scale, 4) @ Matrix.Translation(-foot_center)
    for obj in meshes:
        obj.data.transform(transform)
        obj.data.update()
    bpy.context.view_layer.update()

    normalized_min, normalized_max, normalized_size = mesh_vertex_bounds(meshes)
    return {
        "target_height_m": TARGET_HEIGHT_M,
        "source_min_m": vector_to_list(original_min),
        "source_max_m": vector_to_list(original_max),
        "source_size_m": vector_to_list(original_size),
        "source_foot_center_m": vector_to_list(foot_center),
        "baked_scale": float(scale),
        "normalized_min_m": vector_to_list(normalized_min),
        "normalized_max_m": vector_to_list(normalized_max),
        "normalized_size_m": vector_to_list(normalized_size),
    }


def rename_armature(armature):
    armature.name = "Rig_Mike_Chad_Process"
    armature.data.name = "Rig_Mike_Chad_Process_Data"


def weight_neck_bridge(neck):
    rig.ensure_groups(neck)
    min_v, max_v, size = mesh_vertex_bounds([neck])
    height = max(size.z, 0.001)
    for vert in neck.data.vertices:
        world = neck.matrix_world @ vert.co
        zn = (world.z - min_v.z) / height
        if zn < 0.20:
            weights = {"neck_01": 1.0}
        elif zn < 0.82:
            t = (zn - 0.20) / 0.62
            weights = {"neck_01": 1.0 - t, "head": t}
        else:
            weights = {"head": 1.0}
        rig.add_weight(neck, vert.index, weights)


def estimate_right_hand_grip(body):
    body_min, body_max, body_size = mesh_vertex_bounds([body])
    return Vector(
        (
            body_max.x - body_size.x * 0.105,
            body_min.y + body_size.y * 0.32,
            body_min.z + body_size.z * 0.47,
        )
    )


def import_sword_proxy(points, body):
    before = set(bpy.context.scene.objects)
    bpy.ops.import_scene.gltf(filepath=str(SWORD_GLB))
    imported = [obj for obj in bpy.context.scene.objects if obj not in before]
    meshes = [obj for obj in imported if obj.type == "MESH"]
    if len(meshes) != 1:
        raise RuntimeError(f"Expected one sword mesh from {SWORD_GLB.name}, got {len(meshes)}")

    sword = meshes[0]
    bake_world_transform(sword)
    remove_orphan_empties(imported)

    sword.name = "Mike_Chad_SwordProxy"
    sword.data.name = "Mike_Chad_SwordProxy_Mesh"

    min_v, max_v, size = mesh_vertex_bounds([sword])
    sword_height = max(size.z, 0.001)
    # This source sword is modeled blade-down in its local vertical: the blade
    # tip is near min Z and the handle is near max Z. Attach the handle region
    # to the hand, not the lower blade tip.
    grip_center = Vector(((min_v.x + max_v.x) * 0.5, (min_v.y + max_v.y) * 0.5, max_v.z - sword_height * 0.13))

    target_length = TARGET_HEIGHT_M * 0.72
    scale = target_length / sword_height
    # Angle the blade roughly 45 degrees upward from the hip/hand forward line.
    # Character front is negative Y in these exports. The side-view target is a
    # roughly 45-degree forward/up blade line from the hip/hand, not a vertical
    # sword hanging beside the leg.
    blade_dir = Vector((0.035, -0.72, 0.693)).normalized()
    # The source blade direction is local -Z, so rotate local -Z to the target
    # forward/up direction.
    base_rotation = Vector((0.0, 0.0, -1.0)).rotation_difference(blade_dir).to_matrix().to_4x4()
    hilt_roll_degrees = -90.0
    # Roll around the blade axis so the guard/handle sits sideways with the
    # wrist instead of forming a flat bar across the hand in the front view.
    rotation = Matrix.Rotation(math.radians(hilt_roll_degrees), 4, blade_dir) @ base_rotation
    hand_bone_tail = Vector(points["hand_r"])
    hand_target = estimate_right_hand_grip(body)

    transform = Matrix.Translation(hand_target) @ rotation @ Matrix.Scale(scale, 4) @ Matrix.Translation(-grip_center)
    sword.data.transform(transform)
    sword.data.update()
    sword.matrix_world = Matrix.Identity(4)

    sword["attachment_bone"] = "hand_r"
    sword["attachment_policy"] = "Rigid hand_r vertex group plus shared armature modifier"
    sword["source_glb"] = str(SWORD_GLB)

    return sword, {
        "source": str(SWORD_GLB),
        "mesh": sword.name,
        "attachment_bone": "hand_r",
        "target_length_m": target_length,
        "blade_direction": vector_to_list(blade_dir),
        "hilt_roll_degrees": hilt_roll_degrees,
        "hand_bone_tail_m": vector_to_list(hand_bone_tail),
        "hand_target_m": vector_to_list(hand_target),
        "grip_source_policy": "Source sword handle is near max Z and blade points toward local -Z; attach the upper handle region to the hand, rotate the blade upward plus forward toward character-front, then roll the hilt sideways with the wrist.",
        "hand_target_policy": "Estimated from the visible right-hand volume because the generated hand bone tail sits too low for this stylized mesh.",
    }


def weight_weapon_to_hand(sword):
    rig.ensure_groups(sword)
    group = sword.vertex_groups.get("hand_r")
    indices = [vertex.index for vertex in sword.data.vertices]
    for other in sword.vertex_groups:
        try:
            other.remove(indices)
        except RuntimeError:
            pass
    group.add(indices, 1.0, "REPLACE")


def reset_pose(armature):
    bpy.context.view_layer.objects.active = armature
    bpy.ops.object.mode_set(mode="POSE")
    for pb in armature.pose.bones:
        pb.rotation_mode = "XYZ"
        pb.rotation_euler = (0.0, 0.0, 0.0)
        pb.location = (0.0, 0.0, 0.0)
    bpy.ops.object.mode_set(mode="OBJECT")
    bpy.context.view_layer.update()


def set_idle_pose(armature, phase):
    reset_pose(armature)
    p = float(phase)
    bpy.context.view_layer.objects.active = armature
    bpy.ops.object.mode_set(mode="POSE")
    bones = armature.pose.bones
    bones["pelvis"].rotation_euler = (0.0, math.radians(0.5 * p), math.radians(0.4 * p))
    bones["spine_01"].rotation_euler = (0.0, math.radians(-0.8 * p), math.radians(-0.3 * p))
    bones["spine_02"].rotation_euler = (0.0, math.radians(-0.6 * p), math.radians(-0.3 * p))
    bones["neck_01"].rotation_euler = (math.radians(0.4 * p), 0.0, 0.0)
    bones["head"].rotation_euler = (math.radians(-0.3 * p), 0.0, math.radians(0.4 * p))
    bpy.ops.object.mode_set(mode="OBJECT")
    bpy.context.view_layer.update()


def set_walk_pose(armature, pose_name):
    reset_pose(armature)
    bpy.context.view_layer.objects.active = armature
    bpy.ops.object.mode_set(mode="POSE")
    bones = armature.pose.bones

    if pose_name == "walk_left":
        bones["pelvis"].rotation_euler = (0.0, math.radians(2.0), math.radians(-1.5))
        bones["spine_01"].rotation_euler = (0.0, math.radians(-1.0), math.radians(0.8))
        bones["spine_02"].rotation_euler = (0.0, math.radians(-1.0), math.radians(0.6))
        bones["thigh_l"].rotation_euler = (math.radians(-28.0), 0.0, math.radians(-2.0))
        bones["calf_l"].rotation_euler = (math.radians(34.0), 0.0, 0.0)
        bones["foot_l"].rotation_euler = (math.radians(-3.0), 0.0, 0.0)
        bones["thigh_r"].rotation_euler = (math.radians(22.0), 0.0, math.radians(1.5))
        bones["calf_r"].rotation_euler = (math.radians(-14.0), 0.0, 0.0)
        bones["foot_r"].rotation_euler = (math.radians(3.0), 0.0, 0.0)
    elif pose_name == "walk_pass_left":
        bones["pelvis"].rotation_euler = (0.0, math.radians(1.0), 0.0)
        bones["thigh_l"].rotation_euler = (math.radians(-6.0), 0.0, math.radians(-1.0))
        bones["calf_l"].rotation_euler = (math.radians(22.0), 0.0, 0.0)
        bones["foot_l"].rotation_euler = (math.radians(-2.0), 0.0, 0.0)
        bones["thigh_r"].rotation_euler = (math.radians(8.0), 0.0, math.radians(1.0))
        bones["calf_r"].rotation_euler = (math.radians(8.0), 0.0, 0.0)
    elif pose_name == "walk_right":
        bones["pelvis"].rotation_euler = (0.0, math.radians(-2.0), math.radians(1.5))
        bones["spine_01"].rotation_euler = (0.0, math.radians(1.0), math.radians(-0.8))
        bones["spine_02"].rotation_euler = (0.0, math.radians(1.0), math.radians(-0.6))
        bones["thigh_l"].rotation_euler = (math.radians(22.0), 0.0, math.radians(-1.5))
        bones["calf_l"].rotation_euler = (math.radians(-14.0), 0.0, 0.0)
        bones["foot_l"].rotation_euler = (math.radians(3.0), 0.0, 0.0)
        bones["thigh_r"].rotation_euler = (math.radians(-28.0), 0.0, math.radians(2.0))
        bones["calf_r"].rotation_euler = (math.radians(34.0), 0.0, 0.0)
        bones["foot_r"].rotation_euler = (math.radians(-3.0), 0.0, 0.0)
    elif pose_name == "walk_pass_right":
        bones["pelvis"].rotation_euler = (0.0, math.radians(-1.0), 0.0)
        bones["thigh_l"].rotation_euler = (math.radians(8.0), 0.0, math.radians(-1.0))
        bones["calf_l"].rotation_euler = (math.radians(8.0), 0.0, 0.0)
        bones["thigh_r"].rotation_euler = (math.radians(-6.0), 0.0, math.radians(1.0))
        bones["calf_r"].rotation_euler = (math.radians(22.0), 0.0, 0.0)
        bones["foot_r"].rotation_euler = (math.radians(-2.0), 0.0, 0.0)
    else:
        raise ValueError(f"Unknown walk pose {pose_name}")

    bpy.ops.object.mode_set(mode="OBJECT")
    bpy.context.view_layer.update()


def insert_pose_keys(armature, frame):
    bpy.ops.object.mode_set(mode="POSE")
    for pb in armature.pose.bones:
        pb.keyframe_insert("rotation_euler", frame=frame)
        pb.keyframe_insert("location", frame=frame)
    bpy.ops.object.mode_set(mode="OBJECT")


def build_action(armature, name, frame_pose_pairs, pose_func):
    scene = bpy.context.scene
    action = bpy.data.actions.new(name)
    action.use_fake_user = True
    armature.animation_data_create()
    armature.animation_data.action = action
    for frame, pose in frame_pose_pairs:
        scene.frame_set(frame)
        pose_func(armature, pose)
        insert_pose_keys(armature, frame)
    return action


def keyframe_actions(armature):
    scene = bpy.context.scene
    scene.frame_start = 1
    scene.frame_end = 48
    idle = build_action(
        armature,
        "Mike_Chad_Idle",
        [(1, -1.0), (12, 0.0), (24, 1.0), (36, 0.0), (48, -1.0)],
        set_idle_pose,
    )
    walk = build_action(
        armature,
        "Mike_Chad_WalkLegsOnly",
        [
            (1, "walk_left"),
            (9, "walk_pass_left"),
            (17, "walk_right"),
            (25, "walk_pass_right"),
            (33, "walk_left"),
        ],
        set_walk_pose,
    )
    armature.animation_data.action = walk
    scene.frame_set(1)
    set_walk_pose(armature, "walk_left")
    return idle, walk


def add_camera(center, size, yaw_deg, pitch_deg=4.0, scale=1.22):
    cam_data = bpy.data.cameras.new(name=f"RigQA_Camera_{yaw_deg:g}")
    cam_data.type = "ORTHO"
    cam = bpy.data.objects.new(cam_data.name, cam_data)
    bpy.context.scene.collection.objects.link(cam)
    bpy.context.scene.camera = cam
    yaw = math.radians(yaw_deg)
    pitch = math.radians(pitch_deg)
    direction = Vector((math.sin(yaw) * math.cos(pitch), -math.cos(yaw) * math.cos(pitch), math.sin(pitch)))
    cam.location = center + direction * 4.0
    target = bpy.data.objects.new(f"RigQA_Target_{yaw_deg:g}", None)
    target.location = center
    bpy.context.scene.collection.objects.link(target)
    track = cam.constraints.new(type="TRACK_TO")
    track.target = target
    track.track_axis = "TRACK_NEGATIVE_Z"
    track.up_axis = "UP_Y"
    cam_data.ortho_scale = max(size.x, size.z) * scale
    return cam


def clear_qa_cameras():
    for obj in list(bpy.data.objects):
        if obj.name.startswith("RigQA_Camera_") or obj.name.startswith("RigQA_Target_"):
            bpy.data.objects.remove(obj, do_unlink=True)


def all_meshes():
    return [obj for obj in bpy.context.scene.objects if obj.type == "MESH"]


def render_view(label, yaw, pose_label, resolution=1100, scale=1.24):
    render_dir = OUT_ROOT / "Renders"
    render_dir.mkdir(parents=True, exist_ok=True)
    min_v, max_v, size = mesh_vertex_bounds(all_meshes())
    center = (min_v + max_v) * 0.5
    clear_qa_cameras()
    add_camera(center, size, yaw, scale=scale)
    scene = bpy.context.scene
    scene.render.resolution_x = resolution
    scene.render.resolution_y = resolution
    out = render_dir / f"Mike_Chad_RigPrototype_{pose_label}_{label}.png"
    scene.render.filepath = str(out)
    bpy.ops.render.render(write_still=True)
    return out


def render_closeup(label, yaw, center, ortho_scale=0.48, resolution=1100):
    render_dir = OUT_ROOT / "Renders"
    render_dir.mkdir(parents=True, exist_ok=True)
    clear_qa_cameras()
    cam_data = bpy.data.cameras.new(name=f"RigQA_Camera_Close_{label}")
    cam_data.type = "ORTHO"
    cam_data.ortho_scale = ortho_scale
    cam = bpy.data.objects.new(cam_data.name, cam_data)
    bpy.context.scene.collection.objects.link(cam)
    bpy.context.scene.camera = cam
    yaw_rad = math.radians(yaw)
    pitch = math.radians(5.0)
    direction = Vector((math.sin(yaw_rad) * math.cos(pitch), -math.cos(yaw_rad) * math.cos(pitch), math.sin(pitch)))
    cam.location = center + direction * 2.4
    target = bpy.data.objects.new(f"RigQA_Target_Close_{label}", None)
    target.location = center
    bpy.context.scene.collection.objects.link(target)
    track = cam.constraints.new(type="TRACK_TO")
    track.target = target
    track.track_axis = "TRACK_NEGATIVE_Z"
    track.up_axis = "UP_Y"

    scene = bpy.context.scene
    scene.render.resolution_x = resolution
    scene.render.resolution_y = resolution
    out = render_dir / f"Mike_Chad_RigPrototype_{label}.png"
    scene.render.filepath = str(out)
    bpy.ops.render.render(write_still=True)
    return out


def select_export_objects(armature):
    bpy.ops.object.select_all(action="DESELECT")
    armature.select_set(True)
    for obj in all_meshes():
        obj.select_set(True)
    bpy.context.view_layer.objects.active = armature


def export_action_fbx(armature, action, out_path, frame_start, frame_end):
    armature.animation_data.action = action
    bpy.context.scene.frame_start = frame_start
    bpy.context.scene.frame_end = frame_end
    bpy.context.scene.frame_set(frame_start)
    select_export_objects(armature)
    bpy.ops.export_scene.fbx(
        filepath=str(out_path),
        use_selection=True,
        add_leaf_bones=False,
        bake_anim=True,
        bake_anim_use_all_actions=False,
        bake_anim_use_nla_strips=False,
        object_types={"ARMATURE", "MESH"},
        path_mode="COPY",
        embed_textures=True,
    )


def export_outputs(armature, idle_action, walk_action):
    export_dir = OUT_ROOT / "Exports"
    export_dir.mkdir(parents=True, exist_ok=True)
    glb_path = export_dir / "Hero_3_Mike_Chad_RigPrototype_A03_LiftedNeckBridge_RiggedWithSword.glb"
    fbx_path = export_dir / "Hero_3_Mike_Chad_RigPrototype_A03_LiftedNeckBridge_RiggedWithSword.fbx"
    idle_fbx_path = export_dir / "Hero_3_Mike_Chad_RigPrototype_A03_LiftedNeckBridge_Idle.fbx"
    walk_fbx_path = export_dir / "Hero_3_Mike_Chad_RigPrototype_A03_LiftedNeckBridge_WalkLegsOnly.fbx"

    select_export_objects(armature)
    bpy.ops.export_scene.gltf(
        filepath=str(glb_path),
        export_format="GLB",
        use_selection=True,
        export_animations=True,
        export_materials="EXPORT",
    )
    bpy.ops.export_scene.fbx(
        filepath=str(fbx_path),
        use_selection=True,
        add_leaf_bones=False,
        bake_anim=True,
        bake_anim_use_all_actions=True,
        object_types={"ARMATURE", "MESH"},
        path_mode="COPY",
        embed_textures=True,
    )
    export_action_fbx(armature, idle_action, idle_fbx_path, 1, 48)
    export_action_fbx(armature, walk_action, walk_fbx_path, 1, 33)
    return {
        "glb": glb_path,
        "fbx": fbx_path,
        "idle_fbx": idle_fbx_path,
        "walk_fbx": walk_fbx_path,
    }


def verify_exported_fbx(fbx_path):
    rig.reset_scene()
    bpy.ops.import_scene.fbx(filepath=str(fbx_path))
    armatures = [obj for obj in bpy.context.scene.objects if obj.type == "ARMATURE"]
    meshes = [obj for obj in bpy.context.scene.objects if obj.type == "MESH"]
    actions = [action.name for action in bpy.data.actions]
    skinned_meshes = [
        obj.name
        for obj in meshes
        if any(mod.type == "ARMATURE" and mod.object in armatures for mod in obj.modifiers)
    ]
    return {
        "fbx": str(fbx_path),
        "armature_count": len(armatures),
        "bone_count": len(armatures[0].data.bones) if armatures else 0,
        "mesh_count": len(meshes),
        "skinned_mesh_count": len(skinned_meshes),
        "actions": actions,
        "ok": len(armatures) == 1
        and len(armatures[0].data.bones) == 18
        and len(meshes) >= 4
        and len(skinned_meshes) >= 4
        and any("Mike_Chad_Idle" in action for action in actions)
        and any("Mike_Chad_WalkLegsOnly" in action for action in actions),
    }


def verify_exported_glb(glb_path):
    rig.reset_scene()
    bpy.ops.import_scene.gltf(filepath=str(glb_path))
    armatures = [obj for obj in bpy.context.scene.objects if obj.type == "ARMATURE"]
    meshes = [obj for obj in bpy.context.scene.objects if obj.type == "MESH"]
    actions = [action.name for action in bpy.data.actions]
    return {
        "glb": str(glb_path),
        "armature_count": len(armatures),
        "bone_count": len(armatures[0].data.bones) if armatures else 0,
        "mesh_count": len(meshes),
        "actions": actions,
        "ok": len(armatures) == 1
        and len(armatures[0].data.bones) == 18
        and len(meshes) >= 4
        and any("Mike_Chad_Idle" in action for action in actions)
        and any("Mike_Chad_WalkLegsOnly" in action for action in actions),
    }


def main():
    OUT_ROOT.mkdir(parents=True, exist_ok=True)
    SCENE_OUT.parent.mkdir(parents=True, exist_ok=True)

    rig.OUT_ROOT = OUT_ROOT
    rig.SCENE_OUT = SCENE_OUT
    rig.reset_scene()

    body, head, neck = import_character()
    normalization = normalize_to_feet_origin([body, head, neck])
    armature, points = rig.create_armature(body, head)
    rename_armature(armature)

    rig.weight_body(body)
    stabilized_components = rig.stabilize_small_components(body, max_vertices=100)
    rig.weight_head(head)
    weight_neck_bridge(neck)

    sword, weapon_attachment = import_sword_proxy(points, body)
    weight_weapon_to_hand(sword)

    for mesh in (body, head, neck, sword):
        rig.attach_armature(mesh, armature)

    rig.add_lights()
    idle_action, walk_action = keyframe_actions(armature)

    render_outputs = {}
    armature.animation_data.action = None

    bpy.context.scene.frame_set(1)
    reset_pose(armature)
    for view, yaw in REST_VIEWS.items():
        render_outputs[f"rest_{view}"] = str(render_view(view, yaw, "rest"))

    bpy.context.scene.frame_set(24)
    set_idle_pose(armature, 1.0)
    for view, yaw in {"front": 0.0, "right": 90.0, "back": 180.0}.items():
        render_outputs[f"idle_{view}"] = str(render_view(view, yaw, "idle"))

    bpy.context.scene.frame_set(1)
    set_walk_pose(armature, "walk_left")
    for view, yaw in {"front": 0.0, "right": 90.0, "left_oblique": -35.0}.items():
        render_outputs[f"walk_left_{view}"] = str(render_view(view, yaw, "walk_left"))

    bpy.context.scene.frame_set(17)
    set_walk_pose(armature, "walk_right")
    for view, yaw in {"front": 0.0, "right": 90.0, "left_oblique": -35.0}.items():
        render_outputs[f"walk_right_{view}"] = str(render_view(view, yaw, "walk_right"))

    reset_pose(armature)
    hand_center = Vector(weapon_attachment["hand_target_m"])
    render_outputs["weapon_grip_front_closeup"] = str(render_closeup("weapon_grip_front_closeup", 0.0, hand_center))
    render_outputs["weapon_grip_right_closeup"] = str(render_closeup("weapon_grip_right_closeup", 90.0, hand_center))
    neck_min, neck_max, _ = mesh_vertex_bounds([neck])
    neck_center = (neck_min + neck_max) * 0.5
    render_outputs["neck_right_closeup"] = str(render_closeup("neck_right_closeup", 90.0, neck_center, ortho_scale=0.55))

    armature.animation_data.action = walk_action
    bpy.ops.wm.save_as_mainfile(filepath=str(SCENE_OUT))
    exports = export_outputs(armature, idle_action, walk_action)

    preverify_report = {
        "source": str(SOURCE_GLB),
        "blend": str(SCENE_OUT),
        "exports": {key: str(path) for key, path in exports.items()},
        "renders": render_outputs,
        "weapon_attachment": weapon_attachment,
        "normalization": normalization,
        "armature": armature.name,
        "bones": [bone.name for bone in armature.data.bones],
        "deform_bones": rig.DEFORM_BONES,
        "mesh_vertex_groups": {
            obj.name: [group.name for group in obj.vertex_groups]
            for obj in (body, head, neck, sword)
        },
        "body_small_components_stabilized": stabilized_components,
        "animation": {
            "idle_action": idle_action.name,
            "walk_action": walk_action.name,
            "frame_start": bpy.context.scene.frame_start,
            "frame_end": bpy.context.scene.frame_end,
            "actions": [action.name for action in bpy.data.actions],
            "walk_policy": "lower body only; arms remain stable for weapon attachment review",
        },
    }

    fbx_check = verify_exported_fbx(exports["fbx"])
    glb_check = verify_exported_glb(exports["glb"])
    report = dict(preverify_report)
    report["verification"] = {
        "fbx_reimport": fbx_check,
        "glb_reimport": glb_check,
        "ok": fbx_check["ok"] and glb_check["ok"],
    }
    report_path = OUT_ROOT / "rig_report.json"
    report_path.write_text(json.dumps(report, indent=2), encoding="utf-8")
    print(json.dumps({"report": str(report_path), "exports": report["exports"], "ok": report["verification"]["ok"]}, indent=2))
    if not report["verification"]["ok"]:
        raise SystemExit(2)


if __name__ == "__main__":
    main()
