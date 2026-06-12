# Build the FallGuysShapeKit01: flat candy platform prisms for the tower course
# shape-language pass (T66_MAP_DESIGN_REFERENCE.md section 1.6).
# Run: blender --background --factory-startup --python build_fallguys_shape_kit.py
#
# Canonical sizing contract (matches T66TowerMapTerrain basic-cube scale math):
#   every mesh has an EXACT 100x100x100 AABB centered at origin, so UE spawn
#   scales (HalfExtents/50) and the box-gap traversal proof stay exact.
#   SM_FGShape_Hex : hexagonal prism, corners +-50 X, flats +-50 Y (squashed
#                    regular hex — 13% squash is invisible at game scale and
#                    buys the exact AABB).
#   SM_FGShape_Tri : triangular prism, base edge -50..50 X at Y=-50, apex Y=+50.
#
# Collision is generated at UE import time as a 1-hull convex decomposition —
# the prisms are convex, so the hull IS the mesh (exact-collision contract).

import bpy
import os

OUT_DIR = r"C:\UE\T66\SourceAssets\Import\WorldKit\FallGuysShapeKit01\Meshes"
BLEND_PATH = r"C:\UE\T66\Model Generation\Runs\Environment\FallGuysShapeKit01\Blender\FallGuysShapeKit01.blend"
os.makedirs(OUT_DIR, exist_ok=True)
os.makedirs(os.path.dirname(BLEND_PATH), exist_ok=True)

# meters in Blender; GLB import into UE lands cm via the standard x100.
HALF = 0.5
HEIGHT_HALF = 0.5


def clear_scene():
    for obj in list(bpy.data.objects):
        bpy.data.objects.remove(obj, do_unlink=True)
    for mesh in list(bpy.data.meshes):
        if mesh.users == 0:
            bpy.data.meshes.remove(mesh)


def make_material(name, rgba):
    mat = bpy.data.materials.get(name) or bpy.data.materials.new(name)
    mat.use_nodes = True
    bsdf = mat.node_tree.nodes.get("Principled BSDF")
    if bsdf:
        bsdf.inputs["Base Color"].default_value = rgba
        bsdf.inputs["Roughness"].default_value = 0.45
    return mat


def build_prism(name, profile_2d):
    """Extrude a closed 2D profile (list of (x, y) in meters) into a Z prism."""
    verts = []
    count = len(profile_2d)
    for x, y in profile_2d:
        verts.append((x, y, -HEIGHT_HALF))
    for x, y in profile_2d:
        verts.append((x, y, HEIGHT_HALF))

    faces = [list(range(count - 1, -1, -1))]              # bottom (wound down)
    faces.append([count + i for i in range(count)])        # top
    for i in range(count):                                 # sides
        j = (i + 1) % count
        faces.append([i, j, count + j, count + i])

    mesh = bpy.data.meshes.new(name)
    mesh.from_pydata(verts, [], faces)
    mesh.update()

    # simple planar UVs from XY (color-only candy MIs; just needs valid UVs)
    uv_layer = mesh.uv_layers.new(name="UVMap")
    for loop in mesh.loops:
        x, y, _ = mesh.vertices[loop.vertex_index].co
        uv_layer.data[loop.index].uv = (x + 0.5, y + 0.5)

    obj = bpy.data.objects.new(name, mesh)
    bpy.context.scene.collection.objects.link(obj)
    obj.data.materials.append(make_material(name + "_Mat", (0.98, 0.78, 0.15, 1.0)))
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
    print("[FGShapeKit] exported", path)


clear_scene()

# Hexagonal prism: corners +-50 X, flats +-50 Y (squashed regular hexagon).
HEX_PROFILE = [
    (0.50, 0.0),
    (0.25, 0.50),
    (-0.25, 0.50),
    (-0.50, 0.0),
    (-0.25, -0.50),
    (0.25, -0.50),
]
hex_obj = build_prism("SM_FGShape_Hex", HEX_PROFILE)
export_glb(hex_obj, "SM_FGShape_Hex")

# Triangular prism: generous isoceles filling the AABB.
TRI_PROFILE = [
    (0.50, -0.50),
    (0.0, 0.50),
    (-0.50, -0.50),
]
tri_obj = build_prism("SM_FGShape_Tri", TRI_PROFILE)
export_glb(tri_obj, "SM_FGShape_Tri")

bpy.ops.wm.save_as_mainfile(filepath=BLEND_PATH)
print("[FGShapeKit] done")
