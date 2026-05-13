"""
Generate the QuadRetro character static-mesh LOD ladder.

Run in-editor:
  UnrealEditor.exe T66.uproject -ExecutePythonScript="C:/UE/T66/Scripts/GenerateCharacterMeshLODs.py"
"""

import json
import os
import sys

import unreal


SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
if SCRIPT_DIR not in sys.path:
    sys.path.append(SCRIPT_DIR)

import QuadRetroCharacterPipelineDefaults as CharacterDefaults


LOG_PREFIX = "[GenerateCharacterMeshLODs]"
CHARACTER_ROOT = "/Game/Characters"
REPORT_PATH = os.path.join(
    unreal.SystemLibrary.get_project_directory(),
    "Saved",
    "Logs",
    "GenerateCharacterMeshLODs.json",
)


def _quit_editor():
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
        unreal.log_warning(f"{LOG_PREFIX} Failed to request QUIT_EDITOR: {exc}")


def _iter_quadretro_meshes():
    for asset_path in unreal.EditorAssetLibrary.list_assets(CHARACTER_ROOT, recursive=True, include_folder=False) or []:
        if not CharacterDefaults.is_quadretro_static_mesh_path(asset_path):
            continue
        asset = unreal.EditorAssetLibrary.load_asset(asset_path)
        if asset and isinstance(asset, unreal.StaticMesh):
            yield asset_path, asset


def _write_report(processed, errors, partial):
    payload = {
        "character_root": CHARACTER_ROOT,
        "lod_specs": list(CharacterDefaults.LOD_SPECS),
        "processed_count": len(processed),
        "errored_count": len(errors),
        "processed": processed,
        "errors": errors,
        "partial": partial,
        "ok": (not partial) and len(errors) == 0 and len(processed) > 0,
    }
    os.makedirs(os.path.dirname(REPORT_PATH), exist_ok=True)
    with open(REPORT_PATH, "w", encoding="utf-8") as handle:
        json.dump(payload, handle, indent=2, default=str)
    return payload


def main():
    subsystem = None
    try:
        subsystem = unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)
    except Exception as exc:
        unreal.log_error(f"{LOG_PREFIX} StaticMeshEditorSubsystem unavailable: {exc}")

    processed = []
    errors = []

    for asset_path, mesh in _iter_quadretro_meshes():
        try:
            result = CharacterDefaults.apply_lod_ladder_to_mesh(mesh, subsystem)
            if result.get("ok"):
                result["saved"] = CharacterDefaults.safe_save(mesh, asset_path)
                processed.append(result)
                unreal.log(f"{LOG_PREFIX} LODs ready {asset_path}: {result.get('triangles_after')}")
            else:
                errors.append({"asset": asset_path, "result": result})
                unreal.log_error(f"{LOG_PREFIX} Failed {asset_path}: {result}")
        except Exception as exc:
            errors.append({"asset": asset_path, "error": str(exc)})
            unreal.log_error(f"{LOG_PREFIX} Error {asset_path}: {exc}")
        _write_report(processed, errors, partial=True)

    payload = _write_report(processed, errors, partial=False)

    line = (
        f"{LOG_PREFIX} processed={len(processed)} errored={len(errors)} "
        f"report={REPORT_PATH} ok={str(payload['ok']).lower()}"
    )
    if payload["ok"]:
        unreal.log(line)
    else:
        unreal.log_error(line)

    _quit_editor()


if __name__ == "__main__":
    main()
