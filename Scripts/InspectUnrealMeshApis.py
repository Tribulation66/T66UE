import json
import os

import unreal


REPORT_PATH = os.path.join(
    unreal.SystemLibrary.get_project_directory(),
    "Saved",
    "Logs",
    "InspectUnrealMeshApis.json",
)


def main():
    data = {}

    try:
        data["EditorStaticMeshLibrary"] = [
            name for name in dir(unreal.EditorStaticMeshLibrary)
            if "lod" in name.lower() or "build" in name.lower() or "uv" in name.lower() or "precision" in name.lower()
        ]
    except Exception as exc:
        data["EditorStaticMeshLibrary_error"] = str(exc)

    try:
        data["StaticMeshEditorSubsystem"] = [
            name for name in dir(unreal.StaticMeshEditorSubsystem)
            if "lod" in name.lower() or "build" in name.lower() or "uv" in name.lower() or "precision" in name.lower()
        ]
    except Exception as exc:
        data["StaticMeshEditorSubsystem_error"] = str(exc)

    try:
        data["StaticMesh_methods"] = [
            name for name in dir(unreal.StaticMesh)
            if "lod" in name.lower() or "build" in name.lower() or "uv" in name.lower() or "precision" in name.lower()
        ]
    except Exception as exc:
        data["StaticMesh_methods_error"] = str(exc)

    os.makedirs(os.path.dirname(REPORT_PATH), exist_ok=True)
    with open(REPORT_PATH, "w", encoding="utf-8") as handle:
        json.dump(data, handle, indent=2)

    unreal.log(f"[InspectUnrealMeshApis] Wrote {REPORT_PATH}")


if __name__ == "__main__":
    main()
