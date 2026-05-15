"""
Render small frame previews for baked Arthur action candidates.

Run with Blender after opening the pilot .blend:
  blender --background Arthur_Animation_Pilot.blend --python Tools/render_arthur_action_previews.py -- --out-root Runs/Arthur_Animation_Pilot_YYYYMMDD/PreviewFrames
"""

import argparse
import json
import math
import sys
from pathlib import Path

import bpy
from mathutils import Vector


DEFAULT_ACTION_PREFIX = "AM_Hero_1_Chad_"
VIEW_SPECS = {
    "front": {"location": (0.0, 5.0, 1.25), "target_z": 1.0},
    "side": {"location": (5.0, 0.0, 1.25), "target_z": 1.0},
    "three_quarter": {"location": (4.0, 4.0, 1.35), "target_z": 1.0},
    "gameplay": {"location": (3.35, -5.35, 3.0), "target_z": 0.95},
}


def look_at(obj, target):
    direction = Vector(target) - obj.location
    obj.rotation_euler = direction.to_track_quat("-Z", "Y").to_euler()


def setup_scene():
    scene = bpy.context.scene
    scene.render.engine = "BLENDER_WORKBENCH"
    scene.display.shading.light = "STUDIO"
    scene.display.shading.color_type = "TEXTURE"
    scene.render.resolution_x = 320
    scene.render.resolution_y = 320
    scene.render.film_transparent = False
    scene.world.color = (0.78, 0.78, 0.78)

    camera_data = bpy.data.cameras.new("ArthurPreviewCamera")
    camera = bpy.data.objects.new("ArthurPreviewCamera", camera_data)
    bpy.context.collection.objects.link(camera)
    camera_data.type = "ORTHO"
    scene.camera = camera
    return camera


def visible_mesh_bounds():
    meshes = [obj for obj in bpy.data.objects if obj.type == "MESH" and not obj.hide_render]
    if not meshes:
        return Vector((0, 0, 0)), 2.8
    mins = [float("inf"), float("inf"), float("inf")]
    maxs = [float("-inf"), float("-inf"), float("-inf")]
    for obj in meshes:
        for corner in obj.bound_box:
            world = obj.matrix_world @ Vector(corner)
            for axis in range(3):
                mins[axis] = min(mins[axis], world[axis])
                maxs[axis] = max(maxs[axis], world[axis])
    center = Vector(((mins[0] + maxs[0]) * 0.5, (mins[1] + maxs[1]) * 0.5, (mins[2] + maxs[2]) * 0.5))
    max_dim = max(maxs[0] - mins[0], maxs[1] - mins[1], maxs[2] - mins[2])
    return center, max(max_dim * 1.22, 2.6)


def configure_camera(camera, view_name, center, ortho_scale):
    spec = VIEW_SPECS[view_name]
    base_location = Vector(spec["location"])
    target = Vector((center.x, center.y, center.z * spec["target_z"]))
    camera.location = target + base_location
    camera.data.ortho_scale = ortho_scale
    look_at(camera, target)


def find_target_armature(target_name):
    if target_name:
        armature = bpy.data.objects.get(target_name)
        if armature and armature.type == "ARMATURE":
            return armature
        raise RuntimeError(f"target armature not found: {target_name}")
    for candidate in ("Arthur_QuadRetro_Target_Armature", "Arthur_Target_Armature"):
        armature = bpy.data.objects.get(candidate)
        if armature and armature.type == "ARMATURE":
            return armature
    for obj in bpy.data.objects:
        if obj.type == "ARMATURE" and obj.name.startswith("Arthur"):
            return obj
    armature = next((obj for obj in bpy.data.objects if obj.type == "ARMATURE"), None)
    if armature:
        return armature
    raise RuntimeError("target armature not found")


def hide_source_objects():
    for obj in bpy.data.objects:
        if obj.name.startswith("UAL1_Source"):
            obj.hide_viewport = True
            obj.hide_render = True
        elif obj.type == "MESH" and obj.name.lower().startswith("mannequin"):
            obj.hide_viewport = True
            obj.hide_render = True
        elif obj.type == "MESH" and obj.name.lower() in {"cube", "icosphere"}:
            obj.hide_viewport = True
            obj.hide_render = True


def style_target_objects():
    for obj in bpy.data.objects:
        if obj.type != "MESH" or obj.hide_render:
            continue
        if not obj.material_slots:
            obj.color = (0.08, 0.10, 0.14, 1.0)


def sample_frames(action, samples):
    start = int(math.floor(action.frame_range[0]))
    end = int(math.ceil(action.frame_range[1]))
    if samples <= 1 or end <= start:
        return [start]
    return sorted({round(start + (end - start) * i / (samples - 1)) for i in range(samples)})


def main(argv):
    parser = argparse.ArgumentParser()
    parser.add_argument("--out-root", required=True)
    parser.add_argument("--samples", type=int, default=6)
    parser.add_argument("--action-prefix", default=DEFAULT_ACTION_PREFIX)
    parser.add_argument("--target-armature", default="")
    parser.add_argument("--views", default="front,side,three_quarter,gameplay")
    args = parser.parse_args(argv)

    out_root = Path(args.out_root).resolve()
    out_root.mkdir(parents=True, exist_ok=True)
    camera = setup_scene()
    hide_source_objects()
    style_target_objects()
    armature = find_target_armature(args.target_armature.strip())
    armature.animation_data_create()
    center, ortho_scale = visible_mesh_bounds()
    views = [view.strip() for view in args.views.split(",") if view.strip()]
    unknown_views = [view for view in views if view not in VIEW_SPECS]
    if unknown_views:
        raise RuntimeError(f"unknown preview views: {unknown_views}")

    manifest = []
    for action in sorted(bpy.data.actions, key=lambda item: item.name):
        if not action.name.startswith(args.action_prefix):
            continue
        armature.animation_data.action = action
        if action.slots:
            armature.animation_data.action_slot = action.slots[0]
        frames = sample_frames(action, args.samples)
        rendered_by_view = {}
        for view in views:
            configure_camera(camera, view, center, ortho_scale)
            rendered = []
            for frame in frames:
                bpy.context.scene.frame_set(frame)
                bpy.context.view_layer.update()
                out_path = out_root / f"{action.name}_{view}_f{frame:04d}.png"
                bpy.context.scene.render.filepath = str(out_path)
                bpy.ops.render.render(write_still=True)
                rendered.append(str(out_path))
            rendered_by_view[view] = rendered
        manifest.append({"action": action.name, "frames": frames, "views": rendered_by_view})

    manifest_path = out_root / "preview_manifest.json"
    manifest_path.write_text(
        json.dumps(
            {
                "action_prefix": args.action_prefix,
                "target_armature": armature.name,
                "views": views,
                "items": manifest,
            },
            indent=2,
        ),
        encoding="utf-8",
    )
    print(json.dumps({"preview_manifest": str(manifest_path), "actions": len(manifest), "views": views}, indent=2))


if __name__ == "__main__":
    argv = sys.argv
    if "--" in argv:
        argv = argv[argv.index("--") + 1 :]
    else:
        argv = []
    main(argv)
