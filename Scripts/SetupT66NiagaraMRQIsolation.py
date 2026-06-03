import json
import pathlib
import re
import traceback

import unreal

DEFAULT_OUTPUT_DIR = r"C:\UE\T66\Saved\VFXResearch\Hero1Axe\AOE_AmericanFlagVisualTarget\EditorIsolation\MRQIsolation"
DEFAULT_TEMP_NAME = "T66NiagaraMRQIsolation"


def command_line_value(name, default=""):
    command_line = unreal.SystemLibrary.get_command_line()
    match = re.search(rf"-{re.escape(name)}=(\"[^\"]*\"|\S+)", command_line)
    if not match:
        return default
    value = match.group(1)
    if value.startswith('"') and value.endswith('"'):
        value = value[1:-1]
    return value


def command_line_float(name, default):
    raw_value = command_line_value(name, "")
    if raw_value == "":
        return default
    try:
        return float(raw_value)
    except ValueError:
        return default


def command_line_int(name, default):
    raw_value = command_line_value(name, "")
    if raw_value == "":
        return default
    try:
        return int(raw_value)
    except ValueError:
        return default


def command_line_flag(name):
    return f"-{name}" in unreal.SystemLibrary.get_command_line()


def object_paths(temp_name):
    map_dir = "/Game/VFXLab/Temp/MRQ"
    map_path = f"{map_dir}/L_{temp_name}"
    seq_path = f"{map_dir}/LS_{temp_name}"
    config_path = f"{map_dir}/PC_{temp_name}"
    return map_dir, map_path, seq_path, config_path


def write_manifest(path, **kwargs):
    payload = {
        "tool": "SetupT66NiagaraMRQIsolation",
    }
    payload.update(kwargs)
    pathlib.Path(path).write_text(json.dumps(payload, indent=2), encoding="utf-8")


def delete_asset_if_present(asset_path):
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        unreal.EditorAssetLibrary.delete_asset(asset_path)


def cleanup_assets(map_path, seq_path, config_path):
    for asset_path in [config_path, seq_path, map_path]:
        delete_asset_if_present(asset_path)
    unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)


def setup_assets():
    output_dir = pathlib.Path(command_line_value("T66MRQIsolationOutput", DEFAULT_OUTPUT_DIR))
    frame_dir = output_dir / "frames"
    setup_manifest = output_dir / "setup_manifest.json"
    output_dir.mkdir(parents=True, exist_ok=True)
    frame_dir.mkdir(parents=True, exist_ok=True)

    temp_name = command_line_value("T66MRQIsolationTempName", DEFAULT_TEMP_NAME)
    res_x = command_line_int("T66MRQIsolationResX", 1400)
    res_y = command_line_int("T66MRQIsolationResY", 1400)
    camera_x = command_line_float("T66MRQIsolationCameraX", 0.0)
    camera_y = command_line_float("T66MRQIsolationCameraY", 0.0)
    camera_z = command_line_float("T66MRQIsolationCameraZ", 900.0)
    ortho_width = command_line_float("T66MRQIsolationOrthoWidth", 1250.0)
    map_dir, map_path, seq_path, config_path = object_paths(temp_name)

    for png in frame_dir.rglob("*.png"):
        png.unlink()

    unreal.EditorAssetLibrary.make_directory(map_dir)
    cleanup_assets(map_path, seq_path, config_path)

    world = unreal.EditorLoadingAndSavingUtils.new_blank_map(False)
    if not world:
        raise RuntimeError("new_blank_map returned None")

    lab_class = unreal.load_class(None, "/Script/T66.T66Hero1AxeAOEVFXLabActor")
    if not lab_class:
        raise RuntimeError("Failed to load /Script/T66.T66Hero1AxeAOEVFXLabActor")
    unreal.EditorLevelLibrary.spawn_actor_from_class(lab_class, unreal.Vector(0, 0, 0), unreal.Rotator(), False)

    camera_actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.CameraActor,
        unreal.Vector(camera_x, camera_y, camera_z),
        unreal.Rotator(pitch=-90, yaw=0, roll=0),
        False,
    )
    camera_actor.set_actor_label("T66_MRQ_NiagaraIsolation_Camera")
    camera_component = camera_actor.camera_component
    camera_component.set_editor_property("projection_mode", unreal.CameraProjectionMode.ORTHOGRAPHIC)
    camera_component.set_editor_property("ortho_width", ortho_width)
    camera_component.set_editor_property("constrain_aspect_ratio", False)

    if not unreal.EditorLoadingAndSavingUtils.save_map(world, map_path):
        raise RuntimeError(f"Failed to save map {map_path}")

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    sequence_name = f"LS_{temp_name}"
    config_name = f"PC_{temp_name}"
    sequence = asset_tools.create_asset(sequence_name, map_dir, unreal.LevelSequence, unreal.LevelSequenceFactoryNew())
    if not sequence:
        raise RuntimeError("Failed to create LevelSequence")
    sequence.set_display_rate(unreal.FrameRate(24, 1))
    sequence.set_playback_start(0)
    sequence.set_playback_end(12)
    binding = sequence.add_possessable(camera_actor)
    camera_cut_track = sequence.add_track(unreal.MovieSceneCameraCutTrack)
    camera_cut_section = camera_cut_track.add_section()
    camera_cut_section.set_start_frame(-1)
    camera_cut_section.set_end_frame(12)
    camera_binding_id = unreal.MovieSceneObjectBindingID()
    camera_binding_id.set_editor_property("Guid", binding.get_id())
    camera_cut_section.set_editor_property("CameraBindingID", camera_binding_id)
    unreal.EditorAssetLibrary.save_asset(seq_path, False)

    config = asset_tools.create_asset(
        config_name,
        map_dir,
        unreal.MoviePipelinePrimaryConfig,
        unreal.MoviePipelinePrimaryConfigFactory(),
    )
    if not config:
        raise RuntimeError("Failed to create MoviePipelinePrimaryConfig")
    output_setting = config.find_or_add_setting_by_class(unreal.MoviePipelineOutputSetting)
    output_setting.output_directory = unreal.DirectoryPath(str(frame_dir))
    output_setting.output_resolution = unreal.IntPoint(res_x, res_y)
    output_setting.file_name_format = "mrq_{frame_number}"
    output_setting.override_existing_output = True
    output_setting.flush_disk_writes_per_shot = True
    output_setting.use_custom_playback_range = True
    output_setting.custom_start_frame = 0
    output_setting.custom_end_frame = 1
    output_setting.zero_pad_frame_numbers = 4
    output_setting.use_custom_frame_rate = True
    output_setting.output_frame_rate = unreal.FrameRate(24, 1)

    deferred = config.find_or_add_setting_by_class(unreal.MoviePipelineDeferredPassBase)
    deferred.disable_multisample_effects = True
    config.find_or_add_setting_by_class(unreal.MoviePipelineImageSequenceOutput_PNG)
    config.initialize_transient_settings()
    unreal.EditorAssetLibrary.save_asset(config_path, False)
    unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)

    write_manifest(
        setup_manifest,
        success=True,
        failure_mode="",
        mode="setup",
        output_dir=str(output_dir),
        frame_dir=str(frame_dir),
        temp_name=temp_name,
        map_path=map_path,
        sequence_path=seq_path,
        config_path=config_path,
        map_object_path=f"{map_path}.L_{temp_name}",
        sequence_object_path=f"{seq_path}.{sequence_name}",
        config_object_path=f"{config_path}.{config_name}",
        resolution=[res_x, res_y],
        camera={
            "projection_mode": "ORTHOGRAPHIC",
            "location": [camera_x, camera_y, camera_z],
            "rotation": [-90.0, 0.0, 0.0],
            "ortho_width": ortho_width,
        },
        background="black_world",
        debug_geometry="none",
        temp_assets_are_regenerable=True,
    )
    unreal.log(f"[T66MRQIsolationSetup] success frame_dir={frame_dir}")


def cleanup_mode():
    output_dir = pathlib.Path(command_line_value("T66MRQIsolationOutput", DEFAULT_OUTPUT_DIR))
    output_dir.mkdir(parents=True, exist_ok=True)
    cleanup_manifest = output_dir / "cleanup_manifest.json"
    temp_name = command_line_value("T66MRQIsolationTempName", DEFAULT_TEMP_NAME)
    _map_dir, map_path, seq_path, config_path = object_paths(temp_name)
    cleanup_assets(map_path, seq_path, config_path)
    write_manifest(
        cleanup_manifest,
        success=True,
        failure_mode="",
        mode="cleanup",
        temp_name=temp_name,
        deleted_assets=[config_path, seq_path, map_path],
    )
    unreal.log(f"[T66MRQIsolationSetup] cleanup success temp_name={temp_name}")


try:
    if command_line_flag("T66MRQIsolationCleanup"):
        cleanup_mode()
    else:
        setup_assets()
    unreal.SystemLibrary.quit_editor()
except Exception as exc:
    output_dir = pathlib.Path(command_line_value("T66MRQIsolationOutput", DEFAULT_OUTPUT_DIR))
    output_dir.mkdir(parents=True, exist_ok=True)
    write_manifest(
        output_dir / "setup_manifest.json",
        success=False,
        failure_mode="exception",
        exception=str(exc),
        traceback=traceback.format_exc(),
    )
    unreal.log_error(f"[T66MRQIsolationSetup] exception {exc}")
    unreal.SystemLibrary.quit_editor()
