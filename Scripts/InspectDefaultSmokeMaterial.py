import bpy
bpy.ops.object.select_all(action='SELECT')
bpy.ops.object.delete(use_global=False)
bpy.ops.mesh.primitive_uv_sphere_add(location=(0,0,0.7))
flow = bpy.context.object
bpy.ops.object.select_all(action='DESELECT')
flow.select_set(True)
bpy.context.view_layer.objects.active = flow
bpy.ops.object.quick_smoke()
domain = bpy.data.objects['Smoke Domain']
mat = domain.active_material
print('MAT', mat.name)
node = next(n for n in mat.node_tree.nodes if n.bl_idname == 'ShaderNodeVolumePrincipled')
for sock in node.inputs:
    try:
        val = sock.default_value
    except Exception:
        val = '<no default>'
    print(sock.name, val)
