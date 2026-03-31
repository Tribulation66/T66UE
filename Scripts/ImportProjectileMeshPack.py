"""
Import the generated projectile mesh pack into stable Unreal content paths.

This intentionally leaves imported GLB materials intact instead of converting
them to the unlit master, because the sword and hard-surface idol meshes rely
on their simple lit shading to read as solid objects.
"""

import os
import sys

import unreal


SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
if SCRIPT_DIR not in sys.path:
    sys.path.append(SCRIPT_DIR)

import ImportStaticMeshes


IMPORTS = [
    ("VFX/ProjectileMeshPack/Arthur_Sword.glb", "/Game/VFX/Projectiles/Hero1", "Arthur_Sword"),
    ("VFX/ProjectileMeshPack/Idol_Bone_Projectile.glb", "/Game/VFX/Projectiles/Idols", "Idol_Bone_Projectile"),
    ("VFX/ProjectileMeshPack/Idol_Glass_Shard.glb", "/Game/VFX/Projectiles/Idols", "Idol_Glass_Shard"),
    ("VFX/ProjectileMeshPack/Idol_Steel_Spike.glb", "/Game/VFX/Projectiles/Idols", "Idol_Steel_Spike"),
    ("VFX/ProjectileMeshPack/Idol_Wood_Thorn.glb", "/Game/VFX/Projectiles/Idols", "Idol_Wood_Thorn"),
    ("VFX/ProjectileMeshPack/Idol_Light_Lance.glb", "/Game/VFX/Projectiles/Idols", "Idol_Light_Lance"),
    ("VFX/ProjectileMeshPack/Idol_Wind_Crescent.glb", "/Game/VFX/Projectiles/Idols", "Idol_Wind_Crescent"),
    ("VFX/ProjectileMeshPack/Idol_Rubber_Ball.glb", "/Game/VFX/Projectiles/Idols", "Idol_Rubber_Ball"),
    ("VFX/ProjectileMeshPack/Idol_Ice_Crystal.glb", "/Game/VFX/Projectiles/Idols", "Idol_Ice_Crystal"),
    ("VFX/ProjectileMeshPack/Idol_Star_Shard.glb", "/Game/VFX/Projectiles/Idols", "Idol_Star_Shard"),
    ("VFX/ProjectileMeshPack/Idol_Sound_Disc.glb", "/Game/VFX/Projectiles/Idols", "Idol_Sound_Disc"),
    ("VFX/ProjectileMeshPack/Idol_Shadow_Orb.glb", "/Game/VFX/Projectiles/Idols", "Idol_Shadow_Orb"),
]


def _project_import_root():
    project_dir = unreal.SystemLibrary.get_project_directory().replace("\\", "/").rstrip("/")
    return f"{project_dir}/SourceAssets/Import"


def _ensure_directory(game_path):
    if not unreal.EditorAssetLibrary.does_directory_exist(game_path):
        unreal.EditorAssetLibrary.make_directory(game_path)


def main():
    import_root = _project_import_root()
    unreal.log("=" * 60)
    unreal.log("[ImportProjectileMeshPack] START")
    unreal.log(f"[ImportProjectileMeshPack] Source root: {import_root}")

    success = 0
    failed = 0

    for relative_source, dest_dir, dest_name in IMPORTS:
        source_path = os.path.join(import_root, relative_source).replace("\\", "/")
        if not os.path.isfile(source_path):
            unreal.log_warning(f"[ImportProjectileMeshPack] Missing source: {source_path}")
            failed += 1
            continue

        try:
            _ensure_directory(dest_dir)
            ImportStaticMeshes._cleanup_existing_import_artifacts(dest_dir, dest_name, {})
            imported = ImportStaticMeshes.import_glb(source_path, dest_dir, dest_name)
            final_path = ImportStaticMeshes._flatten_interchange_asset(dest_dir, dest_name)
            unreal.log(
                f"[ImportProjectileMeshPack] Imported {dest_name} -> {dest_dir} "
                f"assets={len(imported)} final={final_path}"
            )
            if final_path:
                mesh = unreal.EditorAssetLibrary.load_asset(final_path)
                if mesh:
                    unreal.EditorAssetLibrary.save_loaded_asset(mesh)
            success += 1
        except Exception as exc:
            unreal.log_error(f"[ImportProjectileMeshPack] FAILED {dest_name}: {exc}")
            failed += 1

    unreal.log(f"[ImportProjectileMeshPack] DONE success={success} failed={failed}")
    unreal.log("=" * 60)


if __name__ == "__main__":
    main()
