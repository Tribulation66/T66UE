"""
Create the SC_Music / SC_SFX SoundClass assets referenced by T66PlayerSettingsSubsystem.

UT66PlayerSettingsSubsystem preloads /Game/Audio/SC_Music and /Game/Audio/SC_SFX
(T66PlayerSettingsSubsystem.cpp) and logs missing-package warnings when they do not
exist. Volume control itself works through component multipliers, so these assets are
plain default SoundClasses whose only job is to exist (see AUDIO_SYSTEM.md and
Content/Audio/pending_issues_Audio.md). Idempotent: re-running skips existing assets.

Run headless (editor closed):
  UnrealEditor-Cmd.exe C:/UE/T66/T66.uproject -run=pythonscript -script="C:/UE/T66/Scripts/SetupAudioSoundClasses.py"
"""

import unreal

SOUND_CLASSES = (
    ("/Game/Audio", "SC_Music"),
    ("/Game/Audio", "SC_SFX"),
)


def main():
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    created = 0
    for package_path, name in SOUND_CLASSES:
        object_path = f"{package_path}/{name}"
        if unreal.EditorAssetLibrary.does_asset_exist(object_path):
            unreal.log(f"SoundClass already exists: {object_path}")
            continue
        factory = unreal.SoundClassFactory()
        asset = asset_tools.create_asset(name, package_path, unreal.SoundClass, factory)
        if asset and unreal.EditorAssetLibrary.save_asset(object_path):
            created += 1
            unreal.log(f"Created SoundClass: {object_path}")
        else:
            unreal.log_error(f"Failed to create or save SoundClass: {object_path}")
    unreal.log(f"SoundClass setup complete: {created} created, {len(SOUND_CLASSES) - created} already present.")


main()
