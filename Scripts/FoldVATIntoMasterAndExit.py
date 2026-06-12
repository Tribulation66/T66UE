"""Fold the mob VAT path into M_FriendSlop_FallGuys behind a StaticSwitch (bUseVAT, default off).
WPO recipe copied EXACTLY from Model Generation/.../import_easy_mob_vat_to_unreal.py (Custom HLSL:
frame-row UV offset into PositionTexture via UV channel 2, bbox decode, TransformLocalVectorToWorld).
Then migrate MI_EasyMobVAT_* onto the master: parent swap, bUseVAT=true, two-sided override, and
recompute-normals on the VAT meshes. Old VAT master deletion is a SEPARATE gated step (after visual
verification). Writes JSON, self-quits."""
from __future__ import annotations
import json, os
import unreal

OUT = r"C:/UE/T66/Saved/CombatTest/FallGuys/fold_vat.json"
MASTER = "/Game/Materials/M_FriendSlop_FallGuys"
MEL = unreal.MaterialEditingLibrary
EAL = unreal.EditorAssetLibrary
out = {"mis": [], "normals": {}, "errors": []}

VAT_HLSL = "\n".join([
    "uint tex_width;",
    "uint tex_height;",
    "PositionTexture.GetDimensions(tex_width, tex_height);",
    "float frame_index = floor(Frame + 0.0001);",
    "float2 sample_uv = UV1;",
    "sample_uv.y += (frame_index * RowsPerFrame) / max(1.0, (float)tex_height);",
    "float3 packed_delta = Texture2DSample(PositionTexture, PositionTextureSampler, sample_uv).rgb;",
    "float3 local_delta = packed_delta * SizeBBox.rgb + MinBBox.rgb;",
    "return TransformLocalVectorToWorld(Parameters, local_delta);",
])

def extend_master():
    m = EAL.load_asset(MASTER)
    if not m:
        out["errors"].append("master not loadable"); return False
    def expr(cls, x, y):
        return MEL.create_material_expression(m, cls, x, y)

    uv2 = expr(unreal.MaterialExpressionTextureCoordinate, -1500, 800)
    uv2.set_editor_property("coordinate_index", 2)
    pos_tex = expr(unreal.MaterialExpressionTextureObjectParameter, -1500, 950)
    pos_tex.set_editor_property("parameter_name", "PositionTexture")
    try:
        pos_tex.set_editor_property("texture", EAL.load_asset("/Game/Characters/MobsVAT/Slime/TX_EasyMobVAT_Slime_Position"))
    except Exception as e:
        out["errors"].append("pos tex default: " + str(e))
    frame = expr(unreal.MaterialExpressionScalarParameter, -1500, 1100)
    frame.set_editor_property("parameter_name", "Frame")
    frame.set_editor_property("default_value", 0.0)
    rows = expr(unreal.MaterialExpressionScalarParameter, -1500, 1250)
    rows.set_editor_property("parameter_name", "RowsPerFrame")
    rows.set_editor_property("default_value", 1.0)
    min_bbox = expr(unreal.MaterialExpressionVectorParameter, -1500, 1400)
    min_bbox.set_editor_property("parameter_name", "MinBBox")
    min_bbox.set_editor_property("default_value", unreal.LinearColor(0.0, 0.0, 0.0, 0.0))
    size_bbox = expr(unreal.MaterialExpressionVectorParameter, -1500, 1550)
    size_bbox.set_editor_property("parameter_name", "SizeBBox")
    size_bbox.set_editor_property("default_value", unreal.LinearColor(1.0, 1.0, 1.0, 0.0))

    custom = expr(unreal.MaterialExpressionCustom, -1000, 1100)
    custom.set_editor_property("description", "EasyMobVAT_WPO")
    custom.set_editor_property("output_type", unreal.CustomMaterialOutputType.CMOT_FLOAT3)
    custom.set_editor_property("code", VAT_HLSL)
    inputs = []
    for name in ("UV1", "PositionTexture", "Frame", "RowsPerFrame", "MinBBox", "SizeBBox"):
        ci = unreal.CustomInput()
        ci.set_editor_property("input_name", name)
        inputs.append(ci)
    custom.set_editor_property("inputs", inputs)

    zero = expr(unreal.MaterialExpressionConstant3Vector, -1000, 1400)
    zero.set_editor_property("constant", unreal.LinearColor(0.0, 0.0, 0.0, 0.0))
    switch = expr(unreal.MaterialExpressionStaticSwitchParameter, -700, 1200)
    switch.set_editor_property("parameter_name", "bUseVAT")
    switch.set_editor_property("default_value", False)

    try:
        MEL.connect_material_expressions(uv2, "", custom, "UV1")
        MEL.connect_material_expressions(pos_tex, "", custom, "PositionTexture")
        MEL.connect_material_expressions(frame, "", custom, "Frame")
        MEL.connect_material_expressions(rows, "", custom, "RowsPerFrame")
        MEL.connect_material_expressions(min_bbox, "", custom, "MinBBox")
        MEL.connect_material_expressions(size_bbox, "", custom, "SizeBBox")
        MEL.connect_material_expressions(custom, "", switch, "True")
        MEL.connect_material_expressions(zero, "", switch, "False")
        MEL.connect_material_property(switch, "", unreal.MaterialProperty.MP_WORLD_POSITION_OFFSET)
        out["master_vat_wired"] = True
    except Exception as e:
        out["errors"].append("master wiring: " + str(e)); out["master_vat_wired"] = False
        return False
    MEL.recompile_material(m)
    out["master_saved"] = EAL.save_asset(MASTER, only_if_is_dirty=False)
    return bool(out["master_saved"])

def migrate_mis():
    master = EAL.load_asset(MASTER)
    has_switch_api = hasattr(MEL, "set_material_instance_static_switch_parameter_value")
    out["switch_api"] = has_switch_api
    ar = unreal.AssetRegistryHelpers.get_asset_registry()
    f = unreal.ARFilter(class_paths=[unreal.TopLevelAssetPath("/Script/Engine", "MaterialInstanceConstant")],
                        package_paths=["/Game/Characters/MobsVAT"], recursive_paths=True)
    for ad in ar.get_assets(f):
        pkg = str(ad.package_name)
        mi = ad.get_asset()
        if not mi:
            continue
        try:
            MEL.set_material_instance_parent(mi, master)
            ok_switch = False
            if has_switch_api:
                ok_switch = MEL.set_material_instance_static_switch_parameter_value(mi, "bUseVAT", True)
            # two-sided like the old VAT master
            try:
                bpo = mi.get_editor_property("base_property_overrides")
                bpo.set_editor_property("override_two_sided", True)
                bpo.set_editor_property("two_sided", True)
                mi.set_editor_property("base_property_overrides", bpo)
            except Exception as e:
                out["errors"].append(f"two-sided {pkg}: {e}")
            MEL.update_material_instance(mi)
            EAL.save_asset(pkg, only_if_is_dirty=False)
            out["mis"].append({"mi": pkg, "switch": bool(ok_switch)})
        except Exception as e:
            out["errors"].append(f"mi {pkg}: {e}")

def vat_mesh_normals():
    lib = unreal.EditorStaticMeshLibrary
    ar = unreal.AssetRegistryHelpers.get_asset_registry()
    f = unreal.ARFilter(class_paths=[unreal.TopLevelAssetPath("/Script/Engine", "StaticMesh")],
                        package_paths=["/Game/Characters/MobsVAT"], recursive_paths=True)
    for ad in ar.get_assets(f):
        pkg = str(ad.package_name)
        mesh = ad.get_asset()
        if not mesh:
            continue
        try:
            bs = lib.get_lod_build_settings(mesh, 0)
            before = bs.get_editor_property("recompute_normals")
            bs.set_editor_property("recompute_normals", True)
            lib.set_lod_build_settings(mesh, 0, bs)
            after = lib.get_lod_build_settings(mesh, 0).get_editor_property("recompute_normals")
            EAL.save_asset(pkg, only_if_is_dirty=False)
            out["normals"][pkg.split("/")[-1]] = f"{before}->{after}"
        except Exception as e:
            out["errors"].append(f"normals {pkg}: {e}")

try:
    if extend_master():
        migrate_mis()
        vat_mesh_normals()
    out["counts"] = {"mis": len(out["mis"]), "normals": len(out["normals"]), "errors": len(out["errors"])}
finally:
    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    with open(OUT, "w", encoding="utf-8") as fjson:
        json.dump(out, fjson, indent=2)
    unreal.log("[FoldVAT] " + json.dumps(out.get("counts", {})))
    try: unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")
    except Exception: pass
