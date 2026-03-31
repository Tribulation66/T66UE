"""
Import hero and companion portrait textures from SourceAssets/FinalPortraits into the project
and reload the matching data tables.

Run with:
  UnrealEditor.exe <uproject> -ExecutePythonScript="C:/UE/T66/Scripts/ImportHeroPortraits.py"
"""

import csv
import os
import unreal


HERO_DEST_DIR = "/Game/UI/Sprites/Heroes"
COMPANION_DEST_DIR = "/Game/UI/Sprites/Companions"
BODY_PREFIXES = {
    "TypeA": "male",
    "TypeB": "female",
}
STATE_SUFFIXES = {
    "Low": "low",
    "Half": "half",
    "Full": "full",
}
DEST_SUFFIXES = {
    "Low": "_Low",
    "Half": "",
    "Full": "_Full",
}


def ensure_directory(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def get_project_dir():
    return unreal.SystemLibrary.get_project_directory().replace("\\", "/").rstrip("/")


def get_portrait_source_dir(project_dir):
    candidates = [
        os.path.normpath(os.path.join(project_dir, "SourceAssets", "FinalPortraits")),
        os.path.normpath(os.path.join(project_dir, "SourceAssets", "HeroPortraits")),
    ]
    for candidate in candidates:
        if os.path.isdir(candidate):
            return candidate
    return candidates[0]


def normalize_display_name(name):
    return "".join(ch for ch in name if ch.isalnum())


def get_live_heroes(csv_path):
    heroes = []
    with open(csv_path, "r", encoding="utf-8-sig", newline="") as handle:
        for row in csv.DictReader(handle):
            hero_id = (row.get("HeroID") or "").strip()
            display_name = (row.get("DisplayName") or "").strip()
            if hero_id and display_name:
                heroes.append((hero_id, normalize_display_name(display_name)))
    return heroes


def get_live_companions(csv_path):
    companions = []
    with open(csv_path, "r", encoding="utf-8-sig", newline="") as handle:
        for row in csv.DictReader(handle):
            companion_id = (row.get("CompanionID") or "").strip()
            display_name = (row.get("DisplayName") or "").strip()
            if companion_id and display_name:
                companions.append((companion_id, normalize_display_name(display_name)))
    return companions


def import_texture(source_path, dest_dir, dest_name):
    task = unreal.AssetImportTask()
    task.automated = True
    task.save = False
    task.replace_existing = True
    task.replace_existing_settings = True
    task.filename = source_path
    task.destination_path = dest_dir
    task.destination_name = dest_name

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

    asset_path = f"{dest_dir}/{dest_name}"
    asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    if not asset or not isinstance(asset, unreal.Texture2D):
        raise RuntimeError(f"Texture import failed: {asset_path}")

    try:
        asset.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_EDITOR_ICON)
    except Exception:
        pass
    try:
        asset.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_UI)
    except Exception:
        pass
    try:
        asset.set_editor_property("mip_gen_settings", unreal.TextureMipGenSettings.TMGS_FROM_TEXTURE_GROUP)
    except Exception:
        pass
    try:
        asset.set_editor_property("filter", unreal.TextureFilter.TF_TRILINEAR)
    except Exception:
        pass
    try:
        asset.set_editor_property("never_stream", True)
    except Exception:
        pass

    asset.set_editor_property("srgb", True)
    unreal.EditorAssetLibrary.save_asset(asset_path)


def reload_data_table(dt_path, csv_path, log_name):
    dt = unreal.EditorAssetLibrary.load_asset(dt_path)
    if not dt:
        raise RuntimeError(f"Data table not found at {dt_path}")

    if not os.path.exists(csv_path):
        raise RuntimeError(f"CSV not found at {csv_path}")

    success = unreal.DataTableFunctionLibrary.fill_data_table_from_csv_file(dt, csv_path)
    if not success:
        raise RuntimeError(f"Failed to reload {dt_path} from CSV.")

    unreal.EditorAssetLibrary.save_asset(dt_path)
    unreal.log(f"[ImportHeroPortraits] Reloaded {log_name}")


def import_hero_portraits(project_dir, source_dir):
    ensure_directory(HERO_DEST_DIR)

    csv_path = os.path.normpath(os.path.join(project_dir, "Content", "Data", "Heroes.csv"))
    if not os.path.exists(csv_path):
        raise RuntimeError(f"Heroes.csv not found: {csv_path}")

    heroes = get_live_heroes(csv_path)
    if not heroes:
        raise RuntimeError("No live hero IDs found in Heroes.csv")

    missing_sources = []
    imports = []
    for hero_id, source_stem in heroes:
        hero_dest_dir = f"{HERO_DEST_DIR}/{hero_id}"
        ensure_directory(hero_dest_dir)
        for body_type, source_body in BODY_PREFIXES.items():
            for state_name, source_state in STATE_SUFFIXES.items():
                source_name = f"{source_stem}_{source_body}_{source_state}.png"
                source_path = os.path.join(source_dir, source_name)
                dest_name = f"T_{hero_id}_{body_type}{DEST_SUFFIXES[state_name]}"
                if not os.path.exists(source_path):
                    missing_sources.append(source_path)
                    continue
                imports.append((source_path, hero_dest_dir, dest_name))

    if missing_sources:
        preview = "\n".join(missing_sources[:12])
        suffix = "" if len(missing_sources) <= 12 else f"\n...and {len(missing_sources) - 12} more"
        raise RuntimeError(f"Missing source hero portraits:\n{preview}{suffix}")

    for source_path, hero_dest_dir, dest_name in imports:
        import_texture(source_path, hero_dest_dir, dest_name)

    reload_data_table(
        "/Game/Data/DT_Heroes",
        csv_path,
        "/Game/Data/DT_Heroes",
    )
    unreal.log(f"[ImportHeroPortraits] Imported {len(imports)} hero portrait textures into {HERO_DEST_DIR}")
    return len(imports)


def import_companion_selection_portraits(project_dir, source_dir):
    ensure_directory(COMPANION_DEST_DIR)

    csv_path = os.path.normpath(os.path.join(project_dir, "Content", "Data", "Companions.csv"))
    if not os.path.exists(csv_path):
        raise RuntimeError(f"Companions.csv not found: {csv_path}")

    companions = get_live_companions(csv_path)
    if not companions:
        raise RuntimeError("No live companion IDs found in Companions.csv")

    missing_sources = []
    imports = []
    for companion_id, source_stem in companions:
        companion_dest_dir = f"{COMPANION_DEST_DIR}/{companion_id}"
        ensure_directory(companion_dest_dir)
        source_name = f"Companion_{source_stem}.png"
        source_path = os.path.join(source_dir, source_name)
        dest_name = f"T_{companion_id}_Selection"
        if not os.path.exists(source_path):
            missing_sources.append(source_path)
            continue
        imports.append((source_path, companion_dest_dir, dest_name))

    if missing_sources:
        preview = "\n".join(missing_sources[:12])
        suffix = "" if len(missing_sources) <= 12 else f"\n...and {len(missing_sources) - 12} more"
        raise RuntimeError(f"Missing source companion portraits:\n{preview}{suffix}")

    for source_path, companion_dest_dir, dest_name in imports:
        import_texture(source_path, companion_dest_dir, dest_name)

    reload_data_table(
        "/Game/Data/DT_Companions",
        csv_path,
        "/Game/Data/DT_Companions",
    )
    unreal.log(
        f"[ImportHeroPortraits] Imported {len(imports)} companion selection portraits into {COMPANION_DEST_DIR}"
    )
    return len(imports)


def main():
    unreal.log("=== ImportHeroPortraits ===")

    project_dir = get_project_dir()
    source_dir = get_portrait_source_dir(project_dir)

    if not os.path.isdir(source_dir):
        raise RuntimeError(f"Portrait source directory not found: {source_dir}")

    hero_count = import_hero_portraits(project_dir, source_dir)
    companion_count = import_companion_selection_portraits(project_dir, source_dir)

    unreal.log(
        f"[ImportHeroPortraits] Imported {hero_count} hero textures and {companion_count} companion textures from {source_dir}"
    )
    unreal.log("=== ImportHeroPortraits DONE ===")


if __name__ == "__main__":
    main()
