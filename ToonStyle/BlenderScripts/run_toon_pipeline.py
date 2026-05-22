#!/usr/bin/env python3
"""Single-session ToonStyle Blender pipeline for Pixal3D raw GLBs."""

from __future__ import annotations

import argparse
import json
import subprocess
import sys
from pathlib import Path

import bpy
import bmesh
import numpy as np

SCRIPT_DIR = Path(__file__).resolve().parent
if str(SCRIPT_DIR) not in sys.path:
	sys.path.insert(0, str(SCRIPT_DIR))

from author_vertex_colors import author_vertex_colors
from flatten_diffuse_texture import flatten_blender_image
from pixal3d_blender_base import (
	clear_scene,
	export_fbx,
	extract_textures,
	import_glb,
	join_meshes,
	normalize_spatial,
	strip_material_texture_references,
	world_bounds,
)
from transfer_face_normals import transfer_face_normals
from curvature_analysis import (
	analyze_vertex_curvature,
	author_outline_b,
	curvature_to_close_gap_b,
	uv_segments_from_curvature,
	write_curvature_report,
)


def parse_args() -> argparse.Namespace:
	argv = sys.argv
	if "--" in argv:
		argv = argv[argv.index("--") + 1 :]
	else:
		argv = []
	parser = argparse.ArgumentParser(description="Run the Phase 1B ToonStyle Blender pipeline.")
	parser.add_argument("--input", required=True, type=Path)
	parser.add_argument("--working-dir", required=True, type=Path)
	parser.add_argument("--asset-name", required=True)
	parser.add_argument("--target-height", default=180.0, type=float)
	parser.add_argument("--is-humanoid", action="store_true")
	parser.add_argument("--flatten-k", default=6, type=int)
	parser.add_argument("--highlight-cap", default=0.85, type=float)
	parser.add_argument("--retained-from-phase1a", action="store_true")
	parser.add_argument("--no-remesh-export", action="store_true")
	parser.add_argument("--texture-postprocess-python", default="python")
	parser.add_argument("--enable-foundation-tools", action="store_true")
	parser.add_argument(
		"--enable-humanoid-face-normal-transfer",
		action="store_true",
		help="Deprecated opt-in for humanoid face/head normal transfer. Production default is disabled; revisit only in a future research pass.",
	)
	parser.add_argument("--accepted-limitation", action="store_true")
	parser.add_argument("--asset-class", choices=["humanoid", "creature", "prop", "accepted-limitation"], default="prop")
	parser.add_argument("--inner-line-size", default=4096, type=int)
	parser.add_argument("--reuse-textures-manifest", type=Path)
	return parser.parse_args(argv)


def duplicate_outline_object(obj: bpy.types.Object, asset_name: str) -> bpy.types.Object:
	outline_mesh = obj.data.copy()
	outline_obj = obj.copy()
	outline_obj.data = outline_mesh
	outline_obj.name = f"{asset_name}_outline"
	outline_obj.data.name = f"{asset_name}_outline_mesh"
	bpy.context.collection.objects.link(outline_obj)
	return outline_obj


def reverse_winding_preserve_vertex_normals(obj: bpy.types.Object) -> dict[str, object]:
	mesh = obj.data
	mesh.update()
	if not mesh.polygons:
		return {"applied": False, "reason": "no_polygons"}
	bpy.ops.object.select_all(action="DESELECT")
	obj.select_set(True)
	bpy.context.view_layer.objects.active = obj
	bpy.ops.object.shade_smooth()
	for poly in mesh.polygons:
		poly.use_smooth = True
	mesh.update()
	before_face = list(mesh.polygons[0].vertices)
	before_vertex_normals = [vertex.normal.copy() for vertex in mesh.vertices]
	centroid = sum((vertex.co for vertex in mesh.vertices), mesh.vertices[0].co.copy()) / max(1, len(mesh.vertices))

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
	outward_samples = 0
	inward_samples = 0
	for vertex, normal in zip(mesh.vertices, before_vertex_normals):
		direction = vertex.co - centroid
		if direction.length <= 1e-5:
			continue
		direction.normalize()
		if direction.dot(normal) >= 0.0:
			outward_samples += 1
		else:
			inward_samples += 1
	return {
		"applied": True,
		"method": "shade_smooth before bmesh.ops.reverse_faces + restored captured outward vertex normals as custom split normals",
		"face_before": before_face,
		"face_after": after_face,
		"face_order_reversed": after_face[1:] == before_face[:0:-1]     ,
		"outward_normal_samples": outward_samples,
		"inward_normal_samples": inward_samples,
	}


def dump_uv_triangles(obj: bpy.types.Object, output_path: Path) -> dict[str, object]:
	mesh = obj.data
	uv_layer = mesh.uv_layers.active
	if uv_layer is None:
		raise RuntimeError(f"Mesh {obj.name} has no active UV layer for texture padding")
	mesh.calc_loop_triangles()
	triangles = []
	for triangle in mesh.loop_triangles:
		uvs = []
		for loop_index in triangle.loops:
			uv = uv_layer.data[loop_index].uv
			uvs.append([float(uv.x), float(uv.y)])
		triangles.append(uvs)
	output_path.parent.mkdir(parents=True, exist_ok=True)
	np.savez_compressed(output_path, triangles=np.asarray(triangles, dtype=np.float32))
	return {"path": str(output_path), "triangle_count": len(triangles)}


def apply_shading_normal_editing(obj: bpy.types.Object, is_humanoid: bool) -> dict[str, object]:
	if is_humanoid:
		return {
			"applied": False,
			"reason": "skipped_for_humanoid_to_avoid_overwriting_transfer_face_normals",
		}
	bpy.ops.object.select_all(action="DESELECT")
	obj.select_set(True)
	bpy.context.view_layer.objects.active = obj
	bpy.ops.object.shade_smooth()
	for poly in obj.data.polygons:
		poly.use_smooth = True
	report: dict[str, object] = {
		"applied": True,
		"method": "shade_smooth",
		"weighted_normal_modifier_applied": False,
	}
	try:
		modifier = obj.modifiers.new("T66_R2_WeightedNormals", "WEIGHTED_NORMAL")
		modifier.keep_sharp = True
		modifier.weight = 50
		bpy.ops.object.modifier_apply(modifier=modifier.name)
		report["weighted_normal_modifier_applied"] = True
		report["method"] = "shade_smooth + WeightedNormal modifier keep_sharp weight=50"
	except Exception as exc:
		report["weighted_normal_modifier_error"] = str(exc)
	obj.data.update()
	return report


def run_texture_postprocess(
	input_path: Path,
	output_path: Path,
	asset_name: str,
	uv_triangles_path: Path,
	python_exe: str,
	padding_pixels: int,
	speckle_threshold: int,
	generate_tint: bool,
	palette_centers: list[list[int]] | None,
) -> dict[str, object]:
	report_path = output_path.with_name(output_path.stem + "_postprocess.json")
	tint_path = output_path.with_name(f"{asset_name}_Tint.png") if generate_tint else None
	command = [
		python_exe,
		str(SCRIPT_DIR / "texture_postprocess.py"),
		"--input",
		str(input_path),
		"--output",
		str(output_path),
		"--uv-triangles",
		str(uv_triangles_path),
		"--padding-pixels",
		str(padding_pixels),
		"--speckle-threshold",
		str(speckle_threshold),
		"--report",
		str(report_path),
	]
	if tint_path:
		command.extend(["--tint-output", str(tint_path)])
	if palette_centers:
		command.extend(["--palette-centers-json", json.dumps(palette_centers)])
	completed = subprocess.run(command, check=True, text=True, capture_output=True)
	report = json.loads(report_path.read_text(encoding="utf-8"))
	report["stdout"] = completed.stdout.strip()
	report["stderr"] = completed.stderr.strip()
	return report


def run_inner_line_bake(
	segments_path: Path,
	uv_triangles_path: Path,
	output_path: Path,
	python_exe: str,
	padding_pixels: int,
	size: int,
) -> dict[str, object]:
	report_path = output_path.with_name(output_path.stem + "_report.json")
	command = [
		python_exe,
		str(SCRIPT_DIR / "inner_line_bake.py"),
		"--segments",
		str(segments_path),
		"--uv-triangles",
		str(uv_triangles_path),
		"--output",
		str(output_path),
		"--size",
		str(size),
		"--padding-pixels",
		str(padding_pixels),
		"--report",
		str(report_path),
	]
	completed = subprocess.run(command, check=True, text=True, capture_output=True)
	report = json.loads(report_path.read_text(encoding="utf-8"))
	report["stdout"] = completed.stdout.strip()
	report["stderr"] = completed.stderr.strip()
	return report


def apply_foundation_tools(
	shading_obj: bpy.types.Object,
	outline_obj: bpy.types.Object,
	args: argparse.Namespace,
	uv_triangles_path: Path,
) -> dict[str, object]:
	if not args.enable_foundation_tools:
		return {"enabled": False, "reason": "flag_not_set"}
	curvature_values, curvature_report = analyze_vertex_curvature(shading_obj)
	write_curvature_report(
		args.working_dir / f"{args.asset_name}_curvature_report.json",
		args.asset_name,
		curvature_values,
		curvature_report,
	)
	result: dict[str, object] = {
		"enabled": True,
		"asset_class": args.asset_class,
		"accepted_limitation": bool(args.accepted_limitation),
		"curvature": curvature_report,
	}
	if args.accepted_limitation or args.asset_class == "accepted-limitation":
		result["close_the_gap"] = {"status": "skipped", "reason": "accepted_limitation_no_remesh_topology"}
		result["inner_lines"] = {"status": "skipped", "reason": "accepted_limitation_no_remesh_topology"}
		return result

	b_values, transfer_report = curvature_to_close_gap_b(curvature_values, args.asset_class)
	readback = author_outline_b(outline_obj, b_values)
	result["close_the_gap"] = {
		"status": "applied",
		"transfer_function": transfer_report,
		"readback": readback,
	}

	segments = uv_segments_from_curvature(shading_obj, curvature_values, percentile=82.0 if args.asset_class == "prop" else 85.0)
	segments_path = args.working_dir / f"{args.asset_name}_inner_line_segments.npz"
	np.savez_compressed(segments_path, segments=segments)
	padding_pixels = 32 if args.no_remesh_export else 16
	inner_line_path = args.working_dir / f"{args.asset_name}_InnerLines.png"
	inner_report = run_inner_line_bake(
		segments_path,
		uv_triangles_path,
		inner_line_path,
		args.texture_postprocess_python,
		padding_pixels=padding_pixels,
		size=args.inner_line_size,
	)
	result["inner_lines"] = {
		"status": "applied",
		"path": str(inner_line_path),
		"segments_path": str(segments_path),
		"segments": int(len(segments)),
		"padding_pixels": padding_pixels,
		"bake_report": inner_report,
	}
	return result


def flatten_extracted_textures(
	textures: list[dict[str, object]],
	asset_name: str,
	k: int,
	highlight_cap: float,
	uv_triangles_path: Path,
	no_remesh_export: bool,
	python_exe: str,
) -> tuple[list[dict[str, object]], dict[str, object] | None]:
	reports: list[dict[str, object]] = []
	tint_texture: dict[str, object] | None = None
	padding_pixels = 32 if no_remesh_export else 16
	speckle_threshold = 8
	for index, entry in enumerate(textures):
		path = Path(str(entry["path"]))
		image = None
		for candidate in bpy.data.images:
			if candidate.filepath_raw and Path(candidate.filepath_raw).name == path.name:
				image = candidate
				break
		if image is None and index < len(bpy.data.images):
			# Pixal3D GLBs generally contain one or two real images and no render
			# targets. Fall back by exported path order if Blender does not retain
			# the saved filepath on the image datablock.
			candidates = [
				img for img in bpy.data.images
				if img.name not in {"Render Result", "Viewer Node"} and not img.name.startswith(".")
			]
			if index < len(candidates):
				image = candidates[index]
		if image is None:
			raise RuntimeError(f"Could not match extracted texture {path} to a Blender image")
		original_path = path.with_name(path.stem + "_original.png")
		if path.exists() and not original_path.exists():
			original_path.write_bytes(path.read_bytes())
		flattened_path = path.with_name(path.stem + "_flattened.png")
		report = flatten_blender_image(image, flattened_path, k=k, highlight_cap=highlight_cap)
		post_report = run_texture_postprocess(
			flattened_path,
			flattened_path,
			asset_name,
			uv_triangles_path,
			python_exe,
			padding_pixels=padding_pixels,
			speckle_threshold=speckle_threshold,
			generate_tint=index == 0,
			palette_centers=report.get("centers_rgb_255"),
		)
		report["original_path"] = str(original_path)
		report["flattened_path"] = str(flattened_path)
		report["postprocess"] = post_report
		entry["original_path"] = str(original_path)
		entry["path"] = str(flattened_path)
		entry["flattened"] = True
		entry["flatten_report"] = report
		entry["postprocess_report"] = post_report
		if index == 0 and post_report.get("tint", {}).get("generated"):
			tint_texture = {
				"path": post_report["tint"]["output"],
				"source_texture": str(flattened_path),
				"postprocess_report": str(flattened_path.with_name(flattened_path.stem + "_postprocess.json")),
			}
		reports.append(report)
	return reports, tint_texture


def bounds_report(obj: bpy.types.Object) -> dict[str, object]:
	mins, maxs = world_bounds(obj)
	dims = maxs - mins
	return {
		"min": [mins.x, mins.y, mins.z],
		"max": [maxs.x, maxs.y, maxs.z],
		"dimensions": [dims.x, dims.y, dims.z],
		"height": dims.z,
	}


def sample_head_normal_delta(
	shading_obj: bpy.types.Object,
	outline_obj: bpy.types.Object,
	normal_transfer_report: dict[str, object] | None = None,
) -> dict[str, object]:
	shading_obj.data.calc_loop_triangles()
	outline_obj.data.calc_loop_triangles()
	mins, maxs = world_bounds(shading_obj)
	center = None
	radius = None
	if normal_transfer_report:
		report_center = normal_transfer_report.get("proxy_center")
		report_radius = normal_transfer_report.get("proxy_radius")
		if isinstance(report_center, list) and len(report_center) == 3 and report_radius:
			center = Vector((float(report_center[0]), float(report_center[1]), float(report_center[2])))
			radius = float(report_radius)
	threshold = mins.z + ((maxs.z - mins.z) * 0.75)
	for loop in shading_obj.data.loops:
		vertex = shading_obj.data.vertices[loop.vertex_index]
		world_pos = shading_obj.matrix_world @ vertex.co
		if center is not None and radius is not None:
			if (world_pos - center).length > radius:
				continue
		elif world_pos.z < threshold:
			continue
		outline_loop = outline_obj.data.loops[loop.index]
		shading_n = loop.normal.copy()
		outline_n = outline_loop.normal.copy()
		delta = (shading_n - outline_n).length
		return {
			"loop_index": loop.index,
			"vertex_index": loop.vertex_index,
			"position": [world_pos.x, world_pos.y, world_pos.z],
			"shading_normal": [shading_n.x, shading_n.y, shading_n.z],
			"outline_normal": [outline_n.x, outline_n.y, outline_n.z],
			"delta": delta,
			"distinct": delta > 0.01,
			"sample_region": "face_mask" if center is not None else "legacy_top_25_percent",
		}
	return {"distinct": False, "reason": "no_face_mask_loop_sample" if center is not None else "no_head_loop_sample"}


def main() -> int:
	args = parse_args()
	args.working_dir.mkdir(parents=True, exist_ok=True)
	clear_scene()

	objects = import_glb(args.input)
	reuse_manifest: dict[str, object] | None = None
	if args.reuse_textures_manifest:
		reuse_manifest = json.loads(args.reuse_textures_manifest.read_text(encoding="utf-8"))
		textures = reuse_manifest.get("textures") or []
		if not textures:
			raise RuntimeError(f"Reuse manifest has no textures: {args.reuse_textures_manifest}")
	else:
		textures = extract_textures(objects, args.working_dir, args.asset_name)
	mesh_count_before = len(objects)
	joined = join_meshes(objects)
	normalization = normalize_spatial(joined, args.target_height)
	uv_triangles = dump_uv_triangles(joined, args.working_dir / f"{args.asset_name}_uv_triangles.npz")
	if reuse_manifest is not None:
		flatten_reports = reuse_manifest.get("texture_flattening") or []
		tint_texture = reuse_manifest.get("tint_texture")
	else:
		flatten_reports, tint_texture = flatten_extracted_textures(
			textures,
			args.asset_name,
			args.flatten_k,
			args.highlight_cap,
			args.working_dir / f"{args.asset_name}_uv_triangles.npz",
			args.no_remesh_export,
			args.texture_postprocess_python,
		)
	material_count_before = strip_material_texture_references(joined, args.asset_name)
	vertex_color_report = author_vertex_colors(joined)
	shading_normal_editing_report = apply_shading_normal_editing(joined, args.is_humanoid)

	outline_obj = duplicate_outline_object(joined, args.asset_name)
	outline_winding_report = reverse_winding_preserve_vertex_normals(outline_obj)
	foundation_report = apply_foundation_tools(
		joined,
		outline_obj,
		args,
		args.working_dir / f"{args.asset_name}_uv_triangles.npz",
	)
	if args.is_humanoid and args.enable_humanoid_face_normal_transfer:
		normal_transfer_report = transfer_face_normals(joined, args.is_humanoid, args.asset_name)
		normal_delta_report = sample_head_normal_delta(joined, outline_obj, normal_transfer_report)
	elif args.is_humanoid:
		normal_transfer_report = {
			"enabled": False,
			"method": "disabled_original_normals",
			"reason": "humanoid_face_normal_transfer_deprecated_disabled_by_default",
			"experimental_opt_in_flag": "--enable-humanoid-face-normal-transfer",
		}
		normal_delta_report = {"distinct": True, "reason": "humanoid_face_normal_transfer_deprecated_disabled"}
	else:
		normal_transfer_report = {"enabled": False, "method": "not_applicable", "reason": "non_humanoid"}
		normal_delta_report = {"distinct": True, "reason": "non_humanoid"}

	shading_fbx = args.working_dir / f"{args.asset_name}.fbx"
	outline_fbx = args.working_dir / f"{args.asset_name}_outline.fbx"
	shading_export = export_fbx(joined, shading_fbx)
	outline_export = export_fbx(outline_obj, outline_fbx)

	manifest = {
		"asset_name": args.asset_name,
		"input_glb": str(args.input),
		"working_dir": str(args.working_dir),
		"fbx_path": str(shading_fbx),
		"outline_fbx_path": str(outline_fbx),
		"textures": textures,
		"tint_texture": tint_texture,
		"is_humanoid": args.is_humanoid,
		"retained_from_phase1a": args.retained_from_phase1a,
		"no_remesh_export": args.no_remesh_export,
		"asset_class": args.asset_class,
		"accepted_limitation": args.accepted_limitation,
		"flatten_k": args.flatten_k,
		"flatten_highlight_cap": args.highlight_cap,
		"texture_flattening": flatten_reports,
		"texture_postprocess_dependency_path": flatten_reports[0].get("postprocess", {}).get("dependency_path") if flatten_reports else None,
		"reused_textures_manifest": str(args.reuse_textures_manifest) if args.reuse_textures_manifest else None,
		"uv_triangles": uv_triangles,
		"mesh_count_before_join": mesh_count_before,
		"mesh_count_after_join": 1,
		"vertex_count": len(joined.data.vertices),
		"outline_vertex_count": len(outline_obj.data.vertices),
		"material_count_before_strip": material_count_before,
		"material_count_after_strip": len(joined.data.materials),
		"target_height": args.target_height,
		"normalization": normalization,
		"shading_bounds": bounds_report(joined),
		"outline_bounds": bounds_report(outline_obj),
		"vertex_colors": vertex_color_report,
		"shading_normal_editing": shading_normal_editing_report,
		"normal_transfer": normal_transfer_report,
		"normal_delta_sample": normal_delta_report,
		"outline_winding": outline_winding_report,
		"foundation_pass": foundation_report,
		"fbx_export_options": {
			"shading": {key: sorted(value) if isinstance(value, set) else value for key, value in shading_export.items() if key != "filepath"},
			"outline": {key: sorted(value) if isinstance(value, set) else value for key, value in outline_export.items() if key != "filepath"},
		},
	}
	if foundation_report.get("inner_lines", {}).get("path"):
		inner_path = Path(str(foundation_report["inner_lines"]["path"]))
		manifest["inner_line_texture"] = {
			"path": str(inner_path),
			"report": str(inner_path.with_name(inner_path.stem + "_report.json")),
		}
	manifest_path = args.working_dir / f"{args.asset_name}_manifest.json"
	manifest_path.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
	print(json.dumps(manifest, sort_keys=True))
	return 0


if __name__ == "__main__":
	raise SystemExit(main())
