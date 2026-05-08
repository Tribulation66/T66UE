#!/usr/bin/env python3
"""
Quad Remesher based retro character processing for T66.

Run from Blender:

blender --background --python "Model Generation/Scripts/t66_quad_retro_character_pipeline.py" -- ^
  --input "Model Generation/Runs/Arthur/Raw/Arthur_HeroReference_Full_White_S1337_D80000_Trellis2.glb" ^
  --output-dir "Model Generation/Runs/Heroes/QuadRetroPipelineSmoke01" ^
  --target-quads 5000 ^
  --adaptive-size 50 ^
  --texture-size 256 ^
  --palette-size 24 ^
  --dither-type bayer4 ^
  --dither-strength 0.85 ^
  --render-qa true

The script intentionally writes new files only. It does not modify source GLBs,
Unreal assets, or DataTables.
"""

from __future__ import annotations

import argparse
import json
import math
import os
import shutil
import subprocess
import sys
import tempfile
import time
import traceback
from pathlib import Path
from typing import Iterable

import bpy
from mathutils import Vector


BAYER_4X4 = (
    (0, 8, 2, 10),
    (12, 4, 14, 6),
    (3, 11, 1, 9),
    (15, 7, 13, 5),
)

BAYER_8X8 = (
    (0, 48, 12, 60, 3, 51, 15, 63),
    (32, 16, 44, 28, 35, 19, 47, 31),
    (8, 56, 4, 52, 11, 59, 7, 55),
    (40, 24, 36, 20, 43, 27, 39, 23),
    (2, 50, 14, 62, 1, 49, 13, 61),
    (34, 18, 46, 30, 33, 17, 45, 29),
    (10, 58, 6, 54, 9, 57, 5, 53),
    (42, 26, 38, 22, 41, 25, 37, 21),
)


def str_bool(value: str | bool) -> bool:
    if isinstance(value, bool):
        return value
    value = value.strip().lower()
    if value in {"1", "true", "yes", "y", "on"}:
        return True
    if value in {"0", "false", "no", "n", "off"}:
        return False
    raise argparse.ArgumentTypeError(f"Expected boolean value, got {value!r}")


def parse_args() -> argparse.Namespace:
    argv = []
    if "--" in os.sys.argv:
        argv = os.sys.argv[os.sys.argv.index("--") + 1 :]

    parser = argparse.ArgumentParser()
    parser.add_argument("--input", required=True, help="Input GLB/GLTF/FBX from Trellis or prior processing.")
    parser.add_argument("--output-dir", required=True, help="Run output directory.")
    parser.add_argument("--label", default="", help="Optional label used in output names.")
    parser.add_argument("--suffix", default="_QuadRetro", help="Suffix for exported model names.")

    parser.add_argument("--qremesh-engine", default="", help="Path to xremesh.exe. Defaults to ProgramData install.")
    parser.add_argument("--retopo-fbx", default="", help="Optional existing Quad Remesher FBX to reuse instead of running xremesh.")
    parser.add_argument("--target-quads", type=int, default=5000)
    parser.add_argument("--adaptive-size", type=float, default=50.0)
    parser.add_argument("--adapt-quad-count", type=str_bool, default=False)
    parser.add_argument("--use-materials", type=str_bool, default=True)
    parser.add_argument("--use-normals", type=str_bool, default=False)
    parser.add_argument("--autodetect-hard-edges", type=str_bool, default=True)
    parser.add_argument("--symmetry-x", type=str_bool, default=False)
    parser.add_argument("--symmetry-y", type=str_bool, default=False)
    parser.add_argument("--symmetry-z", type=str_bool, default=False)

    parser.add_argument("--normalize-height", type=float, default=2.0, help="Target model height in Blender meters.")
    parser.add_argument("--merge-distance", type=float, default=0.0001)
    parser.add_argument("--delete-loose", type=str_bool, default=True)
    parser.add_argument("--recalculate-normals", type=str_bool, default=True)
    parser.add_argument("--shade-flat", type=str_bool, default=True)
    parser.add_argument("--qremesh-source-target-tris", type=int, default=0)

    parser.add_argument("--bake-size", type=int, default=1024)
    parser.add_argument("--bake-margin", type=int, default=16)
    parser.add_argument("--cage-extrusion", type=float, default=0.035)
    parser.add_argument("--max-ray-distance", type=float, default=0.12)
    parser.add_argument("--dilate-pixels", type=int, default=20)

    parser.add_argument("--texture-size", type=int, default=256)
    parser.add_argument("--palette-mode", choices=("none", "kmeans", "per-channel"), default="kmeans")
    parser.add_argument("--palette-size", type=int, default=24)
    parser.add_argument("--palette-steps", type=int, default=5)
    parser.add_argument("--kmeans-iterations", type=int, default=8)
    parser.add_argument("--kmeans-sample-limit", type=int, default=20000)
    parser.add_argument("--dither-type", choices=("none", "bayer4", "bayer8"), default="bayer4")
    parser.add_argument("--dither-strength", type=float, default=0.85)
    parser.add_argument("--unlit-emission-material", type=str_bool, default=True)

    parser.add_argument("--render-qa", type=str_bool, default=False)
    parser.add_argument("--qa-resolution", type=int, default=1200)
    parser.add_argument("--save-blend", type=str_bool, default=False)
    parser.add_argument("--quit-when-done", type=str_bool, default=True)
    parser.add_argument("--timeout-seconds", type=int, default=900)
    return parser.parse_args(argv)


def ensure_parent(path: str | Path) -> None:
    Path(path).parent.mkdir(parents=True, exist_ok=True)


def clear_scene() -> None:
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete()


def import_model(path: str) -> list[bpy.types.Object]:
    before = set(bpy.data.objects)
    lower = path.lower()
    if lower.endswith(".fbx"):
        bpy.ops.import_scene.fbx(filepath=path, use_image_search=True)
    elif lower.endswith((".glb", ".gltf")):
        bpy.ops.import_scene.gltf(filepath=path)
    else:
        raise RuntimeError(f"Unsupported model format: {path}")
    imported = [obj for obj in bpy.data.objects if obj not in before]
    if not any(obj.type == "MESH" for obj in imported):
        raise RuntimeError(f"No mesh objects imported from {path}")
    return imported


def mesh_objects(objects: Iterable[bpy.types.Object] | None = None) -> list[bpy.types.Object]:
    source = list(objects) if objects is not None else list(bpy.context.scene.objects)
    return [obj for obj in source if obj.type == "MESH"]


def triangle_count(meshes: Iterable[bpy.types.Object]) -> int:
    total = 0
    for obj in meshes:
        obj.data.calc_loop_triangles()
        total += len(obj.data.loop_triangles)
    return total


def quad_count(meshes: Iterable[bpy.types.Object]) -> int:
    total = 0
    for obj in meshes:
        for poly in obj.data.polygons:
            if len(poly.vertices) == 4:
                total += 1
    return total


def world_bbox(meshes: Iterable[bpy.types.Object]) -> tuple[Vector, Vector]:
    mins = Vector((math.inf, math.inf, math.inf))
    maxs = Vector((-math.inf, -math.inf, -math.inf))
    for obj in meshes:
        for corner in obj.bound_box:
            world = obj.matrix_world @ Vector(corner)
            mins.x = min(mins.x, world.x)
            mins.y = min(mins.y, world.y)
            mins.z = min(mins.z, world.z)
            maxs.x = max(maxs.x, world.x)
            maxs.y = max(maxs.y, world.y)
            maxs.z = max(maxs.z, world.z)
    return mins, maxs


def join_meshes(meshes: list[bpy.types.Object], name: str) -> bpy.types.Object:
    if len(meshes) == 1:
        obj = meshes[0]
        obj.name = name
        obj.data.name = f"{name}_Mesh"
        bpy.context.view_layer.objects.active = obj
        return obj

    bpy.ops.object.select_all(action="DESELECT")
    for obj in meshes:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = meshes[0]
    bpy.ops.object.join()
    joined = bpy.context.view_layer.objects.active
    joined.name = name
    joined.data.name = f"{name}_Mesh"
    return joined


def normalize_to_height(obj: bpy.types.Object, target_height: float) -> dict:
    bpy.context.view_layer.update()
    mins, maxs = world_bbox([obj])
    size = maxs - mins
    if target_height > 0.0 and size.z > 0.000001:
        scale = target_height / size.z
        obj.scale *= scale
        bpy.context.view_layer.update()
        bpy.ops.object.select_all(action="DESELECT")
        obj.select_set(True)
        bpy.context.view_layer.objects.active = obj
        bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)

    mins, maxs = world_bbox([obj])
    center_x = (mins.x + maxs.x) * 0.5
    center_y = (mins.y + maxs.y) * 0.5
    obj.location.x -= center_x
    obj.location.y -= center_y
    obj.location.z -= mins.z
    bpy.context.view_layer.update()
    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)

    final_mins, final_maxs = world_bbox([obj])
    final_size = final_maxs - final_mins
    return {
        "initial_height": size.z,
        "target_height": target_height,
        "final_size": [final_size.x, final_size.y, final_size.z],
        "final_min": [final_mins.x, final_mins.y, final_mins.z],
        "final_max": [final_maxs.x, final_maxs.y, final_maxs.z],
    }


def clean_mesh(obj: bpy.types.Object, merge_distance: float, delete_loose: bool, recalculate_normals: bool) -> None:
    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    bpy.ops.object.mode_set(mode="EDIT")
    bpy.ops.mesh.select_all(action="SELECT")
    if merge_distance > 0:
        bpy.ops.mesh.remove_doubles(threshold=merge_distance)
    if delete_loose:
        try:
            bpy.ops.mesh.delete_loose()
        except Exception:
            pass
    if recalculate_normals:
        bpy.ops.mesh.normals_make_consistent(inside=False)
    bpy.ops.object.mode_set(mode="OBJECT")
    obj.data.update()


def force_flat_shading(obj: bpy.types.Object) -> None:
    for poly in obj.data.polygons:
        poly.use_smooth = False
    obj.data.update()


def make_qremesh_source(source: bpy.types.Object, target_tris: int, name: str) -> tuple[bpy.types.Object, dict]:
    current_tris = triangle_count([source])
    report = {
        "source_triangles": current_tris,
        "target_triangles": target_tris,
        "used_duplicate": False,
        "output_triangles": current_tris,
    }
    if target_tris <= 0 or current_tris <= target_tris:
        return source, report

    dup = source.copy()
    dup.data = source.data.copy()
    dup.name = name
    dup.data.name = f"{name}_Mesh"
    source.users_collection[0].objects.link(dup)

    ratio = max(0.02, min(1.0, target_tris / max(1, current_tris)))
    mod = dup.modifiers.new(name="T66_QRemesh_Source_Decimate", type="DECIMATE")
    mod.decimate_type = "COLLAPSE"
    mod.ratio = ratio
    bpy.ops.object.select_all(action="DESELECT")
    dup.select_set(True)
    bpy.context.view_layer.objects.active = dup
    bpy.ops.object.modifier_apply(modifier=mod.name)
    clean_mesh(dup, 0.0, True, True)

    report.update(
        {
            "used_duplicate": True,
            "decimate_ratio": ratio,
            "output_triangles": triangle_count([dup]),
        }
    )
    return dup, report


def default_qremesh_engine_path() -> str:
    return str(
        Path(os.environ.get("ProgramData", r"C:\ProgramData"))
        / "Exoside"
        / "QuadRemesher"
        / "Datas_Blender"
        / "QuadRemesherEngine_1.4"
        / "xremesh.exe"
    )


def ensure_quad_remesher_bridge() -> None:
    try:
        import addon_utils

        addon_utils.enable("quad_remesher_1_4", default_set=True, persistent=True)
    except Exception as exc:
        print(f"warning: failed to enable Quad Remesher bridge add-on: {exc}")


def qremesher_host_work_dir() -> Path:
    return Path(tempfile.gettempdir()) / "Exoside" / "QuadRemesher" / "Blender"


def write_qremesh_settings(
    settings_path: Path,
    input_fbx: Path,
    output_fbx: Path,
    progress_path: Path,
    args: argparse.Namespace,
) -> None:
    sym = ""
    if args.symmetry_x:
        sym += "X"
    if args.symmetry_y:
        sym += "Y"
    if args.symmetry_z:
        sym += "Z"

    with open(settings_path, "w", encoding="ascii") as handle:
        handle.write("HostApp=Blender\n")
        handle.write(f"HostAppVer={bpy.app.version_string}\n")
        handle.write(f'FileIn="{str(input_fbx).replace(os.sep, "/")}"\n')
        handle.write(f'FileOut="{str(output_fbx).replace(os.sep, "/")}"\n')
        handle.write(f'ProgressFile="{str(progress_path).replace(os.sep, "/")}"\n')
        handle.write(f"TargetQuadCount={args.target_quads}\n")
        handle.write(f"CurvatureAdaptivness={args.adaptive_size}\n")
        handle.write(f"ExactQuadCount={int(not args.adapt_quad_count)}\n")
        handle.write("UseVertexColorMap=False\n")
        handle.write(f"UseMaterialIds={int(args.use_materials)}\n")
        handle.write(f"UseIndexedNormals={int(args.use_normals)}\n")
        handle.write(f"AutoDetectHardEdges={int(args.autodetect_hard_edges)}\n")
        if sym:
            handle.write(f"SymAxis={sym}\n")
            handle.write("SymLocal=1\n")


def export_selected_fbx(path: Path, obj: bpy.types.Object) -> None:
    ensure_parent(path)
    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    bpy.ops.export_scene.fbx(
        filepath=str(path),
        use_selection=True,
        bake_anim=False,
        global_scale=1,
        apply_unit_scale=False,
        apply_scale_options="FBX_SCALE_NONE",
        use_space_transform=False,
        axis_forward="-X",
        axis_up="Z",
    )


def read_progress(progress_path: Path) -> tuple[float | None, str]:
    if not progress_path.exists():
        return None, ""
    lines = progress_path.read_text(encoding="utf-8", errors="ignore").splitlines()
    if not lines:
        return None, ""
    try:
        value = float(lines[0].strip())
    except ValueError:
        return None, "\n".join(lines)
    text = lines[1].strip() if len(lines) > 1 else ""
    return value, text


def run_quad_remesher(source: bpy.types.Object, args: argparse.Namespace, work_dir: Path) -> tuple[Path, dict]:
    ensure_quad_remesher_bridge()
    engine = Path(args.qremesh_engine or default_qremesh_engine_path())
    if not engine.exists():
        raise RuntimeError(f"Quad Remesher engine missing: {engine}")

    host_dir = qremesher_host_work_dir()
    host_dir.mkdir(parents=True, exist_ok=True)
    input_fbx = host_dir / "inputMesh.fbx"
    output_fbx = host_dir / "retopo.fbx"
    settings_path = host_dir / "RetopoSettings.txt"
    progress_path = host_dir / "progress.txt"
    for path in (output_fbx, progress_path):
        if path.exists():
            path.unlink()

    export_selected_fbx(input_fbx, source)
    write_qremesh_settings(settings_path, input_fbx, output_fbx, progress_path, args)

    started = time.time()
    process = subprocess.Popen([str(engine), "-s", str(settings_path)])
    last_progress = None
    last_text = ""
    while True:
        value, text = read_progress(progress_path)
        if value is not None:
            last_progress = value
            last_text = text
            if value == 2:
                break
            if value == -2:
                raise RuntimeError(
                    "Quad Remesher needs EULA/trial/license activation. "
                    "Open xrLicenseManager.exe, activate the trial/license, then rerun."
                )
            if value < 0:
                raise RuntimeError(f"Quad Remesher failed with progress {value}: {text}")

        if process.poll() is not None and not output_fbx.exists():
            raise RuntimeError(
                f"Quad Remesher stopped without output. progress={last_progress} text={last_text!r}"
            )

        if time.time() - started > args.timeout_seconds:
            try:
                process.terminate()
            except Exception:
                pass
            raise RuntimeError(f"Quad Remesher timed out after {args.timeout_seconds}s")

        if output_fbx.exists() and process.poll() is not None:
            break

        time.sleep(0.5)

    process.wait(timeout=10)
    if not output_fbx.exists():
        raise RuntimeError(f"Quad Remesher reported success but no output exists: {output_fbx}")

    work_dir.mkdir(parents=True, exist_ok=True)
    copied_input = work_dir / "inputMesh.fbx"
    copied_output = work_dir / "retopo.fbx"
    copied_settings = work_dir / "RetopoSettings.txt"
    copied_progress = work_dir / "progress.txt"
    for src, dst in (
        (input_fbx, copied_input),
        (output_fbx, copied_output),
        (settings_path, copied_settings),
        (progress_path, copied_progress),
    ):
        try:
            shutil.copy2(src, dst)
        except Exception:
            pass

    return output_fbx, {
        "engine": str(engine),
        "input_fbx": str(input_fbx),
        "output_fbx": str(output_fbx),
        "settings": str(settings_path),
        "progress": str(progress_path),
        "copied_input_fbx": str(copied_input),
        "copied_output_fbx": str(copied_output),
        "copied_settings": str(copied_settings),
        "copied_progress": str(copied_progress),
        "elapsed_seconds": round(time.time() - started, 3),
        "last_progress": last_progress,
        "last_text": last_text,
    }


def import_retopo(path: Path, name: str) -> bpy.types.Object:
    before = set(bpy.data.objects)
    bpy.ops.import_scene.fbx(filepath=str(path), use_image_search=True)
    imported = [obj for obj in bpy.data.objects if obj not in before]
    meshes = mesh_objects(imported)
    if not meshes:
        raise RuntimeError(f"No retopo mesh objects imported from {path}")
    obj = join_meshes(meshes, name)
    return obj


def clear_qremesher_fbx_axis_rotation(obj: bpy.types.Object) -> None:
    """Quad Remesher FBX imports can carry a -90 Z object rotation in Blender."""
    if abs(obj.rotation_euler.z) < 0.000001:
        return
    obj.rotation_euler = (0.0, 0.0, 0.0)
    bpy.context.view_layer.update()


def unwrap_smart_project(obj: bpy.types.Object) -> None:
    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    if not obj.data.uv_layers:
        obj.data.uv_layers.new(name="UVMap")
    bpy.ops.object.mode_set(mode="EDIT")
    bpy.ops.mesh.select_all(action="SELECT")
    bpy.ops.uv.smart_project(angle_limit=math.radians(66.0), island_margin=0.012)
    bpy.ops.object.mode_set(mode="OBJECT")


def create_bake_target_material(obj: bpy.types.Object, image: bpy.types.Image) -> bpy.types.Material:
    mat = bpy.data.materials.new(f"{obj.name}_BakeTarget")
    mat.use_nodes = True
    nodes = mat.node_tree.nodes
    bsdf = nodes.get("Principled BSDF")
    tex = nodes.new(type="ShaderNodeTexImage")
    tex.name = "T66_Bake_Target"
    tex.image = image
    nodes.active = tex
    if bsdf:
        base = bsdf.inputs.get("Base Color")
        if base:
            mat.node_tree.links.new(tex.outputs["Color"], base)
        roughness = bsdf.inputs.get("Roughness")
        if roughness:
            roughness.default_value = 1.0
        metallic = bsdf.inputs.get("Metallic")
        if metallic:
            metallic.default_value = 0.0
    obj.data.materials.clear()
    obj.data.materials.append(mat)
    for poly in obj.data.polygons:
        poly.material_index = 0
    return mat


def prepare_source_materials_for_color_bake(obj: bpy.types.Object) -> None:
    """Make TRELLIS source materials bake as flat painted color, not metal shading."""
    for slot in obj.material_slots:
        mat = slot.material
        if mat is None:
            continue
        mat.blend_method = "OPAQUE"
        mat.diffuse_color = (1.0, 1.0, 1.0, 1.0)
        if not mat.use_nodes:
            continue

        nodes = mat.node_tree.nodes
        links = mat.node_tree.links
        bsdf = nodes.get("Principled BSDF")
        if bsdf is None:
            bsdf = next((node for node in nodes if node.type == "BSDF_PRINCIPLED"), None)
        if bsdf is None:
            continue

        for input_name, value in (("Metallic", 0.0), ("Roughness", 1.0), ("Alpha", 1.0)):
            socket = bsdf.inputs.get(input_name)
            if socket is None:
                continue
            for link in list(socket.links):
                links.remove(link)
            socket.default_value = value

        base = bsdf.inputs.get("Base Color")
        if base is None or base.links:
            continue

        texture_nodes = [node for node in nodes if node.type == "TEX_IMAGE" and node.image is not None]
        if texture_nodes:
            links.new(texture_nodes[0].outputs["Color"], base)


def configure_bake_scene(args: argparse.Namespace) -> None:
    scene = bpy.context.scene
    scene.render.engine = "CYCLES"
    scene.cycles.samples = 64
    scene.cycles.use_denoising = False
    scene.render.bake.margin = max(0, args.bake_margin)
    scene.render.bake.use_clear = True


def bake_diffuse(source: bpy.types.Object, target: bpy.types.Object, args: argparse.Namespace) -> bpy.types.Image:
    configure_bake_scene(args)
    image = bpy.data.images.new(
        f"{target.name}_Bake_{args.bake_size}",
        width=max(32, args.bake_size),
        height=max(32, args.bake_size),
        alpha=True,
    )
    create_bake_target_material(target, image)
    bpy.ops.object.select_all(action="DESELECT")
    source.hide_render = False
    source.hide_viewport = False
    target.hide_render = False
    target.hide_viewport = False
    source.select_set(True)
    target.select_set(True)
    bpy.context.view_layer.objects.active = target
    bpy.ops.object.bake(
        type="DIFFUSE",
        pass_filter={"COLOR"},
        use_selected_to_active=True,
        cage_extrusion=args.cage_extrusion,
        max_ray_distance=args.max_ray_distance,
    )
    return image


def save_image(image: bpy.types.Image, path: Path) -> None:
    ensure_parent(path)
    image.filepath_raw = str(path)
    image.file_format = "PNG"
    image.save()


def dilate_transparent_pixels(image: bpy.types.Image, iterations: int) -> None:
    if iterations <= 0:
        return
    image.update()
    width, height = [int(v) for v in image.size]
    pixels = list(image.pixels[:])

    def empty(buffer: list[float], index: int) -> bool:
        return buffer[index + 3] <= 0.01

    for _ in range(iterations):
        changed = 0
        next_pixels = pixels[:]
        for y in range(height):
            for x in range(width):
                index = (y * width + x) * 4
                if not empty(pixels, index):
                    continue
                samples = []
                for oy in (-1, 0, 1):
                    ny = y + oy
                    if ny < 0 or ny >= height:
                        continue
                    for ox in (-1, 0, 1):
                        nx = x + ox
                        if ox == 0 and oy == 0:
                            continue
                        if nx < 0 or nx >= width:
                            continue
                        nindex = (ny * width + nx) * 4
                        if not empty(pixels, nindex):
                            samples.append(pixels[nindex : nindex + 4])
                if not samples:
                    continue
                count = float(len(samples))
                next_pixels[index] = sum(sample[0] for sample in samples) / count
                next_pixels[index + 1] = sum(sample[1] for sample in samples) / count
                next_pixels[index + 2] = sum(sample[2] for sample in samples) / count
                next_pixels[index + 3] = 1.0
                changed += 1
        pixels = next_pixels
        if changed == 0:
            break
    image.pixels.foreach_set(pixels)
    image.update()


def clamp01(value: float) -> float:
    return max(0.0, min(1.0, value))


def image_pixels(image: bpy.types.Image) -> tuple[int, int, list[float]]:
    image.update()
    width, height = [int(v) for v in image.size]
    return width, height, list(image.pixels[:])


def sample_pixel(pixels: list[float], width: int, height: int, x: int, y: int) -> tuple[float, float, float, float]:
    x = max(0, min(width - 1, x))
    y = max(0, min(height - 1, y))
    index = (y * width + x) * 4
    return pixels[index], pixels[index + 1], pixels[index + 2], pixels[index + 3]


def collect_palette_samples(
    pixels: list[float],
    width: int,
    height: int,
    limit: int,
) -> list[tuple[float, float, float]]:
    stride = max(1, int(math.sqrt(max(1, width * height // max(1, limit)))))
    samples: list[tuple[float, float, float]] = []
    for y in range(0, height, stride):
        for x in range(0, width, stride):
            r, g, b, a = sample_pixel(pixels, width, height, x, y)
            if a > 0.05:
                samples.append((r, g, b))
            if len(samples) >= limit:
                return samples
    return samples


def nearest_color_index(color: tuple[float, float, float], palette: list[tuple[float, float, float]]) -> int:
    best_index = 0
    best_dist = math.inf
    r, g, b = color
    for index, (pr, pg, pb) in enumerate(palette):
        dist = (r - pr) * (r - pr) + (g - pg) * (g - pg) + (b - pb) * (b - pb)
        if dist < best_dist:
            best_dist = dist
            best_index = index
    return best_index


def kmeans_palette(
    pixels: list[float],
    width: int,
    height: int,
    palette_size: int,
    sample_limit: int,
    iterations: int,
) -> list[tuple[float, float, float]]:
    samples = collect_palette_samples(pixels, width, height, sample_limit)
    if not samples:
        return [(1.0, 1.0, 1.0)]
    wanted = max(2, min(palette_size, len(samples)))
    if len(samples) <= wanted:
        return samples[:wanted]

    step = max(1, len(samples) // wanted)
    centers = [samples[min(len(samples) - 1, i * step)] for i in range(wanted)]
    for _ in range(max(1, iterations)):
        buckets = [[0.0, 0.0, 0.0, 0] for _ in centers]
        for color in samples:
            idx = nearest_color_index(color, centers)
            buckets[idx][0] += color[0]
            buckets[idx][1] += color[1]
            buckets[idx][2] += color[2]
            buckets[idx][3] += 1
        next_centers = []
        for index, bucket in enumerate(buckets):
            count = bucket[3]
            if count == 0:
                next_centers.append(centers[index])
            else:
                next_centers.append((bucket[0] / count, bucket[1] / count, bucket[2] / count))
        centers = next_centers
    return centers


def per_channel_quantize(value: float, steps: int, dither: float) -> float:
    levels = max(2, steps)
    return round(clamp01(value + dither) * (levels - 1)) / float(levels - 1)


def dither_offset(args: argparse.Namespace, x: int, y: int) -> float:
    if args.dither_type == "none" or args.dither_strength <= 0:
        return 0.0
    if args.dither_type == "bayer8":
        threshold = (BAYER_8X8[y % 8][x % 8] / 63.0) - 0.5
    else:
        threshold = (BAYER_4X4[y % 4][x % 4] / 15.0) - 0.5
    return threshold * args.dither_strength / 8.0


def make_pixelated_image(image: bpy.types.Image, args: argparse.Namespace, name: str) -> tuple[bpy.types.Image, dict]:
    src_w, src_h, src_pixels = image_pixels(image)
    dst_w = max(16, min(args.texture_size, src_w))
    dst_h = max(16, min(args.texture_size, src_h))
    dst_pixels: list[float] = [0.0] * (dst_w * dst_h * 4)

    palette: list[tuple[float, float, float]] = []
    if args.palette_mode == "kmeans":
        palette = kmeans_palette(
            src_pixels,
            src_w,
            src_h,
            max(2, args.palette_size),
            max(64, args.kmeans_sample_limit),
            max(1, args.kmeans_iterations),
        )

    for y in range(dst_h):
        for x in range(dst_w):
            sx = min(src_w - 1, int((x + 0.5) * src_w / dst_w))
            sy = min(src_h - 1, int((y + 0.5) * src_h / dst_h))
            r, g, b, a = sample_pixel(src_pixels, src_w, src_h, sx, sy)
            d = dither_offset(args, x, y)
            if args.palette_mode == "none":
                out_r, out_g, out_b = r, g, b
            elif args.palette_mode == "kmeans":
                idx = nearest_color_index((clamp01(r + d), clamp01(g + d), clamp01(b + d)), palette)
                out_r, out_g, out_b = palette[idx]
            else:
                out_r = per_channel_quantize(r, args.palette_steps, d)
                out_g = per_channel_quantize(g, args.palette_steps, d)
                out_b = per_channel_quantize(b, args.palette_steps, d)
            index = (y * dst_w + x) * 4
            dst_pixels[index : index + 4] = [out_r, out_g, out_b, a]

    out = bpy.data.images.new(name, width=dst_w, height=dst_h, alpha=True)
    out.pixels.foreach_set(dst_pixels)
    out.update()
    try:
        out.pack()
    except Exception:
        pass
    return out, {
        "source_size": [src_w, src_h],
        "texture_size": [dst_w, dst_h],
        "palette": [[round(c, 5) for c in color] for color in palette],
        "palette_count": len(palette) if palette else 0,
    }


def assign_pixel_material(obj: bpy.types.Object, image: bpy.types.Image, unlit: bool) -> bpy.types.Material:
    mat = bpy.data.materials.new(f"{obj.name}_Pixelated_Unlit")
    mat.use_nodes = True
    nodes = mat.node_tree.nodes
    for node in list(nodes):
        nodes.remove(node)
    output = nodes.new(type="ShaderNodeOutputMaterial")
    tex = nodes.new(type="ShaderNodeTexImage")
    tex.image = image
    tex.interpolation = "Closest"
    if unlit:
        shader = nodes.new(type="ShaderNodeEmission")
        mat.node_tree.links.new(tex.outputs["Color"], shader.inputs["Color"])
        shader.inputs["Strength"].default_value = 1.0
        mat.node_tree.links.new(shader.outputs["Emission"], output.inputs["Surface"])
    else:
        shader = nodes.new(type="ShaderNodeBsdfPrincipled")
        mat.node_tree.links.new(tex.outputs["Color"], shader.inputs["Base Color"])
        mat.node_tree.links.new(shader.outputs["BSDF"], output.inputs["Surface"])
    mat.blend_method = "CLIP"
    obj.data.materials.clear()
    obj.data.materials.append(mat)
    for poly in obj.data.polygons:
        poly.material_index = 0
    return mat


def export_glb(path: Path, obj: bpy.types.Object) -> None:
    ensure_parent(path)
    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    bpy.ops.export_scene.gltf(
        filepath=str(path),
        export_format="GLB",
        use_selection=True,
        export_skins=False,
        export_animations=False,
        export_apply=True,
    )


def setup_qa_scene(resolution: int) -> None:
    scene = bpy.context.scene
    available_engines = {item.identifier for item in scene.render.bl_rna.properties["engine"].enum_items}
    scene.render.engine = "BLENDER_EEVEE_NEXT" if "BLENDER_EEVEE_NEXT" in available_engines else "BLENDER_EEVEE"
    scene.render.resolution_x = resolution
    scene.render.resolution_y = resolution
    scene.render.image_settings.file_format = "PNG"
    scene.world = bpy.data.worlds.new("T66_QA_World")
    scene.world.use_nodes = True
    bg = scene.world.node_tree.nodes["Background"]
    bg.inputs[0].default_value = (0.88, 0.89, 0.91, 1.0)
    bg.inputs[1].default_value = 0.8


def add_qa_camera(center: Vector, size: Vector, yaw_deg: float, pitch_deg: float = 5.0) -> bpy.types.Object:
    cam_data = bpy.data.cameras.new(name=f"QA_Camera_{int(yaw_deg)}")
    cam_data.type = "ORTHO"
    cam = bpy.data.objects.new(cam_data.name, cam_data)
    bpy.context.scene.collection.objects.link(cam)
    bpy.context.scene.camera = cam

    yaw = math.radians(yaw_deg)
    pitch = math.radians(pitch_deg)
    direction = Vector((math.sin(yaw) * math.cos(pitch), -math.cos(yaw) * math.cos(pitch), math.sin(pitch)))
    distance = max(size.x, size.y, size.z) * 4.0 + 2.0
    cam.location = center + direction * distance

    target = bpy.data.objects.new(f"QA_Target_{int(yaw_deg)}", None)
    target.location = center
    bpy.context.scene.collection.objects.link(target)
    track = cam.constraints.new(type="TRACK_TO")
    track.target = target
    track.track_axis = "TRACK_NEGATIVE_Z"
    track.up_axis = "UP_Y"
    cam_data.ortho_scale = max(size.x, size.z) * 1.35 + 0.35
    return cam


def render_qa(obj: bpy.types.Object, render_dir: Path, base_name: str, resolution: int) -> list[str]:
    setup_qa_scene(resolution)
    mins, maxs = world_bbox([obj])
    center = (mins + maxs) * 0.5
    size = maxs - mins
    outputs = []
    for label, yaw in (("front", 0.0), ("right", 90.0), ("back", 180.0), ("oblique", 45.0)):
        cam = add_qa_camera(center, size, yaw)
        bpy.context.scene.camera = cam
        path = render_dir / f"{base_name}_{label}.png"
        ensure_parent(path)
        bpy.context.scene.render.filepath = str(path)
        bpy.ops.render.render(write_still=True)
        outputs.append(str(path))
    return outputs


def save_report(path: Path, payload: dict) -> None:
    ensure_parent(path)
    with open(path, "w", encoding="ascii") as handle:
        json.dump(payload, handle, indent=2)


def main() -> None:
    args = parse_args()
    output_root = Path(args.output_dir)
    model_dir = output_root / "Models"
    texture_dir = output_root / "Textures"
    render_dir = output_root / "Renders"
    report_dir = output_root / "Reports"
    work_dir = output_root / "Working" / "QuadRemesher"
    for directory in (model_dir, texture_dir, render_dir, report_dir, work_dir):
        directory.mkdir(parents=True, exist_ok=True)

    input_path = Path(args.input)
    base_name = args.label or input_path.stem
    output_base = f"{base_name}{args.suffix}"

    clear_scene()
    imported = import_model(str(input_path))
    source = join_meshes(mesh_objects(imported), f"{base_name}_HighSource")
    raw_triangles = triangle_count([source])
    raw_quads = quad_count([source])
    normalize_report = normalize_to_height(source, args.normalize_height)
    clean_mesh(source, args.merge_distance, args.delete_loose, args.recalculate_normals)
    prepare_source_materials_for_color_bake(source)
    source_triangles_after_cleanup = triangle_count([source])
    qremesh_source, qremesh_source_report = make_qremesh_source(
        source,
        args.qremesh_source_target_tris,
        f"{base_name}_QRemeshSource",
    )

    if args.retopo_fbx:
        qremesh_fbx = Path(args.retopo_fbx)
        if not qremesh_fbx.exists():
            raise RuntimeError(f"Retopo FBX not found: {qremesh_fbx}")
        qremesh_report = {
            "reused_retopo_fbx": str(qremesh_fbx),
            "elapsed_seconds": 0.0,
            "last_progress": 2.0,
            "last_text": "reused existing Quad Remesher output",
        }
    else:
        qremesh_fbx, qremesh_report = run_quad_remesher(qremesh_source, args, work_dir)
    if qremesh_source is not source:
        qremesh_source.hide_render = True
        qremesh_source.hide_viewport = True
    retopo = import_retopo(qremesh_fbx, f"{base_name}_QuadRemesh")
    clear_qremesher_fbx_axis_rotation(retopo)
    retopo_triangles_before_unwrap = triangle_count([retopo])
    retopo_quads_before_unwrap = quad_count([retopo])
    unwrap_smart_project(retopo)
    if args.shade_flat:
        force_flat_shading(retopo)

    bake_image = bake_diffuse(source, retopo, args)
    dilate_transparent_pixels(bake_image, args.dilate_pixels)
    bake_path = texture_dir / f"{output_base}_Bake{args.bake_size}.png"
    save_image(bake_image, bake_path)

    pixel_image, pixel_report = make_pixelated_image(bake_image, args, f"{output_base}_Pixelated")
    pixel_path = texture_dir / f"{output_base}_Pixelated_{args.texture_size}.png"
    save_image(pixel_image, pixel_path)
    assign_pixel_material(retopo, pixel_image, args.unlit_emission_material)

    source.hide_render = True
    source.hide_viewport = True

    output_glb = model_dir / f"{output_base}.glb"
    export_glb(output_glb, retopo)

    render_outputs: list[str] = []
    if args.render_qa:
        render_outputs = render_qa(retopo, render_dir, output_base, args.qa_resolution)

    blend_path = ""
    if args.save_blend:
        blend_file = output_root / "Blender" / f"{output_base}.blend"
        ensure_parent(blend_file)
        bpy.ops.wm.save_as_mainfile(filepath=str(blend_file))
        blend_path = str(blend_file)

    payload = {
        "input": str(input_path),
        "output_glb": str(output_glb),
        "output_blend": blend_path,
        "baked_texture": str(bake_path),
        "pixelated_texture": str(pixel_path),
        "qa_renders": render_outputs,
        "raw_triangles": raw_triangles,
        "raw_quads": raw_quads,
        "source_triangles_after_cleanup": source_triangles_after_cleanup,
        "retopo_triangles": retopo_triangles_before_unwrap,
        "retopo_quads": retopo_quads_before_unwrap,
        "adjustable_values": {
            "target_quads": args.target_quads,
            "adaptive_size": args.adaptive_size,
            "adapt_quad_count": args.adapt_quad_count,
            "use_materials": args.use_materials,
            "use_normals": args.use_normals,
            "autodetect_hard_edges": args.autodetect_hard_edges,
            "symmetry_x": args.symmetry_x,
            "symmetry_y": args.symmetry_y,
            "symmetry_z": args.symmetry_z,
            "normalize_height": args.normalize_height,
            "merge_distance": args.merge_distance,
            "delete_loose": args.delete_loose,
            "recalculate_normals": args.recalculate_normals,
            "shade_flat": args.shade_flat,
            "qremesh_source_target_tris": args.qremesh_source_target_tris,
            "bake_size": args.bake_size,
            "bake_margin": args.bake_margin,
            "cage_extrusion": args.cage_extrusion,
            "max_ray_distance": args.max_ray_distance,
            "dilate_pixels": args.dilate_pixels,
            "texture_size": args.texture_size,
            "palette_mode": args.palette_mode,
            "palette_size": args.palette_size,
            "palette_steps": args.palette_steps,
            "kmeans_iterations": args.kmeans_iterations,
            "kmeans_sample_limit": args.kmeans_sample_limit,
            "dither_type": args.dither_type,
            "dither_strength": args.dither_strength,
            "unlit_emission_material": args.unlit_emission_material,
        },
        "normalize_report": normalize_report,
        "qremesh_source_report": qremesh_source_report,
        "qremesh_report": qremesh_report,
        "pixel_report": pixel_report,
    }
    report_path = report_dir / f"{output_base}_report.json"
    save_report(report_path, payload)
    print(json.dumps({"output_glb": str(output_glb), "report": str(report_path)}, indent=2), flush=True)
    if args.quit_when_done:
        sys.stdout.flush()
        sys.stderr.flush()
        os._exit(0)


if __name__ == "__main__":
    try:
        main()
    except Exception:
        traceback.print_exc()
        sys.stdout.flush()
        sys.stderr.flush()
        os._exit(1)
