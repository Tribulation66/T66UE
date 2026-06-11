# Diagnostic: disable mipmaps on the MotionRig female albedo to test whether
# the limb streaks are UV-island mip bleeding (fragmented Pixal3D atlas).
import unreal

PATH = "/Game/Characters/MotionRig/Hero_1_Female/T_MotionRig_Hero1Female_BaseColor"
tex = unreal.EditorAssetLibrary.load_asset(PATH)
if not tex:
    print("NOMIPS_RESULT=FAIL not found")
else:
    tex.set_editor_property("mip_gen_settings", unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
    ok = unreal.EditorAssetLibrary.save_loaded_asset(tex)
    print(f"NOMIPS_RESULT={'PASS' if ok else 'FAIL'}")
