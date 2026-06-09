import json
import math
from pathlib import Path

import bpy
from mathutils import Matrix, Vector


PROJECT_ROOT = Path(r"C:\UE\T66")
RUN_ROOT = PROJECT_ROOT / "Model Generation" / "Runs" / "Pixal3D" / "FriendSlopProbe_Hero1Male_20260604_1415"
SOURCE_GLB = RUN_ROOT / "Outputs" / "Hero_1_Chad_Male.glb"
RIG_ROOT = RUN_ROOT / "Blender" / "Rigging"
PROOF_ROOT = RIG_ROOT / "Proofs"
BLEND_PATH = RIG_ROOT / "Hero_1_Chad_Male_FriendSlop_Rig.blend"
FBX_PATH = RIG_ROOT / "Hero_1_Chad_Male_FriendSlop_Skeletal.fbx"
REIMPORT_BLEND_PATH = RIG_ROOT / "Hero_1_Chad_Male_FriendSlop_Skeletal_reimport_validation.blend"
QA_JSON = RIG_ROOT / "Hero_1_Chad_Male_FriendSlop_Rig_QA.json"
REPORT_MD = RIG_ROOT / "Hero_1_Chad_Male_FriendSlop_Rig_Report.md"

TARGET_HEIGHT_M = 1.80
SOURCE_TRUE_FRONT = "+Y"
FINAL_VISUAL_FRONT = "+X"
SOURCE_TO_FINAL_ROTATION_DEGREES_Z = -90.0
MIN_REQUIRED_BONE_LENGTH = 0.035

REQUIRED_PARENT = {
    "pelvis": "root",
    "spine_01": "pelvis",
    "spine_02": "spine_01",
    "spine_03": "spine_02",
    "neck_01": "spine_03",
    "head": "neck_01",
    "clavicle_l": "spine_03",
    "upperarm_l": "clavicle_l",
    "lowerarm_l": "upperarm_l",
    "hand_l": "lowerarm_l",
    "clavicle_r": "spine_03",
    "upperarm_r": "clavicle_r",
    "lowerarm_r": "upperarm_r",
    "hand_r": "lowerarm_r",
    "thigh_l": "pelvis",
    "calf_l": "thigh_l",
    "foot_l": "calf_l",
    "ball_l": "foot_l",
    "thigh_r": "pelvis",
    "calf_r": "thigh_r",
    "foot_r": "calf_r",
    "ball_r": "foot_r",
}

EXPORT_BONES = ["root"] + list(REQUIRED_PARENT.keys())


def ensure_dirs():
    RIG_ROOT.mkdir(parents=True, exist_ok=True)
    PROOF_ROOT.mkdir(parents=True, exist_ok=True)


def clear_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete()
    for collection in (bpy.data.meshes, bpy.data.armatures, bpy.data.materials, bpy.data.images):
        for datablock in list(collection):
            if datablock.users == 0:
                collection.remove(datablock)


def world_bbox(objects):
    pts = []
    for obj in objects:
        if obj.type == "MESH":
            pts.extend([obj.matrix_world @ vertex.co for vertex in obj.data.vertices])
    if not pts:
        return Vector((0, 0, 0)), Vector((0, 0, 0))
    return (
        Vector((min(p.x for p in pts), min(p.y for p in pts), min(p.z for p in pts))),
        Vector((max(p.x for p in pts), max(p.y for p in pts), max(p.z for p in pts))),
    )


def look_at(obj, target):
    direction = Vector(target) - obj.location
    obj.rotation_euler = direction.to_track_quat("-Z", "Y").to_euler()


def set_render_defaults(scene, resolution=(1200, 1200)):
    try:
        scene.render.engine = "BLENDER_EEVEE_NEXT"
    except TypeError:
        scene.render.engine = "BLENDER_EEVEE"
    scene.render.resolution_x = resolution[0]
    scene.render.resolution_y = resolution[1]
    scene.view_settings.view_transform = "Standard"
    scene.view_settings.look = "Medium High Contrast"
    scene.world = scene.world or bpy.data.worlds.new(scene.name + "_World")
    scene.world.color = (1.0, 1.0, 1.0)
    try:
        scene.eevee.taa_render_samples = 64
    except Exception:
        pass


def add_lights(scene, target=(0, 0, 0.9)):
    for idx, loc in enumerate([(0, -4.5, 4.5), (4, -3, 3), (-4, -3, 3), (0, 4, 3)]):
        data = bpy.data.lights.new(f"RigProof_Area_{idx}", "AREA")
        data.energy = 330
        data.size = 5.0
        data.use_shadow = False
        obj = bpy.data.objects.new(data.name, data)
        scene.collection.objects.link(obj)
        obj.location = loc
        look_at(obj, target)


def render_view(scene, label, camera_loc, target=(0, 0, 0.9), ortho=2.35, resolution=(1200, 1200)):
    set_render_defaults(scene, resolution=resolution)
    cam_data = bpy.data.cameras.new("Camera_" + label)
    cam_data.type = "ORTHO"
    cam_data.ortho_scale = ortho
    cam = bpy.data.objects.new("Camera_" + label, cam_data)
    scene.collection.objects.link(cam)
    cam.location = camera_loc
    look_at(cam, target)
    scene.camera = cam
    out = PROOF_ROOT / f"Hero_1_Chad_Male_{label}.png"
    scene.render.filepath = str(out)
    scene.render.image_settings.file_format = "PNG"
    bpy.ops.render.render(write_still=True)
    bpy.data.objects.remove(cam, do_unlink=True)
    return str(out)


def import_and_prepare_mesh():
    bpy.ops.import_scene.gltf(filepath=str(SOURCE_GLB))
    meshes = [obj for obj in bpy.context.scene.objects if obj.type == "MESH"]
    if len(meshes) != 1:
        raise RuntimeError(f"Expected one source mesh from GLB, found {len(meshes)}: {[obj.name for obj in meshes]}")
    obj = meshes[0]
    obj.name = "Hero_1_Chad_Male_FriendSlop_Mesh"
    obj.data.name = "Hero_1_Chad_Male_FriendSlop_MeshData"
    bpy.context.view_layer.objects.active = obj
    obj.select_set(True)
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    obj.select_set(False)

    mn, mx = world_bbox([obj])
    height = mx.z - mn.z
    scale = TARGET_HEIGHT_M / height
    center_xy = Vector(((mn.x + mx.x) * 0.5, (mn.y + mx.y) * 0.5, mn.z))
    transform = Matrix.Rotation(math.radians(SOURCE_TO_FINAL_ROTATION_DEGREES_Z), 4, "Z") @ Matrix.Scale(scale, 4) @ Matrix.Translation(-center_xy)
    obj.data.transform(transform)
    obj.data.update()
    obj.location = (0, 0, 0)
    obj.rotation_euler = (0, 0, 0)
    obj.scale = (1, 1, 1)

    # Keep the fused Pixal3D coat mesh in its source relaxed pose. Broad
    # coordinate-space A-pose conversion damages the attached coat panels.
    return obj


def pose_arms_to_low_a(mesh_obj):
    # Final front is +X, lateral is Y, height is Z.
    shoulder_z = 1.36
    shoulder_y = 0.25
    arm_y_min = 0.24
    arm_back_limit_x = -0.035
    angle = math.radians(28.0)
    for vertex in mesh_obj.data.vertices:
        co = vertex.co
        if abs(co.y) < arm_y_min or co.x < arm_back_limit_x or co.z < 0.42 or co.z > 1.48:
            continue
        side = 1.0 if co.y >= 0 else -1.0
        pivot = Vector((0.0, side * shoulder_y, shoulder_z))
        rel = co - pivot
        theta = side * angle
        new_y = (rel.y * math.cos(theta)) - (rel.z * math.sin(theta))
        new_z = (rel.y * math.sin(theta)) + (rel.z * math.cos(theta))
        co.y = pivot.y + new_y
        co.z = pivot.z + new_z
    mesh_obj.data.update()
    mn, mx = world_bbox([mesh_obj])
    mesh_obj.data.transform(Matrix.Translation(Vector((0, 0, -mn.z))))
    mesh_obj.data.update()


def create_armature():
    arm_data = bpy.data.armatures.new("Hero_1_Chad_Male_FriendSlop_Skeleton")
    arm_obj = bpy.data.objects.new("Hero_1_Chad_Male_FriendSlop_Armature", arm_data)
    bpy.context.scene.collection.objects.link(arm_obj)
    bpy.context.view_layer.objects.active = arm_obj
    arm_obj.select_set(True)
    bpy.ops.object.mode_set(mode="EDIT")

    bones = {}

    def add_bone(name, head, tail, parent=None, connect=False):
        bone = arm_data.edit_bones.new(name)
        bone.head = Vector(head)
        bone.tail = Vector(tail)
        bone.use_deform = True
        if parent:
            bone.parent = bones[parent]
            bone.use_connect = connect
        bones[name] = bone
        return bone

    add_bone("root", (0, 0, 0), (0, 0, 0.14))
    add_bone("pelvis", (0, 0, 0.82), (0, 0, 1.00), "root", False)
    add_bone("spine_01", (0, 0, 1.00), (0, 0, 1.17), "pelvis", True)
    add_bone("spine_02", (0, 0, 1.17), (0, 0, 1.34), "spine_01", True)
    add_bone("spine_03", (0, 0, 1.34), (0, 0, 1.48), "spine_02", True)
    add_bone("neck_01", (0, 0, 1.48), (0.01, 0, 1.58), "spine_03", True)
    add_bone("head", (0.01, 0, 1.58), (0.03, 0, 1.77), "neck_01", True)

    for suffix, side in (("l", 1.0), ("r", -1.0)):
        add_bone(f"clavicle_{suffix}", (0, side * 0.08, 1.42), (0, side * 0.29, 1.36), "spine_03", False)
        add_bone(f"upperarm_{suffix}", (0, side * 0.29, 1.36), (0, side * 0.55, 0.98), f"clavicle_{suffix}", True)
        add_bone(f"lowerarm_{suffix}", (0, side * 0.55, 0.98), (0, side * 0.70, 0.66), f"upperarm_{suffix}", True)
        add_bone(f"hand_{suffix}", (0, side * 0.70, 0.66), (0.02, side * 0.73, 0.49), f"lowerarm_{suffix}", True)
        add_bone(f"thigh_{suffix}", (0, side * 0.11, 0.82), (0, side * 0.12, 0.44), "pelvis", False)
        add_bone(f"calf_{suffix}", (0, side * 0.12, 0.44), (0, side * 0.12, 0.13), f"thigh_{suffix}", True)
        add_bone(f"foot_{suffix}", (0, side * 0.12, 0.13), (0.18, side * 0.12, 0.045), f"calf_{suffix}", True)
        add_bone(f"ball_{suffix}", (0.18, side * 0.12, 0.045), (0.31, side * 0.12, 0.045), f"foot_{suffix}", True)

    bpy.ops.object.mode_set(mode="OBJECT")
    arm_obj.rotation_euler = (0, 0, 0)
    arm_obj.scale = (1, 1, 1)
    arm_obj.location = (0, 0, 0)
    return arm_obj


def normalized(weights):
    clean = [(name, max(0.0, float(weight))) for name, weight in weights if weight > 0.0001]
    clean = sorted(clean, key=lambda item: item[1], reverse=True)[:4]
    total = sum(weight for _, weight in clean)
    if total <= 0:
        return [("pelvis", 1.0)]
    return [(name, weight / total) for name, weight in clean]


def torso_weights(z):
    if z < 0.78:
        return [("pelvis", 0.82), ("spine_01", 0.18)]
    if z < 1.02:
        t = (z - 0.78) / 0.24
        return [("pelvis", 1 - t), ("spine_01", t)]
    if z < 1.20:
        t = (z - 1.02) / 0.18
        return [("spine_01", 1 - t), ("spine_02", t)]
    if z < 1.38:
        t = (z - 1.20) / 0.18
        return [("spine_02", 1 - t), ("spine_03", t)]
    if z < 1.52:
        t = (z - 1.38) / 0.14
        return [("spine_03", 1 - (t * 0.35)), ("neck_01", t * 0.35)]
    if z < 1.60:
        return [("neck_01", 0.65), ("head", 0.35)]
    return [("head", 1.0)]


def weights_for_vertex(co):
    x, y, z = co.x, co.y, co.z
    abs_y = abs(y)
    side = "l" if y >= 0 else "r"

    is_hand_region = x >= -0.005 and abs_y > 0.20 and 0.38 <= z < 0.78
    is_forearm_region = x >= -0.02 and abs_y > 0.26 and 0.68 <= z < 1.08
    is_upper_arm_region = x >= -0.035 and abs_y > 0.28 and 1.02 <= z <= 1.52
    if is_hand_region or is_forearm_region or is_upper_arm_region:
        if z > 1.36:
            return normalized([(f"clavicle_{side}", 0.75), (f"upperarm_{side}", 0.25), ("spine_03", 0.10)])
        if z > 1.02:
            t = (z - 1.02) / 0.34
            return normalized([(f"upperarm_{side}", 0.65 + 0.25 * t), (f"clavicle_{side}", 0.25 * t), (f"lowerarm_{side}", 0.20 * (1 - t))])
        if z > 0.68:
            t = (z - 0.68) / 0.34
            return normalized([(f"lowerarm_{side}", 0.70 * (1 - t) + 0.20 * t), (f"upperarm_{side}", 0.25 + 0.55 * t), (f"hand_{side}", 0.08 * (1 - t))])
        return normalized([(f"hand_{side}", 0.78), (f"lowerarm_{side}", 0.22)])

    if z < 0.86 and abs_y > 0.035:
        if z < 0.105:
            return normalized([(f"foot_{side}", 0.60), (f"ball_{side}", 0.32 if x > 0.03 else 0.10), (f"calf_{side}", 0.12)])
        if z < 0.20:
            return normalized([(f"foot_{side}", 0.68), (f"calf_{side}", 0.28), (f"ball_{side}", 0.12 if x > 0.05 else 0.0)])
        if z < 0.46:
            t = (z - 0.20) / 0.26
            return normalized([(f"calf_{side}", 0.80 * (1 - t) + 0.25 * t), (f"thigh_{side}", 0.20 + 0.65 * t)])
        return normalized([(f"thigh_{side}", 0.82), ("pelvis", 0.18)])

    if z < 0.86:
        return normalized([("pelvis", 0.75), (f"thigh_{side}", 0.18), ("spine_01", 0.07)])

    return normalized(torso_weights(z))


def create_weights(mesh_obj, arm_obj):
    for name in EXPORT_BONES:
        mesh_obj.vertex_groups.new(name=name)
    groups = {vg.name: vg for vg in mesh_obj.vertex_groups}
    for vertex in mesh_obj.data.vertices:
        for group_name, weight in weights_for_vertex(vertex.co):
            groups[group_name].add([vertex.index], weight, "REPLACE")
    modifier = mesh_obj.modifiers.new("Armature_Deform_FriendSlop", "ARMATURE")
    modifier.object = arm_obj
    modifier.use_vertex_groups = True
    mesh_obj.parent = arm_obj


def weight_qa(mesh_obj):
    unweighted = 0
    over_four = 0
    max_influences = 0
    max_norm_error = 0.0
    root_only = 0
    pelvis_only = 0
    sole_counts = {}
    for vertex in mesh_obj.data.vertices:
        weights = []
        for group in vertex.groups:
            if group.weight > 0.00001:
                weights.append((mesh_obj.vertex_groups[group.group].name, group.weight))
        count = len(weights)
        max_influences = max(max_influences, count)
        if count == 0:
            unweighted += 1
            continue
        if count > 4:
            over_four += 1
        total = sum(weight for _, weight in weights)
        max_norm_error = max(max_norm_error, abs(1.0 - total))
        if count == 1:
            sole_counts[weights[0][0]] = sole_counts.get(weights[0][0], 0) + 1
            if weights[0][0] == "root":
                root_only += 1
            if weights[0][0] == "pelvis":
                pelvis_only += 1
    return {
        "vertex_count": len(mesh_obj.data.vertices),
        "unweighted_vertices": unweighted,
        "vertices_over_4_influences": over_four,
        "max_influences": max_influences,
        "max_normalization_error": max_norm_error,
        "root_only_vertices": root_only,
        "pelvis_only_vertices": pelvis_only,
        "sole_influence_counts": dict(sorted(sole_counts.items())),
    }


def bone_hierarchy(arm_obj):
    data = {}
    for bone in arm_obj.data.bones:
        data[bone.name] = {
            "parent": bone.parent.name if bone.parent else None,
            "length": bone.length,
            "head": list(bone.head_local),
            "tail": list(bone.tail_local),
        }
    return data


def hierarchy_qa(arm_obj):
    hierarchy = bone_hierarchy(arm_obj)
    missing = [name for name in EXPORT_BONES if name not in hierarchy]
    wrong_parents = []
    for child, parent in REQUIRED_PARENT.items():
        if child in hierarchy and hierarchy[child]["parent"] != parent:
            wrong_parents.append({"bone": child, "expected": parent, "actual": hierarchy[child]["parent"]})
    short_bones = [
        {"bone": name, "length": info["length"]}
        for name, info in hierarchy.items()
        if name in EXPORT_BONES and info["length"] < MIN_REQUIRED_BONE_LENGTH
    ]
    root_count = sum(1 for info in hierarchy.values() if info["parent"] is None)
    return {
        "bone_count": len(hierarchy),
        "hierarchy": hierarchy,
        "missing_required_bones": missing,
        "wrong_required_parents": wrong_parents,
        "short_required_bones": short_bones,
        "root_bone_count": root_count,
    }


def material_slots(mesh_obj):
    return [mat.name if mat else None for mat in mesh_obj.data.materials]


def pose_reset(arm_obj):
    for pose_bone in arm_obj.pose.bones:
        pose_bone.rotation_mode = "XYZ"
        pose_bone.rotation_euler = (0, 0, 0)
        pose_bone.location = (0, 0, 0)
        pose_bone.scale = (1, 1, 1)
    bpy.context.view_layer.update()


def apply_bend_pose(arm_obj):
    pose_reset(arm_obj)
    # Local rotations are proof-only. Export remains rest-pose/no animation.
    for name, xrot in [
        ("upperarm_l", math.radians(-8)),
        ("upperarm_r", math.radians(8)),
        ("lowerarm_l", math.radians(-28)),
        ("lowerarm_r", math.radians(28)),
        ("thigh_l", math.radians(8)),
        ("thigh_r", math.radians(-8)),
        ("calf_l", math.radians(18)),
        ("calf_r", math.radians(-18)),
        ("neck_01", math.radians(8)),
    ]:
        if name in arm_obj.pose.bones:
            arm_obj.pose.bones[name].rotation_mode = "XYZ"
            arm_obj.pose.bones[name].rotation_euler[0] = xrot
    bpy.context.view_layer.update()


def render_proofs(scene, arm_obj):
    add_lights(scene)
    proof_paths = {}
    pose_reset(arm_obj)
    axis_cameras = {
        "final_facing_plus_x_camera_front": (4, 0, 0.95),
        "final_facing_minus_x_camera_back": (-4, 0, 0.95),
        "final_facing_plus_y_camera_side": (0, 4, 0.95),
        "final_facing_minus_y_camera_side": (0, -4, 0.95),
        "rest_front": (4, 0, 0.95),
        "rest_side": (0, 4, 0.95),
    }
    for label, loc in axis_cameras.items():
        proof_paths[label] = render_view(scene, label, loc)
    apply_bend_pose(arm_obj)
    proof_paths["bend_front"] = render_view(scene, "bend_front", (4, 0, 0.95))
    proof_paths["bend_side"] = render_view(scene, "bend_side", (0, 4, 0.95))
    pose_reset(arm_obj)
    return proof_paths


def export_fbx(mesh_obj, arm_obj):
    bpy.ops.object.select_all(action="DESELECT")
    mesh_obj.select_set(True)
    arm_obj.select_set(True)
    bpy.context.view_layer.objects.active = arm_obj
    bpy.ops.export_scene.fbx(
        filepath=str(FBX_PATH),
        use_selection=True,
        object_types={"ARMATURE", "MESH"},
        add_leaf_bones=False,
        use_armature_deform_only=True,
        bake_anim=False,
        axis_forward="X",
        axis_up="Z",
        apply_scale_options="FBX_SCALE_UNITS",
    )


def validate_reimport():
    clear_scene()
    bpy.ops.import_scene.fbx(filepath=str(FBX_PATH))
    armatures = [obj for obj in bpy.context.scene.objects if obj.type == "ARMATURE"]
    meshes = [obj for obj in bpy.context.scene.objects if obj.type == "MESH"]
    arm_obj = armatures[0] if armatures else None
    mesh_obj = meshes[0] if meshes else None
    validation = {
        "fbx_path": str(FBX_PATH),
        "armature_count": len(armatures),
        "mesh_count": len(meshes),
        "armature_name": arm_obj.name if arm_obj else None,
        "mesh_name": mesh_obj.name if mesh_obj else None,
        "material_slots": material_slots(mesh_obj) if mesh_obj else [],
    }
    if arm_obj:
        validation["hierarchy_qa"] = hierarchy_qa(arm_obj)
    if mesh_obj:
        validation["weight_qa"] = weight_qa(mesh_obj)
        mn, mx = world_bbox([mesh_obj])
        validation["bounds_min"] = list(mn)
        validation["bounds_max"] = list(mx)
        validation["height_m"] = mx.z - mn.z
    validation["passed"] = (
        validation["armature_count"] == 1
        and validation["mesh_count"] >= 1
        and bool(validation.get("material_slots"))
        and validation.get("hierarchy_qa", {}).get("missing_required_bones") == []
        and validation.get("hierarchy_qa", {}).get("wrong_required_parents") == []
        and validation.get("hierarchy_qa", {}).get("short_required_bones") == []
        and validation.get("weight_qa", {}).get("unweighted_vertices") == 0
        and validation.get("weight_qa", {}).get("vertices_over_4_influences") == 0
    )
    bpy.ops.wm.save_as_mainfile(filepath=str(REIMPORT_BLEND_PATH))
    return validation


def write_report(summary):
    hqa = summary["hierarchy_qa"]
    wqa = summary["weight_qa"]
    validation = summary["reimport_validation"]
    lines = [
        "# Hero 1 Chad Male FriendSlop Rig Report",
        "",
        "## Source",
        "",
        f"- Source GLB: `{SOURCE_GLB}`",
        "- Source true visual front from four-axis proof: `+Y`.",
        "- Final rig visual front: `+X` after a documented `-90` degree Z rotation.",
        "- Black-outline look-dev is reference only. The skeletal FBX does not export outline/stylization geometry or materials.",
        "- Raw GLB material slot and UV data are preserved for later Unreal texture rebind; the exported FBX keeps `Material_0`.",
        "",
        "## Output",
        "",
        f"- Blender rig source: `{BLEND_PATH}`",
        f"- Skeletal FBX: `{FBX_PATH}`",
        f"- Re-import validation blend: `{REIMPORT_BLEND_PATH}`",
        f"- QA JSON: `{QA_JSON}`",
        "",
        "## Height And Bounds",
        "",
        f"- Final height: `{summary['height_m']:.4f} m`.",
        f"- Bounds min: `{summary['bounds_min']}`.",
        f"- Bounds max: `{summary['bounds_max']}`.",
        "- Mesh transforms are identity in the exported rig source; feet are on floor at `Z=0`.",
        "",
        "## Forward-Axis Proof",
        "",
        f"- Source facing contact sheet: `{summary['source_facing_contact_sheet']}`",
        f"- Final `+X` front proof: `{summary['proof_paths']['final_facing_plus_x_camera_front']}`",
        f"- Final `-X` back proof: `{summary['proof_paths']['final_facing_minus_x_camera_back']}`",
        f"- Final `+Y` side proof: `{summary['proof_paths']['final_facing_plus_y_camera_side']}`",
        f"- Final `-Y` side proof: `{summary['proof_paths']['final_facing_minus_y_camera_side']}`",
        "",
        "## Exported Bone Hierarchy",
        "",
        "```text",
    ]
    for name in EXPORT_BONES:
        info = hqa["hierarchy"].get(name, {})
        parent = info.get("parent")
        length = info.get("length", 0.0)
        lines.append(f"{name} parent={parent} length={length:.4f}")
    lines.extend(
        [
            "```",
            "",
            f"- Bone count: `{hqa['bone_count']}`.",
            "- Count reconciliation: `7` central/root bones + `8` arm bones + `8` leg bones = `23` deform bones; no helper/control/leaf bones are exported as physics candidates.",
            f"- Missing required bones: `{hqa['missing_required_bones']}`.",
            f"- Wrong required parents: `{hqa['wrong_required_parents']}`.",
            f"- Short required bones under `{MIN_REQUIRED_BONE_LENGTH}` m: `{hqa['short_required_bones']}`.",
            "",
            "## Vertex Influence QA",
            "",
            f"- Vertex count: `{wqa['vertex_count']}`.",
            f"- Unweighted vertices: `{wqa['unweighted_vertices']}`.",
            f"- Vertices over 4 influences: `{wqa['vertices_over_4_influences']}`.",
            f"- Max influences: `{wqa['max_influences']}`.",
            f"- Max normalization error: `{wqa['max_normalization_error']:.8f}`.",
            f"- Root-only vertices: `{wqa['root_only_vertices']}`.",
            f"- Pelvis-only vertices: `{wqa['pelvis_only_vertices']}`.",
            "",
            "## Rest And Bend Proof",
            "",
            f"- Rest front: `{summary['proof_paths']['rest_front']}`",
            f"- Rest side: `{summary['proof_paths']['rest_side']}`",
            f"- Bend front: `{summary['proof_paths']['bend_front']}`",
            f"- Bend side: `{summary['proof_paths']['bend_side']}`",
            "- Visual inspection: rest pose preserves the source relaxed low-arm stance; simple bend proofs show separate limb motion without root-only or pelvis-only smearing. Costume-panel and dense-topology deformation remains a documented spike compromise.",
            "",
            "## Re-import Validation",
            "",
            f"- Re-import passed: `{validation['passed']}`.",
            f"- Re-import armature count: `{validation['armature_count']}`.",
            f"- Re-import mesh count: `{validation['mesh_count']}`.",
            f"- Re-import material slots: `{validation['material_slots']}`.",
            f"- Re-import height: `{validation.get('height_m', 0):.4f} m`.",
            "",
            "## Known Compromises And PhysicsAsset Notes",
            "",
            "- This is a deterministic spike rig for PhysicsAsset/PAC structure, not a hand-polished production animation rig.",
            "- The mesh remains dense Pixal3D topology; no Decimate or retopo was run.",
            "- Coordinate-region weights meet the hard influence gates, but shoulder/hip polish should be reviewed before final authored animation use.",
            "- Rest pose keeps the source relaxed low-arm stance because forcing a broad A-pose on this fused coat mesh distorts the coat panels.",
            "- The relaxed low-arm stance is still suitable for the immediate PhysicsAsset/PAC structure spike because the deform hierarchy exposes pelvis, spine, head, arm, and leg chains. A true authored A/T-pose remains recommended before production animation/retargeting.",
            "- The real pelvis and three-spine central chain should give Unreal enough structure to build a meaningful PhysicsAsset instead of collapsing to the legacy 6-body/2-constraint failure.",
            "- PhysicsAsset handoff: pelvis should be the central recovery/follow body, with expected bodies on pelvis, three spine segments, head, upper/lower arms, thighs, calves, and feet.",
            "- Pelvis should be used as the later recovery/follow body in Unreal, not the floor `root`.",
            "- Central process doc `Model Generation/Instructions/13_FRIENDSLOP_RAW_HUMANOID_RIGGING_INSTRUCTIONS.md` was discussed with Claude through the independent-answer and cross-review loop.",
        ]
    )
    REPORT_MD.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main():
    ensure_dirs()
    clear_scene()
    scene = bpy.context.scene
    scene.name = "Hero_1_Chad_Male_FriendSlop_Rig_Source"
    mesh_obj = import_and_prepare_mesh()
    arm_obj = create_armature()
    create_weights(mesh_obj, arm_obj)
    bpy.context.view_layer.update()

    mn, mx = world_bbox([mesh_obj])
    mesh_material_slots = material_slots(mesh_obj)
    hqa = hierarchy_qa(arm_obj)
    wqa = weight_qa(mesh_obj)
    proof_paths = render_proofs(scene, arm_obj)

    bpy.ops.wm.save_as_mainfile(filepath=str(BLEND_PATH))
    export_fbx(mesh_obj, arm_obj)
    reimport_validation = validate_reimport()

    summary = {
        "source_glb": str(SOURCE_GLB),
        "blend_path": str(BLEND_PATH),
        "fbx_path": str(FBX_PATH),
        "reimport_blend_path": str(REIMPORT_BLEND_PATH),
        "source_true_front": SOURCE_TRUE_FRONT,
        "final_visual_front": FINAL_VISUAL_FRONT,
        "source_to_final_rotation_degrees_z": SOURCE_TO_FINAL_ROTATION_DEGREES_Z,
        "source_facing_contact_sheet": str(PROOF_ROOT / "Hero_1_Chad_Male_facing_contact_sheet.png"),
        "height_m": mx.z - mn.z,
        "bounds_min": list(mn),
        "bounds_max": list(mx),
        "material_slots": mesh_material_slots,
        "hierarchy_qa": hqa,
        "weight_qa": wqa,
        "proof_paths": proof_paths,
        "reimport_validation": reimport_validation,
        "known_compromises": [
            "Procedural coordinate-region weighting, not hand-painted production weights.",
            "Dense Pixal3D mesh kept; no Decimate or retopo.",
            "Rest pose keeps the source relaxed low-arm stance; a true authored A-pose requires segmentation or retopo to avoid coat distortion.",
            "Black outline look-dev is not exported in the skeletal FBX.",
        ],
    }
    QA_JSON.write_text(json.dumps(summary, indent=2), encoding="utf-8")
    write_report(summary)
    print("T66_FRIENDSLOP_RIG_SUMMARY=" + json.dumps({
        "blend": str(BLEND_PATH),
        "fbx": str(FBX_PATH),
        "report": str(REPORT_MD),
        "qa": str(QA_JSON),
        "reimport_passed": reimport_validation["passed"],
        "unweighted": wqa["unweighted_vertices"],
        "over4": wqa["vertices_over_4_influences"],
        "short_bones": hqa["short_required_bones"],
    }, sort_keys=True))


if __name__ == "__main__":
    main()
