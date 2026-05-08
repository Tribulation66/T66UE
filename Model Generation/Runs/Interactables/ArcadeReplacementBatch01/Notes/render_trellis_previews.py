import math
import sys
from pathlib import Path

import bpy
from mathutils import Vector


def look_at(obj, target):
    direction = Vector(target) - obj.location
    obj.rotation_euler = direction.to_track_quat("-Z", "Y").to_euler()


def clear_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete()


def bounds_for(objects):
    mins = Vector((math.inf, math.inf, math.inf))
    maxs = Vector((-math.inf, -math.inf, -math.inf))
    for obj in objects:
        for corner in obj.bound_box:
            world = obj.matrix_world @ Vector(corner)
            mins.x = min(mins.x, world.x)
            mins.y = min(mins.y, world.y)
            mins.z = min(mins.z, world.z)
            maxs.x = max(maxs.x, world.x)
            maxs.y = max(maxs.y, world.y)
            maxs.z = max(maxs.z, world.z)
    return mins, maxs


def normalize(objects):
    mins, maxs = bounds_for(objects)
    center = (mins + maxs) * 0.5
    extent = max(maxs.x - mins.x, maxs.y - mins.y, maxs.z - mins.z)
    if extent <= 0:
        return
    scale = 2.4 / extent
    for obj in objects:
        obj.location = (obj.location - center) * scale
        obj.scale = obj.scale * scale


def render_one(glb_path, out_path):
    clear_scene()
    bpy.ops.import_scene.gltf(filepath=str(glb_path))
    meshes = [obj for obj in bpy.context.scene.objects if obj.type == "MESH"]
    if not meshes:
        raise RuntimeError(f"No mesh objects imported from {glb_path}")

    normalize(meshes)

    bpy.ops.object.light_add(type="AREA", location=(0.0, -3.0, 4.0))
    light = bpy.context.object
    light.name = "Preview_Key"
    light.data.energy = 550
    light.data.size = 5

    bpy.ops.object.camera_add(location=(3.4, -4.5, 2.6))
    camera = bpy.context.object
    look_at(camera, (0.0, 0.0, 0.15))
    camera.data.type = "ORTHO"
    camera.data.ortho_scale = 3.4
    bpy.context.scene.camera = camera

    bpy.context.scene.world.color = (1.0, 1.0, 1.0)
    try:
        bpy.context.scene.render.engine = "BLENDER_EEVEE_NEXT"
    except Exception:
        bpy.context.scene.render.engine = "BLENDER_EEVEE"
    bpy.context.scene.render.resolution_x = 900
    bpy.context.scene.render.resolution_y = 900
    bpy.context.scene.render.film_transparent = False
    bpy.context.scene.view_settings.view_transform = "Standard"
    bpy.context.scene.view_settings.look = "Medium High Contrast"
    bpy.context.scene.render.filepath = str(out_path)
    bpy.ops.render.render(write_still=True)


def main():
    argv = sys.argv
    if "--" not in argv:
        raise SystemExit("Expected -- <input_dir> <output_dir>")
    args = argv[argv.index("--") + 1 :]
    input_dir = Path(args[0])
    output_dir = Path(args[1])
    output_dir.mkdir(parents=True, exist_ok=True)

    for glb_path in sorted(input_dir.glob("*.glb")):
        render_one(glb_path, output_dir / f"{glb_path.stem}.png")


if __name__ == "__main__":
    main()
