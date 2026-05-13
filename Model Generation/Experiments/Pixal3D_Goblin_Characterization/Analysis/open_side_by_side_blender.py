import math
from pathlib import Path

import bpy
from mathutils import Matrix, Vector


ROOT = Path(r"C:\UE\T66\Model Generation\Experiments\Pixal3D_Goblin_Characterization")
VARIANTS = [
    ("A", "Variant A - Maximum readability", 3.3),
    ("B", "Variant B - Moderate readability", 0.0),
    ("C", "Variant C - Current style baseline", -3.3),
]


def world_bbox(objects):
    mins = Vector((math.inf, math.inf, math.inf))
    maxs = Vector((-math.inf, -math.inf, -math.inf))
    for obj in objects:
        if obj.type != "MESH":
            continue
        for corner in obj.bound_box:
            world = obj.matrix_world @ Vector(corner)
            mins.x = min(mins.x, world.x)
            mins.y = min(mins.y, world.y)
            mins.z = min(mins.z, world.z)
            maxs.x = max(maxs.x, world.x)
            maxs.y = max(maxs.y, world.y)
            maxs.z = max(maxs.z, world.z)
    return mins, maxs


def import_variant(variant, label, x_offset):
    path = ROOT / "PostProcessed" / "QuadRetro" / f"Variant_{variant}" / "Models" / f"Variant_{variant}_QuadRetro.glb"
    if not path.exists():
        raise FileNotFoundError(str(path))
    before = set(bpy.data.objects)
    bpy.ops.import_scene.gltf(filepath=str(path))
    imported = [obj for obj in bpy.data.objects if obj not in before]
    meshes = [obj for obj in imported if obj.type == "MESH"]
    if not meshes:
        raise RuntimeError(f"No mesh imported for Variant {variant}")

    mins, maxs = world_bbox(meshes)
    center = (mins + maxs) * 0.5
    height = max(maxs.z - mins.z, 1e-5)
    scale = 2.4 / height
    transform = Matrix.Translation(Vector((x_offset, 0.0, 0.0)) - center * scale) @ Matrix.Diagonal((scale, scale, scale, 1.0))
    imported_set = set(imported)
    for obj in imported:
        if obj.parent is None or obj.parent not in imported_set:
            obj.matrix_world = transform @ obj.matrix_world
        obj.name = f"PostProcessed_Variant_{variant}_{obj.name}"
    bpy.context.view_layer.update()

    return imported


def setup_scene():
    bpy.ops.wm.read_factory_settings(use_empty=True)
    scene = bpy.context.scene
    scene.render.engine = "BLENDER_EEVEE"
    scene.render.resolution_x = 1600
    scene.render.resolution_y = 900

    world = bpy.data.worlds.new("WhiteWorld")
    world.color = (1.0, 1.0, 1.0)
    scene.world = world

    for variant, label, x_offset in VARIANTS:
        import_variant(variant, label, x_offset)

    light_data = bpy.data.lights.new("Shared_Area_Key", type="AREA")
    light_data.energy = 1400.0
    light_data.size = 6.0
    light = bpy.data.objects.new("Shared_Area_Key", light_data)
    light.location = (0.0, 5.0, 5.0)
    bpy.context.scene.collection.objects.link(light)

    cam_data = bpy.data.cameras.new("SideBySide_Camera")
    cam_data.type = "ORTHO"
    cam_data.ortho_scale = 10.2
    cam = bpy.data.objects.new("SideBySide_Camera", cam_data)
    cam.location = (0.0, 9.0, 1.0)
    bpy.context.scene.collection.objects.link(cam)
    scene.camera = cam

    target = bpy.data.objects.new("Camera_Target", None)
    target.location = (0.0, 0.0, 0.1)
    bpy.context.scene.collection.objects.link(target)
    track = cam.constraints.new(type="TRACK_TO")
    track.target = target
    track.track_axis = "TRACK_NEGATIVE_Z"
    track.up_axis = "UP_Y"

    if bpy.context.screen is not None:
        for area in bpy.context.screen.areas:
            if area.type == "VIEW_3D":
                region_3d = area.spaces.active.region_3d
                area.spaces.active.shading.type = "MATERIAL"
                region_3d.view_perspective = "CAMERA"

    bpy.ops.wm.save_as_mainfile(filepath=str(ROOT / "Pixal3D_Goblin_SideBySide.blend"))


setup_scene()
