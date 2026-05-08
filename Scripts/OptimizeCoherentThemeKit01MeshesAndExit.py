"""
Apply runtime mesh optimization settings to the generated CoherentThemeKit01
wall/floor kit, then request editor shutdown.

This is intentionally an Unreal-side runtime pass, not a RetopoFlow artifact
creator. Raw TRELLIS meshes remain the source assets; this script prepares the
imported StaticMesh assets with generated LODs, Nanite, and simple fallback
settings for packaged play.
"""

import json
import os
import sys

import unreal


SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
if SCRIPT_DIR not in sys.path:
    sys.path.append(SCRIPT_DIR)

import ImportStaticMeshes


LOG_PREFIX = "[OptimizeCoherentThemeKit01Meshes]"
DEST_DIR = ImportStaticMeshes.COHERENT_THEME_KIT_DEST
MODULE_IDS = ImportStaticMeshes.COHERENT_THEME_KIT_MODULES
REPORT_PATH = os.path.join(
    unreal.SystemLibrary.get_project_directory(),
    "Saved",
    "Logs",
    "OptimizeCoherentThemeKit01Meshes.json",
)

NANITE_MATERIAL_PATHS = (
    "/Game/Materials/M_Environment_Unlit",
    "/Game/Materials/M_GLB_Unlit",
    "/Game/Materials/Retro/M_GLB_Unlit_RetroGeometry",
)

LOD_SPECS = {
    "floor": (
        {"percent_triangles": 0.30, "screen_size": 0.55},
        {"percent_triangles": 0.12, "screen_size": 0.25},
        {"percent_triangles": 0.04, "screen_size": 0.08},
    ),
    "wall": (
        {"percent_triangles": 0.35, "screen_size": 0.65},
        {"percent_triangles": 0.16, "screen_size": 0.32},
        {"percent_triangles": 0.06, "screen_size": 0.12},
    ),
}

NANITE_SPECS = {
    "floor": {
        "keep_percent_triangles": 0.35,
        "trim_relative_error": 0.0,
        "fallback_percent_triangles": 0.06,
        "fallback_relative_error": 1.0,
    },
    "wall": {
        "keep_percent_triangles": 0.45,
        "trim_relative_error": 0.0,
        "fallback_percent_triangles": 0.08,
        "fallback_relative_error": 1.0,
    },
}


def _asset_path(module_id):
    return f"{DEST_DIR}/{module_id}_UnrealReady"


def _kind(module_id):
    return "wall" if "Wall" in module_id else "floor"


def _set_first(obj, names, value):
    errors = []
    for name in names:
        try:
            obj.set_editor_property(name, value)
            return name
        except Exception as exc:
            errors.append(f"{name}: {exc}")
    return None


def _get_first(obj, names, default=None):
    for name in names:
        try:
            return obj.get_editor_property(name)
        except Exception:
            continue
    return default


def _safe_save(asset, path=None):
    try:
        return bool(unreal.EditorAssetLibrary.save_loaded_asset(asset))
    except Exception:
        if path:
            try:
                return bool(unreal.EditorAssetLibrary.save_asset(path))
            except Exception:
                return False
    return False


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
    candidates = (
        "get_number_triangles",
        "get_num_triangles",
        "get_triangle_count",
        "get_lod_triangles",
    )
    for name in candidates:
        func = getattr(unreal.EditorStaticMeshLibrary, name, None)
        if not func:
            continue
        try:
            return int(func(mesh, lod_index))
        except Exception:
            continue
    return None


def _make_lod_options(kind):
    if not hasattr(unreal, "StaticMeshReductionOptions"):
        raise RuntimeError("unreal.StaticMeshReductionOptions is unavailable")
    if not hasattr(unreal, "StaticMeshReductionSettings"):
        raise RuntimeError("unreal.StaticMeshReductionSettings is unavailable")

    settings = []
    for spec in LOD_SPECS[kind]:
        reduction = unreal.StaticMeshReductionSettings()
        _set_first(reduction, ("percent_triangles",), float(spec["percent_triangles"]))
        _set_first(reduction, ("screen_size",), float(spec["screen_size"]))
        settings.append(reduction)

    options = unreal.StaticMeshReductionOptions()
    _set_first(
        options,
        ("auto_compute_lod_screen_size", "b_auto_compute_lod_screen_size"),
        False,
    )
    if not _set_first(options, ("reduction_settings",), settings):
        raise RuntimeError("failed to assign reduction_settings")
    return options


def _apply_lods(mesh, module_id, subsystem):
    before = _lod_count(mesh, subsystem)
    result = {"before_lod_count": before, "after_lod_count": before, "ok": False}

    if not subsystem or not hasattr(subsystem, "set_lods"):
        result["error"] = "StaticMeshEditorSubsystem.set_lods unavailable"
        return result

    kind = _kind(module_id)
    try:
        options = _make_lod_options(kind)
        returned_count = subsystem.set_lods(mesh, options)
        after = _lod_count(mesh, subsystem)
        result.update(
            {
                "returned_lod_count": int(returned_count),
                "after_lod_count": int(after),
                "ok": after >= 3,
                "spec": list(LOD_SPECS[kind]),
            }
        )
    except Exception as exc:
        result["error"] = str(exc)

    return result


def _nanite_snapshot(settings):
    return {
        "enabled": bool(_get_first(settings, ("enabled", "b_enabled"), False)),
        "keep_percent_triangles": _get_first(settings, ("keep_percent_triangles",), None),
        "trim_relative_error": _get_first(settings, ("trim_relative_error",), None),
        "fallback_percent_triangles": _get_first(settings, ("fallback_percent_triangles",), None),
        "fallback_relative_error": _get_first(settings, ("fallback_relative_error",), None),
        "generate_fallback": str(_get_first(settings, ("generate_fallback",), "")),
        "fallback_target": str(_get_first(settings, ("fallback_target",), "")),
    }


def _enum_value(enum_name, value_names):
    enum_type = getattr(unreal, enum_name, None)
    if not enum_type:
        return None
    for value_name in value_names:
        value = getattr(enum_type, value_name, None)
        if value is not None:
            return value
    return None


def _apply_nanite(mesh, module_id, subsystem):
    result = {"ok": False}
    if not subsystem or not hasattr(subsystem, "get_nanite_settings") or not hasattr(subsystem, "set_nanite_settings"):
        result["error"] = "StaticMeshEditorSubsystem Nanite APIs unavailable"
        return result

    kind = _kind(module_id)
    spec = NANITE_SPECS[kind]

    try:
        settings = subsystem.get_nanite_settings(mesh)
        result["before"] = _nanite_snapshot(settings)

        _set_first(settings, ("enabled", "b_enabled"), True)
        _set_first(settings, ("keep_percent_triangles",), float(spec["keep_percent_triangles"]))
        _set_first(settings, ("trim_relative_error",), float(spec["trim_relative_error"]))
        _set_first(settings, ("fallback_percent_triangles",), float(spec["fallback_percent_triangles"]))
        _set_first(settings, ("fallback_relative_error",), float(spec["fallback_relative_error"]))
        _set_first(settings, ("lerp_uvs", "b_lerp_uvs", "lerp_u_vs", "b_lerp_u_vs"), True)

        generate_fallback = _enum_value("NaniteGenerateFallback", ("ENABLED", "Enabled"))
        if generate_fallback is not None:
            _set_first(settings, ("generate_fallback",), generate_fallback)

        fallback_target = _enum_value("NaniteFallbackTarget", ("PERCENT_TRIANGLES", "PercentTriangles"))
        if fallback_target is not None:
            _set_first(settings, ("fallback_target",), fallback_target)

        subsystem.set_nanite_settings(mesh, settings)
        after = subsystem.get_nanite_settings(mesh)
        result["after"] = _nanite_snapshot(after)
        result["spec"] = dict(spec)
        result["ok"] = bool(result["after"].get("enabled"))
    except Exception as exc:
        result["error"] = str(exc)

    return result


def _apply_material_flags():
    results = []
    for material_path in NANITE_MATERIAL_PATHS:
        asset = unreal.EditorAssetLibrary.load_asset(material_path)
        row = {"path": material_path, "loaded": bool(asset), "ok": False}
        if not asset:
            results.append(row)
            continue

        changed = []
        for prop_name in ("used_with_instanced_static_meshes", "used_with_nanite"):
            try:
                current = bool(asset.get_editor_property(prop_name))
                if not current:
                    asset.set_editor_property(prop_name, True)
                    changed.append(prop_name)
                row[prop_name] = bool(asset.get_editor_property(prop_name))
            except Exception as exc:
                row[prop_name] = f"ERROR: {exc}"

        row["changed"] = changed
        row["saved"] = _safe_save(asset, material_path)
        row["ok"] = bool(row.get("used_with_instanced_static_meshes")) and bool(row.get("used_with_nanite"))
        results.append(row)
    return results


def _optimize_mesh(module_id, subsystem):
    path = _asset_path(module_id)
    mesh = unreal.EditorAssetLibrary.load_asset(path)
    row = {
        "module_id": module_id,
        "asset_path": path,
        "kind": _kind(module_id),
        "loaded": bool(mesh),
        "ok": False,
    }

    if not mesh or not isinstance(mesh, unreal.StaticMesh):
        row["error"] = "StaticMesh not found"
        return row

    row["triangles_before"] = [
        _triangle_count(mesh, index) for index in range(max(1, _lod_count(mesh, subsystem)))
    ]
    row["lod"] = _apply_lods(mesh, module_id, subsystem)
    row["nanite"] = _apply_nanite(mesh, module_id, subsystem)

    try:
        mesh.post_edit_change()
    except Exception:
        pass

    row["triangles_after"] = [
        _triangle_count(mesh, index) for index in range(max(1, _lod_count(mesh, subsystem)))
    ]
    row["saved"] = _safe_save(mesh, path)
    row["ok"] = bool(row["lod"].get("ok")) and bool(row["nanite"].get("ok"))
    return row


def main():
    subsystem = None
    try:
        subsystem = unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)
    except Exception as exc:
        unreal.log_error(f"{LOG_PREFIX} StaticMeshEditorSubsystem unavailable: {exc}")

    material_results = _apply_material_flags()
    mesh_results = []

    for index, module_id in enumerate(MODULE_IDS, start=1):
        unreal.log(f"{LOG_PREFIX} Optimizing {index}/40 {module_id}")
        row = _optimize_mesh(module_id, subsystem)
        mesh_results.append(row)
        if not row.get("ok"):
            unreal.log_warning(f"{LOG_PREFIX} {module_id} not fully optimized: {row}")

    report = {
        "destination": DEST_DIR,
        "materials": material_results,
        "meshes": mesh_results,
        "optimized_meshes": sum(1 for row in mesh_results if row.get("ok")),
        "material_ready": sum(1 for row in material_results if row.get("ok")),
    }
    report["ok"] = report["optimized_meshes"] == len(MODULE_IDS)

    os.makedirs(os.path.dirname(REPORT_PATH), exist_ok=True)
    with open(REPORT_PATH, "w", encoding="utf-8") as handle:
        json.dump(report, handle, indent=2, default=str)

    unreal.log(
        f"{LOG_PREFIX} optimized_meshes={report['optimized_meshes']}/40 "
        f"material_ready={report['material_ready']}/{len(material_results)} "
        f"report={REPORT_PATH} ok={str(report['ok']).lower()}"
    )

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
