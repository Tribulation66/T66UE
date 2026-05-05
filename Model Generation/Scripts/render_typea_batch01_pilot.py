import math
import os
import sys

import bpy
from mathutils import Vector


def scene_engine():
    engines = {item.identifier for item in bpy.types.RenderSettings.bl_rna.properties["engine"].enum_items}
    return "BLENDER_EEVEE_NEXT" if "BLENDER_EEVEE_NEXT" in engines else "BLENDER_EEVEE"


def main():
    if len(sys.argv) < 3:
        raise SystemExit("usage: blender --background --python render_typea_batch01_pilot.py -- <input.glb> <output_prefix>")

    argv = sys.argv[sys.argv.index("--") + 1 :] if "--" in sys.argv else sys.argv[1:]
    glb_path = os.path.abspath(argv[0])
    output_prefix = os.path.abspath(argv[1])
    os.makedirs(os.path.dirname(output_prefix), exist_ok=True)

    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete()
    bpy.ops.import_scene.gltf(filepath=glb_path)

    objs = [obj for obj in bpy.context.scene.objects if obj.type in {"MESH", "EMPTY", "ARMATURE"}]
    meshes = [obj for obj in bpy.context.scene.objects if obj.type == "MESH"]

    min_v = Vector((float("inf"), float("inf"), float("inf")))
    max_v = Vector((float("-inf"), float("-inf"), float("-inf")))
    for obj in meshes:
        for corner in obj.bound_box:
            world = obj.matrix_world @ Vector(corner)
            min_v.x = min(min_v.x, world.x)
            min_v.y = min(min_v.y, world.y)
            min_v.z = min(min_v.z, world.z)
            max_v.x = max(max_v.x, world.x)
            max_v.y = max(max_v.y, world.y)
            max_v.z = max(max_v.z, world.z)

    size = max_v - min_v if meshes else Vector((0.0, 0.0, 0.0))
    center = (min_v + max_v) * 0.5 if meshes else Vector((0.0, 0.0, 0.0))
    for obj in objs:
        obj.location -= center

    bpy.ops.object.light_add(type="AREA", location=(0.0, -3.0, 4.0))
    light = bpy.context.object
    light.name = "Pilot_Key_Light"
    light.data.energy = 450
    light.data.size = 4

    bpy.ops.object.camera_add(location=(0.0, -3.0, 1.1), rotation=(math.radians(72.0), 0.0, 0.0))
    bpy.context.scene.camera = bpy.context.object

    bpy.context.scene.render.engine = scene_engine()
    bpy.context.scene.render.resolution_x = 1000
    bpy.context.scene.render.resolution_y = 1000
    bpy.context.scene.view_settings.view_transform = "Standard"
    bpy.context.scene.world.color = (0.05, 0.05, 0.05)

    blend_path = output_prefix + ".blend"
    png_path = output_prefix + ".png"
    report_path = output_prefix + "_report.json"

    bpy.ops.wm.save_as_mainfile(filepath=blend_path)
    bpy.context.scene.render.filepath = png_path
    bpy.ops.render.render(write_still=True)

    import json

    with open(report_path, "w", encoding="utf-8") as handle:
        json.dump(
            {
                "input": glb_path,
                "objects": len(objs),
                "meshes": len(meshes),
                "size": [size.x, size.y, size.z],
                "blend": blend_path,
                "png": png_path,
            },
            handle,
            indent=2,
        )


if __name__ == "__main__":
    main()
