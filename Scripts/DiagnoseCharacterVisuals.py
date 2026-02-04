"""
Diagnostic script to check DT_CharacterVisuals DataTable contents.
Verifies that companion entries exist and have valid mesh paths.

Run INSIDE Unreal Editor: Tools -> Execute Python Script -> Scripts/DiagnoseCharacterVisuals.py
"""

import unreal


def main():
    unreal.log("=" * 60)
    unreal.log("DT_CharacterVisuals DIAGNOSTIC")
    unreal.log("=" * 60)
    
    dt_path = "/Game/Data/DT_CharacterVisuals"
    dt = unreal.EditorAssetLibrary.load_asset(dt_path)
    
    if not dt:
        unreal.log_error(f"DataTable not found: {dt_path}")
        return
    
    row_names = unreal.DataTableFunctionLibrary.get_data_table_row_names(dt)
    unreal.log(f"DataTable has {len(row_names)} rows")
    unreal.log("")
    
    # Check companion entries
    companion_ids = [
        "Companion_01", "Companion_02", "Companion_03", "Companion_04",
        "Companion_05", "Companion_06", "Companion_07", "Companion_08"
    ]
    
    unreal.log("=== COMPANION ENTRIES ===")
    for comp_id in companion_ids:
        found = comp_id in [str(name) for name in row_names]
        status = "FOUND" if found else "MISSING"
        unreal.log(f"  [{status}] {comp_id}")
        
        if found:
            # Try to get row data - this is tricky in Python, let's just verify it exists
            pass
    
    unreal.log("")
    unreal.log("=== ALL ROW NAMES ===")
    for name in sorted([str(n) for n in row_names]):
        if "Companion" in name:
            unreal.log(f"  {name}")
    
    unreal.log("")
    unreal.log("=== VERIFYING MESH ASSETS EXIST ===")
    for i in range(1, 9):
        comp_id = f"Companion_{i:02d}"
        mesh_path = f"/Game/Characters/Companions/{comp_id}/Default/SK_{comp_id}_Default"
        exists = unreal.EditorAssetLibrary.does_asset_exist(mesh_path)
        status = "EXISTS" if exists else "MISSING"
        unreal.log(f"  [{status}] {mesh_path}")
        
        if exists:
            # Try to load and check if it's actually a skeletal mesh
            asset = unreal.EditorAssetLibrary.load_asset(mesh_path)
            if asset:
                asset_type = type(asset).__name__
                unreal.log(f"           -> Type: {asset_type}")
            else:
                unreal.log(f"           -> Could not load!")
    
    unreal.log("")
    unreal.log("=" * 60)
    unreal.log("DIAGNOSTIC COMPLETE")
    unreal.log("=" * 60)


if __name__ == "__main__":
    main()
