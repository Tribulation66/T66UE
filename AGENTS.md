# AGENTS.md

## Standalone Shortcut Rule

- After any change that affects the playable standalone build, refresh the staged standalone build and verify the taskbar `T66 Standalone.lnk` shortcut points to `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.

## Version Naming Rule

- When the user names a new version, treat that value as both the Unreal game version and the GitHub repo version. Update `ProjectVersion` in `Config/DefaultGame.ini` and use the same exact value for the GitHub branch/tag/release naming unless the user explicitly asks for a different split.

## Script Lifecycle Rule

- Keep reusable master scripts tight and documented. Delete task-specific scripts after the task is accomplished, but first move any durable process improvements into the relevant master script, README, or process doc.
