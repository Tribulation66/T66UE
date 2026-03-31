import math
import unreal


PROJECT_ROOT = r"C:\UE\T66"
ASSET_ROOT = "/Game/VFX/Idols/Fire/Zibra"
VOLUME_ROOT = f"{ASSET_ROOT}/Volumes"
MAP_ROOT = f"{ASSET_ROOT}/Maps"
MAP_PATH = f"{MAP_ROOT}/LV_ZibraFirePreview"
MAP_OBJECT_PATH = f"{MAP_PATH}.LV_ZibraFirePreview"
IMPORTS = {
    "GroundExplosion": r"C:\UE\T66\SourceAssets\Import\VFX\ExternalVDB\ZibraFree\ZIbraVDB Sample Effects\GroundExplosion.zibravdb",
    "ShockwaveExplosion": r"C:\UE\T66\SourceAssets\Import\VFX\ExternalVDB\ZibraFree\ZIbraVDB Sample Effects\ShockwaveExplosion.zibravdb",
}
ACTOR_NAMES = {
    "GroundExplosion": "Zibra_GroundExplosion_Preview",
    "ShockwaveExplosion": "Zibra_ShockwaveExplosion_Preview",
}


def log(message: str) -> None:
    unreal.log(f"[ZIBRA FIRE PREVIEW] {message}")


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


def import_zibravdb(asset_name: str, source_path: str) -> str:
    destination_path = VOLUME_ROOT
    destination_object_path = f"{destination_path}/{asset_name}.{asset_name}"
    if unreal.EditorAssetLibrary.does_asset_exist(destination_object_path):
        log(f"Reusing existing asset {destination_object_path}")
        return destination_object_path

    task = unreal.AssetImportTask()
    task.set_editor_property("filename", source_path)
    task.set_editor_property("destination_path", destination_path)
    task.set_editor_property("destination_name", asset_name)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("replace_existing_settings", True)
    task.set_editor_property("automated", True)
    task.set_editor_property("save", True)

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

    if task.imported_object_paths:
        imported_path = task.imported_object_paths[0]
        log(f"Imported {source_path} -> {imported_path}")
        return imported_path

    if unreal.EditorAssetLibrary.does_asset_exist(destination_object_path):
        log(f"Imported asset resolved via fallback path {destination_object_path}")
        return destination_object_path

    raise RuntimeError(f"Failed to import {source_path}")


def clear_preview_actors() -> None:
    for actor in unreal.EditorLevelLibrary.get_all_level_actors():
        if actor.get_actor_label().startswith("Zibra_") or actor.get_actor_label().startswith("FirePreview_"):
            unreal.EditorLevelLibrary.destroy_actor(actor)


def scale_actor_to_target_extent(actor: unreal.Actor, target_extent: float = 260.0) -> None:
    origin, extent = actor.get_actor_bounds(False)
    max_extent = max(extent.x, extent.y, extent.z)
    if max_extent <= 1.0:
        return
    scale = target_extent / max_extent
    actor.set_actor_scale3d(unreal.Vector(scale, scale, scale))


def configure_material(actor: unreal.Actor, asset_name: str) -> None:
    material_component = actor.get_editor_property("material_component")
    material_component.set_editor_property("use_heterogeneous_volume", False)
    material_component.set_editor_property("scattering_color", unreal.LinearColor(0.04, 0.04, 0.045, 1.0))
    material_component.set_editor_property("directional_light_brightness", 0.0)
    material_component.set_editor_property("point_light_brightness", 0.0)
    material_component.set_editor_property("ambient_light_intensity", 0.0)
    material_component.set_editor_property("shadow_intensity", 0.08)
    material_component.set_editor_property("ray_marching_main_step_size", 0.6)
    material_component.set_editor_property("ray_marching_max_main_steps", 768)
    material_component.set_editor_property("ray_marching_self_shadow_step_size", 0.9)
    material_component.set_editor_property("ray_marching_max_self_shadow_steps", 160)
    material_component.set_editor_property("allow_skip_decompression", False)
    material_component.set_editor_property("zibra_vdb_bounds_scale", 1.6)
    material_component.set_editor_property("volume_translucency_sort_priority", 20)

    if asset_name == "GroundExplosion":
        material_component.set_editor_property("density_scale", 0.24)
        material_component.set_editor_property("flame_scale", 2.75)
        material_component.set_editor_property("temperature_scale", 1.25)
        material_component.set_editor_property("flame_color", unreal.LinearColor(2.4, 0.7, 0.08, 1.0))
        material_component.set_editor_property("temperature_color", unreal.LinearColor(1.3, 0.2, 0.02, 1.0))
    else:
        material_component.set_editor_property("density_scale", 0.08)
        material_component.set_editor_property("flame_scale", 1.1)
        material_component.set_editor_property("temperature_scale", 0.6)
        material_component.set_editor_property("flame_color", unreal.LinearColor(1.1, 0.55, 0.08, 1.0))
        material_component.set_editor_property("temperature_color", unreal.LinearColor(0.9, 0.25, 0.05, 1.0))


def set_preview_frame(actor: unreal.Actor, fraction: float) -> None:
    asset_component = actor.get_editor_property("asset_component")
    volume = asset_component.get_editor_property("zibra_vdb_volume")
    playback_component = actor.get_editor_property("playback_component")
    frame_count = max(int(volume.get_editor_property("frame_count")), 1)
    frame = min(max(int(math.floor(frame_count * fraction)), 0), frame_count - 1)
    playback_component.set_editor_property("animate", False)
    playback_component.set_editor_property("current_frame", float(frame))
    playback_component.set_editor_property("visible", True)


def create_actor(asset_name: str, asset_path: str, location: unreal.Vector, preview_fraction: float) -> unreal.Actor:
    volume = unreal.load_asset(asset_path)
    if volume is None:
        raise RuntimeError(f"Failed to load imported asset {asset_path}")

    actor_class = unreal.load_class(None, "/Script/ZibraVDBRuntime.ZibraVDBActor")
    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(actor_class, location, unreal.Rotator(0.0, 0.0, 0.0))
    actor.set_actor_label(ACTOR_NAMES[asset_name])

    asset_component = actor.get_editor_property("asset_component")
    asset_component.set_editor_property("zibra_vdb_volume", volume)
    configure_material(actor, asset_name)
    set_preview_frame(actor, preview_fraction)
    if asset_name == "GroundExplosion":
        actor.set_actor_scale3d(unreal.Vector(1.35, 1.35, 1.35))
    else:
        actor.set_actor_scale3d(unreal.Vector(0.22, 0.22, 0.22))
    actor.set_actor_location(location, False, False)
    return actor


def build_preview_scene() -> None:
    ensure_directory(ASSET_ROOT)
    ensure_directory(VOLUME_ROOT)
    ensure_directory(MAP_ROOT)

    imported_assets = {}
    for asset_name, source_path in IMPORTS.items():
        imported_assets[asset_name] = import_zibravdb(asset_name, source_path)

    load_or_create_preview_map()
    clear_preview_actors()
    world = unreal.EditorLevelLibrary.get_editor_world()
    unreal.SystemLibrary.execute_console_command(world, "viewmode lit")
    unreal.SystemLibrary.execute_console_command(world, "showflag.grid 0")
    unreal.SystemLibrary.execute_console_command(world, "showflag.sprites 0")
    unreal.SystemLibrary.execute_console_command(world, "showflag.billboards 0")
    create_actor("GroundExplosion", imported_assets["GroundExplosion"], unreal.Vector(0.0, 0.0, 0.0), 0.10)
    create_actor("ShockwaveExplosion", imported_assets["ShockwaveExplosion"], unreal.Vector(900.0, 0.0, 0.0), 0.06)
    unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
    log("Zibra fire preview level is ready")


build_preview_scene()
