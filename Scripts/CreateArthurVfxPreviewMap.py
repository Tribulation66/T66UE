import unreal

LOG_PREFIX = "[ArthurVfxPreview]"
SOURCE_MAP = "/Game/Stylized_VFX_StPack/MAP/ProjectilesAndAOE"
DEST_MAP = "/Game/Maps/ProjectilesAndAOE_ArthurPreview"
GAME_MODE_CLASS_PATH = "/Game/Blueprints/GameModes/BP_GameplayGameMode.BP_GameplayGameMode_C"
PLAYER_START_LABEL = "ArthurPreviewStart"
PLAYER_START_LOCATION = unreal.Vector(0.0, 0.0, 200.0)
PLAYER_START_ROTATION = unreal.Rotator(0.0, 90.0, 0.0)


def log(message: str) -> None:
    unreal.log(f"{LOG_PREFIX} {message}")


def warn(message: str) -> None:
    unreal.log_warning(f"{LOG_PREFIX} {message}")


def delete_existing_destination() -> None:
    if unreal.EditorAssetLibrary.does_asset_exist(DEST_MAP):
        log(f"Deleting existing destination map {DEST_MAP}")
        if not unreal.EditorAssetLibrary.delete_asset(DEST_MAP):
            raise RuntimeError(f"Failed to delete existing destination map: {DEST_MAP}")


def duplicate_source_map() -> None:
    dest_dir = DEST_MAP.rsplit("/", 1)[0]
    if not unreal.EditorAssetLibrary.does_directory_exist(dest_dir):
        unreal.EditorAssetLibrary.make_directory(dest_dir)

    log(f"Duplicating {SOURCE_MAP} -> {DEST_MAP}")
    if not unreal.EditorAssetLibrary.duplicate_asset(SOURCE_MAP, DEST_MAP):
        raise RuntimeError(f"Failed to duplicate source map {SOURCE_MAP} to {DEST_MAP}")


def load_destination_world():
    log(f"Loading map {DEST_MAP}")
    world = unreal.EditorLoadingAndSavingUtils.load_map(DEST_MAP)
    if not world:
        raise RuntimeError(f"Failed to load duplicated map {DEST_MAP}")
    return world


def set_game_mode(world) -> None:
    game_mode_class = unreal.load_class(None, GAME_MODE_CLASS_PATH)
    if not game_mode_class:
        raise RuntimeError(f"Failed to load game mode class {GAME_MODE_CLASS_PATH}")

    world_settings = world.get_world_settings()
    property_candidates = ("default_game_mode", "game_mode_override")
    last_error = None
    for prop_name in property_candidates:
        try:
            world_settings.set_editor_property(prop_name, game_mode_class)
            log(f"Assigned {GAME_MODE_CLASS_PATH} via WorldSettings.{prop_name}")
            return
        except Exception as exc:  # noqa: BLE001 - Unreal Python raises generic exceptions
            last_error = exc

    raise RuntimeError(f"Failed to assign game mode on WorldSettings: {last_error}")


def ensure_player_start() -> None:
    actors = unreal.EditorLevelLibrary.get_all_level_actors()
    for actor in actors:
        if actor.get_class().get_name() == "PlayerStart":
            log(f"Map already contains PlayerStart: {actor.get_name()}")
            return

    player_start = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.PlayerStart,
        PLAYER_START_LOCATION,
        PLAYER_START_ROTATION,
    )
    if not player_start:
        raise RuntimeError("Failed to spawn PlayerStart in preview map")

    player_start.set_actor_label(PLAYER_START_LABEL)
    log(f"Spawned {PLAYER_START_LABEL} at {PLAYER_START_LOCATION}")


def save_map(world) -> None:
    world.get_world_settings().modify()
    if not unreal.EditorLevelLibrary.save_current_level():
        warn("EditorLevelLibrary.save_current_level() returned False; attempting save_dirty_packages")
        unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
    log("Saved preview map")


def main() -> None:
    delete_existing_destination()
    duplicate_source_map()
    world = load_destination_world()
    set_game_mode(world)
    ensure_player_start()
    save_map(world)
    log(f"Preview map ready: {DEST_MAP}")


if __name__ == "__main__":
    main()
