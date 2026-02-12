"""
Import UI sprite textures from SourceAssets into Unreal Content.

This script is intended to be re-runnable and safe:
- Imports/updates textures for Items (by rarity), Idols, Hero portraits, and Games.
- Writes assets under /Game/UI/Sprites/** so code + DataTables can reference stable paths.

Run (PowerShell):
& "C:/Program Files/Epic Games/UE_5.7/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" "C:/UE/T66/T66.uproject" -run=pythonscript -script="C:/UE/T66/Scripts/ImportSpriteTextures.py" -unattended -nop4 -nosplash
"""

import os
import re
import unreal


def _proj_dir() -> str:
    try:
        return unreal.SystemLibrary.get_project_directory()
    except Exception:
        # Fallback for odd invocation contexts.
        return os.path.abspath(os.path.join(os.getcwd(), ".."))


def _norm_asset_name(name: str) -> str:
    # UE-friendly: letters/numbers/underscore only, no spaces.
    n = re.sub(r"[^A-Za-z0-9_]+", "_", name.strip())
    n = re.sub(r"_+", "_", n).strip("_")
    if not n:
        return "Unnamed"
    if n[0].isdigit():
        n = f"_{n}"
    return n


def _ensure_dir(game_path: str):
    if not unreal.EditorAssetLibrary.does_directory_exist(game_path):
        unreal.EditorAssetLibrary.make_directory(game_path)


def _import_texture(src_file: str, dest_game_dir: str, dest_asset_name: str) -> str:
    """
    Import or reimport a texture file.
    Returns the resulting asset path (e.g. /Game/UI/Sprites/Items/Black/T_Item_Black_01).
    """
    _ensure_dir(dest_game_dir)

    task = unreal.AssetImportTask()
    task.automated = True
    task.save = True
    task.replace_existing = True
    task.replace_existing_settings = True
    task.filename = src_file
    task.destination_path = dest_game_dir
    task.destination_name = dest_asset_name

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

    if not task.imported_object_paths:
        unreal.log_warning(f"[Sprites] Import produced no assets: {src_file}")
        return ""
    return task.imported_object_paths[0]


def _apply_ui_texture_settings(asset_path: str):
    """
    Apply UI-friendly texture settings (no mipmaps, UI compression).
    """
    if not asset_path:
        return
    tex = unreal.EditorAssetLibrary.load_asset(asset_path)
    if not tex:
        return
    if not isinstance(tex, unreal.Texture2D):
        return

    changed = False
    try:
        # No mips for UI icons; keeps them crisp and avoids unnecessary memory.
        if tex.get_editor_property("mip_gen_settings") != unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS:
            tex.set_editor_property("mip_gen_settings", unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
            changed = True
    except Exception:
        pass

    try:
        if tex.get_editor_property("compression_settings") != unreal.TextureCompressionSettings.TC_EDITOR_ICON:
            tex.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_EDITOR_ICON)
            changed = True
    except Exception:
        pass

    if changed:
        # Some UE Python builds don't expose post_edit_change(); saving is enough.
        try:
            if hasattr(tex, "modify"):
                tex.modify()
        except Exception:
            pass
        try:
            unreal.EditorAssetLibrary.save_loaded_asset(tex)
        except Exception:
            pass
        try:
            unreal.EditorAssetLibrary.save_asset(asset_path)
        except Exception:
            pass


def _read_heroes_csv_display_names() -> dict:
    """
    Returns {HeroID: DisplayName} from Content/Data/Heroes.csv.
    """
    heroes_csv = os.path.join(_proj_dir(), "Content", "Data", "Heroes.csv")
    out = {}
    if not os.path.isfile(heroes_csv):
        unreal.log_warning(f"[Sprites] Missing Heroes.csv: {heroes_csv}")
        return out

    with open(heroes_csv, "r", encoding="utf-8") as f:
        lines = [ln.strip("\n") for ln in f.readlines() if ln.strip()]

    # Expect header like: ---,HeroID,DisplayName,...
    if not lines:
        return out
    header = lines[0].split(",")
    # Locate columns robustly.
    try:
        hero_id_col = header.index("HeroID")
        display_name_col = header.index("DisplayName")
    except ValueError:
        return out

    for ln in lines[1:]:
        cols = ln.split(",")
        if len(cols) <= max(hero_id_col, display_name_col):
            continue
        hero_id = cols[hero_id_col].strip()
        display = cols[display_name_col].strip().strip('"')
        if hero_id:
            out[hero_id] = display
    return out


def main():
    proj = _proj_dir()
    sprites_root = os.path.join(proj, "SourceAssets", "Sprites")
    if not os.path.isdir(sprites_root):
        unreal.log_error(f"[Sprites] SourceAssets/Sprites not found: {sprites_root}")
        return

    unreal.log("=== ImportSpriteTextures: START ===")
    unreal.log(f"[Sprites] ProjectDir: {proj}")
    unreal.log(f"[Sprites] SourceRoot: {sprites_root}")

    imported_count = 0

    # --------------------
    # Items
    # --------------------
    for rarity in ["Black", "Red", "Yellow", "White"]:
        src_dir = os.path.join(sprites_root, "Items", rarity)
        if not os.path.isdir(src_dir):
            unreal.log_warning(f"[Sprites] Missing item dir: {src_dir}")
            continue
        dest_dir = f"/Game/UI/Sprites/Items/{rarity}"
        for fn in sorted(os.listdir(src_dir)):
            if not fn.lower().endswith(".png"):
                continue
            src = os.path.join(src_dir, fn)
            # B1.png -> 01 etc.
            m = re.match(r"^[BRYW](\d+)\.png$", fn, re.IGNORECASE)
            if m:
                idx = int(m.group(1))
                asset = f"T_Item_{rarity}_{idx:02d}"
            else:
                asset = f"T_Item_{rarity}_{_norm_asset_name(os.path.splitext(fn)[0])}"
            asset_path = _import_texture(src, dest_dir, asset)
            _apply_ui_texture_settings(asset_path)
            if asset_path:
                imported_count += 1

    # --------------------
    # Idols
    # --------------------
    idols_dir = os.path.join(sprites_root, "Idols")
    if os.path.isdir(idols_dir):
        dest_dir = "/Game/UI/Sprites/Idols"
        for fn in sorted(os.listdir(idols_dir)):
            if not fn.lower().endswith(".png"):
                continue
            base = os.path.splitext(fn)[0]
            # "Fire Idol" -> "Fire"
            base = base.replace(" Idol", "").replace("idol", "").strip()
            asset = f"T_Idol_{_norm_asset_name(base)}"
            asset_path = _import_texture(os.path.join(idols_dir, fn), dest_dir, asset)
            _apply_ui_texture_settings(asset_path)
            if asset_path:
                imported_count += 1

    # --------------------
    # Hero portraits (one per hero folder: portrait.png)
    # Imported under /Game/UI/Sprites/Heroes/<HeroID>/T_HeroPortrait_<HeroID>
    # --------------------
    heroes_map = _read_heroes_csv_display_names()
    heros_dir = os.path.join(sprites_root, "Heros")
    if os.path.isdir(heros_dir) and heroes_map:
        for hero_id, display_name in heroes_map.items():
            if not hero_id:
                continue
            # Folder name is display name (with spaces) per your convention.
            src_portrait = os.path.join(heros_dir, display_name, "portrait.png")
            if not os.path.isfile(src_portrait):
                # If folder naming drifts, try a normalized match (remove spaces).
                alt = os.path.join(heros_dir, _norm_asset_name(display_name).replace("_", " "), "portrait.png")
                if os.path.isfile(alt):
                    src_portrait = alt
                else:
                    continue

            dest_dir = f"/Game/UI/Sprites/Heroes/{hero_id}"
            asset = f"T_HeroPortrait_{_norm_asset_name(hero_id)}"
            asset_path = _import_texture(src_portrait, dest_dir, asset)
            _apply_ui_texture_settings(asset_path)
            if asset_path:
                imported_count += 1

    # --------------------
    # Games (optional)
    # --------------------
    games_dir = os.path.join(sprites_root, "Games")
    if os.path.isdir(games_dir):
        dest_dir = "/Game/UI/Sprites/Games"
        for fn in sorted(os.listdir(games_dir)):
            if not fn.lower().endswith(".png"):
                continue
            base = os.path.splitext(fn)[0]
            asset = f"T_Game_{_norm_asset_name(base)}"
            asset_path = _import_texture(os.path.join(games_dir, fn), dest_dir, asset)
            _apply_ui_texture_settings(asset_path)
            if asset_path:
                imported_count += 1

    # --------------------
    # Obsidian 9-slice (front-end UI panels/buttons)
    # SourceAssets/Images/obsidian.jpg -> /Game/UI/Obsidian
    # --------------------
    obsidian_src = os.path.join(proj, "SourceAssets", "Images", "obsidian.jpg")
    if os.path.isfile(obsidian_src):
        dest_dir = "/Game/UI"
        _ensure_dir(dest_dir)
        asset_path = _import_texture(obsidian_src, dest_dir, "Obsidian")
        if asset_path:
            _apply_ui_texture_settings(asset_path)
            imported_count += 1
            unreal.log("[Sprites] Imported obsidian 9-slice texture: " + asset_path)
    else:
        unreal.log_warning(f"[Sprites] Obsidian image not found: {obsidian_src}")

    unreal.log(f"=== ImportSpriteTextures: DONE (imported/updated {imported_count} textures) ===")


if __name__ == "__main__":
    main()

