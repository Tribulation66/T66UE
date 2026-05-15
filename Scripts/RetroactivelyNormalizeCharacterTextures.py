"""
Create non-destructive normalized copies of QuadRetro character textures.

Run in-editor:
  UnrealEditor.exe T66.uproject -ExecutePythonScript="C:/UE/T66/Scripts/RetroactivelyNormalizeCharacterTextures.py" -unattended -nop4 -nosplash

The script updates Content/Data/CharacterVisuals.csv to point at the new
*_Normalized Texture2D assets and reloads /Game/Data/DT_CharacterVisuals.
Original textures and mesh material assignments are not modified.
"""

import csv
import json
import os
import shutil
import statistics
import subprocess
import sys
import tempfile
import textwrap
from pathlib import Path

import unreal


SCRIPT_DIR = Path(__file__).resolve().parent
PROJECT_ROOT = Path(unreal.SystemLibrary.get_project_directory()).resolve()
CSV_PATH = PROJECT_ROOT / "Content" / "Data" / "CharacterVisuals.csv"
AUDIT_ROOT = PROJECT_ROOT / "Audit" / "Reference" / "Track1_Normalization"
COMPARISON_DIR = AUDIT_ROOT / "Comparisons"
TEMP_ROOT = PROJECT_ROOT / "Saved" / "Track1_Normalization_Work"
REPORT_JSON = AUDIT_ROOT / "retroactive_normalization_results.json"
REPORT_MD = AUDIT_ROOT / "Report.md"
DT_CHARACTER_VISUALS = "/Game/Data/DT_CharacterVisuals"
CHARACTER_ROOT = "/Game/Characters"
TEXTURE_NAME_MARKER = "_QuadRetro_Pixelated_"
NORMALIZED_SUFFIX = "_Normalized"
TARGET_LUMINANCE = 0.50
MAX_SCALING_FACTOR = 4.0
SATURATION_BOOST = 1.0
BLENDER_EXE = os.environ.get("T66_BLENDER_EXE", r"C:\Program Files\Blender Foundation\Blender 5.1\blender.exe")
SYSTEM_PYTHON = os.environ.get("T66_SYSTEM_PYTHON", "python")
VERIFY_ROWS = {
    "MushroomBrute": "Mushroom Brute",
    "Hell_GreatDragon": "Hell Great Dragon",
    "Slime": "Slime",
}
LOG_PREFIX = "[RetroNormalizeCharacterTextures]"

if str(SCRIPT_DIR) not in sys.path:
    sys.path.append(str(SCRIPT_DIR))

import QuadRetroCharacterPipelineDefaults as CharacterDefaults


def log(message):
    unreal.log(f"{LOG_PREFIX} {message}")


def warn(message):
    unreal.log_warning(f"{LOG_PREFIX} {message}")


def error(message):
    unreal.log_error(f"{LOG_PREFIX} {message}")


def quit_editor():
    world = None
    try:
        subsystem = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
        if subsystem:
            world = subsystem.get_editor_world()
    except Exception:
        pass
    try:
        unreal.SystemLibrary.execute_console_command(world, "QUIT_EDITOR")
    except Exception as exc:
        warn(f"Failed to request QUIT_EDITOR: {exc}")


def package_path(object_path):
    text = (object_path or "").strip()
    return text.split(".", 1)[0]


def asset_name(object_path):
    pkg = package_path(object_path)
    return pkg.rsplit("/", 1)[-1]


def object_path_for_package(pkg):
    name = pkg.rsplit("/", 1)[-1]
    return f"{pkg}.{name}"


def ensure_dirs():
    for path in (AUDIT_ROOT, COMPARISON_DIR, TEMP_ROOT):
        path.mkdir(parents=True, exist_ok=True)


def read_visual_rows():
    with CSV_PATH.open("r", encoding="utf-8-sig", newline="") as handle:
        reader = csv.DictReader(handle)
        fieldnames = list(reader.fieldnames or [])
        rows = list(reader)
    if "---" not in fieldnames:
        raise RuntimeError("CharacterVisuals.csv is missing row-name column '---'")
    return fieldnames, rows


def write_visual_rows(fieldnames, rows):
    with CSV_PATH.open("w", encoding="utf-8", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=fieldnames, quoting=csv.QUOTE_ALL, lineterminator="\n")
        writer.writeheader()
        writer.writerows(rows)


def selected_rows(rows):
    out = []
    for row in rows:
        texture_path = (row.get("PixelatedTextureAssetPath") or "").strip()
        mesh_path = (row.get("StaticMesh") or "").strip()
        if TEXTURE_NAME_MARKER not in texture_path:
            continue
        if NORMALIZED_SUFFIX in asset_name(texture_path):
            continue
        if not texture_path.startswith(CHARACTER_ROOT):
            continue
        if not mesh_path:
            raise RuntimeError(f"{row.get('---')}: QuadRetro texture row has no StaticMesh")
        out.append(row)
    return out


def export_texture(texture, output_png):
    task = unreal.AssetExportTask()
    task.object = texture
    task.filename = str(output_png)
    task.automated = True
    task.prompt = False
    task.replace_identical = True
    task.exporter = unreal.TextureExporterPNG()
    ok = bool(unreal.Exporter.run_asset_export_task(task))
    if not ok or not output_png.exists():
        raise RuntimeError(f"failed to export texture to {output_png}")


def export_mesh(mesh, output_path):
    mesh_name = mesh.get_name()
    prefer_fbx = mesh_name == "SM_Gambler_QuadRetro"

    if not prefer_fbx and hasattr(unreal, "GLTFExporter") and hasattr(unreal, "GLTFExportOptions"):
        try:
            options = unreal.GLTFExportOptions()
            for prop_name, value in (
                ("export_uniform_scale", 0.01),
                ("export_preview_mesh", True),
                ("export_unlit_materials", True),
            ):
                try:
                    options.set_editor_property(prop_name, value)
                except Exception:
                    pass
            ok = bool(unreal.GLTFExporter.export_to_gltf(mesh, str(output_path), options, set()))
            if ok and output_path.exists():
                return output_path
        except Exception as exc:
            warn(f"GLTF mesh export failed for {mesh.get_path_name()}: {exc}; trying FBX")

    fbx_path = output_path.with_suffix(".fbx")
    task = unreal.AssetExportTask()
    task.object = mesh
    task.filename = str(fbx_path)
    task.automated = True
    task.prompt = False
    task.replace_identical = True
    task.exporter = unreal.StaticMeshExporterFBX()
    try:
        task.options = unreal.FbxExportOption()
    except Exception:
        pass
    ok = bool(unreal.Exporter.run_asset_export_task(task))
    if ok and fbx_path.exists():
        return fbx_path
    if prefer_fbx:
        warn(f"FBX mesh export failed for {mesh.get_path_name()}; trying GLTF")

    if prefer_fbx and hasattr(unreal, "GLTFExporter") and hasattr(unreal, "GLTFExportOptions"):
        try:
            options = unreal.GLTFExportOptions()
            for prop_name, value in (
                ("export_uniform_scale", 0.01),
                ("export_preview_mesh", True),
                ("export_unlit_materials", True),
            ):
                try:
                    options.set_editor_property(prop_name, value)
                except Exception:
                    pass
            ok = bool(unreal.GLTFExporter.export_to_gltf(mesh, str(output_path), options, set()))
            if ok and output_path.exists():
                return output_path
        except Exception as exc:
            warn(f"GLTF mesh export failed for {mesh.get_path_name()}: {exc}")

    raise RuntimeError(f"failed to export mesh to {fbx_path} or {output_path}")


def import_texture(source_png, dest_pkg):
    dest_dir = dest_pkg.rsplit("/", 1)[0]
    dest_name = dest_pkg.rsplit("/", 1)[-1]
    if not unreal.EditorAssetLibrary.does_directory_exist(dest_dir):
        unreal.EditorAssetLibrary.make_directory(dest_dir)

    task = unreal.AssetImportTask()
    task.automated = True
    task.save = False
    task.replace_existing = True
    task.replace_existing_settings = True
    task.filename = str(source_png)
    task.destination_path = dest_dir
    task.destination_name = dest_name
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

    imported_paths = [package_path(path) for path in list(task.imported_object_paths or []) if path]
    actual_pkg = imported_paths[0] if imported_paths else dest_pkg
    if actual_pkg != dest_pkg and unreal.EditorAssetLibrary.does_asset_exist(actual_pkg):
        if unreal.EditorAssetLibrary.does_asset_exist(dest_pkg):
            unreal.EditorAssetLibrary.delete_asset(dest_pkg)
        if unreal.EditorAssetLibrary.rename_asset(actual_pkg, dest_pkg):
            actual_pkg = dest_pkg
        else:
            warn(f"Could not rename imported texture {actual_pkg} to requested {dest_pkg}; using imported path")

    texture = unreal.EditorAssetLibrary.load_asset(actual_pkg)
    if not texture or not isinstance(texture, unreal.Texture2D):
        raise RuntimeError(f"normalized Texture2D import failed: requested={dest_pkg} imported={imported_paths}")

    defaults = CharacterDefaults.apply_character_texture_defaults(texture)
    CharacterDefaults.safe_save(texture, actual_pkg)
    return texture, defaults, actual_pkg


def reload_character_visuals_data_table():
    dt = unreal.EditorAssetLibrary.load_asset(DT_CHARACTER_VISUALS)
    if not dt:
        raise RuntimeError(f"missing {DT_CHARACTER_VISUALS}")
    ok = bool(unreal.DataTableFunctionLibrary.fill_data_table_from_csv_file(dt, str(CSV_PATH)))
    if not ok:
        raise RuntimeError("failed to reload DT_CharacterVisuals from CharacterVisuals.csv")
    unreal.EditorAssetLibrary.save_asset(DT_CHARACTER_VISUALS)


def run_subprocess(args, cwd=None):
    completed = subprocess.run(
        args,
        cwd=str(cwd or PROJECT_ROOT),
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        check=False,
    )
    if completed.returncode != 0:
        raise RuntimeError(f"command failed ({completed.returncode}): {' '.join(map(str, args))}\n{completed.stdout}")
    return completed.stdout


def write_helper_scripts(helper_dir):
    pipeline_path = PROJECT_ROOT / "Model Generation" / "Scripts" / "Core" / "QuadRetro" / "t66_quad_retro_character_pipeline.py"
    uv_script = helper_dir / "extract_uv0_triangles_blender.py"
    normalize_script = helper_dir / "normalize_texture_worker.py"
    render_script = helper_dir / "render_mesh_texture_blender.py"
    compose_script = helper_dir / "compose_verification_worker.py"

    uv_script.write_text(
        textwrap.dedent(
            """
            import json
            import os
            import sys
            import bpy

            mesh_path = sys.argv[sys.argv.index("--") + 1]
            out_json = sys.argv[sys.argv.index("--") + 2]

            bpy.ops.object.select_all(action="SELECT")
            bpy.ops.object.delete()
            lower = mesh_path.lower()
            if lower.endswith((".glb", ".gltf")):
                bpy.ops.import_scene.gltf(filepath=mesh_path)
            elif lower.endswith(".fbx"):
                bpy.ops.import_scene.fbx(filepath=mesh_path, use_image_search=False)
            else:
                raise RuntimeError("unsupported mesh export: " + mesh_path)

            triangles = []
            mesh_count = 0
            for obj in bpy.context.scene.objects:
                if obj.type != "MESH" or not obj.data.uv_layers:
                    continue
                mesh_count += 1
                mesh = obj.data
                mesh.calc_loop_triangles()
                uv_layer = mesh.uv_layers.active
                if uv_layer is None:
                    continue
                for tri in mesh.loop_triangles:
                    uv_tri = []
                    for loop_index in tri.loops:
                        uv = uv_layer.data[loop_index].uv
                        uv_tri.append([float(uv.x), float(uv.y)])
                    if len(uv_tri) == 3:
                        triangles.append(uv_tri)

            os.makedirs(os.path.dirname(out_json), exist_ok=True)
            with open(out_json, "w", encoding="utf-8") as handle:
                json.dump({"mesh_path": mesh_path, "mesh_count": mesh_count, "triangle_count": len(triangles), "uv0_triangles": triangles}, handle)
            """
        ).strip()
        + "\n",
        encoding="utf-8",
    )

    normalize_script.write_text(
        textwrap.dedent(
            f"""
            import importlib.util
            import json
            import sys
            from pathlib import Path
            from PIL import Image, ImageDraw

            pipeline_path = Path({str(pipeline_path)!r})
            spec = importlib.util.spec_from_file_location("quadretro_pipeline", pipeline_path)
            module = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(module)

            config = json.loads(Path(sys.argv[1]).read_text(encoding="utf-8"))
            source_png = Path(config["source_png"])
            normalized_png = Path(config["normalized_png"])
            comparison_png = Path(config["comparison_png"])
            uv_json = Path(config["uv_json"])
            uv_payload = json.loads(uv_json.read_text(encoding="utf-8"))
            source = Image.open(source_png).convert("RGBA")
            normalized, report = module.normalize_texture_luminance(
                source,
                uv_payload.get("uv0_triangles", []),
                float(config["target_luminance"]),
                float(config["max_scaling_factor"]),
                float(config["saturation_boost"]),
            )
            normalized_png.parent.mkdir(parents=True, exist_ok=True)
            normalized.save(normalized_png)

            gap = 16
            label_h = 28
            w = source.width + normalized.width + gap
            h = max(source.height, normalized.height) + label_h
            sheet = Image.new("RGBA", (w, h), (245, 245, 245, 255))
            draw = ImageDraw.Draw(sheet)
            draw.text((8, 6), "original", fill=(0, 0, 0, 255))
            draw.text((source.width + gap + 8, 6), "normalized", fill=(0, 0, 0, 255))
            sheet.alpha_composite(source, (0, label_h))
            sheet.alpha_composite(normalized, (source.width + gap, label_h))
            comparison_png.parent.mkdir(parents=True, exist_ok=True)
            sheet.save(comparison_png)

            report["uv_triangle_count"] = int(uv_payload.get("triangle_count", 0))
            report["uv_mesh_count"] = int(uv_payload.get("mesh_count", 0))
            Path(config["report_json"]).write_text(json.dumps(report, indent=2), encoding="utf-8")
            """
        ).strip()
        + "\n",
        encoding="utf-8",
    )

    render_script.write_text(
        textwrap.dedent(
            """
            import math
            import os
            import sys
            import bpy
            from mathutils import Vector

            mesh_path = sys.argv[sys.argv.index("--") + 1]
            texture_path = sys.argv[sys.argv.index("--") + 2]
            output_png = sys.argv[sys.argv.index("--") + 3]

            bpy.ops.object.select_all(action="SELECT")
            bpy.ops.object.delete()
            lower = mesh_path.lower()
            if lower.endswith((".glb", ".gltf")):
                bpy.ops.import_scene.gltf(filepath=mesh_path)
            elif lower.endswith(".fbx"):
                bpy.ops.import_scene.fbx(filepath=mesh_path, use_image_search=False)
            else:
                raise RuntimeError("unsupported mesh export: " + mesh_path)

            meshes = [obj for obj in bpy.context.scene.objects if obj.type == "MESH"]
            if not meshes:
                raise RuntimeError("no meshes imported")

            mat = bpy.data.materials.new("T66_Verification_Texture")
            mat.use_nodes = True
            nodes = mat.node_tree.nodes
            for node in list(nodes):
                nodes.remove(node)
            output = nodes.new(type="ShaderNodeOutputMaterial")
            tex = nodes.new(type="ShaderNodeTexImage")
            tex.image = bpy.data.images.load(texture_path)
            tex.interpolation = "Closest"
            shader = nodes.new(type="ShaderNodeEmission")
            shader.inputs["Strength"].default_value = 1.0
            mat.node_tree.links.new(tex.outputs["Color"], shader.inputs["Color"])
            mat.node_tree.links.new(shader.outputs["Emission"], output.inputs["Surface"])
            for obj in meshes:
                obj.data.materials.clear()
                obj.data.materials.append(mat)

            bpy.context.view_layer.update()
            mins = Vector((math.inf, math.inf, math.inf))
            maxs = Vector((-math.inf, -math.inf, -math.inf))
            for obj in meshes:
                for corner in obj.bound_box:
                    world = obj.matrix_world @ Vector(corner)
                    mins.x = min(mins.x, world.x); mins.y = min(mins.y, world.y); mins.z = min(mins.z, world.z)
                    maxs.x = max(maxs.x, world.x); maxs.y = max(maxs.y, world.y); maxs.z = max(maxs.z, world.z)
            center = (mins + maxs) * 0.5
            size = maxs - mins

            scene = bpy.context.scene
            engines = {item.identifier for item in scene.render.bl_rna.properties["engine"].enum_items}
            scene.render.engine = "BLENDER_EEVEE_NEXT" if "BLENDER_EEVEE_NEXT" in engines else "BLENDER_EEVEE"
            scene.render.resolution_x = 768
            scene.render.resolution_y = 768
            scene.render.image_settings.file_format = "PNG"
            scene.world = bpy.data.worlds.new("T66_Verification_World")
            scene.world.color = (0.82, 0.82, 0.82)

            cam_data = bpy.data.cameras.new("T66_Verification_Camera")
            cam_data.type = "ORTHO"
            cam_data.ortho_scale = max(size.x, size.z) * 1.25 if max(size.x, size.z) > 0 else 2.0
            cam = bpy.data.objects.new("T66_Verification_Camera", cam_data)
            scene.collection.objects.link(cam)
            cam.location = (center.x, center.y - max(size.x, size.y, size.z) * 3.0 - 2.0, center.z)
            cam.rotation_euler = (math.radians(90.0), 0.0, 0.0)
            scene.camera = cam

            light_data = bpy.data.lights.new("T66_Verification_Light", type="AREA")
            light = bpy.data.objects.new("T66_Verification_Light", light_data)
            scene.collection.objects.link(light)
            light.location = (center.x, center.y - 2.0, center.z + max(size.z, 1.0) * 2.0)
            light.data.energy = 300
            light.data.size = 4

            os.makedirs(os.path.dirname(output_png), exist_ok=True)
            scene.render.filepath = output_png
            bpy.ops.render.render(write_still=True)
            """
        ).strip()
        + "\n",
        encoding="utf-8",
    )

    compose_script.write_text(
        textwrap.dedent(
            """
            import json
            import sys
            from pathlib import Path
            from PIL import Image, ImageDraw

            config = json.loads(Path(sys.argv[1]).read_text(encoding="utf-8"))
            items = [
                ("original texture", Image.open(config["original_texture"]).convert("RGBA")),
                ("normalized texture", Image.open(config["normalized_texture"]).convert("RGBA")),
                ("mesh original", Image.open(config["original_render"]).convert("RGBA")),
                ("mesh normalized", Image.open(config["normalized_render"]).convert("RGBA")),
            ]
            cell_w = 512
            cell_h = 512
            label_h = 34
            sheet = Image.new("RGBA", (cell_w * 4, cell_h + label_h), (245, 245, 245, 255))
            draw = ImageDraw.Draw(sheet)
            for idx, (label, image) in enumerate(items):
                image.thumbnail((cell_w, cell_h), Image.Resampling.LANCZOS)
                x = idx * cell_w + (cell_w - image.width) // 2
                y = label_h + (cell_h - image.height) // 2
                draw.text((idx * cell_w + 10, 9), label, fill=(0, 0, 0, 255))
                sheet.alpha_composite(image, (x, y))
            output = Path(config["output"])
            output.parent.mkdir(parents=True, exist_ok=True)
            sheet.save(output)
            """
        ).strip()
        + "\n",
        encoding="utf-8",
    )

    return uv_script, normalize_script, render_script, compose_script


def normalize_one(row, helpers, rows_by_name):
    row_name = row["---"]
    texture_path = row["PixelatedTextureAssetPath"].strip()
    mesh_path = row["StaticMesh"].strip()
    texture_pkg = package_path(texture_path)
    mesh_pkg = package_path(mesh_path)
    texture_name = asset_name(texture_path)
    normalized_name = f"{texture_name}{NORMALIZED_SUFFIX}"
    normalized_pkg = texture_pkg.rsplit("/", 1)[0] + "/" + normalized_name
    normalized_object_path = object_path_for_package(normalized_pkg)

    texture = unreal.EditorAssetLibrary.load_asset(texture_pkg)
    if not texture or not isinstance(texture, unreal.Texture2D):
        raise RuntimeError(f"{row_name}: missing Texture2D {texture_path}")
    mesh = unreal.EditorAssetLibrary.load_asset(mesh_pkg)
    if not mesh or not isinstance(mesh, unreal.StaticMesh):
        raise RuntimeError(f"{row_name}: missing StaticMesh {mesh_path}")

    safe_name = "".join(c if c.isalnum() or c in "_-" else "_" for c in row_name)
    work_dir = TEMP_ROOT / safe_name
    work_dir.mkdir(parents=True, exist_ok=True)
    original_png = work_dir / f"{texture_name}.png"
    normalized_png = work_dir / f"{normalized_name}.png"
    mesh_export = work_dir / f"{asset_name(mesh_path)}.glb"
    uv_json = work_dir / "uv0_triangles.json"
    normalize_config = work_dir / "normalize_config.json"
    normalize_report_json = work_dir / "normalize_report.json"
    comparison_png = COMPARISON_DIR / f"{safe_name}.png"

    existing_normalized_texture = unreal.EditorAssetLibrary.load_asset(normalized_pkg)
    if (
        existing_normalized_texture
        and isinstance(existing_normalized_texture, unreal.Texture2D)
        and normalize_report_json.exists()
        and original_png.exists()
        and normalized_png.exists()
    ):
        defaults_report = CharacterDefaults.apply_character_texture_defaults(existing_normalized_texture)
        CharacterDefaults.safe_save(existing_normalized_texture, normalized_pkg)
        row["PixelatedTextureAssetPath"] = normalized_object_path
        report = json.loads(normalize_report_json.read_text(encoding="utf-8"))
        actual_mesh_export = mesh_export if mesh_export.exists() else mesh_export.with_suffix(".fbx")
        if row_name in VERIFY_ROWS and not (AUDIT_ROOT / f"Verification_{safe_name}.png").exists():
            if not actual_mesh_export.exists():
                actual_mesh_export = export_mesh(mesh, mesh_export)
            uv_script, normalize_script, render_script, compose_script = helpers
            original_render = work_dir / f"{safe_name}_mesh_original.png"
            normalized_render = work_dir / f"{safe_name}_mesh_normalized.png"
            verification_path = AUDIT_ROOT / f"Verification_{safe_name}.png"
            run_subprocess([BLENDER_EXE, "--background", "--python-exit-code", "1", "--python", str(render_script), "--", str(actual_mesh_export), str(original_png), str(original_render)])
            run_subprocess([BLENDER_EXE, "--background", "--python-exit-code", "1", "--python", str(render_script), "--", str(actual_mesh_export), str(normalized_png), str(normalized_render)])
            compose_config = work_dir / "compose_config.json"
            compose_config.write_text(
                json.dumps(
                    {
                        "original_texture": str(original_png),
                        "normalized_texture": str(normalized_png),
                        "original_render": str(original_render),
                        "normalized_render": str(normalized_render),
                        "output": str(verification_path),
                    },
                    indent=2,
                ),
                encoding="utf-8",
            )
            run_subprocess([SYSTEM_PYTHON, str(compose_script), str(compose_config)])
        verification_path = AUDIT_ROOT / f"Verification_{safe_name}.png" if row_name in VERIFY_ROWS else ""
        return {
            "enemy_name": row_name,
            "source_texture_asset_path": texture_path,
            "new_asset_path": normalized_object_path,
            "mesh_asset_path": mesh_path,
            "original_png": str(original_png),
            "normalized_png": str(normalized_png),
            "comparison_png": str(comparison_png),
            "verification_png": str(verification_path) if verification_path and verification_path.exists() else "",
            "before_luminance": float(report["before_luminance"]),
            "after_luminance": float(report["after_luminance"]),
            "scaling_factor": float(report["scaling_factor"]),
            "mapped_pixel_count": int(report.get("mapped_pixel_count", 0)),
            "uv_triangle_count": int(report.get("uv_triangle_count", 0)),
            "uv_mesh_count": int(report.get("mesh_count", report.get("uv_mesh_count", 0))),
            "mask_source": report.get("mask_source", ""),
            "defaults_report": defaults_report,
            "imported_texture": existing_normalized_texture.get_path_name(),
        }

    if normalize_report_json.exists() and original_png.exists() and normalized_png.exists():
        report = json.loads(normalize_report_json.read_text(encoding="utf-8"))
        imported_texture, defaults_report, actual_normalized_pkg = import_texture(normalized_png, normalized_pkg)
        normalized_object_path = object_path_for_package(actual_normalized_pkg)
        row["PixelatedTextureAssetPath"] = normalized_object_path
        actual_mesh_export = mesh_export if mesh_export.exists() else mesh_export.with_suffix(".fbx")

        verification_path = ""
        if row_name in VERIFY_ROWS:
            if not actual_mesh_export.exists():
                actual_mesh_export = export_mesh(mesh, mesh_export)
            uv_script, normalize_script, render_script, compose_script = helpers
            original_render = work_dir / f"{safe_name}_mesh_original.png"
            normalized_render = work_dir / f"{safe_name}_mesh_normalized.png"
            verification_path = AUDIT_ROOT / f"Verification_{safe_name}.png"
            run_subprocess([BLENDER_EXE, "--background", "--python-exit-code", "1", "--python", str(render_script), "--", str(actual_mesh_export), str(original_png), str(original_render)])
            run_subprocess([BLENDER_EXE, "--background", "--python-exit-code", "1", "--python", str(render_script), "--", str(actual_mesh_export), str(normalized_png), str(normalized_render)])
            compose_config = work_dir / "compose_config.json"
            compose_config.write_text(
                json.dumps(
                    {
                        "original_texture": str(original_png),
                        "normalized_texture": str(normalized_png),
                        "original_render": str(original_render),
                        "normalized_render": str(normalized_render),
                        "output": str(verification_path),
                    },
                    indent=2,
                ),
                encoding="utf-8",
            )
            run_subprocess([SYSTEM_PYTHON, str(compose_script), str(compose_config)])

        return {
            "enemy_name": row_name,
            "source_texture_asset_path": texture_path,
            "new_asset_path": normalized_object_path,
            "mesh_asset_path": mesh_path,
            "original_png": str(original_png),
            "normalized_png": str(normalized_png),
            "comparison_png": str(comparison_png),
            "verification_png": str(verification_path) if verification_path else "",
            "before_luminance": float(report["before_luminance"]),
            "after_luminance": float(report["after_luminance"]),
            "scaling_factor": float(report["scaling_factor"]),
            "mapped_pixel_count": int(report.get("mapped_pixel_count", 0)),
            "uv_triangle_count": int(report.get("uv_triangle_count", 0)),
            "uv_mesh_count": int(report.get("uv_mesh_count", 0)),
            "mask_source": report.get("mask_source", ""),
            "defaults_report": defaults_report,
            "imported_texture": imported_texture.get_path_name(),
        }

    export_texture(texture, original_png)
    actual_mesh_export = export_mesh(mesh, mesh_export)

    uv_script, normalize_script, render_script, compose_script = helpers
    run_subprocess([BLENDER_EXE, "--background", "--python-exit-code", "1", "--python", str(uv_script), "--", str(actual_mesh_export), str(uv_json)])
    if not uv_json.exists():
        raise RuntimeError(f"{row_name}: Blender UV extraction did not write {uv_json}")

    normalize_config.write_text(
        json.dumps(
            {
                "source_png": str(original_png),
                "normalized_png": str(normalized_png),
                "comparison_png": str(comparison_png),
                "uv_json": str(uv_json),
                "report_json": str(normalize_report_json),
                "target_luminance": TARGET_LUMINANCE,
                "max_scaling_factor": MAX_SCALING_FACTOR,
                "saturation_boost": SATURATION_BOOST,
            },
            indent=2,
        ),
        encoding="utf-8",
    )
    run_subprocess([SYSTEM_PYTHON, str(normalize_script), str(normalize_config)])
    report = json.loads(normalize_report_json.read_text(encoding="utf-8"))

    imported_texture, defaults_report, actual_normalized_pkg = import_texture(normalized_png, normalized_pkg)
    normalized_object_path = object_path_for_package(actual_normalized_pkg)
    row["PixelatedTextureAssetPath"] = normalized_object_path

    verification_path = ""
    if row_name in VERIFY_ROWS:
        original_render = work_dir / f"{safe_name}_mesh_original.png"
        normalized_render = work_dir / f"{safe_name}_mesh_normalized.png"
        verification_path = AUDIT_ROOT / f"Verification_{safe_name}.png"
        run_subprocess([BLENDER_EXE, "--background", "--python-exit-code", "1", "--python", str(render_script), "--", str(actual_mesh_export), str(original_png), str(original_render)])
        run_subprocess([BLENDER_EXE, "--background", "--python-exit-code", "1", "--python", str(render_script), "--", str(actual_mesh_export), str(normalized_png), str(normalized_render)])
        compose_config = work_dir / "compose_config.json"
        compose_config.write_text(
            json.dumps(
                {
                    "original_texture": str(original_png),
                    "normalized_texture": str(normalized_png),
                    "original_render": str(original_render),
                    "normalized_render": str(normalized_render),
                    "output": str(verification_path),
                },
                indent=2,
            ),
            encoding="utf-8",
        )
        run_subprocess([SYSTEM_PYTHON, str(compose_script), str(compose_config)])

    return {
        "enemy_name": row_name,
        "source_texture_asset_path": texture_path,
        "new_asset_path": normalized_object_path,
        "mesh_asset_path": mesh_path,
        "original_png": str(original_png),
        "normalized_png": str(normalized_png),
        "comparison_png": str(comparison_png),
        "verification_png": str(verification_path) if verification_path else "",
        "before_luminance": float(report["before_luminance"]),
        "after_luminance": float(report["after_luminance"]),
        "scaling_factor": float(report["scaling_factor"]),
        "mapped_pixel_count": int(report.get("mapped_pixel_count", 0)),
        "uv_triangle_count": int(report.get("uv_triangle_count", 0)),
        "uv_mesh_count": int(report.get("uv_mesh_count", 0)),
        "mask_source": report.get("mask_source", ""),
        "defaults_report": defaults_report,
        "imported_texture": imported_texture.get_path_name(),
    }


def write_task2_markdown(results, failures):
    before = [row["before_luminance"] for row in results]
    after = [row["after_luminance"] for row in results]

    def summary(values):
        if not values:
            return {"mean": 0, "min": 0, "max": 0, "stddev": 0}
        return {
            "mean": statistics.mean(values),
            "min": min(values),
            "max": max(values),
            "stddev": statistics.pstdev(values),
        }

    before_summary = summary(before)
    after_summary = summary(after)
    lines = [
        "# Track 1 Normalization Report",
        "",
        "## Task 2 - Retroactive Texture Normalization",
        "",
        f"Processed {len(results)} textures. Failures: {len(failures)}.",
        "",
        "| Metric | Before | After |",
        "|---|---:|---:|",
        f"| Mean luminance | {before_summary['mean']:.4f} | {after_summary['mean']:.4f} |",
        f"| Min luminance | {before_summary['min']:.4f} | {after_summary['min']:.4f} |",
        f"| Max luminance | {before_summary['max']:.4f} | {after_summary['max']:.4f} |",
        f"| Stddev luminance | {before_summary['stddev']:.4f} | {after_summary['stddev']:.4f} |",
        "",
        "| enemy_name | before_luminance | after_luminance | scaling_factor | new_asset_path |",
        "|---|---:|---:|---:|---|",
    ]
    for row in results:
        lines.append(
            f"| {row['enemy_name']} | {row['before_luminance']:.4f} | "
            f"{row['after_luminance']:.4f} | {row['scaling_factor']:.4f} | "
            f"`{row['new_asset_path']}` |"
        )
    if failures:
        lines += ["", "## Failures", ""]
        for failure in failures:
            lines.append(f"- {failure['enemy_name']}: {failure['error']}")

    REPORT_MD.write_text("\n".join(lines) + "\n", encoding="utf-8")


def write_progress_payload(fieldnames, rows, results, failures):
    write_visual_rows(fieldnames, rows)
    payload = {
        "target_luminance": TARGET_LUMINANCE,
        "max_scaling_factor": MAX_SCALING_FACTOR,
        "saturation_boost": SATURATION_BOOST,
        "processed_count": len(results),
        "failure_count": len(failures),
        "results": results,
        "failures": failures,
    }
    REPORT_JSON.write_text(json.dumps(payload, indent=2), encoding="utf-8")


def main():
    ensure_dirs()
    if not Path(BLENDER_EXE).exists():
        raise RuntimeError(f"Blender executable not found: {BLENDER_EXE}")
    if not shutil.which(SYSTEM_PYTHON):
        raise RuntimeError(f"System Python command not found: {SYSTEM_PYTHON}")

    fieldnames, rows = read_visual_rows()
    targets = selected_rows(rows)
    if len(targets) != 75:
        warn(f"Expected 75 QuadRetro runtime rows; found {len(targets)}")

    with tempfile.TemporaryDirectory(prefix="T66_Track1_Helpers_") as helper_tmp:
        helper_dir = Path(helper_tmp)
        helpers = write_helper_scripts(helper_dir)
        results = []
        failures = []
        rows_by_name = {row["---"]: row for row in rows}
        if REPORT_JSON.exists():
            try:
                existing_payload = json.loads(REPORT_JSON.read_text(encoding="utf-8"))
                normalized_row_names = {
                    row["---"]
                    for row in rows
                    if NORMALIZED_SUFFIX in asset_name(row.get("PixelatedTextureAssetPath") or "")
                }
                results = [
                    result
                    for result in existing_payload.get("results", [])
                    if result.get("enemy_name") in normalized_row_names
                ]
                if results:
                    log(f"Resumed {len(results)} completed rows from {REPORT_JSON}")
            except Exception as exc:
                warn(f"Could not load existing progress payload {REPORT_JSON}: {exc}")

        for index, row in enumerate(targets, start=1):
            row_name = row["---"]
            try:
                log(f"({index}/{len(targets)}) Normalizing {row_name}")
                result = normalize_one(row, helpers, rows_by_name)
                results.append(result)
                log(
                    f"{row_name}: {result['before_luminance']:.4f} -> "
                    f"{result['after_luminance']:.4f} scale={result['scaling_factor']:.3f}"
                )
                write_progress_payload(fieldnames, rows, results, failures)
            except Exception as exc:
                failures.append({"enemy_name": row_name, "error": str(exc)})
                error(f"{row_name}: {exc}")
                write_progress_payload(fieldnames, rows, results, failures)

    if failures:
        warn(f"Completed with {len(failures)} failures; CSV/DataTable update will still include successful rows.")

    write_progress_payload(fieldnames, rows, results, failures)
    reload_character_visuals_data_table()
    write_task2_markdown(results, failures)
    unreal.EditorAssetLibrary.save_directory("/Game/Characters", only_if_is_dirty=True, recursive=True)
    unreal.EditorAssetLibrary.save_asset(DT_CHARACTER_VISUALS)
    log(f"Wrote {REPORT_JSON}")
    log(f"Wrote {REPORT_MD}")
    quit_editor()


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        error(str(exc))
        quit_editor()
        raise
