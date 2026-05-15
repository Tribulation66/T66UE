# Release Agents

## Owns

Project policy, packaging, staged standalone, Steamworks upload, private testing, version naming, release validation, and console command references.

## Trigger Words

Release, package, stage, standalone, taskbar shortcut, Steam, Steamworks, SteamPipe, upload, private test, branch, build ID, version, tag, release.

## Read First

- `Release/README.md`
- `Release/PROJECT_GUIDELINES_INSTRUCTIONS.md`
- `Release/Steam/STEAMWORKS_UPLOAD_AND_TESTING_INSTRUCTIONS.md` for Steam operations.
- `Demo/DEMO_RELEASE_INSTRUCTIONS.md` for demo-specific release work.

## Hard Rules

- Packaged Development standalone is the runtime source of truth for runtime-facing changes.
- Refresh staged standalone and verify the taskbar shortcut when the change affects the playable standalone.
- Do not upload the inner `Saved/StagedBuilds/Windows/T66` folder to Steam. Upload the root staged Windows folder.
