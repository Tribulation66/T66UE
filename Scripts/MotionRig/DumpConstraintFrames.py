import unreal

pa = unreal.EditorAssetLibrary.load_asset("/Game/Characters/MotionRig/Hero_1/PA_MotionRig_Hero1")
if not pa:
    print("PA_FRAMES asset_missing")
else:
    templates = pa.get_editor_property("constraint_setup")
    for t in templates:
        inst = t.get_editor_property("default_instance")
        bone1 = inst.get_editor_property("constraint_bone1")
        bone2 = inst.get_editor_property("constraint_bone2")
        pos1 = inst.get_editor_property("pos1")
        pos2 = inst.get_editor_property("pos2")
        print(f"PA_FRAMES {bone1}<-{bone2} pos1=({pos1.x:.1f},{pos1.y:.1f},{pos1.z:.1f}) pos2=({pos2.x:.1f},{pos2.y:.1f},{pos2.z:.1f})")
