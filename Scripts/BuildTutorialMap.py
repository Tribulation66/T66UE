import unreal

LOG_PREFIX = "[BuildTutorialMap]"
MAP_PATH = "/Game/Maps/Gameplay_Tutorial"
AUTHORED_TAG = "T66_TutorialAuthored"
MAP_READY_TAG = "T66_Tutorial_MapReady"
TUTORIAL_FLOOR_TAG = "T66_Floor_Tutorial"


def log(message: str) -> None:
    unreal.log(f"{LOG_PREFIX} {message}")


def warn(message: str) -> None:
    unreal.log_warning(f"{LOG_PREFIX} {message}")


def _name_list(*tags: str):
    return [unreal.Name(tag) for tag in tags if tag]


def load_map():
    log(f"Loading {MAP_PATH}")
    world = unreal.EditorLoadingAndSavingUtils.load_map(MAP_PATH)
    if not world:
        raise RuntimeError(f"Failed to load map: {MAP_PATH}")
    return world


def cleanup_old_authored_actors() -> None:
    actors = list(unreal.EditorLevelLibrary.get_all_level_actors() or [])
    removed = 0
    for actor in actors:
        tags = {str(tag) for tag in actor.tags}
        if AUTHORED_TAG in tags:
            unreal.EditorLevelLibrary.destroy_actor(actor)
            removed += 1
    log(f"Removed {removed} previously authored tutorial actors")


def set_actor_tags(actor, *tags: str) -> None:
    actor.set_editor_property("tags", _name_list(*tags))


def spawn_static_box(label: str, location: unreal.Vector, size_xyz: tuple[float, float, float], tags: list[str]):
    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.StaticMeshActor, location, unreal.Rotator(0.0, 0.0, 0.0))
    if not actor:
        raise RuntimeError(f"Failed to spawn StaticMeshActor: {label}")

    actor.set_actor_label(label)
    set_actor_tags(actor, *tags)

    smc = actor.static_mesh_component
    cube = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Cube.Cube")
    if not cube:
        raise RuntimeError("Failed to load /Engine/BasicShapes/Cube.Cube")

    smc.set_static_mesh(cube)
    smc.set_mobility(unreal.ComponentMobility.STATIC)
    smc.set_collision_profile_name("BlockAll")
    actor.set_actor_scale3d(unreal.Vector(size_xyz[0] / 100.0, size_xyz[1] / 100.0, size_xyz[2] / 100.0))
    return actor


def spawn_target_point(actor_class, label: str, location: unreal.Vector, rotation: unreal.Rotator, tags: list[str]):
    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(actor_class, location, rotation)
    if not actor:
        raise RuntimeError(f"Failed to spawn {label}")
    actor.set_actor_label(label)
    set_actor_tags(actor, *tags)
    return actor


def author_route_geometry() -> None:
    floor_tags = [AUTHORED_TAG, TUTORIAL_FLOOR_TAG]
    wall_tags = [AUTHORED_TAG]

    spawn_static_box("Tutorial_BaseFloor", unreal.Vector(0.0, 62000.0, -50.0), (9000.0, 13000.0, 100.0), floor_tags)

    # Route pads
    spawn_static_box("Tutorial_StartPad", unreal.Vector(-3200.0, 57100.0, 0.0), (2000.0, 1600.0, 40.0), floor_tags)
    spawn_static_box("Tutorial_MovePad", unreal.Vector(-3200.0, 58850.0, 0.0), (2000.0, 2000.0, 40.0), floor_tags)
    spawn_static_box("Tutorial_AutoPocket", unreal.Vector(-800.0, 59350.0, 0.0), (2200.0, 1800.0, 40.0), floor_tags)
    spawn_static_box("Tutorial_LockPocket", unreal.Vector(1800.0, 60350.0, 0.0), (2200.0, 1800.0, 40.0), floor_tags)
    spawn_static_box("Tutorial_ItemPocket", unreal.Vector(2350.0, 60750.0, 0.0), (1500.0, 1200.0, 40.0), floor_tags)
    spawn_static_box("Tutorial_IdolPocket", unreal.Vector(1500.0, 61950.0, 0.0), (2200.0, 1800.0, 40.0), floor_tags)
    spawn_static_box("Tutorial_FountainPocket", unreal.Vector(-300.0, 63150.0, 0.0), (2200.0, 1800.0, 40.0), floor_tags)
    spawn_static_box("Tutorial_TotemPocket", unreal.Vector(1500.0, 64350.0, 0.0), (2200.0, 1800.0, 40.0), floor_tags)
    spawn_static_box("Tutorial_FinalArena", unreal.Vector(0.0, 66500.0, 0.0), (3600.0, 3000.0, 40.0), floor_tags)
    spawn_static_box("Tutorial_PortalOverlook", unreal.Vector(0.0, 68350.0, 0.0), (2200.0, 1600.0, 40.0), floor_tags)

    # Corridors
    spawn_static_box("Tutorial_Path_01", unreal.Vector(-3200.0, 57950.0, 0.0), (900.0, 900.0, 40.0), floor_tags)
    spawn_static_box("Tutorial_Path_02", unreal.Vector(-2000.0, 59350.0, 0.0), (700.0, 1400.0, 40.0), floor_tags)
    spawn_static_box("Tutorial_Path_03", unreal.Vector(650.0, 59850.0, 0.0), (1700.0, 800.0, 40.0), floor_tags)
    spawn_static_box("Tutorial_Path_04", unreal.Vector(1800.0, 61250.0, 0.0), (800.0, 1600.0, 40.0), floor_tags)
    spawn_static_box("Tutorial_Path_05", unreal.Vector(600.0, 62550.0, 0.0), (1700.0, 800.0, 40.0), floor_tags)
    spawn_static_box("Tutorial_Path_06", unreal.Vector(600.0, 63750.0, 0.0), (1700.0, 800.0, 40.0), floor_tags)
    spawn_static_box("Tutorial_Path_07", unreal.Vector(700.0, 65350.0, 0.0), (1800.0, 900.0, 40.0), floor_tags)
    spawn_static_box("Tutorial_Path_08", unreal.Vector(0.0, 67500.0, 0.0), (900.0, 1200.0, 40.0), floor_tags)

    # Corridor walls and one jump obstacle to make the early lesson feel authored.
    spawn_static_box("Tutorial_StartWall_Left", unreal.Vector(-4150.0, 58000.0, 110.0), (120.0, 2200.0, 220.0), wall_tags)
    spawn_static_box("Tutorial_StartWall_Right", unreal.Vector(-2250.0, 58000.0, 110.0), (120.0, 2200.0, 220.0), wall_tags)
    spawn_static_box("Tutorial_JumpBarrier", unreal.Vector(-3200.0, 58150.0, 70.0), (900.0, 180.0, 140.0), wall_tags)
    spawn_static_box("Tutorial_FinalWall_Left", unreal.Vector(-1850.0, 66500.0, 130.0), (120.0, 3200.0, 260.0), wall_tags)
    spawn_static_box("Tutorial_FinalWall_Right", unreal.Vector(1850.0, 66500.0, 130.0), (120.0, 3200.0, 260.0), wall_tags)

    # Light cover props so the route reads as discrete stations.
    spawn_static_box("Tutorial_Cover_A", unreal.Vector(-1200.0, 59200.0, 90.0), (180.0, 180.0, 180.0), wall_tags)
    spawn_static_box("Tutorial_Cover_B", unreal.Vector(1250.0, 60200.0, 90.0), (180.0, 180.0, 180.0), wall_tags)
    spawn_static_box("Tutorial_Cover_C", unreal.Vector(350.0, 66450.0, 90.0), (180.0, 180.0, 180.0), wall_tags)


def author_markers() -> None:
    marker_tags = [AUTHORED_TAG]

    spawn_target_point(
        unreal.PlayerStart,
        "Tutorial_PlayerSpawn",
        unreal.Vector(-3200.0, 56850.0, 120.0),
        unreal.Rotator(0.0, 90.0, 0.0),
        marker_tags + ["T66_Tutorial_PlayerSpawn"],
    )

    spawn_target_point(unreal.TargetPoint, "Tutorial_MapReady", unreal.Vector(0.0, 62000.0, 50.0), unreal.Rotator(0.0, 0.0, 0.0), marker_tags + [MAP_READY_TAG])
    spawn_target_point(unreal.TargetPoint, "Tutorial_GuideStart", unreal.Vector(-3200.0, 57250.0, 0.0), unreal.Rotator(0.0, -90.0, 0.0), marker_tags + ["T66_Tutorial_GuideStart"])
    spawn_target_point(unreal.TargetPoint, "Tutorial_Stop_Move", unreal.Vector(-3200.0, 58850.0, 0.0), unreal.Rotator(0.0, -90.0, 0.0), marker_tags + ["T66_Tutorial_Stop_Move"])
    spawn_target_point(unreal.TargetPoint, "Tutorial_Stop_AutoAttack", unreal.Vector(-800.0, 59350.0, 0.0), unreal.Rotator(0.0, 180.0, 0.0), marker_tags + ["T66_Tutorial_Stop_AutoAttack"])
    spawn_target_point(unreal.TargetPoint, "Tutorial_Spawn_AutoAttackEnemy", unreal.Vector(-200.0, 59350.0, 110.0), unreal.Rotator(0.0, 180.0, 0.0), marker_tags + ["T66_Tutorial_Spawn_AutoAttackEnemy"])
    spawn_target_point(unreal.TargetPoint, "Tutorial_Stop_Lock", unreal.Vector(1200.0, 60350.0, 0.0), unreal.Rotator(0.0, 180.0, 0.0), marker_tags + ["T66_Tutorial_Stop_Lock"])
    spawn_target_point(unreal.TargetPoint, "Tutorial_Spawn_LockEnemy", unreal.Vector(1950.0, 60350.0, 110.0), unreal.Rotator(0.0, 180.0, 0.0), marker_tags + ["T66_Tutorial_Spawn_LockEnemy"])
    spawn_target_point(unreal.TargetPoint, "Tutorial_Stop_Item", unreal.Vector(2350.0, 60750.0, 0.0), unreal.Rotator(0.0, 180.0, 0.0), marker_tags + ["T66_Tutorial_Stop_Item"])
    spawn_target_point(unreal.TargetPoint, "Tutorial_Spawn_ItemBag", unreal.Vector(2350.0, 60750.0, 20.0), unreal.Rotator(0.0, 180.0, 0.0), marker_tags + ["T66_Tutorial_Spawn_ItemBag"])
    spawn_target_point(unreal.TargetPoint, "Tutorial_Stop_Idol", unreal.Vector(900.0, 61950.0, 0.0), unreal.Rotator(0.0, 180.0, 0.0), marker_tags + ["T66_Tutorial_Stop_Idol"])
    spawn_target_point(unreal.TargetPoint, "Tutorial_Spawn_IdolAltar", unreal.Vector(1500.0, 61950.0, 0.0), unreal.Rotator(0.0, 180.0, 0.0), marker_tags + ["T66_Tutorial_Spawn_IdolAltar"])
    spawn_target_point(unreal.TargetPoint, "Tutorial_Stop_Fountain", unreal.Vector(-900.0, 63150.0, 0.0), unreal.Rotator(0.0, 0.0, 0.0), marker_tags + ["T66_Tutorial_Stop_Fountain"])
    spawn_target_point(unreal.TargetPoint, "Tutorial_Spawn_Fountain", unreal.Vector(-300.0, 63150.0, 0.0), unreal.Rotator(0.0, 0.0, 0.0), marker_tags + ["T66_Tutorial_Spawn_Fountain"])
    spawn_target_point(unreal.TargetPoint, "Tutorial_Stop_Totem", unreal.Vector(900.0, 64350.0, 0.0), unreal.Rotator(0.0, 180.0, 0.0), marker_tags + ["T66_Tutorial_Stop_Totem"])
    spawn_target_point(unreal.TargetPoint, "Tutorial_Spawn_Totem", unreal.Vector(1500.0, 64350.0, 0.0), unreal.Rotator(0.0, 180.0, 0.0), marker_tags + ["T66_Tutorial_Spawn_Totem"])
    spawn_target_point(unreal.TargetPoint, "Tutorial_Spawn_TotemWave", unreal.Vector(1850.0, 64650.0, 0.0), unreal.Rotator(0.0, 180.0, 0.0), marker_tags + ["T66_Tutorial_Spawn_TotemWave"])
    spawn_target_point(unreal.TargetPoint, "Tutorial_Stop_Final", unreal.Vector(0.0, 65750.0, 0.0), unreal.Rotator(0.0, 180.0, 0.0), marker_tags + ["T66_Tutorial_Stop_Final"])
    spawn_target_point(unreal.TargetPoint, "Tutorial_Spawn_FinalArena", unreal.Vector(0.0, 66500.0, 0.0), unreal.Rotator(0.0, 180.0, 0.0), marker_tags + ["T66_Tutorial_Spawn_FinalArena"])
    spawn_target_point(unreal.TargetPoint, "Tutorial_Stop_Portal", unreal.Vector(0.0, 67850.0, 0.0), unreal.Rotator(0.0, -90.0, 0.0), marker_tags + ["T66_Tutorial_Stop_Portal"])
    spawn_target_point(unreal.TargetPoint, "Tutorial_Spawn_Portal", unreal.Vector(0.0, 68350.0, 0.0), unreal.Rotator(0.0, -90.0, 0.0), marker_tags + ["T66_Tutorial_Spawn_Portal"])


def save_map() -> None:
    if not unreal.EditorLevelLibrary.save_current_level():
        warn("save_current_level() returned False, attempting save_dirty_packages")
        unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
    log("Gameplay_Tutorial saved")


def main():
    load_map()
    cleanup_old_authored_actors()
    author_route_geometry()
    author_markers()
    save_map()
    log("Tutorial route and markers authored successfully")


if __name__ == "__main__":
    main()
