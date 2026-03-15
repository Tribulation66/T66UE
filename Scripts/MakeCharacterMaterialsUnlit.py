"""
Central script to make character materials Unlit.

Scans /Game/Characters/ for MaterialInstanceConstant assets still parented
to FBXLegacyPhongSurfaceMaterial (Lit) and reparents them to M_Character_Unlit.

Smart texture discovery:
  1. Tries reading DiffuseColorMap from the MIC (works when import bound it).
  2. If empty, searches the same directory for a Texture2D whose name matches
     the MIC by convention  (Material_0_003 -> Image_0_003).
  3. Fallback: if only one Texture2D exists in the directory, uses that.

Safe to re-run — skips MICs already parented to M_Character_Unlit.

Run in-editor:
  Tools > Execute Python Script > Scripts/MakeCharacterMaterialsUnlit.py
"""

import unreal

LOG = "[CharUnlit]"

MASTER_PATH = "/Game/Materials/M_Character_Unlit"
TEXTURE_PARAM = "DiffuseColorMap"
ALT_MASTER_PATHS = [
    "/Game/Materials/M_FBX_Unlit",
]

SCAN_DIRS = [
    "/Game/Characters/Heroes/",
    "/Game/Characters/Companions/",
    "/Game/Characters/Enemies/",
    "/Game/Characters/NPCs/",
]

PARENTS_TO_CONVERT = [
    "FBXLegacyPhongSurfaceMaterial",
]

ALREADY_UNLIT_PARENTS = [
    "M_Character_Unlit",
    "M_FBX_Unlit",
]


def _get_parent_name(mic):
    try:
        parent = mic.get_editor_property("parent")
        if parent:
            return parent.get_name()
    except Exception:
        pass
    return ""


def _ensure_master_usage_flags():
    for material_path in [MASTER_PATH] + ALT_MASTER_PATHS:
        material = unreal.EditorAssetLibrary.load_asset(material_path)
        if not material or not isinstance(material, unreal.Material):
            continue

        changed = False
        for prop, value in [
            ("shading_model", unreal.MaterialShadingModel.MSM_UNLIT),
            ("two_sided", True),
            ("used_with_skeletal_mesh", True),
        ]:
            try:
                current = material.get_editor_property(prop)
                if current != value:
                    material.set_editor_property(prop, value)
                    changed = True
            except Exception:
                continue

        if changed:
            try:
                unreal.MaterialEditingLibrary.recompile_material(material)
            except Exception:
                pass
            try:
                unreal.EditorAssetLibrary.save_asset(material_path)
            except Exception:
                pass


def _get_texture_from_mic(mic):
    """Try to read DiffuseColorMap already set on the MIC."""
    try:
        result = unreal.MaterialEditingLibrary.get_material_instance_texture_parameter_value(
            mic, TEXTURE_PARAM)
        if isinstance(result, (tuple, list)):
            tex = result[-1] if len(result) > 1 else result[0]
        else:
            tex = result
        if tex and isinstance(tex, unreal.Texture):
            return tex
    except Exception:
        pass
    return None


def _find_textures_in_dir(game_dir):
    """Return a dict of {asset_name: Texture2D} for all textures in a directory."""
    textures = {}
    try:
        assets = unreal.EditorAssetLibrary.list_assets(
            game_dir, recursive=False, include_folder=False)
    except Exception:
        return textures

    for p in assets:
        asset = unreal.EditorAssetLibrary.load_asset(p)
        if asset and isinstance(asset, unreal.Texture2D):
            textures[asset.get_name()] = asset
    return textures


def _match_texture_by_convention(mic_name, dir_textures):
    """
    Try to find a matching texture for a MIC by naming convention.

    Interchange FBX naming:  Material_0_003  ->  Image_0_003
    The numeric suffix after 'Material_' or 'Image_' is the key.
    """
    suffix = mic_name
    if suffix.startswith("Material_"):
        suffix = suffix[len("Material_"):]

    candidate_name = f"Image_{suffix}"
    if candidate_name in dir_textures:
        return dir_textures[candidate_name]

    for tex_name, tex in dir_textures.items():
        if tex_name.endswith(suffix) and tex_name != mic_name:
            return tex

    return None


def _get_asset_directory(asset_path):
    """Get the /Game/... directory containing an asset."""
    clean = asset_path.split(".")[0] if "." in asset_path else asset_path
    last_slash = clean.rfind("/")
    if last_slash >= 0:
        return clean[:last_slash]
    return clean


def _list_assets_for_scan_roots(scan_roots):
    roots = scan_roots or SCAN_DIRS
    all_paths = []
    seen = set()

    for root in roots:
        if not root:
            continue
        try:
            paths = unreal.EditorAssetLibrary.list_assets(
                root, recursive=True, include_folder=False)
        except Exception:
            continue
        for path in paths:
            if path not in seen:
                seen.add(path)
                all_paths.append(path)
    return all_paths


def convert_character_materials_unlit(scan_roots=None):
    _ensure_master_usage_flags()
    master = unreal.EditorAssetLibrary.load_asset(MASTER_PATH)
    if not master:
        unreal.log_error(f"{LOG} M_Character_Unlit not found at {MASTER_PATH}")
        unreal.log_error(f"{LOG} Cannot proceed — create the master material first.")
        return {
            "converted": 0,
            "already_ok": 0,
            "skipped": 0,
            "no_texture": 0,
            "errors": 1,
            "no_tex_list": [],
        }

    all_mic_paths = _list_assets_for_scan_roots(scan_roots)

    converted = 0
    already_ok = 0
    skipped = 0
    no_texture = 0
    errors = 0
    no_tex_list = []

    dir_texture_cache = {}

    for p in all_mic_paths:
        asset = unreal.EditorAssetLibrary.load_asset(p)
        if not asset or not isinstance(asset, unreal.MaterialInstanceConstant):
            continue

        mic_name = asset.get_name()
        mic_path = asset.get_path_name()
        parent_name = _get_parent_name(asset)

        is_already_unlit = any(u in parent_name for u in ALREADY_UNLIT_PARENTS)
        if is_already_unlit:
            already_ok += 1
            continue

        should_convert = any(t in parent_name for t in PARENTS_TO_CONVERT)
        if not should_convert:
            skipped += 1
            unreal.log(f"  [{mic_name}] Skipped — parent '{parent_name}' not in convert list")
            continue

        tex = _get_texture_from_mic(asset)

        if not tex:
            asset_dir = _get_asset_directory(mic_path)
            if asset_dir not in dir_texture_cache:
                dir_texture_cache[asset_dir] = _find_textures_in_dir(asset_dir)
            dir_textures = dir_texture_cache[asset_dir]

            tex = _match_texture_by_convention(mic_name, dir_textures)

            if tex:
                unreal.log(f"  [{mic_name}] Texture found by directory search: {tex.get_name()}")
            else:
                if len(dir_textures) == 1:
                    tex = next(iter(dir_textures.values()))
                    unreal.log(f"  [{mic_name}] Using only texture in directory: {tex.get_name()}")

        if not tex:
            no_texture += 1
            no_tex_list.append(mic_path)
            unreal.log_warning(f"  [{mic_name}] No texture found — skipping")
            continue

        tex_name = tex.get_name()

        try:
            asset.set_editor_property("parent", master)
        except Exception as e:
            unreal.log_error(f"  [{mic_name}] Reparent failed: {e}")
            errors += 1
            continue

        try:
            unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(
                asset, TEXTURE_PARAM, tex)
        except Exception as e:
            unreal.log_error(f"  [{mic_name}] Failed to set {TEXTURE_PARAM}: {e}")
            errors += 1
            continue

        try:
            unreal.EditorAssetLibrary.save_asset(
                mic_path.split(".")[0] if "." in mic_path else mic_path)
        except Exception as e:
            unreal.log_error(f"  [{mic_name}] Save failed: {e}")
            errors += 1
            continue

        converted += 1
        unreal.log(f"  [{mic_name}] Converted (was: {parent_name}) — texture: {tex_name}")

    return {
        "converted": converted,
        "already_ok": already_ok,
        "skipped": skipped,
        "no_texture": no_texture,
        "errors": errors,
        "no_tex_list": no_tex_list,
    }


def main():
    unreal.log("=" * 60)
    unreal.log(f"{LOG} MakeCharacterMaterialsUnlit START")
    unreal.log("=" * 60)

    unreal.log(f"{LOG} Loaded master: {MASTER_PATH}")

    all_mic_paths = _list_assets_for_scan_roots(None)
    unreal.log(f"{LOG} Found {len(all_mic_paths)} total assets under /Game/Characters/")
    results = convert_character_materials_unlit()

    unreal.log("")
    unreal.log("=" * 60)
    unreal.log(f"{LOG} RESULTS:")
    unreal.log(f"  Converted:       {results['converted']}")
    unreal.log(f"  Already Unlit:   {results['already_ok']}")
    unreal.log(f"  Skipped:         {results['skipped']} (different parent)")
    unreal.log(f"  No texture:      {results['no_texture']}")
    unreal.log(f"  Errors:          {results['errors']}")

    if results["no_tex_list"]:
        unreal.log("")
        unreal.log_warning(f"{LOG} MICs with no texture found (need manual review):")
        for path in results["no_tex_list"]:
            unreal.log_warning(f"  -> {path}")

    unreal.log("=" * 60)


if __name__ == "__main__":
    main()
