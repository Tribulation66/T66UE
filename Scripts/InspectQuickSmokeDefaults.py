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
ds = next(mod for mod in domain.modifiers if mod.type == 'FLUID').domain_settings
print('domain settings extra')
for name in ['cache_type','cache_data_format','cache_directory','use_adaptive_domain','use_noise','noise_scale','sndparticle_combined_export','use_color_ramp','color_ramp_field','alpha','beta','dissolve_speed','use_dissolve_smoke','vorticity','flame_vorticity','flame_smoke','burning_rate','flame_ignition','flame_max_temp']:
    if hasattr(ds, name):
        try:
            print(name, getattr(ds, name))
        except Exception as e:
            print(name, 'ERR', e)
mat = domain.active_material
print('mat', mat.name if mat else None)
if mat:
    for node in mat.node_tree.nodes:
        print('node', node.bl_idname, node.name)
        if node.bl_idname == 'ShaderNodeVolumePrincipled':
            for sock in node.inputs:
                try:
                    print('  ', sock.name, sock.default_value)
                except Exception:
                    pass
