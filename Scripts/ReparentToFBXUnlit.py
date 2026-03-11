"""
Reparent all imported MaterialInstanceConstants to M_FBX_Unlit.

Because M_FBX_Unlit has the SAME parameter names as FBXLegacyPhongSurfaceMaterial,
all texture overrides (DiffuseColorMap, NormalMap, etc.) carry over automatically
when the parent is changed. No manual texture discovery needed.

Also handles MICs currently parented to M_Character_Unlit or M_Environment_Unlit
(from the old conversion pipeline) — reparents them to M_FBX_Unlit so they get
their native textures back.

Run in-editor: Tools > Execute Python Script > Scripts/ReparentToFBXUnlit.py
"""

import unreal

FBX_UNLIT_PATH = "/Game/Materials/M_FBX_Unlit"
LOG_PREFIX = "[ReparentUnlit]"

# Parent material names that should be reparented to M_FBX_Unlit.
PARENTS_TO_CONVERT = [
    "FBXLegacyPhongSurfaceMaterial",
    "M_Character_Unlit",
    "M_Environment_Unlit",
]

# Directories to skip entirely (these don't contain imported model materials).
SKIP_DIRS = [
    "/Game/Materials/",
    "/Game/Lighting/",
    "/Game/UI/",
    "/Game/World/Ground/",
]

# Materials to never touch (master materials, post-process, eclipse, etc.).
SKIP_ASSET_NAMES = [
    "M_FBX_Unlit",
    "M_Character_Unlit",
    "M_Environment_Unlit",
    "M_EclipseCorona",
    "M_PosterizePostProcess",
    "M_PixelationPostProcess",
    "M_GroundAtlas_2x2_R0",
    "M_GroundAtlas_2x2_R90",
    "M_GroundAtlas_2x2_R180",
    "M_GroundAtlas_2x2_R270",
]


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
    if not parent_name:
        return False
    for target in PARENTS_TO_CONVERT:
        if target in parent_name:
            return True
    return False


def _is_already_fbx_unlit(mic):
    try:
        parent = mic.get_editor_property("parent")
        if parent:
            return parent.get_path_name().startswith(FBX_UNLIT_PATH)
    except Exception:
        pass
    return False


def _should_skip_path(asset_path):
    for skip in SKIP_DIRS:
        if asset_path.startswith(skip):
            return True
    return False


def _get_all_mics():
    """Get all MaterialInstanceConstant assets in the project."""
    all_paths = unreal.EditorAssetLibrary.list_assets(
        "/Game/", recursive=True, include_folder=False
    )
    mics = []
    for p in all_paths:
        clean = p.split(".")[0] if "." in p else p
        if _should_skip_path(clean):
            continue
        asset = unreal.EditorAssetLibrary.load_asset(p)
        if asset and isinstance(asset, unreal.MaterialInstanceConstant):
            if asset.get_name() not in SKIP_ASSET_NAMES:
                mics.append(asset)
    return mics


def main():
    unreal.log("=" * 60)
    unreal.log(f"{LOG_PREFIX} ReparentToFBXUnlit START")
    unreal.log("=" * 60)

    fbx_unlit = unreal.EditorAssetLibrary.load_asset(FBX_UNLIT_PATH)
    if not fbx_unlit:
        unreal.log_error(
            f"{LOG_PREFIX} M_FBX_Unlit not found at {FBX_UNLIT_PATH}. "
            "Run CreateFBXUnlitMaterial.py first!"
        )
        return

    unreal.log(f"{LOG_PREFIX} Loaded target parent: {FBX_UNLIT_PATH}")

    all_mics = _get_all_mics()
    unreal.log(f"{LOG_PREFIX} Found {len(all_mics)} MIC candidates")

    converted = 0
    already_ok = 0
    skipped = 0
    errors = 0
    no_texture_warnings = []

    for mic in all_mics:
        mic_name = mic.get_name()
        mic_path = mic.get_path_name()

        if _is_already_fbx_unlit(mic):
            already_ok += 1
            continue

        if not _should_convert(mic):
            skipped += 1
            continue

        old_parent = _get_parent_name(mic)

        # Check if it has a DiffuseColorMap texture BEFORE reparenting (for reporting).
        has_texture = False
        try:
            result = unreal.MaterialEditingLibrary.get_material_instance_texture_parameter_value(
                mic, "DiffuseColorMap"
            )
            tex = result[-1] if isinstance(result, (tuple, list)) and len(result) > 1 else result
            has_texture = tex is not None and isinstance(tex, unreal.Texture)
        except Exception:
            pass

        # Reparent — textures carry over automatically because param names match.
        try:
            mic.set_editor_property("parent", fbx_unlit)
        except Exception as e:
            unreal.log_error(f"  [{mic_name}] Failed to reparent: {e}")
            errors += 1
            continue

        # Save.
        try:
            unreal.EditorAssetLibrary.save_asset(mic_path)
        except Exception as e:
            unreal.log_error(f"  [{mic_name}] Failed to save: {e}")
            errors += 1
            continue

        converted += 1

        if has_texture:
            unreal.log(f"  [{mic_name}] Reparented (was: {old_parent}) — texture preserved")
        else:
            unreal.log_warning(
                f"  [{mic_name}] Reparented (was: {old_parent}) — WARNING: no DiffuseColorMap texture found!"
            )
            no_texture_warnings.append(mic_path)

    unreal.log("")
    unreal.log("=" * 60)
    unreal.log(f"{LOG_PREFIX} RESULTS:")
    unreal.log(f"  Converted:    {converted}")
    unreal.log(f"  Already OK:   {already_ok}")
    unreal.log(f"  Skipped:      {skipped} (different parent, not an import)")
    unreal.log(f"  Errors:       {errors}")

    if no_texture_warnings:
        unreal.log("")
        unreal.log_warning(f"{LOG_PREFIX} {len(no_texture_warnings)} material(s) have NO texture — need manual review:")
        for p in no_texture_warnings:
            unreal.log_warning(f"  -> {p}")

    unreal.log("=" * 60)


if __name__ == "__main__":
    main()
