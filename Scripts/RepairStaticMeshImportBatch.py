"""
Repair the current GLB static mesh import batch without re-importing source files.

Runs the GLB Unlit conversion over the relevant roots and rebinds the flattened
static mesh assets to the imported material interfaces.
"""

import os
import sys
import unreal


SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
if SCRIPT_DIR not in sys.path:
    sys.path.append(SCRIPT_DIR)

import ImportStaticMeshes
import MakeGLBImportsUnlit


def main():
    unreal.log("=" * 60)
    unreal.log("[RepairStaticMeshImportBatch] START")
    unreal.log("=" * 60)

    repaired = 0

    for entry in ImportStaticMeshes.IMPORTS:
        (
            _,
            dest_dir,
            dest_name,
            material_overrides,
            texture_parameter_overrides,
            mesh_build_settings_overrides,
            _cleanup_overrides,
        ) = ImportStaticMeshes._normalize_import_entry(entry)
        mesh_path = f"{dest_dir}/{dest_name}"
        scan_roots = ImportStaticMeshes._existing_scan_roots(dest_dir, dest_name)
        results = MakeGLBImportsUnlit.convert_glb_imports_unlit(scan_roots)
        unreal.log(
            "  [UNLIT] {name}: converted={converted} already_ok={already_ok} "
            "skipped={skipped} no_texture={no_texture} errors={errors}".format(
                name=dest_name, **results)
        )
        if ImportStaticMeshes._bind_materials_to_flattened_mesh(mesh_path, dest_dir, dest_name):
            ImportStaticMeshes._apply_static_mesh_build_settings(
                mesh_path, mesh_build_settings_overrides)
            ImportStaticMeshes._apply_material_overrides(dest_dir, dest_name, material_overrides)
            ImportStaticMeshes._apply_texture_parameter_overrides(
                dest_dir, dest_name, texture_parameter_overrides)
            repaired += 1

    unreal.log("=" * 60)
    unreal.log(f"[RepairStaticMeshImportBatch] DONE — rebound={repaired}")
    unreal.log("=" * 60)


if __name__ == "__main__":
    main()
