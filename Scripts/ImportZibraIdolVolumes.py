import unreal


VOLUME_ROOT = "/Game/VFX/Idols/Fire/Zibra/Volumes"
IMPORTS = {
    "GroundExplosion": r"C:\UE\T66\SourceAssets\Import\VFX\ExternalVDB\ZibraFree\ZIbraVDB Sample Effects\GroundExplosion.zibravdb",
    "Shockwave": r"C:\UE\T66\SourceAssets\Import\VFX\ExternalVDB\ZibraFree\ZIbraVDB Sample Effects\Shockwave.zibravdb",
    "ShockwaveExplosion": r"C:\UE\T66\SourceAssets\Import\VFX\ExternalVDB\ZibraFree\ZIbraVDB Sample Effects\ShockwaveExplosion.zibravdb",
    "FireTornado": r"C:\UE\T66\SourceAssets\Import\VFX\ExternalVDB\ZibraFree\ZIbraVDB Sample Effects\FireTornado.zibravdb",
}


def log(message: str) -> None:
    unreal.log(f"[IMPORT ZIBRA IDOLS] {message}")


def ensure_directory(path: str) -> None:
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def import_volume(asset_name: str, source_path: str) -> str:
    destination_object_path = f"{VOLUME_ROOT}/{asset_name}.{asset_name}"
    destination_asset_path = f"{VOLUME_ROOT}/{asset_name}"
    if unreal.EditorAssetLibrary.does_asset_exist(destination_object_path):
        log(f"Reusing existing asset {destination_object_path}")
        return destination_object_path

    task = unreal.AssetImportTask()
    task.set_editor_property("filename", source_path)
    task.set_editor_property("destination_path", VOLUME_ROOT)
    task.set_editor_property("destination_name", asset_name)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("replace_existing_settings", True)
    task.set_editor_property("automated", True)
    task.set_editor_property("save", True)

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

    if task.imported_object_paths:
        imported_path = task.imported_object_paths[0]
        if unreal.EditorAssetLibrary.does_asset_exist(imported_path) and unreal.load_asset(destination_asset_path) is not None:
            log(f"Imported {source_path} -> {imported_path}")
            return imported_path

    if unreal.EditorAssetLibrary.does_asset_exist(destination_object_path) and unreal.load_asset(destination_asset_path) is not None:
        log(f"Imported {source_path} -> {destination_object_path} (resolved by destination path)")
        return destination_object_path

    raise RuntimeError(f"Failed to import {source_path}")


def main() -> None:
    ensure_directory("/Game/VFX")
    ensure_directory("/Game/VFX/Idols")
    ensure_directory("/Game/VFX/Idols/Fire")
    ensure_directory("/Game/VFX/Idols/Fire/Zibra")
    ensure_directory(VOLUME_ROOT)

    imported_paths = []
    failed_assets = []
    for asset_name, source_path in IMPORTS.items():
        try:
            imported_paths.append(import_volume(asset_name, source_path))
        except Exception as exc:
            failed_assets.append(f"{asset_name}: {exc}")
            unreal.log_warning(f"[IMPORT ZIBRA IDOLS] Failed {asset_name}: {exc}")

    unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
    log(f"Completed import. Assets={', '.join(imported_paths)}")
    if failed_assets:
        unreal.log_warning(f"[IMPORT ZIBRA IDOLS] Failures={'; '.join(failed_assets)}")


main()
