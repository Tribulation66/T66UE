import math
from pathlib import Path
import sys

import bpy
from mathutils import Vector


RUN_ROOT = Path(r"C:\UE\T66\Model Generation\Runs\Pixal3D\HeroChadStacy_SourceAssets_20260609_0536")
REPO_ROOT = Path(r"C:\UE\T66")
BLENDER_CORE = REPO_ROOT / "Model Generation" / "Scripts" / "Core" / "Blender"
if str(BLENDER_CORE) not in sys.path:
    sys.path.insert(0, str(BLENDER_CORE))

from t66_unreal_friend_slop_preview import apply_unreal_friend_slop_preview

CURRENT_HERO1_MALE = Path(
    r"C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Outputs\Hero_1_Chad_Male.glb"
)
PROJECTILE_RUN_ROOT = Path(
    r"C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProjectiles_20260609_0659"
)
IDOL_PROJECTILE = PROJECTILE_RUN_ROOT / "Outputs" / "IdolProjectile.glb"
WEAPON_PROJECTILE = PROJECTILE_RUN_ROOT / "Outputs" / "WeaponProjectile.glb"
UNREAL_UNLIT_MODELS = [
    ("CurrentHero1Male_UnrealUnlit", CURRENT_HERO1_MALE, -12.8, 4.2, "height"),
    ("Hero1Stacy_UnrealUnlit", RUN_ROOT / "Outputs" / "Hero1Stacy.glb", -9.7, 4.2, "height"),
    ("Hero2Chad_UnrealUnlit", RUN_ROOT / "Outputs" / "Hero2Chad.glb", -6.4, 4.2, "height"),
    ("IdolProjectile_UnrealUnlit", IDOL_PROJECTILE, -3.6, 2.4, "max"),
    ("WeaponProjectile_UnrealUnlit", WEAPON_PROJECTILE, -1.1, 2.4, "max"),
]
PREVIOUS_BLENDER_LIGHT_MODELS = [
    ("CurrentHero1Male_PreviousBlenderLight", CURRENT_HERO1_MALE, 2.4, 4.2, "height"),
    ("Hero1Stacy_PreviousBlenderLight", RUN_ROOT / "Outputs" / "Hero1Stacy.glb", 5.5, 4.2, "height"),
    ("Hero2Chad_PreviousBlenderLight", RUN_ROOT / "Outputs" / "Hero2Chad.glb", 8.8, 4.2, "height"),
    ("IdolProjectile_PreviousBlenderLight", IDOL_PROJECTILE, 11.6, 2.4, "max"),
    ("WeaponProjectile_PreviousBlenderLight", WEAPON_PROJECTILE, 14.1, 2.4, "max"),
]
SCENE_PATH = RUN_ROOT / "Blender" / "HeroChadStacy_with_current_hero1_male.blend"
RENDER_PATH = RUN_ROOT / "QA" / "HeroChadStacy_with_current_hero1_male.png"


def reset_scene():
    bpy.ops.wm.read_homefile(use_empty=True, use_factory_startup=True)
    scene = bpy.context.scene
    scene.render.resolution_x = 3200
    scene.render.resolution_y = 1200
    scene.render.image_settings.file_format = "PNG"
    return scene


def imported_objects_after(path):
    before = set(bpy.data.objects)
    bpy.ops.import_scene.gltf(filepath=str(path))
    imported = [obj for obj in bpy.data.objects if obj not in before]
    meshes = [obj for obj in imported if obj.type == "MESH"]
    if not meshes:
        raise RuntimeError(f"No mesh objects imported from {path}")
    return imported, meshes


def world_bbox(meshes):
    mins = Vector((math.inf, math.inf, math.inf))
    maxs = Vector((-math.inf, -math.inf, -math.inf))
    for obj in meshes:
        for corner in obj.bound_box:
            world = obj.matrix_world @ Vector(corner)
            mins.x = min(mins.x, world.x)
            mins.y = min(mins.y, world.y)
            mins.z = min(mins.z, world.z)
            maxs.x = max(maxs.x, world.x)
            maxs.y = max(maxs.y, world.y)
            maxs.z = max(maxs.z, world.z)
    return mins, maxs


def add_model(label, path, x_position, target_size=4.2, scale_mode="height"):
    imported, meshes = imported_objects_after(path)
    empty = bpy.data.objects.new(f"{label}_ReviewRoot", None)
    bpy.context.scene.collection.objects.link(empty)
    roots = [obj for obj in imported if obj.parent is None]
    for obj in roots:
        obj.parent = empty

    mins, maxs = world_bbox(meshes)
    center = (mins + maxs) * 0.5
    size = maxs - mins
    height = max(size.z, 0.001)
    if scale_mode == "max":
        scale_basis = max(size.x, size.y, size.z, 0.001)
    else:
        scale_basis = height
    scale = target_size / scale_basis

    empty.scale = (scale, scale, scale)
    empty.location = (x_position - center.x * scale, -center.y * scale, -mins.z * scale)

    return empty


def scene_mesh_bounds():
    meshes = [obj for obj in bpy.data.objects if obj.type == "MESH"]
    return world_bbox(meshes)


def add_camera():
    mins, maxs = scene_mesh_bounds()
    center = (mins + maxs) * 0.5
    size = maxs - mins
    cam_data = bpy.data.cameras.new(name="ReviewCamera")
    cam_data.type = "ORTHO"
    aspect = bpy.context.scene.render.resolution_x / bpy.context.scene.render.resolution_y
    cam_data.ortho_scale = max(size.z * 2.05, (size.x / aspect) * 1.95, 8.0)
    cam = bpy.data.objects.new("ReviewCamera", cam_data)
    cam.location = (center.x, center.y + 10.5, center.z)
    bpy.context.scene.collection.objects.link(cam)
    bpy.context.scene.camera = cam
    target = bpy.data.objects.new("ReviewCameraTarget", None)
    target.location = center
    bpy.context.scene.collection.objects.link(target)
    track = cam.constraints.new(type="TRACK_TO")
    track.target = target
    track.track_axis = "TRACK_NEGATIVE_Z"
    track.up_axis = "UP_Y"


def add_previous_blender_light_rig():
    sun_data = bpy.data.lights.new("ReviewSun", "SUN")
    sun_data.energy = 2.1
    sun = bpy.data.objects.new("ReviewSun", sun_data)
    sun.rotation_euler = (math.radians(42), 0.0, math.radians(28))
    bpy.context.scene.collection.objects.link(sun)

    fill_data = bpy.data.lights.new("ReviewFill", "AREA")
    fill_data.energy = 3200
    fill_data.size = 5.0
    fill = bpy.data.objects.new("ReviewFill", fill_data)
    fill.location = (1.2, -5.5, 5.0)
    fill.rotation_euler = (math.radians(60), 0.0, math.radians(12))
    bpy.context.scene.collection.objects.link(fill)


def force_model_materials_opaque(root):
    for obj in root.children_recursive:
        if obj.type != "MESH":
            continue
        for slot in obj.material_slots:
            material = slot.material
            if material is None:
                continue
            material.blend_method = "OPAQUE"
            material.use_screen_refraction = False
            material.diffuse_color[3] = 1.0
            if not material.use_nodes:
                continue
            for node in material.node_tree.nodes:
                if node.bl_idname != "ShaderNodeBsdfPrincipled":
                    continue
                alpha_input = node.inputs.get("Alpha")
                if alpha_input is None:
                    continue
                for link in list(alpha_input.links):
                    material.node_tree.links.remove(link)
                alpha_input.default_value = 1.0


def hide_t66_softbox_lights_for_render():
    for obj in bpy.data.objects:
        if obj.type == "LIGHT" and obj.name.startswith("T66_"):
            obj.hide_render = True
            obj.hide_viewport = True


def main():
    scene = reset_scene()
    for label, path, x_position, target_size, scale_mode in UNREAL_UNLIT_MODELS:
        add_model(label, path, x_position, target_size, scale_mode)
    bpy.context.view_layer.update()
    preview_report = apply_unreal_friend_slop_preview(scene)
    hide_t66_softbox_lights_for_render()
    for label, path, x_position, target_size, scale_mode in PREVIOUS_BLENDER_LIGHT_MODELS:
        root = add_model(label, path, x_position, target_size, scale_mode)
        force_model_materials_opaque(root)
    add_previous_blender_light_rig()
    bpy.context.view_layer.update()
    add_camera()
    RENDER_PATH.parent.mkdir(parents=True, exist_ok=True)
    SCENE_PATH.parent.mkdir(parents=True, exist_ok=True)
    scene["T66_UnrealPreview_Report"] = str(preview_report)
    scene["T66_Comparison_Setup"] = (
        "Left five: Unreal M_GLB_Unlit/emission preview. "
        "Right five: default Blender GLB materials with previous ReviewSun/ReviewFill lighting."
    )
    scene.render.filepath = str(RENDER_PATH)
    bpy.ops.render.render(write_still=True)
    bpy.ops.wm.save_as_mainfile(filepath=str(SCENE_PATH))


if __name__ == "__main__":
    main()
