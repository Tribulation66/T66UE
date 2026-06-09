import colorsys
import json
import math
import os
from pathlib import Path

import bpy
from mathutils import Vector


PROJECT_ROOT = Path(r"C:\UE\T66")
RUN_ROOT = PROJECT_ROOT / "Model Generation" / "Runs" / "Pixal3D" / "FriendSlopProbe_Hero1Male_20260604_1415"
SOURCE_GLB = RUN_ROOT / "Outputs" / "Hero_1_Chad_Male.glb"
SOURCE_IMAGE = RUN_ROOT / "Sources" / "Hero_1_Chad_Male.png"
LOOKDEV_ROOT = RUN_ROOT / "Blender" / "LookDev" / "Hero_1_Chad_Male_Rubber_20260605"
RENDER_ROOT = LOOKDEV_ROOT / "Renders"
RECIPE_JSON = LOOKDEV_ROOT / "rubber_recipe.json"
RECIPE_MD = LOOKDEV_ROOT / "rubber_recipe.md"
BLEND_PATH = LOOKDEV_ROOT / "Hero_1_Chad_Male_RubberLookDev.blend"


def ensure_dirs():
    LOOKDEV_ROOT.mkdir(parents=True, exist_ok=True)
    RENDER_ROOT.mkdir(parents=True, exist_ok=True)


VARIATIONS = [
    {
        "id": "V01_soft_satin",
        "label": "V01 Soft Satin",
        "roughness": 0.46,
        "specular_ior_level": 0.66,
        "coat_weight": 0.16,
        "coat_roughness": 0.30,
        "subsurface_weight": 0.06,
        "subsurface_radius": [1.00, 0.72, 0.50],
        "saturation": 1.12,
        "value": 1.02,
        "contrast": -0.08,
        "bevel_width": 0.012,
        "bevel_segments": 2,
        "bump_strength": 0.006,
        "bump_distance": 0.020,
        "top_candidate": False,
        "notes": "Control rubber pass with a mild satin sheen.",
    },
    {
        "id": "V02_rubber_pop",
        "label": "V02 Rubber Pop",
        "roughness": 0.34,
        "specular_ior_level": 0.78,
        "coat_weight": 0.30,
        "coat_roughness": 0.20,
        "subsurface_weight": 0.10,
        "subsurface_radius": [1.00, 0.62, 0.42],
        "saturation": 1.24,
        "value": 1.07,
        "contrast": -0.16,
        "bevel_width": 0.024,
        "bevel_segments": 3,
        "bump_strength": 0.004,
        "bump_distance": 0.016,
        "top_candidate": True,
        "notes": "Balanced first candidate: saturated, soft, and broad highlight.",
    },
    {
        "id": "V03_vinyl_bounce",
        "label": "V03 Vinyl Bounce",
        "roughness": 0.25,
        "specular_ior_level": 0.88,
        "coat_weight": 0.46,
        "coat_roughness": 0.12,
        "subsurface_weight": 0.07,
        "subsurface_radius": [1.00, 0.55, 0.34],
        "saturation": 1.34,
        "value": 1.08,
        "contrast": -0.24,
        "bevel_width": 0.034,
        "bevel_segments": 4,
        "bump_strength": 0.003,
        "bump_distance": 0.012,
        "top_candidate": True,
        "notes": "Glossier vinyl direction, likely close if the critique is 'not rubbery enough'.",
    },
    {
        "id": "V04_candy_rubber",
        "label": "V04 Candy Rubber",
        "roughness": 0.29,
        "specular_ior_level": 0.82,
        "coat_weight": 0.34,
        "coat_roughness": 0.16,
        "subsurface_weight": 0.18,
        "subsurface_radius": [1.00, 0.68, 0.48],
        "saturation": 1.30,
        "value": 1.12,
        "contrast": -0.30,
        "bevel_width": 0.044,
        "bevel_segments": 4,
        "bump_strength": 0.002,
        "bump_distance": 0.010,
        "top_candidate": True,
        "notes": "Softest bouncy candidate; more subsurface and edge rounding.",
    },
    {
        "id": "V05_matte_gum",
        "label": "V05 Matte Gum",
        "roughness": 0.56,
        "specular_ior_level": 0.62,
        "coat_weight": 0.10,
        "coat_roughness": 0.36,
        "subsurface_weight": 0.16,
        "subsurface_radius": [1.00, 0.74, 0.55],
        "saturation": 1.18,
        "value": 1.06,
        "contrast": -0.22,
        "bevel_width": 0.030,
        "bevel_segments": 3,
        "bump_strength": 0.004,
        "bump_distance": 0.014,
        "top_candidate": False,
        "notes": "Softer gum rubber, useful lower-gloss boundary.",
    },
    {
        "id": "V06_toy_vinyl_gloss",
        "label": "V06 Toy Vinyl Gloss",
        "roughness": 0.18,
        "specular_ior_level": 0.96,
        "coat_weight": 0.62,
        "coat_roughness": 0.08,
        "subsurface_weight": 0.04,
        "subsurface_radius": [1.00, 0.50, 0.30],
        "saturation": 1.40,
        "value": 1.10,
        "contrast": -0.34,
        "bevel_width": 0.040,
        "bevel_segments": 4,
        "bump_strength": 0.001,
        "bump_distance": 0.008,
        "top_candidate": True,
        "notes": "Upper gloss boundary; may read more toy vinyl than rubber.",
    },
]


def clear_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete()
    for block in list(bpy.data.meshes):
        if block.users == 0:
            bpy.data.meshes.remove(block)
    for block in list(bpy.data.materials):
        if block.users == 0:
            bpy.data.materials.remove(block)


def set_render_defaults(scene, resolution=(3600, 1000), frame_end=1):
    try:
        scene.render.engine = "BLENDER_EEVEE_NEXT"
    except Exception:
        scene.render.engine = "BLENDER_EEVEE"
    try:
        scene.eevee.taa_render_samples = 64
    except Exception:
        pass
    try:
        scene.eevee.use_gtao = True
        scene.eevee.gtao_distance = 2.0
        scene.eevee.gtao_factor = 0.35
    except Exception:
        pass
    scene.render.resolution_x = resolution[0]
    scene.render.resolution_y = resolution[1]
    scene.render.fps = 24
    scene.frame_start = 1
    scene.frame_end = frame_end
    scene.world = bpy.data.worlds.new(scene.name + "_World") if scene.world is None else scene.world
    scene.world.color = (1.0, 1.0, 1.0)
    scene.view_settings.view_transform = "Standard"
    scene.view_settings.look = "Medium High Contrast"
    scene.view_settings.exposure = 0.0
    scene.view_settings.gamma = 1.0


def look_at(obj, target):
    direction = Vector(target) - obj.location
    obj.rotation_euler = direction.to_track_quat("-Z", "Y").to_euler()


def add_flat_lighting(scene, center=(0, 0, 1.2), span=18.0):
    lights = [
        ("Key_Softbox_L", (-5.5, -5.2, 5.0), 470.0, 6.5),
        ("Key_Softbox_R", (5.5, -5.2, 4.6), 360.0, 7.0),
        ("Top_Soft_Fill", (0.0, -1.5, 7.2), 230.0, 8.0),
        ("Front_Fill", (0.0, -7.5, 2.1), 150.0, 9.0),
    ]
    for name, loc, energy, size in lights:
        data = bpy.data.lights.new(name, "AREA")
        data.energy = energy
        data.size = size
        data.use_shadow = False
        obj = bpy.data.objects.new(name, data)
        scene.collection.objects.link(obj)
        obj.location = loc
        look_at(obj, center)


def add_camera(scene, name, location, target, ortho_scale):
    data = bpy.data.cameras.new(name)
    data.type = "ORTHO"
    data.ortho_scale = ortho_scale
    obj = bpy.data.objects.new(name, data)
    scene.collection.objects.link(obj)
    obj.location = location
    look_at(obj, target)
    scene.camera = obj
    return obj


def import_glb_template():
    bpy.ops.import_scene.gltf(filepath=str(SOURCE_GLB))
    imported = [obj for obj in bpy.context.selected_objects if obj.type == "MESH"]
    if not imported:
        imported = [obj for obj in bpy.context.scene.objects if obj.type == "MESH"]
    for obj in imported:
        obj.name = "Template_" + obj.name
    return imported


def world_bbox(objects):
    pts = []
    for obj in objects:
        if obj.type != "MESH":
            continue
        for corner in obj.bound_box:
            pts.append(obj.matrix_world @ Vector(corner))
    if not pts:
        return Vector((0, 0, 0)), Vector((0, 0, 0))
    min_v = Vector((min(p.x for p in pts), min(p.y for p in pts), min(p.z for p in pts)))
    max_v = Vector((max(p.x for p in pts), max(p.y for p in pts), max(p.z for p in pts)))
    return min_v, max_v


def normalize_group(objects, target_height=2.65):
    bpy.context.view_layer.update()
    min_v, max_v = world_bbox(objects)
    height = max(max_v.z - min_v.z, 0.001)
    scale = target_height / height
    center = (min_v + max_v) * 0.5
    for obj in objects:
        obj.location -= center
        obj.scale *= scale
    bpy.context.view_layer.update()
    min_v, max_v = world_bbox(objects)
    bottom = min_v.z
    for obj in objects:
        obj.location.z -= bottom
    bpy.context.view_layer.update()


def duplicate_group(
    template_objects,
    group_name,
    location,
    material_builder=None,
    variation=None,
    raw=False,
    target_height=2.65,
    front_rotation_degrees=180.0,
):
    collection = bpy.data.collections.new(group_name)
    bpy.context.scene.collection.children.link(collection)
    empty = bpy.data.objects.new(group_name + "_Root", None)
    collection.objects.link(empty)
    new_objects = []
    for obj in template_objects:
        new_obj = obj.copy()
        new_obj.data = obj.data.copy()
        new_obj.animation_data_clear()
        new_obj.hide_viewport = False
        new_obj.hide_render = False
        collection.objects.link(new_obj)
        new_obj.parent = empty
        new_objects.append(new_obj)
        if not raw:
            bpy.context.view_layer.objects.active = new_obj
            new_obj.select_set(True)
            try:
                bpy.ops.object.shade_smooth()
            except Exception:
                pass
            new_obj.select_set(False)
            bevel = new_obj.modifiers.new("Rubber_Soft_Edge_Bevel", "BEVEL")
            bevel.width = variation["bevel_width"]
            bevel.segments = variation["bevel_segments"]
            bevel.affect = "EDGES"
            try:
                bevel.profile = 0.58
            except Exception:
                pass
            normal = new_obj.modifiers.new("Rubber_Weighted_Normals", "WEIGHTED_NORMAL")
            try:
                normal.keep_sharp = True
            except Exception:
                pass
        if material_builder is not None:
            original_mats = list(obj.data.materials)
            new_obj.data.materials.clear()
            for slot_index, src_mat in enumerate(original_mats):
                new_obj.data.materials.append(material_builder(src_mat, variation, slot_index))
    normalize_group(new_objects, target_height=target_height)
    empty.rotation_euler.z = math.radians(front_rotation_degrees)
    empty.location = location
    return empty, collection, new_objects


def get_principled(mat):
    if mat and mat.use_nodes:
        for node in mat.node_tree.nodes:
            if node.bl_idname == "ShaderNodeBsdfPrincipled":
                return node
    return None


def get_first_image(mat):
    if mat and mat.use_nodes:
        for node in mat.node_tree.nodes:
            if node.bl_idname == "ShaderNodeTexImage" and getattr(node, "image", None):
                return node.image
    return None


def source_base_color(mat):
    if mat:
        bsdf = get_principled(mat)
        if bsdf and "Base Color" in bsdf.inputs:
            value = bsdf.inputs["Base Color"].default_value
            return tuple(value[:4])
        return tuple(mat.diffuse_color)
    return (0.8, 0.8, 0.8, 1.0)


def adjusted_color(color, saturation, value):
    r, g, b, a = color
    h, s, v = colorsys.rgb_to_hsv(r, g, b)
    s = max(0.0, min(1.0, s * saturation))
    v = max(0.0, min(1.0, v * value))
    rr, gg, bb = colorsys.hsv_to_rgb(h, s, v)
    return (rr, gg, bb, a)


def set_input(node, names, value):
    for name in names:
        if name in node.inputs:
            node.inputs[name].default_value = value
            return name
    return None


def make_rubber_material(src_mat, variation, slot_index):
    mat_name = f"{variation['id']}_{slot_index}_{src_mat.name if src_mat else 'mat'}"
    mat = bpy.data.materials.new(mat_name)
    mat.use_nodes = True
    mat.diffuse_color = adjusted_color(source_base_color(src_mat), variation["saturation"], variation["value"])
    nodes = mat.node_tree.nodes
    links = mat.node_tree.links
    nodes.clear()
    output = nodes.new("ShaderNodeOutputMaterial")
    output.location = (520, 0)
    bsdf = nodes.new("ShaderNodeBsdfPrincipled")
    bsdf.location = (260, 0)
    links.new(bsdf.outputs["BSDF"], output.inputs["Surface"])

    set_input(bsdf, ["Base Color"], mat.diffuse_color)
    set_input(bsdf, ["Metallic"], 0.0)
    set_input(bsdf, ["Roughness"], variation["roughness"])
    set_input(bsdf, ["Alpha"], 1.0)
    set_input(bsdf, ["Specular IOR Level", "Specular"], variation["specular_ior_level"])
    set_input(bsdf, ["Coat Weight", "Coat"], variation["coat_weight"])
    set_input(bsdf, ["Coat Roughness"], variation["coat_roughness"])
    set_input(bsdf, ["Subsurface Weight", "Subsurface"], variation["subsurface_weight"])
    set_input(bsdf, ["Subsurface Radius"], tuple(variation["subsurface_radius"]))
    set_input(bsdf, ["Subsurface Scale"], 0.08)
    set_input(bsdf, ["Subsurface Color"], mat.diffuse_color)

    image = get_first_image(src_mat)
    if image is not None:
        tex = nodes.new("ShaderNodeTexImage")
        tex.location = (-760, 90)
        tex.image = image
        hsv = nodes.new("ShaderNodeHueSaturation")
        hsv.location = (-520, 90)
        hsv.inputs["Saturation"].default_value = variation["saturation"]
        hsv.inputs["Value"].default_value = variation["value"]
        bc = nodes.new("ShaderNodeBrightContrast")
        bc.location = (-280, 90)
        bc.inputs["Bright"].default_value = 0.0
        bc.inputs["Contrast"].default_value = variation["contrast"]
        links.new(tex.outputs["Color"], hsv.inputs["Color"])
        links.new(hsv.outputs["Color"], bc.inputs["Color"])
        links.new(bc.outputs["Color"], bsdf.inputs["Base Color"])

    if variation["bump_strength"] > 0:
        noise = nodes.new("ShaderNodeTexNoise")
        noise.location = (-520, -230)
        noise.inputs["Scale"].default_value = 55.0
        noise.inputs["Detail"].default_value = 2.0
        noise.inputs["Roughness"].default_value = 0.42
        bump = nodes.new("ShaderNodeBump")
        bump.location = (-240, -230)
        bump.inputs["Strength"].default_value = variation["bump_strength"]
        bump.inputs["Distance"].default_value = variation["bump_distance"]
        links.new(noise.outputs["Fac"], bump.inputs["Height"])
        if "Normal" in bsdf.inputs:
            links.new(bump.outputs["Normal"], bsdf.inputs["Normal"])

    mat["rubber_recipe"] = json.dumps(variation, sort_keys=True)
    return mat


def make_text(label, location, size=0.16):
    data = bpy.data.curves.new("Text_" + label.replace(" ", "_"), "FONT")
    obj = bpy.data.objects.new("Label_" + label.replace(" ", "_"), data)
    bpy.context.scene.collection.objects.link(obj)
    obj.location = location
    obj.rotation_euler = (math.radians(78), 0, 0)
    obj.name = "Label_" + label.replace(" ", "_")
    data.body = label
    data.align_x = "CENTER"
    data.align_y = "CENTER"
    data.size = size
    mat = bpy.data.materials.get("Label_Black") or bpy.data.materials.new("Label_Black")
    mat.diffuse_color = (0.02, 0.02, 0.02, 1)
    mat.use_nodes = True
    bsdf = get_principled(mat)
    if bsdf:
        set_input(bsdf, ["Base Color"], (0.02, 0.02, 0.02, 1.0))
        set_input(bsdf, ["Roughness"], 0.8)
    data.materials.append(mat)
    return obj


def make_backdrop(name, width, height, center_z):
    mesh = bpy.data.meshes.new(name + "_Mesh")
    verts = [(-width / 2, 0.85, center_z - height / 2), (width / 2, 0.85, center_z - height / 2), (width / 2, 0.85, center_z + height / 2), (-width / 2, 0.85, center_z + height / 2)]
    faces = [(0, 1, 2, 3)]
    mesh.from_pydata(verts, [], faces)
    mesh.update()
    obj = bpy.data.objects.new(name, mesh)
    bpy.context.scene.collection.objects.link(obj)
    mat = bpy.data.materials.new(name + "_Mat")
    mat.use_nodes = True
    mat.node_tree.nodes.clear()
    output = mat.node_tree.nodes.new("ShaderNodeOutputMaterial")
    emission = mat.node_tree.nodes.new("ShaderNodeEmission")
    emission.inputs["Color"].default_value = (0.92, 0.93, 0.95, 1.0)
    emission.inputs["Strength"].default_value = 1.0
    mat.node_tree.links.new(emission.outputs["Emission"], output.inputs["Surface"])
    obj.data.materials.append(mat)
    return obj


def make_image_panel(image_path, label, x, z=0.0, height=2.25):
    width = 1.65
    if image_path.exists():
        img = bpy.data.images.load(str(image_path), check_existing=True)
        if img.size[0] and img.size[1]:
            width = height * (img.size[0] / img.size[1])
    mesh = bpy.data.meshes.new(label + "_Mesh")
    verts = [(-width / 2, 0, 0), (width / 2, 0, 0), (width / 2, 0, height), (-width / 2, 0, height)]
    faces = [(0, 1, 2, 3)]
    mesh.from_pydata(verts, [], faces)
    mesh.update()
    uv_layer = mesh.uv_layers.new(name="UVMap")
    uv_values = [(0, 0), (1, 0), (1, 1), (0, 1)]
    for loop_index, uv in zip(mesh.polygons[0].loop_indices, uv_values):
        uv_layer.data[loop_index].uv = uv
    obj = bpy.data.objects.new(label, mesh)
    bpy.context.scene.collection.objects.link(obj)
    obj.location = (x, 0, z)
    mat = bpy.data.materials.new(label + "_Mat")
    mat.use_nodes = True
    nodes = mat.node_tree.nodes
    nodes.clear()
    output = nodes.new("ShaderNodeOutputMaterial")
    emission = nodes.new("ShaderNodeEmission")
    emission.inputs["Strength"].default_value = 1.0
    if image_path.exists():
        tex = nodes.new("ShaderNodeTexImage")
        tex.image = bpy.data.images.load(str(image_path), check_existing=True)
        mat.node_tree.links.new(tex.outputs["Color"], emission.inputs["Color"])
    else:
        emission.inputs["Color"].default_value = (0.15, 0.15, 0.15, 1.0)
    mat.node_tree.links.new(emission.outputs["Emission"], output.inputs["Surface"])
    mat.use_backface_culling = False
    obj.data.materials.append(mat)
    make_text(label, (x, -0.05, z + height + 0.22), size=0.13)
    return obj


def build_comparison_scene(template_objects):
    scene = bpy.context.scene
    scene.name = "Rubber_Comparison_Grid"
    set_render_defaults(scene, (2600, 1500), 1)
    add_flat_lighting(scene, center=(0, 0, 2.9))
    make_backdrop("Comparison_Backdrop", 12.4, 6.5, 2.95)
    xs = [-4.5, -1.5, 1.5, 4.5]
    top_z = 3.05
    bottom_z = 0.0
    model_height = 2.25
    make_image_panel(SOURCE_IMAGE, "Reference image", xs[0], z=top_z, height=model_height)
    raw_root, raw_coll, _ = duplicate_group(template_objects, "Raw_Pixal3D_GLB", (xs[1], 0, top_z), raw=True, target_height=model_height)
    make_text("Raw Pixal3D GLB", (xs[1], -0.05, top_z + model_height + 0.22), size=0.13)
    roots = {"raw": raw_root}
    for idx, variation in enumerate(VARIATIONS):
        if idx < 2:
            x = xs[idx + 2]
            z = top_z
        else:
            x = xs[idx - 2]
            z = bottom_z
        root, coll, objs = duplicate_group(
            template_objects,
            variation["id"],
            (x, 0, z),
            material_builder=make_rubber_material,
            variation=variation,
            target_height=model_height,
        )
        make_text(variation["label"], (x, -0.05, z + model_height + 0.22), size=0.13)
        roots[variation["id"]] = root
    add_camera(scene, "Camera_Comparison_Ortho", (0, -15.5, 3.0), (0, 0, 2.9), 12.2)
    return roots


def render_turntable_variation(scene, template_objects, variation, filepath, persistent_objects):
    set_render_defaults(scene, (1280, 1280), 72)
    for obj in scene.objects:
        if obj.name not in persistent_objects and obj.type != "LIGHT":
            obj.hide_render = True
            obj.hide_viewport = True
    for obj in scene.objects:
        if obj.name in persistent_objects and obj.type != "LIGHT":
            obj.hide_render = True
            obj.hide_viewport = True

    root, coll, objs = duplicate_group(
        template_objects,
        "Turntable_" + variation["id"],
        (0, 0, 0),
        material_builder=make_rubber_material,
        variation=variation,
    )
    backdrop = make_backdrop("Turntable_Backdrop_" + variation["id"], 4.0, 3.8, 1.45)
    root.rotation_euler = (0, 0, math.radians(180))
    scene.frame_set(1)
    root.keyframe_insert(data_path="rotation_euler", frame=1)
    scene.frame_set(72)
    root.rotation_euler = (0, 0, math.radians(540))
    root.keyframe_insert(data_path="rotation_euler", frame=72)
    # Blender 5.1 moved Action internals; default interpolation is acceptable for
    # review turntables and keeps the script version-tolerant.
    add_camera(scene, "Camera_Turntable_" + variation["id"], (0, -5.2, 1.55), (0, 0, 1.35), 3.25)
    label = make_text(variation["label"], (0, -0.05, 2.95), size=0.13)
    frame_dir = filepath
    frame_dir.mkdir(parents=True, exist_ok=True)
    for old_frame in frame_dir.glob("*.png"):
        old_frame.unlink()
    frame_prefix = frame_dir / (variation["id"] + "_")
    render_scene(scene, frame_prefix)
    for obj in objs + [root, label, scene.camera, backdrop]:
        obj.hide_render = True
        obj.hide_viewport = True
    return {
        "variation": variation["id"],
        "frames_dir": str(frame_dir),
        "frame_prefix": str(frame_prefix),
        "expected_pattern": str(frame_dir / (variation["id"] + "_%04d.png")),
    }


def render_scene(scene, filepath):
    scene.render.filepath = str(filepath)
    if scene.frame_end <= 1:
        scene.render.image_settings.file_format = "PNG"
        bpy.ops.render.render(write_still=True)
    else:
        scene.render.image_settings.file_format = "PNG"
        bpy.ops.render.render(animation=True)


def write_recipe(blender_inputs_seen):
    data = {
        "asset": {
            "asset_id": "Hero_1_Chad_Male",
            "source_glb": str(SOURCE_GLB),
            "source_image": str(SOURCE_IMAGE),
            "lookdev_blend": str(BLEND_PATH),
        },
        "lighting_rig": {
            "world_color": [1.0, 1.0, 1.0],
            "render_engine": "BLENDER_EEVEE_NEXT fallback BLENDER_EEVEE",
            "view_transform": "Standard",
            "look": "Medium High Contrast",
            "lights": [
                {"name": "Key_Softbox_L", "type": "AREA", "location": [-5.5, -5.2, 5.0], "energy": 470.0, "size": 6.5, "shadows": False},
                {"name": "Key_Softbox_R", "type": "AREA", "location": [5.5, -5.2, 4.6], "energy": 360.0, "size": 7.0, "shadows": False},
                {"name": "Top_Soft_Fill", "type": "AREA", "location": [0.0, -1.5, 7.2], "energy": 230.0, "size": 8.0, "shadows": False},
                {"name": "Front_Fill", "type": "AREA", "location": [0.0, -7.5, 2.1], "energy": 150.0, "size": 9.0, "shadows": False},
            ],
        },
        "variations": VARIATIONS,
        "top_candidates": [v["id"] for v in VARIATIONS if v["top_candidate"]],
        "blender_principled_inputs_seen": blender_inputs_seen,
        "ue5_mapping": {
            "base_color": "Use generated texture, then HSV/saturation/value and contrast flattening before Base Color.",
            "roughness": "Set material Roughness to the chosen variation value.",
            "specular": "Approximate Blender Specular IOR Level with UE Specular in 0-1 range; keep metallic 0.",
            "clearcoat": "Map Coat Weight to UE Clear Coat and Coat Roughness to Clear Coat Roughness.",
            "subsurface": "Use Subsurface Profile or Subsurface shading model for skin/body-like colored pieces; use low opacity/tint and keep radius as recipe note.",
            "edge_softness": "Use bevel modifier result or mesh bevels; in UE use bevelled geometry or weighted normals, not a post-process trick.",
            "bump": "Use a very low-amplitude noise normal only if the surface reads too CG-smooth; this is secondary.",
        },
    }
    RECIPE_JSON.write_text(json.dumps(data, indent=2), encoding="utf-8")
    lines = [
        "# Hero 1 Chad Male Rubber Look-Dev Recipe",
        "",
        "## Source",
        "",
        f"- Asset: `Hero_1_Chad_Male`",
        f"- Source GLB: `{SOURCE_GLB}`",
        f"- Source image: `{SOURCE_IMAGE}`",
        f"- Blender file: `{BLEND_PATH}`",
        "- Provenance basis: the look-dev scene imports directly from the FriendSlop Pixal3D source GLB above, not from any archived AccuRig asset.",
        "",
        "## Locked Lighting Rig",
        "",
        "- Engine: EEVEE Next when available, EEVEE fallback.",
        "- World color: white `(1, 1, 1)`.",
        "- View transform: `Standard`, look `Medium High Contrast`, exposure `0`, gamma `1`.",
        "- Shadows are disabled on all area lights; ambient occlusion is weak and used only for form readability.",
        "- Lights:",
        "  - `Key_Softbox_L`: area, location `(-5.5, -5.2, 5.0)`, energy `470`, size `6.5`, shadows off.",
        "  - `Key_Softbox_R`: area, location `(5.5, -5.2, 4.6)`, energy `360`, size `7.0`, shadows off.",
        "  - `Top_Soft_Fill`: area, location `(0, -1.5, 7.2)`, energy `230`, size `8.0`, shadows off.",
        "  - `Front_Fill`: area, location `(0, -7.5, 2.1)`, energy `150`, size `9.0`, shadows off.",
        "",
        "## Material Node Graph",
        "",
        "Per original material slot:",
        "",
        "1. Image Texture from the imported GLB material, when present.",
        "2. Hue/Saturation/Value node.",
        "3. Bright/Contrast node with negative contrast to reduce Pixal3D texture noise.",
        "4. Principled BSDF.",
        "5. Optional Noise Texture -> Bump -> Principled Normal, very low strength.",
        "6. Material Output.",
        "",
        "Geometry softness is a non-destructive Bevel modifier plus Weighted Normal modifier per rubber object.",
        "",
        "## Variation Matrix",
        "",
        "| ID | Roughness | Specular IOR | Coat | Coat Rough | Subsurface | Saturation | Value | Contrast | Bevel | Bump | Notes |",
        "|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---|",
    ]
    for v in VARIATIONS:
        lines.append(
            f"| {v['id']} | {v['roughness']} | {v['specular_ior_level']} | {v['coat_weight']} | "
            f"{v['coat_roughness']} | {v['subsurface_weight']} | {v['saturation']} | {v['value']} | "
            f"{v['contrast']} | {v['bevel_width']} | {v['bump_strength']} | {v['notes']} |"
        )
    lines.extend(
        [
            "",
            "## Recommended First Review Candidates",
            "",
            "- `V02_rubber_pop`: balanced target.",
            "- `V03_vinyl_bounce`: stronger broad highlights if the model still reads too matte.",
            "- `V04_candy_rubber`: softest/bounciest static read.",
            "- `V06_toy_vinyl_gloss`: upper gloss boundary; useful if Pablo wants more toy/vinyl.",
            "",
            "## UE5 Port",
            "",
            "This is a portable parameter mapping, not a direct Blender node-graph transfer. UE5 subsurface, clear coat, and sheen/fresnel reads will need final tuning under the matching UE flat rig because they are not 1:1 with Blender.",
            "",
            "- Metallic: `0`.",
            "- Roughness: use the chosen variation value directly.",
            "- Specular: approximate Blender `Specular IOR Level` as UE `Specular` in the same 0-1 range, then tune under the UE flat rig.",
            "- Clear Coat: use Blender `Coat Weight`; Clear Coat Roughness: use Blender `Coat Roughness`.",
            "- Subsurface: use a Subsurface Profile or Subsurface shading model for the soft body/skin-like pieces; start from `Subsurface Weight` and the radius triplet as color-channel guidance.",
            "- Texture flattening: reduce contrast/noise in the base texture before Base Color; do not bake specular highlights into the texture.",
            "- Edge softness: bevelled geometry or weighted normals should carry the rubber edge catch.",
        ]
    )
    RECIPE_MD.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main():
    ensure_dirs()
    clear_scene()
    template_objects = import_glb_template()
    normalize_group(template_objects)
    template_coll = bpy.data.collections.new("Source_Template_Hidden")
    bpy.context.scene.collection.children.link(template_coll)
    for obj in template_objects:
        for coll in list(obj.users_collection):
            coll.objects.unlink(obj)
        template_coll.objects.link(obj)
        obj.hide_viewport = True
        obj.hide_render = True

    blender_inputs_seen = {}
    for mat in bpy.data.materials:
        bsdf = get_principled(mat)
        if bsdf:
            blender_inputs_seen[mat.name] = list(bsdf.inputs.keys())

    roots = build_comparison_scene(template_objects)
    persistent_objects = set(obj.name for obj in bpy.context.scene.objects)
    grid_path = RENDER_ROOT / "Hero_1_Chad_Male_rubber_comparison_grid.png"
    render_scene(bpy.context.scene, grid_path)

    turntable_outputs = []
    for variation in VARIATIONS:
        if variation["top_candidate"]:
            frame_dir = RENDER_ROOT / "Frames" / variation["id"]
            turntable_outputs.append(render_turntable_variation(bpy.context.scene, template_objects, variation, frame_dir, persistent_objects))

    set_render_defaults(bpy.context.scene, (2600, 1500), 1)
    for obj in bpy.context.scene.objects:
        if obj.name in persistent_objects:
            obj.hide_render = False
            obj.hide_viewport = False
    for obj in template_objects:
        obj.hide_render = True
        obj.hide_viewport = True
    if "Camera_Comparison_Ortho" in bpy.data.objects:
        bpy.context.scene.camera = bpy.data.objects["Camera_Comparison_Ortho"]

    write_recipe(blender_inputs_seen)
    bpy.ops.wm.save_as_mainfile(filepath=str(BLEND_PATH))

    summary = {
        "blend_path": str(BLEND_PATH),
        "comparison_grid": str(grid_path),
        "turntables": turntable_outputs,
        "recipe_json": str(RECIPE_JSON),
        "recipe_md": str(RECIPE_MD),
        "source_glb": str(SOURCE_GLB),
        "variations": [v["id"] for v in VARIATIONS],
    }
    (LOOKDEV_ROOT / "build_summary.json").write_text(json.dumps(summary, indent=2), encoding="utf-8")
    print("T66_RUBBER_LOOKDEV_SUMMARY=" + json.dumps(summary, sort_keys=True))


if __name__ == "__main__":
    main()
