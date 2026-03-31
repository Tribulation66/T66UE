import os
import shutil

import bpy


OUT_DIR = r"C:\UE\T66\SourceAssets\Import\VFX\Idol_Fire_Explosion\replay_default_probe"

if os.path.isdir(OUT_DIR):
    shutil.rmtree(OUT_DIR)
os.makedirs(OUT_DIR, exist_ok=True)

bpy.ops.object.select_all(action="SELECT")
bpy.ops.object.delete(use_global=False)

scene = bpy.context.scene
scene.frame_start = 1
scene.frame_end = 24
scene.render.engine = "CYCLES"
scene.cycles.samples = 24
scene.cycles.use_adaptive_sampling = True
scene.cycles.volume_bounces = 2
scene.render.resolution_x = 640
scene.render.resolution_y = 640
scene.render.image_settings.file_format = "PNG"
scene.render.image_settings.color_mode = "RGBA"
scene.view_settings.look = "AgX - High Contrast"
scene.view_settings.exposure = 0.2

world = bpy.data.worlds.new("ReplayDefaultWorld")
world.use_nodes = True
world.node_tree.nodes["Background"].inputs[0].default_value = (0.01, 0.01, 0.015, 1.0)
world.node_tree.nodes["Background"].inputs[1].default_value = 0.2
scene.world = world

bpy.ops.mesh.primitive_plane_add(size=16, location=(0.0, 0.0, -0.02))
bpy.ops.mesh.primitive_uv_sphere_add(location=(0.0, 0.0, 0.7), segments=48, ring_count=24)
flow = bpy.context.object

bpy.ops.object.select_all(action="DESELECT")
flow.select_set(True)
bpy.context.view_layer.objects.active = flow
bpy.ops.object.quick_smoke(style="FIRE")

cam = bpy.data.cameras.new("ReplayDefaultCam")
cam_obj = bpy.data.objects.new("ReplayDefaultCam", cam)
bpy.context.collection.objects.link(cam_obj)
cam_obj.location = (0.0, -8.0, 3.0)
cam_obj.rotation_euler = (1.15, 0.0, 0.0)
scene.camera = cam_obj

domain = bpy.data.objects["Smoke Domain"]
domain_mod = next(mod for mod in domain.modifiers if mod.type == "FLUID")
ds = domain_mod.domain_settings

print("CACHE_TYPE", ds.cache_type)
for frame in (1, 2, 4, 8, 12, 16, 20):
    scene.frame_set(frame)
    print(
        "FRAME",
        frame,
        "density_len",
        len(ds.density_grid),
        "flame_len",
        len(ds.flame_grid),
        "temp_len",
        len(ds.temperature_grid),
    )

scene.frame_set(12)
scene.render.filepath = os.path.join(OUT_DIR, "render_f12.png")
print("RENDER", bpy.ops.render.render(write_still=True))
