"""
Complete T66 Setup Script
- Creates WBP_T66Button widget blueprint
- Configures BP_T66GameInstance with DataTable references
- Configures BP_T66PlayerController with ScreenClasses
- Configures GameModes with PlayerController class
- Imports CSV data into DataTables
"""

import unreal
import os

# Get asset tools
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

def get_content_path(relative_path):
    """Resolve path under Content/ (works when run from project root or editor)"""
    try:
        proj = unreal.SystemLibrary.get_project_directory()
        return os.path.normpath(os.path.join(proj, "Content", relative_path))
    except Exception:
        return os.path.normpath(os.path.join("Content", relative_path))

def create_button_widget():
    """Create WBP_T66Button widget blueprint"""
    asset_path = "/Game/Blueprints/UI/Components/WBP_T66Button"

    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        unreal.log(f"Button widget already exists: {asset_path}")
        return unreal.EditorAssetLibrary.load_asset(asset_path)

    # Ensure directory exists
    unreal.EditorAssetLibrary.make_directory("/Game/Blueprints/UI/Components")

    # Get the button class
    button_class = unreal.load_class(None, "/Script/T66.T66Button")
    if not button_class:
        unreal.log_error("Could not find T66Button class")
        return None

    # Create widget blueprint
    factory = unreal.WidgetBlueprintFactory()
    factory.set_editor_property("parent_class", button_class)

    widget_bp = asset_tools.create_asset("WBP_T66Button", "/Game/Blueprints/UI/Components", None, factory)
    if widget_bp:
        unreal.log(f"Created button widget: {asset_path}")
        unreal.EditorAssetLibrary.save_asset(asset_path)
    return widget_bp

def configure_game_instance():
    """Configure BP_T66GameInstance with DataTable references"""
    bp_path = "/Game/Blueprints/Core/BP_T66GameInstance"

    bp_gc = unreal.EditorAssetLibrary.load_blueprint_class(bp_path)
    if not bp_gc:
        unreal.log_error(f"Could not load blueprint class: {bp_path}")
        return False

    cdo = unreal.get_default_object(bp_gc)
    if cdo:
        # Set DataTable references using soft object paths
        # Try setting via soft object ptr (this may not work for all UE versions)
        try:
            hero_dt = unreal.EditorAssetLibrary.load_asset("/Game/Data/DT_Heroes")
            companion_dt = unreal.EditorAssetLibrary.load_asset("/Game/Data/DT_Companions")
            items_dt = unreal.EditorAssetLibrary.load_asset("/Game/Data/DT_Items")

            if hero_dt:
                cdo.set_editor_property("HeroDataTable", hero_dt)
                unreal.log("Set HeroDataTable on BP_T66GameInstance")
            if companion_dt:
                cdo.set_editor_property("CompanionDataTable", companion_dt)
                unreal.log("Set CompanionDataTable on BP_T66GameInstance")
            if items_dt:
                cdo.set_editor_property("ItemsDataTable", items_dt)
                unreal.log("Set ItemsDataTable on BP_T66GameInstance")

            unreal.EditorAssetLibrary.save_asset(bp_path)
            return True
        except Exception as e:
            unreal.log_warning(f"Could not set DataTable refs: {e}")
            return False
    return False

def configure_player_controller():
    """Configure BP_T66PlayerController"""
    bp_path = "/Game/Blueprints/Core/BP_T66PlayerController"

    bp_gc = unreal.EditorAssetLibrary.load_blueprint_class(bp_path)
    if not bp_gc:
        unreal.log_error(f"Could not load blueprint class: {bp_path}")
        return False

    cdo = unreal.get_default_object(bp_gc)
    if cdo:
        try:
            # Set auto show initial screen
            cdo.set_editor_property("bAutoShowInitialScreen", True)
            unreal.log("Set bAutoShowInitialScreen on BP_T66PlayerController")

            unreal.EditorAssetLibrary.save_asset(bp_path)
            return True
        except Exception as e:
            unreal.log_warning(f"Could not configure player controller: {e}")
            return False
    return False

def configure_frontend_gamemode():
    """Configure BP_FrontendGameMode"""
    bp_path = "/Game/Blueprints/GameModes/BP_FrontendGameMode"

    bp_gc = unreal.EditorAssetLibrary.load_blueprint_class(bp_path)
    if not bp_gc:
        unreal.log_error(f"Could not load blueprint class: {bp_path}")
        return False

    cdo = unreal.get_default_object(bp_gc)
    if cdo:
        try:
            # Set player controller class
            pc_class = unreal.EditorAssetLibrary.load_blueprint_class("/Game/Blueprints/Core/BP_T66PlayerController")
            if pc_class:
                cdo.set_editor_property("PlayerControllerClass", pc_class)
                unreal.log("Set PlayerControllerClass on BP_FrontendGameMode")

            # Set default pawn to None
            cdo.set_editor_property("DefaultPawnClass", None)

            unreal.EditorAssetLibrary.save_asset(bp_path)
            return True
        except Exception as e:
            unreal.log_warning(f"Could not configure frontend gamemode: {e}")
            return False
    return False

def configure_gameplay_gamemode():
    """Configure BP_GameplayGameMode"""
    bp_path = "/Game/Blueprints/GameModes/BP_GameplayGameMode"

    bp_gc = unreal.EditorAssetLibrary.load_blueprint_class(bp_path)
    if not bp_gc:
        unreal.log_error(f"Could not load blueprint class: {bp_path}")
        return False

    cdo = unreal.get_default_object(bp_gc)
    if cdo:
        try:
            # Set player controller class
            pc_class = unreal.EditorAssetLibrary.load_blueprint_class("/Game/Blueprints/Core/BP_T66PlayerController")
            if pc_class:
                cdo.set_editor_property("PlayerControllerClass", pc_class)
                unreal.log("Set PlayerControllerClass on BP_GameplayGameMode")

            # Set default pawn class to hero base
            hero_class = unreal.EditorAssetLibrary.load_blueprint_class("/Game/Blueprints/Core/BP_HeroBase")
            if hero_class:
                cdo.set_editor_property("DefaultPawnClass", hero_class)
                unreal.log("Set DefaultPawnClass on BP_GameplayGameMode")

            unreal.EditorAssetLibrary.save_asset(bp_path)
            return True
        except Exception as e:
            unreal.log_warning(f"Could not configure gameplay gamemode: {e}")
            return False
    return False

def import_datatable_csv():
    """Import CSV data into DataTables"""
    # Import Heroes
    dt_heroes = unreal.EditorAssetLibrary.load_asset("/Game/Data/DT_Heroes")
    if dt_heroes:
        csv_path = "C:/UE/T66/Content/Data/Heroes.csv"
        success = unreal.DataTableFunctionLibrary.fill_data_table_from_csv_file(dt_heroes, csv_path)
        if success:
            unreal.EditorAssetLibrary.save_asset("/Game/Data/DT_Heroes")
            row_names = unreal.DataTableFunctionLibrary.get_data_table_row_names(dt_heroes)
            unreal.log(f"Imported Heroes CSV - {len(row_names)} rows")
        else:
            unreal.log_warning("Failed to import Heroes CSV (may already have data)")

    # Import Companions
    dt_companions = unreal.EditorAssetLibrary.load_asset("/Game/Data/DT_Companions")
    if dt_companions:
        csv_path = "C:/UE/T66/Content/Data/Companions.csv"
        success = unreal.DataTableFunctionLibrary.fill_data_table_from_csv_file(dt_companions, csv_path)
        if success:
            unreal.EditorAssetLibrary.save_asset("/Game/Data/DT_Companions")
            row_names = unreal.DataTableFunctionLibrary.get_data_table_row_names(dt_companions)
            unreal.log(f"Imported Companions CSV - {len(row_names)} rows")
        else:
            unreal.log_warning("Failed to import Companions CSV (may already have data)")

    # Import Items
    dt_items = unreal.EditorAssetLibrary.load_asset("/Game/Data/DT_Items")
    if dt_items:
        csv_path = get_content_path("Data/Items.csv")
        if os.path.isfile(csv_path):
            success = unreal.DataTableFunctionLibrary.fill_data_table_from_csv_file(dt_items, csv_path)
            if success:
                unreal.EditorAssetLibrary.save_asset("/Game/Data/DT_Items")
                row_names = unreal.DataTableFunctionLibrary.get_data_table_row_names(dt_items)
                unreal.log(f"Imported Items CSV - {len(row_names)} rows")
            else:
                unreal.log_warning("Failed to import Items CSV (may already have data)")
        else:
            unreal.log_warning("Items CSV not found: " + csv_path)
    else:
        unreal.log("DT_Items not found; create via CreateAssets.py or run SetupItemsDataTable.py")

def verify_all_assets():
    """Verify all required assets exist"""
    unreal.log("")
    unreal.log("=== ASSET VERIFICATION ===")

    assets = [
        "/Game/Blueprints/Core/BP_T66GameInstance",
        "/Game/Blueprints/Core/BP_T66PlayerController",
        "/Game/Blueprints/Core/BP_HeroBase",
        "/Game/Blueprints/GameModes/BP_FrontendGameMode",
        "/Game/Blueprints/GameModes/BP_GameplayGameMode",
        "/Game/Blueprints/UI/WBP_MainMenu",
        "/Game/Blueprints/UI/WBP_PartySizePicker",
        "/Game/Blueprints/UI/WBP_HeroSelection",
        "/Game/Blueprints/UI/WBP_CompanionSelection",
        "/Game/Blueprints/UI/WBP_SaveSlots",
        "/Game/Blueprints/UI/WBP_Settings",
        "/Game/Blueprints/UI/WBP_QuitConfirmation",
        "/Game/Blueprints/UI/Components/WBP_T66Button",
        "/Game/Data/DT_Heroes",
        "/Game/Data/DT_Companions",
        "/Game/Data/DT_Items",
        "/Game/Maps/FrontendLevel",
        "/Game/Maps/GameplayLevel",
    ]

    all_ok = True
    for asset_path in assets:
        exists = unreal.EditorAssetLibrary.does_asset_exist(asset_path)
        status = "OK" if exists else "MISSING"
        if not exists:
            all_ok = False
        unreal.log(f"  [{status}] {asset_path}")

    # Check DataTable row counts
    dt_heroes = unreal.EditorAssetLibrary.load_asset("/Game/Data/DT_Heroes")
    if dt_heroes:
        row_names = unreal.DataTableFunctionLibrary.get_data_table_row_names(dt_heroes)
        unreal.log(f"  DT_Heroes has {len(row_names)} rows")

    dt_companions = unreal.EditorAssetLibrary.load_asset("/Game/Data/DT_Companions")
    if dt_companions:
        row_names = unreal.DataTableFunctionLibrary.get_data_table_row_names(dt_companions)
        unreal.log(f"  DT_Companions has {len(row_names)} rows")

    dt_items = unreal.EditorAssetLibrary.load_asset("/Game/Data/DT_Items")
    if dt_items:
        row_names = unreal.DataTableFunctionLibrary.get_data_table_row_names(dt_items)
        unreal.log(f"  DT_Items has {len(row_names)} rows")

    return all_ok

def main():
    unreal.log("=" * 60)
    unreal.log("T66 FULL SETUP SCRIPT")
    unreal.log("=" * 60)

    # 1. Create button widget
    unreal.log("")
    unreal.log("--- Creating Button Widget ---")
    create_button_widget()

    # 2. Configure Game Instance
    unreal.log("")
    unreal.log("--- Configuring BP_T66GameInstance ---")
    configure_game_instance()

    # 3. Configure Player Controller
    unreal.log("")
    unreal.log("--- Configuring BP_T66PlayerController ---")
    configure_player_controller()

    # 4. Configure GameModes
    unreal.log("")
    unreal.log("--- Configuring GameModes ---")
    configure_frontend_gamemode()
    configure_gameplay_gamemode()

    # 5. Import DataTable data
    unreal.log("")
    unreal.log("--- Importing DataTable Data ---")
    import_datatable_csv()

    # 6. Verify
    verify_all_assets()

    unreal.log("")
    unreal.log("=" * 60)
    unreal.log("SETUP COMPLETE")
    unreal.log("=" * 60)

if __name__ == "__main__":
    main()
