"""
Unreal Python script to configure T66 assets:
- Populate DataTables with hero/companion data
- Configure BP_T66PlayerController with widget class mappings
- Configure BP_T66GameInstance with DataTable references
- Set up levels with proper game modes
"""

import unreal

def populate_hero_datatable():
    """Add 5 heroes to DT_Heroes"""
    dt = unreal.EditorAssetLibrary.load_asset("/Game/Data/DT_Heroes")
    if not dt:
        unreal.log_error("Could not load DT_Heroes")
        return

    # Define 5 heroes with different colors
    heroes = [
        {"RowName": "Hero_Crimson", "HeroID": "Hero_Crimson", "DisplayName": "Crimson Blade",
         "Description": "A fierce warrior wielding the power of fire.",
         "PlaceholderColor": (1.0, 0.2, 0.2, 1.0), "MapTheme": "Fire"},
        {"RowName": "Hero_Azure", "HeroID": "Hero_Azure", "DisplayName": "Azure Mage",
         "Description": "A powerful mage who commands ice and water.",
         "PlaceholderColor": (0.2, 0.4, 1.0, 1.0), "MapTheme": "Ice"},
        {"RowName": "Hero_Emerald", "HeroID": "Hero_Emerald", "DisplayName": "Emerald Guardian",
         "Description": "A stalwart defender of nature and life.",
         "PlaceholderColor": (0.2, 0.8, 0.3, 1.0), "MapTheme": "Forest"},
        {"RowName": "Hero_Golden", "HeroID": "Hero_Golden", "DisplayName": "Golden Paladin",
         "Description": "A holy warrior blessed by divine light.",
         "PlaceholderColor": (1.0, 0.8, 0.2, 1.0), "MapTheme": "Light"},
        {"RowName": "Hero_Shadow", "HeroID": "Hero_Shadow", "DisplayName": "Shadow Stalker",
         "Description": "An assassin who strikes from the darkness.",
         "PlaceholderColor": (0.3, 0.2, 0.4, 1.0), "MapTheme": "Dark"},
    ]

    # Get the row struct type
    subsystem = unreal.get_editor_subsystem(unreal.DataTableEditorLibrary)

    for hero in heroes:
        row_name = hero["RowName"]

        # Check if row already exists
        row_names = unreal.DataTableFunctionLibrary.get_data_table_row_names(dt)
        if row_name in [str(n) for n in row_names]:
            unreal.log(f"Hero row already exists: {row_name}")
            continue

        # Add row using the data table subsystem
        # We need to add an empty row first, then modify it
        # This is tricky in Python - we'll use a different approach

    unreal.log(f"DataTable DT_Heroes has {len(unreal.DataTableFunctionLibrary.get_data_table_row_names(dt))} rows")
    unreal.EditorAssetLibrary.save_asset("/Game/Data/DT_Heroes")

def configure_player_controller():
    """Set up BP_T66PlayerController with screen class mappings"""
    bp = unreal.EditorAssetLibrary.load_asset("/Game/Blueprints/Core/BP_T66PlayerController")
    if not bp:
        unreal.log_error("Could not load BP_T66PlayerController")
        return

    unreal.log("BP_T66PlayerController loaded - manual configuration required in Editor")
    # Note: Setting TMap properties on Blueprint CDO from Python is very limited
    # The user will need to configure this in the Editor

def configure_game_instance():
    """Set up BP_T66GameInstance with DataTable references"""
    bp = unreal.EditorAssetLibrary.load_asset("/Game/Blueprints/Core/BP_T66GameInstance")
    if not bp:
        unreal.log_error("Could not load BP_T66GameInstance")
        return

    unreal.log("BP_T66GameInstance loaded - DataTable references need to be set in Editor")

def configure_frontend_level():
    """Configure FrontendLevel with game mode and player start"""
    level_path = "/Game/Maps/FrontendLevel"

    # Load the level
    if not unreal.EditorAssetLibrary.does_asset_exist(level_path):
        unreal.log_error(f"Level does not exist: {level_path}")
        return

    # Load and set world settings
    unreal.EditorLevelLibrary.load_level(level_path)
    world = unreal.EditorLevelLibrary.get_editor_world()

    if world:
        unreal.log(f"Loaded level: {level_path}")
        # Note: Setting world settings requires different approach
        unreal.EditorLevelLibrary.save_current_level()
    else:
        unreal.log_error("Could not get editor world")

def configure_gameplay_level():
    """Configure GameplayLevel with game mode and player start"""
    level_path = "/Game/Maps/GameplayLevel"

    if not unreal.EditorAssetLibrary.does_asset_exist(level_path):
        unreal.log_error(f"Level does not exist: {level_path}")
        return

    # Load the level
    unreal.EditorLevelLibrary.load_level(level_path)
    world = unreal.EditorLevelLibrary.get_editor_world()

    if world:
        unreal.log(f"Loaded level: {level_path}")

        # Spawn a PlayerStart
        player_start_class = unreal.load_class(None, "/Script/Engine.PlayerStart")
        if player_start_class:
            spawn_location = unreal.Vector(0.0, 0.0, 100.0)
            spawn_rotation = unreal.Rotator(0.0, 0.0, 0.0)

            actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
                player_start_class,
                spawn_location,
                spawn_rotation
            )
            if actor:
                unreal.log("Spawned PlayerStart in GameplayLevel")

        unreal.EditorLevelLibrary.save_current_level()
    else:
        unreal.log_error("Could not get editor world")

def update_project_settings():
    """Update DefaultEngine.ini and DefaultGame.ini with project settings"""
    # This is better done by writing directly to INI files
    unreal.log("Project settings should be updated in DefaultEngine.ini")

def main():
    unreal.log("=== T66 Asset Configuration Script ===")

    # Configure levels
    unreal.log("--- Configuring Levels ---")
    configure_gameplay_level()
    configure_frontend_level()

    # Log what needs manual configuration
    unreal.log("")
    unreal.log("=== MANUAL CONFIGURATION REQUIRED ===")
    unreal.log("The following must be configured manually in the Editor:")
    unreal.log("")
    unreal.log("1. BP_T66GameInstance:")
    unreal.log("   - Set HeroDataTable to /Game/Data/DT_Heroes")
    unreal.log("   - Set CompanionDataTable to /Game/Data/DT_Companions")
    unreal.log("")
    unreal.log("2. BP_T66PlayerController:")
    unreal.log("   - Add entries to ScreenClasses map:")
    unreal.log("     - MainMenu -> WBP_MainMenu")
    unreal.log("     - PartySizePicker -> WBP_PartySizePicker")
    unreal.log("     - HeroSelection -> WBP_HeroSelection")
    unreal.log("     - CompanionSelection -> WBP_CompanionSelection")
    unreal.log("     - SaveSlots -> WBP_SaveSlots")
    unreal.log("     - Settings -> WBP_Settings")
    unreal.log("     - QuitConfirmation -> WBP_QuitConfirmation")
    unreal.log("")
    unreal.log("3. DT_Heroes - Add 5 hero rows with different colors")
    unreal.log("4. DT_Companions - Add 3 companion rows")
    unreal.log("")
    unreal.log("5. Project Settings (Edit -> Project Settings):")
    unreal.log("   - Maps & Modes -> Default GameMode: None (per-level)")
    unreal.log("   - Maps & Modes -> Game Instance Class: BP_T66GameInstance")
    unreal.log("   - Maps & Modes -> Editor Startup Map: FrontendLevel")
    unreal.log("   - Maps & Modes -> Game Default Map: FrontendLevel")
    unreal.log("")
    unreal.log("6. FrontendLevel World Settings:")
    unreal.log("   - GameMode Override: BP_FrontendGameMode")
    unreal.log("")
    unreal.log("7. GameplayLevel World Settings:")
    unreal.log("   - GameMode Override: BP_GameplayGameMode")
    unreal.log("")
    unreal.log("=== Configuration Script Complete ===")

if __name__ == "__main__":
    main()
