"""
Creates a PhysicsAsset for the Stage 1 boss skeletal mesh (Boss_01) if missing.

This fixes the common UE issue where a skeletal mesh can be culled / appear invisible
because its bounds are incorrect (Epic notes skeletal meshes without a PhysicsAsset
as a common cause of out-of-bounds / culling artifacts).

Run (unattended is OK):
UnrealEditor-Cmd.exe "<uproject>" -run=pythonscript -script="C:/UE/T66/Scripts/CreatePhysicsAsset_Boss01.py" -unattended -nop4 -nosplash -nullrhi
"""

import unreal


def main():
    unreal.log("=== CreatePhysicsAsset_Boss01 ===")

    mesh_path = "/Game/Characters/Bosses/Stages/Boss_01/Meshy_AI_Character_output.Meshy_AI_Character_output"
    if not unreal.EditorAssetLibrary.does_asset_exist(mesh_path):
        unreal.log_error("Boss_01 SkeletalMesh not found: " + mesh_path)
        return

    sk = unreal.EditorAssetLibrary.load_asset(mesh_path)
    if not sk or not isinstance(sk, unreal.SkeletalMesh):
        unreal.log_error("Asset is not a SkeletalMesh: " + mesh_path)
        return

    try:
        phys = sk.get_editor_property("physics_asset")
    except Exception:
        phys = None

    if phys:
        unreal.log("Boss_01 already has PhysicsAsset: " + phys.get_path_name())
    else:
        unreal.log("Creating PhysicsAsset for Boss_01...")
        pa = unreal.EditorSkeletalMeshLibrary.create_physics_asset(sk)
        if not pa:
            unreal.log_error("Failed to create PhysicsAsset for Boss_01")
            return

        unreal.log("Created PhysicsAsset: " + pa.get_path_name())
        try:
            unreal.EditorAssetLibrary.save_loaded_asset(pa)
        except Exception:
            pass
        try:
            unreal.EditorAssetLibrary.save_loaded_asset(sk)
        except Exception:
            pass

    unreal.log("=== CreatePhysicsAsset_Boss01 Done ===")
    try:
        unreal.SystemLibrary.quit_editor()
    except Exception:
        pass


if __name__ == "__main__":
    main()

