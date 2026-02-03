"""
Import models (static meshes + NPC skeletal meshes) from SourceAssets/Models/*.zip and direct folders.

Run INSIDE the Unreal Editor (not commandlet) via:
  Tools -> Execute Python Script... -> Scripts/ImportModels.py
or from Output Log:
  py "C:/UE/T66/Scripts/ImportModels.py"

What it does:
- Extracts each .zip under SourceAssets/Models/ into SourceAssets/Models/Extracted/<zip-name>/
- Finds the primary FBX in each extracted folder
- Imports to stable /Game paths & names that the C++ code expects
  - Loot bags: /Game/World/LootBags/SM_LootBag_<Color>
  - Gates: /Game/World/Gates/SM_StageGate, SM_CowardiceGate
  - Interactables: SM_IdolAltar, SM_DifficultyTotem, VendingMachine/SM_VendingMachine_Black, Trees/Trucks/Wheels per color
  - NPCs (skeletal): Trickster, Ouroboros

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


def _to_game_log_path(p: str) -> str:
    # Avoid backslashes in f-string expressions (UE python can be picky).
    try:
        return p.replace(os.sep, "/")
    except Exception:
        return str(p)


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
    # Models can live in either:
    # - SourceAssets/Models (preferred)
    # - SourceAssets/Extracted/Models (some setups relocate zips here)
    src_models_candidates = [
        os.path.join(proj, "SourceAssets", "Models"),
        os.path.join(proj, "SourceAssets", "Extracted", "Models"),
    ]
    src_models_roots = [p for p in src_models_candidates if os.path.isdir(p)]
    if not src_models_roots:
        unreal.log_warning("[ImportModels] No models root found under SourceAssets (expected Models/ or Extracted/Models/).")
        return
    # Use the first root for extraction output (keeps behavior stable).
    extracted_root = os.path.join(src_models_roots[0], "Extracted")

    # FBX import (AssetTools) requires full Editor (Slate); commandlet runs will assert. Run script in Editor.
    try:
        is_cmdlet_fn = getattr(unreal.SystemLibrary, "is_running_commandlet", None)
        if callable(is_cmdlet_fn) and bool(is_cmdlet_fn()):
            unreal.log_error("ImportModels must be run inside the full Editor (not commandlet). AssetTools require Slate.")
            return
    except Exception:
        pass

    unreal.log("=== ImportModels: START ===")
    unreal.log(f"[ImportModels] ProjectDir: {proj}")
    unreal.log("[ImportModels] Sources: " + ", ".join([_to_game_log_path(p) for p in src_models_roots]))

    zip_paths = []
    for root in src_models_roots:
        for dirpath, _, files in os.walk(root):
            for fn in files:
                if fn.lower().endswith(".zip"):
                    zip_paths.append(os.path.join(dirpath, fn))

    imported = 0
    if zip_paths:
        for zp in sorted(zip_paths):
            base = os.path.splitext(os.path.basename(zp))[0]
            out_dir = os.path.join(extracted_root, base)

            # Extract if needed.
            if not os.path.isdir(out_dir) or not _find_fbx(out_dir):
                unreal.log(f"[ImportModels] Extracting: {zp} -> {out_dir}")
                _extract_zip(zp, out_dir)

            fbx = _find_fbx(out_dir)
            if not fbx:
                unreal.log_warning(f"[ImportModels] No FBX found in {out_dir}")
                continue

            lower = zp.replace("\\", "/").lower()
            color = _color_from_name(lower)  # optional

            # Mapping
            if "/loot bags/" in lower:
                if not color:
                    unreal.log_warning(f"[ImportModels] Loot bag zip missing color keyword: {zp}")
                    continue
                # Import into per-color subfolders so generic FBX asset names (Material_001, Image_0...)
                # don't collide/overwrite between colors.
                dest_dir = f"/Game/World/LootBags/{color}"
                dest_name = f"SM_LootBag_{color}"
                unreal.log(f"[ImportModels] Import LootBag {color}: {fbx} -> {dest_dir}/{dest_name}")
                paths = _import_fbx_static(fbx, dest_dir, dest_name)
                imported += 1 if paths else 0
                continue

            if "/gates/" in lower:
                if "coward" in lower:
                    dest_dir = "/Game/World/Gates"
                    dest_name = "SM_CowardiceGate"
                    unreal.log(f"[ImportModels] Import CowardiceGate: {fbx} -> {dest_dir}/{dest_name}")
                    paths = _import_fbx_static(fbx, dest_dir, dest_name)
                    imported += 1 if paths else 0
                    continue
                if "portal" in lower or "stage" in lower:
                    dest_dir = "/Game/World/Gates"
                    dest_name = "SM_StageGate"
                    unreal.log(f"[ImportModels] Import StageGate: {fbx} -> {dest_dir}/{dest_name}")
                    paths = _import_fbx_static(fbx, dest_dir, dest_name)
                    imported += 1 if paths else 0
                    continue

            if "/world interactables/" in lower:
                if "/idol altar/" in lower:
                    dest_dir = "/Game/World/Interactables"
                    dest_name = "SM_IdolAltar"
                    unreal.log(f"[ImportModels] Import IdolAltar: {fbx} -> {dest_dir}/{dest_name}")
                    paths = _import_fbx_static(fbx, dest_dir, dest_name)
                    imported += 1 if paths else 0
                    continue
                if "/totem/" in lower:
                    dest_dir = "/Game/World/Interactables"
                    dest_name = "SM_DifficultyTotem"
                    unreal.log(f"[ImportModels] Import DifficultyTotem: {fbx} -> {dest_dir}/{dest_name}")
                    paths = _import_fbx_static(fbx, dest_dir, dest_name)
                    imported += 1 if paths else 0
                    continue
                if "/tree/" in lower:
                    if not color:
                        unreal.log_warning(f"[ImportModels] Tree zip missing color keyword: {zp}")
                        continue
                    # Import into per-color subfolders so generic FBX asset names (Material_001, Image_0...)
                    # don't collide/overwrite between colors.
                    dest_dir = f"/Game/World/Interactables/Trees/{color}"
                    dest_name = f"SM_Tree_{color}"
                    unreal.log(f"[ImportModels] Import Tree {color}: {fbx} -> {dest_dir}/{dest_name}")
                    paths = _import_fbx_static(fbx, dest_dir, dest_name)
                    imported += 1 if paths else 0
                    continue
                if "/truck/" in lower:
                    if not color:
                        unreal.log_warning(f"[ImportModels] Truck zip missing color keyword: {zp}")
                        continue
                    # Import into per-color subfolders so generic FBX asset names (Material_001, Image_0...)
                    # don't collide/overwrite between colors.
                    dest_dir = f"/Game/World/Interactables/Trucks/{color}"
                    dest_name = f"SM_CashTruck_{color}"
                    unreal.log(f"[ImportModels] Import CashTruck {color}: {fbx} -> {dest_dir}/{dest_name}")
                    paths = _import_fbx_static(fbx, dest_dir, dest_name)
                    imported += 1 if paths else 0
                    continue
                if "/wheel/" in lower:
                    if not color:
                        unreal.log_warning(f"[ImportModels] Wheel zip missing color keyword: {zp}")
                        continue
                    # Import into per-color subfolders so generic FBX asset names (Material_001, Image_0...)
                    # don't collide/overwrite between colors.
                    dest_dir = f"/Game/World/Interactables/Wheels/{color}"
                    dest_name = f"SM_Wheel_{color}"
                    unreal.log(f"[ImportModels] Import Wheel {color}: {fbx} -> {dest_dir}/{dest_name}")
                    paths = _import_fbx_static(fbx, dest_dir, dest_name)
                    imported += 1 if paths else 0
                    continue

            if "/npcs/" in lower:
                if "trickster" in lower:
                    dest_dir = "/Game/Characters/NPCs/Trickster/Trickster"
                    dest_name = "SK_Trickster"
                    unreal.log(f"[ImportModels] Import Trickster (skeletal): {fbx} -> {dest_dir}/{dest_name}")
                    paths = _import_fbx_skeletal(fbx, dest_dir, dest_name)
                    imported += 1 if paths else 0
                    continue
                if "ouroboros" in lower:
                    dest_dir = "/Game/Characters/NPCs/Ouroboros/Ouroboros"
                    dest_name = "SK_Ouroboros"
                    unreal.log(f"[ImportModels] Import Ouroboros (skeletal): {fbx} -> {dest_dir}/{dest_name}")
                    paths = _import_fbx_skeletal(fbx, dest_dir, dest_name)
                    imported += 1 if paths else 0
                    continue

            unreal.log_warning(f"[ImportModels] Unmapped zip (skipped): {zp}")

    # Direct folders (no .zip): e.g. SourceAssets/Models/Vending Machine Black
    for models_root in src_models_roots:
        try:
            entries = os.listdir(models_root)
        except OSError:
            continue
        for name in entries:
            if name == "Extracted":
                continue
            folder_path = os.path.join(models_root, name)
            if not os.path.isdir(folder_path):
                continue
            lower = name.lower().replace("\\", "/")
            if "vending" in lower and "black" in lower:
                fbx = _find_fbx(folder_path)
                if not fbx:
                    unreal.log_warning(f"[ImportModels] No FBX in direct folder: {folder_path}")
                    continue
                dest_dir = "/Game/World/Interactables/VendingMachine"
                dest_name = "SM_VendingMachine_Black"
                unreal.log(f"[ImportModels] Import VendingMachine (direct folder): {fbx} -> {dest_dir}/{dest_name}")
                paths = _import_fbx_static(fbx, dest_dir, dest_name)
                imported += 1 if paths else 0
                break  # only one vending machine black folder expected

    unreal.log(f"=== ImportModels: DONE (imported {imported} packs) ===")

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
