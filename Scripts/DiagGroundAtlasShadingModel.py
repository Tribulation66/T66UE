"""
Diagnostic: check and report the shading model of all 4 ground atlas materials.
Run in the UE editor Python console:  py "C:/UE/T66/Scripts/DiagGroundAtlasShadingModel.py"
"""
import unreal

MATS = [
    "/Game/World/Ground/M_GroundAtlas_2x2_R0",
    "/Game/World/Ground/M_GroundAtlas_2x2_R90",
    "/Game/World/Ground/M_GroundAtlas_2x2_R180",
    "/Game/World/Ground/M_GroundAtlas_2x2_R270",
]

for path in MATS:
    mat = unreal.EditorAssetLibrary.load_asset(path)
    if not mat:
        unreal.log_error(f"[DIAG] Could not load {path}")
        continue
    name = path.rsplit("/", 1)[-1]
    try:
        sm = mat.get_editor_property("shading_model")
        unreal.log(f"[DIAG] {name}  shading_model = {sm}")
    except Exception as e:
        unreal.log_warning(f"[DIAG] {name}  shading_model read failed: {e}")
    try:
        ts = mat.get_editor_property("two_sided")
        unreal.log(f"[DIAG] {name}  two_sided = {ts}")
    except Exception as e:
        unreal.log_warning(f"[DIAG] {name}  two_sided read failed: {e}")

    # Also try to list all properties containing "shading" or "lit"
    try:
        for prop_name in ["shading_model", "shading_models", "material_domain"]:
            try:
                val = mat.get_editor_property(prop_name)
                unreal.log(f"[DIAG] {name}  {prop_name} = {val}")
            except Exception:
                pass
    except Exception:
        pass

unreal.log("[DIAG] === DONE ===")
