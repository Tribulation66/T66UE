# Copyright Tribulation 66. All Rights Reserved.
#
# MotionRig UE import (MOTION_RIG.md section 3).
# Imports the Blender-built skeletal meshes, base-color textures, and the six
# pose-target clips for BOTH Hero 1 body types (Chad = male, Stacy = female)
# into /Game/Characters/MotionRig/Hero_1_Male and Hero_1_Female.
#
# Run (must be -ExecutePythonScript; -run=pythonscript crashes in AssetTools):
#   "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" ^
#       "C:\UE\T66\T66.uproject" ^
#       -ExecutePythonScript="C:/UE/T66/Scripts/MotionRig/ImportMotionRig.py" ^
#       -unattended -nop4 -nosplash -stdout

import json
import os

import unreal

CLIPS = ["Idle", "Walk", "Jump", "Dive", "GetUp_Front", "GetUp_Back"]

CHARACTERS = [
    {  # ET66BodyType::Chad — simple-clothing physics model (hanging pose)
        "name": "Hero1Male",
        "source": r"C:\UE\T66\Model Generation\Runs\Pixal3D\HeroChadStacy_SourceAssets_20260609_0536\Blender\MotionRig\Male",
        "dest": "/Game/Characters/MotionRig/Hero_1_Male",
    },
    {  # ET66BodyType::Stacy — T-pose inflatable Variation 1 (designed ref v5)
        "name": "Hero1Female",
        "source": r"C:\UE\T66\Model Generation\Runs\Pixal3D\StacyTPoseVar1_20260611\Blender\MotionRig\Female",
        "dest": "/Game/Characters/MotionRig/Hero_1_Female",
    },
]

# The import report lands next to the FIRST character's sources.
SOURCE_RUN = os.path.dirname(CHARACTERS[0]["source"])

REPORT = {"characters": {}, "errors": []}


def make_skeletal_mesh_options():
    options = unreal.FbxImportUI()
    options.import_mesh = True
    options.import_textures = False
    options.import_materials = False
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
    # The skeletal FBX carries a baked bind-pose animation in real cm (the
    # data bake in BuildMotionRig.py — the exporter itself converts nothing).
    # T0-as-ref-pose rebuilds the reference skeleton AND the render bind from
    # that cm data at import — the only point where the bind-dependent LOD
    # render caches are built. Post-import ref surgery cannot fix rendering.
    smd.set_editor_property("use_t0_as_ref_pose", True)
    smd.set_editor_property("preserve_smoothing_groups", True)
    smd.set_editor_property("import_meshes_in_bone_hierarchy", True)
    # IMPORT the Blender-authored normals (shade-smooth + weighted normals,
    # the FallGuys look-dev recipe). Recomputing here from the decimated
    # Pixal3D geometry bands the glossy sheen along the triangulation.
    smd.set_editor_property("normal_import_method", unreal.FBXNormalImportMethod.FBXNIM_IMPORT_NORMALS)
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


def run_task(filename, destination_path, destination_name, options):
    task = unreal.AssetImportTask()
    task.filename = filename
    task.destination_path = destination_path
    task.destination_name = destination_name
    task.automated = True
    task.replace_existing = True
    task.save = True
    task.options = options
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    return list(task.imported_object_paths)


def import_character(char):
    name = char["name"]
    source = char["source"]
    dest = char["dest"]
    sk_name = f"SK_MotionRig_{name}"
    result = {"skeletal_mesh": None, "skeleton": None, "texture": None, "clips": {}}

    sk_fbx = os.path.join(source, f"{sk_name}.fbx")
    if not os.path.exists(sk_fbx):
        REPORT["errors"].append(f"{name}: missing skeletal FBX: {sk_fbx}")
        return result
    result["skeletal_mesh"] = run_task(sk_fbx, dest, sk_name, make_skeletal_mesh_options())

    # One albedo per material slot (split-generation models carry one atlas
    # per part); legacy single-texture models use the un-suffixed name.
    import glob as glob_module
    tex_pngs = sorted(glob_module.glob(os.path.join(source, f"T_MotionRig_{name}_BaseColor*.png")))
    if tex_pngs:
        result["texture"] = []
        for tex_png in tex_pngs:
            tex_name = os.path.splitext(os.path.basename(tex_png))[0]
            # Plain texture import: no options object needed, defaults are right.
            result["texture"] += run_task(tex_png, dest, tex_name, None)
    else:
        REPORT["errors"].append(f"{name}: no base color pngs in {source}")

    sk_path = f"{dest}/{sk_name}.{sk_name}"
    skeletal_mesh = unreal.EditorAssetLibrary.load_asset(sk_path)
    if not skeletal_mesh:
        REPORT["errors"].append(f"{name}: skeletal mesh did not import: {sk_path}")
        return result

    skeleton = skeletal_mesh.get_editor_property("skeleton")
    result["skeleton"] = skeleton.get_path_name() if skeleton else None

    for clip in CLIPS:
        clip_fbx = os.path.join(source, "AnimationSources", f"AM_MotionRig_{name}_{clip}.fbx")
        if not os.path.exists(clip_fbx):
            REPORT["errors"].append(f"{name}: missing clip FBX: {clip_fbx}")
            continue
        result["clips"][clip] = run_task(clip_fbx, dest, f"AM_MotionRig_{name}_{clip}", make_anim_options(skeleton))

    unreal.EditorAssetLibrary.save_directory(dest, only_if_is_dirty=False, recursive=True)
    return result


def main():
    # UE 5.7 routes FBX through Interchange by default, which IGNORES the
    # legacy FbxImportUI options on the task — measured: use_t0_as_ref_pose
    # had no effect (ref pose stayed meter-scale) until Interchange was
    # disabled for FBX. The legacy importer honors every option above.
    unreal.SystemLibrary.execute_console_command(None, "Interchange.FeatureFlags.Import.FBX 0")

    for char in CHARACTERS:
        REPORT["characters"][char["name"]] = import_character(char)

    report_path = os.path.join(SOURCE_RUN, "Unreal_Import_Report.json")
    with open(report_path, "w") as f:
        json.dump(REPORT, f, indent=2)

    ok = not REPORT["errors"] and all(
        r["skeletal_mesh"] and r["texture"] and len(r["clips"]) == len(CLIPS)
        for r in REPORT["characters"].values())
    print("MOTIONRIG_IMPORT_RESULT=" + ("PASS" if ok else "FAIL"))
    print(json.dumps(REPORT, indent=2))


if __name__ == "__main__":
    main()
