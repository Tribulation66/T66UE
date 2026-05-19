#!/usr/bin/env python3
"""Apply a sphere-proxy style custom-normal transfer to humanoid face regions."""

from __future__ import annotations

import math

import bpy
from mathutils import Vector


def _bounds(obj: bpy.types.Object) -> tuple[Vector, Vector]:
	corners = [obj.matrix_world @ Vector(corner) for corner in obj.bound_box]
	return (
		Vector((min(c.x for c in corners), min(c.y for c in corners), min(c.z for c in corners))),
		Vector((max(c.x for c in corners), max(c.y for c in corners), max(c.z for c in corners))),
	)


def _loop_normals(obj: bpy.types.Object) -> list[Vector]:
	obj.data.calc_loop_triangles()
	return [loop.normal.copy() for loop in obj.data.loops]


def transfer_face_normals(obj: bpy.types.Object, is_humanoid: bool) -> dict[str, object]:
	if not obj or obj.type != "MESH":
		raise RuntimeError("transfer_face_normals requires a mesh object")
	if not is_humanoid:
		return {"applied": False, "reason": "non_humanoid"}

	mesh = obj.data
	mins, maxs = _bounds(obj)
	height = maxs.z - mins.z
	if height <= 0.0:
		return {"applied": False, "reason": "zero_height"}

	threshold = mins.z + (height * 0.75)
	head_vertices = [
		vertex
		for vertex in mesh.vertices
		if (obj.matrix_world @ vertex.co).z >= threshold
	]
	if not head_vertices:
		return {"applied": False, "reason": "no_head_vertices"}

	center = Vector((0.0, 0.0, 0.0))
	for vertex in head_vertices:
		center += obj.matrix_world @ vertex.co
	center /= len(head_vertices)
	radius = max((obj.matrix_world @ vertex.co - center).length for vertex in head_vertices)
	if radius <= 0.0:
		radius = height * 0.1

	# We implement the sphere-proxy transfer directly into split normals so the
	# outline duplicate can retain geometric normals without modifier coupling.
	loop_normals = _loop_normals(obj)
	changed = 0
	for loop in mesh.loops:
		world_pos = obj.matrix_world @ mesh.vertices[loop.vertex_index].co
		if world_pos.z < threshold:
			continue
		proxy_normal_world = (world_pos - center)
		if proxy_normal_world.length <= 0.0001:
			continue
		proxy_normal_world.normalize()
		loop_normals[loop.index] = obj.matrix_world.inverted().to_3x3() @ proxy_normal_world
		loop_normals[loop.index].normalize()
		changed += 1

	mesh.normals_split_custom_set(loop_normals)
	for poly in mesh.polygons:
		poly.use_smooth = True
	mesh.update()
	return {
		"applied": True,
		"method": "sphere_proxy_split_normals",
		"head_threshold_z": threshold,
		"head_vertex_count": len(head_vertices),
		"changed_loop_count": changed,
		"proxy_center": [center.x, center.y, center.z],
		"proxy_radius": radius * 0.8,
	}
