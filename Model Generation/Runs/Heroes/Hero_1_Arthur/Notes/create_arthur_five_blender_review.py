import math
from pathlib import Path

import bpy
from mathutils import Vector


ROOT = Path(r"C:\UE\T66")
RUN_ROOT = ROOT / "Model Generation" / "Runs" / "Heroes" / "Hero_1_Arthur"
RAW_DIR = RUN_ROOT / "Raw" / "Trellis"
SCENE_DIR = ROOT / "Model Generation" / "Scenes"
SCENE_DIR.mkdir(parents=True, exist_ok=True)

BLEND_PATH = SCENE_DIR / "Arthur_Six_Trellis_Review.blend"
RENDER_PATH = SCENE_DIR / "Arthur_Six_Trellis_Review.png"

MODELS = [
    (
        "ModelReference_Green",
        RAW_DIR / "ModelReference_Green_S1337_D80000_Trellis2.glb",
    ),
    (
        "Arthur_01_NoWeapon",
        RAW_DIR / "Hero_1_Arthur_01_Apose_NoWeapon_Green_S1337_D80000_Trellis2.glb",
    ),
    (
        "Arthur_02_RightHandWeapon",
        RAW_DIR / "Hero_1_Arthur_02_Apose_RightHandWeapon_Green_S1337_D80000_Trellis2.glb",
    ),
    (
        "Arthur_05_BodyOnly",
        RAW_DIR / "Hero_1_Arthur_05_BodyOnly_Green_S1337_D80000_Trellis2.glb",
    ),
    (
        "Arthur_03_WeaponOnly",
        RAW_DIR / "Hero_1_Arthur_03_WeaponOnly_Green_S1337_D200000_Trellis2.glb",
    ),
    (
        "Arthur_04_HeadOnly",
        RAW_DIR / "Hero_1_Arthur_04_HeadOnly_Green_S1337_D120000_Trellis2.glb",
    ),
]


def reset_scene() -> None:
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete()
    for collection in list(bpy.data.collections):
        if not collection.users:
            bpy.data.collections.remove(collection)


def world_bounds(objects):
    points = []
    for obj in objects:
        if obj.type != "MESH":
            continue
        for corner in obj.bound_box:
            points.append(obj.matrix_world @ Vector(corner))
    if not points:
        return None
    min_corner = Vector((min(p.x for p in points), min(p.y for p in points), min(p.z for p in points)))
    max_corner = Vector((max(p.x for p in points), max(p.y for p in points), max(p.z for p in points)))
    return min_corner, max_corner


def look_at(obj, target):
    direction = Vector(target) - obj.location
    obj.rotation_euler = direction.to_track_quat("-Z", "Y").to_euler()


def import_model(label, filepath, x_position):
    if not filepath.exists():
        raise FileNotFoundError(filepath)

    before = set(bpy.data.objects)
    bpy.ops.import_scene.gltf(filepath=str(filepath))
    imported = [obj for obj in bpy.data.objects if obj not in before]

    collection = bpy.data.collections.new(label)
    bpy.context.scene.collection.children.link(collection)

    root = bpy.data.objects.new(f"{label}_Root", None)
    collection.objects.link(root)

    for obj in imported:
        for owner in list(obj.users_collection):
            owner.objects.unlink(obj)
        collection.objects.link(obj)
        obj.parent = root

    bounds = world_bounds(imported)
    if bounds is None:
        raise RuntimeError(f"No mesh bounds found for {label}")

    min_corner, max_corner = bounds
    size = max_corner - min_corner
    max_dim = max(size.x, size.y, size.z)
    target_dim = 2.65 if "WeaponOnly" not in label else 2.35
    if max_dim > 0:
        root.scale = (target_dim / max_dim, target_dim / max_dim, target_dim / max_dim)

    bpy.context.view_layer.update()
    min_corner, max_corner = world_bounds(imported)
    center = (min_corner + max_corner) * 0.5
    root.location += Vector((x_position - center.x, -center.y, -min_corner.z))

    text_curve = bpy.data.curves.new(f"{label}_LabelCurve", "FONT")
    text_curve.body = label.replace("_", "\n", 1)
    text_curve.align_x = "CENTER"
    text_curve.align_y = "CENTER"
    text_curve.size = 0.22
    label_obj = bpy.data.objects.new(f"{label}_Label", text_curve)
    label_obj.location = (x_position, -2.05, -0.28)
    label_obj.rotation_euler = (math.radians(90), 0, 0)
    label_mat = bpy.data.materials.get("Review_Label_Mat") or bpy.data.materials.new("Review_Label_Mat")
    label_mat.diffuse_color = (0.82, 0.82, 0.78, 1.0)
    label_mat.use_nodes = True
    bsdf = label_mat.node_tree.nodes.get("Principled BSDF")
    if bsdf:
        if "Base Color" in bsdf.inputs:
            bsdf.inputs["Base Color"].default_value = (0.82, 0.82, 0.78, 1.0)
        if "Emission Color" in bsdf.inputs:
            bsdf.inputs["Emission Color"].default_value = (0.82, 0.82, 0.78, 1.0)
        if "Emission Strength" in bsdf.inputs:
            bsdf.inputs["Emission Strength"].default_value = 1.4
    label_obj.data.materials.append(label_mat)
    collection.objects.link(label_obj)

    return root


def build_scene():
    reset_scene()

    positions = [-8.6, -5.15, -1.7, 1.7, 5.15, 8.6]
    for (label, filepath), x_position in zip(MODELS, positions):
        import_model(label, filepath, x_position)

    floor_mat = bpy.data.materials.new("Review_Floor_Mat")
    floor_mat.diffuse_color = (0.07, 0.075, 0.08, 1.0)
    bpy.ops.mesh.primitive_plane_add(size=17.5, location=(0, 0, -0.02))
    floor = bpy.context.object
    floor.name = "Review_Floor"
    floor.data.materials.append(floor_mat)

    bpy.ops.object.light_add(type="AREA", location=(0, -4.8, 7.0))
    key = bpy.context.object
    key.name = "Arthur_Review_Key_Light"
    key.data.energy = 650
    key.data.size = 6.0

    bpy.ops.object.light_add(type="POINT", location=(-5.5, 3.0, 3.0))
    fill = bpy.context.object
    fill.name = "Arthur_Review_Fill_Light"
    fill.data.energy = 110

    bpy.ops.object.camera_add(location=(0, -12.0, 1.25))
    camera = bpy.context.object
    camera.name = "Arthur_Five_Review_Camera"
    camera.data.type = "ORTHO"
    camera.data.ortho_scale = 22.0
    camera.rotation_euler = (math.radians(90), 0, 0)
    bpy.context.scene.camera = camera

    world = bpy.context.scene.world or bpy.data.worlds.new("World")
    bpy.context.scene.world = world
    world.color = (0.025, 0.028, 0.032)

    available_engines = {item.identifier for item in bpy.types.RenderSettings.bl_rna.properties["engine"].enum_items}
    bpy.context.scene.render.engine = "BLENDER_EEVEE_NEXT" if "BLENDER_EEVEE_NEXT" in available_engines else "BLENDER_EEVEE"
    if hasattr(bpy.context.scene, "eevee"):
        bpy.context.scene.eevee.taa_render_samples = 64
    bpy.context.scene.render.resolution_x = 3000
    bpy.context.scene.render.resolution_y = 900
    bpy.context.scene.render.filepath = str(RENDER_PATH)
    bpy.context.scene.view_settings.view_transform = "Filmic"
    bpy.context.scene.view_settings.look = "Medium High Contrast"
    bpy.context.scene.view_settings.exposure = 0
    bpy.context.scene.view_settings.gamma = 1

    bpy.ops.wm.save_as_mainfile(filepath=str(BLEND_PATH))
    bpy.ops.render.render(write_still=True)


if __name__ == "__main__":
    build_scene()
    print(f"Saved {BLEND_PATH}")
    print(f"Rendered {RENDER_PATH}")
