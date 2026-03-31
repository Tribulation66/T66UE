import bpy

bpy.ops.object.select_all(action="SELECT")
bpy.ops.object.delete(use_global=False)

bpy.ops.mesh.primitive_uv_sphere_add(location=(0, 0, 1))
flow = bpy.context.object
flow.name = "Flow"

bpy.ops.object.select_all(action="DESELECT")
flow.select_set(True)
bpy.context.view_layer.objects.active = flow

print("quick_smoke poll start")
result = bpy.ops.object.quick_smoke()
print("quick_smoke result", result)

print("ALL_OBJECTS", [obj.name for obj in bpy.data.objects])

for obj in bpy.data.objects:
    print("OBJECT", obj.name)
    for mod in obj.modifiers:
        print("  MOD", mod.name, mod.type)
        if hasattr(mod, "fluid_type"):
            print("    fluid_type", mod.fluid_type)
        if hasattr(mod, "domain_settings") and mod.domain_settings:
            ds = mod.domain_settings
            print("    domain_settings type", ds.domain_type if hasattr(ds, "domain_type") else "n/a")
            attrs = [
                "resolution_max",
                "use_noise",
                "cache_type",
                "cache_data_format",
                "cache_mesh_format",
                "cache_directory",
                "beta",
                "vorticity",
                "flame_vorticity",
                "burning_rate",
                "flame_smoke",
                "flame_ignition",
                "flame_max_temp",
                "adaptivity",
            ]
            for attr in attrs:
                if hasattr(ds, attr):
                    try:
                        print("   ", attr, getattr(ds, attr))
                    except Exception as exc:
                        print("   ", attr, "ERR", exc)
        if hasattr(mod, "flow_settings") and mod.flow_settings:
            fs = mod.flow_settings
            attrs = [
                "flow_type",
                "flow_behavior",
                "surface_distance",
                "use_initial_velocity",
                "temperature",
                "density",
                "fuel_amount",
                "subframes",
            ]
            for attr in attrs:
                if hasattr(fs, attr):
                    try:
                        print("   ", attr, getattr(fs, attr))
                    except Exception as exc:
                        print("   ", attr, "ERR", exc)
