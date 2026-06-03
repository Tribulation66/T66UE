"""
Author a BoneWalker rigid-segment biped preview under the 2026-05-25 QA gates.

This is a candidate authoring pass only. Do not treat the output as accepted
until the QA packet receives a separate Claude greenlight.
"""

from __future__ import annotations

import argparse
import importlib.util
import json
import math
import sys
from pathlib import Path

import bpy
from mathutils import Vector


PROJECT_ROOT = Path(__file__).resolve().parents[3]
TOOLS_ROOT = PROJECT_ROOT / "Model Generation" / "Rigging and Animation" / "Tools"
VAT_TOOL_PATH = TOOLS_ROOT / "create_easy_mob_vat_sources.py"
DEFAULT_RUN_ROOT = (
    PROJECT_ROOT
    / "Model Generation"
    / "Rigging and Animation"
    / "Runs"
    / "BoneWalker_RigidBipedMove_20260526"
)


def load_vat_tool():
    spec = importlib.util.spec_from_file_location("easy_mob_vat_sources", VAT_TOOL_PATH)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"Could not load VAT source tool: {VAT_TOOL_PATH}")
    module = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = module
    spec.loader.exec_module(module)
    return module


def look_at(obj: bpy.types.Object, target: Vector) -> None:
    direction = target - obj.location
    obj.rotation_euler = direction.to_track_quat("-Z", "Y").to_euler()


def bounds_for(obj: bpy.types.Object) -> tuple[Vector, Vector, Vector]:
    coords = [obj.matrix_world @ Vector(corner) for corner in obj.bound_box]
    min_v = Vector((min(c.x for c in coords), min(c.y for c in coords), min(c.z for c in coords)))
    max_v = Vector((max(c.x for c in coords), max(c.y for c in coords), max(c.z for c in coords)))
    return min_v, max_v, (min_v + max_v) * 0.5


def find_base_color_image(material: bpy.types.Material) -> bpy.types.Image | None:
    if not material or not material.use_nodes or not material.node_tree:
        return None
    for node in material.node_tree.nodes:
        if node.type != "BSDF_PRINCIPLED":
            continue
        base_input = node.inputs.get("Base Color")
        if not base_input:
            continue
        for link in base_input.links:
            if link.from_node.type == "TEX_IMAGE" and getattr(link.from_node, "image", None):
                return link.from_node.image
    for node in material.node_tree.nodes:
        if node.type == "TEX_IMAGE" and getattr(node, "image", None):
            return node.image
    return None


def convert_material_to_unlit_emissive(material: bpy.types.Material) -> None:
    image = find_base_color_image(material)
    fallback_color = tuple(material.diffuse_color) if material else (1.0, 1.0, 1.0, 1.0)
    material.use_nodes = True
    tree = material.node_tree
    tree.nodes.clear()

    output = tree.nodes.new(type="ShaderNodeOutputMaterial")
    output.location = (360, 0)
    emission = tree.nodes.new(type="ShaderNodeEmission")
    emission.location = (110, 0)
    emission.inputs["Strength"].default_value = 1.0

    if image:
        texture = tree.nodes.new(type="ShaderNodeTexImage")
        texture.location = (-160, 0)
        texture.image = image
        tree.links.new(texture.outputs["Color"], emission.inputs["Color"])
    else:
        emission.inputs["Color"].default_value = fallback_color

    tree.links.new(emission.outputs["Emission"], output.inputs["Surface"])


def apply_unlit_preview_materials(mesh: bpy.types.Object) -> None:
    for material in mesh.data.materials:
        if material:
            convert_material_to_unlit_emissive(material)


def create_rigid_biped_armature(mesh: bpy.types.Object) -> bpy.types.Object:
    min_v, max_v, center = bounds_for(mesh)
    height = max_v.z - min_v.z
    sx = max_v.x - min_v.x

    arm_data = bpy.data.armatures.new("ARM_BoneWalker_RigidBiped")
    arm = bpy.data.objects.new("ARM_BoneWalker_RigidBiped", arm_data)
    bpy.context.collection.objects.link(arm)
    arm.show_in_front = True

    x_left = center.x - sx * 0.20
    x_right = center.x + sx * 0.20
    z_foot = min_v.z + height * 0.03
    z_ankle = min_v.z + height * 0.13
    z_knee = min_v.z + height * 0.34
    z_hip = min_v.z + height * 0.54
    z_chest = min_v.z + height * 0.76
    z_head = min_v.z + height * 0.95

    bpy.context.view_layer.objects.active = arm
    arm.select_set(True)
    bpy.ops.object.mode_set(mode="EDIT")

    bones = arm_data.edit_bones
    root = bones.new("root")
    root.head = Vector((center.x, center.y, min_v.z))
    root.tail = Vector((center.x, center.y, min_v.z + height * 0.12))
    root.use_deform = False

    def add(name: str, head: Vector, tail: Vector, parent: str = "root") -> None:
        bone = bones.new(name)
        bone.head = head
        bone.tail = tail
        bone.parent = bones[parent]
        bone.use_deform = True

    add("pelvis", Vector((center.x, center.y, z_hip - height * 0.08)), Vector((center.x, center.y, z_hip + height * 0.05)))
    add("spine", Vector((center.x, center.y, z_hip + height * 0.02)), Vector((center.x, center.y, z_chest)), "pelvis")
    add("head", Vector((center.x, center.y, z_chest)), Vector((center.x, center.y, z_head)), "spine")
    add("left_thigh", Vector((x_left, center.y, z_hip)), Vector((x_left, center.y, z_knee)), "pelvis")
    add("left_shin", Vector((x_left, center.y, z_knee)), Vector((x_left, center.y, z_ankle)), "left_thigh")
    add("left_foot", Vector((x_left, center.y, z_ankle)), Vector((x_left, center.y + height * 0.10, z_foot)), "left_shin")
    add("right_thigh", Vector((x_right, center.y, z_hip)), Vector((x_right, center.y, z_knee)), "pelvis")
    add("right_shin", Vector((x_right, center.y, z_knee)), Vector((x_right, center.y, z_ankle)), "right_thigh")
    add("right_foot", Vector((x_right, center.y, z_ankle)), Vector((x_right, center.y + height * 0.10, z_foot)), "right_shin")
    add("left_arm", Vector((center.x - sx * 0.28, center.y, z_chest)), Vector((center.x - sx * 0.42, center.y, z_hip)), "spine")
    add("right_arm", Vector((center.x + sx * 0.28, center.y, z_chest)), Vector((center.x + sx * 0.42, center.y, z_hip)), "spine")

    bpy.ops.object.mode_set(mode="OBJECT")
    return arm


def assign_rigid_weights(mesh: bpy.types.Object, arm: bpy.types.Object) -> dict[str, int]:
    min_v, max_v, center = bounds_for(mesh)
    height = max_v.z - min_v.z
    counts: dict[str, int] = {}

    for group_name in arm.data.bones.keys():
        if group_name != "root":
            mesh.vertex_groups.new(name=group_name)

    def pick_group(co: Vector) -> str:
        nz = (co.z - min_v.z) / max(height, 0.001)
        if nz > 0.78:
            return "head"
        if abs(co.x - center.x) > (max_v.x - min_v.x) * 0.31 and 0.28 < nz < 0.78:
            return "left_arm" if co.x < center.x else "right_arm"
        if nz > 0.52:
            return "spine"
        if nz > 0.42:
            return "pelvis"
        if co.x < center.x:
            if nz > 0.26:
                return "left_thigh"
            if nz > 0.09:
                return "left_shin"
            return "left_foot"
        if nz > 0.26:
            return "right_thigh"
        if nz > 0.09:
            return "right_shin"
        return "right_foot"

    for vertex in mesh.data.vertices:
        co = mesh.matrix_world @ vertex.co
        group_name = pick_group(co)
        mesh.vertex_groups[group_name].add([vertex.index], 1.0, "REPLACE")
        counts[group_name] = counts.get(group_name, 0) + 1

    mesh.parent = arm
    modifier = mesh.modifiers.new("RigidBiped_Armature", "ARMATURE")
    modifier.object = arm
    modifier.use_vertex_groups = True
    return counts


def key_pose(arm: bpy.types.Object, frame: int, values: dict[str, tuple[float, float, float]], root_z: float = 0.0) -> None:
    bpy.context.scene.frame_set(frame)
    arm.location.z = root_z
    arm.keyframe_insert("location", frame=frame)
    for name, rot in values.items():
        bone = arm.pose.bones.get(name)
        if not bone:
            continue
        bone.rotation_mode = "XYZ"
        bone.rotation_euler = rot
        bone.keyframe_insert("rotation_euler", frame=frame)


def set_step_keys(action: bpy.types.Action) -> None:
    for fcurve in getattr(action, "fcurves", []) or []:
        for key in fcurve.keyframe_points:
            key.interpolation = "CONSTANT"
        if not any(modifier.type == "CYCLES" for modifier in fcurve.modifiers):
            fcurve.modifiers.new(type="CYCLES")


def animate_walk(arm: bpy.types.Object, frames: int) -> None:
    base = {
        "pelvis": (0.0, 0.0, 0.0),
        "spine": (0.0, 0.0, 0.0),
        "head": (0.0, 0.0, 0.0),
        "left_arm": (0.0, 0.0, 0.0),
        "right_arm": (0.0, 0.0, 0.0),
        "left_thigh": (0.0, 0.0, 0.0),
        "left_shin": (0.0, 0.0, 0.0),
        "left_foot": (0.0, 0.0, 0.0),
        "right_thigh": (0.0, 0.0, 0.0),
        "right_shin": (0.0, 0.0, 0.0),
        "right_foot": (0.0, 0.0, 0.0),
    }
    cycle = [
        (0.000, 0.00, 0.06),
        (0.006, 0.42, -0.16),
        (0.010, 0.22, -0.08),
        (0.000, -0.08, 0.06),
        (0.006, -0.16, 0.42),
        (0.010, -0.08, 0.22),
        (0.000, 0.06, -0.08),
    ]
    step = 4
    frame = 1
    pose_index = 0
    while frame <= frames:
        root_z, left, right = cycle[pose_index % len(cycle)]
        values = dict(base)
        values.update(
            {
                "pelvis": (0.02 * math.sin(frame), 0.0, 0.035 if left > right else -0.035),
                "spine": (-0.025, 0.0, -0.025 if left > right else 0.025),
                "head": (0.012, 0.0, 0.0),
                "left_thigh": (left, 0.0, 0.0),
                "left_shin": (-0.32 if left > 0.2 else 0.04, 0.0, 0.0),
                "left_foot": (0.12 if left > 0.2 else -0.045, 0.0, 0.0),
                "right_thigh": (right, 0.0, 0.0),
                "right_shin": (-0.32 if right > 0.2 else 0.04, 0.0, 0.0),
                "right_foot": (0.12 if right > 0.2 else -0.045, 0.0, 0.0),
                "left_arm": (-right * 0.24, 0.0, -0.05),
                "right_arm": (-left * 0.24, 0.0, 0.05),
            }
        )
        key_pose(arm, frame, values, root_z=root_z)
        pose_index += 1
        frame += step
    if frame - step != frames:
        root_z, left, right = cycle[pose_index % len(cycle)]
        values = dict(base)
        values.update(
            {
                "left_thigh": (left, 0.0, 0.0),
                "left_shin": (-0.32 if left > 0.2 else 0.04, 0.0, 0.0),
                "left_foot": (0.12 if left > 0.2 else -0.045, 0.0, 0.0),
                "right_thigh": (right, 0.0, 0.0),
                "right_shin": (-0.32 if right > 0.2 else 0.04, 0.0, 0.0),
                "right_foot": (0.12 if right > 0.2 else -0.045, 0.0, 0.0),
            }
        )
        key_pose(arm, frames, values, root_z=root_z)


def setup_scene(width: int, height: int, fps: int) -> bpy.types.Object:
    scene = bpy.context.scene
    scene.render.engine = "BLENDER_EEVEE"
    scene.eevee.taa_render_samples = 48
    scene.render.resolution_x = width
    scene.render.resolution_y = height
    scene.render.fps = fps
    scene.view_settings.view_transform = "Standard"
    scene.view_settings.look = "None"
    scene.world = bpy.data.worlds.new("BoneWalker_RigidBiped_World") if not scene.world else scene.world
    scene.world.color = (0.54, 0.56, 0.58)
    camera_data = bpy.data.cameras.new("BoneWalker_RigidBiped_Camera")
    camera = bpy.data.objects.new("BoneWalker_RigidBiped_Camera", camera_data)
    bpy.context.collection.objects.link(camera)
    scene.camera = camera
    return camera


def configure_video_output(path: Path) -> None:
    scene = bpy.context.scene
    image_settings = scene.render.image_settings
    if "media_type" in image_settings.bl_rna.properties:
        image_settings.media_type = "VIDEO"
    image_settings.file_format = "FFMPEG"
    scene.render.ffmpeg.format = "MPEG4"
    scene.render.ffmpeg.codec = "H264"
    scene.render.ffmpeg.constant_rate_factor = "MEDIUM"
    scene.render.ffmpeg.ffmpeg_preset = "GOOD"
    scene.render.filepath = str(path)


def render_video(enemy_id: str, out_root: Path, view: str, frames: int, width: int, height: int, fps: int) -> dict:
    vat = load_vat_tool()
    spec = next((item for item in vat.MOBS if item.enemy_id.lower() == enemy_id.lower()), None)
    if spec is None:
        raise ValueError(enemy_id)

    out_root.mkdir(parents=True, exist_ok=True)
    vat.reset_scene()
    camera = setup_scene(width, height, fps)
    mesh = vat.import_glb(spec.source_glb)
    mesh.name = "BoneWalker_RigidBiped_Mesh"
    apply_unlit_preview_materials(mesh)
    arm = create_rigid_biped_armature(mesh)
    weight_counts = assign_rigid_weights(mesh, arm)
    animate_walk(arm, frames=frames)

    min_v, max_v, center = bounds_for(mesh)
    radius = max(max_v.x - min_v.x, max_v.y - min_v.y, max_v.z - min_v.z, 0.5)
    focus_z = center.z

    mover = bpy.data.objects.new("BoneWalker_RigidBiped_Mover", None)
    bpy.context.collection.objects.link(mover)
    arm.parent = mover
    start_y = -radius * 2.10 if view == "front" else 0.0
    end_y = radius * 0.35 if view == "front" else 0.0
    for frame in range(1, frames + 1):
        progress = (frame - 1) / max(1, frames - 1)
        stepped = math.floor(progress * 18.0) / 18.0
        mover.location = (0.0, start_y + (end_y - start_y) * stepped, 0.0)
        mover.keyframe_insert("location", frame=frame)
    if mover.animation_data and mover.animation_data.action:
        set_step_keys(mover.animation_data.action)

    camera.data.type = "ORTHO"
    camera.data.ortho_scale = max(max_v.z - min_v.z, 0.5) * 2.05
    if view == "side":
        camera.location = Vector((center.x + radius * 3.00, center.y + radius * 0.05, focus_z))
        look_at(camera, Vector((center.x, center.y + radius * 0.05, focus_z)))
    else:
        camera.location = Vector((center.x + radius * 0.16, center.y + radius * 3.80, focus_z))
        look_at(camera, Vector((center.x, center.y + radius * 0.08, focus_z)))

    scene = bpy.context.scene
    scene.frame_start = 1
    scene.frame_end = frames
    scene.frame_set(1)
    out_path = out_root / f"BoneWalker_rigid_biped_{view}_preview.mp4"
    configure_video_output(out_path)
    blend_path = out_root / f"BoneWalker_rigid_biped_{view}_preview.blend"
    bpy.ops.wm.save_as_mainfile(filepath=str(blend_path))
    bpy.ops.render.render(animation=True)

    manifest = {
        "enemy_id": "BoneWalker",
        "source_glb": str(spec.source_glb),
        "front_axis": "+Y",
        "view": view,
        "blend": str(blend_path),
        "video": str(out_path),
        "frames": frames,
        "fps": fps,
        "authoring_method": "simple bones with one-weight rigid segment groups",
        "motion_goal": "rigid anatomical biped joints with reduced vertical hop for better in-map travel-speed match",
        "weight_counts": weight_counts,
        "qa_status": "candidate only; requires frame inspection and Claude QA review before user handoff",
    }
    (out_root / f"BoneWalker_rigid_biped_{view}_manifest.json").write_text(json.dumps(manifest, indent=2), encoding="utf-8")
    print(json.dumps(manifest, indent=2))
    return manifest


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--enemy-id", default="BoneWalker")
    parser.add_argument("--out-root", default=str(DEFAULT_RUN_ROOT))
    parser.add_argument("--view", choices=("front", "side"), default="front")
    parser.add_argument("--frames", type=int, default=96)
    parser.add_argument("--width", type=int, default=1280)
    parser.add_argument("--height", type=int, default=720)
    parser.add_argument("--fps", type=int, default=15)
    argv = sys.argv
    args = parser.parse_args(argv[argv.index("--") + 1 :] if "--" in argv else [])
    render_video(args.enemy_id, Path(args.out_root), args.view, args.frames, args.width, args.height, args.fps)


if __name__ == "__main__":
    main()
