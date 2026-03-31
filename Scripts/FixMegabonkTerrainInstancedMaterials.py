import unreal

LOG = "[FixMegabonkTerrainInstancedMaterials]"
MATERIAL_PATHS = [
    "/Game/Materials/M_Environment_Unlit",
    "/Game/World/Terrain/Megabonk/MI_MegabonkBlock",
    "/Game/World/Terrain/Megabonk/MI_MegabonkDirt",
    "/Game/World/Terrain/Megabonk/MI_MegabonkSlope",
]


def _safe_set(obj, prop, value):
    try:
        obj.set_editor_property(prop, value)
    except Exception as exc:
        unreal.log_warning(f"{LOG} set_editor_property({prop}) failed on {obj.get_name()}: {exc}")


def _fix_material(path):
    asset = unreal.EditorAssetLibrary.load_asset(path)
    if not asset:
        unreal.log_warning(f"{LOG} Missing material asset: {path}")
        return False

    _safe_set(asset, "used_with_instanced_static_meshes", True)
    try:
        unreal.MaterialEditingLibrary.recompile_material(asset)
    except Exception:
        pass
    unreal.EditorAssetLibrary.save_loaded_asset(asset)
    unreal.log(f"{LOG} Saved {path} with instanced-static-mesh usage enabled")
    return True


def main():
    unreal.log(f"{LOG} START")
    fixed = 0
    for path in MATERIAL_PATHS:
        if _fix_material(path):
            fixed += 1
    unreal.log(f"{LOG} DONE fixed={fixed}")


if __name__ == "__main__":
    main()
