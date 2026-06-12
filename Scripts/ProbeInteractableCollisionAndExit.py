"""Probe simple-collision vs render bounds for interactable meshes. Self-quits."""
from __future__ import annotations
import json, os
import unreal

OUT = r"C:/UE/T66/Saved/CombatTest/FallGuys/collision_probe.json"
ROOTS = ["/Game/World/Interactables", "/Game/World/NPCs", "/Game/World/Gates"]
out = {"meshes": [], "errors": []}
try:
    ar = unreal.AssetRegistryHelpers.get_asset_registry()
    paths = []
    for root in ROOTS:
        for ad in ar.get_assets_by_path(root, recursive=True):
            if str(ad.asset_class_path.asset_name) == "StaticMesh":
                paths.append(str(ad.package_name))
    paths = sorted(set(paths))
    for p in paths:
        mesh = unreal.EditorAssetLibrary.load_asset(p)
        if not mesh:
            out["errors"].append("load failed: " + p)
            continue
        render = mesh.get_bounds()
        rext = render.box_extent
        body = mesh.get_editor_property("body_setup")
        nbox = nsphere = nconvex = nsphyl = 0
        cext = None
        if body:
            agg = body.get_editor_property("agg_geom")
            boxes = agg.get_editor_property("box_elems")
            spheres = agg.get_editor_property("sphere_elems")
            convex = agg.get_editor_property("convex_elems")
            sphyls = agg.get_editor_property("sphyl_elems")
            nbox, nsphere, nconvex, nsphyl = len(boxes), len(spheres), len(convex), len(sphyls)
            if nbox == 1 and nsphere == 0 and nconvex == 0 and nsphyl == 0:
                cext = boxes[0].get_editor_property("x"), boxes[0].get_editor_property("y"), boxes[0].get_editor_property("z")
        trace_flag = str(body.get_editor_property("collision_trace_flag")) if body else "none"
        rec = {
            "mesh": p,
            "render_extent": [round(rext.x,1), round(rext.y,1), round(rext.z,1)],
            "simple": {"boxes": nbox, "spheres": nsphere, "convex": nconvex, "capsules": nsphyl},
            "trace_flag": trace_flag,
        }
        if cext:
            rec["box_halfsize"] = [round(cext[0]/2,1), round(cext[1]/2,1), round(cext[2]/2,1)]
            rec["box_vs_render_ratio"] = [
                round((cext[0]/2)/max(rext.x,0.1),2),
                round((cext[1]/2)/max(rext.y,0.1),2),
                round((cext[2]/2)/max(rext.z,0.1),2)]
        out["meshes"].append(rec)
finally:
    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    with open(OUT, "w", encoding="utf-8") as f:
        json.dump(out, f, indent=2)
    unreal.log("[CollisionProbe] wrote " + OUT)
    try: unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")
    except Exception: pass
