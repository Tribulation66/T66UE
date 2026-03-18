"""
Make GLB-imported materials Unlit.

GLB imports create MICs parented to MI_Default_Opaque -> M_Default (engine material).
The texture parameter name is "BaseColorTexture".

This script:
  1. Creates M_GLB_Unlit (/Game/Materials/M_GLB_Unlit) — Unlit material with
     BaseColorTexture -> EmissiveColor. Only created once.
  2. Finds all MICs whose parent chain includes MI_Default_Opaque or M_Default
  3. Reads BaseColorTexture from each MIC BEFORE reparenting
  4. Reparents to M_GLB_Unlit
  5. Re-sets BaseColorTexture AFTER reparenting (UE clears overrides on reparent)

Run in-editor: Tools > Execute Python Script > Scripts/MakeGLBImportsUnlit.py

Safe to re-run — skips MICs already parented to M_GLB_Unlit.
"""

import unreal

LOG = "[GLBUnlit]"
MASTER_PATH = "/Game/Materials/M_GLB_Unlit"
MASTER_DIR = "/Game/Materials"
MASTER_NAME = "M_GLB_Unlit"

PARENTS_TO_CONVERT = [
    "MI_Default_Opaque",
    "M_Default",
    "M_FBX_Unlit",
    "M_Character_Unlit",
]

SKIP_DIRS = [
    "/Game/Materials/",
    "/Game/Lighting/",
    "/Game/UI/",
    "/Game/World/Ground/",
]


def _safe_set(obj, prop, value):
    if obj is None:
        return
    try:
        obj.set_editor_property(prop, value)
    except Exception as e:
        unreal.log_warning(f"{LOG} set_editor_property({prop}): {e}")


def _ensure_master_dir():
    try:
        if not unreal.EditorAssetLibrary.does_directory_exist(MASTER_DIR):
            unreal.EditorAssetLibrary.make_directory(MASTER_DIR)
    except Exception:
        pass


def _create_master_asset():
    _ensure_master_dir()
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    factory = unreal.MaterialFactoryNew()
    mat = asset_tools.create_asset(MASTER_NAME, MASTER_DIR, unreal.Material, factory)
    if not mat:
        unreal.log_error(f"{LOG} Failed to create {MASTER_NAME}")
        return None
    return mat


def _recreate_master_material():
    if unreal.EditorAssetLibrary.does_asset_exist(MASTER_PATH):
        if not unreal.EditorAssetLibrary.delete_asset(MASTER_PATH):
            unreal.log_error(f"{LOG} Failed to delete existing {MASTER_PATH}")
            return None
    return _create_master_asset()


# =========================================================================
# Step 1: Create M_GLB_Unlit (if it doesn't exist)
# =========================================================================

def _create_master_material():
    existing = unreal.EditorAssetLibrary.load_asset(MASTER_PATH)
    if existing and isinstance(existing, unreal.Material):
        mat = existing
    else:
        mat = _create_master_asset()
        if not mat:
            return None

    mel = unreal.MaterialEditingLibrary

    needs_refresh = True
    try:
        scalar_params = set(str(name) for name in mel.get_scalar_parameter_names(mat))
        vector_params = set(str(name) for name in mel.get_vector_parameter_names(mat))
        texture_params = set(str(name) for name in mel.get_texture_parameter_names(mat))
        needs_refresh = not (
            "Brightness" in scalar_params
            and "Tint" in vector_params
            and "BaseColorTexture" in texture_params
            and mat.get_editor_property("used_with_instanced_static_meshes")
            and mat.get_editor_property("used_with_nanite")
            and mat.get_editor_property("two_sided")
        )
    except Exception:
        needs_refresh = True

    if not needs_refresh:
        unreal.log(f"{LOG} M_GLB_Unlit already exists, reusing")
        return mat

    if existing and isinstance(existing, unreal.Material):
        unreal.log(f"{LOG} Recreating {MASTER_PATH} to refresh graph safely")
        mat = _recreate_master_material()
        if not mat:
            return None

    _safe_set(mat, "shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    _safe_set(mat, "blend_mode", unreal.BlendMode.BLEND_OPAQUE)
    _safe_set(mat, "two_sided", True)
    _safe_set(mat, "used_with_instanced_static_meshes", True)
    _safe_set(mat, "used_with_nanite", True)

    tex_param = mel.create_material_expression(
        mat, unreal.MaterialExpressionTextureSampleParameter2D, -300, 0)
    tint_param = mel.create_material_expression(
        mat, unreal.MaterialExpressionVectorParameter, -300, 220)
    brightness_param = mel.create_material_expression(
        mat, unreal.MaterialExpressionScalarParameter, -300, 420)
    tint_mul = mel.create_material_expression(
        mat, unreal.MaterialExpressionMultiply, -20, 80)
    bright_mul = mel.create_material_expression(
        mat, unreal.MaterialExpressionMultiply, 220, 80)

    if tex_param:
        _safe_set(tex_param, "parameter_name", "BaseColorTexture")
    if tint_param:
        _safe_set(tint_param, "parameter_name", "Tint")
        _safe_set(tint_param, "default_value", unreal.LinearColor(1.0, 1.0, 1.0, 1.0))
    if brightness_param:
        _safe_set(brightness_param, "parameter_name", "Brightness")
        _safe_set(brightness_param, "default_value", 1.0)

    if tex_param and tint_mul:
        mel.connect_material_expressions(tex_param, "RGB", tint_mul, "A")
    if tint_param and tint_mul:
        mel.connect_material_expressions(tint_param, "", tint_mul, "B")
    if tint_mul and bright_mul:
        mel.connect_material_expressions(tint_mul, "", bright_mul, "A")
    if brightness_param and bright_mul:
        mel.connect_material_expressions(brightness_param, "", bright_mul, "B")
    if bright_mul:
        mel.connect_material_property(bright_mul, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
        unreal.log(f"{LOG} BaseColorTexture * Tint * Brightness -> EmissiveColor connected")

    try:
        mel.layout_material_expressions(mat)
    except Exception:
        pass
    mel.recompile_material(mat)
    unreal.EditorAssetLibrary.save_asset(MASTER_PATH)
    unreal.log(f"{LOG} Created and saved {MASTER_PATH}")
    return mat


def ensure_glb_unlit_master():
    return _create_master_material()


# =========================================================================
# Step 2: Find and convert MICs
# =========================================================================

def _get_parent_name(mic):
    try:
        parent = mic.get_editor_property("parent")
        if parent:
            return parent.get_name()
    except Exception:
        pass
    return ""


def _should_convert(mic):
    parent_name = _get_parent_name(mic)
    for target in PARENTS_TO_CONVERT:
        if target in parent_name:
            return True
    return False


def _is_already_converted(mic):
    try:
        parent = mic.get_editor_property("parent")
        if parent:
            return MASTER_NAME in parent.get_name()
    except Exception:
        pass
    return False


def _should_skip_path(path):
    clean = path.split(".")[0] if "." in path else path
    for d in SKIP_DIRS:
        if clean.startswith(d):
            return True
    return False


def _is_convertible_material_instance(asset):
    if not asset or not isinstance(asset, unreal.MaterialInterface):
        return False
    try:
        asset.get_editor_property("parent")
        return True
    except Exception:
        return False


def _get_texture(mic, param_name):
    params = [param_name]
    if param_name != "DiffuseColorMap":
        params.append("DiffuseColorMap")

    for param in params:
        try:
            result = unreal.MaterialEditingLibrary.get_material_instance_texture_parameter_value(
                mic, param)
            if isinstance(result, (tuple, list)):
                tex = result[-1] if len(result) > 1 else result[0]
            else:
                tex = result
            if tex and isinstance(tex, unreal.Texture):
                return tex
        except Exception:
            pass
    return None


def _list_assets_for_scan_roots(scan_roots):
    roots = scan_roots or ["/Game/"]
    all_paths = []
    seen = set()

    for root in roots:
        if not root:
            continue
        try:
            paths = unreal.EditorAssetLibrary.list_assets(root, recursive=True, include_folder=False)
        except Exception:
            continue
        for path in paths:
            if path not in seen:
                seen.add(path)
                all_paths.append(path)
    return all_paths


def convert_glb_imports_unlit(scan_roots=None):
    master = _create_master_material()
    if not master:
        unreal.log_error(f"{LOG} Cannot proceed without master material")
        return {
            "converted": 0,
            "already_ok": 0,
            "skipped": 0,
            "no_texture": 0,
            "errors": 1,
            "no_tex_list": [],
        }

    all_paths = _list_assets_for_scan_roots(scan_roots)

    converted = 0
    already_ok = 0
    skipped = 0
    no_texture = 0
    errors = 0
    no_tex_list = []

    for p in all_paths:
        if _should_skip_path(p):
            continue

        asset = unreal.EditorAssetLibrary.load_asset(p)
        if not _is_convertible_material_instance(asset):
            continue

        mic_name = asset.get_name()
        mic_path = asset.get_path_name()

        if _is_already_converted(asset):
            already_ok += 1
            continue

        if not _should_convert(asset):
            skipped += 1
            continue

        # Read texture BEFORE reparenting
        tex = _get_texture(asset, "BaseColorTexture")

        if not tex:
            no_texture += 1
            no_tex_list.append(mic_path)
            unreal.log_warning(f"  [{mic_name}] No BaseColorTexture found — skipping")
            continue

        tex_name = tex.get_name()
        old_parent = _get_parent_name(asset)

        # Reparent
        try:
            asset.set_editor_property("parent", master)
        except Exception as e:
            unreal.log_error(f"  [{mic_name}] Reparent failed: {e}")
            errors += 1
            continue

        # Re-set texture AFTER reparenting
        try:
            unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(
                asset, "BaseColorTexture", tex)
        except Exception as e:
            unreal.log_error(f"  [{mic_name}] Failed to re-set texture: {e}")
            errors += 1
            continue

        # Save
        try:
            unreal.EditorAssetLibrary.save_asset(mic_path)
        except Exception as e:
            unreal.log_error(f"  [{mic_name}] Save failed: {e}")
            errors += 1
            continue

        converted += 1
        unreal.log(f"  [{mic_name}] Converted (was: {old_parent}) — texture: {tex_name}")

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
    unreal.log(f"{LOG} MakeGLBImportsUnlit START")
    unreal.log("=" * 60)

    results = convert_glb_imports_unlit()

    unreal.log("")
    unreal.log("=" * 60)
    unreal.log(f"{LOG} RESULTS:")
    unreal.log(f"  Converted:    {results['converted']}")
    unreal.log(f"  Already OK:   {results['already_ok']}")
    unreal.log(f"  Skipped:      {results['skipped']} (different parent)")
    unreal.log(f"  No texture:   {results['no_texture']}")
    unreal.log(f"  Errors:       {results['errors']}")

    if results["no_tex_list"]:
        unreal.log("")
        unreal.log_warning(f"{LOG} MICs with no BaseColorTexture (need manual review):")
        for p in results["no_tex_list"]:
            unreal.log_warning(f"  -> {p}")

    unreal.log("=" * 60)


if __name__ == "__main__":
    main()
