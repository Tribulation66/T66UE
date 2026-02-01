"""
Verify T66 Setup - Check all assets and configurations
"""

import unreal

def check_asset_exists(asset_path):
    """Check if asset exists and return status"""
    exists = unreal.EditorAssetLibrary.does_asset_exist(asset_path)
    return "OK" if exists else "MISSING"

def main():
    print("=" * 70)
    print("T66 SETUP VERIFICATION")
    print("=" * 70)

    # Core Blueprints
    print("")
    print("=== CORE BLUEPRINTS ===")
    core_bps = [
        "/Game/Blueprints/Core/BP_T66GameInstance",
        "/Game/Blueprints/Core/BP_T66PlayerController",
        "/Game/Blueprints/Core/BP_HeroBase",
    ]
    for path in core_bps:
        status = check_asset_exists(path)
        print(f"  [{status}] {path}")

    # GameModes
    print("")
    print("=== GAMEMODES ===")
    gamemodes = [
        "/Game/Blueprints/GameModes/BP_FrontendGameMode",
        "/Game/Blueprints/GameModes/BP_GameplayGameMode",
    ]
    for path in gamemodes:
        status = check_asset_exists(path)
        print(f"  [{status}] {path}")

    # UI Widgets
    print("")
    print("=== UI WIDGET BLUEPRINTS ===")
    widgets = [
        "/Game/Blueprints/UI/WBP_MainMenu",
        "/Game/Blueprints/UI/WBP_PartySizePicker",
        "/Game/Blueprints/UI/WBP_HeroSelection",
        "/Game/Blueprints/UI/WBP_CompanionSelection",
        "/Game/Blueprints/UI/WBP_SaveSlots",
        "/Game/Blueprints/UI/WBP_Settings",
        "/Game/Blueprints/UI/WBP_QuitConfirmation",
    ]
    for path in widgets:
        status = check_asset_exists(path)
        print(f"  [{status}] {path}")

    # DataTables
    print("")
    print("=== DATATABLES ===")
    dt_heroes = unreal.EditorAssetLibrary.load_asset("/Game/Data/DT_Heroes")
    if dt_heroes:
        row_names = unreal.DataTableFunctionLibrary.get_data_table_row_names(dt_heroes)
        print(f"  [OK] DT_Heroes - {len(row_names)} rows: {', '.join([str(n) for n in row_names])}")
    else:
        print("  [MISSING] DT_Heroes")

    dt_companions = unreal.EditorAssetLibrary.load_asset("/Game/Data/DT_Companions")
    if dt_companions:
        row_names = unreal.DataTableFunctionLibrary.get_data_table_row_names(dt_companions)
        print(f"  [OK] DT_Companions - {len(row_names)} rows: {', '.join([str(n) for n in row_names])}")
    else:
        print("  [MISSING] DT_Companions")

    # Levels
    print("")
    print("=== LEVELS ===")
    levels = [
        "/Game/Maps/FrontendLevel",
        "/Game/Maps/GameplayLevel",
    ]
    for path in levels:
        status = check_asset_exists(path)
        print(f"  [{status}] {path}")

    # Check GameInstance DataTable configuration
    print("")
    print("=== GAME INSTANCE CONFIGURATION ===")
    gi_bp = unreal.EditorAssetLibrary.load_blueprint_class("/Game/Blueprints/Core/BP_T66GameInstance")
    if gi_bp:
        cdo = unreal.get_default_object(gi_bp)
        if cdo:
            try:
                hero_dt = cdo.get_editor_property("HeroDataTable")
                companion_dt = cdo.get_editor_property("CompanionDataTable")
                print(f"  HeroDataTable: {hero_dt.get_path_name() if hero_dt else 'NOT SET'}")
                print(f"  CompanionDataTable: {companion_dt.get_path_name() if companion_dt else 'NOT SET'}")
            except Exception as e:
                print(f"  Could not read properties: {e}")
        else:
            print("  Could not get CDO")
    else:
        print("  BP_T66GameInstance not found")

    # Check PlayerController configuration
    print("")
    print("=== PLAYER CONTROLLER CONFIGURATION ===")
    pc_bp = unreal.EditorAssetLibrary.load_blueprint_class("/Game/Blueprints/Core/BP_T66PlayerController")
    if pc_bp:
        cdo = unreal.get_default_object(pc_bp)
        if cdo:
            try:
                auto_show = cdo.get_editor_property("bAutoShowInitialScreen")
                auto_load = cdo.get_editor_property("bAutoLoadScreenClasses")
                print(f"  bAutoShowInitialScreen: {auto_show}")
                print(f"  bAutoLoadScreenClasses: {auto_load}")
            except Exception as e:
                print(f"  Could not read properties: {e}")
        else:
            print("  Could not get CDO")
    else:
        print("  BP_T66PlayerController not found")

    print("")
    print("=" * 70)
    print("VERIFICATION COMPLETE")
    print("=" * 70)

if __name__ == "__main__":
    main()
