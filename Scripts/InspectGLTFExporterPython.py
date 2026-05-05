import json
import os

import unreal


REPORT_PATH = os.path.join(
    unreal.SystemLibrary.get_project_directory(),
    "Saved",
    "Logs",
    "InspectGLTFExporterPython.json",
)


def main():
    names = [
        name
        for name in dir(unreal)
        if "GLTF" in name or "Gltf" in name or "gltf" in name
    ]
    report = {"names": names}
    for name in names:
        obj = getattr(unreal, name, None)
        if obj:
            report[name] = [member for member in dir(obj) if "export" in member.lower()]
    with open(REPORT_PATH, "w", encoding="utf-8") as handle:
        json.dump(report, handle, indent=2)
    unreal.log(f"[InspectGLTFExporterPython] Wrote {REPORT_PATH}")


if __name__ == "__main__":
    main()
