import bpy


VDB_PATH = r"C:\UE\T66\SourceAssets\Import\VFX\Idol_Fire_Explosion\smoke_only_scale_probe\data\fluid_data_0004.vdb"
OUT_PATH = r"C:\UE\T66\SourceAssets\Import\VFX\Idol_Fire_Explosion\smoke_only_scale_probe\import_constant_render_f04.png"

bpy.ops.object.select_all(action="SELECT")
bpy.ops.object.delete(use_global=False)

scene = bpy.context.scene
scene.render.engine = "CYCLES"
scene.cycles.samples = 24
scene.cycles.volume_bounces = 2
scene.render.resolution_x = 768
scene.render.resolution_y = 768
scene.render.image_settings.file_format = "PNG"

bpy.ops.object.volume_import(filepath=VDB_PATH)
vol = bpy.context.object
vol.location = (0.0, 0.0, 1.2)
vol.scale = (2.2, 2.2, 2.6)
vol.data.grids.load()

cam = bpy.data.cameras.new("Cam")
cam_obj = bpy.data.objects.new("Cam", cam)
bpy.context.collection.objects.link(cam_obj)
cam_obj.location = (0.0, -5.8, 2.8)
cam_obj.rotation_euler = (1.12, 0.0, 0.0)
scene.camera = cam_obj

mat = bpy.data.materials.new("M")
mat.use_nodes = True
nodes = mat.node_tree.nodes
links = mat.node_tree.links
nodes.clear()
out = nodes.new("ShaderNodeOutputMaterial")
pv = nodes.new("ShaderNodeVolumePrincipled")
pv.inputs["Density"].default_value = 2.0
pv.inputs["Emission Strength"].default_value = 0.0
links.new(pv.outputs["Volume"], out.inputs["Volume"])
vol.data.materials.clear()
vol.data.materials.append(mat)

scene.render.filepath = OUT_PATH
print("RENDER", bpy.ops.render.render(write_still=True))
