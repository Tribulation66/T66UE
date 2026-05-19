#!/usr/bin/env python3
"""Reusable Blender helpers for ToonStyle Pixal3D conversion."""

from __future__ import annotations

import math
from pathlib import Path

import bpy
from mathutils import Vector


def clear_scene() -> None:
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete()


def mesh_objects() -> list[bpy.types.Object]:
    return [obj for obj in bpy.context.scene.objects if obj.type == "MESH"]


def import_glb(path: Path | str) -> list[bpy.types.Object]:
    bpy.ops.import_scene.gltf(filepath=str(path))
    imported_meshes = mesh_objects()
    if not imported_meshes:
        raise RuntimeError(f"No mesh objects imported from {path}")
    return imported_meshes


def world_bounds(obj: bpy.types.Object) -> tuple[Vector, Vector]:
    corners = [obj.matrix_world @ Vector(corner) for corner in obj.bound_box]
    mins = Vector((min(c.x for c in corners), min(c.y for c in corners), min(c.z for c in corners)))
    maxs = Vector((max(c.x for c in corners), max(c.y for c in corners), max(c.z for c in corners)))
    return mins, maxs


def dimensions(obj: bpy.types.Object) -> Vector:
    mins, maxs = world_bounds(obj)
    return maxs - mins


def join_meshes(objects: list[bpy.types.Object]) -> bpy.types.Object:
    if not objects:
        raise RuntimeError("No mesh objects supplied for join")
    bpy.ops.object.select_all(action="DESELECT")
    for obj in objects:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = objects[0]
    if len(objects) > 1:
        bpy.ops.object.join()
    return bpy.context.view_layer.objects.active


def maybe_correct_y_up(obj: bpy.types.Object) -> bool:
    dims = dimensions(obj)
    if dims.z <= 0.0:
        return False
    if dims.y > dims.z * 1.5 and dims.y > dims.x * 1.15:
        obj.rotation_euler.rotate_axis("X", math.radians(90.0))
        bpy.context.view_layer.objects.active = obj
        obj.select_set(True)
        bpy.ops.object.transform_apply(location=False, rotation=True, scale=False)
        return True
    return False


def normalize_spatial(obj: bpy.types.Object, target_height: float = 180.0) -> dict[str, object]:
    corrected_y_up = maybe_correct_y_up(obj)
    dims_before = dimensions(obj)
    if dims_before.z <= 0.0:
        raise RuntimeError("Imported mesh has zero Z height; cannot normalize scale.")

    scale = target_height / dims_before.z
    obj.scale = (obj.scale.x * scale, obj.scale.y * scale, obj.scale.z * scale)
    bpy.context.view_layer.objects.active = obj
    obj.select_set(True)
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)

    mins, maxs = world_bounds(obj)
    center_x = (mins.x + maxs.x) * 0.5
    center_y = (mins.y + maxs.y) * 0.5
    obj.location -= Vector((center_x, center_y, mins.z))
    bpy.ops.object.transform_apply(location=True, rotation=False, scale=False)

    final_mins, final_maxs = world_bounds(obj)
    final_dims = final_maxs - final_mins
    return {
        "scale_factor": scale,
        "bounds_min": [final_mins.x, final_mins.y, final_mins.z],
        "bounds_max": [final_maxs.x, final_maxs.y, final_maxs.z],
        "dimensions": [final_dims.x, final_dims.y, final_dims.z],
        "height": final_dims.z,
        "corrected_y_up": corrected_y_up,
    }


def export_image_as_png(image: bpy.types.Image, output_path: Path) -> None:
    output_path.parent.mkdir(parents=True, exist_ok=True)
    original_path = image.filepath_raw
    original_format = image.file_format
    try:
        image.filepath_raw = str(output_path)
        image.file_format = "PNG"
        image.save()
    finally:
        image.filepath_raw = original_path
        image.file_format = original_format


def extract_textures(_objects: list[bpy.types.Object], output_dir: Path | str, asset_name: str) -> list[dict[str, object]]:
    output_dir = Path(output_dir)
    images = [
        image
        for image in bpy.data.images
        if image.name not in {"Render Result", "Viewer Node"} and not image.name.startswith(".")
    ]
    names = [f"{asset_name}_diffuse.png"] if len(images) == 1 else [f"{asset_name}_{index}.png" for index in range(len(images))]
    extracted: list[dict[str, object]] = []
    for image, file_name in zip(images, names):
        output_path = output_dir / file_name
        export_image_as_png(image, output_path)
        extracted.append(
            {
                "image_name": image.name,
                "source_format": str(image.file_format),
                "path": str(output_path),
                "size": [int(image.size[0]), int(image.size[1])],
            }
        )
    return extracted


def strip_material_texture_references(obj: bpy.types.Object, asset_name: str) -> int:
    material_count_before = len(obj.data.materials) if obj and obj.data else 0
    obj.data.materials.clear()
    material = bpy.data.materials.new(f"M_{asset_name}_placeholder")
    material.diffuse_color = (0.6, 0.6, 0.6, 1.0)
    obj.data.materials.append(material)
    return material_count_before


def export_fbx(obj: bpy.types.Object, output_path: Path) -> dict[str, object]:
    output_path.parent.mkdir(parents=True, exist_ok=True)
    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    export_options = {
        "filepath": str(output_path),
        "use_selection": True,
        "object_types": {"MESH"},
        "global_scale": 0.01,
        "apply_unit_scale": True,
        "apply_scale_options": "FBX_SCALE_ALL",
        "bake_space_transform": False,
        "path_mode": "STRIP",
        "embed_textures": False,
        "use_mesh_modifiers": True,
        "mesh_smooth_type": "FACE",
        "add_leaf_bones": False,
    }
    bpy.ops.export_scene.fbx(**export_options)
    return export_options
