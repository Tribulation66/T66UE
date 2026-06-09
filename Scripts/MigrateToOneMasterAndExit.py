"""ONE-MASTER MIGRATION (asset side). Does, in order:
1. CharacterVisuals.csv: repoint non-EASY / non-first-3 rows at the shared engine capsule
   (placeholders), keep full-treatment rows; refill + save DT_CharacterVisuals.
2. M_FriendSlop_FallGuys master: add Tint (vector, white) x Brightness (scalar, 1.0) into
   BaseColor (identity for existing users), kill emissive with a black constant, set usage
   flags (skeletal + instanced static meshes), recompile, save.
3. Reparent environment theme MIs (ToonStyle Environment + TowerForestGround/DungeonRoof)
   onto the master (texture/Brightness/Tint overrides carry by param name).
4. Recompute-normals build setting on full-treatment static meshes (companions, EASY bosses,
   NPCs) + Hero_1 Stacy skeletal if present.
Writes JSON report, self-quits."""
from __future__ import annotations
import csv, json, os, shutil
import unreal

OUT = r"C:/UE/T66/Saved/CombatTest/FallGuys/migrate_one_master.json"
CSV_PATH = r"C:/UE/T66/Content/Data/CharacterVisuals.csv"
DT_PATH = "/Game/Data/DT_CharacterVisuals"
MASTER = "/Game/Materials/M_FriendSlop_FallGuys"
PLACEHOLDER_MESH = "/Engine/BasicShapes/Capsule.Capsule"

KEEP = {
    "Hero_1_Chad", "Hero_1_Chad_DemoSkin", "Hero_1_Stacy", "Hero_1_Stacy_DemoSkin",
    "Companion_01", "Companion_02", "Companion_03",
    "Companion_01_DemoSkin", "Companion_02_DemoSkin", "Companion_03_DemoSkin",
    "Slime", "BoneWalker", "RatPack", "CaveBat", "HexSlinger", "TombSpider",
    "StoneSentinel", "MimicLure", "BoneConjurer", "CryptWraith", "CursedCrow", "FamishedGhoul",
    "Dungeon_SewerSlimeKing", "Dungeon_WebMatriarch", "Dungeon_BoneJailer", "Dungeon_BaelFallenChad",
    "Saint", "Ouroboros", "CasinoNPC", "Collector", "LoanShark", "VendorBoss", "VendorNPC",
}

MEL = unreal.MaterialEditingLibrary
MP = unreal.MaterialProperty
EAL = unreal.EditorAssetLibrary

out = {"placeholders": [], "kept": [], "reparented": [], "recompute_normals": {}, "errors": []}

def step_csv_and_datatable():
    shutil.copyfile(CSV_PATH, CSV_PATH + ".pre_one_master.bak")
    with open(CSV_PATH, "r", encoding="utf-8-sig", newline="") as f:
        rows = list(csv.reader(f))
    header = rows[0]
    col = {name: i for i, name in enumerate(header)}
    clear_cols = ["SkeletalMesh", "OutlineStaticMesh", "PixelatedTextureAssetPath",
                  "WalkAnimation", "IdleAnimation", "JumpAnimation", "LeapAnimation"]
    for r in rows[1:]:
        if not r or not r[0]:
            continue
        name = r[0]
        if name in KEEP:
            out["kept"].append(name)
            continue
        for c in clear_cols:
            r[col[c]] = ""
        r[col["StaticMesh"]] = PLACEHOLDER_MESH
        r[col["MeshRelativeLocation"]] = "(X=0,Y=0,Z=0)"
        r[col["MeshRelativeRotation"]] = "(P=0,Y=0,R=0)"
        r[col["MeshRelativeScale"]] = "(X=1,Y=1,Z=1.8)"
        r[col["bLoopAnimation"]] = "False"
        r[col["bAutoGroundToActorOrigin"]] = "True"
        out["placeholders"].append(name)
    with open(CSV_PATH, "w", encoding="utf-8", newline="") as f:
        csv.writer(f).writerows(rows)

    dt = EAL.load_asset(DT_PATH)
    if not dt:
        out["errors"].append("DT_CharacterVisuals not loadable"); return
    ok = unreal.DataTableFunctionLibrary.fill_data_table_from_csv_file(dt, CSV_PATH)
    out["dt_refilled"] = bool(ok)
    out["dt_saved"] = EAL.save_asset(DT_PATH, only_if_is_dirty=False)

def step_master():
    m = EAL.load_asset(MASTER)
    if not m:
        out["errors"].append("master not loadable"); return
    def expr(cls, x, y):
        return MEL.create_material_expression(m, cls, x, y)
    # New BaseColor chain: BaseColorTexture (same param NAME -> shared value) x Tint x Brightness.
    tex = expr(unreal.MaterialExpressionTextureSampleParameter2D, -1100, -400)
    tex.set_editor_property("parameter_name", "BaseColorTexture")
    try:
        alb = EAL.load_asset("/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/PhysicsFirst/Textures/T_Hero_1_Chad_PhysicsFirst_BaseColor")
        if alb:
            tex.set_editor_property("texture", alb)
    except Exception as e:
        out["errors"].append("master default tex: " + str(e))
    tint = expr(unreal.MaterialExpressionVectorParameter, -1100, -200)
    tint.set_editor_property("parameter_name", "Tint")
    tint.set_editor_property("default_value", unreal.LinearColor(1.0, 1.0, 1.0, 1.0))
    mask = expr(unreal.MaterialExpressionComponentMask, -900, -200)
    mask.set_editor_property("r", True); mask.set_editor_property("g", True)
    mask.set_editor_property("b", True); mask.set_editor_property("a", False)
    bright = expr(unreal.MaterialExpressionScalarParameter, -1100, -60)
    bright.set_editor_property("parameter_name", "Brightness")
    bright.set_editor_property("default_value", 1.0)
    mul_a = expr(unreal.MaterialExpressionMultiply, -700, -320)
    mul_b = expr(unreal.MaterialExpressionMultiply, -520, -300)
    black = expr(unreal.MaterialExpressionConstant3Vector, -700, 100)
    black.set_editor_property("constant", unreal.LinearColor(0.0, 0.0, 0.0, 0.0))
    try:
        MEL.connect_material_expressions(tint, "", mask, "")
        MEL.connect_material_expressions(tex, "RGB", mul_a, "A")
        MEL.connect_material_expressions(mask, "", mul_a, "B")
        MEL.connect_material_expressions(mul_a, "", mul_b, "A")
        MEL.connect_material_expressions(bright, "", mul_b, "B")
        MEL.connect_material_property(mul_b, "", MP.MP_BASE_COLOR)
        MEL.connect_material_property(black, "", MP.MP_EMISSIVE_COLOR)  # sheen rim dead for ALL users
        out["master_chain"] = True
    except Exception as e:
        out["errors"].append("master wiring: " + str(e)); out["master_chain"] = False
    m.set_editor_property("used_with_skeletal_mesh", True)
    m.set_editor_property("used_with_instanced_static_meshes", True)
    MEL.recompile_material(m)
    out["master_saved"] = EAL.save_asset(MASTER, only_if_is_dirty=False)
    out["master_flags"] = {
        "skeletal": m.get_editor_property("used_with_skeletal_mesh"),
        "instanced": m.get_editor_property("used_with_instanced_static_meshes"),
    }

def step_reparent():
    master = EAL.load_asset(MASTER)
    if not master:
        return
    targets = []
    for root in ("/Game/ToonStyle/Environment", "/Game/World/Terrain"):
        try:
            for p in EAL.list_assets(root, recursive=True, include_folder=False):
                short = p.split("/")[-1].split(".")[0]
                if short.startswith("MI_") and (root.endswith("Environment")
                        or short in ("MI_TowerForestGround", "MI_TowerDungeonRoof")):
                    targets.append(p.split(".")[0])
        except Exception as e:
            out["errors"].append(f"list {root}: {e}")
    for path in sorted(set(targets)):
        mi = EAL.load_asset(path)
        if isinstance(mi, unreal.MaterialInstanceConstant):
            try:
                MEL.set_material_instance_parent(mi, master)
                EAL.save_asset(path, only_if_is_dirty=False)
                out["reparented"].append(path)
            except Exception as e:
                out["errors"].append(f"reparent {path}: {e}")

def step_normals():
    # Full-treatment static meshes from the kept CSV rows (skip engine + mob quads), + Hero_1 SKs.
    sm_sub = unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)
    sk_sub = unreal.get_editor_subsystem(unreal.SkeletalMeshEditorSubsystem)
    with open(CSV_PATH, "r", encoding="utf-8-sig", newline="") as f:
        rows = list(csv.reader(f))
    col = {name: i for i, name in enumerate(rows[0])}
    for r in rows[1:]:
        if not r or r[0] not in KEEP:
            continue
        name = r[0]
        sm_path = r[col["StaticMesh"]].split(".")[0]
        sk_path = r[col["SkeletalMesh"]].split(".")[0]
        if sm_path and sm_path.startswith("/Game/") and "/Characters/Mobs/" not in sm_path:
            mesh = EAL.load_asset(sm_path)
            if mesh:
                try:
                    n = sm_sub.get_lod_count(mesh)
                    for lod in range(n):
                        bs = sm_sub.get_lod_build_settings(mesh, lod)
                        bs.set_editor_property("recompute_normals", True)
                        sm_sub.set_lod_build_settings(mesh, lod, bs)
                    EAL.save_asset(sm_path, only_if_is_dirty=False)
                    out["recompute_normals"][name] = f"static lods={n}"
                except Exception as e:
                    out["errors"].append(f"normals {name}: {e}")
        if sk_path and sk_path.startswith("/Game/") and "Hero_1_Chad_PhysicsFirst" not in sk_path:
            mesh = EAL.load_asset(sk_path)
            if mesh:
                try:
                    n = sk_sub.get_lod_count(mesh)
                    for lod in range(n):
                        bs = sk_sub.get_lod_build_settings(mesh, lod)
                        bs.set_editor_property("recompute_normals", True)
                        sk_sub.set_lod_build_settings(mesh, lod, bs)
                    EAL.save_asset(sk_path, only_if_is_dirty=False)
                    out["recompute_normals"][name] = f"skeletal lods={n}"
                except Exception as e:
                    out["errors"].append(f"sk normals {name}: {e}")

def main():
    step_csv_and_datatable()
    step_master()
    step_reparent()
    step_normals()
    out["counts"] = {"kept": len(out["kept"]), "placeholders": len(out["placeholders"]),
                     "reparented": len(out["reparented"]), "normals": len(out["recompute_normals"])}
    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    with open(OUT, "w", encoding="utf-8") as f:
        json.dump(out, f, indent=2)
    unreal.log("[MigrateOneMaster] RESULT " + json.dumps(out["counts"]) + " errors=" + str(len(out["errors"])))

try:
    main()
finally:
    try:
        unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")
    except Exception:
        pass
