import math
import os

import bpy
from mathutils import Vector


PROJECT_ROOT = r"C:\UE\T66"
OUTPUT_ROOT = os.path.join(PROJECT_ROOT, "SourceAssets", "Import", "VFX", "ProjectileMeshPack")
PREVIEW_ROOT = os.path.join(OUTPUT_ROOT, "previews")
BLEND_PATH = os.path.join(OUTPUT_ROOT, "ProjectileMeshPack_Source.blend")


def ensure_dirs():
    os.makedirs(OUTPUT_ROOT, exist_ok=True)
    os.makedirs(PREVIEW_ROOT, exist_ok=True)


def reset_scene():
    bpy.ops.wm.read_factory_settings(use_empty=True)
    scene = bpy.context.scene
    scene.unit_settings.system = "METRIC"
    scene.render.engine = "BLENDER_EEVEE"
    scene.render.resolution_x = 1024
    scene.render.resolution_y = 1024
    scene.render.film_transparent = False
    scene.eevee.taa_render_samples = 32
    scene.world = bpy.data.worlds.new("ProjectilePackWorld")
    scene.world.use_nodes = True
    bg = scene.world.node_tree.nodes["Background"]
    bg.inputs[0].default_value = (0.03, 0.04, 0.06, 1.0)
    bg.inputs[1].default_value = 0.55


def make_material(name, base_color, roughness=0.45, metallic=0.0, emission=0.0):
    mat = bpy.data.materials.new(name)
    mat.use_nodes = True
    bsdf = mat.node_tree.nodes["Principled BSDF"]
    bsdf.inputs["Base Color"].default_value = (*base_color, 1.0)
    bsdf.inputs["Roughness"].default_value = roughness
    bsdf.inputs["Metallic"].default_value = metallic
    if emission > 0.0:
        bsdf.inputs["Emission Color"].default_value = (*base_color, 1.0)
        bsdf.inputs["Emission Strength"].default_value = emission
    return mat


def assign_material(obj, mat):
    obj.data.materials.clear()
    obj.data.materials.append(mat)


def shade_smooth(obj):
    bpy.context.view_layer.objects.active = obj
    obj.select_set(True)
    bpy.ops.object.shade_smooth()
    obj.select_set(False)


def apply_transforms(obj):
    bpy.context.view_layer.objects.active = obj
    obj.select_set(True)
    bpy.ops.object.transform_apply(location=False, rotation=True, scale=True)
    obj.select_set(False)


def add_bevel(obj, width=0.02, segments=2):
    bevel = obj.modifiers.new(name="Bevel", type="BEVEL")
    bevel.width = width
    bevel.segments = segments
    bevel.limit_method = "ANGLE"
    bpy.context.view_layer.objects.active = obj
    obj.select_set(True)
    bpy.ops.object.modifier_apply(modifier=bevel.name)
    obj.select_set(False)


def select_only(obj):
    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj


def join_selected_objects(target_name):
    bpy.ops.object.join()
    obj = bpy.context.view_layer.objects.active
    obj.name = target_name
    obj.data.name = target_name
    return obj


def cleanup_geometry(obj):
    select_only(obj)
    bpy.ops.object.mode_set(mode="EDIT")
    bpy.ops.mesh.select_all(action="SELECT")
    bpy.ops.mesh.remove_doubles()
    bpy.ops.mesh.normals_make_consistent(inside=False)
    bpy.ops.object.mode_set(mode="OBJECT")
    obj.select_set(False)


def create_box(name, location, scale, rotation=(0.0, 0.0, 0.0)):
    bpy.ops.mesh.primitive_cube_add(location=location, rotation=rotation)
    obj = bpy.context.active_object
    obj.name = name
    obj.scale = scale
    return obj


def create_cylinder(name, location, radius, depth, vertices=20, rotation=(0.0, 0.0, 0.0)):
    bpy.ops.mesh.primitive_cylinder_add(
        vertices=vertices,
        radius=radius,
        depth=depth,
        location=location,
        rotation=rotation,
    )
    obj = bpy.context.active_object
    obj.name = name
    return obj


def create_uv_sphere(name, location, radius, segments=20, rings=10):
    bpy.ops.mesh.primitive_uv_sphere_add(
        segments=segments,
        ring_count=rings,
        radius=radius,
        location=location,
    )
    obj = bpy.context.active_object
    obj.name = name
    return obj


def create_ico(name, location, radius, subdivisions=1):
    bpy.ops.mesh.primitive_ico_sphere_add(
        subdivisions=subdivisions,
        radius=radius,
        location=location,
    )
    obj = bpy.context.active_object
    obj.name = name
    return obj


def set_origin_to_geometry(obj):
    select_only(obj)
    bpy.ops.object.origin_set(type="ORIGIN_GEOMETRY", center="BOUNDS")
    obj.select_set(False)


def center_and_ground(obj):
    bpy.context.view_layer.update()
    corners = [obj.matrix_world @ Vector(corner) for corner in obj.bound_box]
    min_corner = Vector((min(v.x for v in corners), min(v.y for v in corners), min(v.z for v in corners)))
    max_corner = Vector((max(v.x for v in corners), max(v.y for v in corners), max(v.z for v in corners)))
    center = (min_corner + max_corner) * 0.5
    obj.location -= Vector((center.x, center.y, min_corner.z))


def export_glb(obj, file_name):
    path = os.path.join(OUTPUT_ROOT, file_name)
    select_only(obj)
    bpy.ops.export_scene.gltf(
        filepath=path,
        export_format="GLB",
        use_selection=True,
        export_apply=True,
        export_yup=True,
        export_texcoords=True,
        export_normals=True,
        export_materials="EXPORT",
        export_cameras=False,
        export_lights=False,
        export_animations=False,
    )
    obj.select_set(False)
    return path


def ensure_camera_and_lights():
    bpy.ops.object.camera_add(location=(6.8, -7.1, 5.2), rotation=(math.radians(63.0), 0.0, math.radians(43.0)))
    cam = bpy.context.active_object
    cam.data.lens = 58
    cam.data.clip_start = 0.01
    bpy.context.scene.camera = cam

    bpy.ops.object.light_add(type="AREA", location=(4.2, -3.8, 5.5))
    key = bpy.context.active_object
    key.data.energy = 2200
    key.data.shape = "RECTANGLE"
    key.data.size = 5.5
    key.data.size_y = 4.0

    bpy.ops.object.light_add(type="AREA", location=(-4.8, 4.4, 3.2))
    rim = bpy.context.active_object
    rim.data.energy = 1000
    rim.data.shape = "RECTANGLE"
    rim.data.size = 4.0
    rim.data.size_y = 3.0

    bpy.ops.object.light_add(type="SUN", location=(0.0, 0.0, 8.0))
    sun = bpy.context.active_object
    sun.rotation_euler = (math.radians(42.0), math.radians(16.0), math.radians(18.0))
    sun.data.energy = 1.7

    bpy.ops.mesh.primitive_plane_add(location=(0.0, 0.0, -0.03))
    floor = bpy.context.active_object
    floor.scale = (4.8, 4.8, 1.0)
    floor_mat = make_material("PreviewFloor", (0.075, 0.085, 0.1), roughness=0.92)
    assign_material(floor, floor_mat)


def render_preview(obj, file_name):
    preview_collection = bpy.data.collections.get("Preview")
    if preview_collection is None:
        preview_collection = bpy.data.collections.new("Preview")
        bpy.context.scene.collection.children.link(preview_collection)

    preview_parent = bpy.data.objects.new(f"{obj.name}_PreviewRoot", None)
    preview_collection.objects.link(preview_parent)

    clone = obj.copy()
    clone.data = obj.data.copy()
    clone.animation_data_clear()
    preview_collection.objects.link(clone)
    clone.parent = preview_parent
    clone.location = (0.0, 0.0, 0.0)
    clone.rotation_euler = (math.radians(14.0), math.radians(-10.0), math.radians(28.0))

    bpy.context.view_layer.update()
    corners = [clone.matrix_world @ Vector(corner) for corner in clone.bound_box]
    min_corner = Vector((min(v.x for v in corners), min(v.y for v in corners), min(v.z for v in corners)))
    max_corner = Vector((max(v.x for v in corners), max(v.y for v in corners), max(v.z for v in corners)))
    size = max_corner - min_corner
    max_dim = max(size.x, size.y, size.z, 0.5)
    scale = 2.7 / max_dim
    clone.scale = (scale, scale, scale)
    bpy.context.view_layer.update()
    corners = [clone.matrix_world @ Vector(corner) for corner in clone.bound_box]
    min_corner = Vector((min(v.x for v in corners), min(v.y for v in corners), min(v.z for v in corners)))
    max_corner = Vector((max(v.x for v in corners), max(v.y for v in corners), max(v.z for v in corners)))
    center = (min_corner + max_corner) * 0.5
    clone.location -= Vector((center.x, center.y, min_corner.z))

    bpy.context.scene.render.filepath = os.path.join(PREVIEW_ROOT, file_name)
    bpy.ops.render.render(write_still=True)

    bpy.data.objects.remove(clone, do_unlink=True)
    bpy.data.objects.remove(preview_parent, do_unlink=True)


def build_arthur_sword(materials):
    blade = create_box("ArthurSword_Blade", (0.34, 0.0, 0.18), (0.92, 0.085, 0.12))
    blade.rotation_euler = (0.0, 0.0, math.radians(2.0))
    tip = create_cylinder("ArthurSword_Tip", (1.08, 0.0, 0.18), 0.07, 0.24, vertices=4, rotation=(0.0, math.radians(90.0), math.radians(45.0)))
    tip.scale = (1.0, 0.92, 1.16)
    ricasso = create_box("ArthurSword_Ricasso", (-0.50, 0.0, 0.18), (0.12, 0.095, 0.11))
    guard_core = create_box("ArthurSword_GuardCore", (-0.64, 0.0, 0.18), (0.10, 0.12, 0.10))
    guard_left = create_box("ArthurSword_GuardLeft", (-0.70, 0.34, 0.18), (0.18, 0.06, 0.08), rotation=(0.0, math.radians(10.0), math.radians(14.0)))
    guard_right = create_box("ArthurSword_GuardRight", (-0.70, -0.34, 0.18), (0.18, 0.06, 0.08), rotation=(0.0, math.radians(-10.0), math.radians(-14.0)))
    grip = create_cylinder("ArthurSword_Grip", (-1.02, 0.0, 0.18), 0.075, 0.52, vertices=12, rotation=(0.0, math.radians(90.0), 0.0))
    wrap_a = create_cylinder("ArthurSword_WrapA", (-1.14, 0.0, 0.18), 0.083, 0.04, vertices=12, rotation=(0.0, math.radians(90.0), 0.0))
    wrap_b = create_cylinder("ArthurSword_WrapB", (-1.00, 0.0, 0.18), 0.083, 0.04, vertices=12, rotation=(0.0, math.radians(90.0), 0.0))
    wrap_c = create_cylinder("ArthurSword_WrapC", (-0.86, 0.0, 0.18), 0.083, 0.04, vertices=12, rotation=(0.0, math.radians(90.0), 0.0))
    pommel = create_ico("ArthurSword_Pommel", (-1.38, 0.0, 0.18), 0.14, subdivisions=1)
    pommel.scale = (1.1, 0.9, 1.1)
    assign_material(blade, materials["blade"])
    assign_material(tip, materials["blade"])
    assign_material(ricasso, materials["fuller"])
    assign_material(guard_core, materials["guard"])
    assign_material(guard_left, materials["guard"])
    assign_material(guard_right, materials["guard"])
    assign_material(grip, materials["grip"])
    assign_material(wrap_a, materials["guard"])
    assign_material(wrap_b, materials["guard"])
    assign_material(wrap_c, materials["guard"])
    assign_material(pommel, materials["guard"])
    pieces = [blade, tip, ricasso, guard_core, guard_left, guard_right, grip, wrap_a, wrap_b, wrap_c, pommel]
    for obj in pieces:
        add_bevel(obj, 0.018 if obj in [blade, ricasso] else 0.01)
        apply_transforms(obj)

    select_only(blade)
    for obj in pieces[1:]:
        obj.select_set(True)
    sword = join_selected_objects("Arthur_Sword")
    cleanup_geometry(sword)
    shade_smooth(sword)
    set_origin_to_geometry(sword)
    center_and_ground(sword)
    return sword


def build_bone_projectile(materials):
    shaft = create_cylinder("Bone_Shaft", (0.04, 0.0, 0.16), 0.082, 1.36, vertices=14, rotation=(0.0, math.radians(90.0), 0.0))
    shaft.scale = (1.0, 0.92, 1.06)
    tip = create_cylinder("Bone_Tip", (0.98, 0.0, 0.16), 0.05, 0.44, vertices=5, rotation=(0.0, math.radians(90.0), math.radians(36.0)))
    tip.scale = (1.0, 0.80, 1.7)
    collar = create_cylinder("Bone_Collar", (0.66, 0.0, 0.16), 0.095, 0.16, vertices=12, rotation=(0.0, math.radians(90.0), 0.0))
    base_knob = create_uv_sphere("Bone_BaseKnob", (-0.70, 0.0, 0.16), 0.16, segments=18, rings=12)
    base_knob.scale = (1.25, 1.05, 1.05)
    rear_nub_a = create_uv_sphere("Bone_RearNubA", (-0.94, 0.14, 0.28), 0.11, segments=16, rings=10)
    rear_nub_b = create_uv_sphere("Bone_RearNubB", (-0.94, -0.14, 0.04), 0.11, segments=16, rings=10)
    pieces = [shaft, tip, collar, base_knob, rear_nub_a, rear_nub_b]
    for piece in pieces:
        assign_material(piece, materials["bone"])
        add_bevel(piece, 0.012)
        apply_transforms(piece)

    select_only(shaft)
    for obj in pieces[1:]:
        obj.select_set(True)
    bone = join_selected_objects("Idol_Bone_Projectile")
    cleanup_geometry(bone)
    shade_smooth(bone)
    set_origin_to_geometry(bone)
    center_and_ground(bone)
    return bone


def build_glass_shard(materials):
    main = create_ico("Glass_Main", (0.18, 0.0, 0.16), 0.26, subdivisions=1)
    main.scale = (1.8, 0.34, 0.40)
    main.rotation_euler = (math.radians(4.0), math.radians(8.0), math.radians(12.0))
    tail = create_ico("Glass_Tail", (-0.28, 0.06, 0.10), 0.12, subdivisions=1)
    tail.scale = (1.1, 0.24, 0.24)
    chip_top = create_ico("Glass_ChipTop", (0.02, 0.12, 0.34), 0.08, subdivisions=1)
    chip_top.scale = (0.8, 0.18, 0.18)
    chip_low = create_ico("Glass_ChipLow", (-0.06, -0.14, 0.02), 0.09, subdivisions=1)
    chip_low.scale = (1.0, 0.18, 0.18)

    pieces = [main, tail, chip_top, chip_low]
    for piece in pieces:
        assign_material(piece, materials["glass"])
        add_bevel(piece, 0.01)
        apply_transforms(piece)

    select_only(main)
    for piece in pieces[1:]:
        piece.select_set(True)
    shard = join_selected_objects("Idol_Glass_Shard")
    cleanup_geometry(shard)
    shade_smooth(shard)
    set_origin_to_geometry(shard)
    center_and_ground(shard)
    return shard


def build_steel_spike(materials):
    shaft = create_box("Steel_Shaft", (0.02, 0.0, 0.16), (0.70, 0.055, 0.055))
    head = create_cylinder("Steel_Head", (0.70, 0.0, 0.16), 0.08, 0.34, vertices=4, rotation=(0.0, math.radians(90.0), math.radians(45.0)))
    head.scale = (1.0, 0.78, 1.34)
    collar = create_box("Steel_Collar", (0.62, 0.0, 0.16), (0.08, 0.075, 0.075))
    fin_a = create_box("Steel_FinA", (-0.62, 0.0, 0.26), (0.18, 0.02, 0.12))
    fin_b = create_box("Steel_FinB", (-0.62, 0.0, 0.06), (0.18, 0.02, 0.12))
    fin_c = create_box("Steel_FinC", (-0.62, 0.12, 0.16), (0.18, 0.12, 0.02))
    fin_d = create_box("Steel_FinD", (-0.62, -0.12, 0.16), (0.18, 0.12, 0.02))

    for piece in [shaft, head, collar, fin_a, fin_b, fin_c, fin_d]:
        assign_material(piece, materials["steel"])
        add_bevel(piece, 0.012)
        apply_transforms(piece)

    select_only(shaft)
    for obj in [head, collar, fin_a, fin_b, fin_c, fin_d]:
        obj.select_set(True)
    steel = join_selected_objects("Idol_Steel_Spike")
    cleanup_geometry(steel)
    shade_smooth(steel)
    set_origin_to_geometry(steel)
    center_and_ground(steel)
    return steel


def build_wood_thorn(materials):
    thorn = create_cylinder("Wood_Thorn", (0.04, 0.0, 0.16), 0.09, 1.48, vertices=7, rotation=(0.0, math.radians(90.0), 0.0))
    thorn.scale = (1.0, 0.82, 1.02)
    tip = create_cylinder("Wood_Tip", (0.90, 0.0, 0.16), 0.04, 0.30, vertices=4, rotation=(0.0, math.radians(90.0), math.radians(45.0)))
    branch_a = create_cylinder("Wood_BranchA", (-0.10, 0.18, 0.30), 0.030, 0.56, vertices=6, rotation=(math.radians(30.0), math.radians(8.0), math.radians(42.0)))
    branch_b = create_cylinder("Wood_BranchB", (-0.34, -0.16, 0.06), 0.028, 0.46, vertices=6, rotation=(math.radians(-20.0), math.radians(6.0), math.radians(-36.0)))
    branch_c = create_cylinder("Wood_BranchC", (0.18, -0.08, 0.34), 0.020, 0.28, vertices=5, rotation=(math.radians(62.0), math.radians(0.0), math.radians(-24.0)))

    for piece in [thorn, tip, branch_a, branch_b, branch_c]:
        assign_material(piece, materials["wood"])
        add_bevel(piece, 0.012)
        apply_transforms(piece)

    select_only(thorn)
    for obj in [tip, branch_a, branch_b, branch_c]:
        obj.select_set(True)
    wood = join_selected_objects("Idol_Wood_Thorn")
    cleanup_geometry(wood)
    shade_smooth(wood)
    set_origin_to_geometry(wood)
    center_and_ground(wood)
    return wood


def build_light_lance(materials):
    shaft = create_cylinder("Light_Core", (0.00, 0.0, 0.16), 0.05, 1.18, vertices=10, rotation=(0.0, math.radians(90.0), 0.0))
    head = create_ico("Light_Head", (0.82, 0.0, 0.16), 0.18, subdivisions=1)
    head.scale = (2.2, 0.34, 0.34)
    crown_a = create_box("Light_CrownA", (0.52, 0.14, 0.16), (0.14, 0.02, 0.10), rotation=(0.0, math.radians(24.0), math.radians(24.0)))
    crown_b = create_box("Light_CrownB", (0.52, -0.14, 0.16), (0.14, 0.02, 0.10), rotation=(0.0, math.radians(-24.0), math.radians(-24.0)))
    tail = create_ico("Light_Tail", (-0.66, 0.0, 0.16), 0.11, subdivisions=1)
    tail.scale = (1.6, 0.32, 0.32)
    for piece in [shaft, head, crown_a, crown_b, tail]:
        assign_material(piece, materials["light"])
        add_bevel(piece, 0.012)
        apply_transforms(piece)

    select_only(shaft)
    for obj in [head, crown_a, crown_b, tail]:
        obj.select_set(True)
    light = join_selected_objects("Idol_Light_Lance")
    cleanup_geometry(light)
    shade_smooth(light)
    set_origin_to_geometry(light)
    center_and_ground(light)
    return light


def build_wind_crescent(materials):
    outer_a = create_box("Wind_OuterA", (0.18, 0.22, 0.18), (0.52, 0.06, 0.05), rotation=(0.0, math.radians(8.0), math.radians(34.0)))
    outer_b = create_box("Wind_OuterB", (0.18, -0.22, 0.18), (0.52, 0.06, 0.05), rotation=(0.0, math.radians(-8.0), math.radians(-34.0)))
    inner_a = create_box("Wind_InnerA", (-0.06, 0.12, 0.18), (0.34, 0.035, 0.03), rotation=(0.0, math.radians(8.0), math.radians(30.0)))
    inner_b = create_box("Wind_InnerB", (-0.06, -0.12, 0.18), (0.34, 0.035, 0.03), rotation=(0.0, math.radians(-8.0), math.radians(-30.0)))
    front_tip = create_cylinder("Wind_FrontTip", (0.74, 0.0, 0.18), 0.038, 0.22, vertices=4, rotation=(0.0, math.radians(90.0), math.radians(45.0)))
    rear_tail = create_box("Wind_RearTail", (-0.44, 0.0, 0.18), (0.14, 0.04, 0.04), rotation=(0.0, math.radians(10.0), 0.0))

    for piece in [outer_a, outer_b, inner_a, inner_b, front_tip, rear_tail]:
        assign_material(piece, materials["wind"])
        add_bevel(piece, 0.012)
        apply_transforms(piece)

    select_only(outer_a)
    for obj in [outer_b, inner_a, inner_b, front_tip, rear_tail]:
        obj.select_set(True)
    wind = join_selected_objects("Idol_Wind_Crescent")
    cleanup_geometry(wind)
    shade_smooth(wind)
    set_origin_to_geometry(wind)
    center_and_ground(wind)
    return wind


def build_rubber_ball(materials):
    ball = create_uv_sphere("Idol_Rubber_Ball", (0.0, 0.0, 0.0), 0.36, segments=24, rings=12)
    seam = create_cylinder("Rubber_Seam", (0.0, 0.0, 0.0), 0.372, 0.02, vertices=24)
    seam.rotation_euler = (math.radians(90.0), 0.0, 0.0)
    nub = create_cylinder("Rubber_Nub", (-0.28, 0.0, -0.10), 0.05, 0.14, vertices=12, rotation=(0.0, math.radians(90.0), 0.0))
    assign_material(ball, materials["rubber"])
    assign_material(seam, materials["rubber_dark"])
    assign_material(nub, materials["rubber_dark"])
    add_bevel(ball, 0.01)
    add_bevel(seam, 0.006)
    add_bevel(nub, 0.006)
    apply_transforms(ball)
    apply_transforms(seam)
    apply_transforms(nub)
    select_only(ball)
    seam.select_set(True)
    nub.select_set(True)
    rubber = join_selected_objects("Idol_Rubber_Ball")
    cleanup_geometry(rubber)
    shade_smooth(rubber)
    set_origin_to_geometry(rubber)
    center_and_ground(rubber)
    return rubber


def build_ice_crystal(materials):
    main = create_ico("Ice_Main", (0.10, 0.0, 0.18), 0.24, subdivisions=1)
    main.scale = (2.0, 0.32, 0.34)
    shard_a = create_ico("Ice_ShardA", (-0.10, 0.12, 0.30), 0.11, subdivisions=1)
    shard_a.scale = (1.2, 0.22, 0.24)
    shard_b = create_ico("Ice_ShardB", (-0.18, -0.14, 0.06), 0.10, subdivisions=1)
    shard_b.scale = (1.0, 0.20, 0.22)
    for piece in [main, shard_a, shard_b]:
        assign_material(piece, materials["ice"])
        add_bevel(piece, 0.01)
        apply_transforms(piece)
    select_only(main)
    for piece in [shard_a, shard_b]:
        piece.select_set(True)
    crystal = join_selected_objects("Idol_Ice_Crystal")
    cleanup_geometry(crystal)
    shade_smooth(crystal)
    set_origin_to_geometry(crystal)
    center_and_ground(crystal)
    return crystal


def build_star_shard(materials):
    core = create_ico("Star_Core", (0.0, 0.0, 0.18), 0.16, subdivisions=1)
    core.scale = (0.94, 0.94, 0.94)
    spikes = []
    for axis, loc, rot in [
        ("x", (0.34, 0.0, 0.18), (0.0, math.radians(90.0), math.radians(45.0))),
        ("x2", (-0.34, 0.0, 0.18), (0.0, math.radians(-90.0), math.radians(45.0))),
        ("y", (0.0, 0.34, 0.18), (math.radians(90.0), 0.0, math.radians(45.0))),
        ("y2", (0.0, -0.34, 0.18), (math.radians(-90.0), 0.0, math.radians(45.0))),
    ]:
        spike = create_cylinder(f"StarSpike_{axis}", loc, 0.05, 0.38, vertices=4, rotation=rot)
        spikes.append(spike)
    pieces = [core] + spikes
    for piece in pieces:
        assign_material(piece, materials["star"])
        add_bevel(piece, 0.012)
        apply_transforms(piece)
    select_only(core)
    for piece in spikes:
        piece.select_set(True)
    star = join_selected_objects("Idol_Star_Shard")
    cleanup_geometry(star)
    shade_smooth(star)
    set_origin_to_geometry(star)
    center_and_ground(star)
    return star


def build_sound_disc(materials):
    outer = create_cylinder("Sound_Outer", (0.0, 0.0, 0.0), 0.40, 0.08, vertices=36)
    inner = create_cylinder("Sound_Inner", (0.0, 0.0, 0.0), 0.22, 0.10, vertices=36)
    ridge = create_cylinder("Sound_Ridge", (0.0, 0.0, 0.04), 0.33, 0.03, vertices=36)
    emitter = create_cylinder("Sound_Emitter", (-0.26, 0.0, 0.0), 0.05, 0.12, vertices=16, rotation=(0.0, math.radians(90.0), 0.0))
    assign_material(outer, materials["sound"])
    assign_material(inner, materials["sound_dark"])
    assign_material(ridge, materials["sound"])
    assign_material(emitter, materials["sound"])
    for piece in [outer, inner, ridge, emitter]:
        add_bevel(piece, 0.008)
        apply_transforms(piece)
    select_only(outer)
    for piece in [inner, ridge, emitter]:
        piece.select_set(True)
    sound = join_selected_objects("Idol_Sound_Disc")
    cleanup_geometry(sound)
    shade_smooth(sound)
    set_origin_to_geometry(sound)
    center_and_ground(sound)
    return sound


def build_shadow_orb(materials):
    orb = create_uv_sphere("Shadow_Orb", (0.0, 0.0, 0.0), 0.28, segments=20, rings=10)
    spikes = []
    spike_specs = [
        ((0.34, 0.0, 0.0), (0.0, math.radians(90.0), math.radians(45.0))),
        ((-0.30, 0.10, 0.02), (0.0, math.radians(-90.0), math.radians(45.0))),
        ((0.0, 0.32, -0.02), (math.radians(90.0), 0.0, math.radians(45.0))),
        ((0.0, -0.28, 0.06), (math.radians(-90.0), 0.0, math.radians(45.0))),
        ((0.0, 0.0, 0.34), (0.0, 0.0, math.radians(45.0))),
    ]
    for index, (loc, rot) in enumerate(spike_specs):
        spike = create_cylinder(f"Shadow_Spike_{index}", loc, 0.04, 0.24, vertices=4, rotation=rot)
        spikes.append(spike)

    glow = create_uv_sphere("Shadow_Glow", (0.0, 0.0, 0.0), 0.16, segments=18, rings=8)
    glow.scale = (1.6, 1.6, 1.6)
    ring = create_cylinder("Shadow_Ring", (0.0, 0.0, 0.0), 0.42, 0.04, vertices=20)
    ring.rotation_euler = (math.radians(70.0), 0.0, math.radians(22.0))
    for piece in [orb] + spikes:
        assign_material(piece, materials["shadow"])
        add_bevel(piece, 0.01)
        apply_transforms(piece)
    assign_material(glow, materials["shadow_glow"])
    assign_material(ring, materials["shadow_glow"])
    add_bevel(glow, 0.008)
    add_bevel(ring, 0.006)
    apply_transforms(glow)
    apply_transforms(ring)

    select_only(orb)
    for piece in spikes + [glow, ring]:
        piece.select_set(True)
    shadow = join_selected_objects("Idol_Shadow_Orb")
    cleanup_geometry(shadow)
    shade_smooth(shadow)
    set_origin_to_geometry(shadow)
    center_and_ground(shadow)
    return shadow


def build_meshes():
    materials = {
        "blade": make_material("M_Blade", (0.80, 0.83, 0.90), roughness=0.22, metallic=0.70),
        "fuller": make_material("M_Fuller", (0.64, 0.68, 0.78), roughness=0.28, metallic=0.55),
        "guard": make_material("M_Guard", (0.89, 0.69, 0.26), roughness=0.30, metallic=0.65),
        "grip": make_material("M_Grip", (0.22, 0.12, 0.06), roughness=0.72, metallic=0.0),
        "bone": make_material("M_Bone", (0.93, 0.89, 0.79), roughness=0.62, metallic=0.0),
        "glass": make_material("M_Glass", (0.66, 0.88, 0.98), roughness=0.16, metallic=0.0, emission=0.08),
        "steel": make_material("M_Steel", (0.70, 0.76, 0.88), roughness=0.18, metallic=0.78),
        "wood": make_material("M_Wood", (0.36, 0.25, 0.11), roughness=0.72, metallic=0.0),
        "light": make_material("M_Light", (0.98, 0.90, 0.50), roughness=0.18, metallic=0.15, emission=0.55),
        "wind": make_material("M_Wind", (0.58, 0.98, 0.90), roughness=0.20, metallic=0.0, emission=0.35),
        "rubber": make_material("M_Rubber", (0.94, 0.34, 0.22), roughness=0.55, metallic=0.0),
        "rubber_dark": make_material("M_RubberDark", (0.44, 0.08, 0.06), roughness=0.72, metallic=0.0),
        "ice": make_material("M_Ice", (0.64, 0.92, 1.00), roughness=0.14, metallic=0.0, emission=0.12),
        "star": make_material("M_Star", (1.00, 0.94, 0.48), roughness=0.20, metallic=0.18, emission=0.30),
        "sound": make_material("M_Sound", (0.88, 0.74, 0.96), roughness=0.24, metallic=0.0, emission=0.16),
        "sound_dark": make_material("M_SoundDark", (0.24, 0.14, 0.30), roughness=0.58, metallic=0.0),
        "shadow": make_material("M_Shadow", (0.10, 0.07, 0.16), roughness=0.34, metallic=0.0),
        "shadow_glow": make_material("M_ShadowGlow", (0.54, 0.18, 0.82), roughness=0.18, metallic=0.0, emission=0.45),
    }

    builders = [
        (build_arthur_sword, "Arthur_Sword.glb", "Arthur_Sword.png"),
        (build_bone_projectile, "Idol_Bone_Projectile.glb", "Idol_Bone_Projectile.png"),
        (build_glass_shard, "Idol_Glass_Shard.glb", "Idol_Glass_Shard.png"),
        (build_steel_spike, "Idol_Steel_Spike.glb", "Idol_Steel_Spike.png"),
        (build_wood_thorn, "Idol_Wood_Thorn.glb", "Idol_Wood_Thorn.png"),
        (build_light_lance, "Idol_Light_Lance.glb", "Idol_Light_Lance.png"),
        (build_wind_crescent, "Idol_Wind_Crescent.glb", "Idol_Wind_Crescent.png"),
        (build_rubber_ball, "Idol_Rubber_Ball.glb", "Idol_Rubber_Ball.png"),
        (build_ice_crystal, "Idol_Ice_Crystal.glb", "Idol_Ice_Crystal.png"),
        (build_star_shard, "Idol_Star_Shard.glb", "Idol_Star_Shard.png"),
        (build_sound_disc, "Idol_Sound_Disc.glb", "Idol_Sound_Disc.png"),
        (build_shadow_orb, "Idol_Shadow_Orb.glb", "Idol_Shadow_Orb.png"),
    ]

    exported = []
    previewed = []
    for builder, file_name, preview_name in builders:
        obj = builder(materials)
        exported.append(export_glb(obj, file_name))
        render_preview(obj, preview_name)
        previewed.append(os.path.join(PREVIEW_ROOT, preview_name))
        obj.hide_render = True
        obj.hide_set(True)

    bpy.ops.wm.save_as_mainfile(filepath=BLEND_PATH)
    return exported, previewed


def main():
    ensure_dirs()
    reset_scene()
    ensure_camera_and_lights()
    exported, previewed = build_meshes()
    print("[GenerateProjectileMeshPack] Exported:")
    for path in exported:
        print(path)
    print("[GenerateProjectileMeshPack] Previews:")
    for path in previewed:
        print(path)


if __name__ == "__main__":
    main()
