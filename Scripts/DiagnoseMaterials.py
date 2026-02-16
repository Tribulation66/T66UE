"""
Diagnostic script to compare Hero vs Companion material properties.
Helps identify why companions might appear dark/unlit.

Run INSIDE Unreal Editor: Tools -> Execute Python Script -> Scripts/DiagnoseMaterials.py
"""

import unreal


def log_material_info(material_path, label):
    """Load a material and log its properties."""
    mat = unreal.EditorAssetLibrary.load_asset(material_path)
    if not mat:
        unreal.log_warning(f"[{label}] Material not found: {material_path}")
        return
    
    unreal.log(f"[{label}] Material: {material_path}")
    
    # Check if it's a Material Instance or base Material
    mat_type = type(mat).__name__
    unreal.log(f"  Type: {mat_type}")
    
    if isinstance(mat, unreal.MaterialInterface):
        # Get the base material
        base_mat = mat.get_base_material() if hasattr(mat, 'get_base_material') else mat
        if base_mat:
            try:
                # Try to get shading model
                shading_model = base_mat.get_editor_property('shading_model') if hasattr(base_mat, 'get_editor_property') else "Unknown"
                unreal.log(f"  Shading Model: {shading_model}")
            except Exception as e:
                unreal.log(f"  Shading Model: Could not read ({e})")
            
            try:
                blend_mode = base_mat.get_editor_property('blend_mode') if hasattr(base_mat, 'get_editor_property') else "Unknown"
                unreal.log(f"  Blend Mode: {blend_mode}")
            except Exception:
                pass
    
    unreal.log("")


def log_skeletal_mesh_materials(mesh_path, label):
    """Load a skeletal mesh and log its material slots."""
    mesh = unreal.EditorAssetLibrary.load_asset(mesh_path)
    if not mesh:
        unreal.log_warning(f"[{label}] Skeletal Mesh not found: {mesh_path}")
        return
    
    unreal.log(f"[{label}] Skeletal Mesh: {mesh_path}")
    
    if isinstance(mesh, unreal.SkeletalMesh):
        materials = mesh.get_editor_property('materials')
        if materials:
            unreal.log(f"  Material Slots: {len(materials)}")
            for i, mat_slot in enumerate(materials):
                mat_interface = mat_slot.get_editor_property('material_interface') if mat_slot else None
                if mat_interface:
                    mat_name = mat_interface.get_name()
                    mat_path = mat_interface.get_path_name()
                    unreal.log(f"  Slot {i}: {mat_name}")
                    # Get base material shading model
                    base_mat = mat_interface.get_base_material() if hasattr(mat_interface, 'get_base_material') else None
                    if base_mat:
                        try:
                            shading_model = base_mat.get_editor_property('shading_model')
                            unreal.log(f"    -> Shading Model: {shading_model}")
                        except Exception as e:
                            unreal.log(f"    -> Shading Model: Could not read")
                else:
                    unreal.log(f"  Slot {i}: (empty)")
        else:
            unreal.log(f"  No materials found")
    
    unreal.log("")


def main():
    unreal.log("=" * 60)
    unreal.log("MATERIAL DIAGNOSTICS: Hero vs Companion")
    unreal.log("=" * 60)
    
    # Check Hero skeletal meshes and materials (Hero_1 = Knight, uses Hero2 mesh path)
    unreal.log("")
    unreal.log("=== HERO MATERIALS ===")
    log_skeletal_mesh_materials(
        "/Game/Characters/Heroes/Hero2/KnightIdle/KnightIdle.KnightIdle",
        "Hero_1 (Knight) TypeA"
    )
    
    # Check Companion skeletal meshes and materials
    unreal.log("")
    unreal.log("=== COMPANION MATERIALS ===")
    log_skeletal_mesh_materials(
        "/Game/Characters/Companions/Companion_01/Default/SK_Companion_01_Default",
        "Companion_01 Default"
    )
    log_skeletal_mesh_materials(
        "/Game/Characters/Companions/Companion_03/Default/SK_Companion_03_Default",
        "Companion_03 Default"
    )
    
    # Direct material comparison (Hero_1 uses Hero2 path; adjust if Knight has a material asset)
    unreal.log("")
    unreal.log("=== DIRECT MATERIAL COMPARISON ===")
    log_material_info(
        "/Game/Characters/Heroes/Hero2/KnightIdle/Material_1",
        "Hero Material"
    )
    log_material_info(
        "/Game/Characters/Companions/Companion_01/Default/Material_1",
        "Companion Material"
    )
    
    unreal.log("=" * 60)
    unreal.log("DIAGNOSTICS COMPLETE - Check Output Log for results")
    unreal.log("=" * 60)


if __name__ == "__main__":
    main()
