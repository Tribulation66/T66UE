import math

import bpy
from mathutils import Euler, Vector


OUT_PATH = r"C:\UE\T66\SourceAssets\Import\VFX\Idol_Fire_Explosion\card_probe.png"


bpy.ops.object.select_all(action="SELECT")
bpy.ops.object.delete(use_global=False)

scene = bpy.context.scene
scene.render.engine = "BLENDER_EEVEE"
scene.render.resolution_x = 512
scene.render.resolution_y = 512
scene.render.image_settings.file_format = "PNG"
scene.render.image_settings.color_mode = "RGBA"
scene.render.film_transparent = True
scene.view_settings.look = "AgX - High Contrast"
scene.view_settings.exposure = 0.05

world = bpy.data.worlds.new("ProbeWorld")
world.use_nodes = True
world.node_tree.nodes["Background"].inputs[1].default_value = 0.0
scene.world = world

cam_data = bpy.data.cameras.new("ProbeCamera")
cam = bpy.data.objects.new("ProbeCamera", cam_data)
bpy.context.collection.objects.link(cam)
cam.location = (0.0, -6.0, 4.3)
direction = Vector((0.0, 0.0, 1.15)) - cam.location
cam.rotation_euler = direction.to_track_quat("-Z", "Y").to_euler()
scene.camera = cam

controller = bpy.data.objects.new("Controller", None)
bpy.context.collection.objects.link(controller)

bpy.ops.mesh.primitive_plane_add(size=2.1, location=(0.0, 0.0, 1.2))
plane = bpy.context.object
plane.rotation_euler = Euler(cam.rotation_euler, "XYZ")
plane.rotation_euler.rotate_axis("Z", math.radians(20.0))

material = bpy.data.materials.new("ProbeFireCard")
material.use_nodes = True
material.blend_method = "BLEND"
nodes = material.node_tree.nodes
links = material.node_tree.links
nodes.clear()

output = nodes.new("ShaderNodeOutputMaterial")
output.location = (900, 0)
mix = nodes.new("ShaderNodeMixShader")
mix.location = (680, 0)
transparent = nodes.new("ShaderNodeBsdfTransparent")
transparent.location = (480, -120)
emission = nodes.new("ShaderNodeEmission")
emission.location = (480, 120)
emission.inputs["Strength"].default_value = 8.0

tex_coord = nodes.new("ShaderNodeTexCoord")
tex_coord.location = (-1200, 20)
mapping = nodes.new("ShaderNodeMapping")
mapping.location = (-980, 20)
mapping.inputs["Location"].default_value = (-0.5, -0.5, 0.0)
noise_a = nodes.new("ShaderNodeTexNoise")
noise_a.location = (-760, 170)
noise_a.noise_dimensions = "4D"
noise_a.inputs["Scale"].default_value = 4.8
noise_a.inputs["Detail"].default_value = 10.0
noise_a.inputs["Roughness"].default_value = 0.6
noise_b = nodes.new("ShaderNodeTexNoise")
noise_b.location = (-760, -40)
noise_b.noise_dimensions = "4D"
noise_b.inputs["Scale"].default_value = 10.5
noise_b.inputs["Detail"].default_value = 4.0
noise_b.inputs["Roughness"].default_value = 0.45
gradient = nodes.new("ShaderNodeTexGradient")
gradient.location = (-760, 330)
gradient.gradient_type = "SPHERICAL"
radial_ramp = nodes.new("ShaderNodeValToRGB")
radial_ramp.location = (-520, 320)
radial_ramp.color_ramp.elements[0].position = 0.26
radial_ramp.color_ramp.elements[1].position = 0.86
combine_noise = nodes.new("ShaderNodeMath")
combine_noise.location = (-520, 70)
combine_noise.operation = "MULTIPLY"
combine_noise.use_clamp = True
alpha_mix = nodes.new("ShaderNodeMath")
alpha_mix.location = (-260, 120)
alpha_mix.operation = "MULTIPLY"
alpha_mix.use_clamp = True
alpha_ramp = nodes.new("ShaderNodeValToRGB")
alpha_ramp.location = (-20, 0)
alpha_ramp.color_ramp.elements[0].position = 0.36
alpha_ramp.color_ramp.elements[1].position = 0.84
fire_ramp = nodes.new("ShaderNodeValToRGB")
fire_ramp.location = (-20, 200)
fire_ramp.color_ramp.elements[0].position = 0.0
fire_ramp.color_ramp.elements[0].color = (0.00, 0.00, 0.00, 1.0)
fire_ramp.color_ramp.elements[1].position = 1.0
fire_ramp.color_ramp.elements[1].color = (1.0, 0.84, 0.42, 1.0)
for position, color in (
    (0.18, (0.14, 0.00, 0.00, 1.0)),
    (0.40, (0.62, 0.03, 0.00, 1.0)),
    (0.62, (0.98, 0.18, 0.00, 1.0)),
    (0.82, (1.0, 0.48, 0.05, 1.0)),
):
    element = fire_ramp.color_ramp.elements.new(position)
    element.color = color
radial_color = nodes.new("ShaderNodeSeparateColor")
radial_color.location = (-260, 320)
alpha_color = nodes.new("ShaderNodeSeparateColor")
alpha_color.location = (220, 0)

links.new(tex_coord.outputs["UV"], mapping.inputs["Vector"])
links.new(mapping.outputs["Vector"], noise_a.inputs["Vector"])
links.new(mapping.outputs["Vector"], noise_b.inputs["Vector"])
links.new(mapping.outputs["Vector"], gradient.inputs["Vector"])
links.new(gradient.outputs["Fac"], radial_ramp.inputs["Fac"])
links.new(noise_a.outputs["Fac"], combine_noise.inputs[0])
links.new(noise_b.outputs["Fac"], combine_noise.inputs[1])
links.new(combine_noise.outputs["Value"], alpha_mix.inputs[0])
links.new(radial_ramp.outputs["Color"], radial_color.inputs["Color"])
links.new(radial_color.outputs["Red"], alpha_mix.inputs[1])
links.new(alpha_mix.outputs["Value"], alpha_ramp.inputs["Fac"])
links.new(alpha_mix.outputs["Value"], fire_ramp.inputs["Fac"])
links.new(alpha_ramp.outputs["Color"], alpha_color.inputs["Color"])
links.new(alpha_color.outputs["Red"], mix.inputs["Fac"])
links.new(fire_ramp.outputs["Color"], emission.inputs["Color"])
links.new(emission.outputs["Emission"], mix.inputs[2])
links.new(transparent.outputs["BSDF"], mix.inputs[1])
links.new(mix.outputs["Shader"], output.inputs["Surface"])

plane.data.materials.append(material)
scene.render.filepath = OUT_PATH
bpy.ops.render.render(write_still=True)
print(OUT_PATH)
