from pathlib import Path

import bpy

LOOKDEV_ROOT = Path(
    r"C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\LookDev\Hero_1_Chad_Male_Rubber_20260605"
)
RENDER_ROOT = LOOKDEV_ROOT / "Renders"
GRID_PATH = RENDER_ROOT / "Hero_1_Chad_Male_rubber_comparison_grid.png"
PREVIEW_PATH = RENDER_ROOT / "Hero_1_Chad_Male_rubber_comparison_grid_preview_1300.png"
BLEND_PATH = LOOKDEV_ROOT / "Hero_1_Chad_Male_RubberLookDev.blend"

for obj in bpy.data.objects:
    if obj.type == "FONT" and obj.data.body == "Raw Pixel3D GLB":
        obj.data.body = "Raw Pixal3D GLB"

scene = bpy.data.scenes.get("Rubber_Comparison_Grid") or bpy.context.scene
if bpy.context.window:
    bpy.context.window.scene = scene

camera = bpy.data.objects.get("Camera_Comparison_Ortho")
if camera:
    scene.camera = camera

scene.frame_set(1)
scene.render.image_settings.file_format = "PNG"
scene.frame_start = 1
scene.frame_end = 1

scene.render.resolution_x = 2600
scene.render.resolution_y = 1500
scene.render.resolution_percentage = 100
scene.render.filepath = str(GRID_PATH)
bpy.ops.render.render(write_still=True, scene=scene.name)

scene.render.resolution_x = 1300
scene.render.resolution_y = 750
scene.render.resolution_percentage = 100
scene.render.filepath = str(PREVIEW_PATH)
bpy.ops.render.render(write_still=True, scene=scene.name)

bpy.ops.wm.save_as_mainfile(filepath=str(BLEND_PATH))

