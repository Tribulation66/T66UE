"""
Make all GLB-imported materials Unlit.

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

PARENTS_TO_CONVERT = ["MI_Default_Opaque", "M_Default"]

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


# =========================================================================
# Step 1: Create M_GLB_Unlit (if it doesn't exist)
# =========================================================================

def _create_master_material():
    existing = unreal.EditorAssetLibrary.load_asset(MASTER_PATH)
    if existing and isinstance(existing, unreal.Material):
        unreal.log(f"{LOG} M_GLB_Unlit already exists, reusing")
        return existing

    try:
        if not unreal.EditorAssetLibrary.does_directory_exist(MASTER_DIR):
            unreal.EditorAssetLibrary.make_directory(MASTER_DIR)
    except Exception:
        pass

    mel = unreal.MaterialEditingLibrary
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    factory = unreal.MaterialFactoryNew()
    mat = asset_tools.create_asset(MASTER_NAME, MASTER_DIR, unreal.Material, factory)
    if not mat:
        unreal.log_error(f"{LOG} Failed to create {MASTER_NAME}")
        return None

    _safe_set(mat, "shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    _safe_set(mat, "two_sided", True)

    tex_param = mel.create_material_expression(
        mat, unreal.MaterialExpressionTextureSampleParameter2D, -300, 0)
    if tex_param:
        _safe_set(tex_param, "parameter_name", "BaseColorTexture")
        mel.connect_material_property(tex_param, "RGB", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
        unreal.log(f"{LOG} BaseColorTexture -> EmissiveColor connected")

    mel.recompile_material(mat)
    unreal.EditorAssetLibrary.save_asset(MASTER_PATH)
    unreal.log(f"{LOG} Created and saved {MASTER_PATH}")
    return mat


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


def _get_texture(mic, param_name):
    try:
        result = unreal.MaterialEditingLibrary.get_material_instance_texture_parameter_value(
            mic, param_name)
        if isinstance(result, (tuple, list)):
            tex = result[-1] if len(result) > 1 else result[0]
        else:
            tex = result
        if tex and isinstance(tex, unreal.Texture):
            return tex
    except Exception:
        pass
    return None


def main():
    unreal.log("=" * 60)
    unreal.log(f"{LOG} MakeGLBImportsUnlit START")
    unreal.log("=" * 60)

    # Step 1: Create master material
    master = _create_master_material()
    if not master:
        unreal.log_error(f"{LOG} Cannot proceed without master material")
        return

    # Step 2: Find all MICs
    all_paths = unreal.EditorAssetLibrary.list_assets("/Game/", recursive=True, include_folder=False)

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
        if not asset or not isinstance(asset, unreal.MaterialInstanceConstant):
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

    unreal.log("")
    unreal.log("=" * 60)
    unreal.log(f"{LOG} RESULTS:")
    unreal.log(f"  Converted:    {converted}")
    unreal.log(f"  Already OK:   {already_ok}")
    unreal.log(f"  Skipped:      {skipped} (different parent)")
    unreal.log(f"  No texture:   {no_texture}")
    unreal.log(f"  Errors:       {errors}")

    if no_tex_list:
        unreal.log("")
        unreal.log_warning(f"{LOG} MICs with no BaseColorTexture (need manual review):")
        for p in no_tex_list:
            unreal.log_warning(f"  -> {p}")

    unreal.log("=" * 60)


if __name__ == "__main__":
    main()
