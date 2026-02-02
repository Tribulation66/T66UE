"""
Creates missing PhysicsAssets for SkeletalMeshes referenced by Content/Data/CharacterVisuals.csv.

Why:
- A SkeletalMesh that animates outside its bounds can get frustum/occlusion-culled and appear "invisible" in-game.
- Epic explicitly calls out "Skeletal Mesh without a physics asset" as a common cause of out-of-bounds pixels / culling artifacts.

Run from Unreal Editor:
- Py Execute Script -> Scripts/CreatePhysicsAssetsForCharacterVisuals.py
"""

import csv
import os
import unreal


def _get_content_path(relative_path: str) -> str:
    try:
        proj = unreal.SystemLibrary.get_project_directory()
        return os.path.normpath(os.path.join(proj, "Content", relative_path))
    except Exception:
        return os.path.normpath(os.path.join("Content", relative_path))


def _load_skeletal_mesh(asset_path: str):
    obj = unreal.EditorAssetLibrary.load_asset(asset_path)
    if not obj:
        return None
    if isinstance(obj, unreal.SkeletalMesh):
        return obj
    return None


def main():
    unreal.log("=== CreatePhysicsAssetsForCharacterVisuals ===")

    csv_path = _get_content_path("Data/CharacterVisuals.csv")
    if not os.path.isfile(csv_path):
        unreal.log_error("CSV not found: " + csv_path)
        return

    created = 0
    already = 0
    skipped = 0
    missing_assets = 0
    seen_mesh_paths = set()

    with open(csv_path, "r", newline="", encoding="utf-8") as f:
        reader = csv.reader(f)
        header = next(reader, None)
        if not header:
            unreal.log_error("CSV is empty: " + csv_path)
            return

        idx_mesh = header.index("SkeletalMesh") if "SkeletalMesh" in header else -1
        if idx_mesh < 0:
            unreal.log_error("CSV missing SkeletalMesh column")
            return

        for row in reader:
            if not row or len(row) <= idx_mesh:
                continue

            mesh_path = row[idx_mesh].strip().strip('"')
            if not mesh_path:
                continue

            if mesh_path in seen_mesh_paths:
                continue
            seen_mesh_paths.add(mesh_path)

            if not unreal.EditorAssetLibrary.does_asset_exist(mesh_path):
                unreal.log_error("Missing SkeletalMesh asset: " + mesh_path)
                missing_assets += 1
                continue

            sk = _load_skeletal_mesh(mesh_path)
            if not sk:
                skipped += 1
                unreal.log_warning("Not a SkeletalMesh (skipping): " + mesh_path)
                continue

            phys = None
            try:
                phys = sk.get_editor_property("physics_asset")
            except Exception:
                phys = None

            if phys:
                already += 1
                continue

            # Creates a PhysicsAsset (default settings) and assigns it to the mesh.
            try:
                new_pa = unreal.EditorSkeletalMeshLibrary.create_physics_asset(sk)
            except Exception as e:
                unreal.log_error(f"Failed to create PhysicsAsset for {mesh_path}: {e}")
                continue

            if new_pa:
                created += 1
                # Save both the new PhysicsAsset and the SkeletalMesh (now referencing it).
                try:
                    unreal.EditorAssetLibrary.save_loaded_asset(new_pa)
                except Exception:
                    pass
                try:
                    unreal.EditorAssetLibrary.save_loaded_asset(sk)
                except Exception:
                    pass
                unreal.log(f"Created PhysicsAsset for {mesh_path} -> {new_pa.get_path_name()}")
            else:
                unreal.log_error("create_physics_asset returned None for: " + mesh_path)

    unreal.log(
        f"Done. unique_meshes={len(seen_mesh_paths)}, created={created}, already_had={already}, "
        f"skipped_non_skel={skipped}, missing_assets={missing_assets}"
    )
    unreal.log("=== CreatePhysicsAssetsForCharacterVisuals Done ===")

    # Keep consistent with other setup scripts: exit when run unattended.
    try:
        unreal.SystemLibrary.quit_editor()
    except Exception:
        pass


if __name__ == "__main__":
    main()

