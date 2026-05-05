import json
import math
from pathlib import Path

import bpy
from mathutils import Matrix, Vector


MODEL_GEN_ROOT = Path(__file__).resolve().parents[1]
BATCH_ROOT = MODEL_GEN_ROOT / "Runs" / "Heroes" / "TypeA_Masculine_Batch01"
SOURCE_GLB = BATCH_ROOT / "Assembly" / "HeadBody" / "Hero_4_Mike_TypeA_Standard_HeadBody_Assembled.glb"
OUT_ROOT = BATCH_ROOT / "Rigging" / "Hero_4_Mike"
SCENE_OUT = MODEL_GEN_ROOT / "Scenes" / "TypeA_Batch01_Rigging" / "Hero_4_Mike_TypeA_Standard_Rigged.blend"


BODY_BONES = [
    "pelvis",
    "spine_01",
    "spine_02",
    "neck_01",
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
HEAD_BONE = "head"
DEFORM_BONES = BODY_BONES + [HEAD_BONE]


def scene_engine():
    engines = {item.identifier for item in bpy.types.RenderSettings.bl_rna.properties["engine"].enum_items}
    return "BLENDER_EEVEE_NEXT" if "BLENDER_EEVEE_NEXT" in engines else "BLENDER_EEVEE"


def reset_scene():
    bpy.ops.wm.read_factory_settings(use_empty=True)
    scene = bpy.context.scene
    scene.render.engine = scene_engine()
    scene.render.image_settings.file_format = "PNG"
    scene.view_settings.view_transform = "Standard"
    scene.world = bpy.data.worlds.new("RigQAWorld")
    scene.world.use_nodes = True
    bg = scene.world.node_tree.nodes["Background"]
    bg.inputs[0].default_value = (0.17, 0.17, 0.17, 1.0)
    bg.inputs[1].default_value = 0.8
    return scene


def import_source():
    bpy.ops.import_scene.gltf(filepath=str(SOURCE_GLB))
    meshes = [obj for obj in bpy.context.scene.objects if obj.type == "MESH"]
    if len(meshes) != 2:
        raise RuntimeError(f"Expected 2 meshes from assembled Mike GLB, got {len(meshes)}")
    # Use world bounds. The assembly GLB stores head placement in object
    # transforms, so local bound boxes can make the unit-normalized head and body
    # look similar and swap them by mistake.
    meshes_by_world_min_z = sorted(meshes, key=lambda obj: object_bounds([obj])[0].z)
    body = meshes_by_world_min_z[0]
    head = meshes_by_world_min_z[-1]
    body.name = "Mike_Body"
    head.name = "Mike_Head"
    # The assembled GLB can carry object-space transforms from the head/body
    # placement step. Armature modifiers expect the mesh and rig to agree in a
    # stable object space, so freeze transforms before generating weights.
    for obj in (body, head):
        world = obj.matrix_world.copy()
        obj.parent = None
        obj.matrix_world = Matrix.Identity(4)
        obj.data.transform(world)
        obj.matrix_world = Matrix.Identity(4)
        obj.data.update()
    for obj in list(bpy.context.scene.objects):
        if obj.type == "EMPTY" and obj.name.endswith("_world"):
            bpy.data.objects.remove(obj, do_unlink=True)
    return body, head


def object_bounds(objects):
    min_v = Vector((math.inf, math.inf, math.inf))
    max_v = Vector((-math.inf, -math.inf, -math.inf))
    for obj in objects:
        for corner in obj.bound_box:
            world = obj.matrix_world @ Vector(corner)
            min_v.x = min(min_v.x, world.x)
            min_v.y = min(min_v.y, world.y)
            min_v.z = min(min_v.z, world.z)
            max_v.x = max(max_v.x, world.x)
            max_v.y = max(max_v.y, world.y)
            max_v.z = max(max_v.z, world.z)
    return min_v, max_v, max_v - min_v


def create_armature(body, head):
    body_min, body_max, body_size = object_bounds([body])
    head_min, head_max, head_size = object_bounds([head])
    h = body_size.z
    cx = (body_min.x + body_max.x) * 0.5
    cy = (body_min.y + body_max.y) * 0.5

    def p(x, y, z):
        return (cx + x * h, cy + y * h, body_min.z + z * h)

    # Positive Y is toward the character back in these exports; negative Y is front.
    points = {
        "root": p(0.0, 0.0, 0.41),
        "pelvis": p(0.0, 0.0, 0.47),
        "spine_01": p(0.0, 0.0, 0.62),
        "spine_02": p(0.0, 0.0, 0.82),
        "neck_01": p(0.0, -0.004, 0.965),
        "head": ((head_min.x + head_max.x) * 0.5, (head_min.y + head_max.y) * 0.5, (head_min.z + head_max.z) * 0.5),
        "head_top": ((head_min.x + head_max.x) * 0.5, (head_min.y + head_max.y) * 0.5, head_max.z),
        "shoulder_l": p(-0.175, 0.0, 0.805),
        "elbow_l": p(-0.235, -0.005, 0.555),
        "wrist_l": p(-0.235, -0.010, 0.320),
        "hand_l": p(-0.235, -0.020, 0.250),
        "shoulder_r": p(0.175, 0.0, 0.805),
        "elbow_r": p(0.235, -0.005, 0.555),
        "wrist_r": p(0.235, -0.010, 0.320),
        "hand_r": p(0.235, -0.020, 0.250),
        "hip_l": p(-0.090, 0.0, 0.405),
        "knee_l": p(-0.090, 0.0, 0.210),
        "ankle_l": p(-0.090, -0.004, 0.045),
        "toe_l": p(-0.090, -0.070, 0.005),
        "hip_r": p(0.090, 0.0, 0.405),
        "knee_r": p(0.090, 0.0, 0.210),
        "ankle_r": p(0.090, -0.004, 0.045),
        "toe_r": p(0.090, -0.070, 0.005),
    }

    arm_data = bpy.data.armatures.new("TypeA_Mike_DeformRig_Data")
    arm_obj = bpy.data.objects.new("TypeA_Mike_DeformRig", arm_data)
    bpy.context.scene.collection.objects.link(arm_obj)
    bpy.context.view_layer.objects.active = arm_obj
    arm_obj.select_set(True)
    bpy.ops.object.mode_set(mode="EDIT")
    arm_data.edit_bones.remove(arm_data.edit_bones[0]) if arm_data.edit_bones else None

    def bone(name, head_key, tail_key, parent=None, connect=False):
        edit = arm_data.edit_bones.new(name)
        edit.head = points[head_key]
        edit.tail = points[tail_key]
        edit.roll = 0.0
        if parent:
            edit.parent = arm_data.edit_bones[parent]
            edit.use_connect = connect
        return edit

    bone("root", "root", "pelvis")
    bone("pelvis", "pelvis", "spine_01", parent="root", connect=True)
    bone("spine_01", "spine_01", "spine_02", parent="pelvis", connect=True)
    bone("spine_02", "spine_02", "neck_01", parent="spine_01", connect=True)
    bone("neck_01", "neck_01", "head", parent="spine_02", connect=True)
    bone("head", "head", "head_top", parent="neck_01", connect=True)
    bone("upperarm_l", "shoulder_l", "elbow_l", parent="spine_02")
    bone("lowerarm_l", "elbow_l", "wrist_l", parent="upperarm_l", connect=True)
    bone("hand_l", "wrist_l", "hand_l", parent="lowerarm_l", connect=True)
    bone("upperarm_r", "shoulder_r", "elbow_r", parent="spine_02")
    bone("lowerarm_r", "elbow_r", "wrist_r", parent="upperarm_r", connect=True)
    bone("hand_r", "wrist_r", "hand_r", parent="lowerarm_r", connect=True)
    bone("thigh_l", "hip_l", "knee_l", parent="pelvis")
    bone("calf_l", "knee_l", "ankle_l", parent="thigh_l", connect=True)
    bone("foot_l", "ankle_l", "toe_l", parent="calf_l", connect=True)
    bone("thigh_r", "hip_r", "knee_r", parent="pelvis")
    bone("calf_r", "knee_r", "ankle_r", parent="thigh_r", connect=True)
    bone("foot_r", "ankle_r", "toe_r", parent="calf_r", connect=True)

    for edit in arm_data.edit_bones:
        edit.use_deform = edit.name in DEFORM_BONES

    bpy.ops.object.mode_set(mode="OBJECT")
    arm_obj.show_in_front = True
    return arm_obj, points


def ensure_groups(mesh_obj):
    existing = {group.name for group in mesh_obj.vertex_groups}
    for name in DEFORM_BONES:
        if name not in existing:
            mesh_obj.vertex_groups.new(name=name)


def add_weight(mesh_obj, vertex_index, weights):
    total = sum(max(0.0, value) for value in weights.values())
    if total <= 0:
        return
    for group_name, value in weights.items():
        value = max(0.0, value) / total
        if value > 0.001:
            mesh_obj.vertex_groups[group_name].add([vertex_index], value, "REPLACE")


def smooth_pair(low_name, high_name, t):
    t = max(0.0, min(1.0, t))
    return {low_name: 1.0 - t, high_name: t}


def weight_body(body):
    ensure_groups(body)
    body_min, body_max, body_size = object_bounds([body])
    h = body_size.z
    cx = (body_min.x + body_max.x) * 0.5
    cy = (body_min.y + body_max.y) * 0.5

    for vert in body.data.vertices:
        w = body.matrix_world @ vert.co
        zn = (w.z - body_min.z) / h
        xn = (w.x - cx) / h
        ax = abs(xn)

        # Arms hang down beside the torso. Restrict by X first so chest vertices
        # do not get pulled by arm rotations.
        arm_threshold = 0.215 if zn < 0.50 else 0.155
        if ax > arm_threshold and 0.20 < zn < 0.86:
            side = "r" if xn > 0 else "l"
            if zn < 0.34:
                weights = {f"hand_{side}": 1.0}
            elif zn < 0.56:
                weights = smooth_pair(f"hand_{side}", f"lowerarm_{side}", (zn - 0.34) / 0.22)
            elif zn < 0.75:
                weights = smooth_pair(f"lowerarm_{side}", f"upperarm_{side}", (zn - 0.56) / 0.19)
            else:
                weights = smooth_pair(f"upperarm_{side}", "spine_02", (zn - 0.75) / 0.11)
            add_weight(body, vert.index, weights)
            continue

        # Legs below pelvis. Keep the center crotch/pelvis weighted to pelvis.
        if zn < 0.42 and ax > 0.060:
            side = "r" if xn > 0 else "l"
            if zn < 0.115:
                weights = {f"foot_{side}": 1.0}
            elif zn < 0.285:
                weights = {f"calf_{side}": 1.0}
            elif zn < 0.385:
                weights = smooth_pair(f"calf_{side}", f"thigh_{side}", (zn - 0.285) / 0.10)
            else:
                weights = smooth_pair(f"thigh_{side}", "pelvis", (zn - 0.385) / 0.035)
            add_weight(body, vert.index, weights)
            continue

        if zn < 0.46:
            weights = {"pelvis": 1.0}
        elif zn < 0.64:
            weights = smooth_pair("pelvis", "spine_01", (zn - 0.46) / 0.18)
        elif zn < 0.82:
            weights = smooth_pair("spine_01", "spine_02", (zn - 0.64) / 0.18)
        elif zn < 0.96:
            weights = smooth_pair("spine_02", "neck_01", (zn - 0.82) / 0.14)
        else:
            weights = {"neck_01": 1.0}
        add_weight(body, vert.index, weights)


def stabilize_small_components(mesh_obj, max_vertices=100, compact_max_vertices=600, compact_z_span=0.19):
    bounds_min, bounds_max, bounds_size = object_bounds([mesh_obj])
    adjacency = [[] for _ in mesh_obj.data.vertices]
    for edge in mesh_obj.data.edges:
        a, b = edge.vertices
        adjacency[a].append(b)
        adjacency[b].append(a)

    group_by_index = {group.index: group for group in mesh_obj.vertex_groups}
    group_by_name = {group.name: group for group in mesh_obj.vertex_groups}
    seen = [False] * len(mesh_obj.data.vertices)
    stabilized = 0

    for start_index in range(len(mesh_obj.data.vertices)):
        if seen[start_index]:
            continue
        stack = [start_index]
        seen[start_index] = True
        component = []
        while stack:
            vertex_index = stack.pop()
            component.append(vertex_index)
            for neighbor in adjacency[vertex_index]:
                if not seen[neighbor]:
                    seen[neighbor] = True
                    stack.append(neighbor)

        min_v = Vector((math.inf, math.inf, math.inf))
        max_v = Vector((-math.inf, -math.inf, -math.inf))
        for vertex_index in component:
            world = mesh_obj.matrix_world @ mesh_obj.data.vertices[vertex_index].co
            min_v.x = min(min_v.x, world.x)
            min_v.y = min(min_v.y, world.y)
            min_v.z = min(min_v.z, world.z)
            max_v.x = max(max_v.x, world.x)
            max_v.y = max(max_v.y, world.y)
            max_v.z = max(max_v.z, world.z)
        component_size = max_v - min_v

        is_small = len(component) <= max_vertices
        is_compact_detail = len(component) <= compact_max_vertices and component_size.z <= bounds_size.z * compact_z_span
        if not (is_small or is_compact_detail):
            continue

        totals = {name: 0.0 for name in DEFORM_BONES}
        for vertex_index in component:
            for vertex_group in mesh_obj.data.vertices[vertex_index].groups:
                group = group_by_index.get(vertex_group.group)
                if group and group.name in totals:
                    totals[group.name] += vertex_group.weight

        dominant_name, dominant_weight = max(totals.items(), key=lambda item: item[1])
        if dominant_weight <= 0.0:
            continue

        for group_name in DEFORM_BONES:
            try:
                group_by_name[group_name].remove(component)
            except RuntimeError:
                pass
        group_by_name[dominant_name].add(component, 1.0, "REPLACE")
        stabilized += 1

    return stabilized


def weight_head(head):
    ensure_groups(head)
    head_min, head_max, head_size = object_bounds([head])
    for vert in head.data.vertices:
        w = head.matrix_world @ vert.co
        zn = (w.z - head_min.z) / max(head_size.z, 0.001)
        if zn < 0.10:
            add_weight(head, vert.index, {"neck_01": 0.35, "head": 0.65})
        else:
            add_weight(head, vert.index, {"head": 1.0})


def attach_armature(mesh_obj, armature):
    mod = mesh_obj.modifiers.new("TypeA_DeformRig", "ARMATURE")
    mod.object = armature
    mesh_obj.parent = armature
    mesh_obj.matrix_parent_inverse = armature.matrix_world.inverted()


def add_lights():
    bpy.ops.object.light_add(type="AREA", location=(0.0, -4.0, 3.5))
    key = bpy.context.object
    key.name = "RigQA_Key_Light"
    key.data.energy = 650
    key.data.size = 4.5
    bpy.ops.object.light_add(type="POINT", location=(-3.0, 2.0, 2.6))
    fill = bpy.context.object
    fill.name = "RigQA_Fill_Light"
    fill.data.energy = 120


def all_mesh_bounds():
    meshes = [obj for obj in bpy.context.scene.objects if obj.type == "MESH"]
    return object_bounds(meshes)


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


def clear_qa_cameras():
    for obj in list(bpy.data.objects):
        if obj.name.startswith("RigQA_Camera_") or obj.name.startswith("RigQA_Target_"):
            bpy.data.objects.remove(obj, do_unlink=True)


def render_view(label, yaw, pose_label, resolution=1200, scale=1.22):
    render_dir = OUT_ROOT / "Renders"
    render_dir.mkdir(parents=True, exist_ok=True)
    min_v, max_v, size = all_mesh_bounds()
    center = (min_v + max_v) * 0.5
    clear_qa_cameras()
    add_camera(center, size, yaw, scale=scale)
    scene = bpy.context.scene
    scene.render.resolution_x = resolution
    scene.render.resolution_y = resolution
    out = render_dir / f"Hero_4_Mike_Rigged_{pose_label}_{label}.png"
    scene.render.filepath = str(out)
    bpy.ops.render.render(write_still=True)
    return out


def set_pose(armature, pose_name):
    bpy.context.view_layer.objects.active = armature
    bpy.ops.object.mode_set(mode="POSE")
    for pb in armature.pose.bones:
        pb.rotation_mode = "XYZ"
        pb.rotation_euler = (0.0, 0.0, 0.0)
        pb.location = (0.0, 0.0, 0.0)

    if pose_name == "action":
        # Deliberately asymmetric so screenshots prove bones are driving mesh,
        # not just being exported as an inert skeleton.
        armature.pose.bones["head"].rotation_euler = (0.0, 0.0, math.radians(12.0))
        armature.pose.bones["spine_01"].rotation_euler = (0.0, math.radians(-2.0), 0.0)
        armature.pose.bones["spine_02"].rotation_euler = (0.0, math.radians(-3.0), math.radians(-2.0))
        armature.pose.bones["upperarm_l"].rotation_euler = (math.radians(-6.0), 0.0, math.radians(-14.0))
        armature.pose.bones["lowerarm_l"].rotation_euler = (0.0, 0.0, math.radians(-8.0))
        armature.pose.bones["upperarm_r"].rotation_euler = (math.radians(4.0), 0.0, math.radians(12.0))
        armature.pose.bones["lowerarm_r"].rotation_euler = (0.0, 0.0, math.radians(7.0))
    elif pose_name == "leg_test":
        armature.pose.bones["pelvis"].rotation_euler = (0.0, math.radians(2.0), 0.0)
        armature.pose.bones["spine_01"].rotation_euler = (0.0, math.radians(-1.5), 0.0)
        armature.pose.bones["thigh_l"].rotation_euler = (math.radians(-18.0), 0.0, math.radians(-2.5))
        armature.pose.bones["calf_l"].rotation_euler = (math.radians(20.0), 0.0, 0.0)
        armature.pose.bones["foot_l"].rotation_euler = (math.radians(-2.0), 0.0, 0.0)
        armature.pose.bones["thigh_r"].rotation_euler = (math.radians(15.0), 0.0, math.radians(2.0))
        armature.pose.bones["calf_r"].rotation_euler = (math.radians(-12.0), 0.0, 0.0)
        armature.pose.bones["foot_r"].rotation_euler = (math.radians(2.0), 0.0, 0.0)
    elif pose_name == "walk_left":
        armature.pose.bones["pelvis"].rotation_euler = (0.0, math.radians(2.0), math.radians(-1.5))
        armature.pose.bones["spine_01"].rotation_euler = (0.0, math.radians(-1.0), math.radians(1.0))
        armature.pose.bones["spine_02"].rotation_euler = (0.0, math.radians(-1.5), math.radians(1.0))
        armature.pose.bones["upperarm_l"].rotation_euler = (math.radians(6.0), 0.0, math.radians(-6.0))
        armature.pose.bones["lowerarm_l"].rotation_euler = (0.0, 0.0, math.radians(-4.0))
        armature.pose.bones["upperarm_r"].rotation_euler = (math.radians(-7.0), 0.0, math.radians(6.0))
        armature.pose.bones["lowerarm_r"].rotation_euler = (0.0, 0.0, math.radians(4.0))
        armature.pose.bones["thigh_l"].rotation_euler = (math.radians(-28.0), 0.0, math.radians(-2.0))
        armature.pose.bones["calf_l"].rotation_euler = (math.radians(34.0), 0.0, 0.0)
        armature.pose.bones["foot_l"].rotation_euler = (math.radians(-3.0), 0.0, 0.0)
        armature.pose.bones["thigh_r"].rotation_euler = (math.radians(22.0), 0.0, math.radians(1.5))
        armature.pose.bones["calf_r"].rotation_euler = (math.radians(-14.0), 0.0, 0.0)
        armature.pose.bones["foot_r"].rotation_euler = (math.radians(3.0), 0.0, 0.0)
    elif pose_name == "walk_right":
        armature.pose.bones["pelvis"].rotation_euler = (0.0, math.radians(-2.0), math.radians(1.5))
        armature.pose.bones["spine_01"].rotation_euler = (0.0, math.radians(1.0), math.radians(-1.0))
        armature.pose.bones["spine_02"].rotation_euler = (0.0, math.radians(1.5), math.radians(-1.0))
        armature.pose.bones["upperarm_l"].rotation_euler = (math.radians(-7.0), 0.0, math.radians(-6.0))
        armature.pose.bones["lowerarm_l"].rotation_euler = (0.0, 0.0, math.radians(-4.0))
        armature.pose.bones["upperarm_r"].rotation_euler = (math.radians(6.0), 0.0, math.radians(6.0))
        armature.pose.bones["lowerarm_r"].rotation_euler = (0.0, 0.0, math.radians(4.0))
        armature.pose.bones["thigh_l"].rotation_euler = (math.radians(22.0), 0.0, math.radians(-1.5))
        armature.pose.bones["calf_l"].rotation_euler = (math.radians(-14.0), 0.0, 0.0)
        armature.pose.bones["foot_l"].rotation_euler = (math.radians(3.0), 0.0, 0.0)
        armature.pose.bones["thigh_r"].rotation_euler = (math.radians(-28.0), 0.0, math.radians(2.0))
        armature.pose.bones["calf_r"].rotation_euler = (math.radians(34.0), 0.0, 0.0)
        armature.pose.bones["foot_r"].rotation_euler = (math.radians(-3.0), 0.0, 0.0)
    elif pose_name == "walk_pass_left":
        armature.pose.bones["pelvis"].rotation_euler = (0.0, math.radians(1.0), 0.0)
        armature.pose.bones["thigh_l"].rotation_euler = (math.radians(-6.0), 0.0, math.radians(-1.0))
        armature.pose.bones["calf_l"].rotation_euler = (math.radians(22.0), 0.0, 0.0)
        armature.pose.bones["foot_l"].rotation_euler = (math.radians(-2.0), 0.0, 0.0)
        armature.pose.bones["thigh_r"].rotation_euler = (math.radians(8.0), 0.0, math.radians(1.0))
        armature.pose.bones["calf_r"].rotation_euler = (math.radians(8.0), 0.0, 0.0)
    elif pose_name == "walk_pass_right":
        armature.pose.bones["pelvis"].rotation_euler = (0.0, math.radians(-1.0), 0.0)
        armature.pose.bones["thigh_l"].rotation_euler = (math.radians(8.0), 0.0, math.radians(-1.0))
        armature.pose.bones["calf_l"].rotation_euler = (math.radians(8.0), 0.0, 0.0)
        armature.pose.bones["thigh_r"].rotation_euler = (math.radians(-6.0), 0.0, math.radians(1.0))
        armature.pose.bones["calf_r"].rotation_euler = (math.radians(22.0), 0.0, 0.0)
        armature.pose.bones["foot_r"].rotation_euler = (math.radians(-2.0), 0.0, 0.0)

    bpy.ops.object.mode_set(mode="OBJECT")
    bpy.context.view_layer.update()


def insert_pose_keys(armature, frame):
    bpy.ops.object.mode_set(mode="POSE")
    for pb in armature.pose.bones:
        pb.keyframe_insert("rotation_euler", frame=frame)
        pb.keyframe_insert("location", frame=frame)
    bpy.ops.object.mode_set(mode="OBJECT")


def build_action(armature, name, pose_frames):
    scene = bpy.context.scene
    action = bpy.data.actions.new(name)
    action.use_fake_user = True
    armature.animation_data_create()
    armature.animation_data.action = action

    for frame, pose_name in pose_frames:
        scene.frame_set(frame)
        set_pose(armature, pose_name)
        insert_pose_keys(armature, frame)
    return action


def keyframe_animation(armature):
    scene = bpy.context.scene
    scene.frame_start = 1
    scene.frame_end = 48
    idle = build_action(armature, "Mike_RigPrototype_IdleAction", [(1, "rest"), (24, "action"), (48, "rest")])
    walk = build_action(
        armature,
        "Mike_RigPrototype_WalkInPlace",
        [(1, "walk_left"), (9, "walk_pass_left"), (17, "walk_right"), (25, "walk_pass_right"), (33, "walk_left")],
    )
    armature.animation_data.action = walk
    scene.frame_set(1)
    set_pose(armature, "walk_left")
    return idle, walk


def export_outputs(armature):
    export_dir = OUT_ROOT / "Exports"
    export_dir.mkdir(parents=True, exist_ok=True)
    glb_path = export_dir / "Hero_4_Mike_TypeA_Standard_RiggedPrototype.glb"
    fbx_path = export_dir / "Hero_4_Mike_TypeA_Standard_RiggedPrototype.fbx"

    bpy.ops.object.select_all(action="DESELECT")
    armature.select_set(True)
    for obj in bpy.context.scene.objects:
        if obj.type == "MESH":
            obj.select_set(True)
    bpy.context.view_layer.objects.active = armature
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
    )
    return glb_path, fbx_path


def main():
    OUT_ROOT.mkdir(parents=True, exist_ok=True)
    SCENE_OUT.parent.mkdir(parents=True, exist_ok=True)
    reset_scene()
    body, head = import_source()
    armature, points = create_armature(body, head)
    weight_body(body)
    stabilized_components = stabilize_small_components(body, max_vertices=100)
    weight_head(head)
    attach_armature(body, armature)
    attach_armature(head, armature)
    add_lights()

    idle_action, walk_action = keyframe_animation(armature)
    render_outputs = {}
    # QA renders are explicit pose checks. Temporarily detach the active action
    # so frame evaluation cannot overwrite the pose label being rendered.
    armature.animation_data.action = None
    bpy.context.scene.frame_set(1)
    set_pose(armature, "rest")
    for view, yaw in {"front": 0.0, "right": 90.0, "left_oblique": -35.0, "right_oblique": 35.0}.items():
        render_outputs[f"rest_{view}"] = str(render_view(view, yaw, "rest"))
    bpy.context.scene.frame_set(24)
    set_pose(armature, "action")
    for view, yaw in {"front": 0.0, "right": 90.0, "left_oblique": -35.0, "right_oblique": 35.0}.items():
        render_outputs[f"action_{view}"] = str(render_view(view, yaw, "action"))
    bpy.context.scene.frame_set(36)
    set_pose(armature, "leg_test")
    for view, yaw in {"front": 0.0, "right": 90.0, "left_oblique": -35.0, "right_oblique": 35.0}.items():
        render_outputs[f"leg_test_{view}"] = str(render_view(view, yaw, "leg_test"))
    bpy.context.scene.frame_set(1)
    set_pose(armature, "walk_left")
    for view, yaw in {"front": 0.0, "right": 90.0, "left_oblique": -35.0, "right_oblique": 35.0}.items():
        render_outputs[f"walk_left_{view}"] = str(render_view(view, yaw, "walk_left"))
    bpy.context.scene.frame_set(17)
    set_pose(armature, "walk_right")
    for view, yaw in {"front": 0.0, "right": 90.0, "left_oblique": -35.0, "right_oblique": 35.0}.items():
        render_outputs[f"walk_right_{view}"] = str(render_view(view, yaw, "walk_right"))
    armature.animation_data.action = walk_action

    glb_path, fbx_path = export_outputs(armature)
    bpy.ops.wm.save_as_mainfile(filepath=str(SCENE_OUT))

    report = {
        "source": str(SOURCE_GLB),
        "blend": str(SCENE_OUT),
        "exports": {"glb": str(glb_path), "fbx": str(fbx_path)},
        "renders": render_outputs,
        "armature": armature.name,
        "bones": [bone.name for bone in armature.data.bones],
        "deform_bones": DEFORM_BONES,
        "mesh_vertex_groups": {
            obj.name: [group.name for group in obj.vertex_groups]
            for obj in bpy.context.scene.objects
            if obj.type == "MESH"
        },
        "body_small_components_stabilized": stabilized_components,
        "animation": {
            "active_action": armature.animation_data.action.name if armature.animation_data and armature.animation_data.action else None,
            "actions": [action.name for action in bpy.data.actions],
            "idle_action": idle_action.name,
            "walk_action": walk_action.name,
            "frame_start": bpy.context.scene.frame_start,
            "frame_end": bpy.context.scene.frame_end,
        },
    }
    report_path = OUT_ROOT / "rig_report.json"
    report_path.write_text(json.dumps(report, indent=2), encoding="utf-8")
    print(json.dumps({"report": str(report_path), "glb": str(glb_path), "fbx": str(fbx_path), "blend": str(SCENE_OUT)}, indent=2))


if __name__ == "__main__":
    main()
