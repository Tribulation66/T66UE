import unreal


def main():
    subsystem_class = getattr(unreal, "T66UISetupSubsystem", None)
    if subsystem_class is None:
        raise RuntimeError("T66UISetupSubsystem is not available in Python")

    subsystem = unreal.get_editor_subsystem(subsystem_class)
    if subsystem is None:
        raise RuntimeError("Failed to get T66UISetupSubsystem")

    if not subsystem.configure_player_controller():
        raise RuntimeError("configure_player_controller() failed")

    if not subsystem.configure_frontend_game_mode():
        raise RuntimeError("configure_frontend_game_mode() failed")

    unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
    unreal.log("Frontend hero-selection Blueprint references updated successfully.")


if __name__ == "__main__":
    main()
