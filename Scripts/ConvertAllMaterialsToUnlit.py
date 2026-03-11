"""
Comprehensive batch-convert: make EVERY material in /Game/ use the Unlit shading model.

Handles both:
  - MaterialInstanceConstants (reparented to M_Character_Unlit or M_Environment_Unlit)
  - Base Materials (shading model set to Unlit, BaseColor connection moved to EmissiveColor)

Skips materials that should remain untouched:
  - Master materials (M_Character_Unlit, M_Environment_Unlit)
  - Post-process materials (M_PosterizePostProcess)
  - Eclipse corona material (M_EclipseCorona)
  - Any material already Unlit

Run INSIDE Unreal Editor:
  Tools -> Execute Python Script -> Scripts/ConvertAllMaterialsToUnlit.py

Prerequisites:
  Run CreateCharacterUnlitMaterial.py and CreateEnvironmentUnlitMaterial.py first.
"""

import unreal

LOG = "[ConvertAll]"

CHAR_UNLIT_PATH = "/Game/Materials/M_Character_Unlit"
ENV_UNLIT_PATH = "/Game/Materials/M_Environment_Unlit"

CHARACTER_DIRS = [
    "/Game/Characters/Heroes/",
    "/Game/Characters/Companions/",
    "/Game/Characters/Enemies/",
    "/Game/Characters/NPCs/",
]

SKIP_MATERIAL_PATHS = {
    "/Game/Materials/M_Character_Unlit",
    "/Game/Materials/M_Environment_Unlit",
    "/Game/UI/M_PosterizePostProcess",
    "/Game/Lighting/M_EclipseCorona",
}

NORMAL_MAP_HINTS = {"normal", "_n_", "_n.", "_normal", "normalmap", "nrm"}


def _should_skip_material_path(path: str) -> bool:
    p = path.split(".")[0]
    for skip in SKIP_MATERIAL_PATHS:
        if p == skip or p.startswith(skip + "."):
            return True
    return False


def _is_in_character_dir(path: str) -> bool:
    p = path.replace("\\", "/")
    for d in CHARACTER_DIRS:
        if d in p:
            return True
    return False


def _asset_data_to_path(asset_data) -> str:
    try:
        return str(asset_data.object_path)
    except Exception:
        pass
    try:
        return str(asset_data.get_soft_object_path())
    except Exception:
        pass
    try:
        pkg = str(asset_data.package_name)
        name = str(asset_data.asset_name)
        return f"{pkg}.{name}"
    except Exception:
        return ""


def _is_unlit_parent(mat_instance) -> bool:
    try:
        parent = mat_instance.get_editor_property("parent")
    except Exception:
        return False
    if not parent:
        return False
    try:
        path = parent.get_path_name()
        return path.startswith(CHAR_UNLIT_PATH) or path.startswith(ENV_UNLIT_PATH)
    except Exception:
        return False


def _get_first_texture_param(mat_instance, candidate_names):
    for param_name in candidate_names:
        try:
            result = unreal.MaterialEditingLibrary.get_material_instance_texture_parameter_value(
                mat_instance, param_name
            )
            tex = result[-1] if isinstance(result, (tuple, list)) and len(result) > 1 else (
                result[0] if isinstance(result, (tuple, list)) else result
            )
            if tex and isinstance(tex, unreal.Texture):
                return param_name, tex
        except Exception:
            pass
    return None, None


def _get_first_vector_param(mat_instance, candidate_names):
    for param_name in candidate_names:
        try:
            result = unreal.MaterialEditingLibrary.get_material_instance_vector_parameter_value(
                mat_instance, param_name
            )
            if result is not None:
                return param_name, result
        except Exception:
            pass
    return None, None


def _find_texture_in_directory(mat_path: str):
    package_path = mat_path.split(".")[0]
    dir_path = package_path.rsplit("/", 1)[0]
    try:
        asset_paths = unreal.EditorAssetLibrary.list_assets(dir_path, recursive=False, include_folder=False)
    except Exception:
        return None
    for p in asset_paths:
        asset = unreal.EditorAssetLibrary.load_asset(p)
        if asset and isinstance(asset, unreal.Texture2D):
            name_lower = asset.get_name().lower()
            if not any(hint in name_lower for hint in NORMAL_MAP_HINTS):
                return asset
    return None


# ---------------------------------------------------------------------------
# MIC conversion
# ---------------------------------------------------------------------------

def _convert_mic(mat_instance, char_parent, env_parent):
    mat_path = mat_instance.get_path_name()
    mat_name = mat_instance.get_name()

    if _is_unlit_parent(mat_instance):
        return "skip_already_unlit", None

    target_parent = char_parent if _is_in_character_dir(mat_path) else env_parent

    diffuse_param_name, diffuse_tex = _get_first_texture_param(
        mat_instance, ["DiffuseColorMap", "BaseColorMap", "BaseColorTexture"]
    )
    source_desc = diffuse_param_name or ""

    if not diffuse_tex:
        diffuse_tex = _find_texture_in_directory(mat_path)
        if diffuse_tex:
            source_desc = f"(dir: {diffuse_tex.get_name()})"

    tint_name, tint_value = _get_first_vector_param(
        mat_instance, ["DiffuseColor", "BaseColor", "Color", "Tint"]
    )

    if not diffuse_tex and not tint_value:
        return "skip_no_data", None

    try:
        mat_instance.set_editor_property("parent", target_parent)
        if diffuse_tex:
            unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(
                mat_instance, "DiffuseColorMap", diffuse_tex
            )
        if tint_value is not None and target_parent == env_parent:
            unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(
                mat_instance, "Tint", tint_value
            )
        unreal.EditorAssetLibrary.save_asset(mat_path)
        tex_info = diffuse_tex.get_name() if diffuse_tex else f"tint={tint_name}"
        return "converted", f"{tex_info} ({source_desc})"
    except Exception as e:
        return "error", str(e)


# ---------------------------------------------------------------------------
# Base Material conversion
# ---------------------------------------------------------------------------

def _is_normal_texture(tex) -> bool:
    if not tex:
        return True
    try:
        name = tex.get_name().lower()
        return any(hint in name for hint in NORMAL_MAP_HINTS)
    except Exception:
        return False


def _convert_base_material(mat):
    """Convert a base Material to Unlit by moving BaseColor graph → EmissiveColor."""
    mat_name = mat.get_name()
    mel = unreal.MaterialEditingLibrary

    try:
        shading = mat.get_editor_property("shading_model")
        if shading == unreal.MaterialShadingModel.MSM_UNLIT:
            return "skip_already_unlit", None
    except Exception:
        pass

    try:
        domain = mat.get_editor_property("material_domain")
        if domain != unreal.MaterialDomain.MD_SURFACE:
            return "skip_non_surface", str(domain)
    except Exception:
        pass

    expressions = []
    for prop_name in ("expressions", "expression_collection"):
        try:
            val = mat.get_editor_property(prop_name)
            if val is not None:
                if hasattr(val, "expressions"):
                    expressions = val.expressions or []
                elif hasattr(val, '__iter__'):
                    expressions = list(val)
                break
        except Exception:
            continue

    best_tex_expr = None
    best_vec_expr = None
    best_multiply = None

    for expr in expressions:
        cls_name = type(expr).__name__

        if "TextureSample" in cls_name:
            try:
                tex = expr.get_editor_property("texture")
                if tex and not _is_normal_texture(tex):
                    best_tex_expr = expr
            except Exception:
                if best_tex_expr is None:
                    best_tex_expr = expr

        if cls_name in ("MaterialExpressionMultiply",) and best_multiply is None:
            best_multiply = expr

        if "VectorParameter" in cls_name and best_vec_expr is None:
            best_vec_expr = expr
        if "Constant3Vector" in cls_name and best_vec_expr is None:
            best_vec_expr = expr

    source_expr = best_tex_expr or best_multiply or best_vec_expr
    source_output = "RGB" if best_tex_expr and source_expr == best_tex_expr else ""

    if not source_expr:
        try:
            mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
            mat.set_editor_property("two_sided", True)
            mel.recompile_material(mat)
            unreal.EditorAssetLibrary.save_loaded_asset(mat)
            return "converted_no_source", "set Unlit but no expression to connect"
        except Exception as e:
            return "error", str(e)

    try:
        mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
        mat.set_editor_property("two_sided", True)
        mel.connect_material_property(source_expr, source_output, unreal.MaterialProperty.MP_EMISSIVE_COLOR)
        mel.recompile_material(mat)
        unreal.EditorAssetLibrary.save_loaded_asset(mat)
        return "converted", f"connected {type(source_expr).__name__} -> EmissiveColor"
    except Exception as e:
        return "error", str(e)


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    unreal.log("=" * 60)
    unreal.log(f"{LOG} ConvertAllMaterialsToUnlit START")
    unreal.log("=" * 60)

    char_parent = unreal.EditorAssetLibrary.load_asset(CHAR_UNLIT_PATH)
    env_parent = unreal.EditorAssetLibrary.load_asset(ENV_UNLIT_PATH)
    if not char_parent or not env_parent:
        unreal.log_error(f"{LOG} Master materials not found. Run CreateCharacterUnlitMaterial.py and CreateEnvironmentUnlitMaterial.py first.")
        return

    # ---- Phase 1: MaterialInstanceConstants ----
    unreal.log(f"\n{LOG} Phase 1: Converting MaterialInstanceConstants ...")

    mic_paths = []
    try:
        registry = unreal.AssetRegistryHelpers.get_asset_registry()
        for ad in registry.get_assets_by_class("MaterialInstanceConstant", True):
            p = _asset_data_to_path(ad)
            if p and "/Game/" in p:
                mic_paths.append(p)
    except Exception:
        all_paths = unreal.EditorAssetLibrary.list_assets("/Game/", recursive=True, include_folder=False)
        for p in all_paths:
            a = unreal.EditorAssetLibrary.load_asset(p)
            if a and isinstance(a, unreal.MaterialInstanceConstant):
                mic_paths.append(p)

    mic_converted = 0
    mic_skip_unlit = 0
    mic_skip_nodata = 0
    mic_errors = 0

    for p in mic_paths:
        if _should_skip_material_path(p):
            continue
        mat = unreal.EditorAssetLibrary.load_asset(p)
        if not mat or not isinstance(mat, unreal.MaterialInstanceConstant):
            continue
        result, detail = _convert_mic(mat, char_parent, env_parent)
        name = mat.get_name()
        if result == "converted":
            mic_converted += 1
            unreal.log(f"  [MIC OK] {name} — {detail}")
        elif result == "skip_already_unlit":
            mic_skip_unlit += 1
        elif result == "skip_no_data":
            mic_skip_nodata += 1
            unreal.log_warning(f"  [MIC SKIP] {name} — no texture or color found")
        else:
            mic_errors += 1
            unreal.log_error(f"  [MIC ERR] {name} — {detail}")

    unreal.log(f"\n{LOG} MIC results: converted={mic_converted}, already_unlit={mic_skip_unlit}, no_data={mic_skip_nodata}, errors={mic_errors}")

    # ---- Phase 2: Base Materials ----
    unreal.log(f"\n{LOG} Phase 2: Converting base Materials ...")

    base_mat_paths = []
    try:
        registry = unreal.AssetRegistryHelpers.get_asset_registry()
        for ad in registry.get_assets_by_class("Material", True):
            p = _asset_data_to_path(ad)
            if p and "/Game/" in p:
                base_mat_paths.append(p)
    except Exception:
        all_paths = unreal.EditorAssetLibrary.list_assets("/Game/", recursive=True, include_folder=False)
        for p in all_paths:
            a = unreal.EditorAssetLibrary.load_asset(p)
            if a and isinstance(a, unreal.Material):
                base_mat_paths.append(p)

    bm_converted = 0
    bm_skip_unlit = 0
    bm_skip_other = 0
    bm_errors = 0

    for p in base_mat_paths:
        if _should_skip_material_path(p):
            continue
        mat = unreal.EditorAssetLibrary.load_asset(p)
        if not mat or not isinstance(mat, unreal.Material):
            continue
        result, detail = _convert_base_material(mat)
        name = mat.get_name()
        if "converted" in result:
            bm_converted += 1
            unreal.log(f"  [MAT OK] {name} — {detail}")
        elif result == "skip_already_unlit":
            bm_skip_unlit += 1
        elif result.startswith("skip"):
            bm_skip_other += 1
            unreal.log(f"  [MAT SKIP] {name} — {result}: {detail}")
        else:
            bm_errors += 1
            unreal.log_error(f"  [MAT ERR] {name} — {detail}")

    unreal.log(f"\n{LOG} Base Material results: converted={bm_converted}, already_unlit={bm_skip_unlit}, skipped={bm_skip_other}, errors={bm_errors}")

    unreal.log("=" * 60)
    unreal.log(f"{LOG} DONE — Total converted: {mic_converted} MICs + {bm_converted} base Materials")
    unreal.log("=" * 60)


if __name__ == "__main__":
    main()
