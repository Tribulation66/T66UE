# Textured QA render of the InflatableTraps01 kit: applies the generated pattern PNGs
# with a rubber principled material and renders two contact sheets.
# Run: blender --background --factory-startup InflatableTraps01.blend --python render_textured_sheet.py
import bpy
import math
import os

RUN = r"C:\UE\T66\Model Generation\Runs\Environment\InflatableTraps01"
TEX = r"C:\UE\T66\Saved\Codex\World\InflatableTraps01\w1_patterns"
RENDER_DIR = os.path.join(RUN, "Renders")

PATTERNS = {
    "SM_Inflatable_SweeperArm": "T_Inflatable_StripesDiag.png",
    "SM_Inflatable_Hub": "T_Inflatable_BandsHoriz.png",
    "SM_Inflatable_Bumper": "T_Inflatable_BandsHoriz.png",
    "SM_Inflatable_Pad": "T_Inflatable_Chevrons.png",
    "SM_Inflatable_Mallet": "T_Inflatable_Stars.png",
    "SM_Inflatable_Tube": "T_Inflatable_BandsHoriz.png",
    "SM_Inflatable_SpikeBall": "T_Inflatable_Dots.png",
}


def make_textured_material(name, image_path):
    mat = bpy.data.materials.new(name)
    mat.use_nodes = True
    nt = mat.node_tree
    bsdf = nt.nodes.get("Principled BSDF")
    tex = nt.nodes.new("ShaderNodeTexImage")
    tex.image = bpy.data.images.load(image_path)
    nt.links.new(tex.outputs["Color"], bsdf.inputs["Base Color"])
    bsdf.inputs["Roughness"].default_value = 0.42
    try:
        bsdf.inputs["Sheen Weight"].default_value = 0.25
        bsdf.inputs["Specular IOR Level"].default_value = 0.35
    except Exception:
        pass
    return mat


for obj_name, png in PATTERNS.items():
    obj = bpy.data.objects.get(obj_name)
    if not obj:
        print(f"[TexturedSheet] MISSING {obj_name}")
        continue
    mat = make_textured_material("TX_" + obj_name, os.path.join(TEX, png))
    obj.data.materials.clear()
    obj.data.materials.append(mat)

# the build script saves the blend with every object parked at the origin for GLB
# export — spread them back out for the sheet
ORDER = [
    "SM_Inflatable_SweeperArm",
    "SM_Inflatable_Hub",
    "SM_Inflatable_Bumper",
    "SM_Inflatable_Pad",
    "SM_Inflatable_Mallet",
    "SM_Inflatable_Tube",
    "SM_Inflatable_SpikeBall",
]
for i, name in enumerate(ORDER):
    obj = bpy.data.objects.get(name)
    if obj:
        obj.location = (i * 1.7 - (len(ORDER) - 1) * 0.85, 0.0, 0.6)

# stronger key light for the texture read
for light in [o for o in bpy.data.objects if o.type == "LIGHT"]:
    light.data.energy = 5.0

scene = bpy.context.scene
scene.render.engine = "BLENDER_EEVEE"
scene.render.resolution_x = 2048
scene.render.resolution_y = 560

cam = bpy.data.objects.get("QACam")
scene.render.filepath = os.path.join(RENDER_DIR, "textured_sheet_front.png")
bpy.ops.render.render(write_still=True)

# second angle: higher, from the other side
if cam:
    cam.location = (2.5, -9.5, 5.2)
    cam.rotation_euler = (math.radians(64), 0.0, math.radians(14))
scene.render.filepath = os.path.join(RENDER_DIR, "textured_sheet_high.png")
bpy.ops.render.render(write_still=True)

# focused pair: sweeper arm + spike ball (cropped in the row sheets)
PAIR = {"SM_Inflatable_SweeperArm": (-1.4, 0.0, 0.6), "SM_Inflatable_SpikeBall": (1.6, 0.0, 0.75)}
for name in ORDER:
    obj = bpy.data.objects.get(name)
    if not obj:
        continue
    if name in PAIR:
        obj.hide_render = False
        obj.location = PAIR[name]
    else:
        obj.hide_render = True
if cam:
    cam.location = (0.6, -6.0, 2.6)
    cam.rotation_euler = (math.radians(70), 0.0, math.radians(6))
scene.render.resolution_x = 1400
scene.render.resolution_y = 720
scene.render.filepath = os.path.join(RENDER_DIR, "textured_arm_spikeball.png")
bpy.ops.render.render(write_still=True)
print("[TexturedSheet] DONE")
