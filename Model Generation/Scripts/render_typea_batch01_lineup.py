import json
import math
import os
import sys
from pathlib import Path

import bpy
from mathutils import Matrix, Vector


def scene_engine():
    engines = {item.identifier for item in bpy.types.RenderSettings.bl_rna.properties["engine"].enum_items}
    return "BLENDER_EEVEE_NEXT" if "BLENDER_EEVEE_NEXT" in engines else "BLENDER_EEVEE"


def object_bounds(objects):
    min_v = Vector((float("inf"), float("inf"), float("inf")))
    max_v = Vector((float("-inf"), float("-inf"), float("-inf")))
    for obj in objects:
        if obj.type != "MESH":
            continue
        for corner in obj.bound_box:
            world = obj.matrix_world @ Vector(corner)
            min_v.x = min(min_v.x, world.x)
            min_v.y = min(min_v.y, world.y)
            min_v.z = min(min_v.z, world.z)
            max_v.x = max(max_v.x, world.x)
            max_v.y = max(max_v.y, world.y)
            max_v.z = max(max_v.z, world.z)
    if min_v.x == float("inf"):
        return Vector((0.0, 0.0, 0.0)), Vector((0.0, 0.0, 0.0)), Vector((0.0, 0.0, 0.0))
    return min_v, max_v, max_v - min_v


def main():
    argv = sys.argv[sys.argv.index("--") + 1 :] if "--" in sys.argv else sys.argv[1:]
    if len(argv) < 2:
        raise SystemExit("usage: blender --background --python render_typea_batch01_lineup.py -- <output_prefix> <input.glb> [input.glb ...]")

    output_prefix = Path(argv[0]).resolve()
    glb_paths = [Path(path).resolve() for path in argv[1:]]
    output_prefix.parent.mkdir(parents=True, exist_ok=True)

    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete()

    reports = []
    spacing = 2.4
    origin_x = -spacing * (len(glb_paths) - 1) * 0.5

    for index, glb_path in enumerate(glb_paths):
        before = set(bpy.context.scene.objects)
        bpy.ops.import_scene.gltf(filepath=str(glb_path))
        imported = [obj for obj in bpy.context.scene.objects if obj not in before]
        meshes = [obj for obj in imported if obj.type == "MESH"]
        min_v, max_v, size = object_bounds(meshes)
        center = (min_v + max_v) * 0.5
        max_dim = max(size.x, size.y, size.z, 0.001)
        scale = 1.05 / max_dim
        x = origin_x + spacing * index
        transform = Matrix.Translation(Vector((x, 0.0, 0.0))) @ Matrix.Scale(scale, 4) @ Matrix.Translation(-center)
        for obj in [candidate for candidate in imported if candidate.parent not in imported]:
            obj.matrix_world = transform @ obj.matrix_world
        reports.append(
            {
                "input": str(glb_path),
                "objects": len(imported),
                "meshes": len(meshes),
                "size": [size.x, size.y, size.z],
                "review_x": x,
            }
        )

        bpy.ops.object.text_add(location=(x, -0.72, -1.0), rotation=(math.radians(80.0), 0.0, 0.0))
        label = bpy.context.object
        label.name = f"Label_{index + 1}"
        label.data.body = glb_path.stem.replace("_TypeA_", "\nTypeA ").replace("_Green_S1337", "\nS1337")
        label.data.align_x = "CENTER"
        label.data.size = 0.08
        label.data.materials.append(make_label_material())

    bpy.ops.object.light_add(type="AREA", location=(0.0, -4.0, 4.0))
    light = bpy.context.object
    light.name = "Lineup_Key_Light"
    light.data.energy = 700
    light.data.size = 5

    bpy.ops.object.camera_add(location=(0.0, -6.8, 1.2), rotation=(math.radians(78.0), 0.0, 0.0))
    camera = bpy.context.object
    camera.data.type = "ORTHO"
    camera.data.ortho_scale = 11.5
    bpy.context.scene.camera = camera

    bpy.context.scene.render.engine = scene_engine()
    bpy.context.scene.render.resolution_x = 1800
    bpy.context.scene.render.resolution_y = 900
    bpy.context.scene.view_settings.view_transform = "Standard"
    bpy.context.scene.world.color = (0.05, 0.05, 0.05)

    bpy.ops.wm.save_as_mainfile(filepath=str(output_prefix.with_suffix(".blend")))
    bpy.context.scene.render.filepath = str(output_prefix.with_suffix(".png"))
    bpy.ops.render.render(write_still=True)

    with output_prefix.with_name(output_prefix.name + "_report.json").open("w", encoding="utf-8") as handle:
        json.dump({"outputs": reports}, handle, indent=2)


def make_label_material():
    mat = bpy.data.materials.get("Lineup_Label")
    if mat is None:
        mat = bpy.data.materials.new("Lineup_Label")
        mat.diffuse_color = (0.9, 0.9, 0.9, 1.0)
    return mat


if __name__ == "__main__":
    main()
