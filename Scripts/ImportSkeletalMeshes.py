"""
Import skeletal mesh FBXs into the project. Vanilla import — no material manipulation.
Each FBX goes to its own subdirectory. NO renaming, NO moving.

Update the IMPORTS list below before each run, then execute in Unreal Editor:
  py "C:/UE/T66/Scripts/ImportSkeletalMeshes.py"

After importing, run MakeAllMaterialsUnlit.py as a separate step.
"""

import os
import unreal

CLEANUP_DIRS = [
    "/Game/Characters/Heroes/Mage",
]

IMPORTS = [
    {
        "source": "ArthurIdle.fbx",
        "dest": "/Game/Characters/Heroes/Hero_1/TypeA/Idle",
        "name": "ArthurIdle",
    },
    {
        "source": "ArthurWalk.fbx",
        "dest": "/Game/Characters/Heroes/Hero_1/TypeA/Walk",
        "name": "ArthurWalk",
    },
]


def _get_project_dir():
    try:
        return unreal.SystemLibrary.get_project_directory().replace("\\", "/").rstrip("/")
    except Exception:
        return os.path.abspath(os.path.join(os.getcwd(), "..")).replace("\\", "/")


def main():
    proj = _get_project_dir()
    import_root = os.path.join(proj, "SourceAssets", "Import").replace("\\", "/")

    unreal.log("=" * 60)
    unreal.log("[ImportSkeletalMeshes] START")
    unreal.log("=" * 60)

    for d in CLEANUP_DIRS:
        if unreal.EditorAssetLibrary.does_directory_exist(d):
            unreal.log(f"  [CLEANUP] {d}")
            unreal.EditorAssetLibrary.delete_directory(d)

    for entry in IMPORTS:
        source = os.path.join(import_root, entry["source"]).replace("\\", "/")
        dest = entry["dest"]
        name = entry["name"]

        if not os.path.isfile(source):
            unreal.log_warning(f"  [SKIP] {entry['source']} not found")
            continue

        if not unreal.EditorAssetLibrary.does_directory_exist(dest):
            unreal.EditorAssetLibrary.make_directory(dest)

        unreal.log(f"")
        unreal.log(f"  [IMPORT] {entry['source']} -> {dest}")

        task = unreal.AssetImportTask()
        task.automated = True
        task.save = True
        task.replace_existing = True
        task.filename = source
        task.destination_path = dest
        task.destination_name = name

        unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

        imported = list(task.imported_object_paths or [])
        for p in imported:
            obj = unreal.EditorAssetLibrary.load_asset(p)
            cls = type(obj).__name__ if obj else "?"
            extra = ""
            try:
                skel = obj.get_editor_property("skeleton")
                if skel:
                    extra = f" skeleton={skel.get_name()}"
            except Exception:
                pass
            unreal.log(f"    {p}  [{cls}]{extra}")

    unreal.log("")
    unreal.log("=" * 60)
    unreal.log("[ImportSkeletalMeshes] DONE")
    unreal.log("Next: run MakeAllMaterialsUnlit.py, then reload DT_CharacterVisuals.")
    unreal.log("=" * 60)


if __name__ == "__main__":
    main()
