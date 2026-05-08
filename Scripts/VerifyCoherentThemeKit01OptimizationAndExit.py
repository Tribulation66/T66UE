"""
Verify that CoherentThemeKit01 imported meshes have runtime optimization data:
generated LODs, Nanite enabled, and Nanite/instancing-compatible material flags.
"""

import json
import os
import sys

import unreal


SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
if SCRIPT_DIR not in sys.path:
    sys.path.append(SCRIPT_DIR)

import ImportStaticMeshes


LOG_PREFIX = "[VerifyCoherentThemeKit01Optimization]"
DEST_DIR = ImportStaticMeshes.COHERENT_THEME_KIT_DEST
MODULE_IDS = ImportStaticMeshes.COHERENT_THEME_KIT_MODULES
REPORT_PATH = os.path.join(
    unreal.SystemLibrary.get_project_directory(),
    "Saved",
    "Logs",
    "VerifyCoherentThemeKit01Optimization.json",
)
MATERIAL_PATHS = (
    "/Game/Materials/M_Environment_Unlit",
    "/Game/Materials/M_GLB_Unlit",
    "/Game/Materials/Retro/M_GLB_Unlit_RetroGeometry",
)


def _asset_path(module_id):
    return f"{DEST_DIR}/{module_id}_UnrealReady"


def _get_first(obj, names, default=None):
    for name in names:
        try:
            return obj.get_editor_property(name)
        except Exception:
            continue
    return default


def _lod_count(mesh, subsystem=None):
    for func in (
        lambda: int(mesh.get_num_lods()),
        lambda: int(subsystem.get_lod_count(mesh)) if subsystem else 0,
        lambda: int(unreal.EditorStaticMeshLibrary.get_lod_count(mesh)),
    ):
        try:
            count = func()
            if count > 0:
                return count
        except Exception:
            continue
    return 0


def _triangle_count(mesh, lod_index):
    for name in ("get_number_triangles", "get_num_triangles", "get_triangle_count", "get_lod_triangles"):
        func = getattr(unreal.EditorStaticMeshLibrary, name, None)
        if not func:
            continue
        try:
            return int(func(mesh, lod_index))
        except Exception:
            continue
    return None


def _nanite_enabled(mesh, subsystem):
    if not subsystem or not hasattr(subsystem, "get_nanite_settings"):
        return False, {}
    try:
        settings = subsystem.get_nanite_settings(mesh)
        snapshot = {
            "enabled": bool(_get_first(settings, ("enabled", "b_enabled"), False)),
            "keep_percent_triangles": _get_first(settings, ("keep_percent_triangles",), None),
            "generate_fallback": str(_get_first(settings, ("generate_fallback",), "")),
            "fallback_target": str(_get_first(settings, ("fallback_target",), "")),
            "fallback_percent_triangles": _get_first(settings, ("fallback_percent_triangles",), None),
            "fallback_relative_error": _get_first(settings, ("fallback_relative_error",), None),
        }
        return bool(snapshot["enabled"]), snapshot
    except Exception as exc:
        return False, {"error": str(exc)}


def _material_row(material_path):
    asset = unreal.EditorAssetLibrary.load_asset(material_path)
    row = {"path": material_path, "loaded": bool(asset), "ok": False}
    if not asset:
        return row
    for prop_name in ("used_with_instanced_static_meshes", "used_with_nanite"):
        try:
            row[prop_name] = bool(asset.get_editor_property(prop_name))
        except Exception as exc:
            row[prop_name] = f"ERROR: {exc}"
    row["ok"] = bool(row.get("used_with_instanced_static_meshes")) and bool(row.get("used_with_nanite"))
    return row


def _mesh_row(module_id, subsystem):
    path = _asset_path(module_id)
    mesh = unreal.EditorAssetLibrary.load_asset(path)
    row = {"module_id": module_id, "path": path, "loaded": bool(mesh), "ok": False}
    if not mesh or not isinstance(mesh, unreal.StaticMesh):
        row["error"] = "StaticMesh not found"
        return row

    lod_count = _lod_count(mesh, subsystem)
    nanite_ok, nanite = _nanite_enabled(mesh, subsystem)
    row["lod_count"] = lod_count
    row["nanite"] = nanite
    row["triangles"] = [_triangle_count(mesh, index) for index in range(max(1, lod_count))]
    row["ok"] = lod_count >= 3 and nanite_ok
    return row


def main():
    try:
        subsystem = unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)
    except Exception:
        subsystem = None

    materials = [_material_row(path) for path in MATERIAL_PATHS]
    meshes = [_mesh_row(module_id, subsystem) for module_id in MODULE_IDS]

    report = {
        "destination": DEST_DIR,
        "materials": materials,
        "meshes": meshes,
        "material_ready": sum(1 for row in materials if row.get("ok")),
        "lod_ready": sum(1 for row in meshes if int(row.get("lod_count") or 0) >= 3),
        "nanite_ready": sum(1 for row in meshes if bool(row.get("nanite", {}).get("enabled"))),
    }
    report["ok"] = (
        report["lod_ready"] == len(MODULE_IDS)
        and report["nanite_ready"] == len(MODULE_IDS)
        and report["material_ready"] == len(MATERIAL_PATHS)
    )

    os.makedirs(os.path.dirname(REPORT_PATH), exist_ok=True)
    with open(REPORT_PATH, "w", encoding="utf-8") as handle:
        json.dump(report, handle, indent=2, default=str)

    line = (
        f"{LOG_PREFIX} lod_ready={report['lod_ready']}/40 "
        f"nanite_ready={report['nanite_ready']}/40 "
        f"material_ready={report['material_ready']}/{len(MATERIAL_PATHS)} "
        f"report={REPORT_PATH} ok={str(report['ok']).lower()}"
    )
    if report["ok"]:
        unreal.log(line)
    else:
        unreal.log_error(line)

    world = None
    try:
        editor_subsystem = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
        if editor_subsystem:
            world = editor_subsystem.get_editor_world()
    except Exception:
        pass

    try:
        unreal.SystemLibrary.execute_console_command(world, "QUIT_EDITOR")
        unreal.log(f"{LOG_PREFIX} QUIT_EDITOR requested")
    except Exception as exc:
        unreal.log_warning(f"{LOG_PREFIX} Failed to request QUIT_EDITOR: {exc}")


if __name__ == "__main__":
    main()
