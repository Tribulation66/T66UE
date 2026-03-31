import bpy, os, shutil
from mathutils import Vector
out = r'C:\UE\T66\SourceAssets\Import\VFX\Idol_Fire_Explosion\procedural_probe'
if os.path.isdir(out): shutil.rmtree(out)
os.makedirs(out, exist_ok=True)
bpy.ops.object.select_all(action='SELECT')
bpy.ops.object.delete(use_global=False)
scene = bpy.context.scene
scene.render.engine='CYCLES'
scene.cycles.samples=64
scene.render.resolution_x=768
scene.render.resolution_y=768
scene.render.image_settings.file_format='PNG'
world = bpy.data.worlds.new('W')
world.use_nodes=True
world.node_tree.nodes['Background'].inputs[0].default_value=(0.01,0.01,0.015,1)
world.node_tree.nodes['Background'].inputs[1].default_value=0.2
scene.world = world
bpy.ops.mesh.primitive_plane_add(size=16, location=(0,0,-0.02))
plane = bpy.context.object
matg = bpy.data.materials.new('Ground')
matg.use_nodes=True
pbsdf=matg.node_tree.nodes['Principled BSDF']
pbsdf.inputs['Base Color'].default_value=(0.02,0.02,0.025,1)
plane.data.materials.append(matg)
bpy.ops.mesh.primitive_ico_sphere_add(subdivisions=4, radius=1.0, location=(0,0,1.1))
obj=bpy.context.object
obj.scale=(1.2,1.2,1.0)
mat=bpy.data.materials.new('Vol')
mat.use_nodes=True
nodes=mat.node_tree.nodes
links=mat.node_tree.links
nodes.clear()
outn=nodes.new('ShaderNodeOutputMaterial')
outn.location=(500,0)
texcoord=nodes.new('ShaderNodeTexCoord')
texcoord.location=(-1200,0)
mapn=nodes.new('ShaderNodeMapping')
mapn.location=(-1000,0)
noise=nodes.new('ShaderNodeTexNoise')
noise.location=(-780,0)
noise.noise_dimensions='4D'
noise.inputs['Scale'].default_value=3.0
noise.inputs['Detail'].default_value=8.0
noise.inputs['Roughness'].default_value=0.55
ramp=nodes.new('ShaderNodeValToRGB')
ramp.location=(-560,0)
ramp.color_ramp.elements[0].position=0.35
ramp.color_ramp.elements[1].position=0.82
vectormath=nodes.new('ShaderNodeVectorMath')
vectormath.operation='LENGTH'
vectormath.location=(-780,-260)
maprange=nodes.new('ShaderNodeMapRange')
maprange.location=(-560,-260)
maprange.inputs['From Min'].default_value=0.0
maprange.inputs['From Max'].default_value=1.8
maprange.inputs['To Min'].default_value=1.0
maprange.inputs['To Max'].default_value=0.0
mult=nodes.new('ShaderNodeMath')
mult.location=(-330,-120)
mult.operation='MULTIPLY'
pr=nodes.new('ShaderNodeVolumePrincipled')
pr.location=(200,0)
pr.inputs['Color'].default_value=(0.18,0.11,0.08,1)
pr.inputs['Density'].default_value=4.0
pr.inputs['Absorption Color'].default_value=(0.35,0.12,0.04,1)
pr.inputs['Emission Strength'].default_value=6.0
pr.inputs['Emission Color'].default_value=(1.0,0.45,0.09,1)
pr.inputs['Blackbody Intensity'].default_value=1.2
pr.inputs['Temperature'].default_value=1400.0
links.new(texcoord.outputs['Object'], mapn.inputs['Vector'])
links.new(mapn.outputs['Vector'], noise.inputs['Vector'])
links.new(texcoord.outputs['Object'], vectormath.inputs[0])
links.new(noise.outputs['Fac'], ramp.inputs['Fac'])
links.new(vectormath.outputs['Value'], maprange.inputs['Value'])
links.new(ramp.outputs['Alpha'], mult.inputs[0])
links.new(maprange.outputs['Result'], mult.inputs[1])
links.new(mult.outputs['Value'], pr.inputs['Density'])
links.new(pr.outputs['Volume'], outn.inputs['Volume'])
obj.data.materials.append(mat)
cam=bpy.data.cameras.new('Cam')
cam_obj=bpy.data.objects.new('Cam', cam)
bpy.context.collection.objects.link(cam_obj)
cam_obj.location=(0,-7.2,3.1)
direction=Vector((0,0,1.2))-cam_obj.location
cam_obj.rotation_euler=direction.to_track_quat('-Z','Y').to_euler()
scene.camera=cam_obj
light_data=bpy.data.lights.new('Key','AREA')
light_data.energy=1000
light_data.shape='RECTANGLE'
light_data.size=6
light=bpy.data.objects.new('Key', light_data)
bpy.context.collection.objects.link(light)
light.location=(0,-4.5,4.5)
ldir=Vector((0,0,1.2))-light.location
light.rotation_euler=ldir.to_track_quat('-Z','Y').to_euler()
scene.render.filepath=os.path.join(out,'probe.png')
print(bpy.ops.render.render(write_still=True))
