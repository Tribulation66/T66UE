# Copyright Tribulation 66. All Rights Reserved.
#
# MotionRig UE import (MOTION_RIG.md section 3).
# Imports the Blender-built skeletal mesh (+auto physics asset) and the six
# pose-target clips into /Game/Characters/MotionRig/Hero_1/.
#
# Run (editor closed is safest; new assets usually import fine alongside an
# open editor, retry on file-lock failures):
#   "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" ^
#       "C:\UE\T66\T66.uproject" -run=pythonscript ^
#       -script="C:/UE/T66/Scripts/MotionRig/ImportMotionRig.py" ^
#       -unattended -nop4 -nosplash -stdout

import json
import os

import unreal

SOURCE_ROOT = r"C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\MotionRig"
DEST_PATH = "/Game/Characters/MotionRig/Hero_1"
SK_NAME = "SK_MotionRig_Hero1"
CLIPS = ["Idle", "Walk", "Jump", "Dive", "GetUp_Front", "GetUp_Back"]

REPORT = {"skeletal_mesh": None, "physics_asset": None, "skeleton": None, "clips": {}, "errors": []}


def make_skeletal_mesh_options():
    options = unreal.FbxImportUI()
    options.import_mesh = True
    options.import_textures = True
    options.import_materials = True
    options.import_animations = False
    options.import_as_skeletal = True
    # The T66MotionRigPhysicsAsset commandlet owns PA generation (authored
    # params + body culling). The legacy importer DOES honor this flag (the
    # old "never runs automated" note was an Interchange behavior), so keep
    # it off or a stray auto-PA asset appears next to the mesh.
    options.create_physics_asset = False
    options.mesh_type_to_import = unreal.FBXImportType.FBXIT_SKELETAL_MESH
    options.automated_import_should_detect_type = False

    smd = options.skeletal_mesh_import_data
    smd.set_editor_property("import_morph_targets", False)
    # The skeletal FBX carries a baked 1-frame bind-pose animation in cm
    # (the exporter unit-converts keyed channels but NOT armature rest bones).
    # T0-as-ref-pose rebuilds the reference skeleton AND the render bind from
    # that cm data at import — the only point where the bind-dependent LOD
    # render caches are built. Post-import ref surgery cannot fix rendering.
    smd.set_editor_property("use_t0_as_ref_pose", True)
    smd.set_editor_property("preserve_smoothing_groups", True)
    smd.set_editor_property("import_meshes_in_bone_hierarchy", True)
    smd.set_editor_property("normal_import_method", unreal.FBXNormalImportMethod.FBXNIM_COMPUTE_NORMALS)
    smd.set_editor_property("convert_scene", True)
    smd.set_editor_property("force_front_x_axis", False)
    return options


def make_anim_options(skeleton):
    options = unreal.FbxImportUI()
    options.import_mesh = False
    options.import_textures = False
    options.import_materials = False
    options.import_animations = True
    options.import_as_skeletal = False
    options.skeleton = skeleton
    options.mesh_type_to_import = unreal.FBXImportType.FBXIT_ANIMATION
    options.automated_import_should_detect_type = False

    asd = options.anim_sequence_import_data
    asd.set_editor_property("animation_length", unreal.FBXAnimationLengthImportType.FBXALIT_EXPORTED_TIME)
    asd.set_editor_property("import_bone_tracks", True)
    asd.set_editor_property("remove_redundant_keys", False)
    asd.set_editor_property("use_default_sample_rate", False)
    asd.set_editor_property("convert_scene", True)
    asd.set_editor_property("force_front_x_axis", False)
    return options


def run_task(filename, destination_name, options):
    task = unreal.AssetImportTask()
    task.filename = filename
    task.destination_path = DEST_PATH
    task.destination_name = destination_name
    task.automated = True
    task.replace_existing = True
    task.save = True
    task.options = options
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    return list(task.imported_object_paths)


def main():
    # UE 5.7 routes FBX through Interchange by default, which IGNORES the
    # legacy FbxImportUI options on the task — measured: use_t0_as_ref_pose
    # had no effect (ref pose stayed meter-scale) until Interchange was
    # disabled for FBX. The legacy importer honors every option above.
    unreal.SystemLibrary.execute_console_command(None, "Interchange.FeatureFlags.Import.FBX 0")

    sk_fbx = os.path.join(SOURCE_ROOT, f"{SK_NAME}.fbx")
    if not os.path.exists(sk_fbx):
        REPORT["errors"].append(f"missing skeletal FBX: {sk_fbx}")
    else:
        imported = run_task(sk_fbx, SK_NAME, make_skeletal_mesh_options())
        REPORT["skeletal_mesh"] = imported

    sk_path = f"{DEST_PATH}/{SK_NAME}.{SK_NAME}"
    skeletal_mesh = unreal.EditorAssetLibrary.load_asset(sk_path)
    if not skeletal_mesh:
        REPORT["errors"].append(f"skeletal mesh did not import: {sk_path}")
    else:
        skeleton = skeletal_mesh.get_editor_property("skeleton")
        REPORT["skeleton"] = skeleton.get_path_name() if skeleton else None

        physics_asset = skeletal_mesh.get_editor_property("physics_asset")
        REPORT["physics_asset"] = physics_asset.get_path_name() if physics_asset else None
        if not physics_asset:
            REPORT["errors"].append("no physics asset was created on import")

        for clip in CLIPS:
            clip_fbx = os.path.join(SOURCE_ROOT, "AnimationSources", f"AM_MotionRig_Hero1_{clip}.fbx")
            if not os.path.exists(clip_fbx):
                REPORT["errors"].append(f"missing clip FBX: {clip_fbx}")
                continue
            imported = run_task(clip_fbx, f"AM_MotionRig_Hero1_{clip}", make_anim_options(skeleton))
            REPORT["clips"][clip] = imported

    unreal.EditorAssetLibrary.save_directory(DEST_PATH, only_if_is_dirty=False, recursive=True)

    report_path = os.path.join(SOURCE_ROOT, "Unreal_Import_Report.json")
    with open(report_path, "w") as f:
        json.dump(REPORT, f, indent=2)

    ok = not REPORT["errors"] and REPORT["physics_asset"] is not None
    print("MOTIONRIG_IMPORT_RESULT=" + ("PASS" if ok else "FAIL"))
    print(json.dumps(REPORT, indent=2))


if __name__ == "__main__":
    main()
