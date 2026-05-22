#!/usr/bin/env python3
"""Deprecated sphere-proxy custom-normal transfer for humanoid face experiments."""

from __future__ import annotations

import math
import json
from pathlib import Path

import bpy
from mathutils import Vector

REPO_ROOT = Path(__file__).resolve().parents[2]
FACE_MASK_OVERRIDES = REPO_ROOT / "SourceAssets" / "ToonStyle" / "Pixal3D" / "FaceMaskOverrides.json"
FACE_MASK_RADIUS_FRACTION = 0.12
FACE_MASK_MAX_LOOP_COVERAGE = 0.10


def _bounds(obj: bpy.types.Object) -> tuple[Vector, Vector]:
	corners = [obj.matrix_world @ Vector(corner) for corner in obj.bound_box]
	return (
		Vector((min(c.x for c in corners), min(c.y for c in corners), min(c.z for c in corners))),
		Vector((max(c.x for c in corners), max(c.y for c in corners), max(c.z for c in corners))),
	)


def _loop_normals(obj: bpy.types.Object) -> list[Vector]:
	obj.data.calc_loop_triangles()
	return [loop.normal.copy() for loop in obj.data.loops]


def _load_face_mask_overrides() -> dict[str, object]:
	if not FACE_MASK_OVERRIDES.exists():
		return {}
	with FACE_MASK_OVERRIDES.open("r", encoding="utf-8") as handle:
		data = json.load(handle)
	if not isinstance(data, dict):
		raise RuntimeError(f"Face mask overrides must be a JSON object: {FACE_MASK_OVERRIDES}")
	return data


def _asset_name_from_object(obj: bpy.types.Object, asset_name: str | None) -> str:
	if asset_name:
		return asset_name
	name = obj.name
	for suffix in ("_joined", "_mesh", "_outline"):
		if name.endswith(suffix):
			return name[: -len(suffix)]
	return name


def _centroid(points: list[Vector]) -> Vector:
	center = Vector((0.0, 0.0, 0.0))
	for point in points:
		center += point
	return center / len(points)


def _lateral_stddev(points: list[Vector], center: Vector) -> float:
	if not points:
		return 0.0
	total = 0.0
	for point in points:
		dx = point.x - center.x
		dy = point.y - center.y
		total += dx * dx + dy * dy
	return math.sqrt(total / len(points))


def _score_face_bands(points: list[Vector], mins: Vector, maxs: Vector, height: float) -> dict[str, object]:
	band_height = height * 0.02
	step = band_height * 0.5
	top_region_min = maxs.z - (height * 0.40)
	candidates = []
	low = top_region_min
	while low <= maxs.z - band_height + 0.0001:
		high = low + band_height
		band_points = [point for point in points if low <= point.z <= high]
		if len(band_points) >= 3:
			center = _centroid(band_points)
			lateral_stddev = _lateral_stddev(band_points, center)
			density = len(band_points) / max(band_height, 0.0001)
			compactness = 1.0 / (1.0 + lateral_stddev)
			combined = density * compactness
			candidates.append(
				{
					"low_z": low,
					"high_z": high,
					"mid_z": (low + high) * 0.5,
					"vertex_count": len(band_points),
					"density": density,
					"compactness": compactness,
					"lateral_stddev": lateral_stddev,
					"combined": combined,
					"center": center,
				}
			)
		low += step
	if not candidates:
		raise RuntimeError("face_mask_no_candidate_bands")
	candidates.sort(key=lambda item: item["combined"], reverse=True)
	return candidates[0]


def _override_center(
	points: list[Vector],
	head_center_z: float,
	sphere_radius: float,
	height: float,
) -> tuple[Vector, int]:
	band_half_height = max(height * 0.01, sphere_radius * 0.25)
	band_points = [point for point in points if abs(point.z - head_center_z) <= band_half_height]
	if not band_points:
		band_points = [point for point in points if abs(point.z - head_center_z) <= sphere_radius]
	if not band_points:
		raise RuntimeError(f"face_mask_override_no_vertices_near_z:{head_center_z:.4f}")
	center = _centroid(band_points)
	center.z = head_center_z
	return center, len(band_points)


def transfer_face_normals(obj: bpy.types.Object, is_humanoid: bool, asset_name: str | None = None) -> dict[str, object]:
	if not obj or obj.type != "MESH":
		raise RuntimeError("transfer_face_normals requires a mesh object")
	if not is_humanoid:
		return {"applied": False, "reason": "non_humanoid"}

	mesh = obj.data
	mins, maxs = _bounds(obj)
	height = maxs.z - mins.z
	if height <= 0.0:
		return {"applied": False, "reason": "zero_height"}

	asset_key = _asset_name_from_object(obj, asset_name)
	points = [obj.matrix_world @ vertex.co for vertex in mesh.vertices]
	overrides = _load_face_mask_overrides()
	override = overrides.get(asset_key)
	if override:
		try:
			head_center_z = float(override["head_center_z"])
			radius = float(override["sphere_radius"])
		except (KeyError, TypeError, ValueError) as exc:
			raise RuntimeError(f"{asset_key}: invalid face mask override in {FACE_MASK_OVERRIDES}: {exc}") from exc
		center, band_vertex_count = _override_center(points, head_center_z, radius, height)
		source = "override"
		density = 0.0
		compactness = 0.0
		combined = 0.0
		lateral_stddev = 0.0
		band_low = head_center_z
		band_high = head_center_z
	else:
		winning = _score_face_bands(points, mins, maxs, height)
		center = winning["center"].copy()
		center.z = winning["mid_z"]
		radius = FACE_MASK_RADIUS_FRACTION * height
		source = "heuristic"
		density = float(winning["density"])
		compactness = float(winning["compactness"])
		combined = float(winning["combined"])
		lateral_stddev = float(winning["lateral_stddev"])
		band_low = float(winning["low_z"])
		band_high = float(winning["high_z"])
		band_vertex_count = int(winning["vertex_count"])

	if radius <= 0.0:
		raise RuntimeError(f"{asset_key}: face_mask_invalid_radius:{radius:.4f}")

	face_vertex_indices = {
		vertex.index
		for vertex in mesh.vertices
		if ((obj.matrix_world @ vertex.co) - center).length <= radius
	}
	if not face_vertex_indices:
		raise RuntimeError(f"{asset_key}: face_mask_no_vertices radius={radius:.4f} center={[center.x, center.y, center.z]}")

	# We implement the sphere-proxy transfer directly into split normals so the
	# outline duplicate can retain geometric normals without modifier coupling.
	loop_normals = _loop_normals(obj)
	changed = 0
	for loop in mesh.loops:
		world_pos = obj.matrix_world @ mesh.vertices[loop.vertex_index].co
		if loop.vertex_index not in face_vertex_indices:
			continue
		proxy_normal_world = (world_pos - center)
		if proxy_normal_world.length <= 0.0001:
			continue
		proxy_normal_world.normalize()
		loop_normals[loop.index] = obj.matrix_world.inverted().to_3x3() @ proxy_normal_world
		loop_normals[loop.index].normalize()
		changed += 1

	total_loops = max(1, len(mesh.loops))
	coverage_fraction = changed / total_loops
	if coverage_fraction > FACE_MASK_MAX_LOOP_COVERAGE:
		raise RuntimeError(
			f"{asset_key}: face_mask_overbroad coverage={coverage_fraction:.4f} "
			f"threshold={FACE_MASK_MAX_LOOP_COVERAGE:.4f} source={source} "
			f"density={density:.4f} compactness={compactness:.4f} combined={combined:.4f}"
		)

	mesh.normals_split_custom_set(loop_normals)
	for poly in mesh.polygons:
		poly.use_smooth = True
	mesh.update()
	return {
		"applied": True,
		"method": "density_compactness_sphere_mask_split_normals",
		"face_mask_source": source,
		"face_mask_pass": True,
		"face_mask_coverage_fraction": coverage_fraction,
		"face_mask_max_coverage_fraction": FACE_MASK_MAX_LOOP_COVERAGE,
		"face_mask_vertex_count": len(face_vertex_indices),
		"band_vertex_count": band_vertex_count,
		"band_low_z": band_low,
		"band_high_z": band_high,
		"band_density_score": density,
		"band_compactness_score": compactness,
		"band_combined_score": combined,
		"band_lateral_stddev": lateral_stddev,
		"changed_loop_count": changed,
		"proxy_center": [center.x, center.y, center.z],
		"proxy_radius": radius,
		"radius_fraction": radius / height,
	}
