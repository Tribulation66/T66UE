"""
Import every .ogg under Content/Audio/OSTS into matching /Game/Audio/OSTS packages.

Companion to Scripts/ComposeT66PlaceholderOSTs.py and the UT66MusicSubsystem folder
contract: the SoundWave asset is created next to its source .ogg, same folder, same
name, so the music subsystem's folder scan finds it. Re-running re-imports in place,
which is also the replacement workflow for professional tracks later.

Run headless:
  UnrealEditor-Cmd.exe C:/UE/T66/T66.uproject -run=pythonscript -script="C:/UE/T66/Scripts/ImportGeneratedOSTs.py"
"""

from pathlib import Path

import unreal

OSTS_GAME_PATH = "/Game/Audio/OSTS"

# Commandlets run without an audio device, so the audio decoder modules never load and
# the first imported SoundWave trips "Decoder for AudioFormat 'BINKA' not found"
# (SoundWave.cpp ensure), failing the commandlet exit code despite a successful import.
AUDIO_DECODER_MODULES = (
    "BinkAudioDecoder",
    "AdpcmAudioDecoder",
    "VorbisAudioDecoder",
    "OpusAudioDecoder",
    "RadAudioDecoder",
)


def ensure_audio_decoder_modules_loaded():
    for module_name in AUDIO_DECODER_MODULES:
        try:
            unreal.load_module(module_name)
        except Exception as err:
            unreal.log_warning(f"Could not load audio decoder module {module_name}: {err}")


def main():
    ensure_audio_decoder_modules_loaded()
    project_root = Path(unreal.SystemLibrary.get_project_directory()).resolve()
    osts_root = project_root / "Content" / "Audio" / "OSTS"
    if not osts_root.exists():
        unreal.log_error(f"OSTS source folder not found: {osts_root}")
        return

    tasks = []
    for ogg in sorted(osts_root.rglob("*.ogg")):
        rel_parent = ogg.parent.relative_to(osts_root)
        dest = OSTS_GAME_PATH if str(rel_parent) == "." else f"{OSTS_GAME_PATH}/{rel_parent.as_posix()}"

        task = unreal.AssetImportTask()
        task.set_editor_property("filename", str(ogg))
        task.set_editor_property("destination_path", dest)
        task.set_editor_property("destination_name", ogg.stem)
        task.set_editor_property("automated", True)
        task.set_editor_property("replace_existing", True)
        try:
            task.set_editor_property("replace_existing_settings", True)
        except Exception:
            pass
        task.set_editor_property("save", True)
        tasks.append(task)

    if not tasks:
        unreal.log_warning("No .ogg files found under Content/Audio/OSTS.")
        return

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)

    imported = 0
    for task in tasks:
        paths = list(task.get_editor_property("imported_object_paths") or [])
        if paths:
            imported += 1
            unreal.log(f"Imported: {paths[0]}")
        else:
            unreal.log_error(f"Import produced no asset: {task.get_editor_property('filename')}")

    unreal.log(f"OST import complete: {imported}/{len(tasks)} SoundWave assets imported under {OSTS_GAME_PATH}.")


main()
