"""
Unreal Python script to create Blueprints, DataTables, and Levels for T66
Run via: UnrealEditor-Cmd.exe ProjectPath -run=pythonscript -script="Scripts/CreateAssets.py"
"""

import unreal

# Asset tools
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

def create_blueprint(asset_path, parent_class):
    """Create a Blueprint inheriting from a C++ class"""
    package_path = asset_path.rsplit('/', 1)[0]
    asset_name = asset_path.rsplit('/', 1)[1]

    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        unreal.log(f"Asset already exists: {asset_path}")
        return unreal.EditorAssetLibrary.load_asset(asset_path)

    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", parent_class)

    blueprint = asset_tools.create_asset(asset_name, package_path, None, factory)
    if blueprint:
        unreal.log(f"Created Blueprint: {asset_path}")
        unreal.EditorAssetLibrary.save_asset(asset_path)
    else:
        unreal.log_error(f"Failed to create Blueprint: {asset_path}")
    return blueprint

def create_widget_blueprint(asset_path, parent_class):
    """Create a Widget Blueprint inheriting from a C++ widget class"""
    package_path = asset_path.rsplit('/', 1)[0]
    asset_name = asset_path.rsplit('/', 1)[1]

    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        unreal.log(f"Widget already exists: {asset_path}")
        return unreal.EditorAssetLibrary.load_asset(asset_path)

    factory = unreal.WidgetBlueprintFactory()
    factory.set_editor_property("parent_class", parent_class)

    widget_bp = asset_tools.create_asset(asset_name, package_path, None, factory)
    if widget_bp:
        unreal.log(f"Created Widget Blueprint: {asset_path}")
        unreal.EditorAssetLibrary.save_asset(asset_path)
    else:
        unreal.log_error(f"Failed to create Widget Blueprint: {asset_path}")
    return widget_bp

def create_data_table(asset_path, struct_class):
    """Create a DataTable with the specified row struct"""
    package_path = asset_path.rsplit('/', 1)[0]
    asset_name = asset_path.rsplit('/', 1)[1]

    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        unreal.log(f"DataTable already exists: {asset_path}")
        return unreal.EditorAssetLibrary.load_asset(asset_path)

    factory = unreal.DataTableFactory()
    factory.set_editor_property("struct", struct_class)

    data_table = asset_tools.create_asset(asset_name, package_path, None, factory)
    if data_table:
        unreal.log(f"Created DataTable: {asset_path}")
        unreal.EditorAssetLibrary.save_asset(asset_path)
    else:
        unreal.log_error(f"Failed to create DataTable: {asset_path}")
    return data_table

def create_level(asset_path):
    """Create an empty level using EditorLevelLibrary"""
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        unreal.log(f"Level already exists: {asset_path}")
        return

    # Create new level using EditorLevelLibrary
    success = unreal.EditorLevelLibrary.new_level(asset_path)
    if success:
        unreal.log(f"Created Level: {asset_path}")
    else:
        unreal.log_error(f"Failed to create Level: {asset_path}")

def main():
    unreal.log("=== T66 Asset Creation Script ===")

    # Ensure directories exist
    directories = [
        "/Game/Blueprints",
        "/Game/Blueprints/Core",
        "/Game/Blueprints/GameModes",
        "/Game/Blueprints/UI",
        "/Game/Data",
        "/Game/Maps",
    ]

    for dir_path in directories:
        if not unreal.EditorAssetLibrary.does_directory_exist(dir_path):
            unreal.EditorAssetLibrary.make_directory(dir_path)
            unreal.log(f"Created directory: {dir_path}")

    # === Create Core Blueprints ===
    unreal.log("--- Creating Core Blueprints ---")

    gi_class = unreal.load_class(None, "/Script/T66.T66GameInstance")
    if gi_class:
        create_blueprint("/Game/Blueprints/Core/BP_T66GameInstance", gi_class)
    else:
        unreal.log_error("Could not find T66GameInstance class")

    pc_class = unreal.load_class(None, "/Script/T66.T66PlayerController")
    if pc_class:
        create_blueprint("/Game/Blueprints/Core/BP_T66PlayerController", pc_class)
    else:
        unreal.log_error("Could not find T66PlayerController class")

    hero_class = unreal.load_class(None, "/Script/T66.T66HeroBase")
    if hero_class:
        create_blueprint("/Game/Blueprints/Core/BP_HeroBase", hero_class)
    else:
        unreal.log_error("Could not find T66HeroBase class")

    # === Create GameMode Blueprints ===
    unreal.log("--- Creating GameMode Blueprints ---")

    frontend_gm_class = unreal.load_class(None, "/Script/T66.T66FrontendGameMode")
    if frontend_gm_class:
        create_blueprint("/Game/Blueprints/GameModes/BP_FrontendGameMode", frontend_gm_class)
    else:
        unreal.log_error("Could not find T66FrontendGameMode class")

    gameplay_gm_class = unreal.load_class(None, "/Script/T66.T66GameMode")
    if gameplay_gm_class:
        create_blueprint("/Game/Blueprints/GameModes/BP_GameplayGameMode", gameplay_gm_class)
    else:
        unreal.log_error("Could not find T66GameMode class")

    # === Create Widget Blueprints ===
    unreal.log("--- Creating Widget Blueprints ---")

    widget_classes = {
        "WBP_MainMenu": "/Script/T66.T66MainMenuScreen",
        "WBP_PartySizePicker": "/Script/T66.T66PartySizePickerScreen",
        "WBP_HeroSelection": "/Script/T66.T66HeroSelectionScreen",
        "WBP_CompanionSelection": "/Script/T66.T66CompanionSelectionScreen",
        "WBP_SaveSlots": "/Script/T66.T66SaveSlotsScreen",
        "WBP_Settings": "/Script/T66.T66SettingsScreen",
        "WBP_QuitConfirmation": "/Script/T66.T66QuitConfirmationModal",
    }

    for widget_name, class_path in widget_classes.items():
        widget_class = unreal.load_class(None, class_path)
        if widget_class:
            create_widget_blueprint(f"/Game/Blueprints/UI/{widget_name}", widget_class)
        else:
            unreal.log_error(f"Could not find class: {class_path}")

    # === Create DataTables ===
    unreal.log("--- Creating DataTables ---")

    # Get struct classes using find_object
    hero_struct = unreal.find_object(None, "/Script/T66.HeroData")
    if hero_struct:
        create_data_table("/Game/Data/DT_Heroes", hero_struct)
    else:
        unreal.log_error("Could not find HeroData struct")

    companion_struct = unreal.find_object(None, "/Script/T66.CompanionData")
    if companion_struct:
        create_data_table("/Game/Data/DT_Companions", companion_struct)
    else:
        unreal.log_error("Could not find CompanionData struct")

    item_struct = unreal.find_object(None, "/Script/T66.ItemData")
    if item_struct:
        create_data_table("/Game/Data/DT_Items", item_struct)
    else:
        unreal.log_error("Could not find ItemData struct")

    idol_struct = unreal.find_object(None, "/Script/T66.IdolData")
    if idol_struct:
        create_data_table("/Game/Data/DT_Idols", idol_struct)
    else:
        unreal.log_warning("Could not find IdolData struct (DT_Idols not created)")

    boss_struct = unreal.find_object(None, "/Script/T66.BossData")
    if boss_struct:
        create_data_table("/Game/Data/DT_Bosses", boss_struct)
    else:
        unreal.log_error("Could not find BossData struct")

    stage_struct = unreal.find_object(None, "/Script/T66.StageData")
    if stage_struct:
        create_data_table("/Game/Data/DT_Stages", stage_struct)
    else:
        unreal.log_error("Could not find StageData struct")

    house_npc_struct = unreal.find_object(None, "/Script/T66.HouseNPCData")
    if house_npc_struct:
        create_data_table("/Game/Data/DT_HouseNPCs", house_npc_struct)
    else:
        unreal.log_error("Could not find HouseNPCData struct")

    loan_shark_struct = unreal.find_object(None, "/Script/T66.LoanSharkData")
    if loan_shark_struct:
        create_data_table("/Game/Data/DT_LoanShark", loan_shark_struct)
    else:
        unreal.log_error("Could not find LoanSharkData struct")

    # === Create Levels ===
    unreal.log("--- Creating Levels ---")
    create_level("/Game/Maps/FrontendLevel")
    create_level("/Game/Maps/GameplayLevel")
    # Coliseum lives inside GameplayLevel (walled-off arena). No separate Coliseum map is used.

    unreal.log("=== Asset Creation Complete ===")

if __name__ == "__main__":
    main()
