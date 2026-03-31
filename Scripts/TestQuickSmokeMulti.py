import bpy
bpy.ops.object.select_all(action='SELECT')
bpy.ops.object.delete(use_global=False)
for i, loc in enumerate([(0,0,0.5),(1,0,0.5),(2,0,0.5)]):
    bpy.ops.mesh.primitive_uv_sphere_add(location=loc)
    bpy.context.object.name = f'Flow{i}'
for obj in bpy.data.objects:
    if obj.type == 'MESH':
        obj.select_set(True)
        bpy.context.view_layer.objects.active = obj
print('before', [obj.name for obj in bpy.data.objects])
res = bpy.ops.object.quick_smoke()
print('res', res)
print('after', [obj.name for obj in bpy.data.objects])
for obj in bpy.data.objects:
    mods = [(m.type, getattr(m,'fluid_type',None)) for m in obj.modifiers]
    if mods:
        print(obj.name, mods)
