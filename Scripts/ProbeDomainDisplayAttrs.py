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
ds = next(mod for mod in domain.modifiers if mod.type == 'FLUID').domain_settings
for name in sorted(dir(ds)):
    if any(token in name.lower() for token in ['display','render','slice','show','color','flame','temperature','density']):
        try:
            print(name, getattr(ds, name))
        except Exception:
            print(name, '<err>')
