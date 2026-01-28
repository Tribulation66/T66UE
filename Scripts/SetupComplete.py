"""
Complete T66 setup script:
- Import CSV data into DataTables
- Configure BP_T66GameInstance with DataTable references
- Configure level world settings
- Add PlayerStart to gameplay level
"""

import unreal
import csv
import os

def import_heroes_from_csv():
    """Import hero data from CSV into DT_Heroes"""
    dt = unreal.EditorAssetLibrary.load_asset("/Game/Data/DT_Heroes")
    if not dt:
        unreal.log_error("Could not load DT_Heroes")
        return False

    csv_path = "C:/UE/T66/Content/Data/Heroes.csv"
    if not os.path.exists(csv_path):
        unreal.log_error(f"CSV file not found: {csv_path}")
        return False

    # Use DataTableFunctionLibrary to fill from CSV
    success = unreal.DataTableFunctionLibrary.fill_data_table_from_csv_file(dt, csv_path)
    if success:
        unreal.log("Successfully imported heroes from CSV")
        unreal.EditorAssetLibrary.save_asset("/Game/Data/DT_Heroes")
    else:
        unreal.log_error("Failed to import heroes from CSV")
    return success

def import_companions_from_csv():
    """Import companion data from CSV into DT_Companions"""
    dt = unreal.EditorAssetLibrary.load_asset("/Game/Data/DT_Companions")
    if not dt:
        unreal.log_error("Could not load DT_Companions")
        return False

    csv_path = "C:/UE/T66/Content/Data/Companions.csv"
    if not os.path.exists(csv_path):
        unreal.log_error(f"CSV file not found: {csv_path}")
        return False

    success = unreal.DataTableFunctionLibrary.fill_data_table_from_csv_file(dt, csv_path)
    if success:
        unreal.log("Successfully imported companions from CSV")
        unreal.EditorAssetLibrary.save_asset("/Game/Data/DT_Companions")
    else:
        unreal.log_error("Failed to import companions from CSV")
    return success

def setup_gameplay_level():
    """Add PlayerStart to GameplayLevel and configure world settings"""
    level_path = "/Game/Maps/GameplayLevel"

    # Load the level
    success = unreal.EditorLevelLibrary.load_level(level_path)
    if not success:
        unreal.log_error(f"Failed to load level: {level_path}")
        return

    world = unreal.EditorLevelLibrary.get_editor_world()
    if not world:
        unreal.log_error("Could not get editor world")
        return

    # Check if PlayerStart already exists
    player_starts = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.PlayerStart)
    if len(player_starts) > 0:
        unreal.log("PlayerStart already exists in GameplayLevel")
    else:
        # Spawn PlayerStart
        player_start_class = unreal.PlayerStart.static_class()
        spawn_location = unreal.Vector(0.0, 0.0, 100.0)
        spawn_rotation = unreal.Rotator(0.0, 0.0, 0.0)

        player_start = unreal.EditorLevelLibrary.spawn_actor_from_class(
            player_start_class,
            spawn_location,
            spawn_rotation
        )
        if player_start:
            unreal.log("Spawned PlayerStart in GameplayLevel")
        else:
            unreal.log_error("Failed to spawn PlayerStart")

    # Add a floor plane for the player to stand on
    plane_mesh = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Plane")
    if plane_mesh:
        floor = unreal.EditorLevelLibrary.spawn_actor_from_object(
            plane_mesh,
            unreal.Vector(0.0, 0.0, 0.0),
            unreal.Rotator(0.0, 0.0, 0.0)
        )
        if floor:
            floor.set_actor_scale3d(unreal.Vector(10.0, 10.0, 1.0))
            unreal.log("Added floor plane to GameplayLevel")

    # Add a directional light
    light_class = unreal.DirectionalLight.static_class()
    light = unreal.EditorLevelLibrary.spawn_actor_from_class(
        light_class,
        unreal.Vector(0.0, 0.0, 500.0),
        unreal.Rotator(-45.0, 0.0, 0.0)
    )
    if light:
        unreal.log("Added directional light to GameplayLevel")

    # Save the level
    unreal.EditorLevelLibrary.save_current_level()
    unreal.log("GameplayLevel configured and saved")

def setup_frontend_level():
    """Configure FrontendLevel (minimal setup - just needs lighting)"""
    level_path = "/Game/Maps/FrontendLevel"

    success = unreal.EditorLevelLibrary.load_level(level_path)
    if not success:
        unreal.log_error(f"Failed to load level: {level_path}")
        return

    # Save level
    unreal.EditorLevelLibrary.save_current_level()
    unreal.log("FrontendLevel configured")

def verify_setup():
    """Verify all assets are properly configured"""
    unreal.log("")
    unreal.log("=== VERIFICATION ===")

    # Check DataTables have rows
    dt_heroes = unreal.EditorAssetLibrary.load_asset("/Game/Data/DT_Heroes")
    if dt_heroes:
        row_names = unreal.DataTableFunctionLibrary.get_data_table_row_names(dt_heroes)
        unreal.log(f"DT_Heroes has {len(row_names)} rows: {[str(n) for n in row_names]}")
    else:
        unreal.log_error("DT_Heroes not found")

    dt_companions = unreal.EditorAssetLibrary.load_asset("/Game/Data/DT_Companions")
    if dt_companions:
        row_names = unreal.DataTableFunctionLibrary.get_data_table_row_names(dt_companions)
        unreal.log(f"DT_Companions has {len(row_names)} rows: {[str(n) for n in row_names]}")
    else:
        unreal.log_error("DT_Companions not found")

    # List all created blueprints
    blueprints = [
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
    ]

    unreal.log("")
    unreal.log("Blueprint verification:")
    for bp_path in blueprints:
        exists = unreal.EditorAssetLibrary.does_asset_exist(bp_path)
        status = "OK" if exists else "MISSING"
        unreal.log(f"  [{status}] {bp_path}")

    # Check levels
    levels = [
        "/Game/Maps/FrontendLevel",
        "/Game/Maps/GameplayLevel",
    ]
    unreal.log("")
    unreal.log("Level verification:")
    for level_path in levels:
        exists = unreal.EditorAssetLibrary.does_asset_exist(level_path)
        status = "OK" if exists else "MISSING"
        unreal.log(f"  [{status}] {level_path}")

def print_remaining_manual_steps():
    """Print what still needs to be done manually"""
    unreal.log("")
    unreal.log("=" * 60)
    unreal.log("REMAINING MANUAL CONFIGURATION (in Unreal Editor)")
    unreal.log("=" * 60)
    unreal.log("")
    unreal.log("1. BP_T66GameInstance (Open and set in Class Defaults):")
    unreal.log("   - Hero Data Table -> DT_Heroes")
    unreal.log("   - Companion Data Table -> DT_Companions")
    unreal.log("")
    unreal.log("2. BP_T66PlayerController (Open and set in Class Defaults):")
    unreal.log("   - Add to Screen Classes map:")
    unreal.log("     MainMenu -> WBP_MainMenu")
    unreal.log("     PartySizePicker -> WBP_PartySizePicker")
    unreal.log("     HeroSelection -> WBP_HeroSelection")
    unreal.log("     CompanionSelection -> WBP_CompanionSelection")
    unreal.log("     SaveSlots -> WBP_SaveSlots")
    unreal.log("     Settings -> WBP_Settings")
    unreal.log("     QuitConfirmation -> WBP_QuitConfirmation")
    unreal.log("")
    unreal.log("3. FrontendLevel World Settings:")
    unreal.log("   - Game Mode Override -> BP_FrontendGameMode")
    unreal.log("   - Default Pawn Class -> None")
    unreal.log("")
    unreal.log("4. GameplayLevel World Settings:")
    unreal.log("   - Game Mode Override -> BP_GameplayGameMode")
    unreal.log("")
    unreal.log("5. BP_FrontendGameMode (Class Defaults):")
    unreal.log("   - Player Controller Class -> BP_T66PlayerController")
    unreal.log("   - Default Pawn Class -> None")
    unreal.log("")
    unreal.log("6. BP_GameplayGameMode (Class Defaults):")
    unreal.log("   - Player Controller Class -> BP_T66PlayerController")
    unreal.log("   - Default Pawn Class -> BP_HeroBase")
    unreal.log("")
    unreal.log("7. Widget Blueprints - Add UI elements:")
    unreal.log("   Open each WBP_* and add buttons/text using the Designer")
    unreal.log("   Wire button OnClicked to the C++ functions")
    unreal.log("")

def main():
    unreal.log("=" * 60)
    unreal.log("T66 COMPLETE SETUP SCRIPT")
    unreal.log("=" * 60)

    # Import DataTable data
    unreal.log("")
    unreal.log("--- Importing DataTable Data ---")
    import_heroes_from_csv()
    import_companions_from_csv()

    # Setup levels
    unreal.log("")
    unreal.log("--- Setting Up Levels ---")
    setup_gameplay_level()
    setup_frontend_level()

    # Verify
    verify_setup()

    # Print remaining steps
    print_remaining_manual_steps()

    unreal.log("")
    unreal.log("=" * 60)
    unreal.log("SETUP SCRIPT COMPLETE")
    unreal.log("=" * 60)

if __name__ == "__main__":
    main()
