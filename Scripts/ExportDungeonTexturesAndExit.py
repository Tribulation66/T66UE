"""Export the dungeon kit floor/wall BaseColor textures (.uasset) to PNG for Blender. Self-quits."""
from __future__ import annotations
import json, os
import unreal

KIT = "/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/"
ASSETS = [
    "DungeonFloor_StoneSlabs_A_UnrealReady_BaseColor_00",
    "DungeonFloor_Bones_A_UnrealReady_BaseColor_00",
    "DungeonFloor_Cracked_A_UnrealReady_BaseColor_00",
    "DungeonFloor_Drain_A_UnrealReady_BaseColor_00",
    "DungeonWall_StoneBlocks_A_UnrealReady_BaseColor_00",
    "DungeonWall_BonesNiche_A_UnrealReady_BaseColor_00",
    "DungeonWall_Chains_A_UnrealReady_BaseColor_00",
    "DungeonWall_TorchSconce_A_UnrealReady_BaseColor_00",
]
OUT_DIR = r"C:/UE/T66/Model Generation/Runs/Pixal3D/HeroChadStacy_SourceAssets_20260609_0536/Blender/DungeonTex"

def main() -> int:
    os.makedirs(OUT_DIR, exist_ok=True)
    out = {"out_dir": OUT_DIR, "exported": [], "errors": []}
    for name in ASSETS:
        apath = KIT + name
        tex = unreal.EditorAssetLibrary.load_asset(apath)
        if not tex:
            out["errors"].append(f"missing asset: {apath}")
            continue
        png = os.path.join(OUT_DIR, name + ".png")
        try:
            task = unreal.AssetExportTask()
            task.set_editor_property("object", tex)
            task.set_editor_property("filename", png)
            task.set_editor_property("exporter", unreal.TextureExporterPNG())
            task.set_editor_property("automated", True)
            task.set_editor_property("prompt", False)
            task.set_editor_property("replace_identical", True)
            ok = unreal.Exporter.run_asset_export_task(task)
            sz = [0, 0]
            try:
                sz = [tex.blueprint_get_size_x(), tex.blueprint_get_size_y()]
            except Exception:
                try:
                    sz = [tex.get_editor_property("imported_size").x, tex.get_editor_property("imported_size").y]
                except Exception:
                    pass
            out["exported"].append({"asset": apath, "png": png, "ok": bool(ok),
                                     "exists": os.path.isfile(png),
                                     "size_kb": round(os.path.getsize(png)/1024, 1) if os.path.isfile(png) else 0,
                                     "dims": sz})
        except Exception as exc:
            out["errors"].append(f"{name}: {exc}")
    report = os.path.join(OUT_DIR, "_export_report.json")
    with open(report, "w", encoding="utf-8") as f:
        json.dump(out, f, indent=2)
    unreal.log(f"[ExportDungeonTextures] RESULT {json.dumps(out)}")
    return 0

try:
    main()
finally:
    try:
        unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")
    except Exception:
        pass
