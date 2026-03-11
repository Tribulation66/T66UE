"""
Universal Unlit conversion — works with ANY import pipeline (Interchange, Legacy FBX, etc.)

Two strategies based on material type:

1. Base Materials (Material assets, not MICs):
   - Modified IN-PLACE: set shading model to Unlit, rewire existing texture graph to EmissiveColor
   - No reparenting, no texture loss — the existing connections stay intact

2. MaterialInstanceConstants whose parent is an engine material:
   - Reads ALL texture parameter overrides directly from the MIC
   - Creates a per-parameter-name Unlit parent (cached) with matching param → EmissiveColor
   - Reparents — texture overrides carry over because param names match

Run INSIDE Unreal Editor:
  Tools -> Execute Python Script -> Scripts/MakeAllMaterialsUnlit.py

No prerequisites needed. This script is self-contained.
"""

import unreal

LOG = "[MakeUnlit]"

SKIP_MATERIAL_NAMES = {
    "M_Character_Unlit",
    "M_Environment_Unlit",
    "M_FBX_Unlit",
    "M_PosterizePostProcess",
    "M_PixelationPostProcess",
    "M_EclipseCorona",
    "M_GroundAtlas_2x2_R0",
    "M_GroundAtlas_2x2_R90",
    "M_GroundAtlas_2x2_R180",
    "M_GroundAtlas_2x2_R270",
    "M_PreviewGround",
    "M_PreviewSky",
    "WorldGridMaterial",
    "M_PlaceholderColor",
}

SKIP_DIRS = [
    "/Game/Materials/",
    "/Game/Lighting/",
    "/Game/UI/",
    "/Game/World/Ground/",
]

NORMAL_MAP_HINTS = {"normal", "_n_", "_n.", "_normal", "normalmap", "nrm", "norm"}

# Cache: parameter_name -> Unlit Material asset
_unlit_parent_cache = {}


def _should_skip(path, name):
    if name in SKIP_MATERIAL_NAMES:
        return True
    clean = path.split(".")[0]
    for d in SKIP_DIRS:
        if clean.startswith(d):
            return True
    return False


def _is_normal_texture(tex):
    if not tex:
        return True
    try:
        name = tex.get_name().lower()
        return any(hint in name for hint in NORMAL_MAP_HINTS)
    except Exception:
        return False


# =========================================================================
# Phase 1: Convert base Materials IN-PLACE
# =========================================================================

def _convert_base_material(mat):
    mat_name = mat.get_name()
    mel = unreal.MaterialEditingLibrary

    try:
        shading = mat.get_editor_property("shading_model")
        if shading == unreal.MaterialShadingModel.MSM_UNLIT:
            return "already_unlit"
    except Exception:
        pass

    try:
        domain = mat.get_editor_property("material_domain")
        if domain != unreal.MaterialDomain.MD_SURFACE:
            return "skip_non_surface"
    except Exception:
        pass

    # Find expressions in the material graph
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

    # Find the best texture expression (skip normal maps)
    best_tex = None
    best_multiply = None
    best_vec = None

    for expr in expressions:
        cls = type(expr).__name__
        if "TextureSample" in cls:
            try:
                tex = expr.get_editor_property("texture")
                if tex and not _is_normal_texture(tex):
                    best_tex = expr
            except Exception:
                if best_tex is None:
                    best_tex = expr
        if cls == "MaterialExpressionMultiply" and best_multiply is None:
            best_multiply = expr
        if ("VectorParameter" in cls or "Constant3Vector" in cls) and best_vec is None:
            best_vec = expr

    source = best_tex or best_multiply or best_vec
    output_pin = "RGB" if source == best_tex and best_tex else ""

    try:
        mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
        mat.set_editor_property("two_sided", True)
        if source:
            mel.connect_material_property(source, output_pin, unreal.MaterialProperty.MP_EMISSIVE_COLOR)
        mel.recompile_material(mat)
        unreal.EditorAssetLibrary.save_loaded_asset(mat)
        desc = type(source).__name__ if source else "no source expression"
        return f"converted:{desc}"
    except Exception as e:
        return f"error:{e}"


# =========================================================================
# Phase 2: Convert MICs
# =========================================================================

def _get_mic_texture_overrides(mic):
    """Read ALL texture parameter overrides from a MIC, regardless of parameter name."""
    overrides = []
    try:
        tex_params = mic.get_editor_property("texture_parameter_values")
        if tex_params:
            for tp in tex_params:
                try:
                    param_name = str(tp.get_editor_property("parameter_info").get_editor_property("name"))
                    tex_value = tp.get_editor_property("parameter_value")
                    if tex_value and isinstance(tex_value, unreal.Texture):
                        overrides.append((param_name, tex_value))
                except Exception:
                    continue
    except Exception:
        pass
    return overrides


def _find_texture_in_directory(mat_path):
    """Fallback: find first non-normal Texture2D in the same directory."""
    dir_path = mat_path.split(".")[0].rsplit("/", 1)[0]
    try:
        assets = unreal.EditorAssetLibrary.list_assets(dir_path, recursive=False, include_folder=False)
    except Exception:
        return None
    for p in assets:
        asset = unreal.EditorAssetLibrary.load_asset(p)
        if asset and isinstance(asset, unreal.Texture2D):
            if not _is_normal_texture(asset):
                return asset
    return None


def _get_or_create_unlit_parent(param_name):
    """Get or create an Unlit Material with a TextureSampleParameter2D named param_name → EmissiveColor."""
    if param_name in _unlit_parent_cache:
        cached = _unlit_parent_cache[param_name]
        if cached:
            return cached

    safe_name = "".join(c if c.isalnum() or c == "_" else "_" for c in param_name)
    mat_name = f"M_Unlit_{safe_name}"
    mat_path = f"/Game/Materials/Generated/{mat_name}"

    existing = unreal.EditorAssetLibrary.load_asset(mat_path)
    if existing and isinstance(existing, unreal.Material):
        _unlit_parent_cache[param_name] = existing
        return existing

    mel = unreal.MaterialEditingLibrary
    dest_dir = "/Game/Materials/Generated"
    try:
        if not unreal.EditorAssetLibrary.does_directory_exist(dest_dir):
            unreal.EditorAssetLibrary.make_directory(dest_dir)
    except Exception:
        pass

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    factory = unreal.MaterialFactoryNew()
    mat = asset_tools.create_asset(mat_name, dest_dir, unreal.Material, factory)
    if not mat:
        _unlit_parent_cache[param_name] = None
        return None

    try:
        mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
        mat.set_editor_property("two_sided", True)
    except Exception:
        pass

    try:
        tex_param = mel.create_material_expression(mat, unreal.MaterialExpressionTextureSampleParameter2D, -300, 0)
        if tex_param:
            tex_param.set_editor_property("parameter_name", param_name)
            mel.connect_material_property(tex_param, "RGB", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    except Exception:
        pass

    try:
        mel.recompile_material(mat)
        unreal.EditorAssetLibrary.save_asset(mat_path)
        unreal.log(f"{LOG} Created Unlit parent: {mat_path} (param: {param_name})")
    except Exception as e:
        unreal.log_error(f"{LOG} Failed to create {mat_path}: {e}")
        _unlit_parent_cache[param_name] = None
        return None

    _unlit_parent_cache[param_name] = mat
    return mat


def _is_already_unlit_mic(mic):
    """Check if MIC's parent is already an Unlit material we created."""
    try:
        parent = mic.get_editor_property("parent")
        if not parent:
            return False
        path = parent.get_path_name()
        if "/Game/Materials/Generated/" in path:
            return True
        shading = parent.get_editor_property("shading_model")
        return shading == unreal.MaterialShadingModel.MSM_UNLIT
    except Exception:
        return False


def _convert_mic(mic):
    mic_name = mic.get_name()
    mic_path = mic.get_path_name()

    if _is_already_unlit_mic(mic):
        return "already_unlit"

    # Read all texture overrides from the MIC
    tex_overrides = _get_mic_texture_overrides(mic)

    # Pick the primary diffuse texture (first non-normal)
    primary_param = None
    primary_tex = None
    for pname, tex in tex_overrides:
        if not _is_normal_texture(tex):
            primary_param = pname
            primary_tex = tex
            break

    # Fallback: if no texture override found, look in the directory
    if not primary_tex:
        dir_tex = _find_texture_in_directory(mic_path)
        if dir_tex:
            primary_param = "DiffuseColorMap"
            primary_tex = dir_tex

    if not primary_tex:
        return "no_texture"

    # Get or create an Unlit parent with matching parameter name
    unlit_parent = _get_or_create_unlit_parent(primary_param)
    if not unlit_parent:
        return "error:could not create unlit parent"

    try:
        mic.set_editor_property("parent", unlit_parent)
        # If param name from override differs from the parent param, set it explicitly
        unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(
            mic, primary_param, primary_tex
        )
        unreal.EditorAssetLibrary.save_asset(mic_path)
        return f"converted:{primary_param}={primary_tex.get_name()}"
    except Exception as e:
        return f"error:{e}"


# =========================================================================
# Main
# =========================================================================

def main():
    unreal.log("=" * 60)
    unreal.log(f"{LOG} MakeAllMaterialsUnlit START")
    unreal.log("=" * 60)

    # ---- Phase 1: Base Materials (in-place conversion) ----
    unreal.log(f"\n{LOG} Phase 1: Converting base Materials in-place ...")

    all_paths = unreal.EditorAssetLibrary.list_assets("/Game/", recursive=True, include_folder=False)

    base_converted = 0
    base_already = 0
    base_skipped = 0
    base_errors = 0

    for p in all_paths:
        asset = unreal.EditorAssetLibrary.load_asset(p)
        if not asset or not isinstance(asset, unreal.Material):
            continue

        name = asset.get_name()
        clean_path = p.split(".")[0] if "." in p else p

        if _should_skip(clean_path, name):
            continue

        result = _convert_base_material(asset)

        if result == "already_unlit":
            base_already += 1
        elif result == "skip_non_surface":
            base_skipped += 1
        elif result.startswith("converted"):
            base_converted += 1
            unreal.log(f"  [MAT OK] {name} — {result}")
        else:
            base_errors += 1
            unreal.log_error(f"  [MAT ERR] {name} — {result}")

    unreal.log(f"\n{LOG} Base Materials: converted={base_converted}, already_unlit={base_already}, skipped={base_skipped}, errors={base_errors}")

    # ---- Phase 2: MaterialInstanceConstants ----
    unreal.log(f"\n{LOG} Phase 2: Converting MaterialInstanceConstants ...")

    mic_converted = 0
    mic_already = 0
    mic_no_tex = 0
    mic_errors = 0
    no_tex_list = []

    for p in all_paths:
        asset = unreal.EditorAssetLibrary.load_asset(p)
        if not asset or not isinstance(asset, unreal.MaterialInstanceConstant):
            continue

        name = asset.get_name()
        clean_path = p.split(".")[0] if "." in p else p

        if _should_skip(clean_path, name):
            continue

        result = _convert_mic(asset)

        if result == "already_unlit":
            mic_already += 1
        elif result == "no_texture":
            mic_no_tex += 1
            no_tex_list.append(clean_path)
            unreal.log_warning(f"  [MIC WARN] {name} — no texture found, needs manual review")
        elif result.startswith("converted"):
            mic_converted += 1
            unreal.log(f"  [MIC OK] {name} — {result}")
        else:
            mic_errors += 1
            unreal.log_error(f"  [MIC ERR] {name} — {result}")

    unreal.log(f"\n{LOG} MICs: converted={mic_converted}, already_unlit={mic_already}, no_texture={mic_no_tex}, errors={mic_errors}")

    if no_tex_list:
        unreal.log("")
        unreal.log_warning(f"{LOG} {len(no_tex_list)} MIC(s) have NO texture — need manual review or reimport:")
        for p in no_tex_list:
            unreal.log_warning(f"  -> {p}")

    unreal.log("")
    unreal.log("=" * 60)
    unreal.log(f"{LOG} DONE — Converted {base_converted} base Materials + {mic_converted} MICs")
    unreal.log("=" * 60)


if __name__ == "__main__":
    main()
