"""
Inspect imported material asset classes under a few GLB batch roots.
"""

import json
import os
import unreal


ROOTS = [
    "/Game/World/Props/Tree2",
    "/Game/World/Props/Rock",
    "/Game/World/Props/Grass",
    "/Game/World/Props/Bush",
    "/Game/World/Props/Boulder",
    "/Game/World/Interactables/Fountain",
    "/Game/World/LootBags/Black",
]

REPORT_PATH = os.path.join(
    unreal.SystemLibrary.get_project_directory(), "Saved", "Logs", "InspectImportedMaterialClasses.json")


def main():
    rows = []
    unreal.log("=" * 60)
    unreal.log("[InspectImportedMaterialClasses] START")
    unreal.log("=" * 60)

    for root in ROOTS:
        unreal.log(f"[ROOT] {root}")
        try:
            asset_paths = unreal.EditorAssetLibrary.list_assets(
                root, recursive=True, include_folder=False)
        except Exception as exc:
            unreal.log_warning(f"  list_assets failed: {exc}")
            continue

        for path in asset_paths:
            asset = unreal.EditorAssetLibrary.load_asset(path)
            if not asset:
                unreal.log_warning(f"  [MISSING] {path}")
                rows.append({"path": path, "class": "<missing>", "parent": ""})
                continue

            class_name = asset.get_class().get_name()
            parent_path = ""
            try:
                parent = asset.get_editor_property("parent")
                if parent:
                    parent_path = parent.get_path_name()
            except Exception:
                parent_path = "<no parent property>"

            rows.append({"path": path, "class": class_name, "parent": parent_path})
            unreal.log(f"  {path} :: {class_name} :: {parent_path}")

    os.makedirs(os.path.dirname(REPORT_PATH), exist_ok=True)
    with open(REPORT_PATH, "w", encoding="utf-8") as handle:
        json.dump(rows, handle, indent=2)

    unreal.log("=" * 60)
    unreal.log("[InspectImportedMaterialClasses] DONE")
    unreal.log("=" * 60)


if __name__ == "__main__":
    main()
