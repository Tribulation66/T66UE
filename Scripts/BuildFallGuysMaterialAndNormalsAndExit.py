"""Author M_FriendSlop_FallGuys (lit Subsurface + Fresnel sheen + exposed params), create
MI_Hero_1_Chad_FallGuys -> raw albedo, and recompute-normals on SK_Hero_1_Chad_PhysicsFirst.
Additive (new material assets); SK build-setting change is reversible. Writes JSON, self-quits."""
from __future__ import annotations
import json, os
import unreal

OUT = r"C:/UE/T66/Saved/CombatTest/build_fallguys_mat.json"
MAT_DIR = "/Game/Materials"
MAT_NAME = "M_FriendSlop_FallGuys"
MI_NAME = "MI_Hero_1_Chad_FallGuys"
ALBEDO = "/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/PhysicsFirst/Textures/T_Hero_1_Chad_PhysicsFirst_BaseColor"
SK = "/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/PhysicsFirst/SK_Hero_1_Chad_PhysicsFirst"

MEL = unreal.MaterialEditingLibrary
MP = unreal.MaterialProperty

def main():
    out = {"errors": [], "connections": {}}
    at = unreal.AssetToolsHelpers.get_asset_tools()

    # --- Author master ---
    mpath = MAT_DIR + "/" + MAT_NAME
    if unreal.EditorAssetLibrary.does_asset_exist(mpath):
        unreal.EditorAssetLibrary.delete_asset(mpath)
    mat = at.create_asset(MAT_NAME, MAT_DIR, unreal.Material, unreal.MaterialFactoryNew())
    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_SUBSURFACE)

    def expr(cls, x, y): return MEL.create_material_expression(mat, cls, x, y)
    def conn(node, out_name, prop):
        try:
            MEL.connect_material_property(node, out_name, prop); return True
        except Exception as e:
            out["errors"].append(f"connect {prop}: {e}"); return False

    tex = expr(unreal.MaterialExpressionTextureSampleParameter2D, -700, 0)
    tex.set_editor_property("parameter_name", "BaseColorTexture")
    try: tex.set_editor_property("texture", unreal.EditorAssetLibrary.load_asset(ALBEDO))
    except Exception as e: out["errors"].append("set default tex: " + str(e))

    def scalar(name, val, y):
        n = expr(unreal.MaterialExpressionScalarParameter, -700, y)
        n.set_editor_property("parameter_name", name); n.set_editor_property("default_value", val)
        return n
    p_sub = scalar("SubsurfaceIntensity", 0.20, 200)
    p_sheen = scalar("SheenIntensity", 0.35, 320)
    p_spec = scalar("Specular", 0.30, 440)
    p_rough = scalar("Roughness", 0.60, 560)

    # Fresnel sheen -> emissive rim = Fresnel(exp=4) * SheenIntensity
    fres = expr(unreal.MaterialExpressionFresnel, -700, 700)
    try: fres.set_editor_property("exponent", 4.0)
    except Exception as e: out["errors"].append("fresnel exp: " + str(e))
    mul = expr(unreal.MaterialExpressionMultiply, -350, 650)
    try:
        MEL.connect_material_expressions(fres, "", mul, "A")
        MEL.connect_material_expressions(p_sheen, "", mul, "B")
    except Exception as e:
        out["errors"].append("sheen mul wiring: " + str(e))

    out["connections"]["BaseColor"] = conn(tex, "RGB", MP.MP_BASE_COLOR)
    out["connections"]["SubsurfaceColor"] = conn(tex, "RGB", MP.MP_SUBSURFACE_COLOR)
    out["connections"]["Opacity(SSS)"] = conn(p_sub, "", MP.MP_OPACITY)
    out["connections"]["Specular"] = conn(p_spec, "", MP.MP_SPECULAR)
    out["connections"]["Roughness"] = conn(p_rough, "", MP.MP_ROUGHNESS)
    out["connections"]["Emissive(sheen)"] = conn(mul, "", MP.MP_EMISSIVE_COLOR)

    MEL.recompile_material(mat)
    unreal.EditorAssetLibrary.save_asset(mpath, only_if_is_dirty=False)
    out["master"] = {"path": mpath, "shading_model": "Subsurface",
                     "scalar_params": [str(n) for n in MEL.get_scalar_parameter_names(mat)],
                     "texture_params": [str(n) for n in MEL.get_texture_parameter_names(mat)]}

    # --- MI for Hero 1 -> raw albedo ---
    ipath = MAT_DIR + "/" + MI_NAME
    if unreal.EditorAssetLibrary.does_asset_exist(ipath):
        unreal.EditorAssetLibrary.delete_asset(ipath)
    mi = at.create_asset(MI_NAME, MAT_DIR, unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
    MEL.set_material_instance_parent(mi, mat)
    alb = unreal.EditorAssetLibrary.load_asset(ALBEDO)
    if alb: MEL.set_material_instance_texture_parameter_value(mi, "BaseColorTexture", alb)
    else: out["errors"].append("albedo not loadable: " + ALBEDO)
    unreal.EditorAssetLibrary.save_asset(ipath, only_if_is_dirty=False)
    out["instance"] = {"path": ipath, "albedo_set": bool(alb)}

    # --- Recompute normals on SK (reversible build setting) ---
    sk = unreal.EditorAssetLibrary.load_asset(SK)
    sub = unreal.get_editor_subsystem(unreal.SkeletalMeshEditorSubsystem)
    rn = {"sk": SK}
    try:
        n_lods = sub.get_lod_count(sk); rn["lod_count"] = n_lods
        for lod in range(n_lods):
            bs = sub.get_lod_build_settings(sk, lod)
            rn[f"lod{lod}_before"] = bs.get_editor_property("recompute_normals")
            bs.set_editor_property("recompute_normals", True)
            sub.set_lod_build_settings(sk, lod, bs)
            bs2 = sub.get_lod_build_settings(sk, lod)
            rn[f"lod{lod}_after"] = bs2.get_editor_property("recompute_normals")
        unreal.EditorAssetLibrary.save_asset(SK, only_if_is_dirty=False)
        rn["saved"] = True
        rn["applied"] = all(rn.get(f"lod{l}_after") for l in range(n_lods))
    except Exception as e:
        rn["error"] = str(e); rn["applied"] = False
    out["recompute_normals"] = rn

    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    with open(OUT, "w", encoding="utf-8") as f: json.dump(out, f, indent=2)
    unreal.log("[BuildFallGuysMat] RESULT " + json.dumps(out))

try:
    main()
finally:
    try: unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")
    except Exception: pass
