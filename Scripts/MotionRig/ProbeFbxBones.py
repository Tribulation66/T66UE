import bpy
import sys

argv = sys.argv[sys.argv.index("--") + 1:]
fbx_path = argv[0]

bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.fbx(filepath=fbx_path)

for obj in bpy.data.objects:
    if obj.type == "ARMATURE":
        arm = obj.data
        print(f"FBX_BONES armature scale={tuple(round(s, 3) for s in obj.scale)}")
        for name in ("pelvis", "spine_01", "thigh_l", "calf_l", "head"):
            b = arm.bones.get(name)
            if b:
                print(f"FBX_BONES {name} head=({b.head_local.x:.2f},{b.head_local.y:.2f},{b.head_local.z:.2f}) len={b.length:.2f}")
