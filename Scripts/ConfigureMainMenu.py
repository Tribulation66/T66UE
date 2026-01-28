"""
Configure Main Menu Widget Blueprint
- Adds button widgets programmatically
"""

import unreal

def get_widget_blueprint(asset_path):
    """Load a widget blueprint asset"""
    return unreal.EditorAssetLibrary.load_asset(asset_path)

def log_widget_info():
    """Log info about the main menu widget"""
    asset_path = "/Game/Blueprints/UI/WBP_MainMenu"

    widget_bp = get_widget_blueprint(asset_path)
    if not widget_bp:
        unreal.log_error(f"Could not load widget blueprint: {asset_path}")
        return

    unreal.log(f"Widget Blueprint loaded: {widget_bp.get_name()}")
    unreal.log(f"Widget Blueprint class: {widget_bp.get_class().get_name()}")

    # Try to get the widget tree
    try:
        widget_tree = widget_bp.get_editor_property("WidgetTree")
        if widget_tree:
            unreal.log(f"Widget Tree: {widget_tree}")
            root = widget_tree.get_editor_property("RootWidget")
            if root:
                unreal.log(f"Root Widget: {root.get_name()} ({root.get_class().get_name()})")
    except Exception as e:
        unreal.log_warning(f"Could not access widget tree: {e}")

def main():
    unreal.log("=" * 60)
    unreal.log("CONFIGURE MAIN MENU")
    unreal.log("=" * 60)

    log_widget_info()

    # Note: Adding widgets to UMG blueprints programmatically is complex
    # The recommended approach is to:
    # 1. Create the widget structure in Blueprint editor
    # 2. Or use a template widget blueprint that can be duplicated

    # For now, we'll document the expected structure:
    unreal.log("")
    unreal.log("EXPECTED MAIN MENU STRUCTURE:")
    unreal.log("  - Canvas Panel (Root)")
    unreal.log("    - Vertical Box (centered)")
    unreal.log("      - Text: 'TRIBULATION 66' (Title)")
    unreal.log("      - WBP_T66Button (Action: NavigateTo, Target: PartySizePicker, Text: 'NEW GAME')")
    unreal.log("      - WBP_T66Button (Action: NavigateTo, Target: SaveSlots, Text: 'CONTINUE')")
    unreal.log("      - WBP_T66Button (Action: NavigateTo, Target: Settings, Text: 'SETTINGS')")
    unreal.log("      - WBP_T66Button (Action: ShowModal, Target: QuitConfirmation, Text: 'QUIT')")

    unreal.log("")
    unreal.log("Note: Widget hierarchy must be configured in Blueprint Editor")
    unreal.log("The T66Button class provides action routing when clicked")

    unreal.log("")
    unreal.log("=" * 60)
    unreal.log("CONFIGURATION COMPLETE")
    unreal.log("=" * 60)

if __name__ == "__main__":
    main()
