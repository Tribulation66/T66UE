import unreal

LOG = "[SetupRetroPs1Variants]"
SOURCE_INSTANCE = "/Game/UE5RFX/Materials/PostProcess/UE5RFX_PPM_PS1_Sample_Instance"
DEST_DIR = "/Game/Materials/Retro/PS1"


def log(message):
    unreal.log(f"{LOG} {message}")


def warn(message):
    unreal.log_warning(f"{LOG} {message}")


def ensure_directory(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def set_switch(asset, name, value):
    unreal.MaterialEditingLibrary.set_material_instance_static_switch_parameter_value(asset, name, value)


def build_variant_name(use_color_lut, use_scene_depth_fog, use_bayer_dithering):
    return f"MI_T66_PS1_C{int(use_color_lut)}_S{int(use_scene_depth_fog)}_B{int(use_bayer_dithering)}"


def build_variant_asset_path(asset_name):
    return f"{DEST_DIR}/{asset_name}"


def rebuild_variant(use_color_lut, use_scene_depth_fog, use_bayer_dithering):
    asset_name = build_variant_name(use_color_lut, use_scene_depth_fog, use_bayer_dithering)
    asset_path = build_variant_asset_path(asset_name)

    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        unreal.EditorAssetLibrary.delete_asset(asset_path)

    if not unreal.EditorAssetLibrary.duplicate_asset(SOURCE_INSTANCE, asset_path):
        raise RuntimeError(f"Failed to duplicate source instance to {asset_path}")

    asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    if not asset or not isinstance(asset, unreal.MaterialInstanceConstant):
        raise RuntimeError(f"Failed to load duplicated material instance {asset_path}")

    # Preserve the sample defaults for the other static switches and only vary the hidden PS1 options.
    set_switch(asset, "UseDithering", True)
    set_switch(asset, "UseFog", True)
    set_switch(asset, "Use 15bit Color Depth", True)
    set_switch(asset, "2DFogFalloff", False)
    set_switch(asset, "UseColorLUT", use_color_lut)
    set_switch(asset, "UseSceneDepthForFog", use_scene_depth_fog)
    set_switch(asset, "UseBayerDithering", use_bayer_dithering)

    unreal.MaterialEditingLibrary.update_material_instance(asset)
    unreal.EditorAssetLibrary.save_loaded_asset(asset)
    log(f"Built {asset_path}")


def main():
    ensure_directory(DEST_DIR)

    source_asset = unreal.EditorAssetLibrary.load_asset(SOURCE_INSTANCE)
    if not source_asset or not isinstance(source_asset, unreal.MaterialInstanceConstant):
        raise RuntimeError(f"Source material instance not found: {SOURCE_INSTANCE}")

    for use_color_lut in (False, True):
        for use_scene_depth_fog in (False, True):
            for use_bayer_dithering in (False, True):
                rebuild_variant(use_color_lut, use_scene_depth_fog, use_bayer_dithering)

    log("Finished generating PS1 variant material instances")


if __name__ == "__main__":
    main()
