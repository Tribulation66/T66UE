# Copyright Tribulation 66. All Rights Reserved.
#
# MotionRig split-generation assembly: Pixal3D generates the BODY (headless,
# neck stump) and the HEAD (with neck stub) as separate models so facial
# detail gets full model resolution. This joins them into one GLB for
# BuildMotionRig.py:
#   - scale the head so its neck-stub diameter matches the body's neck-stump
#     diameter
#   - seat the head so the stub sinks into the stump (continuous neck)
#   - join into a single mesh, export combined GLB
#
#   blender.exe --background --factory-startup --python AssembleHeadBody.py -- \
#       --body <body.glb> --head <head.glb> --out <combined.glb>

import argparse
import sys

import bpy
import numpy as np


def parse_args():
    argv = sys.argv
    argv = argv[argv.index("--") + 1:] if "--" in argv else []
    parser = argparse.ArgumentParser()
    parser.add_argument("--body", required=True)
    parser.add_argument("--head", required=True)
    parser.add_argument("--out", required=True)
    # How deep the head's neck stub sinks into the body's stump, as a
    # fraction of the estimated stub height (0.10 * head height). The stub
    # cylinders Pixal3D generates are long — sink most of the way so the
    # chin sits just above the body's stump (measured: 0.6 left a visible
    # double-neck pole).
    parser.add_argument("--sink", type=float, default=2.0)
    return parser.parse_args(argv)


def import_glb_as_single(path, name):
    before = set(bpy.context.scene.objects)
    bpy.ops.import_scene.gltf(filepath=path)
    new = [o for o in bpy.context.scene.objects if o not in before]
    meshes = [o for o in new if o.type == "MESH"]
    if not meshes:
        raise RuntimeError(f"no mesh in {path}")
    bpy.ops.object.select_all(action="DESELECT")
    for m in meshes:
        m.select_set(True)
    bpy.context.view_layer.objects.active = meshes[0]
    if len(meshes) > 1:
        bpy.ops.object.join()
    obj = bpy.context.view_layer.objects.active
    obj.name = name
    for o in new:
        if o.type != "MESH" and o.name in bpy.data.objects:
            bpy.data.objects.remove(o, do_unlink=True)
    obj.parent = None
    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    return drop_flat_shards(obj)


def drop_flat_shards(obj):
    """Pixal3D sometimes hallucinates floating flat slab artifacts (measured
    on the headless body generation: two gray side panels). Split loose
    parts, drop any whose bounding box is nearly two-dimensional, rejoin.
    Balloon body parts are fat closed shells and always survive."""
    size = max(obj.dimensions)
    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    bpy.ops.mesh.separate(type="LOOSE")
    parts = [o for o in bpy.context.selected_objects if o.type == "MESH"]
    kept, dropped = [], 0
    for part in parts:
        if min(part.dimensions) < 0.015 * size:
            bpy.data.objects.remove(part, do_unlink=True)
            dropped += 1
        else:
            kept.append(part)
    if not kept:
        raise RuntimeError("shard filter dropped everything")
    bpy.ops.object.select_all(action="DESELECT")
    for part in kept:
        part.select_set(True)
    bpy.context.view_layer.objects.active = kept[0]
    if len(kept) > 1:
        bpy.ops.object.join()
    merged = bpy.context.view_layer.objects.active
    print(f"SHARD_FILTER kept={len(kept)} dropped={dropped}")
    return merged


def verts(obj):
    count = len(obj.data.vertices)
    coords = np.empty(count * 3, dtype=np.float64)
    obj.data.vertices.foreach_get("co", coords)
    return coords.reshape(count, 3)


def band_mean_texture_color(obj, z_lo, z_hi):
    """Mean RGB of the texels mapped by faces inside a z band. Used on the
    neck stub/stump bands (pure skin on both parts) to anchor a color match."""
    mesh = obj.data
    if not mesh.uv_layers.active or not mesh.materials:
        return None, None
    image = None
    for mat in mesh.materials:
        if mat and mat.use_nodes:
            for node in mat.node_tree.nodes:
                if node.type == "TEX_IMAGE" and node.image:
                    image = node.image
                    break
        if image:
            break
    if not image:
        return None, None

    width, height = image.size
    pixels = np.array(image.pixels[:], dtype=np.float32).reshape(height, width, 4)
    uv_data = mesh.uv_layers.active.data

    samples = []
    for poly in mesh.polygons:
        zs = [mesh.vertices[mesh.loops[li].vertex_index].co.z for li in poly.loop_indices]
        if min(zs) < z_lo or max(zs) > z_hi:
            continue
        for li in poly.loop_indices:
            u, vv = uv_data[li].uv
            x = min(width - 1, max(0, int(u * width)))
            y = min(height - 1, max(0, int(vv * height)))
            samples.append(pixels[y, x, :3])
    if len(samples) < 20:
        return None, image
    return np.mean(np.array(samples), axis=0), image


def match_head_color_to_body(body, head):
    """The two parts are generated independently and their skin tones drift
    (measured: head visibly more orange). Sample skin color on the body's
    neck stump and the head's neck stub, then multiply the whole head
    texture by the per-channel ratio."""
    bv = verts(body)
    hv = verts(head)
    b_top = bv[:, 2].max()
    b_h = b_top - bv[:, 2].min()
    h_bot = hv[:, 2].min()
    h_h = hv[:, 2].max() - h_bot

    body_color, _ = band_mean_texture_color(body, b_top - 0.04 * b_h, b_top)
    head_color, head_image = band_mean_texture_color(head, h_bot, h_bot + 0.05 * h_h)
    if body_color is None or head_color is None or head_image is None:
        print("COLOR_MATCH skipped (no samples)")
        return
    ratio = np.clip(body_color / np.maximum(head_color, 1e-4), 0.5, 2.0)

    # SKIN-WEIGHTED correction: a blanket multiply re-tints everything (the
    # white ponytail went blue from the B-channel boost). Weight each texel
    # by its closeness to the head's skin tone so hair/eyes stay untouched.
    px = np.array(head_image.pixels[:], dtype=np.float32).reshape(-1, 4)
    dist = np.linalg.norm(px[:, :3] - head_color[None, :], axis=1)
    weight = np.exp(-(dist / 0.22) ** 2)
    px[:, :3] = np.clip(px[:, :3] * (1.0 + (ratio[None, :] - 1.0) * weight[:, None]), 0.0, 1.0)
    head_image.pixels = px.reshape(-1).tolist()
    head_image.pack()
    print(f"COLOR_MATCH body={np.round(body_color,3)} head={np.round(head_color,3)} ratio={np.round(ratio,3)} skin_weighted=1")


def band_stats(v, z_lo, z_hi):
    band = v[(v[:, 2] >= z_lo) & (v[:, 2] <= z_hi)]
    if not band.size:
        raise RuntimeError("empty band")
    cx = (band[:, 0].max() + band[:, 0].min()) * 0.5
    cy = (band[:, 1].max() + band[:, 1].min()) * 0.5
    width = band[:, 0].max() - band[:, 0].min()
    return cx, cy, width


def main():
    args = parse_args()
    bpy.ops.wm.read_factory_settings(use_empty=True)

    body = import_glb_as_single(args.body, "Body")
    head = import_glb_as_single(args.head, "Head")

    match_head_color_to_body(body, head)

    bv = verts(body)
    hv = verts(head)

    # Body neck stump: the topmost sliver of the body.
    b_top = bv[:, 2].max()
    b_h = b_top - bv[:, 2].min()
    stump_cx, stump_cy, stump_w = band_stats(bv, b_top - 0.02 * b_h, b_top)

    # Head neck stub: the bottommost sliver of the head.
    h_bot = hv[:, 2].min()
    h_h = hv[:, 2].max() - h_bot
    stub_cx, stub_cy, stub_w = band_stats(hv, h_bot, h_bot + 0.04 * h_h)

    print(f"ASSEMBLE_DEBUG stump_w={stump_w:.4f} stub_w={stub_w:.4f} body_h={b_h:.4f} head_h={h_h:.4f}")
    scale = stump_w / stub_w
    # Sanity clamp: the assembled head+hair should be roughly 28-40% of the
    # final figure height for this toy style. If the neck-diameter anchor
    # lands outside that, fall back to a height-ratio anchor.
    head_frac = (h_h * scale) / (b_h + 0.75 * h_h * scale)
    if not (0.22 <= head_frac <= 0.45):
        scale = (0.42 * b_h) / h_h
        print(f"ASSEMBLE_DEBUG neck anchor rejected (head_frac={head_frac:.3f}), height fallback scale={scale:.4f}")
    head.scale = (scale, scale, scale)
    bpy.ops.object.select_all(action="DESELECT")
    head.select_set(True)
    bpy.context.view_layer.objects.active = head
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)

    hv = verts(head)
    h_bot = hv[:, 2].min()
    h_h = hv[:, 2].max() - h_bot
    stub_cx, stub_cy, _ = band_stats(hv, h_bot, h_bot + 0.04 * h_h)
    stub_height = 0.10 * h_h  # generous stub estimate for the sink depth

    head.location = (
        stump_cx - stub_cx,
        stump_cy - stub_cy,
        (b_top - args.sink * stub_height) - h_bot,
    )
    bpy.ops.object.transform_apply(location=True, rotation=False, scale=False)

    # Join body+head into one object (BuildMotionRig joins shells anyway, but
    # a single object keeps the GLB tidy) and export.
    bpy.ops.object.select_all(action="DESELECT")
    body.select_set(True)
    head.select_set(True)
    bpy.context.view_layer.objects.active = body
    bpy.ops.object.join()

    bpy.ops.object.select_all(action="DESELECT")
    body.select_set(True)
    bpy.ops.export_scene.gltf(filepath=args.out, export_format="GLB", use_selection=True)

    print(f"ASSEMBLE_RESULT=PASS scale={scale:.3f}")


if __name__ == "__main__":
    main()
