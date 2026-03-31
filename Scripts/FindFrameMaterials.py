import unreal


asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
assets = asset_registry.get_assets_by_path("/Game", recursive=True)

mel = unreal.MaterialEditingLibrary

for asset_data in assets:
    if asset_data.asset_class_path.asset_name not in ("Material", "MaterialInstanceConstant", "MaterialInstanceDynamic"):
        continue

    path = asset_data.object_path.string
    asset = unreal.load_asset(path)
    if not asset:
        continue

    try:
        scalar_names = [str(name) for name in mel.get_scalar_parameter_names(asset)]
    except Exception:
        continue

    if "Frame" in scalar_names:
        unreal.log(f"{path} :: {asset.get_class().get_name()} :: scalar={scalar_names}")
