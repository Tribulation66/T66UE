# Copyright Tribulation 66. All Rights Reserved.
#
# RAW FBX probe — no importer interpretation. Uses Blender's own binary FBX
# parser to dump, for a given bone: the LimbNode's Lcl Translation property
# (the rest pose as written) and the animation-curve key values feeding its
# translation. This is the unit ground truth the UE importer actually reads.
#
#   blender.exe --background --factory-startup --python ProbeFbxRaw.py -- <file.fbx> [bone]

import sys

from io_scene_fbx import parse_fbx

argv = sys.argv[sys.argv.index("--") + 1:]
fbx_path = argv[0]
bone = argv[1] if len(argv) > 1 else "pelvis"

root, _version = parse_fbx.parse(fbx_path)


def find(elem, name):
    return [e for e in elem.elems if e.id == name.encode()]


objects = find(root, "Objects")[0]
connections = find(root, "Connections")[0]

models = {}       # id -> name
curve_nodes = {}  # id -> name
curves = {}       # id -> key values tuple

for e in objects.elems:
    if e.id == b"Model":
        uid, name_class = e.props[0], e.props[1]
        models[uid] = name_class.split(b"\x00")[0].decode(errors="replace")
    elif e.id == b"AnimationCurveNode":
        uid, name_class = e.props[0], e.props[1]
        curve_nodes[uid] = name_class.split(b"\x00")[0].decode(errors="replace")
    elif e.id == b"AnimationCurve":
        uid = e.props[0]
        kv = find(e, "KeyValueFloat")
        curves[uid] = kv[0].props[0] if kv else ()

# connections: C: "OP", src, dst, prop
op_links = []
for c in connections.elems:
    if c.id == b"C" and c.props[0] == b"OP":
        op_links.append((c.props[1], c.props[2], c.props[3]))

target_model_ids = {uid for uid, n in models.items() if n == bone}
if not target_model_ids:
    print(f"FBX_RAW bone '{bone}' not found; models sample={list(models.values())[:8]}")

# rest pose: Lcl Translation property on the model node
for e in objects.elems:
    if e.id == b"Model" and e.props[0] in target_model_ids:
        for p70 in find(e, "Properties70"):
            for p in p70.elems:
                if p.props[0] == b"Lcl Translation":
                    vals = tuple(round(float(v), 4) for v in p.props[4:7])
                    print(f"FBX_RAW {bone} LclTranslation={vals}")

# translation curve nodes attached to the bone
t_nodes = {src for src, dst, prop in op_links
           if dst in target_model_ids and prop == b"Lcl Translation"}
print(f"FBX_RAW {bone} translation_curve_nodes={len(t_nodes)}")

for src, dst, prop in op_links:
    if dst in t_nodes and src in curves:
        vals = curves[src]
        sample = tuple(round(float(v), 4) for v in list(vals)[:4])
        print(f"FBX_RAW {bone} curve[{prop.decode(errors='replace')}] n={len(vals)} sample={sample}")
