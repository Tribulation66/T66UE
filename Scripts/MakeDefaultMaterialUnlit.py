"""
Make M_Default Unlit — modifies the GLB/glTF import base material in-place.

M_Default is the root parent of MI_Default_Opaque, which is the parent of
all GLB-imported MaterialInstanceConstants. By changing M_Default to Unlit
and rewiring its BaseColor texture to EmissiveColor, every prop/interactable
imported through GLB becomes Unlit instantly — no reparenting, no texture loss.

Run in-editor: Tools > Execute Python Script > Scripts/MakeDefaultMaterialUnlit.py
"""

import unreal

LOG = "[MakeDefaultUnlit]"
M_DEFAULT_PATH = "/Game/M_Default"


def main():
    unreal.log("=" * 60)
    unreal.log(f"{LOG} START")
    unreal.log("=" * 60)

    mel = unreal.MaterialEditingLibrary

    # Try multiple possible paths for M_Default
    mat = None
    paths_to_try = [
        "/Game/M_Default",
        "/Game/M_Default.M_Default",
    ]

    # Also search for it if not at expected path
    for p in paths_to_try:
        mat = unreal.EditorAssetLibrary.load_asset(p)
        if mat and isinstance(mat, unreal.Material):
            unreal.log(f"{LOG} Found M_Default at: {p}")
            break
        mat = None

    if not mat:
        # Search the whole project for it
        unreal.log(f"{LOG} M_Default not at expected paths, searching project...")
        all_paths = unreal.EditorAssetLibrary.list_assets("/Game/", recursive=True, include_folder=False)
        for p in all_paths:
            asset = unreal.EditorAssetLibrary.load_asset(p)
            if asset and isinstance(asset, unreal.Material) and asset.get_name() == "M_Default":
                mat = asset
                unreal.log(f"{LOG} Found M_Default at: {p}")
                break

    if not mat:
        unreal.log_error(f"{LOG} M_Default not found anywhere in /Game/")
        return

    # Check current shading model
    try:
        current_shading = mat.get_editor_property("shading_model")
        unreal.log(f"{LOG} Current shading model: {current_shading}")
        if current_shading == unreal.MaterialShadingModel.MSM_UNLIT:
            unreal.log(f"{LOG} Already Unlit — nothing to do.")
            return
    except Exception as e:
        unreal.log_warning(f"{LOG} Could not read shading model: {e}")

    # Find all expressions in the material graph
    expressions = []
    for prop_name in ("expressions", "expression_collection"):
        try:
            val = mat.get_editor_property(prop_name)
            if val is not None:
                if hasattr(val, "expressions"):
                    expressions = list(val.expressions or [])
                elif hasattr(val, '__iter__'):
                    expressions = list(val)
                break
        except Exception:
            continue

    unreal.log(f"{LOG} Found {len(expressions)} expressions in material graph")

    # Find the texture expression that drives BaseColor
    # Look for TextureSample or TextureSampleParameter2D nodes
    best_tex_expr = None
    best_multiply = None

    for expr in expressions:
        cls_name = type(expr).__name__
        unreal.log(f"{LOG}   Expression: {cls_name}")

        if "TextureSample" in cls_name:
            best_tex_expr = expr
        if cls_name == "MaterialExpressionMultiply" and best_multiply is None:
            best_multiply = expr

    # Prefer Multiply (in case texture is multiplied by something) over raw texture
    source_expr = best_multiply or best_tex_expr
    source_output = ""

    if source_expr == best_tex_expr and best_tex_expr:
        source_output = "RGB"

    if not source_expr:
        unreal.log_warning(f"{LOG} No texture/multiply expression found — setting Unlit anyway")

    # Apply changes
    try:
        mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
        unreal.log(f"{LOG} Set shading model to Unlit")
    except Exception as e:
        unreal.log_error(f"{LOG} Failed to set shading model: {e}")
        return

    try:
        mat.set_editor_property("two_sided", True)
        unreal.log(f"{LOG} Set two_sided to True")
    except Exception as e:
        unreal.log_warning(f"{LOG} Could not set two_sided: {e}")

    if source_expr:
        try:
            mel.connect_material_property(
                source_expr, source_output,
                unreal.MaterialProperty.MP_EMISSIVE_COLOR
            )
            unreal.log(f"{LOG} Connected {type(source_expr).__name__} -> EmissiveColor")
        except Exception as e:
            unreal.log_error(f"{LOG} Failed to connect to EmissiveColor: {e}")
            return

    # Recompile and save
    try:
        mel.recompile_material(mat)
        unreal.log(f"{LOG} Material recompiled")
    except Exception as e:
        unreal.log_error(f"{LOG} Recompile failed: {e}")
        return

    try:
        unreal.EditorAssetLibrary.save_loaded_asset(mat)
        unreal.log(f"{LOG} Saved")
    except Exception as e:
        unreal.log_error(f"{LOG} Save failed: {e}")
        return

    unreal.log("=" * 60)
    unreal.log(f"{LOG} DONE — M_Default is now Unlit")
    unreal.log(f"{LOG} All GLB-imported props/interactables using MI_Default_Opaque")
    unreal.log(f"{LOG} will now render Unlit with their original textures.")
    unreal.log("=" * 60)


if __name__ == "__main__":
    main()
