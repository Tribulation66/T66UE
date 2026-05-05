"""
Import skeletal mesh FBXs into the project and immediately convert imported
character materials to the project's Unlit pipeline.
Each FBX goes to its own subdirectory. NO renaming, NO moving.

Update the IMPORTS list below before each run, then execute in Unreal Editor:
  py "C:/UE/T66/Scripts/ImportSkeletalMeshes.py"
"""

import os
import sys
import unreal

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
if SCRIPT_DIR not in sys.path:
    sys.path.append(SCRIPT_DIR)

import MakeCharacterMaterialsUnlit

CLEANUP_DIRS = []
FILTER_SOURCE = os.environ.get("T66_IMPORT_SKELETAL_SOURCE", "").strip().replace("\\", "/")

IMPORTS = [
    {
        "source": "Heros/ArthurAIdle.fbx",
        "dest": "/Game/Characters/Heroes/Hero_1/Chad/Idle",
        "name": "ArthurAIdle",
    },
    {
        "source": "Heros/ArthurAWalk.fbx",
        "dest": "/Game/Characters/Heroes/Hero_1/Chad/Walk",
        "name": "ArthurAWalk",
    },
    {
        "source": "Heros/ArthurBIdle.fbx",
        "dest": "/Game/Characters/Heroes/Hero_1/Stacy/Idle",
        "name": "ArthurBIdle",
    },
    {
        "source": "Heros/ArthurBWalk.fbx",
        "dest": "/Game/Characters/Heroes/Hero_1/Stacy/Walk",
        "name": "ArthurBWalk",
    },
    {
        "source": "Companion/CompanionIdle.fbx",
        "dest": "/Game/Characters/Companions/Companion_01/Default/Idle",
        "name": "CompanionIdle",
    },
    {
        "source": "Companion/CompanionWalk.fbx",
        "dest": "/Game/Characters/Companions/Companion_01/Default/Walk",
        "name": "CompanionWalk",
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
    imports = list(IMPORTS)
    if FILTER_SOURCE:
        imports = [entry for entry in IMPORTS if entry["source"].replace("\\", "/") == FILTER_SOURCE]

    unreal.log("=" * 60)
    unreal.log("[ImportSkeletalMeshes] START")
    unreal.log("=" * 60)
    if FILTER_SOURCE:
        unreal.log(f"[ImportSkeletalMeshes] Filter source: {FILTER_SOURCE} ({len(imports)} matching entries)")

    for d in CLEANUP_DIRS:
        if unreal.EditorAssetLibrary.does_directory_exist(d):
            unreal.log(f"  [CLEANUP] {d}")
            unreal.EditorAssetLibrary.delete_directory(d)

    for entry in imports:
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

        unlit_results = MakeCharacterMaterialsUnlit.convert_character_materials_unlit([dest])
        unreal.log(
            "    [UNLIT] converted={converted} already_ok={already_ok} "
            "skipped={skipped} no_texture={no_texture} errors={errors}".format(**unlit_results)
        )

    unreal.log("")
    unreal.log("=" * 60)
    unreal.log("[ImportSkeletalMeshes] DONE")
    unreal.log("Next: update CharacterVisuals.csv and run SetupCharacterVisualsDataTable.py.")
    unreal.log("=" * 60)


if __name__ == "__main__":
    main()
