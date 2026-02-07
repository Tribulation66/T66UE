"""
Delete ALL companion animation assets (broken imports) so they can be re-imported correctly.
Run this BEFORE re-running ImportCompanions.py.

Run INSIDE Unreal Editor: Tools -> Execute Python Script -> Scripts/DeleteBrokenCompanionAnims.py
"""

import unreal


def main():
    unreal.log("=== DeleteBrokenCompanionAnims: START ===")
    deleted = 0
    
    # Delete ALL companion animation assets - both naming variants
    # ImportCompanions.py will recreate them correctly
    assets_to_delete = []
    
    for i in range(1, 9):  # Companion_01 to Companion_08
        companion_id = f"Companion_{i:02d}"
        base_path = f"/Game/Characters/Companions/{companion_id}"
        
        for skin in ["Default", "Beach"]:
            skin_key = skin if skin == "Default" else "Beach"
            for anim_type in ["Walking", "Alert", "Running"]:
                # Old naming (without _Anim suffix)
                assets_to_delete.append(f"{base_path}/{skin_key}/AM_{companion_id}_{skin_key}_{anim_type}")
                # New naming (with _Anim suffix) - might have wrong internal name
                assets_to_delete.append(f"{base_path}/{skin_key}/AM_{companion_id}_{skin_key}_{anim_type}_Anim")
    
    for asset_path in assets_to_delete:
        if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
            success = unreal.EditorAssetLibrary.delete_asset(asset_path)
            if success:
                unreal.log(f"[Deleted] {asset_path}")
                deleted += 1
            else:
                unreal.log_warning(f"[Failed to delete] {asset_path}")
        # Don't log "not found" - too noisy
    
    unreal.log(f"=== DeleteBrokenCompanionAnims: DONE (deleted {deleted} assets) ===")
    unreal.log("Now run ImportCompanions.py to re-import animations correctly.")


if __name__ == "__main__":
    main()
