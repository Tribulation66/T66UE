import math
import os
import sys

import bpy
from mathutils import Vector


def arg_value(name, default=None):
    if "--" not in sys.argv:
        return default
    args = sys.argv[sys.argv.index("--") + 1:]
    for index, arg in enumerate(args):
        if arg == name and index + 1 < len(args):
            return args[index + 1]
        if arg.startswith(name + "="):
            return arg.split("=", 1)[1]
    return default


def clear_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete()


def mat(name, color, roughness=0.8, metallic=0.0):
    material = bpy.data.materials.new(name)
    material.use_nodes = True
    bsdf = material.node_tree.nodes.get("Principled BSDF")
    bsdf.inputs["Base Color"].default_value = color
    bsdf.inputs["Roughness"].default_value = roughness
    bsdf.inputs["Metallic"].default_value = metallic
    return material


def assign(obj, material):
    obj.data.materials.clear()
    obj.data.materials.append(material)
    return obj


def cube(name, loc, scale, material):
    bpy.ops.mesh.primitive_cube_add(size=1, location=loc)
    obj = bpy.context.object
    obj.name = name
    obj.scale = scale
    assign(obj, material)
    return obj


def sphere(name, loc, scale, material, segments=32):
    bpy.ops.mesh.primitive_uv_sphere_add(segments=segments, ring_count=16, location=loc)
    obj = bpy.context.object
    obj.name = name
    obj.scale = scale
    assign(obj, material)
    return obj


def cylinder_between(name, start, end, radius, material, vertices=24):
    start_v = Vector(start)
    end_v = Vector(end)
    center = (start_v + end_v) * 0.5
    direction = end_v - start_v
    length = direction.length
    bpy.ops.mesh.primitive_cylinder_add(vertices=vertices, radius=radius, depth=length, location=center)
    obj = bpy.context.object
    obj.name = name
    obj.rotation_euler = direction.to_track_quat("Z", "Y").to_euler()
    assign(obj, material)
    return obj


def cone_between(name, start, end, radius1, radius2, material, vertices=24):
    start_v = Vector(start)
    end_v = Vector(end)
    center = (start_v + end_v) * 0.5
    direction = end_v - start_v
    length = direction.length
    bpy.ops.mesh.primitive_cone_add(
        vertices=vertices,
        radius1=radius1,
        radius2=radius2,
        depth=length,
        location=center,
    )
    obj = bpy.context.object
    obj.name = name
    obj.rotation_euler = direction.to_track_quat("Z", "Y").to_euler()
    assign(obj, material)
    return obj


def bevel(obj, amount=0.035):
    modifier = obj.modifiers.new("soft_block_bevel", "BEVEL")
    modifier.width = amount
    modifier.segments = 2
    modifier.affect = "EDGES"
    obj.modifiers.new("weighted_normals", "WEIGHTED_NORMAL")
    return obj


def render():
    output = arg_value("--output")
    if not output:
        raise RuntimeError("--output is required")
    os.makedirs(os.path.dirname(output), exist_ok=True)

    clear_scene()

    skin = mat("warm_skin", (0.86, 0.57, 0.38, 1.0), 0.65)
    hair = mat("hero_blond_hair", (1.0, 0.72, 0.12, 1.0), 0.72)
    armor = mat("brushed_silver_armor", (0.78, 0.82, 0.86, 1.0), 0.42, 0.25)
    armor_dark = mat("dark_steel_shadow", (0.20, 0.24, 0.30, 1.0), 0.6, 0.15)
    red = mat("royal_red_cloth", (0.72, 0.07, 0.05, 1.0), 0.78)
    gold = mat("gold_trim", (1.0, 0.70, 0.18, 1.0), 0.45, 0.25)
    leather = mat("brown_leather", (0.34, 0.18, 0.09, 1.0), 0.82)
    black = mat("ink_black", (0.02, 0.018, 0.015, 1.0), 0.8)
    blue = mat("blue_eyes", (0.07, 0.25, 0.85, 1.0), 0.5)

    # T66 Arthur silhouette reference: 2 m tall, very broad A-pose span,
    # oversized upper body, compact waist, short readable legs.
    bevel(cube("wide_chest_plate", (0, 0, 1.22), (0.42, 0.16, 0.34), armor), 0.045)
    bevel(cube("narrow_waist_tunic", (0, 0, 0.86), (0.26, 0.14, 0.22), red), 0.035)
    bevel(cube("neck_guard", (0, 0, 1.56), (0.13, 0.11, 0.11), armor_dark), 0.03)
    bevel(cube("red_scarf", (0, -0.035, 1.48), (0.46, 0.06, 0.055), red), 0.025)
    bevel(cube("cape_hint", (0, 0.11, 1.08), (0.50, 0.055, 0.55), red), 0.035)

    sphere("left_shoulder_plate", (-0.45, 0, 1.42), (0.18, 0.13, 0.14), armor)
    sphere("right_shoulder_plate", (0.45, 0, 1.42), (0.18, 0.13, 0.14), armor)
    cylinder_between("left_upper_arm", (-0.56, 0, 1.35), (-0.83, 0, 1.17), 0.075, skin)
    cylinder_between("left_forearm", (-0.83, 0, 1.17), (-1.02, 0, 1.05), 0.065, skin)
    cylinder_between("right_upper_arm", (0.56, 0, 1.35), (0.83, 0, 1.17), 0.075, skin)
    cylinder_between("right_forearm", (0.83, 0, 1.17), (1.02, 0, 1.05), 0.065, skin)
    sphere("left_fist", (-1.06, -0.005, 1.03), (0.075, 0.065, 0.065), skin)
    sphere("right_fist_grip", (1.06, -0.005, 1.03), (0.075, 0.065, 0.065), skin)

    cylinder_between("left_leg", (-0.15, 0, 0.66), (-0.18, 0, 0.25), 0.095, armor_dark)
    cylinder_between("right_leg", (0.15, 0, 0.66), (0.18, 0, 0.25), 0.095, armor_dark)
    bevel(cube("left_boot", (-0.19, -0.01, 0.12), (0.14, 0.19, 0.08), black), 0.025)
    bevel(cube("right_boot", (0.19, -0.01, 0.12), (0.14, 0.19, 0.08), black), 0.025)

    sphere("chad_head", (0, -0.01, 1.76), (0.19, 0.15, 0.21), skin)
    bevel(cube("square_jaw", (0, -0.025, 1.61), (0.145, 0.11, 0.075), skin), 0.035)
    cone_between("hair_peak_left", (-0.03, -0.015, 1.96), (-0.22, -0.015, 1.86), 0.06, 0.0, hair, 5)
    cone_between("hair_peak_right", (0.02, -0.015, 1.97), (0.22, -0.015, 1.88), 0.06, 0.0, hair, 5)
    sphere("hair_mass", (0, -0.01, 1.91), (0.18, 0.13, 0.08), hair, 24)
    sphere("left_eye", (-0.065, -0.145, 1.78), (0.025, 0.012, 0.018), blue, 16)
    sphere("right_eye", (0.065, -0.145, 1.78), (0.025, 0.012, 0.018), blue, 16)
    bevel(cube("stern_brow", (0, -0.158, 1.825), (0.15, 0.018, 0.018), hair), 0.008)

    # Sword in Arthur's right hand, clearly readable beside the A-pose arm.
    cylinder_between("sword_grip", (1.02, -0.045, 1.0), (1.14, -0.045, 0.92), 0.035, leather, 16)
    cylinder_between("sword_guard", (0.96, -0.05, 1.03), (1.20, -0.05, 0.87), 0.025, gold, 16)
    cone_between("sword_blade", (1.13, -0.055, 0.93), (1.52, -0.055, 0.32), 0.055, 0.01, armor, 4)
    sphere("sword_pommel", (1.0, -0.045, 1.02), (0.045, 0.04, 0.045), gold, 16)

    # Ground-contact shadow plate helps background removal find feet without
    # hiding the body silhouette.
    bevel(cube("soft_contact_shadow", (0, 0.17, 0.035), (0.44, 0.015, 0.018), black), 0.02)

    bpy.ops.object.light_add(type="AREA", location=(0, -3.5, 4.0))
    key = bpy.context.object
    key.name = "large_softbox"
    key.data.energy = 550
    key.data.size = 4.5

    bpy.ops.object.camera_add(location=(0, -5.2, 1.08), rotation=(math.radians(90), 0, 0))
    camera = bpy.context.object
    camera.data.type = "ORTHO"
    camera.data.ortho_scale = 2.65
    bpy.context.scene.camera = camera

    scene = bpy.context.scene
    scene.render.engine = "CYCLES"
    scene.cycles.samples = 96
    scene.render.resolution_x = 1024
    scene.render.resolution_y = 1024
    scene.view_settings.view_transform = "Standard"
    scene.view_settings.look = "Medium High Contrast"
    scene.world.color = (1, 1, 1)
    scene.render.film_transparent = False
    scene.render.filepath = output
    bpy.ops.render.render(write_still=True)


if __name__ == "__main__":
    render()
