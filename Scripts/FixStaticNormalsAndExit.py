"""Recompute-normals on full-treatment STATIC meshes (StaticMeshEditorSubsystem was None headless;
use EditorStaticMeshLibrary instead). Writes JSON, self-quits."""
from __future__ import annotations
import csv, json, os
import unreal

OUT = r"C:/UE/T66/Saved/CombatTest/FallGuys/fix_static_normals.json"
CSV_PATH = r"C:/UE/T66/Content/Data/CharacterVisuals.csv"
KEEP = {
    "Hero_1_Chad", "Hero_1_Chad_DemoSkin", "Hero_1_Stacy", "Hero_1_Stacy_DemoSkin",
    "Companion_01", "Companion_02", "Companion_03",
    "Companion_01_DemoSkin", "Companion_02_DemoSkin", "Companion_03_DemoSkin",
    "Slime", "BoneWalker", "RatPack", "CaveBat", "HexSlinger", "TombSpider",
    "StoneSentinel", "MimicLure", "BoneConjurer", "CryptWraith", "CursedCrow", "FamishedGhoul",
    "Dungeon_SewerSlimeKing", "Dungeon_WebMatriarch", "Dungeon_BoneJailer", "Dungeon_BaelFallenChad",
    "Saint", "Ouroboros", "CasinoNPC", "Collector", "LoanShark", "VendorBoss", "VendorNPC",
}
EAL = unreal.EditorAssetLibrary
out = {"done": {}, "errors": []}

def main():
    lib = unreal.EditorStaticMeshLibrary
    with open(CSV_PATH, "r", encoding="utf-8-sig", newline="") as f:
        rows = list(csv.reader(f))
    col = {name: i for i, name in enumerate(rows[0])}
    seen = set()
    for r in rows[1:]:
        if not r or r[0] not in KEEP:
            continue
        sm_path = r[col["StaticMesh"]].split(".")[0]
        if not sm_path or not sm_path.startswith("/Game/") or "/Characters/Mobs/" in sm_path or sm_path in seen:
            continue
        seen.add(sm_path)
        mesh = EAL.load_asset(sm_path)
        if not mesh:
            out["errors"].append("load fail: " + sm_path); continue
        try:
            n = lib.get_lod_count(mesh)
            lod_count = n if n and n > 0 else 1  # -1 from the deprecated lib; every mesh has LOD0
            applied = []
            for lod in range(lod_count):
                bs = lib.get_lod_build_settings(mesh, lod)
                before = bs.get_editor_property("recompute_normals")
                bs.set_editor_property("recompute_normals", True)
                lib.set_lod_build_settings(mesh, lod, bs)
                after = lib.get_lod_build_settings(mesh, lod).get_editor_property("recompute_normals")
                applied.append(f"lod{lod}:{before}->{after}")
            EAL.save_asset(sm_path, only_if_is_dirty=False)
            out["done"][r[0]] = f"{sm_path} {' '.join(applied)}"
        except Exception as e:
            out["errors"].append(f"{r[0]}: {e}")
    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    with open(OUT, "w", encoding="utf-8") as f:
        json.dump(out, f, indent=2)
    unreal.log("[FixStaticNormals] done=%d errors=%d" % (len(out["done"]), len(out["errors"])))

try:
    main()
finally:
    try: unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")
    except Exception: pass
