# Section 10 - Repo and Pipeline State

## Git Branch

Current branch at audit time:

- `alpha-0.6`

Remote default:

- `origin/main`

## Last 10 Commits on main

- `405fbc79 Version 3.6 UI library standardization`
- `19332486 Version 3.5 UI chrome sync`
- `a7a0c4f2 Version 3.5`
- `f0b9daa0 Route UI buttons through shared helpers`
- `316165a4 Version 3.4`
- `68afbd2d Version 3.35`
- `450a4a4d Version 3.3`
- `188fc03c Version 3.2`
- `b2412b5b Version 3.1`
- `b3a7cb18 Version 3.0 UI and Steam leaderboard pass`

## Dirty Working Tree Summary

Captured with:

- `git status --porcelain=v1 --untracked-files=no`

The working tree was very dirty before these report files were created.

Summary:

- Total tracked status lines: 1,403
- Modified: 717
- Deleted: 357
- Added: 223
- Added+modified: 87
- Staged+modified: 11
- Other staged/deleted combinations: 8

Top-level distribution:

- `Content`: 873
- `Source`: 249
- `SourceAssets`: 175
- `ToonStyle`: 42
- `Model Generation`: 24
- `Scripts`: 21
- `Gameplay`: 9
- `UI`: 5
- `Config`: 3
- `AGENTS.md`: 1
- `T66.uproject`: 1

Representative visible changes:

- Large ToonStyle/Pixal3D asset and tooling wave.
- Numerous `Content/Characters` and boss/enemy asset changes.
- Source changes across core, gameplay, UI, minigames.
- `Source/T66Versus` deleted.
- Several `Source/T66Mini` classes deleted/changed.
- Config files changed.
- Model Generation docs/scripts changed.
- Audit/report additions should now be considered a separate additional change after this audit.

## Cross-Platform Artifacts

No Mac/Linux build artifacts were found in:

- `Binaries`
- `Intermediate`
- `Saved`

Windows staged artifact exists:

- `Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`

The audit worker checked that the project shortcut pointed to that staged exe at the time of the read-only audit, but this report-writing pass did not restage or launch.

## Pipeline Scripts

Standalone:

- `Scripts/StageStandaloneBuild.ps1`
  - Win64 `BuildCookRun` workflow.
  - stages to `Saved\StagedBuilds`.
  - copies loose runtime roots.
  - refreshes shortcuts.

Steam:

- `Tools/Release/Steam/UploadToSteam.ps1`
  - expects a build source.
  - copies staged build into Steam ContentBuilder.
  - removes local-only `steam_appid.txt` before upload.

Capture/UI:

- `Scripts/CaptureT66UIScreen.ps1`
- `Scripts/CaptureT66UIWidget.ps1`
- Both expose `ExtraArgs`.

## Existing Audit Folder Note

`Audit/README.md` says there are no active pending audit files, but `Audit/Pending/T66_PERFORMANCE_SOURCE_AUDIT_2026-05-14.md` exists. Treat this as an audit-index documentation gap.

