import bpy
bpy.ops.object.select_all(action='SELECT')
bpy.ops.object.delete(use_global=False)
bpy.ops.mesh.primitive_uv_sphere_add(location=(0,0,0.7))
flow = bpy.context.object
bpy.ops.object.select_all(action='DESELECT')
flow.select_set(True)
bpy.context.view_layer.objects.active = flow
bpy.ops.object.quick_smoke(style='FIRE')
domain = bpy.data.objects['Smoke Domain']
mat = domain.active_material
pv = next(node for node in mat.node_tree.nodes if node.bl_idname == 'ShaderNodeVolumePrincipled')
print([sock.name for sock in pv.inputs])
