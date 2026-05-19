#!/usr/bin/env python3
"""Shared curvature utilities for ToonStyle foundation tools.

The outputs are intentionally data-first: close-the-gap authoring and inner
line baking both consume the same normalized per-vertex curvature vector.
"""

from __future__ import annotations

import json
import math
from pathlib import Path

import bmesh
import bpy
import numpy as np


def _percentile(values: np.ndarray, pct: float) -> float:
	if values.size == 0:
		return 0.0
	return float(np.percentile(values, pct))


def curvature_stats(values: np.ndarray) -> dict[str, object]:
	if values.size == 0:
		return {
			"count": 0,
			"min": 0.0,
			"max": 0.0,
			"mean": 0.0,
			"std": 0.0,
			"percentiles": {},
		}
	return {
		"count": int(values.size),
		"min": float(values.min()),
		"max": float(values.max()),
		"mean": float(values.mean()),
		"std": float(values.std()),
		"percentiles": {
			"p05": _percentile(values, 5),
			"p25": _percentile(values, 25),
			"p50": _percentile(values, 50),
			"p75": _percentile(values, 75),
			"p90": _percentile(values, 90),
			"p95": _percentile(values, 95),
			"p99": _percentile(values, 99),
		},
	}


def analyze_vertex_curvature(obj: bpy.types.Object) -> tuple[np.ndarray, dict[str, object]]:
	"""Return normalized per-vertex curvature in [0, 1].

	Uses edge face-angle divergence. This is deliberately topology-local and does
	not depend on rendered normals, so it remains stable after the R2 custom
	normal work.
	"""
	if not obj or obj.type != "MESH":
		raise RuntimeError("analyze_vertex_curvature requires a mesh object")
	mesh = obj.data
	mesh.update()
	bm = bmesh.new()
	bm.from_mesh(mesh)
	bm.verts.ensure_lookup_table()
	bm.edges.ensure_lookup_table()
	bm.faces.ensure_lookup_table()
	bm.normal_update()
	accum = np.zeros(len(mesh.vertices), dtype=np.float64)
	weights = np.zeros(len(mesh.vertices), dtype=np.float64)
	edge_samples = 0
	boundary_edges = 0
	for edge in bm.edges:
		if len(edge.link_faces) != 2:
			boundary_edges += 1
			continue
		try:
			angle = abs(float(edge.calc_face_angle_signed(0.0)))
		except Exception:
			angle = abs(float(edge.calc_face_angle(0.0)))
		score = min(angle / math.pi, 1.0)
		if score <= 1.0e-6:
			continue
		length = float(edge.calc_length())
		for vert in edge.verts:
			accum[vert.index] += score * max(length, 1.0e-6)
			weights[vert.index] += max(length, 1.0e-6)
		edge_samples += 1
	bm.free()
	raw = np.divide(accum, weights, out=np.zeros_like(accum), where=weights > 0.0)
	scale = _percentile(raw[raw > 0.0], 95) if np.any(raw > 0.0) else 0.0
	if scale <= 1.0e-8:
		values = np.zeros_like(raw)
	else:
		values = np.clip(raw / scale, 0.0, 1.0)
	report = curvature_stats(values)
	report.update(
		{
			"method": "edge_face_angle_divergence_abs_normalized_p95",
			"edge_samples": int(edge_samples),
			"boundary_edges": int(boundary_edges),
			"normalization_scale_raw_p95": float(scale),
		}
	)
	return values.astype(np.float32), report


def write_curvature_report(path: Path, asset_name: str, values: np.ndarray, report: dict[str, object]) -> None:
	payload = {
		"asset": asset_name,
		"curvature": report,
		"codex_opinion": (
			"edge-angle divergence produced non-degenerate curvature suitable for mechanical "
			"tool activation" if report.get("max", 0.0) > 0.0 and report.get("std", 0.0) > 0.0 else
			"curvature output is degenerate and should not feed foundation tools"
		),
	}
	path.parent.mkdir(parents=True, exist_ok=True)
	path.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n", encoding="utf-8")


def curvature_to_close_gap_b(values: np.ndarray, asset_class: str) -> tuple[np.ndarray, dict[str, object]]:
	"""Map curvature to VertexColor.B for close-the-gap depth offset."""
	if values.size == 0 or not np.any(values > 0.0):
		return np.zeros_like(values, dtype=np.float32), {
			"method": "p75_to_p98_response_curve",
			"reason": "degenerate_curvature",
		}
	low = _percentile(values, 65 if asset_class == "prop" else 70)
	high = _percentile(values, 97 if asset_class == "prop" else 95)
	if high <= low + 1.0e-6:
		high = float(values.max())
	scaled = np.clip((values - low) / max(high - low, 1.0e-6), 0.0, 1.0)
	# Gentle response keeps low curvature vertices at zero while giving strong
	# valleys enough B data to prove the mechanism is non-stale.
	b = np.power(scaled, 0.7).astype(np.float32)
	return b, {
		"method": "clamp_below_percentile_then_power_response",
		"asset_class": asset_class,
		"low_percentile_value": float(low),
		"high_percentile_value": float(high),
		"power": 0.7,
	}


def ensure_corner_color_attribute(obj: bpy.types.Object, name: str = "Col") -> bpy.types.Attribute:
	mesh = obj.data
	attr = mesh.color_attributes.get(name)
	if attr is None:
		attr = mesh.color_attributes.new(name=name, type="BYTE_COLOR", domain="CORNER")
	mesh.color_attributes.active = attr
	mesh.color_attributes.render_color_index = list(mesh.color_attributes).index(attr)
	return attr


def author_outline_b(obj: bpy.types.Object, b_values: np.ndarray, attr_name: str = "Col") -> dict[str, object]:
	if not obj or obj.type != "MESH":
		raise RuntimeError("author_outline_b requires a mesh object")
	mesh = obj.data
	if len(b_values) != len(mesh.vertices):
		raise RuntimeError(f"B value count {len(b_values)} does not match vertex count {len(mesh.vertices)}")
	attr = ensure_corner_color_attribute(obj, attr_name)
	for loop in mesh.loops:
		color = list(attr.data[loop.index].color)
		while len(color) < 4:
			color.append(1.0)
		color[2] = float(np.clip(b_values[loop.vertex_index], 0.0, 1.0))
		attr.data[loop.index].color = tuple(color[:4])
	mesh.update()
	return readback_b(obj, attr_name)


def readback_b(obj: bpy.types.Object, attr_name: str = "Col") -> dict[str, object]:
	mesh = obj.data
	attr = mesh.color_attributes.get(attr_name)
	if attr is None:
		return {
			"N": 0,
			"B_min": 0.0,
			"B_max": 0.0,
			"B_mean": 0.0,
			"B_nonzero_fraction": 0.0,
			"attribute": None,
		}
	per_vertex: list[list[float]] = [[] for _ in mesh.vertices]
	for loop in mesh.loops:
		per_vertex[loop.vertex_index].append(float(attr.data[loop.index].color[2]))
	values = np.asarray(
		[float(np.mean(samples)) if samples else 0.0 for samples in per_vertex],
		dtype=np.float32,
	)
	if values.size == 0:
		return {
			"N": 0,
			"B_min": 0.0,
			"B_max": 0.0,
			"B_mean": 0.0,
			"B_nonzero_fraction": 0.0,
			"attribute": attr.name,
		}
	return {
		"N": int(values.size),
		"B_min": float(values.min()),
		"B_max": float(values.max()),
		"B_mean": float(values.mean()),
		"B_nonzero_fraction": float(np.mean(values > 1.0e-5)),
		"attribute": attr.name,
	}


def uv_segments_from_curvature(obj: bpy.types.Object, values: np.ndarray, percentile: float = 65.0) -> np.ndarray:
	"""Return UV edge segments with intensity for external line-mask baking."""
	mesh = obj.data
	uv_layer = mesh.uv_layers.active
	if uv_layer is None:
		raise RuntimeError(f"Mesh {obj.name} has no active UV layer for inner line bake")
	mesh.update()
	threshold = _percentile(values[values > 0.0], percentile) if np.any(values > 0.0) else 1.0
	segments: list[list[float]] = []
	for poly in mesh.polygons:
		loop_indices = list(poly.loop_indices)
		for index, loop_index_a in enumerate(loop_indices):
			loop_index_b = loop_indices[(index + 1) % len(loop_indices)]
			vertex_a = mesh.loops[loop_index_a].vertex_index
			vertex_b = mesh.loops[loop_index_b].vertex_index
			score = max(float(values[vertex_a]), float(values[vertex_b]))
			if score < threshold:
				continue
			uv_a = uv_layer.data[loop_index_a].uv
			uv_b = uv_layer.data[loop_index_b].uv
			segments.append(
				[
					float(uv_a.x),
					float(uv_a.y),
					float(uv_b.x),
					float(uv_b.y),
					float(np.clip(score, 0.0, 1.0)),
				]
			)
	return np.asarray(segments, dtype=np.float32)
