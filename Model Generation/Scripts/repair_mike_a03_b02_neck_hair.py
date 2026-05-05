import argparse
import bmesh
import json
import math
import os
from pathlib import Path

import bpy
from mathutils import Vector


REPAIR_PREFIX = "MikeRepair_"


def parse_args():
    argv = os.sys.argv[os.sys.argv.index("--") + 1 :] if "--" in os.sys.argv else []
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", required=True)
    parser.add_argument("--report", required=True)
    parser.add_argument("--render-dir", required=True)
    parser.add_argument("--blend", required=True)
    parser.add_argument("--stem", required=True)
    parser.add_argument("--resolution", type=int, default=1400)
    parser.add_argument("--variant", default="C01")
    parser.add_argument("--neck-mode", choices=["bridge", "none"], default="bridge")
    parser.add_argument("--hair-mode", choices=["cap", "small_patch", "paint_mesh", "texture_paint", "duplicate_hair", "tufts", "cards", "none"], default="cap")
    return parser.parse_args(argv)


def scene_engine():
    engines = {item.identifier for item in bpy.types.RenderSettings.bl_rna.properties["engine"].enum_items}
    return "BLENDER_EEVEE_NEXT" if "BLENDER_EEVEE_NEXT" in engines else "BLENDER_EEVEE"


def ensure_render_setup():
    scene = bpy.context.scene
    scene.render.engine = scene_engine()
    scene.render.image_settings.file_format = "PNG"
    scene.render.film_transparent = False
    scene.view_settings.view_transform = "Standard"
    if not scene.world:
        scene.world = bpy.data.worlds.new("ChadRepairWorld")
    scene.world.use_nodes = True
    bg = scene.world.node_tree.nodes["Background"]
    bg.inputs[0].default_value = (0.18, 0.18, 0.18, 1.0)
    bg.inputs[1].default_value = 0.8

    if not bpy.data.objects.get("Assembly_Key_Light"):
        bpy.ops.object.light_add(type="AREA", location=(0.0, -4.2, 4.5))
        key = bpy.context.object
        key.name = "Assembly_Key_Light"
        key.data.energy = 650
        key.data.size = 5.0
    if not bpy.data.objects.get("Assembly_Fill_Light"):
        bpy.ops.object.light_add(type="POINT", location=(-3.0, 2.5, 2.6))
        fill = bpy.context.object
        fill.name = "Assembly_Fill_Light"
        fill.data.energy = 110


def clean_previous_review_objects():
    for obj in list(bpy.context.scene.objects):
        if obj.name.startswith("QA_") or obj.name.startswith(REPAIR_PREFIX):
            bpy.data.objects.remove(obj, do_unlink=True)


def make_material(name, color, roughness=0.72):
    mat = bpy.data.materials.get(name) or bpy.data.materials.new(name)
    mat.use_nodes = True
    bsdf = mat.node_tree.nodes.get("Principled BSDF")
    if bsdf:
        if "Base Color" in bsdf.inputs:
            bsdf.inputs["Base Color"].default_value = color
        if "Roughness" in bsdf.inputs:
            bsdf.inputs["Roughness"].default_value = roughness
        if "Metallic" in bsdf.inputs:
            bsdf.inputs["Metallic"].default_value = 0.0
    mat.diffuse_color = color
    return mat


def create_oval_neck(mat, variant):
    # Coordinate orientation in the review scene: front is -Y, back is +Y.
    # The ring stack deliberately intersects both head and body so no daylight
    # appears at the join after export or rigging.
    if variant == "C17":
        # Slim rear-biased neck graft for A03_B02. This is the "cut down the
        # thick generated neck" idea reduced to the side silhouette: keep it
        # mostly behind the jaw so the good front view is not covered.
        rings = [
            (0.350, 0.038, 0.030, 0.016),
            (0.392, 0.047, 0.036, 0.012),
            (0.438, 0.056, 0.043, 0.006),
            (0.486, 0.064, 0.047, -0.002),
        ]
    elif variant == "C18":
        rings = [
            (0.348, 0.034, 0.028, 0.020),
            (0.392, 0.044, 0.035, 0.016),
            (0.440, 0.052, 0.040, 0.010),
            (0.492, 0.060, 0.043, 0.004),
        ]
    elif variant == "C19":
        # Wider body-owned neck for A03_B02. This intentionally reads from
        # the side profile while staying below the accepted A03 jaw plane.
        rings = [
            (0.342, 0.078, 0.052, -0.018),
            (0.386, 0.084, 0.057, -0.021),
            (0.434, 0.088, 0.061, -0.024),
            (0.486, 0.082, 0.055, -0.026),
            (0.518, 0.068, 0.044, -0.024),
        ]
    elif variant == "C20":
        # More vertical plug, closer to the thick-neck block idea but without
        # importing A04's face texture into the accepted A03 head.
        rings = [
            (0.338, 0.072, 0.050, -0.010),
            (0.386, 0.080, 0.057, -0.014),
            (0.438, 0.086, 0.060, -0.018),
            (0.492, 0.078, 0.052, -0.020),
            (0.526, 0.060, 0.039, -0.018),
        ]
    elif variant == "C21":
        # Head-lift repair: keep the neck mostly below the cheek plane and let
        # the lifted A03 jaw hide the top transition.
        rings = [
            (0.344, 0.070, 0.046, 0.002),
            (0.386, 0.076, 0.051, 0.000),
            (0.430, 0.074, 0.049, -0.002),
            (0.474, 0.064, 0.041, -0.004),
            (0.504, 0.048, 0.030, -0.005),
        ]
    elif variant == "C22":
        rings = [
            (0.342, 0.074, 0.048, 0.006),
            (0.386, 0.080, 0.053, 0.004),
            (0.432, 0.078, 0.051, 0.001),
            (0.480, 0.066, 0.042, -0.002),
            (0.514, 0.050, 0.031, -0.004),
        ]
    elif variant == "C23":
        rings = [
            (0.342, 0.070, 0.048, 0.016),
            (0.386, 0.074, 0.052, 0.013),
            (0.432, 0.068, 0.048, 0.009),
            (0.474, 0.052, 0.037, 0.005),
            (0.504, 0.034, 0.024, 0.002),
        ]
    elif variant == "C24":
        rings = [
            (0.340, 0.074, 0.050, 0.022),
            (0.384, 0.078, 0.054, 0.018),
            (0.430, 0.072, 0.050, 0.012),
            (0.476, 0.052, 0.036, 0.006),
            (0.508, 0.032, 0.023, 0.002),
        ]
    elif variant == "C25":
        rings = [
            (0.340, 0.072, 0.049, 0.018),
            (0.384, 0.076, 0.053, 0.015),
            (0.430, 0.070, 0.049, 0.010),
            (0.474, 0.052, 0.037, 0.005),
            (0.506, 0.034, 0.024, 0.002),
        ]
    elif variant == "C26":
        rings = [
            (0.338, 0.076, 0.052, 0.022),
            (0.384, 0.080, 0.056, 0.018),
            (0.430, 0.074, 0.051, 0.012),
            (0.478, 0.054, 0.037, 0.006),
            (0.510, 0.034, 0.023, 0.002),
        ]
    elif variant == "C13":
        # Lower, body-owned neck insert for the B01 head placement. It sits
        # behind the chin plane and fills the read of a neck without masking
        # the front of the jaw.
        rings = [
            (0.350, 0.090, 0.060, -0.020),
            (0.395, 0.081, 0.054, -0.027),
            (0.442, 0.068, 0.047, -0.032),
            (0.486, 0.054, 0.039, -0.034),
        ]
    elif variant == "C02":
        rings = [
            (0.382, 0.104, 0.067, -0.039),
            (0.425, 0.092, 0.061, -0.042),
            (0.468, 0.076, 0.052, -0.043),
            (0.510, 0.062, 0.044, -0.041),
        ]
    else:
        rings = [
            (0.388, 0.096, 0.061, -0.039),
            (0.430, 0.084, 0.056, -0.041),
            (0.472, 0.071, 0.049, -0.042),
            (0.508, 0.061, 0.043, -0.040),
        ]
    verts = []
    faces = []
    segments = 40
    for _ring_index, (z, rx, ry, cy) in enumerate(rings):
        for i in range(segments):
            a = 2.0 * math.pi * i / segments
            # Slight front broadening reads better under the jaw in the front
            # camera while the rear stays hidden inside the vest/head overlap.
            front_bias = 1.0 if variant in {"C17", "C18", "C19", "C20", "C21", "C22", "C23", "C24", "C25", "C26"} else (1.10 if math.sin(a) < 0 else 0.93)
            verts.append((math.cos(a) * rx, cy + math.sin(a) * ry * front_bias, z))

    for r in range(len(rings) - 1):
        for i in range(segments):
            a = r * segments + i
            b = r * segments + (i + 1) % segments
            c = (r + 1) * segments + (i + 1) % segments
            d = (r + 1) * segments + i
            faces.append((a, b, c, d))

    bottom_center = len(verts)
    verts.append((0.0, rings[0][3], rings[0][0]))
    for i in range(segments):
        faces.append((bottom_center, i, (i + 1) % segments))
    top_center = len(verts)
    top_start = (len(rings) - 1) * segments
    verts.append((0.0, rings[-1][3], rings[-1][0]))
    for i in range(segments):
        faces.append((top_center, top_start + (i + 1) % segments, top_start + i))

    mesh = bpy.data.meshes.new(f"{REPAIR_PREFIX}NeckBridgeMesh_{variant}")
    mesh.from_pydata(verts, [], faces)
    mesh.update()
    obj = bpy.data.objects.new(f"{REPAIR_PREFIX}NeckBridge_{variant}", mesh)
    bpy.context.scene.collection.objects.link(obj)
    obj.data.materials.append(mat)
    for poly in mesh.polygons:
        poly.use_smooth = True
    return obj


def create_hair_back_patch(mat, variant, hair_mode):
    # A shallow curved cap covers the rear scalp diamond without changing the
    # face silhouette. It is intentionally separate so the repair can be
    # visually accepted before merging into a rigged asset.
    if variant == "C09":
        center = Vector((0.0, 0.096, 0.583))
        rx, ry, rz = 0.078, 0.024, 0.050
        theta_min, theta_max = -1.12, 1.12
        phi_min, phi_max = -0.38, 0.84
    elif hair_mode == "small_patch":
        center = Vector((0.0, 0.105, 0.574))
        rx, ry, rz = 0.031, 0.010, 0.038
        theta_min, theta_max = -1.05, 1.05
        phi_min, phi_max = -0.92, 0.86
    elif variant == "C02":
        center = Vector((0.0, 0.087, 0.558))
        rx, ry, rz = 0.082, 0.033, 0.079
        theta_min, theta_max = -1.18, 1.18
        phi_min, phi_max = -0.80, 0.76
    else:
        center = Vector((0.0, 0.089, 0.559))
        rx, ry, rz = 0.074, 0.030, 0.070
        theta_min, theta_max = -1.08, 1.08
        phi_min, phi_max = -0.72, 0.68

    theta_steps = 18
    phi_steps = 14
    verts = []
    faces = []
    for py in range(phi_steps + 1):
        phi = phi_min + (phi_max - phi_min) * py / phi_steps
        for tx in range(theta_steps + 1):
            theta = theta_min + (theta_max - theta_min) * tx / theta_steps
            x = center.x + rx * math.sin(theta) * math.cos(phi)
            y = center.y + ry * math.cos(theta) * math.cos(phi)
            z = center.z + rz * math.sin(phi)
            verts.append((x, y, z))

    for py in range(phi_steps):
        for tx in range(theta_steps):
            a = py * (theta_steps + 1) + tx
            b = a + 1
            c = a + theta_steps + 2
            d = a + theta_steps + 1
            faces.append((a, b, c, d))

    mesh = bpy.data.meshes.new(f"{REPAIR_PREFIX}HairPatchMesh_{variant}")
    mesh.from_pydata(verts, [], faces)
    mesh.update()
    obj = bpy.data.objects.new(f"{REPAIR_PREFIX}HairPatch_{variant}", mesh)
    bpy.context.scene.collection.objects.link(obj)
    obj.data.materials.append(mat)
    for poly in mesh.polygons:
        poly.use_smooth = True
    return obj


def create_rear_hair_tufts(mat, variant):
    if variant == "C07":
        centers = [-0.027, -0.016, -0.006, 0.005, 0.016, 0.027]
        base_z = 0.600
        y = 0.109
        width = 0.012
        tip_drop = [0.034, 0.046, 0.052, 0.043, 0.050, 0.036]
    elif variant == "C06":
        centers = [-0.048, -0.035, -0.022, -0.010, 0.002, 0.016, 0.030, 0.045]
        base_z = 0.604
        y = 0.108
        width = 0.019
        tip_drop = [0.046, 0.057, 0.064, 0.052, 0.068, 0.055, 0.061, 0.048]
    else:
        centers = [-0.040, -0.025, -0.010, 0.006, 0.022, 0.038]
        base_z = 0.602
        y = 0.108
        width = 0.018
        tip_drop = [0.048, 0.060, 0.054, 0.066, 0.052, 0.058]

    verts = []
    faces = []
    for i, cx in enumerate(centers):
        top_left = (cx - width * 0.5, y, base_z + 0.004 * math.sin(i))
        top_right = (cx + width * 0.5, y + 0.001, base_z + 0.002 * math.cos(i))
        tip = (cx + 0.006 * math.sin(i * 1.7), y + 0.007, base_z - tip_drop[i])
        root = len(verts)
        verts.extend([top_left, tip, top_right])
        faces.append((root, root + 1, root + 2))

        # A shorter overlapping side shard breaks up the regular comb shape.
        side = len(verts)
        side_offset = -0.006 if i % 2 == 0 else 0.006
        verts.extend([
            (cx + side_offset - width * 0.30, y + 0.004, base_z - 0.004),
            (cx + side_offset, y + 0.010, base_z - tip_drop[i] * 0.72),
            (cx + side_offset + width * 0.25, y + 0.004, base_z - 0.002),
        ])
        faces.append((side, side + 1, side + 2))

    mesh = bpy.data.meshes.new(f"{REPAIR_PREFIX}RearHairTuftsMesh_{variant}")
    mesh.from_pydata(verts, [], faces)
    mesh.update()
    obj = bpy.data.objects.new(f"{REPAIR_PREFIX}RearHairTufts_{variant}", mesh)
    bpy.context.scene.collection.objects.link(obj)
    obj.data.materials.append(mat)
    return obj


def create_rear_hair_cards(mat, variant):
    # Layered rear cards cover the Trellis-inferred scalp opening using real
    # geometry. The cards overlap and vary in width/drop so the back reads as
    # stylized hair instead of a single painted patch or helmet cap.
    specs = [
        (-0.065, -0.040, 0.628, 0.574, 0.018),
        (-0.048, -0.030, 0.632, 0.548, 0.025),
        (-0.030, -0.020, 0.636, 0.562, 0.021),
        (-0.014, -0.006, 0.638, 0.540, 0.028),
        (0.004, 0.010, 0.637, 0.552, 0.024),
        (0.022, 0.026, 0.635, 0.544, 0.027),
        (0.042, 0.034, 0.631, 0.566, 0.020),
        (0.060, 0.045, 0.627, 0.578, 0.017),
    ]
    verts = []
    faces = []

    def surface_y(x, z, extra=0.0):
        crown = max(0.0, 1.0 - (abs(x) / 0.086) ** 2)
        vertical = max(0.0, min(1.0, (z - 0.545) / 0.095))
        return 0.102 + 0.018 * crown + 0.004 * vertical + extra

    for i, (cx, tip_x, root_z, tip_z, width) in enumerate(specs):
        root_w = width
        mid_w = width * 0.72
        tip_w = width * 0.23
        mid_z = (root_z + tip_z) * 0.52
        mid_x = (cx + tip_x) * 0.5 + (0.004 if i % 2 else -0.003)
        base = len(verts)
        verts.extend([
            (cx - root_w, surface_y(cx - root_w, root_z), root_z),
            (cx + root_w, surface_y(cx + root_w, root_z), root_z - 0.002),
            (mid_x + mid_w, surface_y(mid_x + mid_w, mid_z, 0.004), mid_z),
            (tip_x + tip_w, surface_y(tip_x + tip_w, tip_z, 0.007), tip_z),
            (tip_x - tip_w, surface_y(tip_x - tip_w, tip_z, 0.007), tip_z - 0.001),
            (mid_x - mid_w, surface_y(mid_x - mid_w, mid_z, 0.004), mid_z + 0.002),
        ])
        faces.extend([
            (base, base + 1, base + 2, base + 5),
            (base + 5, base + 2, base + 3, base + 4),
        ])

    # A short upper band tucks the cards under the existing top hair so there
    # is no visible skin strip between generated hair and repair geometry.
    row_top = [(-0.074, 0.622), (-0.047, 0.635), (-0.018, 0.641), (0.016, 0.641), (0.048, 0.634), (0.075, 0.622)]
    row_low = [(-0.078, 0.596), (-0.050, 0.606), (-0.018, 0.613), (0.016, 0.612), (0.050, 0.605), (0.078, 0.594)]
    band = len(verts)
    for x, z in row_top + row_low:
        verts.append((x, surface_y(x, z, 0.002), z))
    for i in range(len(row_top) - 1):
        faces.append((band + i, band + i + 1, band + len(row_top) + i + 1, band + len(row_top) + i))

    mesh = bpy.data.meshes.new(f"{REPAIR_PREFIX}RearHairCardsMesh_{variant}")
    mesh.from_pydata(verts, [], faces)
    mesh.update()
    obj = bpy.data.objects.new(f"{REPAIR_PREFIX}RearHairCards_{variant}", mesh)
    bpy.context.scene.collection.objects.link(obj)
    obj.data.materials.append(mat)
    return obj


def paint_head_back_scalp(mat, variant):
    head = bpy.data.objects.get("Head_geometry_0")
    if not head or head.type != "MESH":
        candidates = [obj for obj in bpy.context.scene.objects if obj.type == "MESH" and obj.name.startswith("Head_")]
        head = candidates[0] if candidates else None
    if not head:
        raise RuntimeError("Could not find head mesh for scalp material repair")

    if mat.name not in [slot.material.name if slot.material else "" for slot in head.material_slots]:
        head.data.materials.append(mat)
    mat_index = [slot.material.name if slot.material else "" for slot in head.material_slots].index(mat.name)

    if variant == "C15":
        radius_x = 0.092
        radius_z = 0.072
        center_z = 0.588
        min_y = 0.050
    elif variant == "C08":
        radius_x = 0.046
        radius_z = 0.027
        center_z = 0.605
        min_y = 0.075
    elif variant == "C05":
        radius_x = 0.030
        radius_z = 0.030
        center_z = 0.589
        min_y = 0.080
    else:
        radius_x = 0.060
        radius_z = 0.052
        center_z = 0.575
        min_y = 0.066

    painted = 0
    for poly in head.data.polygons:
        world_center = head.matrix_world @ poly.center
        # Center rear scalp patch visible in the back camera. The condition is
        # deliberately tight so the natural surrounding hair texture remains.
        nx = abs(world_center.x) / radius_x
        nz = (world_center.z - center_z) / radius_z
        in_oval = (nx * nx + nz * nz) <= 1.0
        if in_oval and world_center.y > min_y:
            poly.material_index = mat_index
            painted += 1
    return head, painted


def point_in_tri(px, py, a, b, c):
    v0x, v0y = c[0] - a[0], c[1] - a[1]
    v1x, v1y = b[0] - a[0], b[1] - a[1]
    v2x, v2y = px - a[0], py - a[1]
    dot00 = v0x * v0x + v0y * v0y
    dot01 = v0x * v1x + v0y * v1y
    dot02 = v0x * v2x + v0y * v2y
    dot11 = v1x * v1x + v1y * v1y
    dot12 = v1x * v2x + v1y * v2y
    denom = dot00 * dot11 - dot01 * dot01
    if abs(denom) < 1e-9:
        return False
    inv = 1.0 / denom
    u = (dot11 * dot02 - dot01 * dot12) * inv
    v = (dot00 * dot12 - dot01 * dot02) * inv
    return u >= -0.02 and v >= -0.02 and (u + v) <= 1.02


def texture_paint_head_back_scalp(variant):
    head = bpy.data.objects.get("Head_geometry_0")
    if not head or head.type != "MESH":
        candidates = [obj for obj in bpy.context.scene.objects if obj.type == "MESH" and obj.name.startswith("Head_")]
        head = candidates[0] if candidates else None
    if not head:
        raise RuntimeError("Could not find head mesh for scalp texture repair")
    mat = head.active_material
    if not mat or not mat.use_nodes:
        raise RuntimeError("Head material has no node tree for texture repair")

    tex_node = None
    for node in mat.node_tree.nodes:
        if node.bl_idname == "ShaderNodeTexImage" and node.label == "BASE COLOR" and node.image:
            tex_node = node
            break
    if not tex_node:
        for node in mat.node_tree.nodes:
            if node.bl_idname == "ShaderNodeTexImage" and node.image:
                tex_node = node
                break
    if not tex_node:
        raise RuntimeError("Could not find head base-color image")

    source = tex_node.image
    image = source.copy()
    image.name = f"{source.name}_{REPAIR_PREFIX}ScalpTexture_{variant}"
    tex_node.image = image

    width, height = image.size
    pixels = list(image.pixels[:])
    uv_layer = head.data.uv_layers.active
    if not uv_layer:
        raise RuntimeError("Head mesh has no active UV layer")

    if variant == "C14":
        radius_x = 0.086
        radius_z = 0.068
        center_z = 0.588
        min_y = 0.052
        blend = 0.60
        hair_color = (0.095, 0.066, 0.043)
    elif variant == "C12":
        radius_x = 0.062
        radius_z = 0.046
        center_z = 0.598
        min_y = 0.066
        blend = 0.46
        hair_color = (0.070, 0.052, 0.038)
    elif variant == "C10":
        radius_x = 0.047
        radius_z = 0.033
        center_z = 0.603
        min_y = 0.075
        blend = 0.58
        hair_color = (0.080, 0.058, 0.040)
    else:
        radius_x = 0.050
        radius_z = 0.038
        center_z = 0.596
        min_y = 0.070
        blend = 0.64
        hair_color = (0.075, 0.055, 0.040)

    painted_faces = 0
    painted_pixels = set()
    for poly in head.data.polygons:
        world_center = head.matrix_world @ poly.center
        nx = abs(world_center.x) / radius_x
        nz = (world_center.z - center_z) / radius_z
        dist2 = nx * nx + nz * nz
        if not (dist2 <= 1.0 and world_center.y > min_y):
            continue
        face_blend = blend
        if variant == "C14":
            dist = min(1.0, math.sqrt(dist2))
            height_bias = max(0.0, min(1.0, (world_center.z - 0.535) / 0.090))
            stripe = 0.86 + 0.14 * math.sin(world_center.x * 210.0 + world_center.z * 90.0)
            face_blend = blend * (0.38 + 0.62 * (1.0 - dist)) * (0.72 + 0.28 * height_bias) * stripe
        elif variant == "C12":
            dist = min(1.0, math.sqrt(dist2))
            face_blend = blend * (0.32 + 0.68 * (1.0 - dist))
        loop_indices = list(poly.loop_indices)
        if len(loop_indices) < 3:
            continue
        uv_points = []
        for li in loop_indices:
            uv = uv_layer.data[li].uv
            uv_points.append((uv.x * (width - 1), uv.y * (height - 1)))
        for i in range(1, len(uv_points) - 1):
            tri = (uv_points[0], uv_points[i], uv_points[i + 1])
            min_x = max(0, int(math.floor(min(p[0] for p in tri))) - 2)
            max_x = min(width - 1, int(math.ceil(max(p[0] for p in tri))) + 2)
            min_y_px = max(0, int(math.floor(min(p[1] for p in tri))) - 2)
            max_y_px = min(height - 1, int(math.ceil(max(p[1] for p in tri))) + 2)
            for y in range(min_y_px, max_y_px + 1):
                for x in range(min_x, max_x + 1):
                    if point_in_tri(x + 0.5, y + 0.5, *tri):
                        idx = (y * width + x) * 4
                        pixels[idx + 0] = pixels[idx + 0] * (1.0 - face_blend) + hair_color[0] * face_blend
                        pixels[idx + 1] = pixels[idx + 1] * (1.0 - face_blend) + hair_color[1] * face_blend
                        pixels[idx + 2] = pixels[idx + 2] * (1.0 - face_blend) + hair_color[2] * face_blend
                        painted_pixels.add((x, y))
        painted_faces += 1

    image.pixels[:] = pixels
    image.update()
    image.pack()
    return head, painted_faces, len(painted_pixels), image.name


def get_head_base_image(head):
    mat = head.active_material
    if not mat or not mat.use_nodes:
        return None
    for node in mat.node_tree.nodes:
        if node.bl_idname == "ShaderNodeTexImage" and node.label == "BASE COLOR" and node.image:
            return node.image
    for node in mat.node_tree.nodes:
        if node.bl_idname == "ShaderNodeTexImage" and node.image:
            return node.image
    return None


def sample_image_luma(image, uv):
    if not image:
        return 1.0
    width, height = image.size
    x = max(0, min(width - 1, int(uv.x * (width - 1))))
    y = max(0, min(height - 1, int(uv.y * (height - 1))))
    idx = (y * width + x) * 4
    r, g, b = image.pixels[idx], image.pixels[idx + 1], image.pixels[idx + 2]
    return 0.2126 * r + 0.7152 * g + 0.0722 * b


def sample_pixel_buffer_luma(pixels, width, height, uv):
    x = max(0, min(width - 1, int(uv.x * (width - 1))))
    y = max(0, min(height - 1, int(uv.y * (height - 1))))
    idx = (y * width + x) * 4
    r, g, b = pixels[idx], pixels[idx + 1], pixels[idx + 2]
    return 0.2126 * r + 0.7152 * g + 0.0722 * b


def duplicate_existing_hair_patch(variant):
    head = bpy.data.objects.get("Head_geometry_0")
    if not head or head.type != "MESH":
        candidates = [obj for obj in bpy.context.scene.objects if obj.type == "MESH" and obj.name.startswith("Head_")]
        head = candidates[0] if candidates else None
    if not head:
        raise RuntimeError("Could not find head mesh for hair duplicate repair")
    uv_layer = head.data.uv_layers.active
    if not uv_layer:
        raise RuntimeError("Head mesh has no active UV layer")
    image = get_head_base_image(head)
    if image:
        width, height = image.size
        image_pixels = list(image.pixels[:])
    else:
        width = height = 0
        image_pixels = None

    max_y = None
    if variant == "C16":
        offset = Vector((0.0, 0.085, -0.055))
        max_abs_x = 0.082
        min_z, max_z = 0.615, 0.655
        min_y = -0.085
        max_y = 0.070
        max_luma = 0.215
    elif variant == "C11":
        offset = Vector((0.0, 0.007, -0.035))
        max_abs_x = 0.080
        min_z, max_z = 0.578, 0.632
        min_y = 0.020
        max_luma = 0.155
    else:
        offset = Vector((0.0, 0.006, -0.030))
        max_abs_x = 0.075
        min_z, max_z = 0.582, 0.632
        min_y = 0.025
        max_luma = 0.165

    verts = []
    faces = []
    uv_values = []
    mat_indices = []
    for poly in head.data.polygons:
        world_center = head.matrix_world @ poly.center
        if abs(world_center.x) > max_abs_x or world_center.y < min_y or not (min_z <= world_center.z <= max_z):
            continue
        if max_y is not None and world_center.y > max_y:
            continue
        loop_indices = list(poly.loop_indices)
        if image_pixels is not None:
            lumas = [sample_pixel_buffer_luma(image_pixels, width, height, uv_layer.data[li].uv) for li in loop_indices]
        else:
            lumas = [sample_image_luma(image, uv_layer.data[li].uv) for li in loop_indices]
        if sum(lumas) / len(lumas) > max_luma:
            continue
        root = len(verts)
        for li in loop_indices:
            vert = head.data.vertices[head.data.loops[li].vertex_index]
            verts.append(tuple((head.matrix_world @ vert.co) + offset))
            uv = uv_layer.data[li].uv
            uv_values.append((uv.x, uv.y))
        faces.append(tuple(range(root, root + len(loop_indices))))
        mat_indices.append(poly.material_index)

    mesh = bpy.data.meshes.new(f"{REPAIR_PREFIX}ExistingHairDuplicateMesh_{variant}")
    mesh.from_pydata(verts, [], faces)
    mesh.update()
    obj = bpy.data.objects.new(f"{REPAIR_PREFIX}ExistingHairDuplicate_{variant}", mesh)
    bpy.context.scene.collection.objects.link(obj)
    for slot in head.material_slots:
        if slot.material:
            obj.data.materials.append(slot.material)
    new_uv = obj.data.uv_layers.new(name="UVMap")
    cursor = 0
    for poly_index, poly in enumerate(obj.data.polygons):
        poly.material_index = mat_indices[poly_index] if mat_indices else 0
        for li in poly.loop_indices:
            new_uv.data[li].uv = uv_values[cursor]
            cursor += 1
    for poly in obj.data.polygons:
        poly.use_smooth = True
    return obj, len(faces)


def object_bounds(objects):
    min_v = Vector((math.inf, math.inf, math.inf))
    max_v = Vector((-math.inf, -math.inf, -math.inf))
    for obj in objects:
        if obj.type != "MESH":
            continue
        for corner in obj.bound_box:
            world = obj.matrix_world @ Vector(corner)
            min_v.x = min(min_v.x, world.x)
            min_v.y = min(min_v.y, world.y)
            min_v.z = min(min_v.z, world.z)
            max_v.x = max(max_v.x, world.x)
            max_v.y = max(max_v.y, world.y)
            max_v.z = max(max_v.z, world.z)
    return min_v, max_v, max_v - min_v


def add_camera(center, size, yaw_deg, pitch_deg=4.0, ortho_scale=None):
    cam_data = bpy.data.cameras.new(name=f"QA_Camera_{yaw_deg:g}")
    cam_data.type = "ORTHO"
    cam = bpy.data.objects.new(cam_data.name, cam_data)
    bpy.context.scene.collection.objects.link(cam)
    bpy.context.scene.camera = cam

    yaw = math.radians(yaw_deg)
    pitch = math.radians(pitch_deg)
    direction = Vector(
        (
            math.sin(yaw) * math.cos(pitch),
            -math.cos(yaw) * math.cos(pitch),
            math.sin(pitch),
        )
    )
    distance = max(size.x, size.y, size.z) * 4.0 + 2.0
    cam.location = center + direction * distance

    target = bpy.data.objects.new(f"QA_Target_{yaw_deg:g}", None)
    target.location = center
    bpy.context.scene.collection.objects.link(target)

    track = cam.constraints.new(type="TRACK_TO")
    track.target = target
    track.track_axis = "TRACK_NEGATIVE_Z"
    track.up_axis = "UP_Y"
    cam_data.ortho_scale = ortho_scale if ortho_scale else max(size.x, size.z) * 1.28
    return cam


def render_views(stem, render_dir, resolution):
    render_dir.mkdir(parents=True, exist_ok=True)
    scene = bpy.context.scene
    scene.render.resolution_x = resolution
    scene.render.resolution_y = resolution
    meshes = [obj for obj in scene.objects if obj.type == "MESH"]
    min_v, max_v, size = object_bounds(meshes)
    center = (min_v + max_v) * 0.5

    outputs = {}
    for label, yaw in {"front": 0.0, "right": 90.0, "back": 180.0, "left": -90.0}.items():
        add_camera(center, size, yaw)
        out = render_dir / f"{stem}_{label}.png"
        scene.render.filepath = str(out)
        bpy.ops.render.render(write_still=True)
        outputs[label] = str(out)

    neck_center = Vector((0.0, -0.02, 0.462))
    neck_size = Vector((0.34, 0.24, 0.22))
    add_camera(neck_center, neck_size, 0.0, pitch_deg=2.0, ortho_scale=0.46)
    out = render_dir / f"{stem}_neck_closeup.png"
    scene.render.filepath = str(out)
    bpy.ops.render.render(write_still=True)
    outputs["neck_closeup"] = str(out)

    add_camera(Vector((0.0, 0.076, 0.558)), Vector((0.24, 0.18, 0.18)), 180.0, pitch_deg=1.0, ortho_scale=0.34)
    out = render_dir / f"{stem}_hair_back_closeup.png"
    scene.render.filepath = str(out)
    bpy.ops.render.render(write_still=True)
    outputs["hair_back_closeup"] = str(out)
    return outputs


def export_glb(path):
    path.parent.mkdir(parents=True, exist_ok=True)
    bpy.ops.object.select_all(action="DESELECT")
    for obj in bpy.context.scene.objects:
        if obj.type in {"MESH", "ARMATURE", "EMPTY"} and not obj.name.startswith("QA_"):
            obj.select_set(True)
    bpy.ops.export_scene.gltf(
        filepath=str(path),
        export_format="GLB",
        use_selection=True,
        export_apply=True,
        export_materials="EXPORT",
    )


def total_triangles(objects):
    total = 0
    for obj in objects:
        if obj.type == "MESH":
            obj.data.calc_loop_triangles()
            total += len(obj.data.loop_triangles)
    return total


def mesh_top(obj):
    return max((obj.matrix_world @ Vector(corner)).z for corner in obj.bound_box)


def maybe_adjust_head(variant):
    shifts = {
        "C21": (0.0, 0.0, 0.030),
        "C22": (0.0, 0.0, 0.042),
        "C23": (0.0, 0.0, 0.034),
        "C24": (0.0, 0.0, 0.038),
        "C25": (0.0, 0.0, 0.036),
        "C26": (0.0, 0.0, 0.040),
    }
    if variant not in shifts:
        return None
    meshes = [obj for obj in bpy.context.scene.objects if obj.type == "MESH" and not obj.name.startswith(REPAIR_PREFIX)]
    if not meshes:
        return None
    head = bpy.data.objects.get("Head_geometry_0") or max(meshes, key=mesh_top)
    dx, dy, dz = shifts[variant]
    head.location.x += dx
    head.location.y += dy
    head.location.z += dz
    return {"object": head.name, "translation": [dx, dy, dz]}


def maybe_carve_body_neck_socket(variant):
    limits_by_variant = {
        "C25": {"x": 0.105, "y_min": -0.015, "y_max": 0.140, "z_min": 0.420, "z_max": 0.560},
        "C26": {"x": 0.130, "y_min": -0.040, "y_max": 0.155, "z_min": 0.405, "z_max": 0.565},
    }
    limits = limits_by_variant.get(variant)
    if not limits:
        return None
    body = bpy.data.objects.get("Body_geometry_0")
    if not body or body.type != "MESH":
        return None

    bm = bmesh.new()
    bm.from_mesh(body.data)
    bm.faces.ensure_lookup_table()
    delete_faces = []
    for face in bm.faces:
        center = body.matrix_world @ face.calc_center_median()
        if abs(center.x) > limits["x"]:
            continue
        if center.y < limits["y_min"] or center.y > limits["y_max"]:
            continue
        if center.z < limits["z_min"] or center.z > limits["z_max"]:
            continue
        delete_faces.append(face)
    if delete_faces:
        bmesh.ops.delete(bm, geom=delete_faces, context="FACES")
    bm.normal_update()
    bm.to_mesh(body.data)
    body.data.update()
    bm.free()
    return {"object": body.name, "deleted_faces": len(delete_faces), "limits": limits}


def main():
    args = parse_args()
    clean_previous_review_objects()
    ensure_render_setup()

    head_adjustment = maybe_adjust_head(args.variant)
    body_socket = maybe_carve_body_neck_socket(args.variant)
    skin_color = (
        (0.50, 0.27, 0.14, 1.0)
        if args.variant in {"C17", "C18", "C19", "C20", "C21", "C22"}
        else (0.58, 0.30, 0.14, 1.0)
        if args.variant in {"C23", "C24", "C25", "C26"}
        else (0.76, 0.39, 0.19, 1.0)
    )
    skin_mat = make_material(f"{REPAIR_PREFIX}SkinMaterial", skin_color, 0.78)
    hair_mat = make_material(f"{REPAIR_PREFIX}HairMaterial", (0.075, 0.055, 0.040, 1.0), 0.85)

    neck = create_oval_neck(skin_mat, args.variant) if args.neck_mode == "bridge" else None
    painted_faces = 0
    if args.hair_mode == "paint_mesh":
        hair = None
        _head, painted_faces = paint_head_back_scalp(hair_mat, args.variant)
        painted_pixels = 0
        painted_image = None
    elif args.hair_mode == "texture_paint":
        hair = None
        _head, painted_faces, painted_pixels, painted_image = texture_paint_head_back_scalp(args.variant)
    elif args.hair_mode == "duplicate_hair":
        hair, duplicated_faces = duplicate_existing_hair_patch(args.variant)
        painted_faces = duplicated_faces
        painted_pixels = 0
        painted_image = None
    elif args.hair_mode == "none":
        hair = None
        painted_pixels = 0
        painted_image = None
    elif args.hair_mode == "tufts":
        hair = create_rear_hair_tufts(hair_mat, args.variant)
        painted_pixels = 0
        painted_image = None
    elif args.hair_mode == "cards":
        hair = create_rear_hair_cards(hair_mat, args.variant)
        painted_pixels = 0
        painted_image = None
    else:
        hair = create_hair_back_patch(hair_mat, args.variant, args.hair_mode)
        painted_pixels = 0
        painted_image = None

    output_path = Path(args.output)
    blend_path = Path(args.blend)
    report_path = Path(args.report)
    render_dir = Path(args.render_dir)

    export_glb(output_path)
    renders = render_views(args.stem, render_dir, args.resolution)
    blend_path.parent.mkdir(parents=True, exist_ok=True)
    bpy.ops.wm.save_as_mainfile(filepath=str(blend_path))

    meshes = [obj for obj in bpy.context.scene.objects if obj.type == "MESH"]
    min_v, max_v, size = object_bounds(meshes)
    payload = {
        "stem": args.stem,
        "variant": args.variant,
        "output_glb": str(output_path),
        "blend": str(blend_path),
        "renders": renders,
        "repair_objects": {
            "neck": neck.name if neck else None,
            "hair_patch": hair.name if hair else None,
            "painted_scalp_faces": painted_faces,
            "painted_texture_pixels": painted_pixels,
            "painted_texture_image": painted_image,
            "head_adjustment": head_adjustment,
            "body_socket": body_socket,
        },
        "triangles": {
            "neck": total_triangles([neck]) if neck else 0,
            "hair_patch": total_triangles([hair]) if hair else 0,
            "scene_total": total_triangles(meshes),
        },
        "bounds": {
            "combined_size": [size.x, size.y, size.z],
        },
        "notes": [
            "Adds a separate skin neck bridge that intersects head and body.",
            "Adds a separate dark rear hair patch to cover the scalp artifact.",
            "Repair geometry is intentionally separate for visual approval before rigging.",
        ],
    }
    report_path.parent.mkdir(parents=True, exist_ok=True)
    report_path.write_text(json.dumps(payload, indent=2), encoding="utf-8")
    print(f"WROTE {report_path}")


if __name__ == "__main__":
    main()
