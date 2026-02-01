"""
Import world models (static + a couple NPC skeletal meshes) from SourceAssets/Models/*.zip.

This script is intended to be run INSIDE the Unreal Editor (not commandlet) via:
  Tools -> Execute Python Script... -> Scripts/ImportWorldModels.py
or from Output Log:
  py "C:/UE/T66/Scripts/ImportWorldModels.py"

What it does:
- Extracts each .zip under SourceAssets/Models/ into SourceAssets/Models/Extracted/<zip-name>/
- Finds the primary FBX in each extracted folder
- Imports to stable /Game paths & names that the C++ code expects
  - Loot bags:
    /Game/World/LootBags/SM_LootBag_<Color>
  - Gates:
    /Game/World/Gates/SM_StageGate
    /Game/World/Gates/SM_CowardiceGate
  - Interactables:
    /Game/World/Interactables/SM_IdolAltar
    /Game/World/Interactables/SM_DifficultyTotem
    /Game/World/Interactables/Trees/SM_Tree_<Color>
    /Game/World/Interactables/Trucks/SM_CashTruck_<Color>
    /Game/World/Interactables/Wheels/SM_Wheel_<Color>
  - NPCs (skeletal):
    /Game/Characters/NPCs/Trickster/Trickster/SK_Trickster
    /Game/Characters/NPCs/Ouroboros/Ouroboros/SK_Ouroboros

After running, also reimport CSVs:
  py "C:/UE/T66/Scripts/ImportData.py"
  py "C:/UE/T66/Scripts/SetupCharacterVisualsDataTable.py"
"""

import os
import re
import zipfile
import unreal


def _proj_dir() -> str:
    try:
        return unreal.SystemLibrary.get_project_directory().replace("\\", "/")
    except Exception:
        return os.path.abspath(os.path.join(os.getcwd(), "..")).replace("\\", "/")


def _ensure_game_dir(game_path: str):
    if not unreal.EditorAssetLibrary.does_directory_exist(game_path):
        unreal.EditorAssetLibrary.make_directory(game_path)


def _extract_zip(zip_path: str, out_dir: str):
    os.makedirs(out_dir, exist_ok=True)
    with zipfile.ZipFile(zip_path, "r") as z:
        z.extractall(out_dir)


def _find_fbx(root_dir: str) -> str | None:
    best = None
    for dirpath, _, files in os.walk(root_dir):
        for fn in files:
            if not fn.lower().endswith(".fbx"):
                continue
            p = os.path.join(dirpath, fn)
            name = fn.lower()
            score = 0
            if "character_output" in name:
                score += 1000
            if "texture" in name:
                score += 100
            if "animation" in name:
                score -= 500
            if score > (best[0] if best else -999999):
                best = (score, p)
    return best[1] if best else None


def _import_fbx_static(fbx_path: str, dest_dir: str, dest_name: str) -> list[str]:
    _ensure_game_dir(dest_dir)
    task = unreal.AssetImportTask()
    task.automated = True
    task.save = True
    task.replace_existing = True
    task.replace_existing_settings = True
    task.filename = fbx_path
    task.destination_path = dest_dir
    task.destination_name = dest_name

    opts = unreal.FbxImportUI()
    opts.import_mesh = True
    opts.import_textures = True
    opts.import_materials = True
    opts.import_as_skeletal = False
    opts.import_animations = False
    task.options = opts

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    return list(task.imported_object_paths or [])


def _import_fbx_skeletal(fbx_path: str, dest_dir: str, dest_name: str) -> list[str]:
    _ensure_game_dir(dest_dir)
    task = unreal.AssetImportTask()
    task.automated = True
    task.save = True
    task.replace_existing = True
    task.replace_existing_settings = True
    task.filename = fbx_path
    task.destination_path = dest_dir
    task.destination_name = dest_name

    opts = unreal.FbxImportUI()
    opts.import_mesh = True
    opts.import_textures = True
    opts.import_materials = True
    opts.import_as_skeletal = True
    opts.import_animations = True
    try:
        opts.set_editor_property("create_physics_asset", False)
    except Exception:
        pass
    task.options = opts

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    return list(task.imported_object_paths or [])


def _color_from_name(s: str) -> str | None:
    s = s.lower()
    # Some of the provided zip filenames truncate color tokens (e.g. "blac", "whit", "yell").
    if "black" in s or re.search(r"\bblac\b", s) or "blac_" in s or "_blac" in s:
        return "Black"
    if "red" in s:
        return "Red"
    if "yellow" in s or re.search(r"\byell\b", s) or "yell_" in s or "_yell" in s:
        return "Yellow"
    if "white" in s or re.search(r"\bwhit\b", s) or "whit_" in s or "_whit" in s:
        return "White"
    return None


def main():
    proj = _proj_dir()
    src_models = os.path.join(proj, "SourceAssets", "Models")
    extracted_root = os.path.join(src_models, "Extracted")

    # Some UE Python builds don't expose SystemLibrary.is_running_commandlet; guard safely.
    try:
        is_cmdlet_fn = getattr(unreal.SystemLibrary, "is_running_commandlet", None)
        if callable(is_cmdlet_fn) and bool(is_cmdlet_fn()):
            unreal.log_error("ImportWorldModels must be run inside the full Editor (not commandlet).")
            return
    except Exception:
        pass

    unreal.log("=== ImportWorldModels: START ===")
    unreal.log(f"[Models] ProjectDir: {proj}")
    unreal.log(f"[Models] Source: {src_models}")

    zip_paths = []
    for dirpath, _, files in os.walk(src_models):
        for fn in files:
            if fn.lower().endswith(".zip"):
                zip_paths.append(os.path.join(dirpath, fn))
    if not zip_paths:
        unreal.log_warning("[Models] No .zip files found under SourceAssets/Models")
        return

    imported = 0
    for zp in sorted(zip_paths):
        base = os.path.splitext(os.path.basename(zp))[0]
        out_dir = os.path.join(extracted_root, base)

        # Extract if needed.
        if not os.path.isdir(out_dir) or not _find_fbx(out_dir):
            unreal.log(f"[Models] Extracting: {zp} -> {out_dir}")
            _extract_zip(zp, out_dir)

        fbx = _find_fbx(out_dir)
        if not fbx:
            unreal.log_warning(f"[Models] No FBX found in {out_dir}")
            continue

        lower = zp.replace("\\", "/").lower()
        color = _color_from_name(lower)  # optional

        # Mapping
        if "/loot bags/" in lower:
            if not color:
                unreal.log_warning(f"[Models] Loot bag zip missing color keyword: {zp}")
                continue
            # Import into per-color subfolders so generic FBX asset names (Material_001, Image_0...)
            # don't collide/overwrite between colors.
            dest_dir = f"/Game/World/LootBags/{color}"
            dest_name = f"SM_LootBag_{color}"
            unreal.log(f"[Models] Import LootBag {color}: {fbx} -> {dest_dir}/{dest_name}")
            paths = _import_fbx_static(fbx, dest_dir, dest_name)
            imported += 1 if paths else 0
            continue

        if "/gates/" in lower:
            if "coward" in lower:
                dest_dir = "/Game/World/Gates"
                dest_name = "SM_CowardiceGate"
                unreal.log(f"[Models] Import CowardiceGate: {fbx} -> {dest_dir}/{dest_name}")
                paths = _import_fbx_static(fbx, dest_dir, dest_name)
                imported += 1 if paths else 0
                continue
            if "portal" in lower or "stage" in lower:
                dest_dir = "/Game/World/Gates"
                dest_name = "SM_StageGate"
                unreal.log(f"[Models] Import StageGate: {fbx} -> {dest_dir}/{dest_name}")
                paths = _import_fbx_static(fbx, dest_dir, dest_name)
                imported += 1 if paths else 0
                continue

        if "/world interactables/" in lower:
            if "/idol altar/" in lower:
                dest_dir = "/Game/World/Interactables"
                dest_name = "SM_IdolAltar"
                unreal.log(f"[Models] Import IdolAltar: {fbx} -> {dest_dir}/{dest_name}")
                paths = _import_fbx_static(fbx, dest_dir, dest_name)
                imported += 1 if paths else 0
                continue
            if "/totem/" in lower:
                dest_dir = "/Game/World/Interactables"
                dest_name = "SM_DifficultyTotem"
                unreal.log(f"[Models] Import DifficultyTotem: {fbx} -> {dest_dir}/{dest_name}")
                paths = _import_fbx_static(fbx, dest_dir, dest_name)
                imported += 1 if paths else 0
                continue
            if "/tree/" in lower:
                if not color:
                    unreal.log_warning(f"[Models] Tree zip missing color keyword: {zp}")
                    continue
                dest_dir = "/Game/World/Interactables/Trees"
                dest_name = f"SM_Tree_{color}"
                unreal.log(f"[Models] Import Tree {color}: {fbx} -> {dest_dir}/{dest_name}")
                paths = _import_fbx_static(fbx, dest_dir, dest_name)
                imported += 1 if paths else 0
                continue
            if "/truck/" in lower:
                if not color:
                    unreal.log_warning(f"[Models] Truck zip missing color keyword: {zp}")
                    continue
                dest_dir = "/Game/World/Interactables/Trucks"
                dest_name = f"SM_CashTruck_{color}"
                unreal.log(f"[Models] Import CashTruck {color}: {fbx} -> {dest_dir}/{dest_name}")
                paths = _import_fbx_static(fbx, dest_dir, dest_name)
                imported += 1 if paths else 0
                continue
            if "/wheel/" in lower:
                if not color:
                    unreal.log_warning(f"[Models] Wheel zip missing color keyword: {zp}")
                    continue
                dest_dir = "/Game/World/Interactables/Wheels"
                dest_name = f"SM_Wheel_{color}"
                unreal.log(f"[Models] Import Wheel {color}: {fbx} -> {dest_dir}/{dest_name}")
                paths = _import_fbx_static(fbx, dest_dir, dest_name)
                imported += 1 if paths else 0
                continue

        if "/npcs/" in lower:
            if "trickster" in lower:
                dest_dir = "/Game/Characters/NPCs/Trickster/Trickster"
                dest_name = "SK_Trickster"
                unreal.log(f"[Models] Import Trickster (skeletal): {fbx} -> {dest_dir}/{dest_name}")
                paths = _import_fbx_skeletal(fbx, dest_dir, dest_name)
                imported += 1 if paths else 0
                continue
            if "ouroboros" in lower:
                dest_dir = "/Game/Characters/NPCs/Ouroboros/Ouroboros"
                dest_name = "SK_Ouroboros"
                unreal.log(f"[Models] Import Ouroboros (skeletal): {fbx} -> {dest_dir}/{dest_name}")
                paths = _import_fbx_skeletal(fbx, dest_dir, dest_name)
                imported += 1 if paths else 0
                continue

        unreal.log_warning(f"[Models] Unmapped zip (skipped): {zp}")

    unreal.log(f"=== ImportWorldModels: DONE (imported {imported} packs) ===")

    # If launched via -ExecutePythonScript, auto-quit so automation doesn't leave an editor open.
    try:
        get_cmd = getattr(unreal.SystemLibrary, "get_command_line", None)
        cmdline = str(get_cmd()) if callable(get_cmd) else ""
        if "ExecutePythonScript" in cmdline or "executePythonScript" in cmdline:
            unreal.SystemLibrary.quit_editor()
    except Exception:
        pass


if __name__ == "__main__":
    main()

