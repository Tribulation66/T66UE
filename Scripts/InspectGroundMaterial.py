"""
Inspect the generated ground material to ensure it references the atlas texture.

Run (PowerShell):
& "C:/Program Files/Epic Games/UE_5.7/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" "C:/UE/T66/T66.uproject" -run=pythonscript -script="C:/UE/T66/Scripts/InspectGroundMaterial.py" -unattended -nop4 -nosplash -nullrhi
"""

from __future__ import annotations

import unreal


def main():
    mat_path = "/Game/World/Ground/M_GroundAtlas_2x2.M_GroundAtlas_2x2"
    tex_path = "/Game/World/Ground/T_GroundAtlas_2x2_1024.T_GroundAtlas_2x2_1024"

    mat = unreal.EditorAssetLibrary.load_asset(mat_path)
    tex = unreal.EditorAssetLibrary.load_asset(tex_path)

    unreal.log(f"[InspectGround] Mat loaded: {bool(mat)} ({mat_path})")
    unreal.log(f"[InspectGround] Tex loaded: {bool(tex)} ({tex_path})")

    if not mat:
        return

    mel = unreal.MaterialEditingLibrary
    exprs = mel.get_material_expressions(mat)
    unreal.log(f"[InspectGround] Expressions: {len(exprs)}")

    tex_refs = []
    for e in exprs:
        try:
            cls = e.get_class().get_name()
        except Exception:
            cls = type(e).__name__

        if cls in ("MaterialExpressionTextureSampleParameter2D", "MaterialExpressionTextureSample"):
            t = None
            try:
                t = e.get_editor_property("texture")
            except Exception:
                pass
            pname = None
            try:
                pname = e.get_editor_property("parameter_name")
            except Exception:
                pname = None
            tex_refs.append((cls, str(pname), t.get_path_name() if t else "None"))

    for (cls, pname, tpath) in tex_refs:
        unreal.log(f"[InspectGround] {cls} param={pname} texture={tpath}")

    # Also print which properties are connected.
    try:
        base = mel.get_material_property_input_node(mat, unreal.MaterialProperty.MP_BASE_COLOR)
        emissive = mel.get_material_property_input_node(mat, unreal.MaterialProperty.MP_EMISSIVE_COLOR)
        unreal.log(f"[InspectGround] BaseColor input node: {base.get_name() if base else 'None'}")
        unreal.log(f"[InspectGround] Emissive input node: {emissive.get_name() if emissive else 'None'}")
    except Exception as e:
        unreal.log_warning(f"[InspectGround] Failed to query property inputs: {e}")


if __name__ == "__main__":
    main()

