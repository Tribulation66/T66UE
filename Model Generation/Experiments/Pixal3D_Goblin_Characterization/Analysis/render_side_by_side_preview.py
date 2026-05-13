import bpy
from pathlib import Path


ROOT = Path(r"C:\UE\T66\Model Generation\Experiments\Pixal3D_Goblin_Characterization")


scene = bpy.context.scene
scene.camera = bpy.data.objects["SideBySide_Camera"]
scene.render.engine = "BLENDER_EEVEE"
scene.render.resolution_x = 1600
scene.render.resolution_y = 900
scene.render.image_settings.file_format = "PNG"
scene.render.image_settings.color_mode = "RGBA"
scene.render.film_transparent = False
scene.render.filepath = str(ROOT / "Renders" / "SideBySide_Verification.png")
bpy.ops.render.render(write_still=True)
