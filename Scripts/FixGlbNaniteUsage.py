import unreal

LOG = "[FixGlbNaniteUsage]"
MATERIAL_PATHS = [
    "/Game/Materials/M_GLB_Unlit",
    "/Game/Materials/Retro/M_GLB_Unlit_RetroGeometry",
]


def _safe_set(obj, prop, value):
    try:
        obj.set_editor_property(prop, value)
    except Exception as exc:
        unreal.log_warning(f"{LOG} set_editor_property({prop}) failed: {exc}")


def _fix_material(path):
    mat = unreal.EditorAssetLibrary.load_asset(path)
    if not mat:
        unreal.log_warning(f"{LOG} Missing material: {path}")
        return False

    _safe_set(mat, "used_with_instanced_static_meshes", True)
    _safe_set(mat, "used_with_nanite", True)
    _safe_set(mat, "two_sided", True)

    unreal.MaterialEditingLibrary.recompile_material(mat)
    unreal.EditorAssetLibrary.save_loaded_asset(mat)
    unreal.log(f"{LOG} Saved {path} with used_with_nanite=True")
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
