"""
Inspect Blender/GLTF/GLB/FBX animation assets and write a lightweight JSON report.

Run with Blender:
  blender --background --python Tools/inspect_animation_assets.py -- --asset path --json-out report.json
"""

import argparse
import json
import os
import sys
from pathlib import Path

import bpy


def reset_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete()
    for collection in (
        bpy.data.meshes,
        bpy.data.armatures,
        bpy.data.actions,
        bpy.data.materials,
        bpy.data.images,
    ):
        for item in list(collection):
            collection.remove(item)


def import_asset(path: Path):
    suffix = path.suffix.lower()
    if suffix == ".blend":
        bpy.ops.wm.open_mainfile(filepath=str(path))
        return
    reset_scene()
    if suffix in {".glb", ".gltf"}:
        bpy.ops.import_scene.gltf(filepath=str(path))
        return
    if suffix == ".fbx":
        bpy.ops.import_scene.fbx(filepath=str(path))
        return
    raise ValueError(f"Unsupported asset type: {path}")


def inspect_current_scene(path: Path):
    armatures = []
    for obj in bpy.data.objects:
        if obj.type == "ARMATURE" and obj.data:
            armatures.append(
                {
                    "object": obj.name,
                    "armature": obj.data.name,
                    "bone_count": len(obj.data.bones),
                    "root_bones": [bone.name for bone in obj.data.bones if bone.parent is None],
                }
            )

    actions = []
    for action in bpy.data.actions:
        legacy_fcurves = list(getattr(action, "fcurves", []) or [])
        nested_fcurve_count = count_nested_fcurves(action)
        actions.append(
            {
                "name": action.name,
                "frame_range": [float(action.frame_range[0]), float(action.frame_range[1])],
                "fcurve_count": nested_fcurve_count,
                "legacy_fcurve_count": len(legacy_fcurves),
                "uses_layered_action_data": nested_fcurve_count > len(legacy_fcurves),
            }
        )

    meshes = [
        {
            "object": obj.name,
            "mesh": obj.data.name if obj.data else "",
            "vertex_count": len(obj.data.vertices) if obj.data else 0,
        }
        for obj in bpy.data.objects
        if obj.type == "MESH"
    ]

    return {
        "path": str(path),
        "exists": path.exists(),
        "size_bytes": path.stat().st_size if path.exists() else 0,
        "armatures": armatures,
        "actions": actions,
        "meshes": meshes,
    }


def count_nested_fcurves(value):
    seen_values = set()
    seen_curves = set()

    def walk(item):
        item_id = id(item)
        if item_id in seen_values:
            return
        seen_values.add(item_id)

        try:
            fcurves = getattr(item, "fcurves", None)
        except Exception:
            fcurves = None
        if fcurves is not None:
            try:
                for fcurve in fcurves:
                    seen_curves.add(id(fcurve))
            except Exception:
                pass

        for attr_name in ("layers", "strips", "channels", "channelbags", "groups"):
            try:
                children = getattr(item, attr_name, None)
            except Exception:
                children = None
            if children is None:
                continue
            try:
                iterator = iter(children)
            except TypeError:
                continue
            except Exception:
                continue
            for child in iterator:
                walk(child)

    walk(value)
    return len(seen_curves)


def main(argv):
    parser = argparse.ArgumentParser()
    parser.add_argument("--asset", action="append", default=[], help="Asset path to inspect.")
    parser.add_argument("--root", help="Optional root to scan for supported assets.")
    parser.add_argument("--json-out", required=True)
    parser.add_argument("--max-files", type=int, default=40)
    args = parser.parse_args(argv)

    assets = [Path(p).resolve() for p in args.asset]
    if args.root:
        root = Path(args.root).resolve()
        for path in root.rglob("*"):
            if path.suffix.lower() in {".blend", ".glb", ".gltf", ".fbx"}:
                assets.append(path)

    unique_assets = []
    seen = set()
    for asset in assets:
        key = str(asset).lower()
        if key not in seen:
            seen.add(key)
            unique_assets.append(asset)

    results = []
    for asset in unique_assets[: args.max_files]:
        try:
            import_asset(asset)
            result = inspect_current_scene(asset)
            result["status"] = "ok"
        except Exception as exc:
            result = {
                "path": str(asset),
                "exists": asset.exists(),
                "size_bytes": asset.stat().st_size if asset.exists() else 0,
                "status": "error",
                "error": str(exc),
            }
        results.append(result)

    report = {
        "blender_version": bpy.app.version_string,
        "asset_count": len(results),
        "assets": results,
    }

    out = Path(args.json_out).resolve()
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(json.dumps(report, indent=2), encoding="utf-8")
    print(json.dumps({"json_out": str(out), "asset_count": len(results)}, indent=2))


if __name__ == "__main__":
    argv = sys.argv
    if "--" in argv:
        argv = argv[argv.index("--") + 1 :]
    else:
        argv = []
    main(argv)
