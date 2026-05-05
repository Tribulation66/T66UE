"""
Render an in-engine visual lineup for the Chad Batch01 heroes.

This is intentionally a viewport/render check, not just an asset-registry check:
it spawns the imported SkeletalMesh assets in a transient blank editor world,
places them on a floor at Z=0, and schedules a high-res screenshot from a
camera actor. Run twice: default captures Standard; pass -T66ChadBeachgoer
to capture Beachgoer.
"""

import csv
import json
import math
import os

import unreal


ACTIVE_HERO_IDS = (1, 3, 4, 5, 7, 8, 9, 11, 12, 13, 14, 15)
CSV_RELATIVE = ("Content", "Data", "CharacterVisuals.csv")
OUTPUT_DIR = os.path.join(
    unreal.SystemLibrary.get_project_directory(),
    "Saved",
    "ChadBatch01VisualCheck",
)
REPORT_PATH = os.path.join(
    unreal.SystemLibrary.get_project_directory(),
    "Saved",
    "ChadBatch01VisualLineupReport.json",
)

WIDTH = 2560
HEIGHT = 1440
SPACING_CM = 90.0
ORTHO_WIDTH_CM = 1500.0


def _clean_path(object_path):
    return (object_path or "").split(".", 1)[0]


def _load_asset(object_path):
    return unreal.EditorAssetLibrary.load_asset(_clean_path(object_path))


def _read_csv_rows():
    project_dir = unreal.SystemLibrary.get_project_directory()
    csv_path = os.path.join(project_dir, *CSV_RELATIVE)
    with open(csv_path, "r", encoding="utf-8", newline="") as handle:
        return {row["---"]: row for row in csv.DictReader(handle)}


def _row_id(hero_id, beachgoer):
    suffix = "_Chad_Beachgoer" if beachgoer else "_Chad"
    return f"Hero_{hero_id}{suffix}"


def _look_at_rotation(camera_location, target_location):
    direction = target_location - camera_location
    yaw = math.degrees(math.atan2(direction.y, direction.x))
    horizontal = math.sqrt(direction.x * direction.x + direction.y * direction.y)
    pitch = math.degrees(math.atan2(direction.z, horizontal))
    return unreal.Rotator(pitch, yaw, 0.0)


def _set_component_mesh(component, mesh):
    if hasattr(component, "set_skeletal_mesh_asset"):
        component.set_skeletal_mesh_asset(mesh)
        return
    if hasattr(component, "set_skeletal_mesh"):
        component.set_skeletal_mesh(mesh)
        return
    component.set_editor_property("skeletal_mesh_asset", mesh)


def _set_component_animation(component, animation):
    if not animation:
        return
    try:
        component.set_animation_mode(unreal.AnimationMode.ANIMATION_SINGLE_NODE)
    except Exception:
        try:
            component.set_animation_mode(unreal.AnimationMode.ANIMATION_SINGLE_NODE, True)
        except Exception:
            pass
    try:
        component.play_animation(animation, True)
    except Exception:
        try:
            component.set_animation(animation)
        except Exception:
            pass


def _spawn_floor():
    plane = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Plane.Plane")
    if not plane:
        return None
    actor = unreal.EditorLevelLibrary.spawn_actor_from_object(
        plane,
        unreal.Vector(0.0, 0.0, -2.0),
        unreal.Rotator(0.0, 0.0, 0.0),
        False,
    )
    if actor:
        actor.set_actor_label("Chad_VisualCheck_Floor")
        actor.set_editor_property("tags", [unreal.Name("ChadVisualCheck_Floor")])
        actor.set_actor_scale3d(unreal.Vector(16.0, 5.0, 1.0))
    return actor


def _spawn_lighting():
    directional = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.DirectionalLight,
        unreal.Vector(-300.0, -500.0, 900.0),
        unreal.Rotator(-45.0, -35.0, 0.0),
        False,
    )
    if directional:
        directional.set_actor_label("Chad_VisualCheck_KeyLight")
        try:
            directional.get_component_by_class(unreal.LightComponent).set_editor_property("intensity", 80000.0)
        except Exception:
            pass
    skylight = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.SkyLight,
        unreal.Vector(0.0, 0.0, 500.0),
        unreal.Rotator(0.0, 0.0, 0.0),
        False,
    )
    if skylight:
        skylight.set_actor_label("Chad_VisualCheck_SkyLight")
        try:
            skylight.get_component_by_class(unreal.SkyLightComponent).set_editor_property("intensity", 5.0)
        except Exception:
            pass


def _spawn_lineup(rows, beachgoer, y_offset=0.0, z_offset=0.0, yaw=-90.0):
    spawned = []
    start_x = -0.5 * SPACING_CM * (len(ACTIVE_HERO_IDS) - 1)
    for index, hero_id in enumerate(ACTIVE_HERO_IDS):
        row_name = _row_id(hero_id, beachgoer)
        row = rows.get(row_name)
        if not row:
            raise RuntimeError(f"Missing CharacterVisuals row: {row_name}")

        mesh = _load_asset(row.get("SkeletalMesh", ""))
        animation = _load_asset(row.get("AlertAnimation", ""))
        if not mesh or mesh.get_class().get_name() != "SkeletalMesh":
            raise RuntimeError(f"Missing skeletal mesh for {row_name}: {row.get('SkeletalMesh')}")

        location = unreal.Vector(start_x + index * SPACING_CM, y_offset, z_offset)
        actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
            unreal.SkeletalMeshActor,
            location,
            unreal.Rotator(0.0, yaw, 0.0),
            False,
        )
        if not actor:
            raise RuntimeError(f"Failed to spawn actor for {row_name}")
        actor.set_actor_label(f"VisualCheck_{row_name}")
        actor.set_editor_property(
            "tags",
            [
                unreal.Name("ChadVisualCheck_Lineup"),
                unreal.Name("ChadVisualCheck_Beachgoer" if beachgoer else "ChadVisualCheck_Standard"),
                unreal.Name(row_name),
            ],
        )

        component = actor.get_editor_property("skeletal_mesh_component")
        _set_component_mesh(component, mesh)
        _set_component_animation(component, animation)
        try:
            component.set_editor_property("update_animation_in_editor", True)
        except Exception:
            pass
        component.set_editor_property("cast_shadow", True)
        spawned.append(
            {
                "row_id": row_name,
                "mesh": mesh.get_path_name(),
                "animation": animation.get_path_name() if animation else "",
                "location": [location.x, location.y, location.z],
            }
        )
    return spawned


def _capture_lineup(world, name):
    camera_location = unreal.Vector(0.0, -950.0, 135.0)
    target = unreal.Vector(0.0, 0.0, 95.0)
    camera = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.CameraActor,
        camera_location,
        _look_at_rotation(camera_location, target),
        False,
    )
    if not camera:
        raise RuntimeError(f"Failed to spawn camera for {name}")
        camera.set_actor_label(f"Chad_VisualCheck_Camera_{name}")
    try:
        camera_component = camera.get_editor_property("camera_component")
        camera_component.set_editor_property("projection_mode", unreal.CameraProjectionMode.ORTHOGRAPHIC)
        camera_component.set_editor_property("ortho_width", ORTHO_WIDTH_CM)
    except Exception as exc:
        unreal.log_warning(f"[CaptureTypeABatch01VisualLineup] Could not set ortho camera properties: {exc}")

    try:
        unreal.EditorLevelLibrary.set_level_viewport_camera_info(
            camera_location,
            _look_at_rotation(camera_location, target),
        )
        unreal.EditorLevelLibrary.editor_set_game_view(True)
        unreal.EditorLevelLibrary.editor_invalidate_viewports()
    except Exception as exc:
        unreal.log_warning(f"[CaptureTypeABatch01VisualLineup] Could not set editor viewport camera: {exc}")

    path = os.path.join(OUTPUT_DIR, f"{name}.png")
    if os.path.isfile(path):
        os.remove(path)

    unreal.AutomationLibrary.take_high_res_screenshot(
        WIDTH,
        HEIGHT,
        path,
        camera,
        False,
        False,
        unreal.ComparisonTolerance.LOW,
        f"Chad visual lineup: {name}",
        1.0,
        True,
    )
    unreal.log(f"[CaptureTypeABatch01VisualLineup] Scheduled screenshot {path}")
    return path


def _clear_visual_actors():
    for actor in unreal.EditorLevelLibrary.get_all_level_actors():
        try:
            label = actor.get_actor_label()
        except Exception:
            label = ""
        if label.startswith("VisualCheck_") or label.startswith("TypeA_VisualCheck_") or label.startswith("Chad_VisualCheck_"):
            unreal.EditorLevelLibrary.destroy_actor(actor)


def main():
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    try:
        unreal.EditorPythonScripting.set_keep_python_script_alive(True)
    except Exception as exc:
        unreal.log_warning(f"[CaptureTypeABatch01VisualLineup] Could not keep editor alive after script: {exc}")
    rows = _read_csv_rows()
    world = unreal.EditorLoadingAndSavingUtils.new_blank_map(False)
    if not world:
        world = unreal.EditorLevelLibrary.get_editor_world()
    if not world:
        raise RuntimeError("Could not create or resolve an editor world for visual capture")

    command_line = unreal.SystemLibrary.get_command_line()
    beachgoer = "-T66ChadBeachgoer" in command_line or "-T66TypeABeachgoer" in command_line
    name = "ChadBatch01_Beachgoer_Lineup" if beachgoer else "ChadBatch01_Standard_Lineup"

    _clear_visual_actors()
    _spawn_floor()
    _spawn_lighting()
    spawned = _spawn_lineup(rows, beachgoer)
    screenshot = _capture_lineup(world, name)

    if os.path.isfile(REPORT_PATH):
        try:
            with open(REPORT_PATH, "r", encoding="utf-8") as handle:
                report = json.load(handle)
        except Exception:
            report = {"screenshots": [], "lineups": []}
    else:
        report = {"screenshots": [], "lineups": []}

    report["screenshots"] = [p for p in report.get("screenshots", []) if os.path.basename(p) != os.path.basename(screenshot)]
    report["lineups"] = [l for l in report.get("lineups", []) if l.get("name") != name]
    report["screenshots"].append(screenshot)
    report["lineups"].append({"name": name, "beachgoer": beachgoer, "actors": spawned})

    with open(REPORT_PATH, "w", encoding="utf-8") as handle:
        json.dump(report, handle, indent=2)
    unreal.log(f"[CaptureTypeABatch01VisualLineup] Wrote {REPORT_PATH}")
    unreal.log(f"[CaptureTypeABatch01VisualLineup] Screenshots: {report['screenshots']}")


if __name__ == "__main__":
    main()
