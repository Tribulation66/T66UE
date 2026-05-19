#!/usr/bin/env python3
"""Verify the Phase 1C outline winding reversal strategy on disposable geometry."""

from __future__ import annotations

import argparse
import json
from pathlib import Path

import bmesh
import bpy
from mathutils import Vector


def is_cyclic_reverse(before: list[int], after: list[int]) -> bool:
	if len(before) != len(after) or not before:
		return False
	reversed_before = list(reversed(before))
	return any(after == reversed_before[offset:] + reversed_before[:offset] for offset in range(len(reversed_before)))


def reverse_winding_preserve_vertex_normals(obj: bpy.types.Object) -> dict[str, object]:
	mesh = obj.data
	mesh.update()
	before_face = list(mesh.polygons[0].vertices)
	before_vertex_normals = [vertex.normal.copy() for vertex in mesh.vertices]

	bm = bmesh.new()
	bm.from_mesh(mesh)
	bmesh.ops.reverse_faces(bm, faces=list(bm.faces))
	bm.to_mesh(mesh)
	bm.free()
	mesh.update()

	loop_normals = []
	for loop in mesh.loops:
		normal = before_vertex_normals[loop.vertex_index].copy()
		normal.normalize()
		loop_normals.append(normal)
	for poly in mesh.polygons:
		poly.use_smooth = True
	mesh.normals_split_custom_set(loop_normals)
	mesh.update()

	after_face = list(mesh.polygons[0].vertices)
	center = sum((vertex.co for vertex in mesh.vertices), Vector()) / max(1, len(mesh.vertices))
	samples = []
	for index, vertex in enumerate(mesh.vertices[:4]):
		radial = vertex.co - center
		if radial.length > 0.0001:
			radial.normalize()
		vertex_loop = next(loop for loop in mesh.loops if loop.vertex_index == index)
		normal = vertex_loop.normal.copy()
		normal.normalize()
		samples.append(
			{
				"vertex_index": index,
				"normal": [normal.x, normal.y, normal.z],
				"radial": [radial.x, radial.y, radial.z],
				"dot_outward": float(normal.dot(radial)),
			}
		)

	return {
		"method": "bmesh.ops.reverse_faces + restore captured vertex normals as custom split normals",
		"face_before": before_face,
		"face_after": after_face,
		"face_order_reversed": is_cyclic_reverse(before_face, after_face),
		"normal_samples": samples,
		"normals_outward": all(sample["dot_outward"] > 0.0 for sample in samples),
	}


def parse_args() -> argparse.Namespace:
	argv = []
	import sys
	if "--" in sys.argv:
		argv = sys.argv[sys.argv.index("--") + 1 :]
	parser = argparse.ArgumentParser(description="Verify outline winding reversal on a cube.")
	parser.add_argument("--output", required=True, type=Path)
	return parser.parse_args(argv)


def main() -> int:
	args = parse_args()
	bpy.ops.object.select_all(action="SELECT")
	bpy.ops.object.delete()
	bpy.ops.mesh.primitive_cube_add(size=2.0, location=(0.0, 0.0, 0.0))
	cube = bpy.context.object
	report = reverse_winding_preserve_vertex_normals(cube)
	report["passed"] = bool(report["face_order_reversed"] and report["normals_outward"])
	args.output.parent.mkdir(parents=True, exist_ok=True)
	args.output.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
	print(json.dumps(report, sort_keys=True))
	return 0 if report["passed"] else 2


if __name__ == "__main__":
	raise SystemExit(main())
