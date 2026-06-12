# FallGuysShapeKit01 v2 — BEVELED course prisms (Tier A of FALLGUYS_MAP_ANALYSIS.md:
# rounded chunky geometry is the cheapest big Fall Guys read).
# Run: blender --background --factory-startup --python build_fallguys_bevel_kit.py
#
# Sizing contract unchanged: every mesh has an EXACT 100x100x100 AABB centered at
# origin (UE spawn scales = HalfExtents/50; box-gap proof contacts stay on AABB
# faces — the bevel only CUTS inward, never protrudes).
#   SM_FGShape_BevelCube : cube with 8uu edge bevel (2 segments, smooth bevel)
#   SM_FGShape_BevelPuck : cylinder (32 sides) with 8uu rim bevel top+bottom
#   SM_FGShape_Hex       : RE-AUTHORED squashed hex prism with 8uu bevels (same asset name)
#   SM_FGShape_Tri       : RE-AUTHORED triangular prism with 8uu bevels (same asset name)
# Collision is generated at UE import as a 1-hull convex (convex prisms: hull == mesh).

import bpy
import math
import os

OUT_DIR = r"C:\UE\T66\SourceAssets\Import\WorldKit\FallGuysShapeKit01\Meshes"
BLEND_PATH = r"C:\UE\T66\Model Generation\Runs\Environment\FallGuysShapeKit01\Blender\FallGuysBevelKit.blend"
os.makedirs(OUT_DIR, exist_ok=True)
os.makedirs(os.path.dirname(BLEND_PATH), exist_ok=True)

HALF = 0.5
BEVEL = 0.08  # 8uu at game scale


def clear_scene():
    for obj in list(bpy.data.objects):
        bpy.data.objects.remove(obj, do_unlink=True)
    for mesh in list(bpy.data.meshes):
        if mesh.users == 0:
            bpy.data.meshes.remove(mesh)


def make_material(name):
    mat = bpy.data.materials.get(name) or bpy.data.materials.new(name)
    mat.use_nodes = True
    bsdf = mat.node_tree.nodes.get("Principled BSDF")
    if bsdf:
        bsdf.inputs["Base Color"].default_value = (0.98, 0.78, 0.15, 1.0)
        bsdf.inputs["Roughness"].default_value = 0.45
    return mat


def add_uvs(obj):
    mesh = obj.data
    if not mesh.uv_layers:
        uv_layer = mesh.uv_layers.new(name="UVMap")
        for loop in mesh.loops:
            x, y, _ = mesh.vertices[loop.vertex_index].co
            uv_layer.data[loop.index].uv = (x + 0.5, y + 0.5)


def finish(obj, name):
    obj.name = name
    obj.data.name = name
    bpy.context.view_layer.objects.active = obj
    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    # bevel modifier: 2 segments, clamped, smooth-shaded bevel faces only
    mod = obj.modifiers.new("Bevel", "BEVEL")
    mod.width = BEVEL
    mod.segments = 2
    mod.limit_method = "ANGLE"
    mod.angle_limit = math.radians(40)
    bpy.ops.object.modifier_apply(modifier=mod.name)
    bpy.ops.object.shade_smooth_by_angle(angle=math.radians(35))
    add_uvs(obj)
    obj.data.materials.clear()
    obj.data.materials.append(make_material(name + "_Mat"))


def build_prism(profile_2d):
    verts = []
    count = len(profile_2d)
    for x, y in profile_2d:
        verts.append((x, y, -HALF))
    for x, y in profile_2d:
        verts.append((x, y, HALF))
    faces = [list(range(count - 1, -1, -1))]
    faces.append([count + i for i in range(count)])
    for i in range(count):
        j = (i + 1) % count
        faces.append([i, j, count + j, count + i])
    mesh = bpy.data.meshes.new("Prism")
    mesh.from_pydata(verts, [], faces)
    mesh.update()
    obj = bpy.data.objects.new("Prism", mesh)
    bpy.context.scene.collection.objects.link(obj)
    return obj


def export_glb(obj, name):
    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    path = os.path.join(OUT_DIR, name + ".glb")
    bpy.ops.export_scene.gltf(
        filepath=path,
        use_selection=True,
        export_format="GLB",
        export_yup=True,
        export_apply=True,
    )
    print("[FGBevelKit] exported", path)


clear_scene()

# Beveled cube
bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0, 0))
cube = bpy.context.active_object
finish(cube, "SM_FGShape_BevelCube")
export_glb(cube, "SM_FGShape_BevelCube")

# Beveled puck (cylinder)
bpy.ops.mesh.primitive_cylinder_add(vertices=32, radius=HALF, depth=1.0, location=(0, 0, 0))
puck = bpy.context.active_object
finish(puck, "SM_FGShape_BevelPuck")
export_glb(puck, "SM_FGShape_BevelPuck")

# Hex prism (squashed regular: corners +-50 X, flats +-50 Y) — re-authored with bevel
HEX_PROFILE = [
    (0.50, 0.0),
    (0.25, 0.50),
    (-0.25, 0.50),
    (-0.50, 0.0),
    (-0.25, -0.50),
    (0.25, -0.50),
]
hex_obj = build_prism(HEX_PROFILE)
finish(hex_obj, "SM_FGShape_Hex")
export_glb(hex_obj, "SM_FGShape_Hex")

# Triangular prism — re-authored with bevel
TRI_PROFILE = [
    (0.50, -0.50),
    (0.0, 0.50),
    (-0.50, -0.50),
]
tri_obj = build_prism(TRI_PROFILE)
finish(tri_obj, "SM_FGShape_Tri")
export_glb(tri_obj, "SM_FGShape_Tri")

bpy.ops.wm.save_as_mainfile(filepath=BLEND_PATH)
print("[FGBevelKit] done")
