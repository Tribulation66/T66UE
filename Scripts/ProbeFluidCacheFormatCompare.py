import os
import shutil
import sys

import bpy


ROOT = r"C:\UE\T66\SourceAssets\Import\VFX\Idol_Fire_Explosion\format_compare"
CASES = (
    ("UNI", "ALL"),
    ("OPENVDB", "ALL"),
    ("UNI", "MODULAR"),
    ("OPENVDB", "MODULAR"),
)


def reset_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)


def setup_scene(out_dir: str, cache_format: str, cache_type: str):
    reset_scene()
    if os.path.isdir(out_dir):
        shutil.rmtree(out_dir)
    os.makedirs(out_dir, exist_ok=True)

    scene = bpy.context.scene
    scene.frame_start = 1
    scene.frame_end = 12
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

    bpy.ops.mesh.primitive_plane_add(size=16, location=(0.0, 0.0, -0.02))
    plane = bpy.context.object
    ground_mat = bpy.data.materials.new(f"Ground_{cache_format}")
    ground_mat.use_nodes = True
    ground_bsdf = ground_mat.node_tree.nodes["Principled BSDF"]
    ground_bsdf.inputs["Base Color"].default_value = (0.02, 0.02, 0.025, 1.0)
    plane.data.materials.append(ground_mat)

    bpy.ops.mesh.primitive_uv_sphere_add(location=(0.0, 0.0, 0.7), segments=48, ring_count=24)
    flow = bpy.context.object
    flow.name = f"Flow_{cache_format}"
    flow.scale = (0.8, 0.8, 0.8)

    bpy.ops.object.select_all(action="DESELECT")
    flow.select_set(True)
    bpy.context.view_layer.objects.active = flow
    bpy.ops.object.quick_smoke(style="FIRE")

    domain = bpy.data.objects["Smoke Domain"]
    domain.name = f"Domain_{cache_format}"
    domain.location.z = 1.2
    domain.scale = (2.2, 2.2, 2.6)

    flow_mod = next(mod for mod in flow.modifiers if mod.type == "FLUID")
    domain_mod = next(mod for mod in domain.modifiers if mod.type == "FLUID")
    fs = flow_mod.flow_settings
    ds = domain_mod.domain_settings

    fs.flow_type = "BOTH"
    fs.flow_behavior = "GEOMETRY"
    fs.use_initial_velocity = True
    fs.velocity_coord = (0.0, 0.0, 1.2)
    fs.velocity_normal = 1.0
    fs.velocity_random = 0.2
    fs.surface_distance = 0.0
    fs.fuel_amount = 1.6
    fs.temperature = 1.6
    fs.density = 0.9

    ds.cache_type = cache_type
    ds.cache_data_format = cache_format
    ds.cache_directory = out_dir
    ds.cache_frame_start = 1
    ds.cache_frame_end = 12
    ds.resolution_max = 64
    ds.use_noise = True
    ds.vorticity = 0.4
    ds.flame_vorticity = 1.2
    ds.burning_rate = 1.0
    ds.flame_smoke = 0.6
    ds.flame_ignition = 1.0
    ds.flame_max_temp = 3.0

    cam = bpy.data.cameras.new(f"Cam_{cache_format}")
    cam_obj = bpy.data.objects.new(f"Cam_{cache_format}", cam)
    bpy.context.collection.objects.link(cam_obj)
    cam_obj.location = (0.0, -8.0, 3.0)
    cam_obj.rotation_euler = (1.15, 0.0, 0.0)
    scene.camera = cam_obj

    bpy.context.view_layer.objects.active = domain
    return scene, flow, domain, ds


def summarize_cache(out_dir: str):
    files = []
    for root, _, names in os.walk(out_dir):
        for name in names:
            path = os.path.join(root, name)
            files.append((path, os.path.getsize(path)))
    files.sort()
    return files


def maybe_report_vdb(out_dir: str):
    if "openvdb" not in sys.modules:
        try:
            import openvdb  # noqa: F401
        except Exception as exc:
            print("OPENVDB_IMPORT_FAILED", exc)
            return

    import openvdb

    data_dir = os.path.join(out_dir, "data")
    sample = os.path.join(data_dir, "fluid_data_0006.vdb")
    if not os.path.isfile(sample):
        print("OPENVDB_SAMPLE_MISSING", sample)
        return
    grids, _ = openvdb.readAll(sample)
    for grid in grids:
        print(
            "OPENVDB_GRID",
            grid.name,
            "active",
            grid.activeVoxelCount(),
            "leaf",
            grid.activeLeafVoxelCount(),
            "minmax",
            grid.evalMinMax(),
        )


def run_case(cache_format: str, cache_type: str):
    out_dir = os.path.join(ROOT, f"{cache_format.lower()}_{cache_type.lower()}")
    scene, flow, domain, ds = setup_scene(out_dir, cache_format, cache_type)
    print("CASE", cache_format, cache_type, "BAKE_START", out_dir)
    if cache_type == "ALL":
        result = bpy.ops.fluid.bake_all()
        print("CASE", cache_format, cache_type, "BAKE_RESULT", result)
    else:
        result = bpy.ops.fluid.bake_data()
        print("CASE", cache_format, cache_type, "BAKE_DATA_RESULT", result)
        if ds.use_noise:
            noise_result = bpy.ops.fluid.bake_noise()
            print("CASE", cache_format, cache_type, "BAKE_NOISE_RESULT", noise_result)

    for frame in (1, 2, 4, 6, 8, 12):
        scene.frame_set(frame)
        print(
            "CASE",
            cache_format,
            cache_type,
            "FRAME",
            frame,
            "density_len",
            len(ds.density_grid),
            "flame_len",
            len(ds.flame_grid),
            "temp_len",
            len(ds.temperature_grid),
        )

    for path, size in summarize_cache(out_dir):
        print("CASE", cache_format, cache_type, "FILE", path, size)

    if cache_format == "OPENVDB":
        maybe_report_vdb(out_dir)

    flow.hide_render = True
    flow.hide_viewport = True
    scene.frame_set(6)
    scene.render.filepath = os.path.join(out_dir, "render_f06.png")
    print("CASE", cache_format, cache_type, "RENDER", bpy.ops.render.render(write_still=True))


for cache_format, cache_type in CASES:
    run_case(cache_format, cache_type)
