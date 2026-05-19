#!/usr/bin/env python3
"""Author ToonStyle vertex colors on the active normalized mesh.

Layout:
  R = threshold/AO offset (occluded areas shade earlier)
  G = outline width multiplier
  B = outline depth offset
  A = outline mask
"""

from __future__ import annotations

import math

import bpy
from mathutils import Vector


def _ensure_color_attribute(obj: bpy.types.Object, name: str = "Col") -> bpy.types.Attribute:
	mesh = obj.data
	attr = mesh.color_attributes.get(name)
	if attr is None:
		attr = mesh.color_attributes.new(name=name, type="BYTE_COLOR", domain="CORNER")
	mesh.color_attributes.active = attr
	mesh.color_attributes.render_color_index = list(mesh.color_attributes).index(attr)
	return attr


def _bounds(obj: bpy.types.Object) -> tuple[Vector, Vector]:
	corners = [obj.matrix_world @ Vector(corner) for corner in obj.bound_box]
	return (
		Vector((min(c.x for c in corners), min(c.y for c in corners), min(c.z for c in corners))),
		Vector((max(c.x for c in corners), max(c.y for c in corners), max(c.z for c in corners))),
	)


def _analytic_ao_value(world_pos: Vector, mins: Vector, maxs: Vector) -> float:
	dims = maxs - mins
	z01 = 0.0 if dims.z <= 0 else (world_pos.z - mins.z) / dims.z
	center = (mins + maxs) * 0.5
	radius = max(dims.x, dims.y) * 0.5
	xy_dist = math.sqrt((world_pos.x - center.x) ** 2 + (world_pos.y - center.y) ** 2)
	edge01 = 0.0 if radius <= 0 else min(xy_dist / radius, 1.0)
	# Cheap, deterministic occlusion proxy when Cycles vertex-color AO bake is not available.
	# Low and central areas shade slightly earlier; exposed top/outer silhouette shades later.
	return max(0.0, min(1.0, 0.35 + (1.0 - z01) * 0.45 + (1.0 - edge01) * 0.15))


def _write_analytic_colors(obj: bpy.types.Object, attr: bpy.types.Attribute) -> str:
	mins, maxs = _bounds(obj)
	mesh = obj.data
	for loop in mesh.loops:
		vertex = mesh.vertices[loop.vertex_index]
		world_pos = obj.matrix_world @ vertex.co
		attr.data[loop.index].color = (
			_analytic_ao_value(world_pos, mins, maxs),
			1.0,
			0.0,
			1.0,
		)
	return "analytic_fallback"


def _try_cycles_vertex_ao_bake(obj: bpy.types.Object, attr: bpy.types.Attribute) -> bool:
	scene = bpy.context.scene
	bpy.ops.object.select_all(action="DESELECT")
	obj.select_set(True)
	bpy.context.view_layer.objects.active = obj
	try:
		scene.render.engine = "CYCLES"
		scene.cycles.samples = 64
		scene.cycles.use_denoising = False
		scene.world.color = (1.0, 1.0, 1.0)
		# Blender 5.1 supports vertex-color bake targets in the operator. If the
		# active build changes that API, callers fall back to the deterministic proxy.
		bpy.ops.object.bake(type="AO", target="VERTEX_COLORS")
		for item in attr.data:
			r = item.color[0]
			item.color = (max(0.0, min(1.0, 1.0 - r)), 1.0, 0.0, 1.0)
		return True
	except Exception as exc:
		print(f"[author_vertex_colors] Cycles AO vertex bake unavailable: {exc}")
		return False


def author_vertex_colors(obj: bpy.types.Object) -> dict[str, object]:
	if not obj or obj.type != "MESH":
		raise RuntimeError("author_vertex_colors requires a mesh object")
	attr = _ensure_color_attribute(obj)
	mode = "cycles_ao"
	if not _try_cycles_vertex_ao_bake(obj, attr):
		mode = _write_analytic_colors(obj, attr)
	obj.data.update()
	return {
		"attribute": attr.name,
		"domain": str(attr.domain),
		"type": str(attr.data_type),
		"loop_count": len(attr.data),
		"mode": mode,
	}
