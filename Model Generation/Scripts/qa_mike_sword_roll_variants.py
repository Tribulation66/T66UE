import math
from pathlib import Path

import bpy
from mathutils import Matrix, Vector


OUT_DIR = Path(
    r"C:\UE\T66\Model Generation\Runs\Heroes\Chad_Pass02_ProcessBuild\Rigging\Mike_Chad_RigPrototype_A03_LiftedNeckBridge\Renders\SwordScaleRollTests"
)
HAND_TARGET = Vector((0.4735041856765747, -0.09144768118858337, 0.8323389291763306))
BLADE_DIR = Vector((0.035002201795578, -0.7200453281402588, 0.6930436491966248)).normalized()


def look_at(obj, target):
    direction = Vector(target) - obj.location
    obj.rotation_euler = direction.to_track_quat("-Z", "Y").to_euler()


def make_camera(name, location, target, ortho):
    data = bpy.data.cameras.new(name)
    camera = bpy.data.objects.new(name, data)
    bpy.context.collection.objects.link(camera)
    camera.location = Vector(location)
    look_at(camera, target)
    data.type = "ORTHO"
    data.ortho_scale = ortho
    return camera


def setup_scene():
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    for obj in list(bpy.context.scene.objects):
        if obj.name.startswith(("RollTest_", "RollCam_", "RollLight_")):
            bpy.data.objects.remove(obj, do_unlink=True)

    scene = bpy.context.scene
    scene.render.engine = "BLENDER_EEVEE"
    scene.eevee.taa_render_samples = 8
    scene.render.resolution_x = 512
    scene.render.resolution_y = 640
    scene.view_settings.view_transform = "Standard"
    scene.view_settings.look = "Medium High Contrast"
    scene.world.color = (0.45, 0.45, 0.45)

    light_data = bpy.data.lights.new("RollLight_Key", "AREA")
    light_data.energy = 500
    light_data.size = 5
    light = bpy.data.objects.new("RollLight_Key", light_data)
    bpy.context.collection.objects.link(light)
    light.location = (0, -4, 5)

    return [
        ("front", make_camera("RollCam_Front", (0, -4.0, 1.05), HAND_TARGET, 1.25)),
        ("right", make_camera("RollCam_Right", (4.0, -0.08, 1.03), HAND_TARGET, 1.25)),
        ("grip_front", make_camera("RollCam_GripFront", (0.45, -1.35, 0.86), HAND_TARGET, 0.52)),
        ("grip_right", make_camera("RollCam_GripRight", (1.5, -0.1, 0.86), HAND_TARGET, 0.52)),
    ]


def render_variant(sword, label, degrees, cameras):
    duplicate = sword.copy()
    duplicate.data = sword.data.copy()
    duplicate.name = f"RollTest_{label}"
    duplicate.data.name = f"RollTest_{label}_Mesh"
    bpy.context.collection.objects.link(duplicate)
    duplicate.hide_render = False
    duplicate.hide_set(False)

    transform = (
        Matrix.Translation(HAND_TARGET)
        @ Matrix.Rotation(math.radians(degrees), 4, BLADE_DIR)
        @ Matrix.Scale(2.0, 4)
        @ Matrix.Translation(-HAND_TARGET)
    )
    duplicate.data.transform(transform)
    duplicate.data.update()

    paths = []
    scene = bpy.context.scene
    for suffix, camera in cameras:
        scene.camera = camera
        path = OUT_DIR / f"{label}_{suffix}.png"
        scene.render.filepath = str(path)
        bpy.ops.render.render(write_still=True)
        paths.append(path)

    duplicate.hide_render = True
    duplicate.hide_set(True)
    return paths


def main():
    sword = bpy.data.objects.get("Mike_Chad_SwordProxy")
    if sword is None:
        raise RuntimeError("Missing Mike_Chad_SwordProxy")

    cameras = setup_scene()
    sword.hide_render = True
    sword.hide_set(True)

    rows = []
    for label, degrees in [("roll_m90", -90), ("roll_p90", 90), ("roll_180", 180)]:
        rows.append((label, degrees, render_variant(sword, label, degrees, cameras)))

    for label, degrees, paths in rows:
        print(f"SWORD_ROLL_VARIANT={label};degrees={degrees};paths={','.join(str(path) for path in paths)}")


if __name__ == "__main__":
    main()
