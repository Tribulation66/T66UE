"""
Render a quick native Blender MP4 for one Easy mob moving toward camera.

This is a visual preview tool, not an Unreal import step. It reuses the Easy
mob VAT source rig builder, adds actor-level travel, and renders a single video
that makes movement readable before VAT baking decisions.

Run:
    blender --background --python Tools/render_easy_mob_movement_preview.py -- --enemy-id Slime
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
    / "Slime_MoveTowardPreview_20260521"
)


def load_vat_tool():
    spec = importlib.util.spec_from_file_location("easy_mob_vat_sources", VAT_TOOL_PATH)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"Could not load VAT source tool: {VAT_TOOL_PATH}")
    module = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = module
    spec.loader.exec_module(module)
    return module


def look_at(obj, target: Vector) -> None:
    direction = target - obj.location
    obj.rotation_euler = direction.to_track_quat("-Z", "Y").to_euler()


def add_cycles(action: bpy.types.Action) -> None:
    for fcurve in getattr(action, "fcurves", []) or []:
        if not any(modifier.type == "CYCLES" for modifier in fcurve.modifiers):
            fcurve.modifiers.new(type="CYCLES")


def set_linear_keys(action: bpy.types.Action) -> None:
    for fcurve in getattr(action, "fcurves", []) or []:
        for key in fcurve.keyframe_points:
            key.interpolation = "LINEAR"


def set_constant_keys(action: bpy.types.Action) -> None:
    for fcurve in getattr(action, "fcurves", []) or []:
        for key in fcurve.keyframe_points:
            key.interpolation = "CONSTANT"


def pulsed_progress_values(frames: int, hop_frames: int = 8) -> list[float]:
    if frames <= 1:
        return [0.0]
    velocity_pattern = [0.62, 0.82, 1.18, 1.52, 1.42, 1.05, 0.78, 0.61]
    distances = [velocity_pattern[index % hop_frames] for index in range(frames - 1)]
    total = sum(distances)
    progress = [0.0]
    running = 0.0
    for distance in distances:
        running += distance
        progress.append(running / total)
    return progress


def preview_front_sign(spec) -> float:
    if spec.enemy_id == "BoneWalker":
        return -1.0
    return 1.0


def preview_camera_profile(spec) -> dict[str, float]:
    if spec.enemy_id == "BoneWalker":
        return {
            "lens": 58.0,
            "distance": 4.00,
            "height": 1.04,
            "look_offset": 0.10,
        }
    return {
        "lens": 38.0,
        "distance": 5.65,
        "height": 1.18,
        "look_offset": -0.42,
    }


def preview_travel_profile(spec) -> dict[str, float]:
    if spec.enemy_id == "BoneWalker":
        return {
            "start": 2.15,
            "end": 0.40,
            "lift": 0.035,
            "sink": 0.012,
        }
    return {
        "start": 2.45,
        "end": 0.72,
        "lift": 0.13,
        "sink": 0.030,
    }


def bounds_for_objects(objects) -> tuple[Vector, Vector, Vector]:
    coords = []
    for obj in objects:
        if obj.type != "MESH":
            continue
        coords.extend(obj.matrix_world @ Vector(corner) for corner in obj.bound_box)
    if not coords:
        raise RuntimeError("No mesh bounds available")
    min_v = Vector((min(c.x for c in coords), min(c.y for c in coords), min(c.z for c in coords)))
    max_v = Vector((max(c.x for c in coords), max(c.y for c in coords), max(c.z for c in coords)))
    return min_v, max_v, (min_v + max_v) * 0.5


def make_material(name: str, color: tuple[float, float, float, float]) -> bpy.types.Material:
    mat = bpy.data.materials.new(name)
    mat.diffuse_color = color
    mat.use_nodes = True
    tree = mat.node_tree
    tree.nodes.clear()
    output = tree.nodes.new(type="ShaderNodeOutputMaterial")
    output.location = (260, 0)
    emission = tree.nodes.new(type="ShaderNodeEmission")
    emission.location = (20, 0)
    emission.inputs["Color"].default_value = color
    emission.inputs["Strength"].default_value = 1.0
    tree.links.new(emission.outputs["Emission"], output.inputs["Surface"])
    return mat


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


def add_floor_and_markers(spec, center: Vector, radius: float, floor_z: float, front_sign: float) -> None:
    floor_mat = make_material("Preview_Floor_Matte", (0.22, 0.24, 0.25, 1.0))
    marker_mat = make_material("Preview_Movement_Markers", (0.85, 0.70, 0.24, 1.0))
    bpy.ops.mesh.primitive_plane_add(size=radius * 8.0, location=(center.x, center.y, floor_z))
    floor = bpy.context.object
    floor.name = f"{spec.enemy_id}_MoveToward_Floor"
    floor.data.materials.append(floor_mat)
    for index, y in enumerate([radius * 2.2, radius * 1.2, radius * 0.2, -radius * 0.8]):
        bpy.ops.mesh.primitive_cube_add(size=1.0, location=(center.x - radius * 0.85, center.y + y * front_sign, floor_z + 0.01))
        marker = bpy.context.object
        marker.name = f"Approach_Depth_Marker_{index + 1:02d}"
        marker.dimensions = (radius * 0.08, radius * 0.04, 0.012)
        bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
        marker.data.materials.append(marker_mat)


def setup_render_scene(width: int, height: int, fps: int) -> bpy.types.Object:
    scene = bpy.context.scene
    scene.render.engine = "BLENDER_EEVEE"
    scene.eevee.taa_render_samples = 48
    scene.render.resolution_x = width
    scene.render.resolution_y = height
    scene.render.fps = fps
    scene.view_settings.view_transform = "Standard"
    scene.view_settings.look = "None"
    scene.world = bpy.data.worlds.new("Slime_MoveToward_World") if not scene.world else scene.world
    scene.world.color = (0.54, 0.56, 0.58)

    camera_data = bpy.data.cameras.new("Slime_MoveToward_Camera")
    camera_data.lens = 38.0
    camera = bpy.data.objects.new("Slime_MoveToward_Camera", camera_data)
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


def render_preview(enemy_id: str, out_root: Path, frames: int, width: int, height: int, fps: int) -> dict:
    vat = load_vat_tool()
    spec = next((item for item in vat.MOBS if item.enemy_id.lower() == enemy_id.lower()), None)
    if spec is None:
        available = ", ".join(item.enemy_id for item in vat.MOBS)
        raise ValueError(f"Unknown --enemy-id {enemy_id}. Available: {available}")

    out_root.mkdir(parents=True, exist_ok=True)
    vat.reset_scene()
    camera = setup_render_scene(width, height, fps)

    mesh = vat.import_glb(spec.source_glb)
    mesh.name = f"Preview_{spec.enemy_id}_Mesh"
    mesh.data.name = f"Preview_{spec.enemy_id}_MeshData"
    apply_unlit_preview_materials(mesh)
    armature = vat.create_armature(spec, mesh)
    weight_counts = vat.assign_weights(spec, mesh)
    action = vat.create_action(spec, armature, "Move", vat.CLIPS["Move"])
    add_cycles(action)
    armature.animation_data_create()
    armature.animation_data.action = action
    if hasattr(action, "slots") and action.slots:
        armature.animation_data.action_slot = action.slots[0]

    bpy.context.view_layer.update()
    min_v, max_v, center = bounds_for_objects([mesh])
    size = max_v - min_v
    radius = max(size.x, size.y, size.z, 0.5)
    focus_z = min_v.z + max(size.z, 0.1) * 0.48
    floor_z = min_v.z - radius * 0.035
    front_sign = preview_front_sign(spec)
    camera_profile = preview_camera_profile(spec)
    travel_profile = preview_travel_profile(spec)

    add_floor_and_markers(spec, center, radius, floor_z, front_sign)

    mover = bpy.data.objects.new(f"{spec.enemy_id}_MoveToward_Mover", None)
    bpy.context.collection.objects.link(mover)
    armature.parent = mover
    start_y = center.y - front_sign * radius * travel_profile["start"]
    end_y = center.y + front_sign * radius * travel_profile["end"]
    progress_values = pulsed_progress_values(frames)
    for frame, progress in enumerate(progress_values, start=1):
        hop_u = ((frame - 1) % 8) / 8.0
        lift = max(0.0, math.sin(math.pi * hop_u))
        contact = max(0.0, math.cos(math.tau * hop_u)) ** 2
        y = (start_y - center.y) + ((end_y - start_y) * progress)
        mover.location = (0.0, y, radius * (travel_profile["lift"] * lift - travel_profile["sink"] * contact))
        mover.keyframe_insert("location", frame=frame)
    if mover.animation_data and mover.animation_data.action:
        set_constant_keys(mover.animation_data.action)

    camera.data.lens = camera_profile["lens"]
    camera.location = Vector((
        center.x + radius * 0.22,
        center.y + front_sign * radius * camera_profile["distance"],
        focus_z + radius * camera_profile["height"],
    ))
    look_at(camera, Vector((center.x, center.y + front_sign * radius * camera_profile["look_offset"], focus_z)))

    scene = bpy.context.scene
    scene.frame_start = 1
    scene.frame_end = frames
    scene.frame_set(1)
    configure_video_output(out_root / f"{spec.enemy_id}_MoveTowardCamera_preview.mp4")

    blend_path = out_root / f"{spec.enemy_id}_MoveTowardCamera_preview.blend"
    bpy.ops.wm.save_as_mainfile(filepath=str(blend_path))
    bpy.ops.render.render(animation=True)

    manifest = {
        "enemy_id": spec.enemy_id,
        "source_glb": str(spec.source_glb),
        "blend": str(blend_path),
        "video": str(out_root / f"{spec.enemy_id}_MoveTowardCamera_preview.mp4"),
        "frames": frames,
        "fps": scene.render.fps,
        "movement": {
            "start_location": [0.0, start_y - center.y, 0.0],
            "end_location": [0.0, end_y - center.y, 0.0],
            "description": f"{spec.enemy_id} actor moves toward the fixed camera with pulsed per-frame travel while the local Move deformation loops.",
            "front_sign_y": front_sign,
            "front_axis_note": "+Y camera for Slime baseline; -Y camera for BoneWalker source front.",
            "camera_profile": camera_profile,
            "travel_profile": travel_profile,
        },
        "style": {
            "pose_interpolation": "CONSTANT",
            "travel_interpolation": "CONSTANT",
            "travel_pattern": "positive per-frame pulsed progress",
        },
        "weight_counts": weight_counts,
    }
    (out_root / f"{spec.enemy_id}_MoveTowardCamera_preview_manifest.json").write_text(
        json.dumps(manifest, indent=2),
        encoding="utf-8",
    )
    print(json.dumps(manifest, indent=2))
    return manifest


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--enemy-id", default="Slime")
    parser.add_argument("--out-root", default=str(DEFAULT_RUN_ROOT))
    parser.add_argument("--frames", type=int, default=96)
    parser.add_argument("--width", type=int, default=1280)
    parser.add_argument("--height", type=int, default=720)
    parser.add_argument("--fps", type=int, default=30)
    argv = sys.argv
    args = parser.parse_args(argv[argv.index("--") + 1 :] if "--" in argv else [])
    render_preview(args.enemy_id, Path(args.out_root), args.frames, args.width, args.height, args.fps)


if __name__ == "__main__":
    main()
