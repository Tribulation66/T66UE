import unreal

LOG = "[InspectMegabonkTerrainAssets]"
ASSET_PATHS = [
    "/Game/World/Terrain/Megabonk/SM_MegabonkBlock",
    "/Game/World/Terrain/Megabonk/SM_MegabonkSlope",
]


def log(msg):
    unreal.log(f"{LOG} {msg}")


def inspect_mesh(path):
    mesh = unreal.EditorAssetLibrary.load_asset(path)
    if not mesh:
        log(f"Missing mesh: {path}")
        return

    try:
        bounds = mesh.get_bounding_box()
        size = bounds.max - bounds.min
        center = (bounds.min + bounds.max) * 0.5
        log(f"{path} bounds min={bounds.min} max={bounds.max} size={size} center={center}")
    except Exception as exc:
        log(f"{path} bounds unavailable: {exc}")

    try:
        body_setup = mesh.get_editor_property("body_setup")
        if body_setup:
            agg = body_setup.get_editor_property("agg_geom")
            box_count = len(list(agg.box_elems or []))
            sphyl_count = len(list(agg.sphyl_elems or []))
            sphere_count = len(list(agg.sphere_elems or []))
            convex_count = len(list(agg.convex_elems or []))
            collision_trace_flag = body_setup.get_editor_property("collision_trace_flag")
            log(
                f"{path} collision trace={collision_trace_flag} boxes={box_count} sphyls={sphyl_count} "
                f"spheres={sphere_count} convex={convex_count}"
            )
        else:
            log(f"{path} has no body_setup")
    except Exception as exc:
        log(f"{path} collision unavailable: {exc}")


def main():
    log("START")
    for path in ASSET_PATHS:
        inspect_mesh(path)
    log("DONE")


if __name__ == "__main__":
    main()
