import unreal


ASSET_ROOT = "/Game/VFX/Idols/Preview"
MAP_ROOT = f"{ASSET_ROOT}/Maps"
MAP_PATH = f"{MAP_ROOT}/LV_IdolVFXPreview"
MAP_OBJECT_PATH = f"{MAP_PATH}.LV_IdolVFXPreview"
STAGE_LABEL = "IdolPreview_Stage"


def log(message: str) -> None:
    unreal.log(f"[IDOL VFX PREVIEW] {message}")


def ensure_directory(path: str) -> None:
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def load_or_create_preview_map() -> None:
    editor_loading = unreal.EditorLoadingAndSavingUtils
    if unreal.EditorAssetLibrary.does_asset_exist(MAP_OBJECT_PATH):
        editor_loading.load_map(MAP_PATH)
        log(f"Loaded existing preview map {MAP_PATH}")
        return

    unreal.EditorLevelLibrary.new_level(MAP_PATH)
    log(f"Created preview map {MAP_PATH}")


def build_stage() -> unreal.Actor:
    actor_class = unreal.load_class(None, "/Script/T66.T66IdolVFXPreviewStage")
    if actor_class is None:
        raise RuntimeError("Failed to load T66IdolVFXPreviewStage")

    existing = None
    for actor in unreal.EditorLevelLibrary.get_all_level_actors():
        if actor.get_actor_label() == STAGE_LABEL:
            existing = actor
            break

    if existing is None:
        existing = unreal.EditorLevelLibrary.spawn_actor_from_class(actor_class, unreal.Vector(0.0, 0.0, 0.0), unreal.Rotator(0.0, 0.0, 0.0))
        existing.set_actor_label(STAGE_LABEL)
        log("Spawned preview stage actor")
    else:
        log("Reusing preview stage actor")

    existing.set_actor_location(unreal.Vector(0.0, 0.0, 0.0), False, False)
    existing.build_preview()
    return existing


def frame_camera() -> None:
    editor_subsystem = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
    focus = unreal.Vector(1050.0, 0.0, 280.0)
    camera_location = focus + unreal.Vector(-420.0, -2600.0, 980.0)
    camera_rotation = unreal.MathLibrary.find_look_at_rotation(camera_location, focus)
    editor_subsystem.set_level_viewport_camera_info(camera_location, camera_rotation)
    world = editor_subsystem.get_editor_world()
    unreal.SystemLibrary.execute_console_command(world, "viewmode lit")
    unreal.SystemLibrary.execute_console_command(world, "showflag.grid 0")
    unreal.SystemLibrary.execute_console_command(world, "showflag.billboards 0")
    unreal.SystemLibrary.execute_console_command(world, "showflag.sprites 0")
    unreal.SystemLibrary.execute_console_command(world, "showflag.modewidgets 0")
    unreal.SystemLibrary.execute_console_command(world, "showflag.selectionoutline 0")


def main() -> None:
    ensure_directory("/Game/VFX")
    ensure_directory("/Game/VFX/Idols")
    ensure_directory(ASSET_ROOT)
    ensure_directory(MAP_ROOT)
    load_or_create_preview_map()
    build_stage()
    frame_camera()
    unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
    log("Idol VFX preview stage is ready")


main()
