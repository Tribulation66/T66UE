"""
Import static mesh GLBs into the project. Vanilla import — no material manipulation.

Update the IMPORTS list below before each run, then execute in Unreal Editor:
  Tools -> Execute Python Script -> Scripts/ImportStaticMeshes.py
  or from Output Log:
  py "C:/UE/T66/Scripts/ImportStaticMeshes.py"

After importing, run MakeAllMaterialsUnlit.py as a separate step.
"""

import os
import unreal

# ======================================================================
# IMPORT MANIFEST — edit this before each import run.
#
# Each entry: (source_glb_relative_to_SourceAssets/Import, destination_game_path, asset_name)
#
# source:      Relative path from SourceAssets/Import/ to the .glb file
# destination: /Game/... content path where the asset should live
# name:        Desired asset name in the content browser
# ======================================================================

IMPORTS = [
    # --- Interactables: Chests (4 rarity variants) ---
    ("Interactables/ChestBlack.glb",  "/Game/World/Interactables/Chests/Black",   "ChestBlack"),
    ("Interactables/ChestRed.glb",    "/Game/World/Interactables/Chests/Red",     "ChestRed"),
    ("Interactables/ChestWhite.glb",  "/Game/World/Interactables/Chests/White",   "ChestWhite"),
    ("Interactables/ChestYellow.glb", "/Game/World/Interactables/Chests/Yellow",  "ChestYellow"),

    # --- Interactables: single-mesh ---
    ("Interactables/Crate.glb",  "/Game/World/Interactables",        "Crate"),
    ("Interactables/Totem.glb",  "/Game/World/Interactables",        "Totem"),
    ("Interactables/Wheel.glb",  "/Game/World/Interactables/Wheels", "Wheel"),

    # --- Props ---
    ("Props/Barn.glb",  "/Game/World/Props", "Barn"),
    ("Props/Hay.glb",   "/Game/World/Props", "Hay"),
    ("Props/Hay2.glb",  "/Game/World/Props", "Hay2"),
    ("Props/Tree.glb",  "/Game/World/Props", "Tree"),
    ("Props/Tree2.glb", "/Game/World/Props", "Tree2"),
]


def _get_project_dir():
    try:
        return unreal.SystemLibrary.get_project_directory().replace("\\", "/").rstrip("/")
    except Exception:
        return os.path.abspath(os.path.join(os.getcwd(), "..")).replace("\\", "/")


def _ensure_game_dir(game_path):
    if not unreal.EditorAssetLibrary.does_directory_exist(game_path):
        unreal.EditorAssetLibrary.make_directory(game_path)


def import_glb(source_path, dest_dir, dest_name):
    """Vanilla GLB import via Interchange pipeline. No material manipulation."""
    _ensure_game_dir(dest_dir)

    task = unreal.AssetImportTask()
    task.automated = True
    task.save = True
    task.replace_existing = True
    task.replace_existing_settings = True
    task.filename = source_path
    task.destination_path = dest_dir
    task.destination_name = dest_name

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

    imported = list(task.imported_object_paths or [])
    return imported


def _find_static_mesh_in_dir(game_dir):
    """Search a directory (recursively) for the first StaticMesh asset."""
    try:
        assets = unreal.EditorAssetLibrary.list_assets(game_dir, recursive=True, include_folder=False)
    except Exception:
        return None
    for asset_path in assets:
        asset = unreal.EditorAssetLibrary.load_asset(asset_path)
        if asset and isinstance(asset, unreal.StaticMesh):
            return asset_path
    return None


def _flatten_interchange_asset(dest_dir, dest_name):
    """
    Interchange creates: dest_dir/dest_name/StaticMeshes/dest_name
    We want:             dest_dir/dest_name

    Find the StaticMesh wherever Interchange put it and move it to the expected flat path.
    """
    expected_path = f"{dest_dir}/{dest_name}"

    existing = unreal.EditorAssetLibrary.load_asset(expected_path)
    if existing and isinstance(existing, unreal.StaticMesh):
        return expected_path

    interchange_dir = f"{dest_dir}/{dest_name}"
    mesh_path = _find_static_mesh_in_dir(interchange_dir)

    if not mesh_path:
        interchange_dir2 = dest_dir
        mesh_path = _find_static_mesh_in_dir(interchange_dir2)

    if not mesh_path:
        return None

    clean_path = str(mesh_path).split(".")[0]
    if clean_path == expected_path:
        return expected_path

    if unreal.EditorAssetLibrary.does_asset_exist(expected_path):
        unreal.EditorAssetLibrary.delete_asset(expected_path)

    success = unreal.EditorAssetLibrary.rename_asset(clean_path, expected_path)
    if success:
        unreal.EditorAssetLibrary.save_asset(expected_path)
        unreal.log(f"    [RENAME] {clean_path} -> {expected_path}")

        _cleanup_empty_interchange_dirs(dest_dir, dest_name)
        return expected_path
    else:
        unreal.log_warning(f"    [RENAME FAILED] {clean_path} -> {expected_path}")
        return clean_path


def _cleanup_empty_interchange_dirs(dest_dir, dest_name):
    """Remove the empty Interchange subdirectories after flattening."""
    for sub in ["StaticMeshes", "Materials", "Textures", "AnimationSequences"]:
        sub_dir = f"{dest_dir}/{dest_name}/{sub}"
        try:
            if unreal.EditorAssetLibrary.does_directory_exist(sub_dir):
                assets = unreal.EditorAssetLibrary.list_assets(sub_dir, recursive=True, include_folder=False)
                if not assets or len(assets) == 0:
                    unreal.EditorAssetLibrary.delete_directory(sub_dir)
        except Exception:
            pass


def main():
    proj = _get_project_dir()
    import_root = os.path.join(proj, "SourceAssets", "Import").replace("\\", "/")

    unreal.log("=" * 60)
    unreal.log("[ImportStaticMeshes] START")
    unreal.log(f"[ImportStaticMeshes] Source root: {import_root}")
    unreal.log(f"[ImportStaticMeshes] {len(IMPORTS)} entries in manifest")
    unreal.log("=" * 60)

    success_count = 0
    fail_count = 0
    skip_count = 0

    for relative_path, dest_dir, dest_name in IMPORTS:
        source = os.path.join(import_root, relative_path).replace("\\", "/")

        if not os.path.isfile(source):
            unreal.log_warning(f"  [SKIP] {relative_path} — file not found at {source}")
            skip_count += 1
            continue

        try:
            imported = import_glb(source, dest_dir, dest_name)
            asset_count = len(imported)
            unreal.log(f"  [OK] {dest_name} -> {dest_dir} ({asset_count} assets)")
            for p in imported:
                unreal.log(f"        {p}")

            final_path = _flatten_interchange_asset(dest_dir, dest_name)
            if final_path:
                unreal.log(f"    [FINAL] {final_path}")
            else:
                unreal.log_warning(f"    [WARN] Could not locate StaticMesh for {dest_name}")

            success_count += 1
        except Exception as e:
            unreal.log_error(f"  [FAIL] {dest_name} — {e}")
            fail_count += 1

    unreal.log("")
    unreal.log("=" * 60)
    unreal.log(f"[ImportStaticMeshes] DONE — success={success_count}, skipped={skip_count}, failed={fail_count}")
    unreal.log("=" * 60)
    if success_count > 0:
        unreal.log("[ImportStaticMeshes] Next step: run MakeAllMaterialsUnlit.py to convert materials to Unlit")


if __name__ == "__main__":
    main()
