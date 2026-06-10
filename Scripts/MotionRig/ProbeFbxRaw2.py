# Copyright Tribulation 66. All Rights Reserved.
#
# RAW FBX probe part 2: declared unit scale + mesh vertex extents + armature
# object node transform. Together with ProbeFbxRaw.py this pins down exactly
# which data the exporter unit-converted.
#
#   blender.exe --background --factory-startup --python ProbeFbxRaw2.py -- <file.fbx>

import sys

from io_scene_fbx import parse_fbx

argv = sys.argv[sys.argv.index("--") + 1:]
fbx_path = argv[0]

root, _version = parse_fbx.parse(fbx_path)


def find(elem, name):
    return [e for e in elem.elems if e.id == name.encode()]


# Declared units
for gs in find(root, "GlobalSettings"):
    for p70 in find(gs, "Properties70"):
        for p in p70.elems:
            if p.props[0] in (b"UnitScaleFactor", b"OriginalUnitScaleFactor"):
                print(f"FBX_RAW2 {p.props[0].decode()}={float(p.props[4])}")

objects = find(root, "Objects")[0]

# Mesh vertex z-extent (raw numbers)
for e in objects.elems:
    if e.id == b"Geometry":
        verts = find(e, "Vertices")
        if verts:
            v = verts[0].props[0]
            zs = [v[i] for i in range(2, len(v), 3)]
            print(f"FBX_RAW2 mesh vertices n={len(v)//3} zmin={min(zs):.3f} zmax={max(zs):.3f}")

# Model nodes: name + LclScaling + LclTranslation (armature/mesh objects + a bone or two)
for e in objects.elems:
    if e.id == b"Model":
        name = e.props[1].split(b"\x00")[0].decode(errors="replace")
        klass = e.props[2].decode(errors="replace") if isinstance(e.props[2], bytes) else str(e.props[2])
        scaling = translation = None
        for p70 in find(e, "Properties70"):
            for p in p70.elems:
                if p.props[0] == b"Lcl Scaling":
                    scaling = tuple(round(float(x), 4) for x in p.props[4:7])
                elif p.props[0] == b"Lcl Translation":
                    translation = tuple(round(float(x), 4) for x in p.props[4:7])
        if klass in ("Mesh", "Null", "Root") or name in ("MotionRigArmature", "MotionRigMesh", "pelvis", "head"):
            print(f"FBX_RAW2 model name={name} class={klass} scaling={scaling} translation={translation}")
