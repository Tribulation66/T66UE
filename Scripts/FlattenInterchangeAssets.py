"""
Fix Interchange folder structure for already-imported assets.

Interchange creates: dest_dir/Name/StaticMeshes/Name
This script moves them to: dest_dir/Name

Run INSIDE Unreal Editor:
  py "C:/UE/T66/Scripts/FlattenInterchangeAssets.py"

Run this ONCE after importing with ImportStaticMeshes.py if assets
ended up in Interchange's nested folder structure.
"""

import unreal

EXPECTED_ASSETS = [
    ("/Game/World/Interactables/Chests/Yellow", "ChestYellow"),
    ("/Game/World/Interactables",               "Crate"),
    ("/Game/World/Interactables",               "Totem"),
    ("/Game/World/Interactables/Wheels",        "Wheel"),
    ("/Game/World/Props",                       "Barn"),
    ("/Game/World/Props",                       "Tree"),
    ("/Game/World/Props",                       "Tree2"),
]


def _find_static_mesh_recursive(game_dir):
    """Find first StaticMesh asset recursively in a directory."""
    try:
        assets = unreal.EditorAssetLibrary.list_assets(game_dir, recursive=True, include_folder=False)
    except Exception:
        return None
    for p in assets:
        asset = unreal.EditorAssetLibrary.load_asset(p)
        if asset and isinstance(asset, unreal.StaticMesh):
            return str(p).split(".")[0]
    return None


def main():
    unreal.log("=" * 60)
    unreal.log("[FlattenInterchange] START")
    unreal.log("=" * 60)

    fixed = 0
    already_ok = 0
    not_found = 0

    for dest_dir, name in EXPECTED_ASSETS:
        expected = f"{dest_dir}/{name}"

        existing = unreal.EditorAssetLibrary.load_asset(expected)
        if existing and isinstance(existing, unreal.StaticMesh):
            unreal.log(f"  [OK] {expected} — already in place")
            already_ok += 1
            continue

        interchange_dir = f"{dest_dir}/{name}"
        mesh_path = _find_static_mesh_recursive(interchange_dir)

        if not mesh_path:
            unreal.log_warning(f"  [NOT FOUND] No StaticMesh under {interchange_dir}")
            not_found += 1
            continue

        if mesh_path == expected:
            already_ok += 1
            continue

        if unreal.EditorAssetLibrary.does_asset_exist(expected):
            unreal.EditorAssetLibrary.delete_asset(expected)

        if unreal.EditorAssetLibrary.rename_asset(mesh_path, expected):
            unreal.EditorAssetLibrary.save_asset(expected)
            unreal.log(f"  [FIXED] {mesh_path} -> {expected}")
            fixed += 1
        else:
            unreal.log_error(f"  [FAIL] Could not rename {mesh_path} -> {expected}")

    unreal.log("")
    unreal.log("=" * 60)
    unreal.log(f"[FlattenInterchange] DONE — fixed={fixed}, already_ok={already_ok}, not_found={not_found}")
    unreal.log("=" * 60)


if __name__ == "__main__":
    main()
