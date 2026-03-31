import os
import shutil

import bpy
from mathutils import Vector


OUT_DIR = r"C:\UE\T66\SourceAssets\Import\VFX\Idol_Fire_Explosion\domain_scale_preview"

if os.path.isdir(OUT_DIR):
    shutil.rmtree(OUT_DIR)
os.makedirs(OUT_DIR, exist_ok=True)

bpy.ops.object.select_all(action="SELECT")
bpy.ops.object.delete(use_global=False)

scene = bpy.context.scene
scene.frame_start = 1
scene.frame_end = 12
scene.render.engine = "CYCLES"
scene.cycles.samples = 64
scene.cycles.use_adaptive_sampling = True
scene.cycles.volume_bounces = 3
scene.render.resolution_x = 768
scene.render.resolution_y = 768
scene.render.image_settings.file_format = "PNG"
scene.render.image_settings.color_mode = "RGBA"
scene.view_settings.look = "AgX - High Contrast"
scene.view_settings.exposure = 0.10

world = bpy.data.worlds.new("DomainPreviewWorld")
world.use_nodes = True
world.node_tree.nodes["Background"].inputs[0].default_value = (0.008, 0.010, 0.018, 1.0)
world.node_tree.nodes["Background"].inputs[1].default_value = 0.18
scene.world = world

bpy.ops.mesh.primitive_plane_add(size=16, location=(0.0, 0.0, -0.02))
ground = bpy.context.object
ground_mat = bpy.data.materials.new("Ground")
ground_mat.use_nodes = True
ground_bsdf = ground_mat.node_tree.nodes["Principled BSDF"]
ground_bsdf.inputs["Base Color"].default_value = (0.018, 0.02, 0.024, 1.0)
ground_bsdf.inputs["Roughness"].default_value = 0.72
ground.data.materials.append(ground_mat)

cam = bpy.data.cameras.new("CameraHero")
cam_obj = bpy.data.objects.new("CameraHero", cam)
bpy.context.collection.objects.link(cam_obj)
cam_obj.location = (0.0, -5.8, 2.8)
cam_obj.rotation_euler = (1.12, 0.0, 0.0)
scene.camera = cam_obj

key = bpy.data.lights.new("ExplosionKey", "AREA")
key.energy = 1600
key.shape = "RECTANGLE"
key.size = 8.0
key.size_y = 5.0
key_obj = bpy.data.objects.new("ExplosionKey", key)
bpy.context.collection.objects.link(key_obj)
key_obj.location = (0.0, -4.0, 5.2)
key_obj.rotation_euler = (0.95, 0.0, 0.0)

bpy.ops.mesh.primitive_uv_sphere_add(location=(0.0, 0.0, 0.7), segments=48, ring_count=24)
flow = bpy.context.object
for frame, scale in (
    (1, (0.10, 0.10, 0.10)),
    (2, (0.90, 0.90, 0.65)),
    (3, (1.45, 1.45, 0.95)),
    (4, (1.70, 1.70, 1.10)),
    (5, (1.30, 1.30, 0.90)),
    (6, (0.70, 0.70, 0.52)),
    (7, (0.24, 0.24, 0.18)),
    (8, (0.04, 0.04, 0.04)),
    (9, (0.01, 0.01, 0.01)),
):
    flow.scale = scale
    flow.keyframe_insert(data_path="scale", frame=frame)

bpy.ops.object.select_all(action="DESELECT")
flow.select_set(True)
bpy.context.view_layer.objects.active = flow
bpy.ops.object.quick_smoke(style="SMOKE")

domain = bpy.data.objects["Smoke Domain"]
domain.location = (0.0, 0.0, 1.2)
domain.scale = (2.2, 2.2, 2.6)
flow_mod = next(mod for mod in flow.modifiers if mod.type == "FLUID")
domain_mod = next(mod for mod in domain.modifiers if mod.type == "FLUID")
fs = flow_mod.flow_settings
ds = domain_mod.domain_settings

fs.flow_type = "SMOKE"
fs.flow_behavior = "GEOMETRY"
fs.use_initial_velocity = True
fs.velocity_coord = (0.0, 0.0, 2.6)
fs.velocity_normal = 1.8
fs.velocity_random = 0.45
fs.density = 2.4
fs.temperature = 2.3

ds.cache_type = "ALL"
ds.cache_data_format = "OPENVDB"
ds.cache_directory = OUT_DIR
ds.cache_frame_start = 1
ds.cache_frame_end = 12
ds.resolution_max = 64
ds.use_noise = True
ds.use_adaptive_domain = True
ds.adapt_margin = 3
ds.adapt_threshold = 0.01
ds.vorticity = 0.45
if hasattr(ds, "use_dissolve_smoke"):
    ds.use_dissolve_smoke = True
if hasattr(ds, "dissolve_speed"):
    ds.dissolve_speed = 10

mat = domain.active_material
nodes = mat.node_tree.nodes
for node in nodes:
    if node.bl_idname == "ShaderNodeVolumePrincipled":
        node.inputs["Density"].default_value = 5.2
        node.inputs["Density Attribute"].default_value = "density"
        node.inputs["Color"].default_value = (0.09, 0.05, 0.03, 1.0)
        node.inputs["Absorption Color"].default_value = (0.16, 0.08, 0.03, 1.0)
        node.inputs["Blackbody Intensity"].default_value = 2.6
        node.inputs["Temperature"].default_value = 1450.0
        node.inputs["Temperature Attribute"].default_value = "temperature"
        node.inputs["Anisotropy"].default_value = 0.12
        emission_input = node.inputs.get("Emission Strength")
        if emission_input is not None:
            emission_input.default_value = 1.2
        emission_color = node.inputs.get("Emission Color")
        if emission_color is not None:
            emission_color.default_value = (1.0, 0.42, 0.10, 1.0)
        break

bpy.context.view_layer.objects.active = domain
print("BAKE", bpy.ops.fluid.bake_all())
flow.hide_render = True
flow.hide_viewport = True

for frame in (3, 4, 5, 6, 7, 8):
    scene.frame_set(frame)
    scene.render.filepath = os.path.join(OUT_DIR, f"render_f{frame:02d}.png")
    print("RENDER", frame, bpy.ops.render.render(write_still=True))
