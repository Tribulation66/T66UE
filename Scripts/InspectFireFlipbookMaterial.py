import unreal

ASSET_PATHS = [
    "/Game/VFX/Idols/Fire/MI_IdolFire_Explosion_Flipbook_Additive",
    "/Game/VFX/Idols/Fire/M_IdolFire_Explosion_Flipbook_Additive",
]


def inspect(asset_path: str) -> None:
    asset = unreal.load_asset(asset_path)
    if not asset:
        unreal.log_warning(f"[InspectFireFlipbook] Missing asset: {asset_path}")
        return

    unreal.log(f"[InspectFireFlipbook] Asset={asset_path} Class={asset.get_class().get_name()}")

    mel = unreal.MaterialEditingLibrary
    try:
        scalar_names = mel.get_scalar_parameter_names(asset)
        unreal.log(f"[InspectFireFlipbook] Scalars={list(scalar_names)}")
    except Exception as exc:
        unreal.log_warning(f"[InspectFireFlipbook] Scalar query failed: {exc}")

    try:
        texture_names = mel.get_texture_parameter_names(asset)
        unreal.log(f"[InspectFireFlipbook] Textures={list(texture_names)}")
    except Exception as exc:
        unreal.log_warning(f"[InspectFireFlipbook] Texture query failed: {exc}")

    if isinstance(asset, unreal.MaterialInstanceConstant):
        try:
            tex = mel.get_material_instance_texture_parameter_value(asset, "FlipbookTexture")
            unreal.log(f"[InspectFireFlipbook] FlipbookTexture={tex.get_path_name() if tex else 'None'}")
        except Exception as exc:
            unreal.log_warning(f"[InspectFireFlipbook] FlipbookTexture failed: {exc}")

        for name in ("Rows", "Columns", "GlowStrength", "Age01"):
            try:
                value = mel.get_material_instance_scalar_parameter_value(asset, name)
                unreal.log(f"[InspectFireFlipbook] {name}={value}")
            except Exception as exc:
                unreal.log_warning(f"[InspectFireFlipbook] {name} failed: {exc}")


for path in ASSET_PATHS:
    inspect(path)
