# Build the InflatableTraps01 kit: puffy balloon meshes for the T66 obstacle traps.
# Run: blender --background --factory-startup --python build_inflatable_traps.py
#
# Canonical sizing contract (matches T66ObstacleTrap.cpp engine-shape scale math):
#   every mesh fits a 1m native envelope so UE component scales (Dim/100) keep working.
#   SM_Inflatable_SweeperArm : 1m long (X centered), 1m max diameter YZ, pinch seams
#   SM_Inflatable_Hub        : 1m diameter, 1m tall (Z centered), scalloped squat dome
#   SM_Inflatable_Bumper     : 1m diameter, 1m tall (Z centered), donut ring + dome cap
#   SM_Inflatable_Pad        : 1m cube envelope (centered), pillow with weld seam
#   SM_Inflatable_Mallet     : 1m long (X centered), barrel head with bulged caps
#   SM_Inflatable_Tube       : 1m long (Z centered), 1m max diameter, segmented tube
#   SM_Inflatable_SpikeBall  : 1m body diameter (center), puffy studs to ~1.3m (probe)
#
# The inflatable read comes from: fat rounded lobes, pinched seam rings, scallop lobes
# around revolve axes, smooth+weighted normals, and (in UE) the FriendSlop rubber master.

import bpy
import bmesh
import math
import os

OUT_DIR = r"C:\UE\T66\Model Generation\Runs\Environment\InflatableTraps01"
GLB_DIR = os.path.join(OUT_DIR, "UnrealImport")
RENDER_DIR = os.path.join(OUT_DIR, "Renders")
BLEND_PATH = os.path.join(OUT_DIR, "Blender", "InflatableTraps01.blend")
os.makedirs(GLB_DIR, exist_ok=True)
os.makedirs(RENDER_DIR, exist_ok=True)
os.makedirs(os.path.dirname(BLEND_PATH), exist_ok=True)

# ---------------------------------------------------------------------------
# helpers
# ---------------------------------------------------------------------------

def clear_scene():
    for obj in list(bpy.data.objects):
        bpy.data.objects.remove(obj, do_unlink=True)
    for mesh in list(bpy.data.meshes):
        if mesh.users == 0:
            bpy.data.meshes.remove(mesh)


def make_material(name, rgba, roughness=0.45):
    mat = bpy.data.materials.get(name) or bpy.data.materials.new(name)
    mat.use_nodes = True
    bsdf = mat.node_tree.nodes.get("Principled BSDF")
    if bsdf:
        bsdf.inputs["Base Color"].default_value = rgba
        bsdf.inputs["Roughness"].default_value = roughness
    return mat


def finish_object(obj, mat):
    """Smooth shading + weighted normals + material."""
    mesh = obj.data
    mesh.polygons.foreach_set("use_smooth", [True] * len(mesh.polygons))
    mesh.update()
    obj.data.materials.clear()
    obj.data.materials.append(mat)
    mod = obj.modifiers.new("WeightedNormal", "WEIGHTED_NORMAL")
    mod.mode = "FACE_AREA"
    mod.keep_sharp = False
    mod.weight = 50
    bpy.context.view_layer.objects.active = obj
    obj.select_set(True)
    bpy.ops.object.modifier_apply(modifier=mod.name)
    obj.select_set(False)


def lathe(name, profile, axis="X", ring_segments=28, scallop_lobes=0, scallop_amount=0.0):
    """Revolve a profile around an axis with cylindrical UVs.

    profile: list of (t, radius) where t runs along the axis in meters and radius >= 0.
             radius == 0 at the first/last entry produces a closed pole.
    axis:    'X' or 'Z' revolve axis.
    scallop: radial lobes around the axis (r *= 1 + amount * sin(angle * lobes)).
    UVs:     u = angle fraction, v = normalized arc length along the profile.
    """
    verts = []
    faces = []
    uvs = {}  # loop key (face_index, corner) handled after creation

    # arc-length for v coordinate
    arc = [0.0]
    for i in range(1, len(profile)):
        dt = profile[i][0] - profile[i - 1][0]
        dr = profile[i][1] - profile[i - 1][1]
        arc.append(arc[-1] + math.hypot(dt, dr))
    total_arc = max(arc[-1], 1e-6)
    v_coords = [a / total_arc for a in arc]

    ring_index = {}
    pole_index = {}
    for i, (t, radius) in enumerate(profile):
        if radius <= 1e-5:
            if axis == "X":
                verts.append((t, 0.0, 0.0))
            else:
                verts.append((0.0, 0.0, t))
            pole_index[i] = len(verts) - 1
            continue
        ring = []
        for s in range(ring_segments):
            ang = 2.0 * math.pi * s / ring_segments
            r = radius
            if scallop_lobes > 0:
                r *= 1.0 + scallop_amount * math.sin(ang * scallop_lobes)
            if axis == "X":
                verts.append((t, math.cos(ang) * r, math.sin(ang) * r))
            else:
                verts.append((math.cos(ang) * r, math.sin(ang) * r, t))
            ring.append(len(verts) - 1)
        ring_index[i] = ring

    face_uvs = []  # parallel to faces: list of (u, v) per corner
    for i in range(len(profile) - 1):
        a_ring = ring_index.get(i)
        b_ring = ring_index.get(i + 1)
        a_pole = pole_index.get(i)
        b_pole = pole_index.get(i + 1)
        for s in range(ring_segments):
            s2 = (s + 1) % ring_segments
            u0 = s / ring_segments
            u1 = (s + 1) / ring_segments
            va, vb = v_coords[i], v_coords[i + 1]
            if a_ring is not None and b_ring is not None:
                faces.append((a_ring[s], a_ring[s2], b_ring[s2], b_ring[s]))
                face_uvs.append(((u0, va), (u1, va), (u1, vb), (u0, vb)))
            elif a_pole is not None and b_ring is not None:
                faces.append((a_pole, b_ring[s2], b_ring[s]))
                face_uvs.append((((u0 + u1) * 0.5, va), (u1, vb), (u0, vb)))
            elif a_ring is not None and b_pole is not None:
                faces.append((a_ring[s], a_ring[s2], b_pole))
                face_uvs.append(((u0, va), (u1, va), ((u0 + u1) * 0.5, vb)))

    mesh = bpy.data.meshes.new(name)
    mesh.from_pydata(verts, [], faces)
    mesh.update()
    uv_layer = mesh.uv_layers.new(name="UVMap")
    loop_i = 0
    for poly, corner_uvs in zip(mesh.polygons, face_uvs):
        for c in range(poly.loop_total):
            uv_layer.data[poly.loop_start + c].uv = corner_uvs[c]
    obj = bpy.data.objects.new(name, mesh)
    bpy.context.collection.objects.link(obj)
    return obj


def balloon_radius_profile(length, max_radius, pinches, samples=44, pinch_depth=0.34, pinch_width=0.055):
    """Rounded-end sausage with gaussian pinch seams. Returns lathe profile along X."""
    profile = []
    half = length * 0.5
    for i in range(samples + 1):
        t = i / samples
        x = -half + t * length
        # rounded ends: elliptical falloff over the outer 18% of each side
        end = 0.18
        if t < end:
            cap = math.sin((t / end) * math.pi * 0.5)
        elif t > 1.0 - end:
            cap = math.sin(((1.0 - t) / end) * math.pi * 0.5)
        else:
            cap = 1.0
        r = max_radius * (0.55 + 0.45 * cap)
        for p in pinches:
            r *= 1.0 - pinch_depth * math.exp(-((t - p) ** 2) / (2 * pinch_width ** 2))
        if i == 0 or i == samples:
            r = 0.0
        profile.append((x, r))
    return profile


# ---------------------------------------------------------------------------
# meshes
# ---------------------------------------------------------------------------

def build_sweeper_arm(mat):
    profile = balloon_radius_profile(1.0, 0.5, pinches=[0.30, 0.52, 0.74], samples=56)
    obj = lathe("SM_Inflatable_SweeperArm", profile, axis="X", ring_segments=28)
    finish_object(obj, mat)
    return obj


def build_mallet(mat):
    # fat barrel: two big lobes with one deep center seam, bulging caps
    profile = balloon_radius_profile(1.0, 0.5, pinches=[0.5], samples=48, pinch_depth=0.22, pinch_width=0.07)
    obj = lathe("SM_Inflatable_Mallet", profile, axis="X", ring_segments=28)
    finish_object(obj, mat)
    return obj


def build_tube(mat):
    profile = balloon_radius_profile(1.0, 0.5, pinches=[0.25, 0.5, 0.75], samples=52, pinch_depth=0.28)
    # revolve along Z for the hanging cable
    profile_z = [(x, r) for (x, r) in profile]
    obj = lathe("SM_Inflatable_Tube", profile_z, axis="Z", ring_segments=20)
    finish_object(obj, mat)
    return obj


def build_hub(mat):
    # squat dome, Z centered (-0.5..0.5), scalloped lobes around the axis
    prof = []
    samples = 30
    for i in range(samples + 1):
        t = i / samples              # 0 bottom .. 1 top
        z = -0.5 + t
        if t < 0.12:                 # rounded under-lip
            r = 0.42 + 0.08 * math.sin((t / 0.12) * math.pi * 0.5)
        elif t < 0.75:               # belly
            r = 0.5 * (1.0 - 0.05 * math.cos((t - 0.12) * math.pi))
        else:                        # dome to pole
            k = (t - 0.75) / 0.25
            r = 0.5 * math.cos(k * math.pi * 0.5)
        if i == 0 or i == samples:
            r = 0.0
        prof.append((z, max(r, 0.0)))
    obj = lathe("SM_Inflatable_Hub", prof, axis="Z", ring_segments=32, scallop_lobes=8, scallop_amount=0.045)
    finish_object(obj, mat)
    return obj


def build_bumper(mat):
    # donut ring with dome cap, Z centered: profile walks bottom-center -> outer ring -> crease -> dome pole
    prof = [
        (-0.50, 0.0),
        (-0.49, 0.20),
        (-0.45, 0.36),
        (-0.36, 0.47),
        (-0.22, 0.50),   # ring belly
        (-0.08, 0.49),
        (0.04, 0.43),
        (0.12, 0.315),   # deep crease between ring and cap
        (0.16, 0.27),
        (0.20, 0.30),    # cap swells back out
        (0.30, 0.305),
        (0.40, 0.235),
        (0.47, 0.12),
        (0.50, 0.0),     # pole
    ]
    obj = lathe("SM_Inflatable_Bumper", prof, axis="Z", ring_segments=36, scallop_lobes=10, scallop_amount=0.03)
    finish_object(obj, mat)
    return obj


def build_pad(mat):
    # pillow: subdivided cube, rounded by partial cast-to-sphere, with a weld seam ring
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0, 0))
    obj = bpy.context.active_object
    obj.name = "SM_Inflatable_Pad"
    obj.data.name = "SM_Inflatable_Pad"

    bm = bmesh.new()
    bm.from_mesh(obj.data)
    bmesh.ops.subdivide_edges(bm, edges=bm.edges[:], cuts=13, use_grid_fill=True)
    # mattress inflation: every vert bulges along its dominant axes with a dome
    # falloff across the face, plus quilt seam lines at the third-lines of the
    # big +/-X punch faces, plus a perimeter weld seam.
    def dome(a, b):
        return max(0.0, 1.0 - (abs(a) * 2.0) ** 2.0) * max(0.0, 1.0 - (abs(b) * 2.0) ** 2.0)

    PUFF = 0.17
    for v in bm.verts:
        x, y, z = v.co
        wx = (abs(x) / 0.5) ** 1.6
        wy = (abs(y) / 0.5) ** 1.6
        wz = (abs(z) / 0.5) ** 1.6
        # quilt seams only on the punch faces (X): two lines across Y and Z thirds
        seam = 1.0
        for line in (-1.0 / 6.0, 1.0 / 6.0):
            seam *= 1.0 - 0.45 * math.exp(-((y - line * 2.0 * 0.5) ** 2) / (2 * 0.035 ** 2))
            seam *= 1.0 - 0.45 * math.exp(-((z - line * 2.0 * 0.5) ** 2) / (2 * 0.035 ** 2))
        v.co.x = x + math.copysign(PUFF * dome(y, z) * wx * seam, x if abs(x) > 1e-6 else 1.0)
        v.co.y = y + math.copysign(PUFF * 0.6 * dome(x, z) * wy, y if abs(y) > 1e-6 else 1.0)
        v.co.z = z + math.copysign(PUFF * 0.6 * dome(x, y) * wz, z if abs(z) > 1e-6 else 1.0)
    # perimeter weld seam: pinch the band where the side faces meet (|x| small)
    for v in bm.verts:
        if abs(v.co.x) < 0.05:
            v.co.y *= 0.95
            v.co.z *= 0.95
    bm.to_mesh(obj.data)
    bm.free()

    # box-project UVs
    bpy.context.view_layer.objects.active = obj
    obj.select_set(True)
    bpy.ops.object.mode_set(mode="EDIT")
    bpy.ops.mesh.select_all(action="SELECT")
    bpy.ops.uv.cube_project(cube_size=1.0)
    bpy.ops.object.mode_set(mode="OBJECT")
    obj.select_set(False)
    finish_object(obj, mat)
    return obj


def build_spike_ball(mat_body, mat_stud):
    # puffy core with scallop lobes
    prof = []
    samples = 36
    for i in range(samples + 1):
        t = i / samples
        z = -0.5 + t
        r = math.sqrt(max(0.0, 0.25 - z * z)) * 2.0 * 0.5  # sphere radius 0.5
        if i == 0 or i == samples:
            r = 0.0
        prof.append((z, r))
    body = lathe("SpikeBallBody", prof, axis="Z", ring_segments=32, scallop_lobes=8, scallop_amount=0.035)
    finish_object(body, mat_body)

    # puffy studs: rounded cones on icosahedron directions
    bpy.ops.mesh.primitive_ico_sphere_add(subdivisions=1, radius=1.0)
    ico = bpy.context.active_object
    directions = [v.co.normalized().copy() for v in ico.data.vertices]
    bpy.data.objects.remove(ico, do_unlink=True)

    stud_objs = []
    stud_profile = [
        (0.0, 0.0), (0.01, 0.16), (0.06, 0.20), (0.14, 0.17), (0.24, 0.115),
        (0.32, 0.065), (0.37, 0.03), (0.40, 0.0),
    ]
    for i, d in enumerate(directions):
        stud = lathe(f"Stud_{i}", stud_profile, axis="Z", ring_segments=16)
        finish_object(stud, mat_stud)
        quat = d.to_track_quat("Z", "Y")
        stud.rotation_mode = "QUATERNION"
        stud.rotation_quaternion = quat
        stud.location = d * 0.44
        stud_objs.append(stud)

    for o in [body] + stud_objs:
        o.select_set(True)
    bpy.context.view_layer.objects.active = body
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    bpy.ops.object.join()
    joined = bpy.context.active_object
    joined.name = "SM_Inflatable_SpikeBall"
    joined.data.name = "SM_Inflatable_SpikeBall"
    joined.select_set(False)
    return joined


# ---------------------------------------------------------------------------
# build, render QA, export
# ---------------------------------------------------------------------------

clear_scene()

mat_pink = make_material("FG_Pink", (0.95, 0.25, 0.55, 1.0))
mat_yellow = make_material("FG_Yellow", (0.98, 0.80, 0.15, 1.0))
mat_blue = make_material("FG_Blue", (0.20, 0.55, 0.95, 1.0))
mat_red = make_material("FG_Red", (0.92, 0.20, 0.18, 1.0))
mat_purple = make_material("FG_Purple", (0.55, 0.30, 0.90, 1.0))

builders = [
    ("SM_Inflatable_SweeperArm", lambda: build_sweeper_arm(mat_red)),
    ("SM_Inflatable_Hub", lambda: build_hub(mat_blue)),
    ("SM_Inflatable_Bumper", lambda: build_bumper(mat_yellow)),
    ("SM_Inflatable_Pad", lambda: build_pad(mat_pink)),
    ("SM_Inflatable_Mallet", lambda: build_mallet(mat_purple)),
    ("SM_Inflatable_Tube", lambda: build_tube(mat_blue)),
    ("SM_Inflatable_SpikeBall", lambda: build_spike_ball(mat_blue, mat_yellow)),
]

built = []
for name, fn in builders:
    obj = fn()
    obj.name = name
    obj.data.name = name
    built.append(obj)

# line them up for the contact sheet
for i, obj in enumerate(built):
    obj.location = (i * 1.7 - (len(built) - 1) * 0.85, 0.0, 0.6)

# light + camera for QA render
sun = bpy.data.objects.new("Sun", bpy.data.lights.new("Sun", type="SUN"))
sun.data.energy = 4.0
sun.rotation_euler = (math.radians(55), 0, math.radians(25))
bpy.context.collection.objects.link(sun)
world = bpy.data.worlds["World"] if bpy.data.worlds else bpy.data.worlds.new("World")
bpy.context.scene.world = world
world.use_nodes = True
bg = world.node_tree.nodes.get("Background")
if bg:
    bg.inputs[0].default_value = (0.55, 0.55, 0.58, 1.0)
    bg.inputs[1].default_value = 1.1

cam_data = bpy.data.cameras.new("QACam")
cam = bpy.data.objects.new("QACam", cam_data)
bpy.context.collection.objects.link(cam)
cam.location = (0.0, -11.5, 3.0)
cam.rotation_euler = (math.radians(78), 0.0, 0.0)
cam_data.lens = 42
bpy.context.scene.camera = cam

scene = bpy.context.scene
scene.render.engine = "BLENDER_EEVEE"
scene.render.resolution_x = 2048
scene.render.resolution_y = 560
scene.render.filepath = os.path.join(RENDER_DIR, "contact_sheet.png")
bpy.ops.render.render(write_still=True)

# per-mesh GLB export at origin
report = []
for obj in built:
    obj.location = (0.0, 0.0, 0.0)
for obj in built:
    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    glb_path = os.path.join(GLB_DIR, obj.name + ".glb")
    bpy.ops.export_scene.gltf(
        filepath=glb_path,
        use_selection=True,
        export_format="GLB",
        export_yup=True,
        export_apply=True,
        export_materials="NONE",
    )
    mesh = obj.data
    report.append((obj.name, len(mesh.vertices), len(mesh.polygons), os.path.getsize(glb_path)))

bpy.ops.wm.save_as_mainfile(filepath=BLEND_PATH)

print("[InflatableTraps01] BUILD OK")
for name, nv, nf, size in report:
    print(f"[InflatableTraps01] {name}: verts={nv} faces={nf} glb={size//1024}KB")
