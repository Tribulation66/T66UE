# Copyright Tribulation 66. All Rights Reserved.
# Prints the imported MotionRig skeletal mesh bounds so unit-scale problems are
# measured, not guessed.
import unreal

mesh = unreal.EditorAssetLibrary.load_asset("/Game/Characters/MotionRig/Hero_1/SK_MotionRig_Hero1.SK_MotionRig_Hero1")
if not mesh:
    print("MOTIONRIG_SCALE: mesh missing")
else:
    bounds = mesh.get_editor_property("imported_bounds")
    extent = bounds.box_extent
    print(f"MOTIONRIG_SCALE: box_extent x={extent.x:.1f} y={extent.y:.1f} z={extent.z:.1f} (height ~= {extent.z * 2:.1f} uu; expect ~180)")
