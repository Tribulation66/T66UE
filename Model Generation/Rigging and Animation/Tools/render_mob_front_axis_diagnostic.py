"""
Render front-axis diagnostic views for one Easy mob source GLB.

This is a Blender QA tool, not an Unreal import step. It renders the source
mesh from +Y, -Y, +X, and -X using unlit materials so front/back decisions are
based on mesh shape and texture, not lighting.

Run:
  blender --background --python "Model Generation/Rigging and Animation/Tools/render_mob_front_axis_diagnostic.py" -- --enemy-id BoneWalker
"""

from __future__ import annotations

import argparse
import importlib.util
import json
import math
import sys
from pathlib import Path

import bpy
from mathutils import Vector


PROJECT_ROOT = Path(__file__).resolve().parents[3]
TOOLS_ROOT = PROJECT_ROOT / "Model Generation" / "Rigging and Animation" / "Tools"
VAT_TOOL_PATH = TOOLS_ROOT / "create_easy_mob_vat_sources.py"
DEFAULT_RUN_ROOT = (
    PROJECT_ROOT
    / "Model Generation"
    / "Rigging and Animation"
    / "Runs"
    / "BoneWalker_FrontAxisDiagnostic_20260525"
)


AXES = {
    "plus_y": Vector((0.0, 1.0, 0.0)),
    "minus_y": Vector((0.0, -1.0, 0.0)),
    "plus_x": Vector((1.0, 0.0, 0.0)),
    "minus_x": Vector((-1.0, 0.0, 0.0)),
}


def load_vat_tool():
    spec = importlib.util.spec_from_file_location("easy_mob_vat_sources", VAT_TOOL_PATH)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"Could not load VAT source tool: {VAT_TOOL_PATH}")
    module = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = module
    spec.loader.exec_module(module)
    return module


def look_at(obj: bpy.types.Object, target: Vector) -> None:
    direction = target - obj.location
    obj.rotation_euler = direction.to_track_quat("-Z", "Y").to_euler()


def bounds_for_objects(objects) -> tuple[Vector, Vector, Vector]:
    coords = []
    for obj in objects:
        if obj.type != "MESH":
            continue
        coords.extend(obj.matrix_world @ Vector(corner) for corner in obj.bound_box)
    if not coords:
        raise RuntimeError("No mesh bounds available")
    min_v = Vector((min(c.x for c in coords), min(c.y for c in coords), min(c.z for c in coords)))
    max_v = Vector((max(c.x for c in coords), max(c.y for c in coords), max(c.z for c in coords)))
    return min_v, max_v, (min_v + max_v) * 0.5


def find_base_color_image(material: bpy.types.Material) -> bpy.types.Image | None:
    if not material or not material.use_nodes or not material.node_tree:
        return None
    for node in material.node_tree.nodes:
        if node.type != "BSDF_PRINCIPLED":
            continue
        base_input = node.inputs.get("Base Color")
        if not base_input:
            continue
        for link in base_input.links:
            if link.from_node.type == "TEX_IMAGE" and getattr(link.from_node, "image", None):
                return link.from_node.image
    for node in material.node_tree.nodes:
        if node.type == "TEX_IMAGE" and getattr(node, "image", None):
            return node.image
    return None


def convert_material_to_unlit_emissive(material: bpy.types.Material) -> None:
    image = find_base_color_image(material)
    fallback_color = tuple(material.diffuse_color) if material else (1.0, 1.0, 1.0, 1.0)
    material.use_nodes = True
    tree = material.node_tree
    tree.nodes.clear()

    output = tree.nodes.new(type="ShaderNodeOutputMaterial")
    output.location = (360, 0)
    emission = tree.nodes.new(type="ShaderNodeEmission")
    emission.location = (110, 0)
    emission.inputs["Strength"].default_value = 1.0

    if image:
        texture = tree.nodes.new(type="ShaderNodeTexImage")
        texture.location = (-160, 0)
        texture.image = image
        tree.links.new(texture.outputs["Color"], emission.inputs["Color"])
    else:
        emission.inputs["Color"].default_value = fallback_color

    tree.links.new(emission.outputs["Emission"], output.inputs["Surface"])


def apply_unlit_preview_materials(mesh: bpy.types.Object) -> None:
    for material in mesh.data.materials:
        if material:
            convert_material_to_unlit_emissive(material)


def add_axis_marker(name: str, direction: Vector, center: Vector, radius: float, floor_z: float) -> None:
    mat = bpy.data.materials.get("Axis_Marker_Material")
    if not mat:
        mat = bpy.data.materials.new("Axis_Marker_Material")
        mat.diffuse_color = (1.0, 0.80, 0.10, 1.0)

    location = center + direction * radius * 1.25
    bpy.ops.mesh.primitive_cone_add(
        vertices=3,
        radius1=radius * 0.08,
        radius2=0.0,
        depth=radius * 0.22,
        location=(location.x, location.y, floor_z + radius * 0.16),
        rotation=(math.radians(90.0), 0.0, math.atan2(direction.y, direction.x) - math.radians(90.0)),
    )
    marker = bpy.context.object
    marker.name = f"{name}_Axis_Direction_Marker"
    marker.data.materials.append(mat)


def setup_scene(width: int, height: int) -> bpy.types.Object:
    scene = bpy.context.scene
    scene.render.engine = "BLENDER_EEVEE"
    scene.eevee.taa_render_samples = 64
    scene.render.resolution_x = width
    scene.render.resolution_y = height
    scene.view_settings.view_transform = "Standard"
    scene.view_settings.look = "None"
    scene.world = bpy.data.worlds.new("FrontAxisDiagnostic_World") if not scene.world else scene.world
    scene.world.color = (0.50, 0.52, 0.54)
    scene.render.image_settings.file_format = "PNG"

    camera_data = bpy.data.cameras.new("FrontAxisDiagnostic_Camera")
    camera_data.lens = 72.0
    camera = bpy.data.objects.new("FrontAxisDiagnostic_Camera", camera_data)
    bpy.context.collection.objects.link(camera)
    scene.camera = camera
    return camera


def render_diagnostic(enemy_id: str, out_root: Path, width: int, height: int) -> dict:
    vat = load_vat_tool()
    spec = next((item for item in vat.MOBS if item.enemy_id.lower() == enemy_id.lower()), None)
    if spec is None:
        available = ", ".join(item.enemy_id for item in vat.MOBS)
        raise ValueError(f"Unknown --enemy-id {enemy_id}. Available: {available}")

    out_root.mkdir(parents=True, exist_ok=True)
    vat.reset_scene()
    camera = setup_scene(width, height)

    mesh = vat.import_glb(spec.source_glb)
    mesh.name = f"{spec.enemy_id}_FrontAxisDiagnostic_Mesh"
    mesh.data.name = f"{spec.enemy_id}_FrontAxisDiagnostic_MeshData"
    apply_unlit_preview_materials(mesh)

    bpy.context.view_layer.update()
    min_v, max_v, center = bounds_for_objects([mesh])
    size = max_v - min_v
    radius = max(size.x, size.y, size.z, 0.5)
    focus_z = min_v.z + max(size.z, 0.1) * 0.50
    floor_z = min_v.z - radius * 0.04

    bpy.ops.mesh.primitive_plane_add(size=radius * 3.4, location=(center.x, center.y, floor_z))
    floor = bpy.context.object
    floor.name = "FrontAxisDiagnostic_Floor"
    mat = bpy.data.materials.new("FrontAxisDiagnostic_Floor_Material")
    mat.diffuse_color = (0.36, 0.37, 0.38, 1.0)
    floor.data.materials.append(mat)

    outputs: dict[str, str] = {}
    for axis_name, direction in AXES.items():
        add_axis_marker(axis_name, direction, center, radius, floor_z)
        camera.location = Vector((
            center.x + direction.x * radius * 2.55,
            center.y + direction.y * radius * 2.55,
            focus_z + radius * 0.48,
        ))
        look_at(camera, Vector((center.x, center.y, focus_z)))
        out_path = out_root / f"{spec.enemy_id}_front_axis_{axis_name}.png"
        bpy.context.scene.render.filepath = str(out_path)
        bpy.context.scene.frame_set(1)
        bpy.ops.render.render(write_still=True)
        outputs[axis_name] = str(out_path)

        marker = bpy.data.objects.get(f"{axis_name}_Axis_Direction_Marker")
        if marker:
            bpy.data.objects.remove(marker, do_unlink=True)

    blend_path = out_root / f"{spec.enemy_id}_front_axis_diagnostic.blend"
    bpy.ops.wm.save_as_mainfile(filepath=str(blend_path))

    manifest = {
        "enemy_id": spec.enemy_id,
        "source_glb": str(spec.source_glb),
        "blend": str(blend_path),
        "outputs": outputs,
        "bounds": {
            "min": [min_v.x, min_v.y, min_v.z],
            "max": [max_v.x, max_v.y, max_v.z],
            "size": [size.x, size.y, size.z],
        },
        "qa_note": "Front-axis proof only. Do not choose final front until the four views are visually inspected.",
    }
    manifest_path = out_root / f"{spec.enemy_id}_front_axis_diagnostic_manifest.json"
    manifest_path.write_text(json.dumps(manifest, indent=2), encoding="utf-8")
    print(json.dumps(manifest, indent=2))
    return manifest


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--enemy-id", default="BoneWalker")
    parser.add_argument("--out-root", default=str(DEFAULT_RUN_ROOT))
    parser.add_argument("--width", type=int, default=900)
    parser.add_argument("--height", type=int, default=900)
    argv = sys.argv
    args = parser.parse_args(argv[argv.index("--") + 1 :] if "--" in argv else [])
    render_diagnostic(args.enemy_id, Path(args.out_root), args.width, args.height)


if __name__ == "__main__":
    main()
