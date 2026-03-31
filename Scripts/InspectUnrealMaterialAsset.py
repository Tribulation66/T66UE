import unreal


ASSET_PATHS = [
    "/Game/UE5RFX/Materials/BaseMaterials/UE5RFX_Flipbook_BaseMaterial",
    "/Game/VFX/Hero1/MI_Hero1_Attack_Impact",
    "/Game/UE5RFX/Blueprints/Explosions/Resources/Bin/UE5RFX_Explosion_Flipbook",
]


def print_material_details(asset_path: str) -> None:
    asset = unreal.load_asset(asset_path)
    if not asset:
        unreal.log_warning(f"Missing asset: {asset_path}")
        return

    unreal.log(f"Asset: {asset_path} class={asset.get_class().get_name()}")

    mel = unreal.MaterialEditingLibrary
    for label, getter in (
        ("scalar", getattr(mel, "get_scalar_parameter_names", None)),
        ("vector", getattr(mel, "get_vector_parameter_names", None)),
        ("texture", getattr(mel, "get_texture_parameter_names", None)),
    ):
        if getter is None:
            unreal.log_warning(f"MaterialEditingLibrary has no getter for {label}")
            continue
        try:
            names = getter(asset)
            unreal.log(f"  {label}: {[str(name) for name in names]}")
        except Exception as exc:
            unreal.log_warning(f"  {label}: failed: {exc}")

    try:
        if isinstance(asset, unreal.Texture):
            unreal.log(f"  size={asset.blueprint_get_size_x()}x{asset.blueprint_get_size_y()}")
    except Exception:
        pass


for path in ASSET_PATHS:
    print_material_details(path)
