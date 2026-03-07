"""
Import the final hero roster from SourceAssets/Heros.

Expected source layout:
  SourceAssets/Heros/<order>_<HeroName>/
    <HeroName>Idle.fbx
    <HeroName>Walk.fbx
    <HeroName>Jump.fbx

Uses the legacy FBX pipeline (FbxImportUI) — same as ImportCompanions.py —
so that materials expose DiffuseColorMap for emissive fill lighting.

Run INSIDE Unreal Editor:
  Tools -> Execute Python Script -> Scripts/ImportHeros.py
"""

import os
import re
import unreal


def _proj_dir():
    try:
        return unreal.SystemLibrary.get_project_directory().replace("\\", "/")
    except Exception:
        return os.path.abspath(os.path.join(os.getcwd(), "..")).replace("\\", "/")


def _ensure_game_dir(game_path):
    if not unreal.EditorAssetLibrary.does_directory_exist(game_path):
        unreal.EditorAssetLibrary.make_directory(game_path)


def _delete_dir_if_exists(game_dir):
    try:
        if unreal.EditorAssetLibrary.does_directory_exist(game_dir):
            unreal.EditorAssetLibrary.delete_directory(game_dir)
    except Exception:
        pass


def _parse_hero_folders(heros_root):
    heroes = []
    for name in os.listdir(heros_root):
        full = os.path.join(heros_root, name)
        if not os.path.isdir(full):
            continue
        match = re.match(r"^(\d+)_([^\\/]+)$", name)
        if not match:
            unreal.log_warning(f"[ImportHeros] Skip malformed folder name: {name}")
            continue
        order = int(match.group(1))
        hero_name = match.group(2)
        hero_id = f"Hero_{order}"
        heroes.append((order, hero_name, hero_id, full))
    heroes.sort(key=lambda item: item[0])
    return heroes


def _find_action_fbx(hero_dir, hero_name, action):
    expected = f"{hero_name}{action}".lower()
    for fn in os.listdir(hero_dir):
        if not fn.lower().endswith(".fbx"):
            continue
        stem = os.path.splitext(fn)[0].lower()
        if stem == expected:
            return os.path.join(hero_dir, fn)
    return None


def _load_asset(path):
    try:
        return unreal.EditorAssetLibrary.load_asset(path)
    except Exception:
        return None


def _asset_class_name(asset):
    try:
        return asset.get_class().get_name()
    except Exception:
        return ""


def _rename_asset(old_path, new_path):
    if not old_path or old_path == new_path:
        return new_path
    if unreal.EditorAssetLibrary.does_asset_exist(new_path):
        unreal.EditorAssetLibrary.delete_asset(new_path)
    if unreal.EditorAssetLibrary.rename_asset(old_path, new_path):
        return new_path
    unreal.log_warning(f"[ImportHeros] Failed to rename: {old_path} -> {new_path}")
    return old_path


def _find_of_class(imported_paths, target_classes):
    for path in imported_paths or []:
        clean = str(path).split(".")[0]
        asset = _load_asset(clean)
        if asset and _asset_class_name(asset) in target_classes:
            return clean, asset
    return None, None


def _import_fbx(fbx_path, dest_dir, dest_name,
                import_animations=True, import_mesh=True, skeleton=None):
    """Legacy FBX import (FbxImportUI) — same pipeline as ImportCompanions.py."""
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
    opts.import_mesh = bool(import_mesh)
    opts.import_textures = bool(import_mesh)
    opts.import_materials = bool(import_mesh)
    opts.import_as_skeletal = True
    opts.import_animations = bool(import_animations)
    try:
        opts.set_editor_property("create_physics_asset", False)
    except Exception:
        pass
    if skeleton is not None:
        try:
            opts.set_editor_property("skeleton", skeleton)
        except Exception:
            pass
    if not import_mesh and import_animations:
        try:
            opts.set_editor_property("mesh_type_to_import",
                                     unreal.FBXImportType.FBXIT_ANIMATION)
            opts.set_editor_property("original_import_type",
                                     unreal.FBXImportType.FBXIT_ANIMATION)
        except Exception:
            pass

    task.options = opts
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    return list(task.imported_object_paths or [])


def main():
    proj = _proj_dir()
    heros_root = os.path.join(proj, "SourceAssets", "Heros")
    if not os.path.isdir(heros_root):
        unreal.log_error("[ImportHeros] SourceAssets/Heros not found")
        return

    hero_entries = _parse_hero_folders(heros_root)
    if not hero_entries:
        unreal.log_error("[ImportHeros] No valid hero folders found")
        return

    unreal.log("=== ImportHeros: START (legacy FBX pipeline) ===")
    success_count = 0

    for order, hero_name, hero_id, hero_dir in hero_entries:
        idle_fbx = _find_action_fbx(hero_dir, hero_name, "Idle")
        walk_fbx = _find_action_fbx(hero_dir, hero_name, "Walk")
        jump_fbx = _find_action_fbx(hero_dir, hero_name, "Jump")

        if not idle_fbx or not walk_fbx or not jump_fbx:
            unreal.log_warning(
                f"[ImportHeros] Skip {hero_id} ({hero_name}): missing Idle/Walk/Jump FBX"
            )
            continue

        dest_dir = f"/Game/Characters/Heroes/{hero_id}/TypeA"
        mesh_name = f"SK_{hero_id}_TypeA"
        idle_anim_name = f"AM_{hero_id}_TypeA_Idle"
        walk_anim_name = f"AM_{hero_id}_TypeA_Walk"
        jump_anim_name = f"AM_{hero_id}_TypeA_Jump"

        unreal.log(f"[ImportHeros] === {hero_id} ({hero_name}) ===")

        # Clean destination to remove old Interchange-created materials
        _delete_dir_if_exists(dest_dir)
        _ensure_game_dir(dest_dir)

        # ---- Step 1: Mesh + materials + textures from idle FBX (no animation) ----
        mesh_paths = _import_fbx(idle_fbx, dest_dir, mesh_name,
                                  import_animations=False, import_mesh=True)
        unreal.log(f"[ImportHeros]   Mesh import: {len(mesh_paths)} assets")

        skeleton = None
        mesh_asset = _load_asset(f"{dest_dir}/{mesh_name}")
        if mesh_asset:
            try:
                skeleton = mesh_asset.get_editor_property("skeleton")
            except Exception:
                pass
        if not skeleton:
            unreal.log_error(f"[ImportHeros]   FAILED: no Skeleton for {hero_id}")
            continue

        # ---- Step 2: Idle animation (anim-only, reuses skeleton) ----
        idle_paths = _import_fbx(idle_fbx, dest_dir, idle_anim_name,
                                  import_animations=True, import_mesh=False,
                                  skeleton=skeleton)
        idle_anim, _ = _find_of_class(idle_paths, {"AnimSequence", "AnimationAsset"})
        if idle_anim:
            _rename_asset(idle_anim, f"{dest_dir}/{idle_anim_name}")
            unreal.log(f"[ImportHeros]   Idle anim: OK")
        else:
            unreal.log_warning(f"[ImportHeros]   Idle anim: FAILED")

        # ---- Step 3: Walk animation (anim-only) ----
        walk_paths = _import_fbx(walk_fbx, dest_dir, walk_anim_name,
                                  import_animations=True, import_mesh=False,
                                  skeleton=skeleton)
        walk_anim, _ = _find_of_class(walk_paths, {"AnimSequence", "AnimationAsset"})
        if walk_anim:
            _rename_asset(walk_anim, f"{dest_dir}/{walk_anim_name}")
            unreal.log(f"[ImportHeros]   Walk anim: OK")
        else:
            unreal.log_warning(f"[ImportHeros]   Walk anim: FAILED")

        # ---- Step 4: Jump animation (anim-only) ----
        jump_paths = _import_fbx(jump_fbx, dest_dir, jump_anim_name,
                                  import_animations=True, import_mesh=False,
                                  skeleton=skeleton)
        jump_anim, _ = _find_of_class(jump_paths, {"AnimSequence", "AnimationAsset"})
        if jump_anim:
            _rename_asset(jump_anim, f"{dest_dir}/{jump_anim_name}")
            unreal.log(f"[ImportHeros]   Jump anim: OK")
        else:
            unreal.log_warning(f"[ImportHeros]   Jump anim: FAILED")

        success_count += 1
        unreal.log(f"[ImportHeros]   {hero_id} done.")

    unreal.log(f"=== ImportHeros: DONE ({success_count}/{len(hero_entries)} heroes) ===")


if __name__ == "__main__":
    main()
