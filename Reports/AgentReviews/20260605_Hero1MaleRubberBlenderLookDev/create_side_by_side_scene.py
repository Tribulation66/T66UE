from pathlib import Path
import importlib.util

import bpy

BUILD_SCRIPT = Path(
    r"C:\UE\T66\Reports\AgentReviews\20260605_Hero1MaleRubberBlenderLookDev\build_hero1_male_rubber_lookdev.py"
)
BLEND_PATH = Path(
    r"C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\LookDev\Hero_1_Chad_Male_Rubber_20260605\Hero_1_Chad_Male_RubberLookDev.blend"
)
SCENE_NAME = "SideBySide_Raw_vs_V04"

spec = importlib.util.spec_from_file_location("rubber_lookdev_build", BUILD_SCRIPT)
build = importlib.util.module_from_spec(spec)
spec.loader.exec_module(build)

old_scene = bpy.data.scenes.get(SCENE_NAME)
if old_scene:
    bpy.data.scenes.remove(old_scene, do_unlink=True)

scene = bpy.data.scenes.new(SCENE_NAME)
try:
    bpy.context.window.scene = scene
except Exception:
    pass

template_collection = bpy.data.collections.get("Source_Template_Hidden")
if not template_collection:
    raise RuntimeError("Source_Template_Hidden collection was not found in the look-dev blend.")

template_objects = [obj for obj in template_collection.objects if obj.type == "MESH"]
if not template_objects:
    raise RuntimeError("No mesh objects found in Source_Template_Hidden.")

v04 = next(v for v in build.VARIATIONS if v["id"] == "V04_candy_rubber")

with bpy.context.temp_override(scene=scene, view_layer=scene.view_layers[0]):
    build.set_render_defaults(scene, (1800, 1050), 1)
    build.add_flat_lighting(scene, center=(0.0, 0.0, 1.45))
    build.make_backdrop("SideBySide_Backdrop", 6.2, 3.7, 1.55)

    build.duplicate_group(
        template_objects,
        "SideBySide_Raw_Pixal3D_GLB",
        (-1.45, 0.0, 0.0),
        raw=True,
        target_height=2.65,
    )
    build.make_text("Raw Pixal3D GLB", (-1.45, -0.05, 2.92), size=0.13)

    build.duplicate_group(
        template_objects,
        "SideBySide_V04_Candy_Rubber",
        (1.45, 0.0, 0.0),
        material_builder=build.make_rubber_material,
        variation=v04,
        target_height=2.65,
    )
    build.make_text("V04 Candy Rubber", (1.45, -0.05, 2.92), size=0.13)

    build.add_camera(scene, "Camera_SideBySide_Ortho", (0.0, -7.2, 1.55), (0.0, 0.0, 1.42), 4.25)

scene.camera = bpy.data.objects["Camera_SideBySide_Ortho"]
bpy.ops.wm.save_as_mainfile(filepath=str(BLEND_PATH))
print(f"T66_SIDE_BY_SIDE_SCENE_READY={SCENE_NAME}")

