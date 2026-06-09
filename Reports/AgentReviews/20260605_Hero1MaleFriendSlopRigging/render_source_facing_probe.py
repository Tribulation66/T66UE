from pathlib import Path
import math

import bpy
from mathutils import Vector


RUN_ROOT = Path(r"C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415")
SOURCE_GLB = RUN_ROOT / "Outputs" / "Hero_1_Chad_Male.glb"
OUT_DIR = RUN_ROOT / "Blender" / "Rigging" / "Proofs"
OUT_DIR.mkdir(parents=True, exist_ok=True)


def world_bbox(objects):
    pts = []
    for obj in objects:
        if obj.type == "MESH":
            pts.extend([obj.matrix_world @ Vector(corner) for corner in obj.bound_box])
    mn = Vector((min(p.x for p in pts), min(p.y for p in pts), min(p.z for p in pts)))
    mx = Vector((max(p.x for p in pts), max(p.y for p in pts), max(p.z for p in pts)))
    return mn, mx


def look_at(obj, target):
    direction = Vector(target) - obj.location
    obj.rotation_euler = direction.to_track_quat("-Z", "Y").to_euler()


def make_text(label, loc):
    font = bpy.data.curves.new("Label_" + label, "FONT")
    obj = bpy.data.objects.new("Label_" + label, font)
    bpy.context.scene.collection.objects.link(obj)
    font.body = label
    font.align_x = "CENTER"
    font.align_y = "CENTER"
    font.size = 0.12
    obj.location = loc
    obj.rotation_euler = (math.radians(75), 0, 0)
    return obj


def setup_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete()
    bpy.ops.import_scene.gltf(filepath=str(SOURCE_GLB))
    meshes = [obj for obj in bpy.context.scene.objects if obj.type == "MESH"]
    bpy.context.view_layer.update()
    mn, mx = world_bbox(meshes)
    height = mx.z - mn.z
    scale = 1.8 / height
    center = (mn + mx) * 0.5
    for obj in meshes:
        obj.location -= center
        obj.scale *= scale
    bpy.context.view_layer.update()
    mn, mx = world_bbox(meshes)
    for obj in meshes:
        obj.location.z -= mn.z
    bpy.context.view_layer.update()
    return meshes


def render_axis(label, camera_loc):
    scene = bpy.context.scene
    try:
        scene.render.engine = "BLENDER_EEVEE_NEXT"
    except TypeError:
        scene.render.engine = "BLENDER_EEVEE"
    scene.render.resolution_x = 900
    scene.render.resolution_y = 1100
    scene.view_settings.view_transform = "Standard"
    scene.world.color = (1, 1, 1)
    target = (0, 0, 0.9)
    cam_data = bpy.data.cameras.new("Camera_" + label)
    cam_data.type = "ORTHO"
    cam_data.ortho_scale = 2.25
    cam = bpy.data.objects.new("Camera_" + label, cam_data)
    scene.collection.objects.link(cam)
    cam.location = camera_loc
    look_at(cam, target)
    scene.camera = cam
    lights = []
    for idx, loc in enumerate([(0, -4, 4), (3, -3, 3), (-3, -3, 3)]):
        data = bpy.data.lights.new(f"Light_{label}_{idx}", "AREA")
        data.energy = 350
        data.size = 5
        data.use_shadow = False
        obj = bpy.data.objects.new(data.name, data)
        scene.collection.objects.link(obj)
        obj.location = loc
        look_at(obj, target)
        lights.append(obj)
    text = make_text(label, (0, -0.35, 2.05))
    out = OUT_DIR / f"Hero_1_Chad_Male_facing_{label}.png"
    scene.render.filepath = str(out)
    scene.render.image_settings.file_format = "PNG"
    bpy.ops.render.render(write_still=True)
    bpy.data.objects.remove(cam, do_unlink=True)
    bpy.data.objects.remove(text, do_unlink=True)
    for obj in lights:
        bpy.data.objects.remove(obj, do_unlink=True)


setup_scene()
render_axis("plus_x_camera", (4, 0, 1.0))
render_axis("minus_x_camera", (-4, 0, 1.0))
render_axis("plus_y_camera", (0, 4, 1.0))
render_axis("minus_y_camera", (0, -4, 1.0))
print(f"T66_FACING_PROOF_DIR={OUT_DIR}")
