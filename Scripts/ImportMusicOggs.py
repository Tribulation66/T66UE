import os
import unreal


def _log(msg: str) -> None:
    unreal.log(f"[ImportMusicOggs] {msg}")


def _find_ogg_files(content_dir: str) -> list[str]:
    out: list[str] = []
    for root, _, files in os.walk(content_dir):
        for f in files:
            if f.lower().endswith(".ogg"):
                out.append(os.path.join(root, f))
    return out


def _to_game_path(project_content_dir: str, filesystem_dir: str) -> str:
    """
    Convert 'C:/UE/T66/Content/Audio/OSTS/Heroes/Hero_X' to '/Game/Audio/OSTS/Heroes/Hero_X'
    """
    rel = os.path.relpath(filesystem_dir, project_content_dir).replace("\\", "/")
    return "/Game/" + rel


def main() -> None:
    project_content_dir = unreal.Paths.project_content_dir()
    ost_fs_root = os.path.join(project_content_dir, "Audio", "OSTS")

    if not os.path.isdir(ost_fs_root):
        _log(f"OST folder not found: {ost_fs_root}")
        return

    oggs = _find_ogg_files(ost_fs_root)
    if not oggs:
        _log("No .ogg files found under Content/Audio/OSTS.")
        return

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    tasks: list[unreal.AssetImportTask] = []

    for src in oggs:
        dest_dir = os.path.dirname(src)
        dest_game_path = _to_game_path(project_content_dir, dest_dir)

        task = unreal.AssetImportTask()
        task.filename = src
        task.destination_path = dest_game_path
        task.replace_existing = True
        task.automated = True
        task.save = True
        tasks.append(task)

    _log(f"Importing {len(tasks)} ogg file(s) into SoundWave assets...")
    asset_tools.import_asset_tasks(tasks)

    # Save any dirty packages
    unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
    _log("Done.")


if __name__ == "__main__":
    main()

