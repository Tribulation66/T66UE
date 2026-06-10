# Copyright Tribulation 66. All Rights Reserved.
#
# Probe an FBX for baked animation: lists actions/curves and samples the
# pelvis local location at the first frame so unit conversion can be judged.
#
#   blender.exe --background --factory-startup --python ProbeFbxAnim.py -- <file.fbx>

import bpy
import sys

argv = sys.argv[sys.argv.index("--") + 1:]
fbx_path = argv[0]

bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.fbx(filepath=fbx_path)

print(f"FBX_ANIM actions={len(bpy.data.actions)}")
for action in bpy.data.actions:
    fcurves = []
    if hasattr(action, "fcurves"):
        fcurves = list(action.fcurves)
    else:
        for layer in action.layers:
            for strip in layer.strips:
                for channelbag in strip.channelbags:
                    fcurves.extend(channelbag.fcurves)
    frames = sorted({k.co.x for fc in fcurves for k in fc.keyframe_points})
    print(f"FBX_ANIM action={action.name} fcurves={len(fcurves)} frame_range={frames[:3]}..{frames[-3:] if frames else []}")
    for fc in fcurves:
        if "pelvis" in fc.data_path and fc.data_path.endswith("location"):
            keys = [(round(k.co.x, 2), round(k.co.y, 4)) for k in fc.keyframe_points[:3]]
            print(f"FBX_ANIM   pelvis loc[{fc.array_index}] keys={keys}")
